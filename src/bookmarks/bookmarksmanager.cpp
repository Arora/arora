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

#include "bookmarksmanager.h"

#include "autosaver.h"
#include "bookmarknode.h"
#include "bookmarksmodel.h"
#include "browserapplication.h"
#include "history.h"
#include "xbelreader.h"
#include "xbelwriter.h"

#include <qbuffer.h>
#include <qdesktopservices.h>
#include <qevent.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qheaderview.h>
#include <qicon.h>
#include <qmessagebox.h>
#include <qmimedata.h>
#include <qtoolbutton.h>
#include <qprocess.h>

#include <qwebsettings.h>

#include <qdebug.h>

#define BOOKMARKBAR QT_TRANSLATE_NOOP("BookmarksManager", "Bookmarks Bar")
#define BOOKMARKMENU QT_TRANSLATE_NOOP("BookmarksManager", "Bookmarks Menu")

BookmarksManager::BookmarksManager(QObject *parent)
    : QObject(parent)
    , m_loaded(false)
    , m_saveTimer(new AutoSaver(this))
    , m_bookmarkRootNode(0)
    , m_toolbar(0)
    , m_menu(0)
    , m_bookmarkModel(0)
{
    connect(this, SIGNAL(entryAdded(BookmarkNode *)),
            m_saveTimer, SLOT(changeOccurred()));
    connect(this, SIGNAL(entryRemoved(BookmarkNode *, int, BookmarkNode *)),
            m_saveTimer, SLOT(changeOccurred()));
    connect(this, SIGNAL(entryChanged(BookmarkNode *)),
            m_saveTimer, SLOT(changeOccurred()));
}

BookmarksManager::~BookmarksManager()
{
    m_saveTimer->saveIfNeccessary();
    delete m_bookmarkRootNode;
}

void BookmarksManager::changeExpanded()
{
    m_saveTimer->changeOccurred();
}

void BookmarksManager::load()
{
    if (m_loaded)
        return;
    m_loaded = true;

    QString dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/data/Arora";
    QString bookmarkFile = dir + QLatin1String("/bookmarks.xbel");
    if (!QFile::exists(bookmarkFile))
        bookmarkFile = QLatin1String(":defaultbookmarks.xbel");

    XbelReader reader;
    m_bookmarkRootNode = reader.read(bookmarkFile);
    if (reader.error() != QXmlStreamReader::NoError) {
        QMessageBox::warning(0, QLatin1String("Loading Bookmark"),
            tr("Error when loading bookmarks on line %1, column %2:\n"
               "%3").arg(reader.lineNumber()).arg(reader.columnNumber()).arg(reader.errorString()));
    }

    QList<BookmarkNode*> others;
    for (int i = m_bookmarkRootNode->children().count() - 1; i >= 0; --i) {
        BookmarkNode *node = m_bookmarkRootNode->children().at(i);
        if (node->type() == BookmarkNode::Folder) {
            // Automatically convert
            if ((node->title == tr("Toolbar Bookmarks")
                 || node->title == QLatin1String(BOOKMARKBAR)) && !m_toolbar) {
                node->title = tr(BOOKMARKBAR);

                m_toolbar = node;
            }

            // Automatically convert
            if ((node->title == tr("Menu")
                 || node->title == QLatin1String(BOOKMARKMENU)) && !m_menu) {
                node->title = tr(BOOKMARKMENU);
                m_menu = node;
            }
        } else {
            others.append(node);
        }
        m_bookmarkRootNode->remove(node);
    }
    Q_ASSERT(m_bookmarkRootNode->children().count() == 0);
    if (!m_toolbar) {
        m_toolbar = new BookmarkNode(BookmarkNode::Folder, m_bookmarkRootNode);
        m_toolbar->title = tr(BOOKMARKBAR);
    } else {
        m_bookmarkRootNode->add(m_toolbar);
    }

    if (!m_menu) {
        m_menu = new BookmarkNode(BookmarkNode::Folder, m_bookmarkRootNode);
        m_menu->title = tr(BOOKMARKMENU);
    } else {
        m_bookmarkRootNode->add(m_menu);
    }

    for (int i = 0; i < others.count(); ++i)
        m_menu->add(others.at(i));
}

