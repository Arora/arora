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

#include <QtTest/QtTest>
#include <QtNetwork/QtNetwork>
#include <QtGui/QtGui>
#include "downloadmanager.h"

#define BIGFILE "http://10.0.0.3/~ben/distccKNOPPIX-1.3-2004-08-20-gcc-3.3.iso"
#define BIGFILENAME "distccKNOPPIX-1.3-2004-08-20-gcc-3.3.iso"
#define BIGFILENAME2 "distccKNOPPIX-1.3-2004-08-20-gcc-3.3-1.iso"

class tst_DownloadManager : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void downloadmanager_data();
    void downloadmanager();
    void cleanupButton_data();
    void cleanupButton();
    void download_data();
    void download();
    void removePolicy_data();
    void removePolicy();
};

// Subclass that exposes the protected functions.
class SubDownloadManager : public DownloadManager
{
public:
    SubDownloadManager(QWidget *parent = 0)
     : DownloadManager(parent)
        {}

};

// This will be called before the first test function is executed.
// It is only called once.
void tst_DownloadManager::initTestCase()
{
    QCoreApplication::setApplicationName("downloadmanagertest");
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_DownloadManager::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_DownloadManager::init()
{
    QSettings settings;
    settings.clear();

    QFile file(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)  + '/' + BIGFILENAME);
    file.remove();
}

// This will be called after every test function.
void tst_DownloadManager::cleanup()
{
}

void tst_DownloadManager::downloadmanager_data()
{
}

void tst_DownloadManager::downloadmanager()
{
    SubDownloadManager manager;
    manager.cleanup();
    manager.download(QUrl());
    manager.handleUnsupportedContent(0);
    QCOMPARE(manager.removePolicy(), DownloadManager::Never);
    manager.setRemovePolicy(DownloadManager::Never);
}

void tst_DownloadManager::cleanupButton_data()
{
    QTest::addColumn<bool>("waitForDownload");
    QTest::newRow("cancel") << false;
    QTest::newRow("cleanup") << true;
}

// public void cleanup()
void tst_DownloadManager::cleanupButton()
{
    QFETCH(bool, waitForDownload);
    {
        SubDownloadManager manager;
        QTableView *view = manager.findChild<QTableView*>();
        QVERIFY(view);
        QCOMPARE(view->model()->rowCount(), 0);
        QPushButton *cleanupButton = manager.findChild<QPushButton*>();
        QVERIFY(cleanupButton);
        QVERIFY(!cleanupButton->isEnabled());

        manager.download(QUrl(BIGFILE));
        QProgressBar *bar = manager.findChild<QProgressBar*>();
        QVERIFY(bar);

        QList<QPushButton*>buttons = manager.findChildren<QPushButton*>();
        QPushButton *tryAgainButton = 0;
        for (int i = 0; i < buttons.count(); ++i)
            if (buttons[i]->text().contains("Try"))
                tryAgainButton = buttons[i];
        QVERIFY(tryAgainButton);
        QVERIFY(tryAgainButton->isHidden());
        QVERIFY(!tryAgainButton->isEnabled());

        QCOMPARE(view->model()->rowCount(), 1);
        if (!waitForDownload) {
            QTest::qWait(500);
        } else {
            while (bar->value() != bar->maximum())
                QTest::qWait(500);
        }
        QCOMPARE(cleanupButton->isEnabled(), waitForDownload);
        QCOMPARE(view->model()->rowCount(), 1);
        manager.cleanup();
        QCOMPARE(view->model()->rowCount(), waitForDownload ? 0 : 1);
        if (view->model()->rowCount() != 0) {
            QVERIFY(tryAgainButton->isHidden());
            QVERIFY(!tryAgainButton->isEnabled());
        }
    }

    QFile file(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + '/' + BIGFILENAME);
    QCOMPARE(file.exists(), true);
    file.remove();
}

