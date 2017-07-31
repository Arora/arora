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

#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <qdatetime.h>
#include <qhash.h>
#include <qtimer.h>
#include <qurl.h>
#include <qwebhistoryinterface.h>

class HistoryEntry
{
public:
    HistoryEntry() {}
    HistoryEntry(const QString &u,
                const QDateTime &d = QDateTime(), const QString &t = QString())
            : url(u), title(t), dateTime(d) {}

    inline bool operator==(const HistoryEntry &other) const {
        return other.title == title
               && other.url == url && other.dateTime == dateTime;
    }

    // history is sorted in reverse
    inline bool operator <(const HistoryEntry &other) const
        { return dateTime > other.dateTime; }

    QString userTitle() const;

    QString url;
    QString title;
    QDateTime dateTime;
};

class AutoSaver;
class HistoryModel;
class HistoryFilterModel;
class HistoryTreeModel;
class HistoryManager : public QWebHistoryInterface
{
    Q_OBJECT
    Q_PROPERTY(int daysToExpire READ daysToExpire WRITE setDaysToExpire)

signals:
    void historyCleared();
    void historyGoingToChange();
    void historyReset();
    void entryAdded(const HistoryEntry &item);
    void entryRemoved(const HistoryEntry &item);
    void entryUpdated(int offset);

public:
    HistoryManager(QObject *parent = 0);
    ~HistoryManager();

    bool historyContains(const QString &url) const;
    void addHistoryEntry(const QString &url);
    void updateHistoryEntry(const QUrl &url, const QString &title);
    void removeHistoryEntry(const QUrl &url, const QString &title = QString());

    int daysToExpire() const;
    void setDaysToExpire(int limit);

    QList<HistoryEntry> history() const;
    void setHistory(const QList<HistoryEntry> &history, bool loadedAndSorted = false);

    // History manager keeps around these models for use by the completer and other classes
    HistoryModel *historyModel() const;
    HistoryFilterModel *historyFilterModel() const;
    HistoryTreeModel *historyTreeModel() const;

public slots:
    void clear();
    void loadSettings();

private slots:
    void save();
    void checkForExpired();

protected:
    void prependHistoryEntry(const HistoryEntry &item);
    void removeHistoryEntry(const HistoryEntry &item);

private:
    void load();
    QString atomicString(const QString &string);
    void startFrecencyTimer();

    AutoSaver *m_saveTimer;
    int m_daysToExpire;
    QTimer m_expiredTimer;
    QHash<QString, int> m_atomicStringHash;
    QList<HistoryEntry> m_history;
    QString m_lastSavedUrl;

    HistoryModel *m_historyModel;
    HistoryFilterModel *m_historyFilterModel;
    HistoryTreeModel *m_historyTreeModel;
};

#endif // HISTORYMANAGER_H

