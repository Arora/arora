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

#include <qmainwindow.h>
#include <qtableview.h>

#include <history.h>
#include <historycompleter.h>
#include <historymanager.h>
#include <modeltest.h>
#include <qheaderview.h>

int main(int argc, char **argv)
{
    QApplication application(argc, argv);
    QMainWindow w;

    QCoreApplication::setApplicationName("endorphin");

    HistoryManager history;

    QTabWidget tabWidget;

    QTableView *historyModelView = new QTableView(&tabWidget);
    historyModelView->horizontalHeader()->setStretchLastSection(true);
    historyModelView->setModel(history.historyModel());
    tabWidget.addTab(historyModelView, "HistoryModel");

    QTableView *historyFilterModelView = new QTableView(&tabWidget);
    historyFilterModelView->horizontalHeader()->setStretchLastSection(true);
    historyFilterModelView->setModel(history.historyFilterModel());
    tabWidget.addTab(historyFilterModelView, "HistoryFilterModel");

    QTreeView *historyTreeModelView = new QTreeView(&tabWidget);
    historyTreeModelView->header()->setStretchLastSection(true);
    historyTreeModelView->setModel(history.historyTreeModel());
    new ModelTest(history.historyTreeModel());
    tabWidget.addTab(historyTreeModelView, "HistoryTreeModel");

    QTreeView *historyMenuModelView = new QTreeView(&tabWidget);
    historyMenuModelView->header()->setStretchLastSection(true);
    HistoryMenuModel *menuModel = new HistoryMenuModel((HistoryTreeModel*)history.historyTreeModel(), &tabWidget);
    new ModelTest(menuModel);
    historyMenuModelView->setModel(menuModel);
    tabWidget.addTab(historyMenuModelView, "HistoryMenuModel");

    QTableView *historyCompletionModelView = new QTableView(&tabWidget);
    historyCompletionModelView->horizontalHeader()->setStretchLastSection(true);
    HistoryCompletionModel *completionModel = new HistoryCompletionModel(&tabWidget);
    completionModel->setSourceModel(history.historyFilterModel());
    historyCompletionModelView->setModel(completionModel);
    tabWidget.addTab(historyCompletionModelView, "HistoryCompletionModel");

    HistoryDialog dialog(0, &history);
    tabWidget.addTab(dialog.tree, "DialogModel");

    tabWidget.setCurrentIndex(3);
    tabWidget.show();

    return application.exec();
}

