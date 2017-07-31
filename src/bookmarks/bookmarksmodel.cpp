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

#include "bookmarksmodel.h"

#include "addbookmarkdialog.h"
#include "bookmarknode.h"
#include "bookmarksmanager.h"
#include "browserapplication.h"
#include "xbelreader.h"
#include "xbelwriter.h"

#include <qbuffer.h>
#include <qevent.h>
#include <QMimeData>


BookmarksModel::BookmarksModel(BookmarksManager *bookmarkManager, QObject *parent)
    : QAbstractItemModel(parent)
    , m_endMacro(false)
    , m_bookmarksManager(bookmarkManager)
{
    connect(bookmarkManager, SIGNAL(entryAdded(BookmarkNode *)),
            this, SLOT(entryAdded(BookmarkNode *)));
    connect(bookmarkManager, SIGNAL(entryRemoved(BookmarkNode *, int, BookmarkNode *)),
            this, SLOT(entryRemoved(BookmarkNode *, int, BookmarkNode *)));
    connect(bookmarkManager, SIGNAL(entryChanged(BookmarkNode *)),
            this, SLOT(entryChanged(BookmarkNode *)));
}

QModelIndex BookmarksModel::index(BookmarkNode *node) const
{
    BookmarkNode *parent = node->parent();
    if (!parent)
        return QModelIndex();
    return createIndex(parent->children().indexOf(node), 0, node);
}

void BookmarksModel::entryAdded(BookmarkNode *item)
{
    Q_ASSERT(item && item->parent());
    int row = item->parent()->children().indexOf(item);
    BookmarkNode *parent = item->parent();
    // item was already added so remove beore beginInsertRows is called
    parent->remove(item);
    beginInsertRows(index(parent), row, row);
    parent->add(item, row);
    endInsertRows();
}

void BookmarksModel::entryRemoved(BookmarkNode *parent, int row, BookmarkNode *item)
{
    // item was already removed, re-add so beginRemoveRows works
    parent->add(item, row);
    beginRemoveRows(index(parent), row, row);
    parent->remove(item);
    endRemoveRows();
}

void BookmarksModel::entryChanged(BookmarkNode *item)
{
    QModelIndex idx = index(item);
    emit dataChanged(idx, idx);
}

bool BookmarksModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || count <= 0 || row + count > rowCount(parent))
        return false;

    BookmarkNode *bookmarkNode = node(parent);
    for (int i = row + count - 1; i >= row; --i) {
        BookmarkNode *node = bookmarkNode->children().at(i);
        if (node == m_bookmarksManager->menu()
            || node == m_bookmarksManager->toolbar())
            continue;

        m_bookmarksManager->removeBookmark(node);
    }
    if (m_endMacro) {
        m_bookmarksManager->undoRedoStack()->endMacro();
        m_endMacro = false;
    }
    return true;
}

QVariant BookmarksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return tr("Title");
        case 1: return tr("Address");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant BookmarksModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this)
        return QVariant();

    const BookmarkNode *bookmarkNode = node(index);
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (bookmarkNode->type() == BookmarkNode::Separator) {
            switch (index.column()) {
            case 0: return QString(50, 0xB7);
            case 1: return QString();
            }
        }

        switch (index.column()) {
        case 0: return bookmarkNode->title;
        case 1: return bookmarkNode->url;
        }
        break;
    case BookmarksModel::UrlRole:
        return QUrl(bookmarkNode->url);
        break;
    case BookmarksModel::UrlStringRole:
        return bookmarkNode->url;
        break;
    case BookmarksModel::TypeRole:
        return bookmarkNode->type();
        break;
    case BookmarksModel::SeparatorRole:
        return (bookmarkNode->type() == BookmarkNode::Separator);
        break;
    case Qt::DecorationRole:
        if (index.column() == 0) {
            if (bookmarkNode->type() == BookmarkNode::Folder)
                return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
            return BrowserApplication::instance()->icon(bookmarkNode->url);
        }
    }

    return QVariant();
}

int BookmarksModel::columnCount(const QModelIndex &parent) const
{
    return (parent.column() > 0) ? 0 : 2;
}

int BookmarksModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return m_bookmarksManager->bookmarks()->children().count();

    const BookmarkNode *item = static_cast<BookmarkNode*>(parent.internalPointer());
    return item->children().count();
}

QModelIndex BookmarksModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
        return QModelIndex();

    // get the parent node
    BookmarkNode *parentNode = node(parent);
    return createIndex(row, column, parentNode->children().at(row));
}

