/*
 * Copyright 2008 Benjamin C. Meyer <ben@meyerhome.net>
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

#include "history.h"

#include "autosaver.h"
#ifndef NO_BROWSERAPPLICATION
#include "browserapplication.h"
#endif
#include "historymanager.h"
#include "treesortfilterproxymodel.h"

#include <qbuffer.h>
#include <qclipboard.h>
#include <qdesktopservices.h>
#include <qheaderview.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qsettings.h>
#include <qstyle.h>
#include <qtemporaryfile.h>
#include <qtextstream.h>
#include <qmessagebox.h>
#include <qmimedata.h>

#include <qwebhistoryinterface.h>
#include <qwebsettings.h>

#include <qdebug.h>

HistoryModel::HistoryModel(HistoryManager *history, QObject *parent)
    : QAbstractTableModel(parent)
    , m_history(history)
{
    Q_ASSERT(m_history);
    connect(m_history, SIGNAL(historyGoingToChange()), this, SLOT(historyGoingToChange()));
    connect(m_history, SIGNAL(historyReset()),
            this, SLOT(historyReset()));
    connect(m_history, SIGNAL(entryRemoved(const HistoryEntry &)),
            this, SLOT(historyReset()));

    connect(m_history, SIGNAL(entryAdded(const HistoryEntry &)),
            this, SLOT(entryAdded()));
    connect(m_history, SIGNAL(entryUpdated(int)),
            this, SLOT(entryUpdated(int)));
}

void HistoryModel::historyReset()
{
    endResetModel();
}

void HistoryModel::historyGoingToChange()
{
    beginResetModel();
}

void HistoryModel::entryAdded()
{
    beginInsertRows(QModelIndex(), 0, 0);
    endInsertRows();
}

void HistoryModel::entryUpdated(int offset)
{
    QModelIndex idx = index(offset, 0);
    emit dataChanged(idx, idx);
}

QVariant HistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal
        && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return tr("Title");
        case 1: return tr("Address");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    QList<HistoryEntry> lst = m_history->history();
    if (index.row() < 0 || index.row() >= lst.size())
        return QVariant();

    const HistoryEntry &item = lst.at(index.row());
    switch (role) {
    case DateTimeRole:
        return item.dateTime;
    case DateRole:
        return item.dateTime.date();
    case UrlRole:
        return QUrl(item.url);
    case UrlStringRole:
        return item.url;
    case TitleRole:
        return item.userTitle();
    case Qt::DisplayRole:
    case Qt::EditRole: {
        switch (index.column()) {
        case 0:
            return item.userTitle();
        case 1:
            return item.url;
        }
    }
#ifndef NO_BROWSERAPPLICATION
    case Qt::DecorationRole:
        if (index.column() == 0) {
            return BrowserApplication::instance()->icon(item.url);
        }
#endif
    }
    return QVariant();
}

int HistoryModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : 2;
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : m_history->history().count();
}

bool HistoryModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid())
        return false;
    int lastRow = row + count - 1;
    beginRemoveRows(parent, row, lastRow);
    QList<HistoryEntry> lst = m_history->history();
    for (int i = lastRow; i >= row; --i)
        lst.removeAt(i);
    disconnect(m_history, SIGNAL(historyReset()), this, SLOT(historyReset()));
    m_history->setHistory(lst);
    connect(m_history, SIGNAL(historyReset()), this, SLOT(historyReset()));
    endRemoveRows();
    return true;
}

#define MOVEDROWS 15

/*
    Maps the first bunch of items of the source model to the root
*/
HistoryMenuModel::HistoryMenuModel(HistoryTreeModel *sourceModel, QObject *parent)
    : QAbstractProxyModel(parent)
    , m_treeModel(sourceModel)
{
    setSourceModel(sourceModel);
}

int HistoryMenuModel::bumpedRows() const
{
    QModelIndex first = m_treeModel->index(0, 0);
    if (!first.isValid())
        return 0;
    return qMin(m_treeModel->rowCount(first), MOVEDROWS);
}

int HistoryMenuModel::columnCount(const QModelIndex &parent) const
{
    return m_treeModel->columnCount(mapToSource(parent));
}

int HistoryMenuModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid()) {
        int folders = sourceModel()->rowCount();
        int bumpedItems = bumpedRows();
        if (bumpedItems <= MOVEDROWS
            && bumpedItems == sourceModel()->rowCount(sourceModel()->index(0, 0)))
            --folders;
        return bumpedItems + folders;
    }

    if (parent.internalId() == -1) {
        if (parent.row() < bumpedRows())
            return 0;
    }

    QModelIndex idx = mapToSource(parent);
    int defaultCount = sourceModel()->rowCount(idx);
    if (idx == sourceModel()->index(0, 0))
        return defaultCount - bumpedRows();
    return defaultCount;
}

QModelIndex HistoryMenuModel::buddy(const QModelIndex &index) const
{
    return index;
}

QModelIndex HistoryMenuModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    // currently not used or autotested
    Q_ASSERT(false);
    int sr = m_treeModel->mapToSource(sourceIndex).row();
    return createIndex(sourceIndex.row(), sourceIndex.column(), sr);
}

QModelIndex HistoryMenuModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid())
        return QModelIndex();

    if (proxyIndex.internalId() == -1) {
        int bumpedItems = bumpedRows();
        if (proxyIndex.row() < bumpedItems)
            return m_treeModel->index(proxyIndex.row(), proxyIndex.column(), m_treeModel->index(0, 0));
        if (bumpedItems <= MOVEDROWS && bumpedItems == sourceModel()->rowCount(m_treeModel->index(0, 0)))
            --bumpedItems;
        return m_treeModel->index(proxyIndex.row() - bumpedItems, proxyIndex.column());
    }

    QModelIndex historyIndex = m_treeModel->sourceModel()->index(proxyIndex.internalId(), proxyIndex.column());
    QModelIndex treeIndex = m_treeModel->mapFromSource(historyIndex);
    return treeIndex;
}

QModelIndex HistoryMenuModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0
        || column < 0 || column >= columnCount(parent)
        || parent.column() > 0)
        return QModelIndex();
    if (!parent.isValid())
        return createIndex(row, column, -1);

    QModelIndex treeIndexParent = mapToSource(parent);

    int bumpedItems = 0;
    if (treeIndexParent == m_treeModel->index(0, 0))
        bumpedItems = bumpedRows();
    QModelIndex treeIndex = m_treeModel->index(row + bumpedItems, column, treeIndexParent);
    QModelIndex historyIndex = m_treeModel->mapToSource(treeIndex);
    int historyRow = historyIndex.row();
    if (historyRow == -1)
        historyRow = treeIndex.row();
    return createIndex(row, column, historyRow);
}

QModelIndex HistoryMenuModel::parent(const QModelIndex &index) const
{
    int offset = index.internalId();
    if (offset == -1 || !index.isValid())
        return QModelIndex();

    QModelIndex historyIndex = m_treeModel->sourceModel()->index(index.internalId(), 0);
    QModelIndex treeIndex = m_treeModel->mapFromSource(historyIndex);
    QModelIndex treeIndexParent = treeIndex.parent();

    int sr = m_treeModel->mapToSource(treeIndexParent).row();
    int bumpedItems = bumpedRows();
    if (bumpedItems <= MOVEDROWS && bumpedItems == sourceModel()->rowCount(sourceModel()->index(0, 0)))
        --bumpedItems;
    return createIndex(bumpedItems + treeIndexParent.row(), treeIndexParent.column(), sr);
}

QMimeData *HistoryMenuModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData;
    QList<QUrl> urls;
    foreach (const QModelIndex &idx, indexes) {
        QUrl url = idx.data(HistoryModel::UrlRole).toUrl();
        urls.append(url);
    }
    mimeData->setUrls(urls);
    return mimeData;
}

#ifndef NO_BROWSERAPPLICATION
HistoryMenu::HistoryMenu(QWidget *parent)
    : ModelMenu(parent)
    , m_history(0)
    , m_historyMenuModel(0)
{
    setMaxRows(7);
    connect(this, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(activated(const QModelIndex &)));
    setStatusBarTextRole(HistoryModel::UrlStringRole);
}

