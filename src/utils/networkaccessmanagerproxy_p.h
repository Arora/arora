/**
 * Copyright (c) 2009, Benjamin C. Meyer  <ben@meyerhome.net>
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

#ifndef NETWORKACCESSMANAGERPROXY_P_H
#define NETWORKACCESSMANAGERPROXY_P_H

#include <qnetworkcookie.h>
#include <qnetworkcookiejar.h>

#include "networkaccessmanagerproxy.h"

class NetworkCookieJarProxy : public QNetworkCookieJar
{
    Q_OBJECT

public:
    NetworkCookieJarProxy(QObject *parent = 0)
        : QNetworkCookieJar(parent) { }

    inline QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const
        { return NetworkAccessManagerProxy::m_primaryManager->cookieJar()->cookiesForUrl(url); }

    inline bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
        { return NetworkAccessManagerProxy::m_primaryManager->cookieJar()->setCookiesFromUrl(cookieList, url); }

};

#endif // NETWORKACCESSMANAGERPROXY_P_H

