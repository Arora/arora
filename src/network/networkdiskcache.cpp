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

#include "networkdiskcache.h"

#include "browserapplication.h"

#include <qdesktopservices.h>
#include <qsettings.h>

NetworkDiskCache::NetworkDiskCache(QObject *parent)
    : QNetworkDiskCache(parent)
    , m_private(false)
{
    QString diskCacheDirectory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                                + QLatin1String("/browser");
    setCacheDirectory(diskCacheDirectory);
    connect(BrowserApplication::instance(), SIGNAL(privacyChanged(bool)),
            this, SLOT(privacyChanged(bool)));
}

void NetworkDiskCache::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("network"));
    qint64 maximumCacheSize = settings.value(QLatin1String("maximumCacheSize"), 50).toInt();
    maximumCacheSize = maximumCacheSize * 1024 * 1024;
    setMaximumCacheSize(maximumCacheSize);
}

void NetworkDiskCache::privacyChanged(bool isPrivate)
{
    m_private = isPrivate;
}

QIODevice *NetworkDiskCache::prepare(const QNetworkCacheMetaData &metaData)
{
    if (m_private)
        return 0;
    return QNetworkDiskCache::prepare(metaData);
}