void HistoryMenu::activated(const QModelIndex &index)
{
    emit openUrl(index.data(HistoryModel::UrlRole).toUrl(),
                 index.data(HistoryModel::TitleRole).toString());
}

bool HistoryMenu::prePopulated()
{
    if (!m_history) {
        m_history = BrowserApplication::historyManager();
        m_historyMenuModel = new HistoryMenuModel(m_history->historyTreeModel(), this);
        setModel(m_historyMenuModel);
    }
    // initial actions
    for (int i = 0; i < m_initialActions.count(); ++i)
        addAction(m_initialActions.at(i));
    if (!m_initialActions.isEmpty())
        addSeparator();
    setFirstSeparator(m_historyMenuModel->bumpedRows());

    return false;
}

void HistoryMenu::postPopulated()
{
    if (m_history->history().count() > 0)
        addSeparator();

    QAction *showAllAction = new QAction(tr("Show All History"), this);
#if !defined(Q_WS_MAC)
    showAllAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
#endif
    connect(showAllAction, SIGNAL(triggered()), this, SLOT(showHistoryDialog()));
    addAction(showAllAction);

    QAction *clearAction = new QAction(tr("Clear History..."), this);
    connect(clearAction, SIGNAL(triggered()), this, SLOT(clearHistoryDialog()));
    addAction(clearAction);
}

void HistoryMenu::showHistoryDialog()
{
    HistoryDialog *dialog = new HistoryDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, SIGNAL(openUrl(const QUrl&, const QString &)),
            this, SIGNAL(openUrl(const QUrl&, const QString &)));
    dialog->show();
}

void HistoryMenu::clearHistoryDialog()
{
    if (m_history && QMessageBox::question(0, tr("Clear History"), tr("Do you want to clear the history?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        m_history->clear();
    }
}

void HistoryMenu::setInitialActions(QList<QAction*> actions)
{
    m_initialActions = actions;
    for (int i = 0; i < m_initialActions.count(); ++i)
        addAction(m_initialActions.at(i));
}

HistoryDialog::HistoryDialog(QWidget *parent, HistoryManager *setHistory) : QDialog(parent)
{
    HistoryManager *history = setHistory;
    if (!history)
        history = BrowserApplication::historyManager();
    setupUi(this);
    tree->setUniformRowHeights(true);
    tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree->setTextElideMode(Qt::ElideMiddle);
    QAbstractItemModel *model = history->historyTreeModel();
    TreeSortFilterProxyModel *proxyModel = new TreeSortFilterProxyModel(this);
    proxyModel->setSortRole(HistoryModel::DateTimeRole);
    proxyModel->setFilterKeyColumn(-1);
    connect(search, SIGNAL(textChanged(QString)),
            proxyModel, SLOT(setFilterFixedString(QString)));
    connect(removeButton, SIGNAL(clicked()), tree, SLOT(removeSelected()));
    connect(removeAllButton, SIGNAL(clicked()), history, SLOT(clear()));
    proxyModel->setSourceModel(model);
    tree->setModel(proxyModel);
    tree->setExpanded(proxyModel->index(0, 0), true);
    tree->setAlternatingRowColors(true);
    QFontMetrics fm(font());
    int header = fm.horizontalAdvance(QLatin1Char('m')) * 40;
    tree->header()->resizeSection(0, header);
    tree->header()->setStretchLastSection(true);
    connect(tree, SIGNAL(activated(const QModelIndex&)),
            this, SLOT(open()));
    tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tree, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(customContextMenuRequested(const QPoint &)));
}

void HistoryDialog::customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    QModelIndex index = tree->indexAt(pos);
    index = index.sibling(index.row(), 0);
    if (index.isValid() && !tree->model()->hasChildren(index)) {
        menu.addAction(tr("Open"), this, SLOT(open()));
        menu.addSeparator();
        menu.addAction(tr("Copy"), this, SLOT(copy()));
    }
    menu.addAction(tr("Delete"), tree, SLOT(removeSelected()));
    menu.exec(QCursor::pos());
}

