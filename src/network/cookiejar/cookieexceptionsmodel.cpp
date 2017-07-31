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

#include "cookieexceptionsmodel.h"

#include "cookiejar.h"

#include <qfontmetrics.h>

CookieExceptionsModel::CookieExceptionsModel(CookieJar *cookiejar, QObject *parent)
    : QAbstractTableModel(parent)
    , m_cookieJar(cookiejar)
{
    m_allowedCookies = m_cookieJar->allowedCookies();
    m_blockedCookies = m_cookieJar->blockedCookies();
    m_sessionCookies = m_cookieJar->allowForSessionCookies();
}

QVariant CookieExceptionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::SizeHintRole) {
        QFont font;
        font.setPointSize(10);
        QFontMetrics fm(font);
        int height = fm.height() + fm.height() / 3;
        int width = fm.width(headerData(section, orientation, Qt::DisplayRole).toString());
        return QSize(width, height);
    }

    if (orientation == Qt::Horizontal
        && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Website");
        case 1:
            return tr("Rule");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant CookieExceptionsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: {
        int row = index.row();
        if (row < m_allowedCookies.count()) {
            switch (index.column()) {
            case 0:
                return m_allowedCookies.at(row);
            case 1:
                return tr("Allow");
            }
        }
        row = row - m_allowedCookies.count();
        if (row < m_blockedCookies.count()) {
            switch (index.column()) {
            case 0:
                return m_blockedCookies.at(row);
            case 1:
                return tr("Block");
            }
        }
        row = row - m_blockedCookies.count();
        if (row < m_sessionCookies.count()) {
            switch (index.column()) {
            case 0:
                return m_sessionCookies.at(row);
            case 1:
                return tr("Allow For Session");
            }
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

int CookieExceptionsModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : 2;
}

int CookieExceptionsModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() || !m_cookieJar) ? 0 : m_allowedCookies.count() + m_blockedCookies.count() + m_sessionCookies.count();
}

bool CookieExceptionsModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || !m_cookieJar)
        return false;

    int lastRow = row + count - 1;
    beginRemoveRows(parent, row, lastRow);

    for (int i = lastRow; i >= row; --i) {
        int rowToRemove = i;
        if (rowToRemove < m_allowedCookies.count()) {
            m_allowedCookies.removeAt(rowToRemove);
            continue;
        }
        rowToRemove = rowToRemove - m_allowedCookies.count();
        if (rowToRemove < m_blockedCookies.count()) {
            m_blockedCookies.removeAt(rowToRemove);
            continue;
        }
        rowToRemove = rowToRemove - m_blockedCookies.count();
        if (rowToRemove < m_sessionCookies.count()) {
            m_sessionCookies.removeAt(rowToRemove);
            continue;
        }
    }
    endRemoveRows();
    return true;
}

void CookieExceptionsModel::addRule(QString host, CookieJar::CookieRule rule)
{
    if (host.isEmpty())
        return;
    switch (rule) {
        case CookieJar::Allow :
            addHost(host, m_allowedCookies, m_blockedCookies, m_sessionCookies);
            return;
        case CookieJar::Block :
            addHost(host, m_blockedCookies, m_allowedCookies, m_sessionCookies);
            return;
        case CookieJar::AllowForSession :
            addHost(host, m_sessionCookies, m_allowedCookies, m_blockedCookies);
            return;
    }
}

void CookieExceptionsModel::addHost(QString host, QStringList &add, QStringList &remove1, QStringList &remove2)
{
    beginResetModel();
    if (!add.contains(host)) {
        add.append(host);
        remove1.removeOne(host);
        remove2.removeOne(host);
    }
    // avoid to have similar rules, with or without starting dot, eg "arora-browser.org" and ".arora-browser.org"
    // means the same domain.
    QString otherRule;
    if (host.startsWith(QLatin1Char('.'))) {
        otherRule = host.mid(1);
    } else {
        otherRule = QLatin1String(".") + host;
    }
    add.removeOne(otherRule);
    remove1.removeOne(otherRule);
    remove2.removeOne(otherRule);
    endResetModel();
}

