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

#include "historymanager.h"

#include "autosaver.h"
#include "browserapplication.h"
#include "history.h"

#include <qbuffer.h>
#include <qdesktopservices.h>
#include <qdir.h>
#include <qfile.h>
#include <qsettings.h>
#include <qtemporaryfile.h>
#include <qwebhistoryinterface.h>
#include <qwebsettings.h>

#include <qdebug.h>

QString HistoryEntry::userTitle() const
{
    // when there is no title try to generate one from the url
    if (title.isEmpty()) {
        QString page = QFileInfo(QUrl(url).path()).fileName();
        if (!page.isEmpty())
            return page;
        return url;
    }
    return title;
}

static const unsigned int HISTORY_VERSION = 23;

HistoryManager::HistoryManager(QObject *parent)
    : QWebHistoryInterface(parent)
    , m_saveTimer(new AutoSaver(this))
    , m_daysToExpire(30)
    , m_historyModel(0)
    , m_historyFilterModel(0)
    , m_historyTreeModel(0)
{
    m_expiredTimer.setSingleShot(true);
    connect(&m_expiredTimer, SIGNAL(timeout()),
            this, SLOT(checkForExpired()));
    connect(this, SIGNAL(entryAdded(const HistoryEntry &)),
            m_saveTimer, SLOT(changeOccurred()));
    connect(this, SIGNAL(entryRemoved(const HistoryEntry &)),
            m_saveTimer, SLOT(changeOccurred()));
    load();

    m_historyModel = new HistoryModel(this, this);
    m_historyFilterModel = new HistoryFilterModel(m_historyModel, this);
    m_historyTreeModel = new HistoryTreeModel(m_historyFilterModel, this);

    // QWebHistoryInterface will delete the history manager
    QWebHistoryInterface::setDefaultInterface(this);
}

HistoryManager::~HistoryManager()
{
    // remove history items on application exit
    if (m_daysToExpire == -2)
        clear();
    m_saveTimer->saveIfNeccessary();
}

QList<HistoryEntry> HistoryManager::history() const
{
    return m_history;
}

bool HistoryManager::historyContains(const QString &url) const
{
    return m_historyFilterModel->historyContains(url);
}

void HistoryManager::addHistoryEntry(const QString &url)
{
    QUrl cleanUrl(url);
    cleanUrl.setPassword(QString());
    cleanUrl.setHost(cleanUrl.host().toLower());
    HistoryEntry item(atomicString(cleanUrl.toString()), QDateTime::currentDateTime());
    prependHistoryEntry(item);
}

void HistoryManager::setHistory(const QList<HistoryEntry> &history, bool loadedAndSorted)
{
    emit historyGoingToChange();
    m_history = history;

    // verify that it is sorted by date
    if (!loadedAndSorted)
        qSort(m_history.begin(), m_history.end());

    checkForExpired();

    if (loadedAndSorted) {
        m_lastSavedUrl = m_history.value(0).url;
    } else {
        m_lastSavedUrl.clear();
        m_saveTimer->changeOccurred();
    }
    emit historyReset();
}

HistoryModel *HistoryManager::historyModel() const
{
    return m_historyModel;
}

HistoryFilterModel *HistoryManager::historyFilterModel() const
{
    return m_historyFilterModel;
}

HistoryTreeModel *HistoryManager::historyTreeModel() const
{
    return m_historyTreeModel;
}

void HistoryManager::checkForExpired()
{
    if (m_daysToExpire < 0 || m_history.isEmpty())
        return;

    QDateTime now = QDateTime::currentDateTime();
    int nextTimeout = 0;

    while (!m_history.isEmpty()) {
        QDateTime checkForExpired = m_history.last().dateTime;
        checkForExpired.setDate(checkForExpired.date().addDays(m_daysToExpire));
        if (now.daysTo(checkForExpired) > 7) {
            // check at most in a week to prevent int overflows on the timer
            nextTimeout = 7 * 86400;
        } else {
            nextTimeout = now.secsTo(checkForExpired);
        }
        if (nextTimeout > 0)
            break;
        HistoryEntry item = m_history.takeLast();
        // remove from saved file also
        m_lastSavedUrl.clear();
        emit entryRemoved(item);
    }

    if (nextTimeout > 0)
        m_expiredTimer.start(nextTimeout * 1000);
}

void HistoryManager::prependHistoryEntry(const HistoryEntry &item)
{
    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return;

    m_history.prepend(item);
    emit entryAdded(item);
    if (m_history.count() == 1)
        checkForExpired();
}

void HistoryManager::updateHistoryEntry(const QUrl &url, const QString &title)
{
    for (int i = 0; i < m_history.count(); ++i) {
        if (url == m_history.at(i).url) {
            m_history[i].title = atomicString(title);
            m_saveTimer->changeOccurred();
            if (m_lastSavedUrl.isEmpty())
                m_lastSavedUrl = m_history.at(i).url;
            emit entryUpdated(i);
            break;
        }
    }
}

