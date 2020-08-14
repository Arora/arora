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

#include "cookiedialog.h"

#include <qsortfilterproxymodel.h>
#include <qdatetime.h>
#include <qheaderview.h>

#include "cookiemodel.h"
#include "cookieexceptionsdialog.h"

CookieDialog::CookieDialog(CookieJar *cookieJar, QWidget *parent)
    : QDialog(parent)
    , m_cookieJar(cookieJar)
{
    setupUi(this);
    setWindowFlags(Qt::Sheet);
    CookieModel *model = new CookieModel(cookieJar, this);
    m_proxyModel = new QSortFilterProxyModel(this);
    connect(search, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterFixedString(QString)));
    connect(removeButton, SIGNAL(clicked()), cookiesTable, SLOT(removeSelected()));
    connect(removeAllButton, SIGNAL(clicked()), cookiesTable, SLOT(removeAll()));
    connect(addRuleButton, SIGNAL(clicked()), this, SLOT(addRule()));
    m_proxyModel->setSourceModel(model);
    m_proxyModel->setSortRole(CookieModel::SortRole);
    cookiesTable->verticalHeader()->hide();
    cookiesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    cookiesTable->setModel(m_proxyModel);
    cookiesTable->setAlternatingRowColors(true);
    cookiesTable->setTextElideMode(Qt::ElideMiddle);
    cookiesTable->setShowGrid(false);
    cookiesTable->setSortingEnabled(true);
    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int height = fm.height() + fm.height() / 3;
    cookiesTable->verticalHeader()->setDefaultSectionSize(height);
    cookiesTable->verticalHeader()->setMinimumSectionSize(-1);
    for (int i = 0; i < model->columnCount(); ++i) {
        int header = cookiesTable->horizontalHeader()->sectionSizeHint(i);
        switch (i) {
        case 0:
            header = fm.horizontalAdvance(QLatin1String("averagehost.domain.com"));
            break;
        case 1:
            header = fm.horizontalAdvance(QLatin1String("_session_id"));
            break;
        case 4:
            header = fm.horizontalAdvance(QDateTime::currentDateTime().toString(Qt::LocalDate));
            break;
        }
        int buffer = fm.horizontalAdvance(QLatin1String("xx"));
        header += buffer;
        cookiesTable->horizontalHeader()->resizeSection(i, header);
    }
    cookiesTable->horizontalHeader()->setStretchLastSection(true);
}

void CookieDialog::addRule()
{
    const QModelIndexList selection = cookiesTable->selectionModel()->selectedRows();
    if (selection.isEmpty())
        return;
    QModelIndex firstSelected = selection.at(0);
    QModelIndex domainSelection = firstSelected.sibling(firstSelected.row(), 0);
    QString domain = m_proxyModel->data(domainSelection, Qt::DisplayRole).toString();
    CookieExceptionsDialog dialog(m_cookieJar, this);
    dialog.setDomainName(domain);
    dialog.exec();
}


