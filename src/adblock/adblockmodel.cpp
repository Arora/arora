/**
 * Copyright (c) 2009, Benjamin C. Meyer <ben@meyerhome.net>
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

#include "adblockmodel.h"

#include "adblockrule.h"
#include "adblocksubscription.h"
#include "adblockmanager.h"

AdBlockModel::AdBlockModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_manager(AdBlockManager::instance())
{
    connect(m_manager, SIGNAL(rulesChanged()), this, SLOT(rulesChanged()));
    connect(m_manager, SIGNAL(rulesGoingToChange()), this, SLOT(rulesGoingToChange()));
}

void AdBlockModel::rulesGoingToChange()
{
    beginResetModel();
}

void AdBlockModel::rulesChanged()
{
    endResetModel();
}

const AdBlockRule AdBlockModel::rule(const QModelIndex &index) const
{
    const AdBlockSubscription *parent = static_cast<AdBlockSubscription*>(index.internalPointer());
    Q_ASSERT(parent);
    return parent->allRules().at(index.row());
}

AdBlockSubscription *AdBlockModel::subscription(const QModelIndex &index) const
{
    const AdBlockSubscription *parent = static_cast<AdBlockSubscription*>(index.internalPointer());
    if (parent)
        return 0;
    int row = index.row();
    if (row < 0 || row >= m_manager->subscriptions().count())
        return 0;
    return m_manager->subscriptions().at(row);
}

QModelIndex AdBlockModel::index(AdBlockSubscription *subscription)
{
    int row = m_manager->subscriptions().indexOf(subscription);
    if (row < 0 || row >= m_manager->subscriptions().count())
        return QModelIndex();
    return createIndex(row, 0, (void*) 0);
}

QVariant AdBlockModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return tr("Rule");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant AdBlockModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()
        || index.model() != this
        || index.column() != 0)
        return QVariant();

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (index.parent().isValid()) {
            const AdBlockRule r = rule(index);
            return r.filter();
        } else {
            AdBlockSubscription *sub = subscription(index);
            if (sub)
                return sub->title();
        }
        break;
    case Qt::CheckStateRole:
        if (index.parent().isValid()) {
            const AdBlockRule r = rule(index);
            return r.isEnabled() ? Qt::Checked : Qt::Unchecked;
        } else {
            AdBlockSubscription *sub = subscription(index);
            if (sub)
                return sub->isEnabled() ? Qt::Checked : Qt::Unchecked;
        }
        break;
    default:
        break;
    }

    return QVariant();
}

int AdBlockModel::columnCount(const QModelIndex &parent) const
{
    return (parent.column() > 0) ? 0 : 1;
}

int AdBlockModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return m_manager->subscriptions().count();

    if (parent.internalPointer() != 0)
        return 0;

    const AdBlockSubscription *parentNode = subscription(parent);
    return parentNode ? parentNode->allRules().count() : 0;
}

QModelIndex AdBlockModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column, (void*)0);

    // get the parent node
    const AdBlockSubscription *parentNode = subscription(parent);
    return createIndex(row, column, (void*)parentNode);
}

QModelIndex AdBlockModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    AdBlockSubscription *parent = static_cast<AdBlockSubscription*>(index.internalPointer());
    if (!parent)
        return QModelIndex();

    int parentRow = m_manager->subscriptions().indexOf(parent);
    return createIndex(parentRow, 0, (void*)0);
}

Qt::ItemFlags AdBlockModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsSelectable;

    if (index.parent().isValid()) {
        flags |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
        const AdBlockSubscription *parentNode = subscription(index.parent());
        if (parentNode && parentNode->isEnabled())
            flags |= Qt::ItemIsEnabled;
    } else {
        flags |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }

    return flags;
}

bool AdBlockModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || count <= 0 || row + count > rowCount(parent))
        return false;

    if (!parent.isValid()) {
        disconnect(m_manager, SIGNAL(rulesChanged()), this, SLOT(rulesChanged()));
        beginRemoveRows(QModelIndex(), row, row + count - 1);
        for (int i = row + count - 1; i >= row; --i) {
            AdBlockManager *manager = AdBlockManager::instance();
            manager->removeSubscription(manager->subscriptions().at(i));
        }
        endRemoveRows();
        connect(m_manager, SIGNAL(rulesChanged()), this, SLOT(rulesChanged()));
        return true;
    } else {
        AdBlockSubscription *sub = subscription(parent);
        if (sub) {
            disconnect(m_manager, SIGNAL(rulesChanged()), this, SLOT(rulesChanged()));
            beginRemoveRows(parent, row, row + count - 1);
            QList<AdBlockRule> rules = sub->allRules();
            for (int i = row + count - 1; i >= row; --i)
                sub->removeRule(i);
            endRemoveRows();
            connect(m_manager, SIGNAL(rulesChanged()), this, SLOT(rulesChanged()));
            return true;
        }
    }

    return false;
}

bool AdBlockModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()
        || index.model() != this
        || index.column() != 0
        || (flags(index) & Qt::ItemIsEditable) == 0)
        return false;

    disconnect(m_manager, SIGNAL(rulesChanged()), this, SLOT(rulesChanged()));
    bool changed = false;
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (index.parent().isValid()) {
            AdBlockSubscription *sub = subscription(index.parent());
            if (sub) {
                AdBlockRule r = rule(index);
                r.setFilter(value.toString());
                sub->replaceRule(r, index.row());
                dataChanged(index, index);
                changed = true;
            }
        } else {
            AdBlockSubscription *sub = subscription(index);
            if (sub) {
                sub->setTitle(value.toString());
                dataChanged(index, index);
                changed = true;
            }
        }
        break;
    case Qt::CheckStateRole:
        if (index.parent().isValid()) {
            AdBlockSubscription *sub = subscription(index.parent());
            if (sub) {
                AdBlockRule r = rule(index);
                r.setEnabled(value == Qt::Checked);
                sub->replaceRule(r, index.row());
                dataChanged(index, index);
                changed = true;
            }
        } else {
            AdBlockSubscription *sub = subscription(index);
            if (sub) {
                sub->setEnabled(value == Qt::Checked);
                dataChanged(index, index);
                changed = true;
            }
        }
        break;
    default:
        break;
    }
    connect(m_manager, SIGNAL(rulesChanged()), this, SLOT(rulesChanged()));
    return changed;
}

bool AdBlockModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return true;
    if (parent.internalPointer() == 0)
        return true;
    return false;
}

