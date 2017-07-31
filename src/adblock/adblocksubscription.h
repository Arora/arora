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

#ifndef ADBLOCKSUBSCRIPTION_H
#define ADBLOCKSUBSCRIPTION_H

#include <qobject.h>

#include "adblockrule.h"

#include <qlist.h>
#include <qdatetime.h>

class QNetworkReply;
class QUrl;
class AdBlockSubscription : public QObject
{
    Q_OBJECT

signals:
    void goingToChange();
    void changed();
    void rulesChanged();

public:
    AdBlockSubscription(const QUrl &url, QObject *parent = 0);
    QUrl url() const;

    bool isEnabled() const;
    void setEnabled(bool enabled);

    QString title() const;
    void setTitle(const QString &title);

    QUrl location() const;
    void setLocation(const QUrl &url);

    void updateNow();
    QDateTime lastUpdate() const;

    void saveRules();

    const AdBlockRule *allow(const QString &urlString) const;
    const AdBlockRule *block(const QString &urlString) const;
    QList<const AdBlockRule*> pageRules() const;

    QList<AdBlockRule> allRules() const;
    void addRule(const AdBlockRule &rule);
    void removeRule(int offset);
    void replaceRule(const AdBlockRule &rule, int offset);

private slots:
    void rulesDownloaded();

private:
    void populateCache();
    QString rulesFileName() const;
    void parseUrl(const QUrl &url);
    void loadRules();

    QByteArray m_url;

    QString m_title;
    QByteArray m_location;
    QDateTime m_lastUpdate;
    bool m_enabled;

    QNetworkReply *m_downloading;
    QList<AdBlockRule> m_rules;

    // sorted list
    QList<const AdBlockRule*> m_networkExceptionRules;
    QList<const AdBlockRule*> m_networkBlockRules;
    QList<const AdBlockRule*> m_pageRules;
};

#endif // ADBLOCKSUBSCRIPTION_H

