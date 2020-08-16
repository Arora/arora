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

#include "cookieexceptionsdialog.h"

#include <qsortfilterproxymodel.h>

#include <qheaderview.h>
#include <qcompleter.h>
#include "cookiemodel.h"
#include "cookiejar.h"
#include "cookieexceptionsmodel.h"

CookieExceptionsDialog::CookieExceptionsDialog(CookieJar *cookieJar, QWidget *parent)
    : QDialog(parent)
    , m_cookieJar(cookieJar)
{
    setupUi(this);
    setWindowFlags(Qt::Sheet);
    connect(removeButton, SIGNAL(clicked()), exceptionTable, SLOT(removeSelected()));
    connect(removeAllButton, SIGNAL(clicked()), exceptionTable, SLOT(removeAll()));
    exceptionTable->verticalHeader()->hide();
    exceptionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    exceptionTable->setAlternatingRowColors(true);
    exceptionTable->setTextElideMode(Qt::ElideMiddle);
    exceptionTable->setShowGrid(false);
    exceptionTable->setSortingEnabled(true);
    m_exceptionsModel = new CookieExceptionsModel(cookieJar, this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_exceptionsModel);
    connect(search, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterFixedString(QString)));
    exceptionTable->setModel(m_proxyModel);

    CookieModel *cookieModel = new CookieModel(cookieJar, this);
    domainLineEdit->setCompleter(new QCompleter(cookieModel, domainLineEdit));

    connect(domainLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(blockButton, SIGNAL(clicked()), this, SLOT(block()));
    connect(allowButton, SIGNAL(clicked()), this, SLOT(allow()));
    connect(allowForSessionButton, SIGNAL(clicked()), this, SLOT(allowForSession()));

    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int height = fm.height() + fm.height() / 3;
    exceptionTable->verticalHeader()->setDefaultSectionSize(height);
    exceptionTable->verticalHeader()->setMinimumSectionSize(-1);
    for (int i = 0; i < m_exceptionsModel->columnCount(); ++i) {
        int header = exceptionTable->horizontalHeader()->sectionSizeHint(i);
        switch (i) {
        case 0:
            header = fm.horizontalAdvance(QLatin1String("averagebiglonghost.domain.com"));
            break;
        case 1:
            header = fm.horizontalAdvance(QLatin1String("Allow For Session"));
            break;
        }
        int buffer = fm.horizontalAdvance(QLatin1String("xx"));
        header += buffer;
        exceptionTable->horizontalHeader()->resizeSection(i, header);
    }
}

void CookieExceptionsDialog::textChanged(const QString &text)
{
    bool enabled = !text.isEmpty();
    blockButton->setEnabled(enabled);
    allowButton->setEnabled(enabled);
    allowForSessionButton->setEnabled(enabled);
}

void CookieExceptionsDialog::setDomainName(const QString &domainName)
{
    domainLineEdit->setText(domainName);
}

void CookieExceptionsDialog::block()
{
    m_exceptionsModel->addRule(domainLineEdit->text(), CookieJar::Block);
}

void CookieExceptionsDialog::allow()
{
    m_exceptionsModel->addRule(domainLineEdit->text(), CookieJar::Allow);
}

void CookieExceptionsDialog::allowForSession()
{
    m_exceptionsModel->addRule(domainLineEdit->text(), CookieJar::AllowForSession);
}

void CookieExceptionsDialog::accept()
{
    m_cookieJar->setBlockedCookies(m_exceptionsModel->m_blockedCookies);
    m_cookieJar->setAllowedCookies(m_exceptionsModel->m_allowedCookies);
    m_cookieJar->setAllowForSessionCookies(m_exceptionsModel->m_sessionCookies);
    QDialog::accept();
}