void BookmarksManager::save() const
{
    if (!m_loaded)
        return;

    XbelWriter writer;
    QString dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/data/Arora";
    QString bookmarkFile = dir + QLatin1String("/bookmarks.xbel");
    // Save root folder titles in English (i.e. not localized)
    m_menu->title = QLatin1String(BOOKMARKMENU);
    m_toolbar->title = QLatin1String(BOOKMARKBAR);
    if (!writer.write(bookmarkFile, m_bookmarkRootNode))
        qWarning() << "BookmarkManager: error saving to" << bookmarkFile;
    // Restore localized titles
    retranslate();
}

void BookmarksManager::retranslate() const
{
    if (m_menu)
        m_menu->title = tr(BOOKMARKMENU);
    if (m_toolbar)
        m_toolbar->title = tr(BOOKMARKBAR);
}

void BookmarksManager::addBookmark(BookmarkNode *parent, BookmarkNode *node, int row)
{
    if (!m_loaded)
        return;
    Q_ASSERT(parent);
    InsertBookmarksCommand *command = new InsertBookmarksCommand(this, parent, node, row);
    m_commands.push(command);
}

void BookmarksManager::removeBookmark(BookmarkNode *node)
{
    if (!m_loaded)
        return;

    Q_ASSERT(node);
    BookmarkNode *parent = node->parent();
    int row = parent->children().indexOf(node);
    RemoveBookmarksCommand *command = new RemoveBookmarksCommand(this, parent, row);
    m_commands.push(command);
}

void BookmarksManager::setTitle(BookmarkNode *node, const QString &newTitle)
{
    if (!m_loaded)
        return;

    Q_ASSERT(node);
    ChangeBookmarkCommand *command = new ChangeBookmarkCommand(this, node, newTitle, true);
    m_commands.push(command);
}

void BookmarksManager::setUrl(BookmarkNode *node, const QString &newUrl)
{
    if (!m_loaded)
        return;

    Q_ASSERT(node);
    ChangeBookmarkCommand *command = new ChangeBookmarkCommand(this, node, newUrl, false);
    m_commands.push(command);
}

BookmarkNode *BookmarksManager::bookmarks()
{
    if (!m_loaded)
        load();
    return m_bookmarkRootNode;
}

BookmarkNode *BookmarksManager::menu()
{
    if (!m_loaded)
        load();

    Q_ASSERT(m_menu);
    return m_menu;
}

BookmarkNode *BookmarksManager::toolbar()
{
    if (!m_loaded)
        load();

    Q_ASSERT(m_toolbar);
    return m_toolbar;
}

BookmarksModel *BookmarksManager::bookmarksModel()
{
    if (!m_bookmarkModel)
        m_bookmarkModel = new BookmarksModel(this, this);
    return m_bookmarkModel;
}