void HistoryManager::removeHistoryEntry(const HistoryEntry &item)
{
    m_lastSavedUrl.clear();
    m_history.removeOne(item);
    emit entryRemoved(item);
}

void HistoryManager::removeHistoryEntry(const QUrl &url, const QString &title)
{
    for (int i = 0; i < m_history.count(); ++i) {
        if (url == m_history.at(i).url
            && (title.isEmpty() || title == m_history.at(i).title)) {
            removeHistoryEntry(m_history.at(i));
            break;
        }
    }
}

int HistoryManager::daysToExpire() const
{
    return m_daysToExpire;
}

void HistoryManager::setDaysToExpire(int limit)
{
    if (m_daysToExpire == limit)
        return;
    m_daysToExpire = limit;
    checkForExpired();
    m_saveTimer->changeOccurred();
}

void HistoryManager::clear()
{
    emit historyGoingToChange();
    m_history.clear();
    m_atomicStringHash.clear();
    m_lastSavedUrl.clear();
    m_saveTimer->changeOccurred();
    m_saveTimer->saveIfNeccessary();
    emit historyReset();
    emit historyCleared();
}

void HistoryManager::loadSettings()
{
    // load settings
    QSettings settings;
    settings.beginGroup(QLatin1String("history"));
    m_daysToExpire = settings.value(QLatin1String("historyLimit"), 30).toInt();
}

void HistoryManager::load()
{
    loadSettings();

    QFile historyFile(BrowserApplication::dataFilePath(QLatin1String("history")));

    if (!historyFile.exists())
        return;
    if (!historyFile.open(QFile::ReadOnly)) {
        qWarning() << "Unable to open history file" << historyFile.fileName();
        return;
    }

    QList<HistoryEntry> list;
    QDataStream in(&historyFile);
    // Double check that the history file is sorted as it is read in
    bool needToSort = false;
    HistoryEntry lastInsertedItem;
    QByteArray data;
    QDataStream stream;
    QBuffer buffer;
    QString string;
    stream.setDevice(&buffer);
    while (!historyFile.atEnd()) {
        in >> data;
        buffer.close();
        buffer.setBuffer(&data);
        buffer.open(QIODevice::ReadOnly);
        quint32 ver;
        stream >> ver;
        if (ver != HISTORY_VERSION)
            continue;
        HistoryEntry item;
        stream >> string;
        item.url = atomicString(string);
        stream >> item.dateTime;
        stream >> string;
        item.title = atomicString(string);

        if (!item.dateTime.isValid())
            continue;

        if (item == lastInsertedItem) {
            if (lastInsertedItem.title.isEmpty() && !list.isEmpty())
                list[0].title = item.title;
            continue;
        }

        if (!needToSort && !list.isEmpty() && lastInsertedItem < item)
            needToSort = true;

        list.prepend(item);
        lastInsertedItem = item;
    }
    if (needToSort)
        qSort(list.begin(), list.end());

    setHistory(list, true);

    // If we had to sort re-write the whole history sorted
    if (needToSort) {
        m_lastSavedUrl.clear();
        m_saveTimer->changeOccurred();
    }
}

QString HistoryManager::atomicString(const QString &string) {
    QHash<QString, int>::const_iterator it = m_atomicStringHash.constFind(string);
    if (it == m_atomicStringHash.constEnd()) {
        QHash<QString, int>::iterator insertedIterator = m_atomicStringHash.insert(string, 0);
        return insertedIterator.key();
    }
    return it.key();
}

void HistoryManager::save()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("history"));
    settings.setValue(QLatin1String("historyLimit"), m_daysToExpire);

    bool saveAll = m_lastSavedUrl.isEmpty();
    int first = m_history.count() - 1;
    if (!saveAll) {
        // find the first one to save
        for (int i = 0; i < m_history.count(); ++i) {
            if (m_history.at(i).url == m_lastSavedUrl) {
                first = i - 1;
                break;
            }
        }
    }
    if (first == m_history.count() - 1)
        saveAll = true;

    QFile historyFile(BrowserApplication::dataFilePath(QLatin1String("history")));

    // When saving everything use a temporary file to prevent possible data loss.
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    bool open = false;
    if (saveAll) {
        open = tempFile.open();
    } else {
        open = historyFile.open(QFile::Append);
    }

    if (!open) {
        qWarning() << "Unable to open history file for saving"
                   << (saveAll ? tempFile.fileName() : historyFile.fileName());
        return;
    }

    QDataStream out(saveAll ? &tempFile : &historyFile);
    for (int i = first; i >= 0; --i) {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        HistoryEntry item = m_history.at(i);
        stream << HISTORY_VERSION << item.url << item.dateTime << item.title;
        out << data;
    }
    tempFile.close();

    if (saveAll) {
        if (historyFile.exists() && !historyFile.remove())
            qWarning() << "History: error removing old history." << historyFile.errorString();
        if (!tempFile.rename(historyFile.fileName()))
            qWarning() << "History: error moving new history over old." << tempFile.errorString() << historyFile.fileName();
    }
    m_lastSavedUrl = m_history.value(0).url;
}
