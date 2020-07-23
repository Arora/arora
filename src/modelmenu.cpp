/*
 * Copyright 2008 Benjamin C. Meyer <ben@meyerhome.net>
 * Copyright 2009 Jakub Wieczorek <faw217@gmail.com>
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
** Copyright (C) 2008-2008 Trolltech ASA. All rights reserved.
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

#include "modelmenu.h"

// option to remove the browserapplication for arora-placesimport
#ifndef NO_BROWSERAPPLICATION
#include "browserapplication.h"
#endif

#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qevent.h>
#include <QWidget>
#include <QDrag>
#include <QMimeData>

#include <qdebug.h>

ModelMenu::ModelMenu(QWidget *parent)
    : QMenu(parent)
    , m_maxRows(-1)
    , m_firstSeparator(-1)
    , m_maxWidth(-1)
    , m_statusBarTextRole(0)
    , m_separatorRole(0)
    , m_model(0)
{
    setAcceptDrops(true);

    connect(this, SIGNAL(aboutToShow()), this, SLOT(aboutToShow()));
    connect(this, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
}

bool ModelMenu::prePopulated()
{
    return false;
}

void ModelMenu::postPopulated()
{
}

void ModelMenu::setModel(QAbstractItemModel *model)
{
    m_model = model;
}

QAbstractItemModel *ModelMenu::model() const
{
    return m_model;
}

void ModelMenu::setMaxRows(int max)
{
    m_maxRows = max;
}

int ModelMenu::maxRows() const
{
    return m_maxRows;
}

void ModelMenu::setFirstSeparator(int offset)
{
    m_firstSeparator = offset;
}

int ModelMenu::firstSeparator() const
{
    return m_firstSeparator;
}

void ModelMenu::setRootIndex(const QModelIndex &index)
{
    m_root = index;
}

QModelIndex ModelMenu::rootIndex() const
{
    return m_root;
}

void ModelMenu::setStatusBarTextRole(int role)
{
    m_statusBarTextRole = role;
}

int ModelMenu::statusBarTextRole() const
{
    return m_statusBarTextRole;
}

void ModelMenu::setSeparatorRole(int role)
{
    m_separatorRole = role;
}

int ModelMenu::separatorRole() const
{
    return m_separatorRole;
}

Q_DECLARE_METATYPE(QModelIndex)
void ModelMenu::aboutToShow()
{
    clear();

    if (prePopulated())
        addSeparator();
    int max = m_maxRows;
    if (max != -1)
        max += m_firstSeparator;
    createMenu(m_root, max, this, this);
    postPopulated();
}

ModelMenu *ModelMenu::createBaseMenu()
{
    return new ModelMenu(this);
}

void ModelMenu::createMenu(const QModelIndex &parent, int max, QMenu *parentMenu, QMenu *menu)
{
    if (!menu) {
        QVariant v;
        v.setValue(parent);

        QString title = parent.data().toString();
        ModelMenu *modelMenu = createBaseMenu();
        // triggered goes all the way up the menu structure
        disconnect(modelMenu, SIGNAL(triggered(QAction*)),
                   modelMenu, SLOT(actionTriggered(QAction*)));
        modelMenu->setTitle(title);
        QIcon icon = qvariant_cast<QIcon>(parent.data(Qt::DecorationRole));
        modelMenu->setIcon(icon);
        parentMenu->addMenu(modelMenu)->setData(v);
        modelMenu->setRootIndex(parent);
        modelMenu->setModel(m_model);
        return;
    }

    if (!m_model)
        return;

    int end = m_model->rowCount(parent);
    if (max != -1)
        end = qMin(max, end);


    for (int i = 0; i < end; ++i) {
        QModelIndex idx = m_model->index(i, 0, parent);
        if (m_model->hasChildren(idx)) {
            createMenu(idx, -1, menu);
        } else {
            if (m_separatorRole != 0
                && idx.data(m_separatorRole).toBool())
                addSeparator();
            else
                menu->addAction(makeAction(idx));
        }
        if (menu == this && i == m_firstSeparator - 1)
            addSeparator();
    }
}

QAction *ModelMenu::makeAction(const QModelIndex &index)
{
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    QAction *action = makeAction(icon, index.data().toString(), this);
    action->setStatusTip(index.data(m_statusBarTextRole).toString());

    QVariant v;
    v.setValue(index);
    action->setData(v);
    return action;
}

QAction *ModelMenu::makeAction(const QIcon &icon, const QString &text, QObject *parent)
{
    QFontMetrics fm(font());
    if (-1 == m_maxWidth)
        m_maxWidth = fm.horizontalAdvance(QLatin1Char('m')) * 30;
    QString smallText = fm.elidedText(text, Qt::ElideMiddle, m_maxWidth);
    return new QAction(icon, smallText, parent);
}

void ModelMenu::actionTriggered(QAction *action)
{
    QModelIndex idx = index(action);
    if (idx.isValid())
        emit activated(idx);
}

QModelIndex ModelMenu::index(QAction *action)
{
    if (!action)
        return QModelIndex();
    QVariant variant = action->data();
    if (!variant.canConvert<QModelIndex>())
        return QModelIndex();

    return qvariant_cast<QModelIndex>(variant);
}

void ModelMenu::dragEnterEvent(QDragEnterEvent *event)
{
    if (!m_model) {
        QMenu::dragEnterEvent(event);
        return;
    }

    QStringList mimeTypes = m_model->mimeTypes();
    foreach (const QString &mimeType, mimeTypes) {
        if (event->mimeData()->hasFormat(mimeType))
            event->acceptProposedAction();
    }

    QMenu::dragEnterEvent(event);
}

void ModelMenu::dropEvent(QDropEvent *event)
{
    if (!m_model) {
        QMenu::dropEvent(event);
        return;
    }

    int row;
    QAction *action = actionAt(mapFromGlobal(QCursor::pos()));
    QModelIndex index;
    QModelIndex parentIndex = m_root;
    if (!action) {
        row = m_model->rowCount(m_root);
    } else {
        index = this->index(action);
        Q_ASSERT(index.isValid());
        row = index.row();

        if (m_model->hasChildren(index))
            parentIndex = index;
    }

    event->acceptProposedAction();
    m_model->dropMimeData(event->mimeData(), event->dropAction(), row, 0, parentIndex);
    QMenu::dropEvent(event);
}

void ModelMenu::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragStartPos = event->pos();
    QMenu::mousePressEvent(event);
}

#ifndef NO_BROWSERAPPLICATION
void ModelMenu::mouseReleaseEvent(QMouseEvent *event)
{
    BrowserApplication::instance()->setEventMouseButtons(event->button());
    BrowserApplication::instance()->setEventKeyboardModifiers(event->modifiers());
    QMenu::mouseReleaseEvent(event);
}
#endif

void ModelMenu::mouseMoveEvent(QMouseEvent *event)
{
    int manhattanLength = (event->pos() - m_dragStartPos).manhattanLength();

    if (manhattanLength <= QApplication::startDragDistance()) {
        QMenu::mouseMoveEvent(event);
        return;
    }

    if (!(event->buttons() & Qt::LeftButton)) {
        QMenu::mouseMoveEvent(event);
        return;
    }

    QAction *action = actionAt(m_dragStartPos);
    QModelIndex idx = index(action);

    if (!idx.isValid()) {
        QMenu::mouseMoveEvent(event);
        return;
    }

    QDrag *drag = new QDrag(this);
    drag->setMimeData(m_model->mimeData((QModelIndexList() << idx)));
    QRect actionRect = actionGeometry(action);
    drag->setPixmap(this->grab(actionRect));

    if (drag->exec() == Qt::MoveAction) {
        m_model->removeRow(idx.row(), m_root);

        QWidget *target = static_cast<QWidget*>(drag->target());
        if (!this->isAncestorOf(target))
            close();
        else
            aboutToShow();
    }
}

