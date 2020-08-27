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

#include <qmenubar.h>

#include "addbookmarkdialog.h"
#include "bookmarksdialog.h"
#include "bookmarksmanager.h"
#include "bookmarksmodel.h"
#include "browserapplication.h"
#include "modeltest.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName(QLatin1String("endorphin"));
    QCoreApplication::setApplicationVersion(QLatin1String("0.1"));

    new ModelTest(BrowserApplication::bookmarksManager()->bookmarksModel());
    BookmarksDialog *dialog = new BookmarksDialog;
    dialog->show();

    QString url("http://www.reddit.com");
    AddBookmarkDialog adddialog;
    adddialog.setTitle(QString("Reddit.com: a time drain"));
    adddialog.setUrl(url);
    //adddialog.show();

    QMenuBar bar;
    QMenu *edit = bar.addMenu(("&Edit"));
    edit->addAction(BrowserApplication::bookmarksManager()->undoRedoStack()->createUndoAction(edit));
    edit->addAction(BrowserApplication::bookmarksManager()->undoRedoStack()->createRedoAction(edit));
    return app.exec();
}

