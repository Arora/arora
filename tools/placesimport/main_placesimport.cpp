/*
 * Copyright 2008-2009 Benjamin K. Stuhl <bks24@cornell.edu>
 * Copyright 2009 Benjamin C. Meyer <ben@meyerhome.net>
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

#include <qdatetime.h>
#include <qdebug.h>
#include <qdir.h>
#include <qfile.h>
#include <QSqlDatabase>
#include <qsqlerror.h>
#include <qsqlquery.h>
#include <qtextstream.h>
#include <qvariant.h>

#include "singleapplication.h"
#include "historymanager.h"

static HistoryEntry formatEntry(QByteArray url, QByteArray title, qlonglong prdate)
{
    QDateTime dateTime = QDateTime::fromTime_t(prdate / 1000000);
    dateTime.addMSecs((prdate % 1000000) / 1000);
    HistoryEntry entry(url, dateTime, title);
    return entry;
}

int main(int argc, char **argv)
{
    SingleApplication application(argc, argv);
    QCoreApplication::setOrganizationDomain(QLatin1String("arora-browser.org"));
    QCoreApplication::setApplicationName(QLatin1String("Arora"));

    if (application.sendMessage(QByteArray())) {
        qWarning() << "To prevent the loss of any history please exit Arora while this is tool is being run";
        return 1;
    }

    QStringList args = application.arguments();
    args.takeFirst();
    if (args.isEmpty()) {
        QTextStream stream(stdout);
        stream << "arora-placesimport is a tool for importing browser history from Firefox 3 and up" << endl;
        stream << "arora-placesinfo ~/.mozilla/firefox/[profile-dir]/places.sqlite" << endl;
        return 0;
    }

    QSqlDatabase placesDatabase = QSqlDatabase::addDatabase("QSQLITE");
    placesDatabase.setDatabaseName(args.first());

    if (!placesDatabase.open()) {
        qWarning("Unable to open database: %s", qPrintable(placesDatabase.lastError().text()));
        return 1;
    }

    QSqlQuery historyQuery(
        "SELECT moz_places.url, moz_places.title, moz_historyvisits.visit_date "
        "FROM moz_places, moz_historyvisits "
        "WHERE moz_places.id = moz_historyvisits.place_id;");
    historyQuery.setForwardOnly(true);

    if (!historyQuery.exec()) {
        qWarning("Unable to extract history: %s.  Is Firefox running?", qPrintable(historyQuery.lastError().text()));
        return 1;
    }

    HistoryManager manager;
    QList<HistoryEntry> history = manager.history();
    while (historyQuery.next()) {
        QByteArray url = historyQuery.value(0).toByteArray();
        QByteArray title = historyQuery.value(1).toByteArray();
        qlonglong prdate = historyQuery.value(2).toLongLong();
        HistoryEntry entry = formatEntry(url, title, prdate);
        history.append(entry);
    }
    manager.setHistory(history);

    return 0;
}