void HistoryDialog::open()
{
    QModelIndex index = tree->currentIndex();
    if (!index.parent().isValid())
        return;
    BrowserApplication::instance()->setEventMouseButtons(qApp->mouseButtons());
    BrowserApplication::instance()->setEventKeyboardModifiers(qApp->keyboardModifiers());
    emit openUrl(index.data(HistoryModel::UrlRole).toUrl(),
                 index.data(HistoryModel::TitleRole).toString());
}

void HistoryDialog::copy()
{
    QModelIndex index = tree->currentIndex();
    if (!index.parent().isValid())
        return;
    QString url = index.data(HistoryModel::UrlStringRole).toString();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(url);
}
#endif
HistoryFilterModel::HistoryFilterModel(QAbstractItemModel *sourceModel, QObject *parent)
    : QAbstractProxyModel(parent)
    , m_loaded(false)
{
    m_frecencyTimer.setSingleShot(true);
    connect(&m_frecencyTimer, SIGNAL(timeout()),
            this, SLOT(refreshFrecencies()));

    setSourceModel(sourceModel);
    startFrecencyTimer();
}

void HistoryFilterModel::refreshFrecencies()
{
    recalculateFrecencies();
    startFrecencyTimer();
}

void HistoryFilterModel::startFrecencyTimer()
{
    // schedule us to recalculate the frecencies once per day, at 3:00 am (aka 03h00)
    QDateTime tomorrow(QDate::currentDate().addDays(1), QTime(3, 00));
    m_frecencyTimer.start(QDateTime::currentDateTime().secsTo(tomorrow)*1000);
}

int HistoryFilterModel::historyLocation(const QString &url) const
{
    load();
    if (!m_historyHash.contains(url))
        return 0;

    return sourceModel()->rowCount() - m_historyHash.value(url);
}

QVariant HistoryFilterModel::data(const QModelIndex &index, int role) const
{
    if (role == FrecencyRole && index.isValid()) {
        return m_filteredRows[index.row()].frecency;
    }

    return QAbstractProxyModel::data(index, role);
}

void HistoryFilterModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    if (sourceModel()) {
        disconnect(sourceModel(), SIGNAL(modelReset()), this, SLOT(sourceReset()));
        disconnect(sourceModel(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                   this, SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));
        disconnect(sourceModel(), SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                   this, SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
        disconnect(sourceModel(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                   this, SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
    }

    QAbstractProxyModel::setSourceModel(newSourceModel);

    if (sourceModel()) {
        m_loaded = false;
        connect(sourceModel(), SIGNAL(modelReset()), this, SLOT(sourceReset()));
        connect(sourceModel(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                this, SLOT(sourceDataChanged(const QModelIndex &, const QModelIndex &)));
        connect(sourceModel(), SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                this, SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
        connect(sourceModel(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                this, SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
    }
}

void HistoryFilterModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    emit dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight));
}

QVariant HistoryFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return sourceModel()->headerData(section, orientation, role);
}

void HistoryFilterModel::recalculateFrecencies()
{
    sourceReset();
}

void HistoryFilterModel::sourceReset()
{
    beginResetModel();
    m_loaded = false;
    endResetModel();
}

int HistoryFilterModel::rowCount(const QModelIndex &parent) const
{
    load();
    if (parent.isValid())
        return 0;
    return m_historyHash.count();
}

int HistoryFilterModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : 2;
}

QModelIndex HistoryFilterModel::mapToSource(const QModelIndex &proxyIndex) const
{
    load();
    int sourceRow = sourceModel()->rowCount() - proxyIndex.internalId();
    return sourceModel()->index(sourceRow, proxyIndex.column());
}

QModelIndex HistoryFilterModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    load();
    QString url = sourceIndex.data(HistoryModel::UrlStringRole).toString();
    if (!m_historyHash.contains(url))
        return QModelIndex();

    int sourceOffset = sourceModel()->rowCount() - sourceIndex.row();

    QList<HistoryData>::iterator pos = std::lower_bound(m_filteredRows.begin(),
        m_filteredRows.end(), HistoryData(sourceOffset, -1));

    if (pos == m_filteredRows.end())
        return QModelIndex();

    return createIndex(pos - m_filteredRows.begin(), sourceIndex.column(), sourceOffset);
}

