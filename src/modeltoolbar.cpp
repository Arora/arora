/*
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

#include "modeltoolbar.h"

#include "browserapplication.h"
#include "modelmenu.h"

#include <qevent.h>
#include <qtoolbutton.h>
#include <QDrag>
#include <QMimeData>


ModelToolBar::ModelToolBar(QWidget *parent)
    : QToolBar(parent)
    , m_model(0)
{
    if (isVisible())
        build();

    setAcceptDrops(true);
}

ModelToolBar::ModelToolBar(const QString &title, QWidget *parent)
    : QToolBar(title, parent)
    , m_model(0)
{
    if (isVisible())
        build();

    setAcceptDrops(true);
}

void ModelToolBar::setModel(QAbstractItemModel *model)
{
    if (m_model) {
        disconnect(m_model, SIGNAL(modelReset()),
                   this, SLOT(build()));
        disconnect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                   this, SLOT(build()));
        disconnect(m_model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                   this, SLOT(build()));
        disconnect(m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                   this, SLOT(build()));
    }

    m_model = model;

    if (m_model) {
        connect(m_model, SIGNAL(modelReset()),
                this, SLOT(build()));
        connect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                this, SLOT(build()));
        connect(m_model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                this, SLOT(build()));
        connect(m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                this, SLOT(build()));
    }
}

QAbstractItemModel *ModelToolBar::model() const
{
    return m_model;
}

void ModelToolBar::setRootIndex(const QModelIndex &index)
{
    m_rootIndex = index;
}

QModelIndex ModelToolBar::rootIndex() const
{
    return m_rootIndex;
}

void ModelToolBar::build()
{
    Q_ASSERT(m_model);

    clear();

    for (int i = 0; i < m_model->rowCount(m_rootIndex); ++i) {
        QModelIndex index = m_model->index(i, 0, m_rootIndex);
        QVariant variant;
        variant.setValue(index);

        QString title = index.data(Qt::DisplayRole).toString();
        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        bool hasChildren = m_model->hasChildren(index);

        QAction *action = addAction(icon, title);
        action->setData(variant);

        QWidget *actionWidget = widgetForAction(action);
        QToolButton *button = qobject_cast<QToolButton*>(actionWidget);
        Q_ASSERT(button);
        button->installEventFilter(this);

        if (hasChildren) {
            ModelMenu *menu = createMenu();
            menu->setModel(m_model);
            menu->setRootIndex(index);
            action->setMenu(menu);
            button->setPopupMode(QToolButton::InstantPopup);
            button->setArrowType(Qt::DownArrow);
        }
    }
}

QModelIndex ModelToolBar::index(QAction *action)
{
    if (!action)
        return QModelIndex();

    QVariant variant = action->data();
    if (!variant.canConvert<QModelIndex>())
        return QModelIndex();

    QModelIndex index = qvariant_cast<QModelIndex>(variant);
    return index;
}

ModelMenu *ModelToolBar::createMenu()
{
    return new ModelMenu(this);
}

bool ModelToolBar::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QToolButton *button = static_cast<QToolButton*>(object);
        Q_ASSERT(button);

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        BrowserApplication::instance()->setEventMouseButtons(mouseEvent->button());
        BrowserApplication::instance()->setEventKeyboardModifiers(mouseEvent->modifiers());
        QAction *action = button->defaultAction();
        Q_ASSERT(action);
        QModelIndex index = this->index(action);
        Q_ASSERT(this->index(action).isValid());
        emit activated(index);
    } else if (event->type() == QEvent::MouseButtonPress) {
        Q_ASSERT(static_cast<QToolButton*>(object));

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->buttons() & Qt::LeftButton)
            m_dragStartPos = mapFromGlobal(mouseEvent->globalPos());
    }

    return false;
}

void ModelToolBar::dragEnterEvent(QDragEnterEvent *event)
{
    if (!m_model) {
        QToolBar::dragEnterEvent(event);
        return;
    }

    QStringList mimeTypes = m_model->mimeTypes();
    foreach (const QString &mimeType, mimeTypes) {
        if (event->mimeData()->hasFormat(mimeType))
            event->acceptProposedAction();
    }

    QToolBar::dragEnterEvent(event);
}

void ModelToolBar::hideEvent(QHideEvent *event)
{
    clear();
    QToolBar::hideEvent(event);
}

void ModelToolBar::showEvent(QShowEvent *event)
{
    if (actions().isEmpty())
        build();
    QToolBar::showEvent(event);
}

void ModelToolBar::dropEvent(QDropEvent *event)
{
    if (!m_model) {
        QToolBar::dropEvent(event);
        return;
    }

    int row;
    QAction *action = actionAt(mapFromGlobal(QCursor::pos()));
    QModelIndex index;
    QModelIndex parentIndex = m_rootIndex;
    if (!action) {
        row = m_model->rowCount(m_rootIndex);
    } else {
        index = this->index(action);
        Q_ASSERT(index.isValid());
        row = index.row();

        if (m_model->hasChildren(index))
            parentIndex = index;
    }

    event->acceptProposedAction();
    m_model->dropMimeData(event->mimeData(), event->dropAction(), row, 0, parentIndex);
    QToolBar::dropEvent(event);
}

void ModelToolBar::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_model) {
        QToolBar::mouseMoveEvent(event);
        return;
    }

    if (!(event->buttons() & Qt::LeftButton)) {
        QToolBar::mouseMoveEvent(event);
        return;
    }

    int manhattanLength = (event->pos() - m_dragStartPos).manhattanLength();
    if (manhattanLength <= QApplication::startDragDistance()) {
        QToolBar::mouseMoveEvent(event);
        return;
    }

    QAction *action = actionAt(m_dragStartPos);
    if (!action) {
        QToolBar::mouseMoveEvent(event);
        return;
    }

    QPersistentModelIndex index = this->index(action);
    Q_ASSERT(index.isValid());

    QDrag *drag = new QDrag(this);
    drag->setMimeData(m_model->mimeData(QModelIndexList() << index));
    QRect actionRect = actionGeometry(action);
    drag->setPixmap(QPixmap::grabWidget(this, actionRect));

    if (drag->exec() == Qt::MoveAction)
        m_model->removeRow(index.row(), m_rootIndex);
}
