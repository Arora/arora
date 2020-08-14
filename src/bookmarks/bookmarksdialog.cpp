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

#include "bookmarksdialog.h"

#include "bookmarknode.h"
#include "bookmarksmanager.h"
#include "bookmarksmodel.h"
#include "browserapplication.h"
#include "treesortfilterproxymodel.h"

#include <qheaderview.h>
#include <qmenu.h>

BookmarksDialog::BookmarksDialog(QWidget *parent, BookmarksManager *manager)
    : QDialog(parent)
    , m_bookmarksManager(0)
    , m_bookmarksModel(0)
    , m_proxyModel(0)
{
    m_bookmarksManager = manager;
    if (!m_bookmarksManager)
        m_bookmarksManager = BrowserApplication::bookmarksManager();
    setupUi(this);

    tree->setUniformRowHeights(true);
    tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree->setTextElideMode(Qt::ElideMiddle);
    m_bookmarksModel = m_bookmarksManager->bookmarksModel();
    m_proxyModel = new TreeSortFilterProxyModel(this);
    m_proxyModel->setFilterKeyColumn(-1);
    connect(search, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterFixedString(QString)));
    connect(removeButton, SIGNAL(clicked()), tree, SLOT(removeSelected()));
    m_proxyModel->setSourceModel(m_bookmarksModel);
    tree->setModel(m_proxyModel);
    tree->setDragDropMode(QAbstractItemView::InternalMove);
    tree->setExpanded(m_proxyModel->index(0, 0), true);
    tree->setAlternatingRowColors(true);
    QFontMetrics fm(font());
    int header = fm.horizontalAdvance(QLatin1Char('m')) * 40;
    tree->header()->resizeSection(0, header);
    tree->header()->setStretchLastSection(true);
    connect(tree, SIGNAL(activated(const QModelIndex&)),
            this, SLOT(openBookmark()));
    tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tree, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(customContextMenuRequested(const QPoint &)));
    connect(addFolderButton, SIGNAL(clicked()),
            this, SLOT(newFolder()));
    expandNodes(m_bookmarksManager->bookmarks());
}

BookmarksDialog::~BookmarksDialog()
{
    if (saveExpandedNodes(tree->rootIndex()))
        m_bookmarksManager->changeExpanded();
}

bool BookmarksDialog::saveExpandedNodes(const QModelIndex &parent)
{
    bool changed = false;
    for (int i = 0; i < m_proxyModel->rowCount(parent); ++i) {
        QModelIndex child = m_proxyModel->index(i, 0, parent);
        QModelIndex sourceIndex = m_proxyModel->mapToSource(child);
        BookmarkNode *childNode = m_bookmarksModel->node(sourceIndex);
        bool wasExpanded = childNode->expanded;
        if (tree->isExpanded(child)) {
            childNode->expanded = true;
            changed |= saveExpandedNodes(child);
        } else {
            childNode->expanded = false;
        }
        changed |= (wasExpanded != childNode->expanded);
    }
    return changed;
}

void BookmarksDialog::expandNodes(BookmarkNode *node)
{
    for (int i = 0; i < node->children().count(); ++i) {
        BookmarkNode *childNode = node->children()[i];
        if (childNode->expanded) {
            QModelIndex idx = m_bookmarksModel->index(childNode);
            idx = m_proxyModel->mapFromSource(idx);
            tree->setExpanded(idx, true);
            expandNodes(childNode);
        }
    }
}

void BookmarksDialog::customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    QModelIndex index = tree->indexAt(pos);
    index = index.sibling(index.row(), 0);
    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    const BookmarkNode *node = m_bookmarksModel->node(sourceIndex);
    if (index.isValid() && node->type() != BookmarkNode::Folder) {
        menu.addAction(tr("Open"), this, SLOT(openInCurrentTab()));
        menu.addAction(tr("Open in New Tab"), this, SLOT(openInNewTab()));
        menu.addSeparator();
    }
    menu.addSeparator();
    QAction *renameAction = menu.addAction(tr("Edit Name"), this, SLOT(editName()));
    renameAction->setEnabled(index.flags() & Qt::ItemIsEditable);
    if (index.isValid() && node->type() != BookmarkNode::Folder) {
        menu.addAction(tr("Edit Address"), this, SLOT(editAddress()));
    }
    menu.addSeparator();
    QAction *deleteAction = menu.addAction(tr("Delete"), tree, SLOT(removeSelected()));
    deleteAction->setEnabled(index.flags() & Qt::ItemIsDragEnabled);
    menu.exec(QCursor::pos());
}

void BookmarksDialog::openBookmark(TabWidget::OpenUrlIn tab)
{
    QModelIndex index = tree->currentIndex();
    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    const BookmarkNode *node = m_bookmarksModel->node(sourceIndex);
    if (!index.parent().isValid() || !node || node->type() == BookmarkNode::Folder)
        return;
    emit openUrl(
          index.sibling(index.row(), 1).data(BookmarksModel::UrlRole).toUrl(),
          tab,
          index.sibling(index.row(), 0).data(Qt::DisplayRole).toString());
}

void BookmarksDialog::openBookmark()
{
    BrowserApplication::instance()->setEventMouseButtons(qApp->mouseButtons());
    BrowserApplication::instance()->setEventKeyboardModifiers(qApp->keyboardModifiers());
    openBookmark(TabWidget::UserOrCurrent);
}

void BookmarksDialog::openInCurrentTab()
{
    openBookmark(TabWidget::CurrentTab);
}

void BookmarksDialog::openInNewTab()
{
    openBookmark(TabWidget::NewTab);
}

void BookmarksDialog::editName()
{
    QModelIndex idx = tree->currentIndex();
    idx = idx.sibling(idx.row(), 0);
    tree->edit(idx);
}

void BookmarksDialog::editAddress()
{
    QModelIndex idx = tree->currentIndex();
    idx = idx.sibling(idx.row(), 1);
    tree->edit(idx);
}

void BookmarksDialog::newFolder()
{
    QModelIndex currentIndex = tree->currentIndex();
    QModelIndex idx = currentIndex;
    QModelIndex sourceIndex = m_proxyModel->mapToSource(idx);
    const BookmarkNode *sourceNode = m_bookmarksModel->node(sourceIndex);
    int row = -1; // default: append new folder as last item in selected folder
    if (sourceNode && sourceNode->type() != BookmarkNode::Folder) {
        // if selected item is not a folder, add new folder to parent folder,
        // but direcly below the selected item
        idx = idx.parent();
        row = currentIndex.row() + 1;
    }
    if (!idx.isValid())
        idx = m_proxyModel->index(1, 0); // Select Bookmarks menu as default
    idx = m_proxyModel->mapToSource(idx);
    BookmarkNode *parent = m_bookmarksManager->bookmarksModel()->node(idx);
    BookmarkNode *node = new BookmarkNode(BookmarkNode::Folder);
    node->title = tr("New Folder");
    m_bookmarksManager->addBookmark(parent, node, row);
}

