/*
 * Copyright 2009 Christian Franke <cfchris6@ts2server.com>
 * Copyright 2009 Jakub Wieczorek <faw217@gmail.com>
 * Copyright 2009 Christopher Eby <kreed@kreed.org>
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

#include "opensearchenginemodel.h"

#include "browserapplication.h"
#include "opensearchengine.h"
#include "opensearchmanager.h"

#include <qimage.h>
#include <qicon.h>

OpenSearchEngineModel::OpenSearchEngineModel(OpenSearchManager *manager, QObject *parent)
    : QAbstractTableModel(parent)
    , m_manager(manager)
{
    connect(manager, SIGNAL(changed()),
            this, SLOT(enginesChanged()));
}

bool OpenSearchEngineModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid())
        return false;

    if (count <= 0)
        return false;

    if (rowCount() <= 1)
        return false;

    int lastRow = row + count - 1;

    beginRemoveRows(parent, row, lastRow);

    QStringList nameList = m_manager->allEnginesNames();
    for (int i = row; i <= lastRow; ++i)
        m_manager->removeEngine(nameList.at(i));

    // removeEngine emits changed
    //endRemoveRows();

    return true;
}

int OpenSearchEngineModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : m_manager->enginesCount();
}

int OpenSearchEngineModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

Qt::ItemFlags OpenSearchEngineModel::flags(const QModelIndex &index) const
{
    switch (index.column()) {
    case 1:
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    default:
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
}

QVariant OpenSearchEngineModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= m_manager->enginesCount() || index.row() < 0)
        return QVariant();

    OpenSearchEngine *engine = m_manager->engine(m_manager->allEnginesNames().at(index.row()));

    if (!engine)
        return QVariant();

    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::DisplayRole:
            return engine->name();
        break;
        case Qt::DecorationRole: {
            QImage image = engine->image();
            if (image.isNull())
                return BrowserApplication::icon(engine->imageUrl());
            return image;
        break;
        }
        case Qt::ToolTipRole:
            QString description = tr("<strong>Description:</strong> %1").arg(engine->description());

            if (engine->providesSuggestions()) {
                description += QLatin1String("<br />");
                description += tr("<strong>Provides contextual suggestions</strong>");
            }

            return description;
        break;
        }
        break;

    case 1:
        switch (role) {
        case Qt::EditRole:
        case Qt::DisplayRole:
            return QStringList(m_manager->keywordsForEngine(engine)).join(QLatin1String(","));
        case Qt::ToolTipRole:
            return tr("Comma-separated list of keywords that may be entered in the location bar"
                      "followed by search terms to search with this engine");
        }
        break;
    }

    return QVariant();
}

bool OpenSearchEngineModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.column() != 1)
        return false;

    if (index.row() >= rowCount() || index.row() < 0)
        return false;

    if (role != Qt::EditRole)
        return false;

    QString engineName = m_manager->allEnginesNames().at(index.row());
    QStringList keywords = value.toString().split(QRegExp(QLatin1String("[ ,]+")), QString::SkipEmptyParts);

    m_manager->setKeywordsForEngine(m_manager->engine(engineName), keywords);

    return true;
}

QVariant OpenSearchEngineModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
    case 0:
        return tr("Name");
    case 1:
        return tr("Keywords");
    }

    return QVariant();
}

void OpenSearchEngineModel::enginesChanged()
{
    beginResetModel();
    endResetModel();
}

