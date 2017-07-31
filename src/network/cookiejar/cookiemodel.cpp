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

#include "cookiemodel.h"

#include "cookiejar.h"
#include "autosaver.h"

#include <qdatetime.h>
#include <qfontmetrics.h>

CookieModel::CookieModel(CookieJar *cookieJar, QObject *parent)
    : QAbstractTableModel(parent)
    , m_cookieJar(cookieJar)
{
    connect(m_cookieJar, SIGNAL(cookiesChanged()), this, SLOT(cookiesChanged()));
    m_cookies = m_cookieJar->cookies();
}

QVariant CookieModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::SizeHintRole) {
        QFont font;
        font.setPointSize(10);
        QFontMetrics fm(font);
        int height = fm.height() + fm.height() / 3;
        int width = fm.width(headerData(section, orientation, Qt::DisplayRole).toString());
        return QSize(width, height);
    }

    if (orientation == Qt::Horizontal) {
        if (role != Qt::DisplayRole)
            return QVariant();

        switch (section) {
        case 0:
            return tr("Website");
        case 1:
            return tr("Name");
        case 2:
            return tr("Path");
        case 3:
            return tr("Secure");
        case 4:
            return tr("Expires");
        case 5:
            return tr("Contents");
        default:
            return QVariant();
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant CookieModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_cookies.size())
        return QVariant();

    switch (role) {
    case CookieModel::SortRole:
    {
        QNetworkCookie cookie = m_cookies.at(index.row());
        switch (index.column()) {
        case 0:
            return cookie.domain();
        case 1:
            return cookie.name();
        case 2:
            return cookie.path();
        case 3:
            return cookie.isSecure();
        case 4:
            return cookie.expirationDate();
        case 5:
            return cookie.value();
        }
    }
    case Qt::DisplayRole:
    case Qt::EditRole: {
        QNetworkCookie cookie = m_cookies.at(index.row());
        switch (index.column()) {
        case 0:
            return cookie.domain();
        case 1:
            return cookie.name();
        case 2:
            return cookie.path();
        case 3:
            return cookie.isSecure() ? tr("true") : tr("false");
        case 4:
            return cookie.isSessionCookie() ? tr("Session cookie") : cookie.expirationDate().toString();
        case 5:
            return cookie.value();
        }
    }
    case Qt::FontRole: {
        QFont font;
        font.setPointSize(10);
        return font;
    }
    }

    return QVariant();
}

int CookieModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : 6;
}

int CookieModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() || !m_cookieJar) ? 0 : m_cookies.count();
}

bool CookieModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || !m_cookieJar)
        return false;
    int lastRow = row + count - 1;
    beginRemoveRows(parent, row, lastRow);
    QList<QNetworkCookie> lst = m_cookies;
    for (int i = lastRow; i >= row; --i) {
        lst.removeAt(i);
    }
    disconnect(m_cookieJar, SIGNAL(cookiesChanged()), this, SLOT(cookiesChanged()));
    m_cookieJar->setCookies(lst);
    connect(m_cookieJar, SIGNAL(cookiesChanged()), this, SLOT(cookiesChanged()));

    m_cookies = lst;
    endRemoveRows();
    return true;
}

void CookieModel::cookiesChanged()
{
    beginResetModel();
    if (m_cookieJar)
        m_cookies = m_cookieJar->cookies();
    endResetModel();
}