QModelIndex BookmarksModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    BookmarkNode *itemNode = node(index);
    BookmarkNode *parentNode = (itemNode ? itemNode->parent() : 0);
    if (!parentNode || parentNode == m_bookmarksManager->bookmarks())
        return QModelIndex();

    // get the parent's row
    BookmarkNode *grandParentNode = parentNode->parent();
    int parentRow = grandParentNode->children().indexOf(parentNode);
    Q_ASSERT(parentRow >= 0);
    return createIndex(parentRow, 0, parentNode);
}

bool BookmarksModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return true;
    const BookmarkNode *parentNode = node(parent);
    return (parentNode->type() == BookmarkNode::Folder);
}

Qt::ItemFlags BookmarksModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    BookmarkNode *node = this->node(index);
    BookmarkNode::Type type = node->type();
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    if (hasChildren(index))
        flags |= Qt::ItemIsDropEnabled;

    if (node == m_bookmarksManager->menu()
        || node == m_bookmarksManager->toolbar())
        return flags;

    flags |= Qt::ItemIsDragEnabled;

    if ((index.column() == 0 && type != BookmarkNode::Separator)
        || (index.column() == 1 && type == BookmarkNode::Bookmark))
        flags |= Qt::ItemIsEditable;

    return flags;
}

Qt::DropActions BookmarksModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

#define MIMETYPE QLatin1String("application/bookmarks.xbel")

QStringList BookmarksModel::mimeTypes() const
{
    QStringList types;
    types << MIMETYPE;
    types << QLatin1String("text/uri-list");
    return types;
}

QMimeData *BookmarksModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    QList<QUrl> urls;
    foreach (const QModelIndex &index, indexes) {
        if (index.column() != 0 || !index.isValid())
            continue;
        QByteArray encodedData;
        QBuffer buffer(&encodedData);
        buffer.open(QBuffer::ReadWrite);
        XbelWriter writer;
        const BookmarkNode *parentNode = node(index);
        writer.write(&buffer, parentNode);
        stream << encodedData;
        urls.append(index.data(BookmarksModel::UrlRole).toUrl());
    }
    mimeData->setData(MIMETYPE, data);
    mimeData->setUrls(urls);
    return mimeData;
}

bool BookmarksModel::dropMimeData(const QMimeData *data,
     Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (column > 0)
        return false;

    BookmarkNode *parentNode = node(parent);

    if (!data->hasFormat(MIMETYPE)) {
        if (!data->hasUrls())
            return false;

        BookmarkNode *node = new BookmarkNode(BookmarkNode::Bookmark, parentNode);
        node->url = QString::fromUtf8(data->urls().at(0).toEncoded());

        if (data->hasText())
            node->title = data->text();
        else
            node->title = node->url;

        m_bookmarksManager->addBookmark(parentNode, node, row);
        return true;
    }

    QByteArray ba = data->data(MIMETYPE);
    QDataStream stream(&ba, QIODevice::ReadOnly);
    if (stream.atEnd())
        return false;

    QUndoStack *undoStack = m_bookmarksManager->undoRedoStack();
    undoStack->beginMacro(QLatin1String("Move Bookmarks"));

    while (!stream.atEnd()) {
        QByteArray encodedData;
        stream >> encodedData;
        QBuffer buffer(&encodedData);
        buffer.open(QBuffer::ReadOnly);

        XbelReader reader;
        BookmarkNode *rootNode = reader.read(&buffer);
        QList<BookmarkNode*> children = rootNode->children();
        for (int i = 0; i < children.count(); ++i) {
            BookmarkNode *bookmarkNode = children.at(i);
            rootNode->remove(bookmarkNode);
            row = qMax(0, row);
            m_bookmarksManager->addBookmark(parentNode, bookmarkNode, row);
            m_endMacro = true;
        }
        delete rootNode;
    }
    return true;
}

bool BookmarksModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || (flags(index) & Qt::ItemIsEditable) == 0)
        return false;

    BookmarkNode *item = node(index);

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (index.column() == 0) {
            m_bookmarksManager->setTitle(item, value.toString());
            break;
        }
        if (index.column() == 1) {
            m_bookmarksManager->setUrl(item, value.toString());
            break;
        }
        return false;
    case BookmarksModel::UrlRole:
        m_bookmarksManager->setUrl(item, value.toUrl().toString());
        break;
    case BookmarksModel::UrlStringRole:
        m_bookmarksManager->setUrl(item, value.toString());
        break;
    default:
        break;
        return false;
    }

    return true;
}

BookmarkNode *BookmarksModel::node(const QModelIndex &index) const
{
    BookmarkNode *itemNode = static_cast<BookmarkNode*>(index.internalPointer());
    if (!itemNode)
        return m_bookmarksManager->bookmarks();
    return itemNode;
}