QModelIndex HistoryFilterModel::index(int row, int column, const QModelIndex &parent) const
{
    load();
    if (row < 0 || row >= rowCount(parent)
        || column < 0 || column >= columnCount(parent))
        return QModelIndex();

    return createIndex(row, column, m_filteredRows[row].tailOffset);
}

QModelIndex HistoryFilterModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

void HistoryFilterModel::load() const
{
    if (m_loaded)
        return;
    m_filteredRows.clear();
    m_historyHash.clear();
    m_historyHash.reserve(sourceModel()->rowCount());
    m_scaleTime = QDateTime::currentDateTime();
    for (int i = 0; i < sourceModel()->rowCount(); ++i) {
        QModelIndex idx = sourceModel()->index(i, 0);
        QString url = idx.data(HistoryModel::UrlStringRole).toString();
        if (!m_historyHash.contains(url)) {
            int sourceOffset = sourceModel()->rowCount() - i;
            m_filteredRows.append(HistoryData(sourceOffset, frecencyScore(idx)));
            m_historyHash.insert(url, sourceOffset);
        } else {
            // we already know about this url: just increment its frecency score
            QList<HistoryData>::iterator pos = std::lower_bound(m_filteredRows.begin(),
                m_filteredRows.end(), HistoryData(m_historyHash[url], -1));
            Q_ASSERT(pos != m_filteredRows.end());
            pos->frecency += frecencyScore(idx);
        }
    }
    m_loaded = true;
}

void HistoryFilterModel::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(start == end && start == 0);
    Q_UNUSED(end);
    if (!m_loaded)
        return;
    QModelIndex idx = sourceModel()->index(start, 0, parent);
    QString url = idx.data(HistoryModel::UrlStringRole).toString();
    int currentFrecency = 0;
    if (m_historyHash.contains(url)) {
        QList<HistoryData>::iterator pos = std::lower_bound(m_filteredRows.begin(),
            m_filteredRows.end(), HistoryData(m_historyHash[url], -1));
        Q_ASSERT(pos != m_filteredRows.end());
        int realRow = pos - m_filteredRows.begin();
        currentFrecency = pos->frecency;
        beginRemoveRows(QModelIndex(), realRow, realRow);
        m_filteredRows.erase(pos);
        m_historyHash.remove(url);
        endRemoveRows();
    }
    beginInsertRows(QModelIndex(), 0, 0);
    m_filteredRows.insert(0, HistoryData(sourceModel()->rowCount(), frecencyScore(idx) + currentFrecency));
    m_historyHash.insert(url, sourceModel()->rowCount());
    endInsertRows();
}

void HistoryFilterModel::sourceRowsRemoved(const QModelIndex &, int start, int end)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    sourceReset();
}

