/*
   Copyright (C) 2009, Torch Mobile Inc. and Linden Research, Inc. All rights reserved.
*/

/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Torch Mobile Inc. (http://www.torchmobile.com/) code
 *
 * The Initial Developer of the Original Code is:
 *   Benjamin Meyer (benjamin.meyer@torchmobile.com)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef NETWORKCOOKIEJARPRIVATE_H
#define NETWORKCOOKIEJARPRIVATE_H

#include "trie_p.h"
#include <QNetworkCookie>

QT_BEGIN_NAMESPACE
QDataStream &operator<<(QDataStream &stream, const QNetworkCookie &cookie)
{
    stream << cookie.toRawForm();
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QNetworkCookie &cookie)
{
    QByteArray value;
    stream >> value;
    QList<QNetworkCookie> newCookies = QNetworkCookie::parseCookies(value);
    if (!newCookies.isEmpty())
        cookie = newCookies.first();
    return stream;
}
QT_END_NAMESPACE

class NetworkCookieJarPrivate {
public:
    NetworkCookieJarPrivate()
        : setSecondLevelDomain(false)
    {}

    Trie<QNetworkCookie> tree;
    mutable bool setSecondLevelDomain;
    mutable QStringList secondLevelDomains;

    bool matchesBlacklist(const QString &string) const;
    bool matchingDomain(const QNetworkCookie &cookie, const QUrl &url) const;
    QString urlPath(const QUrl &url) const;
    bool matchingPath(const QNetworkCookie &cookie, const QString &urlPath) const;
};

#endif

