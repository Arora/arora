/**
 * Copyright (c) 2009, Benjamin C. Meyer <ben@meyerhome.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Benjamin Meyer nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "adblocksubscription.h"

#include "browserapplication.h"
#include "networkaccessmanager.h"

#include <qcryptographichash.h>
#include <qdebug.h>
#include <qfile.h>
#include <qnetworkreply.h>
#include <qtextstream.h>
#include <QUrlQuery>

// #define ADBLOCKSUBSCRIPTION_DEBUG

AdBlockSubscription::AdBlockSubscription(const QUrl &url, QObject *parent)
    : QObject(parent)
    , m_url(url.toEncoded())
    , m_enabled(false)
    , m_downloading(0)
{
    parseUrl(url);
}

void AdBlockSubscription::parseUrl(const QUrl &url)
{
#if defined(ADBLOCKSUBSCRIPTION_DEBUG)
    qDebug() << "AdBlockSubscription::" << __FUNCTION__ << url;
#endif
    if (url.scheme() != QLatin1String("abp"))
        return;
    if (url.path() != QLatin1String("subscribe"))
        return;
    QUrlQuery query(url.query());
    m_title = query.queryItemValue("title");
    m_enabled = query.queryItemValue("enabled") != QLatin1String("false");
    m_location = query.queryItemValue("location").toUtf8();
    QString lastUpdateString = query.queryItemValue("lastUpdate");
    m_lastUpdate = QDateTime::fromString(lastUpdateString, Qt::ISODate);
    loadRules();
}

QUrl AdBlockSubscription::url() const
{
    QUrl url;
    url.setScheme(QLatin1String("abp"));
    url.setPath(QLatin1String("subscribe"));

    QUrlQuery queryItems;

    queryItems.addQueryItem(QLatin1String("location"), QString::fromUtf8(m_location));
    queryItems.addQueryItem(QLatin1String("title"), m_title);
    if (!m_enabled)
        queryItems.addQueryItem(QLatin1String("enabled"), QLatin1String("false"));
    if (m_lastUpdate.isValid())
        queryItems.addQueryItem(QLatin1String("lastUpdate"), m_lastUpdate.toString(Qt::ISODate));
    url.setQuery(queryItems);
    return url;
}

bool AdBlockSubscription::isEnabled() const
{
    return m_enabled;
}

void AdBlockSubscription::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    populateCache();
    emit changed();
}

QString AdBlockSubscription::title() const
{
    return m_title;
}

void AdBlockSubscription::setTitle(const QString &title)
{
    if (m_title == title)
        return;
    m_title = title;
    emit changed();
}

QUrl AdBlockSubscription::location() const
{
    return QUrl::fromEncoded(m_location);
}

void AdBlockSubscription::setLocation(const QUrl &url)
{
    if (url == location())
        return;
    m_location = url.toEncoded();
    m_lastUpdate = QDateTime();
    emit changed();
}

QDateTime AdBlockSubscription::lastUpdate() const
{
    return m_lastUpdate;
}

QString AdBlockSubscription::rulesFileName() const
{
    if (location().scheme() == QLatin1String("file"))
        return location().toLocalFile();

    if (m_location.isEmpty())
        return QString();

    QByteArray sha1 = QCryptographicHash::hash(m_location, QCryptographicHash::Sha1).toHex();
    QString fileName = BrowserApplication::dataFilePath(QString(QLatin1String("adblock_subscription_%1")).arg(QLatin1String(sha1)));
    return fileName;
}

void AdBlockSubscription::loadRules()
{
    QString fileName = rulesFileName();
#if defined(ADBLOCKSUBSCRIPTION_DEBUG)
    qDebug() << "AdBlockSubscription::" << __FUNCTION__ << fileName;
#endif
    QFile file(fileName);
    if (file.exists()) {
        if (!file.open(QFile::ReadOnly)) {
            qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "Unable to open adblock file for reading" << fileName;
        } else {
            QTextStream textStream(&file);
            QString header = textStream.readLine(1024);
            if (!header.startsWith(QLatin1String("[Adblock"))) {
                qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "adblock file does not start with [Adblock" << fileName << "Header:" << header;
                file.close();
                file.remove();
                m_lastUpdate = QDateTime();
            } else {
                m_rules.clear();
                while (!textStream.atEnd()) {
                    QString line = textStream.readLine();
                    m_rules.append(AdBlockRule(line));
                }
                populateCache();
                emit rulesChanged();
            }
        }
    }

    if (!m_lastUpdate.isValid()
        || m_lastUpdate.addDays(7) < QDateTime::currentDateTime()) {
        updateNow();
    }
}

void AdBlockSubscription::updateNow()
{
#if defined(ADBLOCKSUBSCRIPTION_DEBUG)
    qDebug() << "AdBlockSubscription::" << __FUNCTION__ << location();
#endif
    if (m_downloading) {
#if defined(ADBLOCKSUBSCRIPTION_DEBUG)
        qDebug() << "AdBlockSubscription::" << __FUNCTION__ << "already downloading, stopping";
#endif
        return;
    }

    if (!location().isValid()) {
#if defined(ADBLOCKSUBSCRIPTION_DEBUG)
        qDebug() << "AdBlockSubscription::" << __FUNCTION__ << location() << "isn't valid";
#endif
        return;
    }

    if (location().scheme() == QLatin1String("file")) {
#if defined(ADBLOCKSUBSCRIPTION_DEBUG)
        qDebug() << "AdBlockSubscription::" << __FUNCTION__ << "local file, not downloading";
#endif
        m_lastUpdate = QDateTime::currentDateTime();
        loadRules();
        emit changed();
        return;
    }

    QNetworkRequest request(location());
    QNetworkReply *reply = BrowserApplication::networkAccessManager()->get(request);
    m_downloading = reply;
    connect(reply, SIGNAL(finished()), this, SLOT(rulesDownloaded()));
}

void AdBlockSubscription::rulesDownloaded()
{
#if defined(ADBLOCKSUBSCRIPTION_DEBUG)
    qDebug() << "AdBlockSubscription::" << __FUNCTION__ << rulesFileName();
#endif
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
#if defined(ADBLOCKSUBSCRIPTION_DEBUG)
        qDebug() << "AdBlockSubscription::" << __FUNCTION__ << "no reply?";
#endif
        return;
    }

    QByteArray response = reply->readAll();
    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    reply->close();
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "error" << reply->errorString();
        return;
    }

    if (redirect.isValid()) {
#if defined(ADBLOCKSUBSCRIPTION_DEBUG)
        qDebug() << "AdBlockSubscription::" << __FUNCTION__ << "redirect to:" << redirect;
#endif
        QNetworkRequest request(redirect);
        m_downloading = BrowserApplication::networkAccessManager()->get(request);
        connect(m_downloading, SIGNAL(finished()), this, SLOT(rulesDownloaded()));
        return;
    }

    if (response.isEmpty()) {
        qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "empty response";
        return;
    }

    QString fileName = rulesFileName();
    QFile file(fileName);
    if (!file.open(QFile::ReadWrite)) {
        qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "Unable to open adblock file for writing:" << fileName;
        return;
    }
    file.write(response);
    m_lastUpdate = QDateTime::currentDateTime();
    loadRules();
    emit changed();
    m_downloading = 0;
}

void AdBlockSubscription::saveRules()
{
#if defined(ADBLOCKSUBSCRIPTION_DEBUG)
    qDebug() << "AdBlockSubscription::" << __FUNCTION__ << rulesFileName() << m_rules.count();
#endif
    QString fileName = rulesFileName();
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QFile::ReadWrite | QIODevice::Truncate)) {
        qWarning() << "AdBlockSubscription::" << __FUNCTION__ << "Unable to open adblock file for writing:" << fileName;
        return;
    }

    QTextStream textStream(&file);
    textStream << "[Adblock Plus 0.7.1]" << endl;
    foreach (const AdBlockRule &rule, m_rules)
        textStream << rule.filter() << endl;
}

QList<const AdBlockRule*> AdBlockSubscription::pageRules() const
{
    return m_pageRules;
}

const AdBlockRule *AdBlockSubscription::allow(const QString &urlString) const
{
    foreach (const AdBlockRule *rule, m_networkExceptionRules) {
        if (rule->networkMatch(urlString))
            return rule;
    }
    return 0;
}

const AdBlockRule *AdBlockSubscription::block(const QString &urlString) const
{
    foreach (const AdBlockRule *rule, m_networkBlockRules) {
        if (rule->networkMatch(urlString))
            return rule;
    }
    return 0;
}

QList<AdBlockRule> AdBlockSubscription::allRules() const
{
    return m_rules;
}

void AdBlockSubscription::addRule(const AdBlockRule &rule)
{
#if defined(ADBLOCKSUBSCRIPTION_DEBUG)
    qDebug() << "AdBlockSubscription::" << __FUNCTION__ << rule.filter();
#endif
    m_rules.append(rule);
    populateCache();
    emit rulesChanged();
}

void AdBlockSubscription::removeRule(int offset)
{
#if defined(ADBLOCKSUBSCRIPTION_DEBUG)
    qDebug() << "AdBlockSubscription::" << __FUNCTION__ << offset << m_rules.count();
#endif
    if (offset < 0 || offset >= m_rules.count())
        return;
    m_rules.removeAt(offset);
    populateCache();
    emit rulesChanged();
}

void AdBlockSubscription::replaceRule(const AdBlockRule &rule, int offset)
{
    if (offset < 0 || offset >= m_rules.count())
        return;
    m_rules[offset] = rule;
    populateCache();
    emit rulesChanged();
}

void AdBlockSubscription::populateCache()
{
    m_networkExceptionRules.clear();
    m_networkBlockRules.clear();
    m_pageRules.clear();
    if (!isEnabled())
        return;

    for (int i = 0; i < m_rules.count(); ++i) {
        const AdBlockRule *rule = &m_rules.at(i);
        if (!rule->isEnabled())
            continue;

        if (rule->isCSSRule()) {
            m_pageRules.append(rule);
            continue;
        }

        if (rule->isException()) {
            m_networkExceptionRules.append(rule);
        } else {
            m_networkBlockRules.append(rule);
        }
    }
}

