/*
 * Copyright 2008-2009 Benjamin C. Meyer <ben@meyerhome.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

/****************************************************************************
**
** Copyright (C) 2007-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.2, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "cookiejar.h"

#include "autosaver.h"
#include "browserapplication.h"

#include <qapplication.h>
#include <qdir.h>
#include <qmetaobject.h>
#include <qsettings.h>
#include <qurl.h>
#include <QNetworkCookie>

#include <qdebug.h>

static const unsigned int JAR_VERSION = 23;

QT_BEGIN_NAMESPACE
QDataStream &operator<<(QDataStream &stream, const QList<QNetworkCookie> &list)
{
    stream << JAR_VERSION;
    stream << quint32(list.size());
    for (int i = 0; i < list.size(); ++i)
        stream << list.at(i).toRawForm();
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QList<QNetworkCookie> &list)
{
    list.clear();

    quint32 version;
    stream >> version;

    if (version != JAR_VERSION)
        return stream;

    quint32 count;
    stream >> count;
    for (quint32 i = 0; i < count; ++i) {
        QByteArray value;
        stream >> value;
        QList<QNetworkCookie> newCookies = QNetworkCookie::parseCookies(value);
        if (newCookies.count() == 0 && value.length() != 0) {
            qWarning() << "CookieJar: Unable to parse saved cookie:" << value;
        }
        for (int j = 0; j < newCookies.count(); ++j)
            list.append(newCookies.at(j));
        if (stream.atEnd())
            break;
    }
    return stream;
}
QT_END_NAMESPACE

CookieJar::CookieJar(QObject *parent)
    : NetworkCookieJar(parent)
    , m_loaded(false)
    , m_saveTimer(new AutoSaver(this))
    , m_filterTrackingCookies(false)
    , m_acceptCookies(AcceptOnlyFromSitesNavigatedTo)
    , m_isPrivate(false)
{
}

CookieJar::~CookieJar()
{
    if (m_loaded && m_keepCookies == KeepUntilExit)
        clear();
    m_saveTimer->saveIfNeccessary();
}

void CookieJar::setPrivate(bool isPrivate)
{
    m_isPrivate = isPrivate;
}

void CookieJar::clear()
{
    if (!m_loaded)
        load();
    setAllCookies(QList<QNetworkCookie>());
    m_saveTimer->changeOccurred();
    emit cookiesChanged();
}

void CookieJar::load()
{
    if (m_loaded)
        return;
    // load cookies and exceptions
    qRegisterMetaTypeStreamOperators<QList<QNetworkCookie> >("QList<QNetworkCookie>");
    QSettings cookieSettings(BrowserApplication::dataFilePath(QLatin1String("cookies.ini")), QSettings::IniFormat);
    if (!m_isPrivate) {
        setAllCookies(qvariant_cast<QList<QNetworkCookie> >(cookieSettings.value(QLatin1String("cookies"))));
    }
    cookieSettings.beginGroup(QLatin1String("Exceptions"));
    m_exceptions_block = cookieSettings.value(QLatin1String("block")).toStringList();
    m_exceptions_allow = cookieSettings.value(QLatin1String("allow")).toStringList();
    m_exceptions_allowForSession = cookieSettings.value(QLatin1String("allowForSession")).toStringList();
    qSort(m_exceptions_block.begin(), m_exceptions_block.end());
    qSort(m_exceptions_allow.begin(), m_exceptions_allow.end());
    qSort(m_exceptions_allowForSession.begin(), m_exceptions_allowForSession.end());

    loadSettings();
}

void CookieJar::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("cookies"));
    QByteArray value = settings.value(QLatin1String("acceptCookies"),
                                      QLatin1String("AcceptOnlyFromSitesNavigatedTo")).toByteArray();
    QMetaEnum acceptPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    m_acceptCookies = acceptPolicyEnum.keyToValue(value) == -1 ?
                      AcceptOnlyFromSitesNavigatedTo :
                      static_cast<AcceptPolicy>(acceptPolicyEnum.keyToValue(value));

    value = settings.value(QLatin1String("keepCookiesUntil"), QLatin1String("KeepUntilExpire")).toByteArray();
    QMetaEnum keepPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("KeepPolicy"));
    m_keepCookies = keepPolicyEnum.keyToValue(value) == -1 ?
                    KeepUntilExpire :
                    static_cast<KeepPolicy>(keepPolicyEnum.keyToValue(value));

    if (m_keepCookies == KeepUntilExit)
        setAllCookies(QList<QNetworkCookie>());

    m_loaded = true;
    m_filterTrackingCookies = settings.value(QLatin1String("filterTrackingCookies"), m_filterTrackingCookies).toBool();
    m_sessionLength = settings.value(QLatin1String("sessionLength"), -1).toInt();
    emit cookiesChanged();
}

void CookieJar::save()
{
    if (!m_loaded || m_isPrivate)
        return;
    purgeOldCookies();

    QSettings cookieSettings(BrowserApplication::dataFilePath(QLatin1String("cookies.ini")), QSettings::IniFormat);

    QList<QNetworkCookie> cookies = allCookies();
    for (int i = cookies.count() - 1; i >= 0; --i) {
        if (cookies.at(i).isSessionCookie())
            cookies.removeAt(i);
    }
    cookieSettings.setValue(QLatin1String("cookies"), qVariantFromValue<QList<QNetworkCookie> >(cookies));
    cookieSettings.beginGroup(QLatin1String("Exceptions"));
    cookieSettings.setValue(QLatin1String("block"), m_exceptions_block);
    cookieSettings.setValue(QLatin1String("allow"), m_exceptions_allow);
    cookieSettings.setValue(QLatin1String("allowForSession"), m_exceptions_allowForSession);

    // save cookie settings
    QSettings settings;
    settings.beginGroup(QLatin1String("cookies"));
    QMetaEnum acceptPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    settings.setValue(QLatin1String("acceptCookies"), QLatin1String(acceptPolicyEnum.valueToKey(m_acceptCookies)));

    QMetaEnum keepPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("KeepPolicy"));
    settings.setValue(QLatin1String("keepCookiesUntil"), QLatin1String(keepPolicyEnum.valueToKey(m_keepCookies)));

    settings.setValue(QLatin1String("filterTrackingCookies"), m_filterTrackingCookies);
    settings.setValue(QLatin1String("sessionLength"), m_sessionLength);
}

void CookieJar::purgeOldCookies()
{
    QList<QNetworkCookie> cookies = allCookies();
    if (cookies.isEmpty())
        return;
    int oldCount = cookies.count();
    QDateTime now = QDateTime::currentDateTime();
    for (int i = cookies.count() - 1; i >= 0; --i) {
        if (!cookies.at(i).isSessionCookie() && cookies.at(i).expirationDate() < now)
            cookies.removeAt(i);
    }
    if (oldCount == cookies.count())
        return;
    setAllCookies(cookies);
    emit cookiesChanged();
}

QList<QNetworkCookie> CookieJar::cookiesForUrl(const QUrl &url) const
{
    CookieJar *that = const_cast<CookieJar*>(this);
    if (!m_loaded)
        that->load();

    return NetworkCookieJar::cookiesForUrl(url);
}

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    if (!m_loaded)
        load();

    QString host = url.host();
    bool eBlock = isOnDomainList(m_exceptions_block, host);
    bool eAllow = !eBlock && isOnDomainList(m_exceptions_allow, host);
    bool eAllowSession = !eBlock && !eAllow && isOnDomainList(m_exceptions_allowForSession, host);

    bool addedCookies = false;
    // pass exceptions
    bool acceptInitially = (m_acceptCookies != AcceptNever);
    if ((acceptInitially && !eBlock)
        || (!acceptInitially && (eAllow || eAllowSession))) {
        // pass url domain == cookie domain
        QDateTime soon = QDateTime::currentDateTime();
        soon = soon.addDays(90);
        foreach (QNetworkCookie cookie, cookieList) {
            if (cookie.isSessionCookie() && m_sessionLength != -1) {
                QDateTime now = QDateTime::currentDateTime();
                cookie.setExpirationDate(now.addDays(m_sessionLength));
            }

            QList<QNetworkCookie> lst;
            if (!(m_filterTrackingCookies && cookie.name().startsWith("__utm"))) {


                if (eAllowSession) {
                    cookie.setExpirationDate(QDateTime());
                }
                if (m_keepCookies == KeepUntilTimeLimit
                    && !cookie.isSessionCookie()
                    && cookie.expirationDate() > soon) {
                    cookie.setExpirationDate(soon);
                }
                lst += cookie;
                if (NetworkCookieJar::setCookiesFromUrl(lst, url)) {
                    addedCookies = true;
                } else {
                    // finally force it in if wanted
                    if (m_acceptCookies == AcceptAlways) {
                        QList<QNetworkCookie> cookies = allCookies();
                        QList<QNetworkCookie>::Iterator it = cookies.begin(),
                                   end = cookies.end();
                        for (; it != end; ++it) {
                            // does this cookie already exist?
                            if (cookie.name() == it->name() &&
                                cookie.domain() == it->domain() &&
                                cookie.path() == it->path()) {
                                // found a match
                                cookies.erase(it);
                                break;
                            }
                        }

                        cookies += cookie;
                        setAllCookies(cookies);
                        addedCookies = true;
                    }
    #if 0
                    else
                        qWarning() << "setCookiesFromUrl failed" << url << cookieList.value(0).toRawForm();
    #endif
                }
#if 0
            } else {
                qWarning() << "cookie treated as tracking cookie" << cookie;
#endif
            }
        }
    }

    if (addedCookies) {
        m_saveTimer->changeOccurred();
        emit cookiesChanged();
    }
    return addedCookies;
}

QList<QNetworkCookie> CookieJar::cookies() const
{
    CookieJar *that = const_cast<CookieJar*>(this);
    if (!m_loaded)
        that->load();

    return allCookies();
}

void CookieJar::setCookies(const QList<QNetworkCookie> &cookies)
{
    if (!m_loaded)
        load();
    setAllCookies(cookies);
    m_saveTimer->changeOccurred();
    emit cookiesChanged();
}

bool CookieJar::isOnDomainList(const QStringList &rules, const QString &domain)
{
    // Either the rule matches the domain exactly
    // or the domain ends with ".rule"
    foreach (const QString &rule, rules) {
        if (rule.startsWith(QLatin1String("."))) {
            if (domain.endsWith(rule))
                return true;

            QStringRef withoutDot = rule.rightRef(rule.size() - 1);
            if (domain == withoutDot)
                return true;
        } else {
            QStringRef domainEnding = domain.rightRef(rule.size() + 1);
            if (!domainEnding.isEmpty()
                && domainEnding.at(0) == QLatin1Char('.')
                && domain.endsWith(rule)) {
                return true;
            }

            if (rule == domain)
                return true;
        }
    }
    return false;
}

CookieJar::AcceptPolicy CookieJar::acceptPolicy() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_acceptCookies;
}

void CookieJar::setAcceptPolicy(AcceptPolicy policy)
{
    if (!m_loaded)
        load();
    if (policy == m_acceptCookies)
        return;
    m_acceptCookies = policy;
    m_saveTimer->changeOccurred();
}

CookieJar::KeepPolicy CookieJar::keepPolicy() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_keepCookies;
}

void CookieJar::setKeepPolicy(KeepPolicy policy)
{
    if (!m_loaded)
        load();
    if (policy == m_keepCookies)
        return;
    m_keepCookies = policy;
    m_saveTimer->changeOccurred();
}

QStringList CookieJar::blockedCookies() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_exceptions_block;
}

QStringList CookieJar::allowedCookies() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_exceptions_allow;
}

QStringList CookieJar::allowForSessionCookies() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_exceptions_allowForSession;
}

void CookieJar::setBlockedCookies(const QStringList &list)
{
    if (!m_loaded)
        load();
    m_exceptions_block = list;
    qSort(m_exceptions_block.begin(), m_exceptions_block.end());
    applyRules();
    m_saveTimer->changeOccurred();
}

void CookieJar::setAllowedCookies(const QStringList &list)
{
    if (!m_loaded)
        load();
    m_exceptions_allow = list;
    qSort(m_exceptions_allow.begin(), m_exceptions_allow.end());
    applyRules();
    m_saveTimer->changeOccurred();
}

void CookieJar::setAllowForSessionCookies(const QStringList &list)
{
    if (!m_loaded)
        load();
    m_exceptions_allowForSession = list;
    qSort(m_exceptions_allowForSession.begin(), m_exceptions_allowForSession.end());
    applyRules();
    m_saveTimer->changeOccurred();
}

void CookieJar::applyRules()
{
    QList<QNetworkCookie> cookies = allCookies();
    bool changed = false;
    for (int i = cookies.count() - 1; i >= 0; --i) {
        const QNetworkCookie &cookie = cookies.at(i);
        if (isOnDomainList(m_exceptions_block, cookie.domain())) {
            cookies.removeAt(i);
            changed = true;
        } else if (isOnDomainList(m_exceptions_allowForSession, cookie.domain())) {
            const_cast<QNetworkCookie&>(cookie).setExpirationDate(QDateTime());
            changed = true;
        }
    }
    if (changed) {
        setAllCookies(cookies);
        m_saveTimer->changeOccurred();
        emit cookiesChanged();
    }
}

bool CookieJar::filterTrackingCookies() const
{
    return this->m_filterTrackingCookies;
}

void CookieJar::setFilterTrackingCookies(bool filterTrackingCookies)
{
    this->m_filterTrackingCookies = filterTrackingCookies;
}

