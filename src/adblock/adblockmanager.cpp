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

#include "adblockmanager.h"

#include "autosaver.h"
#include "adblockdialog.h"
#include "adblocknetwork.h"
#include "adblockpage.h"
#include "adblocksubscription.h"
#include "browserapplication.h"
#include "networkaccessmanager.h"

#include <qstringlist.h>
#include <qsettings.h>

#include <qdebug.h>

// #define ADBLOCKMANAGER_DEBUG

AdBlockManager *AdBlockManager::s_adBlockManager = 0;

AdBlockManager::AdBlockManager(QObject *parent)
    : QObject(parent)
    , m_loaded(false)
    , m_enabled(true)
    , m_saveTimer(new AutoSaver(this))
    , m_adBlockDialog(0)
    , m_adBlockNetwork(0)
    , m_adBlockPage(0)
{
    connect(this, SIGNAL(rulesChanged()),
            m_saveTimer, SLOT(changeOccurred()));
}

AdBlockManager::~AdBlockManager()
{
    m_saveTimer->saveIfNeccessary();
}

AdBlockManager *AdBlockManager::instance()
{
    if (!s_adBlockManager) {
        // Set a parent that will delete us before the application exits
        s_adBlockManager = new AdBlockManager(BrowserApplication::networkAccessManager());
    }
    return s_adBlockManager;
}

bool AdBlockManager::isEnabled() const
{
    if (!m_loaded) {
        AdBlockManager *that = const_cast<AdBlockManager*>(this);
        that->load();
    }
    return m_enabled;
}

void AdBlockManager::setEnabled(bool enabled)
{
    if (isEnabled() == enabled)
        return;
    emit rulesGoingToChange();
    m_enabled = enabled;
    emit rulesChanged();
}

AdBlockNetwork *AdBlockManager::network()
{
    if (!m_adBlockNetwork)
        m_adBlockNetwork = new AdBlockNetwork(this);
    return m_adBlockNetwork;
}

AdBlockPage *AdBlockManager::page()
{
    if (!m_adBlockPage)
        m_adBlockPage = new AdBlockPage(this);
    return m_adBlockPage;
}

static QUrl customSubscriptionLocation()
{
    QString fileName = BrowserApplication::dataFilePath(QLatin1String("adblock_subscription_custom"));
    return QUrl::fromLocalFile(fileName);
}

QUrl AdBlockManager::customSubscriptionUrl()
{
    QUrl location = customSubscriptionLocation();
    QString encodedUrl = QString::fromUtf8(location.toEncoded());
    QUrl url(QString(QLatin1String("abp:subscribe?location=%1&title=%2"))
            .arg(encodedUrl)
            .arg(tr("Custom Rules")));
    return url;
}

AdBlockSubscription *AdBlockManager::customRules()
{
    QUrl location = customSubscriptionLocation();
    foreach (AdBlockSubscription *subscription, m_subscriptions) {
        if (subscription->location() == location)
            return subscription;
    }
    QUrl url = customSubscriptionUrl();
    AdBlockSubscription *customAdBlockSubscription = new AdBlockSubscription(url, this);
    addSubscription(customAdBlockSubscription);
    return customAdBlockSubscription;
}

QList<AdBlockSubscription*> AdBlockManager::subscriptions() const
{
    if (!m_loaded) {
        AdBlockManager *that = const_cast<AdBlockManager*>(this);
        that->load();
    }
    return m_subscriptions;
}

void AdBlockManager::removeSubscription(AdBlockSubscription *subscription)
{
    if (!subscription)
        return;
#if defined(ADBLOCKMANAGER_DEBUG)
    qDebug() << "AdBlockManager::" << __FUNCTION__ << subscription->location();
#endif
    m_saveTimer->saveIfNeccessary();
    emit rulesGoingToChange();
    m_subscriptions.removeOne(subscription);
    if (subscription->parent() == this)
        subscription->deleteLater();
    emit rulesChanged();
}

void AdBlockManager::addSubscription(AdBlockSubscription *subscription)
{
    if (!subscription)
        return;
#if defined(ADBLOCKMANAGER_DEBUG)
    qDebug() << "AdBlockManager::" << __FUNCTION__ << subscription->location();
#endif
    emit rulesGoingToChange();
    m_subscriptions.append(subscription);
    connect(subscription, SIGNAL(goingToChange()), this, SIGNAL(rulesGoingToChange()));
    connect(subscription, SIGNAL(rulesChanged()), this, SIGNAL(rulesChanged()));
    connect(subscription, SIGNAL(changed()), this, SIGNAL(rulesChanged()));
    emit rulesChanged();
}

void AdBlockManager::save()
{
#if defined(ADBLOCKMANAGER_DEBUG)
    qDebug() << "AdBlockManager::" << __FUNCTION__ << m_loaded;
#endif
    if (!m_loaded)
        return;

    QSettings settings;
    settings.beginGroup(QLatin1String("AdBlock"));
    settings.setValue(QLatin1String("enabled"), m_enabled);
    QStringList subscriptions;
    foreach (AdBlockSubscription *subscription, m_subscriptions) {
        if (!subscription)
            continue;
        subscriptions.append(QString::fromUtf8(subscription->url().toEncoded()));
        subscription->saveRules();
    }
    settings.setValue(QLatin1String("subscriptions"), subscriptions);
}

void AdBlockManager::load()
{
#if defined(ADBLOCKMANAGER_DEBUG)
    qDebug() << "AdBlockManager::" << __FUNCTION__ << m_loaded;
#endif

    if (m_loaded)
        return;
    m_loaded = true;

    QSettings settings;
    settings.beginGroup(QLatin1String("AdBlock"));
    m_enabled = settings.value(QLatin1String("enabled"), m_enabled).toBool();

    QStringList defaultSubscriptions;
    defaultSubscriptions.append(QString::fromUtf8(customSubscriptionUrl().toEncoded()));
    defaultSubscriptions.append(QLatin1String("abp:subscribe?location=http://adblockplus.mozdev.org/easylist/easylist.txt&title=EasyList"));

    QStringList subscriptions = settings.value(QLatin1String("subscriptions"), defaultSubscriptions).toStringList();
    foreach (const QString &subscription, subscriptions) {
        QUrl url = QUrl::fromEncoded(subscription.toUtf8());
        AdBlockSubscription *adBlockSubscription = new AdBlockSubscription(url, this);
        connect(adBlockSubscription, SIGNAL(goingToChange()), this, SIGNAL(rulesGoingToChange()));
        connect(adBlockSubscription, SIGNAL(rulesChanged()), this, SIGNAL(rulesChanged()));
        connect(adBlockSubscription, SIGNAL(changed()), this, SIGNAL(rulesChanged()));
        m_subscriptions.append(adBlockSubscription);
    }
}

AdBlockDialog *AdBlockManager::showDialog()
{
    if (!m_adBlockDialog) {
        m_adBlockDialog = new AdBlockDialog(0);
        m_adBlockDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    }
    m_adBlockDialog->show();
    return m_adBlockDialog;
}

