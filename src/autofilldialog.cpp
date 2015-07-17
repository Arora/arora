/**
 * Copyright (c) 2009, Benjamin C. Meyer  <ben@meyerhome.net>
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

#include "autofilldialog.h"

#include "autofillmanager.h"
#include "browserapplication.h"

#include <qdebug.h>

AutoFillModel::AutoFillModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    AutoFillManager *manager = BrowserApplication::instance()->autoFillManager();
    Q_ASSERT(manager);
    connect(manager, SIGNAL(autoFillChanged()), this, SLOT(autoFillChanged()));
    autoFillChanged();
}

void AutoFillModel::autoFillChanged()
{
    AutoFillManager *manager = BrowserApplication::instance()->autoFillManager();
    m_forms = manager->forms();
    beginResetModel();
    endResetModel();
}

QVariant AutoFillModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal
        && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return tr("WebSite");
        case 1: return tr("User Name");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant AutoFillModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_forms.size())
        return QVariant();

    switch (role) {
    case Qt::UserRole:
        return m_forms[index.row()].hasAPassword;
    case Qt::DisplayRole:
    case Qt::EditRole: {
        switch (index.column()) {
        case 0:
            return m_forms[index.row()].url.host();
        case 1: {
            QStringList help;
            QStringList choices;
            foreach (const AutoFillManager::Element &element, m_forms[index.row()].elements) {
                QString key = element.first.toLower();
                if (key.contains(QLatin1String("pass")))
                    continue;
                if (key.contains(QLatin1String("user"))
                    || key.contains(QLatin1String("email"))
                    || key.contains(QLatin1String("login"))
                    || key == QLatin1String("u"))
                    return element.second;
                choices.append(element.second);
                help.append(element.first);
            }
            if (choices.count() == 1)
                return choices.first();
            qWarning() << "AutoFillModel: Unknown user id, choices:" << help << "url" << m_forms[index.row()].url.toString();
            return help.join(QLatin1String(","));
        }
        }
    }
    default:
        break;
    }
    return QVariant();
}

int AutoFillModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : 2;
}

int AutoFillModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : m_forms.count();
}

bool AutoFillModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid())
        return false;
    int lastRow = row + count - 1;
    beginRemoveRows(parent, row, lastRow);
    for (int i = lastRow; i >= row; --i)
        m_forms.removeAt(i);
    AutoFillManager *manager = BrowserApplication::instance()->autoFillManager();
    disconnect(manager, SIGNAL(autoFillChanged()), this, SLOT(autoFillChanged()));
    manager->setForms(m_forms);
    connect(manager, SIGNAL(autoFillChanged()), this, SLOT(autoFillChanged()));
    endRemoveRows();
    return true;
}

AutoFillDialog::AutoFillDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    setupUi(this);
    setWindowFlags(Qt::Sheet);
    connect(removeButton, SIGNAL(clicked()), tableView, SLOT(removeSelected()));
    connect(removeAllButton, SIGNAL(clicked()), tableView, SLOT(removeAll()));
    tableView->verticalHeader()->hide();
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setAlternatingRowColors(true);
    tableView->setTextElideMode(Qt::ElideMiddle);
    tableView->setShowGrid(false);
    tableView->setSortingEnabled(true);

    AutoFillModel *model = new AutoFillModel();
    QSortFilterProxyModel *m_proxyModel = new QSortFilterProxyModel(this);
    connect(search, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterFixedString(QString)));
    m_proxyModel->setSourceModel(model);
    tableView->setModel(m_proxyModel);

    QFont f = font();
    f.setPointSize(10);
    QFontMetrics fm(f);
    int height = fm.height() + fm.height() / 3;
    tableView->verticalHeader()->setDefaultSectionSize(height);
    tableView->verticalHeader()->setMinimumSectionSize(-1);
    for (int i = 0; i < model->columnCount(); ++i) {
        int header = tableView->horizontalHeader()->sectionSizeHint(i);
        switch (i) {
        case 0:
            header = fm.width(QLatin1String("averagehost.domain.com"));
            break;
        case 1:
            header = fm.width(QLatin1String("_session_id"));
            break;
        }
        int buffer = fm.width(QLatin1String("xx"));
        header += buffer;
        tableView->horizontalHeader()->resizeSection(i, header);
    }
    tableView->horizontalHeader()->setStretchLastSection(true);
}

