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

#include "adblocknetwork.h"

#include "adblockblockednetworkreply.h"
#include "adblockmanager.h"
#include "adblocksubscription.h"

#include <qdebug.h>

// #define ADBLOCKNETWORK_DEBUG

AdBlockNetwork::AdBlockNetwork(QObject *parent)
    : QObject(parent)
{
}

QNetworkReply *AdBlockNetwork::block(const QNetworkRequest &request)
{
    QUrl url = request.url();

    if (url.scheme() == QLatin1String("data"))
        return 0;

    AdBlockManager *manager = AdBlockManager::instance();
    if (!manager->isEnabled())
        return 0;

    QString urlString = QString::fromUtf8(url.toEncoded());
    const AdBlockRule *blockedRule = 0;
#if defined(ADBLOCKNETWORK_DEBUG)
    const AdBlockSubscription *blockingSubscription = 0;
#endif

    QList<AdBlockSubscription*> subscriptions = manager->subscriptions();
    foreach (AdBlockSubscription *subscription, subscriptions) {
        if (subscription->allow(urlString))
            return 0;

        if (const AdBlockRule *rule = subscription->block(urlString)) {
            blockedRule = rule;
#if defined(ADBLOCKNETWORK_DEBUG)
            blockingSubscription = subscription;
#endif
            break;
        }
    }

    if (blockedRule) {
#if defined(ADBLOCKNETWORK_DEBUG)
        qDebug() << "AdBlockNetwork::" << __FUNCTION__ << "rule:" << blockedRule->filter() << "subscription:" << blockingSubscription->title() << url;
#endif
       AdBlockBlockedNetworkReply *reply = new AdBlockBlockedNetworkReply(request, blockedRule, this);
        return reply;
    }
    return 0;
}