void tst_DownloadManager::download_data()
{
    QTest::addColumn<QStringList>("request");
    QTest::addColumn<QStringList>("requestfilename");
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<bool>("exists");
    QTest::newRow("badfile") << (QStringList() << BIGFILE ".dne") << (QStringList() << BIGFILENAME ".dne") << 1 << false;
    QTest::newRow("twofiles") << (QStringList() << BIGFILE << BIGFILE) << (QStringList() << BIGFILENAME << BIGFILENAME2) << 2 << true;
}

// public void download(QNetworkRequest const &request)
void tst_DownloadManager::download()
{
    QFETCH(QStringList, request);
    QFETCH(QStringList, requestfilename);
    QFETCH(int, rowCount);
    QFETCH(bool, exists);
    {
        SubDownloadManager manager;
        QTableView *view = manager.findChild<QTableView*>();
        QVERIFY(view);
        QPushButton *cleanupButton = manager.findChild<QPushButton*>();
        QVERIFY(cleanupButton);
        QVERIFY(!cleanupButton->isEnabled());
        for (int i = 0; i < request.count(); ++i)
            manager.download(QUrl(request[i]));

        QList<QPushButton*>buttons = manager.findChildren<QPushButton*>();
        QPushButton *tryAgainButton = 0;
        for (int i = 0; i < buttons.count(); ++i)
            if (buttons[i]->text().contains("Try"))
                tryAgainButton = buttons[i];
        QVERIFY(tryAgainButton);
        QVERIFY(!tryAgainButton->isEnabled());
        QVERIFY(tryAgainButton->isHidden());

        QList<QProgressBar*>bars = manager.findChildren<QProgressBar*>();
        QCOMPARE(bars.count(), rowCount);
        QCOMPARE(view->model()->rowCount(), request.count());
        QTest::qWait(1000);
        QCOMPARE(cleanupButton->isEnabled(), !exists);
        QCOMPARE(tryAgainButton->isEnabled(), !exists);
        QCOMPARE(tryAgainButton->isVisible(), !exists);
    }

    for (int i = 0; i < requestfilename.count(); ++i) {
        QFile file(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + '/' + requestfilename[i]);
        // bad file stil returns a 404 webpage
        QVERIFY(file.exists());
        file.remove();
    }
}

Q_DECLARE_METATYPE(DownloadManager::RemovePolicy)
void tst_DownloadManager::removePolicy_data()
{
    QTest::addColumn<DownloadManager::RemovePolicy>("removePolicy");
    QTest::newRow("Never") << DownloadManager::Never;
    QTest::newRow("Exit") << DownloadManager::Exit;
    QTest::newRow("SuccessFullDownload") << DownloadManager::SuccessFullDownload;
}

// public DownloadManager::RemovePolicy removePolicy() const
void tst_DownloadManager::removePolicy()
{
    QFETCH(DownloadManager::RemovePolicy, removePolicy);
    {
        SubDownloadManager manager;
        manager.setRemovePolicy(removePolicy);
        QCOMPARE(manager.removePolicy(), removePolicy);
        QTableView *view = manager.findChild<QTableView*>();
        QVERIFY(view);
        manager.download(QUrl(BIGFILE));
        QProgressBar *bar = manager.findChild<QProgressBar*>();
        QVERIFY(bar);
        QCOMPARE(view->model()->rowCount(), 1);
        while (bar && bar->value() != bar->maximum()) {
            QTest::qWait(500);
            bar = manager.findChild<QProgressBar*>();
        }
        QCOMPARE(view->model()->rowCount(), (removePolicy == DownloadManager::SuccessFullDownload) ? 0 : 1);
    }

    QFile file(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + '/' + BIGFILENAME);
    file.remove();

    SubDownloadManager manager;
    QTableView *view = manager.findChild<QTableView*>();
    QVERIFY(view);
    QCOMPARE(view->model()->rowCount(), removePolicy == DownloadManager::Never ? 1 : 0);
}

QTEST_MAIN(tst_DownloadManager)
#include "tst_downloadmanager.moc"