/*
    Removing a continuous block of rows will remove filtered rows too as this is
    the users intention.
*/
bool HistoryFilterModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || count <= 0 || row + count > rowCount(parent) || parent.isValid())
        return false;
    beginResetModel();
    int lastRow = row + count - 1;
    disconnect(sourceModel(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
               this, SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
    beginRemoveRows(parent, row, lastRow);
    int start = sourceModel()->rowCount() - m_filteredRows[row].tailOffset;
    int end = sourceModel()->rowCount() - m_filteredRows[lastRow].tailOffset;
    sourceModel()->removeRows(start, end - start + 1);
    endRemoveRows();
    connect(sourceModel(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
    m_loaded = false;
    endResetModel();
    return true;
}

int HistoryFilterModel::frecencyScore(const QModelIndex &sourceIndex) const
{
    QDateTime loadTime = sourceModel()->data(sourceIndex, HistoryModel::DateTimeRole).toDateTime();
    int days = loadTime.daysTo(m_scaleTime);

    if (days <= 1) {
        return 100;
    } else if (days < 5) { // within the last 4 days
        return 90;
    } else if (days < 15) { // within the last two weeks
        return 70;
    } else if (days < 31) { // within the last month
        return 50;
    } else if (days < 91) { // within the last 3 months
        return 30;
    }

    return 10;
}

HistoryTreeModel::HistoryTreeModel(QAbstractItemModel *sourceModel, QObject *parent)
    : QAbstractProxyModel(parent)
    , removingDown(false)
{
    setSourceModel(sourceModel);
}

QVariant HistoryTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return sourceModel()->headerData(section, orientation, role);
}

QVariant HistoryTreeModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: {
        int start = index.internalId();
        if (start == 0) {
            int offset = sourceDateRow(index.row());
            if (index.column() == 0) {
                QModelIndex idx = sourceModel()->index(offset, 0);
                QDate date = idx.data(HistoryModel::DateRole).toDate();
                if (date == QDate::currentDate())
                    return tr("Earlier Today");
                return date.toString(QLatin1String("dddd, MMMM d, yyyy"));
            }
            if (index.column() == 1) {
                return tr("%n item(s)", "", rowCount(index.sibling(index.row(), 0)));
            }
        }
    }
    case Qt::DecorationRole: {
        if (index.column() == 0 && !index.parent().isValid())
            return QIcon(QLatin1String(":graphics/history.png"));
    }
    case HistoryModel::DateRole: {
        if (index.column() == 0 && index.internalId() == 0) {
            int offset = sourceDateRow(index.row());
            QModelIndex idx = sourceModel()->index(offset, 0);
            return idx.data(HistoryModel::DateRole);
        }
    }
    }

    return QAbstractProxyModel::data(index, role);
}

int HistoryTreeModel::columnCount(const QModelIndex &parent) const
{
    return sourceModel()->columnCount(mapToSource(parent));
}

int HistoryTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.internalId() != 0
        || parent.column() > 0
        || !sourceModel())
        return 0;

    // row count OF dates
    if (!parent.isValid()) {
        if (!m_sourceRowCache.isEmpty())
            return m_sourceRowCache.count();
        QDate currentDate;
        int rows = 0;
        int totalRows = sourceModel()->rowCount();

        for (int i = 0; i < totalRows; ++i) {
            QDate rowDate = sourceModel()->index(i, 0).data(HistoryModel::DateRole).toDate();
            if (rowDate != currentDate) {
                m_sourceRowCache.append(i);
                currentDate = rowDate;
                ++rows;
            }
        }
        Q_ASSERT(m_sourceRowCache.count() == rows);
        return rows;
    }

    // row count FOR a date
    int start = sourceDateRow(parent.row());
    int end = sourceDateRow(parent.row() + 1);
    return (end - start);
}

// Translate the top level date row into the offset where that date starts
int HistoryTreeModel::sourceDateRow(int row) const
{
    if (row <= 0)
        return 0;

    if (m_sourceRowCache.isEmpty())
        rowCount(QModelIndex());

    if (row >= m_sourceRowCache.count()) {
        if (!sourceModel())
            return 0;
        return sourceModel()->rowCount();
    }
    return m_sourceRowCache.at(row);
}

QModelIndex HistoryTreeModel::mapToSource(const QModelIndex &proxyIndex) const
{
    int offset = proxyIndex.internalId();
    if (offset == 0)
        return QModelIndex();
    int startDateRow = sourceDateRow(offset - 1);
    return sourceModel()->index(startDateRow + proxyIndex.row(), proxyIndex.column());
}

QModelIndex HistoryTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0
        || column < 0 || column >= columnCount(parent)
        || parent.column() > 0)
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column, (void*) 0);
    return createIndex(row, column, parent.row() + 1);
}

QModelIndex HistoryTreeModel::parent(const QModelIndex &index) const
{
    int offset = index.internalId();
    if (offset == 0 || !index.isValid())
        return QModelIndex();
    return createIndex(offset - 1, 0, (void*) 0);
}

bool HistoryTreeModel::hasChildren(const QModelIndex &parent) const
{
    QModelIndex grandparent = parent.parent();
    if (!grandparent.isValid())
        return true;
    return false;
}

Qt::ItemFlags HistoryTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
}

void HistoryTreeModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    beginResetModel();

    if (sourceModel()) {
        disconnect(sourceModel(), SIGNAL(modelReset()), this, SLOT(sourceReset()));
        disconnect(sourceModel(), SIGNAL(layoutChanged()), this, SLOT(sourceReset()));
        disconnect(sourceModel(), SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                   this, SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
        disconnect(sourceModel(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                   this, SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
    }

    QAbstractProxyModel::setSourceModel(newSourceModel);

    if (newSourceModel) {
        connect(sourceModel(), SIGNAL(modelReset()), this, SLOT(sourceReset()));
        connect(sourceModel(), SIGNAL(layoutChanged()), this, SLOT(sourceReset()));
        connect(sourceModel(), SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                this, SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
        connect(sourceModel(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                this, SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
    }

    endResetModel();
}

void HistoryTreeModel::sourceReset()
{
    beginResetModel();
    m_sourceRowCache.clear();
    endResetModel();
}

void HistoryTreeModel::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent); // Avoid warnings when compiling release
    Q_ASSERT(!parent.isValid());
    if (start != 0 || start != end) {
        beginResetModel();
        m_sourceRowCache.clear();
        endResetModel();
        return;
    }

    m_sourceRowCache.clear();
    QModelIndex treeIndex = mapFromSource(sourceModel()->index(start, 0));
    QModelIndex treeParent = treeIndex.parent();
    if (rowCount(treeParent) == 1) {
        beginInsertRows(QModelIndex(), 0, 0);
        endInsertRows();
    } else {
        beginInsertRows(treeParent, treeIndex.row(), treeIndex.row());
        endInsertRows();
    }
}

QModelIndex HistoryTreeModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid())
        return QModelIndex();

    if (m_sourceRowCache.isEmpty())
        rowCount(QModelIndex());

    QList<int>::iterator it;
    it = std::lower_bound(m_sourceRowCache.begin(), m_sourceRowCache.end(), sourceIndex.row());
    if (*it != sourceIndex.row())
        --it;
    int dateRow = qMax(0, it - m_sourceRowCache.begin());
    int row = sourceIndex.row() - m_sourceRowCache.at(dateRow);
    return createIndex(row, sourceIndex.column(), dateRow + 1);
}

bool HistoryTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || count <= 0 || row + count > rowCount(parent))
        return false;

    removingDown = true;
    if (parent.isValid() && rowCount(parent) == count - row)
        beginRemoveRows(QModelIndex(), parent.row(), parent.row());
    else
        beginRemoveRows(parent, row, row + count - 1);
    if (parent.isValid()) {
        // removing pages
        int offset = sourceDateRow(parent.row());
        return sourceModel()->removeRows(offset + row, count);
    } else {
        // removing whole dates
        for (int i = row + count - 1; i >= row; --i) {
            QModelIndex dateParent = index(i, 0);
            int offset = sourceDateRow(dateParent.row());
            if (!sourceModel()->removeRows(offset, rowCount(dateParent)))
                return false;
        }
    }
    return true;
}

void HistoryTreeModel::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    if (!removingDown) {
        beginResetModel();
        m_sourceRowCache.clear();
        endResetModel();
        return;
    }
    Q_UNUSED(parent); // Avoid warnings when compiling release
    Q_ASSERT(!parent.isValid());
    if (!m_sourceRowCache.isEmpty())
    for (int i = end; i >= start;) {
        QList<int>::iterator it;
        it = std::lower_bound(m_sourceRowCache.begin(), m_sourceRowCache.end(), i);
        if (*it != i)
            --it;
        int row = qMax(0, it - m_sourceRowCache.begin());
        int offset = m_sourceRowCache[row];
        QModelIndex dateParent = index(row, 0);
        // If we can remove all the rows in the date do that and skip over them
        int rc = rowCount(dateParent);
        if (i - rc + 1 == offset && start <= i - rc + 1) {
            m_sourceRowCache.removeAt(row);
            i -= rc + 1;
        } else {
            ++row;
            --i;
        }
        for (int j = row; j < m_sourceRowCache.count(); ++j)
            --m_sourceRowCache[j];
    }
    if (removingDown) {
        endRemoveRows();
        removingDown = false;
    }
}