void BookmarksManager::importBookmarks()
{
    QStringList supportedFormats;
    supportedFormats << tr("XBEL bookmarks").append(QLatin1String("(*.xbel *.xml)"));
    supportedFormats << tr("HTML Netscape bookmarks").append(QLatin1String("(*.html)"));

    QString fileName = QFileDialog::getOpenFileName(0, tr("Open File"),
                                                    QString(), supportedFormats.join(QLatin1String(";;")));
    if (fileName.isEmpty())
        return;

    XbelReader reader;
    BookmarkNode *importRootNode = 0;
    if (fileName.endsWith(QLatin1String(".html"))) {
        QString program = QLatin1String("htmlToXBel");
        QStringList arguments;
        arguments << fileName;
        QProcess process;
        process.start(program, arguments);
        process.waitForFinished(-1);
        if (process.error() != QProcess::UnknownError) {
            if (process.error() == QProcess::FailedToStart) {
                QMessageBox::warning(0, tr("htmlToXBel tool required"),
                    tr("htmlToXBel tool, which is shipped with Arora and is needed to import HTML bookmarks, "
                       "is not installed or not available in the search paths."));
            } else {
                QMessageBox::warning(0, tr("Loading Bookmark"),
                    tr("Error when loading HTML bookmarks: %1\n").arg(process.errorString()));
            }
            return;
        }
        importRootNode = reader.read(&process);
    } else {
        importRootNode = reader.read(fileName);
    }
    if (reader.error() != QXmlStreamReader::NoError) {
        QMessageBox::warning(0, QLatin1String("Loading Bookmark"),
            tr("Error when loading bookmarks on line %1, column %2:\n"
               "%3").arg(reader.lineNumber()).arg(reader.columnNumber()).arg(reader.errorString()));
        delete importRootNode;
        return;
    }

    importRootNode->setType(BookmarkNode::Folder);
    importRootNode->title = (tr("Imported %1").arg(QDate::currentDate().toString(Qt::SystemLocaleShortDate)));
    addBookmark(menu(), importRootNode);
}

void BookmarksManager::exportBookmarks()
{
    QString fileName = QFileDialog::getSaveFileName(0, tr("Save File"),
                                tr("%1 Bookmarks.xbel").arg(QCoreApplication::applicationName()),
                                tr("XBEL bookmarks").append(QLatin1String("(*.xbel *.xml)")));
    if (fileName.isEmpty())
        return;

    XbelWriter writer;
    if (!writer.write(fileName, m_bookmarkRootNode))
        QMessageBox::critical(0, tr("Export error"), tr("error saving bookmarks"));
}

RemoveBookmarksCommand::RemoveBookmarksCommand(BookmarksManager *m_bookmarkManagaer, BookmarkNode *parent, int row)
    : QUndoCommand(BookmarksManager::tr("Remove Bookmark"))
    , m_row(row)
    , m_bookmarkManagaer(m_bookmarkManagaer)
    , m_node(parent->children().value(row))
    , m_parent(parent)
    , m_done(false)
{
}

RemoveBookmarksCommand::~RemoveBookmarksCommand()
{
    if (m_done && !m_node->parent()) {
        delete m_node;
    }
}

void RemoveBookmarksCommand::undo()
{
    m_parent->add(m_node, m_row);
    emit m_bookmarkManagaer->entryAdded(m_node);
    m_done = false;
}

void RemoveBookmarksCommand::redo()
{
    m_parent->remove(m_node);
    emit m_bookmarkManagaer->entryRemoved(m_parent, m_row, m_node);
    m_done = true;
}

InsertBookmarksCommand::InsertBookmarksCommand(BookmarksManager *m_bookmarkManagaer,
                BookmarkNode *parent, BookmarkNode *node, int row)
    : RemoveBookmarksCommand(m_bookmarkManagaer, parent, row)
{
    setText(BookmarksManager::tr("Insert Bookmark"));
    m_node = node;
}

ChangeBookmarkCommand::ChangeBookmarkCommand(BookmarksManager *m_bookmarkManagaer, BookmarkNode *node,
                        const QString &newValue, bool title)
    : QUndoCommand()
    , m_bookmarkManagaer(m_bookmarkManagaer)
    , m_title(title)
    , m_newValue(newValue)
    , m_node(node)
{
    if (m_title) {
        m_oldValue = m_node->title;
        setText(BookmarksManager::tr("Name Change", "Undo bookmark title change"));
    } else {
        m_oldValue = m_node->url;
        setText(BookmarksManager::tr("Address Change", "Undo bookmark url change"));
    }
}

void ChangeBookmarkCommand::undo()
{
    if (m_title)
        m_node->title = m_oldValue;
    else
        m_node->url = m_oldValue;
    emit m_bookmarkManagaer->entryChanged(m_node);
}

void ChangeBookmarkCommand::redo()
{
    if (m_title)
        m_node->title = m_newValue;
    else
        m_node->url = m_newValue;
    emit m_bookmarkManagaer->entryChanged(m_node);
}

