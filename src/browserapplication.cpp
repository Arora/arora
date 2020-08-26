/*
 * Copyright 2008-2009 Benjamin C. Meyer <ben@meyerhome.net>
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

#include "browserapplication.h"

#include "autosaver.h"
#include "autofillmanager.h"
#include "bookmarksmanager.h"
#include "browsermainwindow.h"
#include "cookiejar.h"
#include "downloadmanager.h"
#include "historymanager.h"
#include "languagemanager.h"
#include "networkaccessmanager.h"
#include "tabwidget.h"
#include "webview.h"

#include <qbuffer.h>
#include <qdesktopservices.h>
#include <qdir.h>
#include <qevent.h>
#include <qlibraryinfo.h>
#include <qlocalsocket.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qwebsettings.h>

#include <qdebug.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// #define BROWSERAPPLICATION_DEBUG

DownloadManager *BrowserApplication::s_downloadManager = 0;
HistoryManager *BrowserApplication::s_historyManager = 0;
NetworkAccessManager *BrowserApplication::s_networkAccessManager = 0;
BookmarksManager *BrowserApplication::s_bookmarksManager = 0;
LanguageManager *BrowserApplication::s_languageManager = 0;
AutoFillManager *BrowserApplication::s_autoFillManager = 0;

BrowserApplication::BrowserApplication(int &argc, char **argv)
    : SingleApplication(argc, argv)
    , quitting(false)
{
    QCoreApplication::setOrganizationDomain(QLatin1String("arora-browser.org"));
    QCoreApplication::setApplicationName(QLatin1String("Arora"));
    QCoreApplication::setApplicationVersion(QLatin1String("0.12.0"
#ifdef GITVERSION
    " (Git: " GITCHANGENUMBER " " GITVERSION ")"
#endif
    ));

#ifndef AUTOTESTS
    connect(this, SIGNAL(messageReceived(QLocalSocket *)),
            this, SLOT(messageReceived(QLocalSocket *)));

    QStringList args = QCoreApplication::arguments();
    if (args.count() > 1) {
        QString message = parseArgumentUrl(args.last());
        sendMessage(message.toUtf8());
    }
    // If we could connect to another Arora then exit
    QString message = QString(QLatin1String("aroramessage://getwinid"));
    if (sendMessage(message.toUtf8(), 500))
        return;

#ifdef BROWSERAPPLICATION_DEBUG
    qDebug() << "BrowserApplication::" << __FUNCTION__ << "I am the only arora";
#endif

    // not sure what else to do...
    if (!startSingleServer())
        return;
#endif

#if defined(Q_WS_MAC)
    QApplication::setQuitOnLastWindowClosed(false);
#else
    QApplication::setQuitOnLastWindowClosed(true);
#endif

    QDesktopServices::setUrlHandler(QLatin1String("http"), this, "openUrl");

    // Until QtWebkit defaults to 16
    QWebSettings::globalSettings()->setFontSize(QWebSettings::DefaultFontSize, 16);
    QWebSettings::globalSettings()->setFontSize(QWebSettings::DefaultFixedFontSize, 16);

    QSettings settings;
    settings.beginGroup(QLatin1String("sessions"));
    m_lastSession = settings.value(QLatin1String("lastSession")).toByteArray();
    settings.endGroup();

#if defined(Q_WS_MAC)
    connect(this, SIGNAL(lastWindowClosed()),
            this, SLOT(lastWindowClosed()));
#endif

    // setting this in the postLaunch actually takes a lot more time
    // because the event has to be propagated to everyone.
    setWindowIcon(QIcon(QLatin1String(":128x128/arora.png")));

#ifndef AUTOTESTS
    QTimer::singleShot(0, this, SLOT(postLaunch()));
#endif
    languageManager();
}

BrowserApplication::~BrowserApplication()
{
    quitting = true;
    delete s_downloadManager;
    qDeleteAll(m_mainWindows);
    delete s_networkAccessManager;
    delete s_bookmarksManager;
    delete s_languageManager;
    delete s_historyManager;
    delete s_autoFillManager;
}

#if defined(Q_WS_MAC)
void BrowserApplication::lastWindowClosed()
{
    clean();
    BrowserMainWindow *mw = new BrowserMainWindow;
    mw->goHome();
    m_mainWindows.prepend(mw);
}
#endif

BrowserApplication *BrowserApplication::instance()
{
    return (static_cast<BrowserApplication*>(QCoreApplication::instance()));
}

void BrowserApplication::retranslate()
{
    bookmarksManager()->retranslate();
    networkAccessManager()->loadSettings();
}

// The only special property of an argument url is that the file's
// can be local, they don't have to be absolute.
QString BrowserApplication::parseArgumentUrl(const QString &string) const
{
    if (QFile::exists(string)) {
        QFileInfo info(string);
        return info.canonicalFilePath();
    }
    return string;
}

void BrowserApplication::messageReceived(QLocalSocket *socket)
{
    QString message;
    QTextStream stream(socket);
    stream >> message;
#ifdef BROWSERAPPLICATION_DEBUG
    qDebug() << "BrowserApplication::" << __FUNCTION__ << message;
#endif
    if (message.isEmpty())
        return;

    // Got a normal url
    if (!message.startsWith(QLatin1String("arora://"))) {
        QSettings settings;
        settings.beginGroup(QLatin1String("tabs"));
        TabWidget::OpenUrlIn tab = TabWidget::OpenUrlIn(settings.value(QLatin1String("openLinksFromAppsIn"), TabWidget::NewSelectedTab).toInt());
        settings.endGroup();
        if (QUrl(message) == m_lastAskedUrl
                && m_lastAskedUrlDateTime.addSecs(10) > QDateTime::currentDateTime()) {
            qWarning() << "Possible recursive openUrl called, ignoring url:" << m_lastAskedUrl;
            return;
        }
        mainWindow()->tabWidget()->loadString(message, tab);
        return;
    } else {
        if (message.startsWith(QLatin1String("arora://getwinid"))) {
        #ifdef Q_OS_WIN
            QString winid = QString(QLatin1String("%1")).arg((qlonglong)mainWindow()->winId());
        #else
            mainWindow()->show();
            mainWindow()->setFocus();
            mainWindow()->raise();
            mainWindow()->activateWindow();
            alert(mainWindow());
            QString winid;
        #endif
        #ifdef BROWSERAPPLICATION_DEBUG
            qDebug() << "BrowserApplication::" << __FUNCTION__ << "sending win id" << winid << mainWindow()->winId();
        #endif
            QString message = QLatin1String("arora://winid/") + winid;
            socket->write(message.toUtf8());
            socket->waitForBytesWritten();
            return;
	}

       if (message.startsWith(QLatin1String("arora://winid"))) {
            QString winid = message.mid(21);
        #ifdef BROWSERAPPLICATION_DEBUG
            qDebug() << "BrowserApplication::" << __FUNCTION__ << "got win id:" << winid;
        #endif
        #ifdef Q_OS_WIN
            WId wid = (WId)winid.toLongLong();
            SetForegroundWindow(wid);
        #endif
            return;
        }
        QSettings settings;
        settings.beginGroup(QLatin1String("tabs"));
        TabWidget::OpenUrlIn tab = TabWidget::OpenUrlIn(settings.value(QLatin1String("openLinksFromAppsIn"), TabWidget::NewSelectedTab).toInt());
        settings.endGroup();
        if (QUrl(message) == m_lastAskedUrl
                && m_lastAskedUrlDateTime.addSecs(10) > QDateTime::currentDateTime()) {
            qWarning() << "Possible recursive openUrl called, ignoring url:" << m_lastAskedUrl;
            return;
        }
        mainWindow()->tabWidget()->loadString(message, tab);
        return;
    }
}

void BrowserApplication::quitBrowser()
{
    if (s_downloadManager && !downloadManager()->allowQuit())
        return;

    if (QSettings().value(QLatin1String("tabs/confirmClosingMultipleTabs"), true).toBool()) {
        clean();
        int tabCount = 0;
        for (int i = 0; i < m_mainWindows.count(); ++i) {
            tabCount += m_mainWindows.at(i)->tabWidget()->count();
        }

        if (tabCount > 1) {
            QWidget *widget = mainWindow();
            QApplication::alert(widget);
            int ret = QMessageBox::warning(widget, QString(),
                               tr("There are %1 windows and %2 tabs open\n"
                                  "Do you want to quit anyway?").arg(m_mainWindows.count()).arg(tabCount),
                               QMessageBox::Yes | QMessageBox::No,
                               QMessageBox::No);
            if (ret == QMessageBox::No)
                return;
        }
    }

    saveSession();
    exit(0);
}

/*!
    Any actions that can be delayed until the window is visible
 */
void BrowserApplication::postLaunch()
{
    QString directory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (directory.isEmpty())
        directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
    QWebSettings::setIconDatabasePath(directory);

    loadSettings();

    // newMainWindow() needs to be called in main() for this to happen
    if (m_mainWindows.count() > 0) {
        QSettings settings;
        settings.beginGroup(QLatin1String("MainWindow"));
        int startup = settings.value(QLatin1String("startupBehavior")).toInt();
        QStringList args = QCoreApplication::arguments();

        if (args.count() > 1) {
            QString argumentUrl = parseArgumentUrl(args.last());
            switch (startup) {
            case 2: {
                restoreLastSession();
                mainWindow()->tabWidget()->loadString(argumentUrl, TabWidget::NewSelectedTab);
                break;
            }
            default:
                mainWindow()->tabWidget()->loadString(argumentUrl);
                break;
            }
        } else {
            switch (startup) {
            case 0:
                mainWindow()->goHome();
                break;
            case 1:
                break;
            case 2:
                restoreLastSession();
                break;
            }
        }
    }
    BrowserApplication::historyManager();
}

void BrowserApplication::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("websettings"));

    QWebSettings *defaultSettings = QWebSettings::globalSettings();
    QString standardFontFamily = defaultSettings->fontFamily(QWebSettings::StandardFont);
    int standardFontSize = defaultSettings->fontSize(QWebSettings::DefaultFontSize);
    QFont standardFont = QFont(standardFontFamily, standardFontSize);
    standardFont = settings.value(QLatin1String("standardFont"), standardFont).value<QFont>();
    defaultSettings->setFontFamily(QWebSettings::StandardFont, standardFont.family());
    defaultSettings->setFontSize(QWebSettings::DefaultFontSize, standardFont.pointSize());
    int minimumFontSize = settings.value(QLatin1String("minimumFontSize"),
                defaultSettings->fontSize(QWebSettings::MinimumFontSize)).toInt();
    defaultSettings->setFontSize(QWebSettings::MinimumFontSize, minimumFontSize);

    QString fixedFontFamily = defaultSettings->fontFamily(QWebSettings::FixedFont);
    int fixedFontSize = defaultSettings->fontSize(QWebSettings::DefaultFixedFontSize);
    QFont fixedFont = QFont(fixedFontFamily, fixedFontSize);
    fixedFont = settings.value(QLatin1String("fixedFont"), fixedFont).value<QFont>();
    defaultSettings->setFontFamily(QWebSettings::FixedFont, fixedFont.family());
    defaultSettings->setFontSize(QWebSettings::DefaultFixedFontSize, fixedFont.pointSize());

    defaultSettings->setAttribute(QWebSettings::JavascriptCanOpenWindows, !(settings.value(QLatin1String("blockPopupWindows"), true).toBool()));
    defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, settings.value(QLatin1String("enableJavascript"), true).toBool());
    defaultSettings->setAttribute(QWebSettings::PluginsEnabled, settings.value(QLatin1String("enablePlugins"), true).toBool());
    defaultSettings->setAttribute(QWebSettings::AutoLoadImages, settings.value(QLatin1String("enableImages"), true).toBool());
    defaultSettings->setAttribute(QWebSettings::LocalStorageEnabled, settings.value(QLatin1String("enableLocalStorage"), true).toBool());
    defaultSettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, settings.value(QLatin1String("enableInspector"), false).toBool());
    defaultSettings->setAttribute(QWebSettings::DnsPrefetchEnabled, true);

    QUrl url = settings.value(QLatin1String("userStyleSheet")).toUrl();
    defaultSettings->setUserStyleSheetUrl(url);

    int maximumPagesInCache = settings.value(QLatin1String("maximumPagesInCache"), 3).toInt();
    QWebSettings::globalSettings()->setMaximumPagesInCache(maximumPagesInCache);

    settings.endGroup();
}

QList<BrowserMainWindow*> BrowserApplication::mainWindows()
{
    clean();
    QList<BrowserMainWindow*> list;
    for (int i = 0; i < m_mainWindows.count(); ++i)
        list.append(m_mainWindows.at(i));
    return list;
}

bool BrowserApplication::allowToCloseWindow(BrowserMainWindow *window)
{
    Q_UNUSED(window)
    if (mainWindows().count() > 1)
        return true;

    if (s_downloadManager)
        return downloadManager()->allowQuit();

    return true;
}

void BrowserApplication::clean()
{
    // cleanup any deleted main windows first
    for (int i = m_mainWindows.count() - 1; i >= 0; --i)
        if (m_mainWindows.at(i).isNull())
            m_mainWindows.removeAt(i);
}

static const qint32 BrowserApplicationMagic = 0xec;

void BrowserApplication::saveSession()
{
    if (quitting)
        return;
    QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));
    settings.setValue(QLatin1String("restoring"), false);
    settings.endGroup();

    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return;

    clean();

    settings.beginGroup(QLatin1String("sessions"));

    int version = 2;

    QByteArray data;
    QBuffer buffer(&data);
    QDataStream stream(&buffer);
    buffer.open(QIODevice::WriteOnly);

    stream << qint32(BrowserApplicationMagic);
    stream << qint32(version);

    stream << qint32(m_mainWindows.count());
    for (int i = 0; i < m_mainWindows.count(); ++i)
        stream << m_mainWindows.at(i)->saveState();
    settings.setValue(QLatin1String("lastSession"), data);
    settings.endGroup();
}

bool BrowserApplication::canRestoreSession() const
{
    return !m_lastSession.isEmpty();
}

bool BrowserApplication::restoreLastSession()
{
    {
        QSettings settings;
        settings.beginGroup(QLatin1String("MainWindow"));
        if (settings.value(QLatin1String("restoring"), false).toBool()) {
            QMessageBox::StandardButton result = QMessageBox::question(0, tr("Restore failed"),
                tr("Arora crashed while trying to restore this session.  Should I try again?"), QMessageBox::Yes | QMessageBox::No);
            if (result == QMessageBox::No)
                return false;
        }
        // saveSession will be called by an AutoSaver timer from the set tabs
        // and in saveSession we will reset this flag back to false
        settings.setValue(QLatin1String("restoring"), true);
    }
    int version = 2;
    QList<QByteArray> windows;
    QBuffer buffer(&m_lastSession);
    QDataStream stream(&buffer);
    buffer.open(QIODevice::ReadOnly);

    qint32 marker;
    qint32 v;
    stream >> marker;
    stream >> v;
    if (marker != BrowserApplicationMagic || v != version)
        return false;

    qint32 windowCount;
    stream >> windowCount;
    for (qint32 i = 0; i < windowCount; ++i) {
        QByteArray windowState;
        stream >> windowState;
        windows.append(windowState);
    }
    for (int i = 0; i < windows.count(); ++i) {
        BrowserMainWindow *newWindow = 0;
        if (i == 0 && m_mainWindows.count() >= 1) {
            newWindow = mainWindow();
        } else {
            newWindow = newMainWindow();
        }
        newWindow->restoreState(windows.at(i));
    }
    return true;
}

#if defined(Q_WS_MAC)
bool BrowserApplication::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ApplicationActivate: {
        clean();
        if (!m_mainWindows.isEmpty()) {
            BrowserMainWindow *mw = mainWindow();
            if (mw && !mw->isMinimized()) {
                mainWindow()->show();
            }
            return true;
        }
    }
    case QEvent::FileOpen:
        if (!m_mainWindows.isEmpty()) {
            QString file = static_cast<QFileOpenEvent*>(event)->file();
            mainWindow()->tabWidget()->loadUrl(QUrl::fromLocalFile(file));
            return true;
        }
    default:
        break;
    }
    return QApplication::event(event);
}
#endif

void BrowserApplication::askDesktopToOpenUrl(const QUrl &url)
{
    m_lastAskedUrl = url;
    m_lastAskedUrlDateTime = QDateTime::currentDateTime();
    QDesktopServices::openUrl(url);
}

void BrowserApplication::openUrl(const QUrl &url)
{
    setEventMouseButtons(mouseButtons());
    setEventKeyboardModifiers(keyboardModifiers());
    mainWindow()->tabWidget()->loadUrl(url);
}

BrowserMainWindow *BrowserApplication::newMainWindow()
{
    if (!m_mainWindows.isEmpty())
        mainWindow()->m_autoSaver->saveIfNeccessary();
    BrowserMainWindow *browser = new BrowserMainWindow();
    m_mainWindows.prepend(browser);
    connect(this, SIGNAL(privacyChanged(bool)),
            browser, SLOT(privacyChanged(bool)));
    browser->show();
    return browser;
}

BrowserMainWindow *BrowserApplication::mainWindow()
{
    clean();

    BrowserMainWindow *activeWindow = 0;

    if (m_mainWindows.isEmpty()) {
        activeWindow = newMainWindow();
    } else {
        activeWindow = qobject_cast<BrowserMainWindow*>(QApplication::activeWindow());
        if (!activeWindow)
            activeWindow = m_mainWindows[0];
    }

    return activeWindow;
}

CookieJar *BrowserApplication::cookieJar()
{
    return (CookieJar*)networkAccessManager()->cookieJar();
}

DownloadManager *BrowserApplication::downloadManager()
{
    if (!s_downloadManager)
        s_downloadManager = new DownloadManager();
    return s_downloadManager;
}

NetworkAccessManager *BrowserApplication::networkAccessManager()
{
    if (!s_networkAccessManager)
        s_networkAccessManager = new NetworkAccessManager();
    return s_networkAccessManager;
}

HistoryManager *BrowserApplication::historyManager()
{
    if (!s_historyManager)
        s_historyManager = new HistoryManager();
    return s_historyManager;
}

BookmarksManager *BrowserApplication::bookmarksManager()
{
    if (!s_bookmarksManager)
        s_bookmarksManager = new BookmarksManager;
    return s_bookmarksManager;
}

LanguageManager *BrowserApplication::languageManager()
{
    if (!s_languageManager) {
        s_languageManager = new LanguageManager();
        s_languageManager->addLocaleDirectory(dataFilePath(QLatin1String("locale")));
        s_languageManager->addLocaleDirectory(qApp->applicationDirPath() + QLatin1String("/src/.qm/locale"));
        s_languageManager->addLocaleDirectory(installedDataDirectory() + QLatin1String("/locale"));
        s_languageManager->loadLanguageFromSettings();
        connect(s_languageManager, SIGNAL(languageChanged(const QString &)),
                qApp, SLOT(retranslate()));
    }
    return s_languageManager;
}

AutoFillManager *BrowserApplication::autoFillManager()
{
    if (!s_autoFillManager) {
        s_autoFillManager = new AutoFillManager;
    }
    return s_autoFillManager;
}

QIcon BrowserApplication::icon(const QUrl &url)
{
    QIcon icon = QWebSettings::iconForUrl(url);
    if (!icon.isNull())
        return icon.pixmap(16, 16);
    if (icon.isNull()) {
        QPixmap pixmap = QWebSettings::webGraphic(QWebSettings::DefaultFrameIconGraphic);
        if (pixmap.isNull()) {
            pixmap = QPixmap(QLatin1String(":graphics/defaulticon.png"));
            QWebSettings::setWebGraphic(QWebSettings::DefaultFrameIconGraphic, pixmap);
        }
        return pixmap;
    }
    return icon;
}

QString BrowserApplication::installedDataDirectory()
{
#if defined(Q_WS_X11)
    return QLatin1String(PKGDATADIR);
#else
    return qApp->applicationDirPath();
#endif
}

QString BrowserApplication::dataFilePath(const QString &fileName)
{
    QString directory = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/data/Arora";
    if (directory.isEmpty())
        directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
    if (!QFile::exists(directory)) {
        QDir dir;
        dir.mkpath(directory);
    }
    return directory + QLatin1String("/") + fileName;
}

bool BrowserApplication::zoomTextOnly()
{
    return QWebSettings::globalSettings()->testAttribute(QWebSettings::ZoomTextOnly);
}

void BrowserApplication::setZoomTextOnly(bool textOnly)
{
    QWebSettings::globalSettings()->setAttribute(QWebSettings::ZoomTextOnly, textOnly);
    emit instance()->zoomTextOnlyChanged(textOnly);
}

bool BrowserApplication::isPrivate()
{
    return QWebSettings::globalSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled);
}

void BrowserApplication::setPrivate(bool isPrivate)
{
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, isPrivate);
    emit instance()->privacyChanged(isPrivate);
}

Qt::MouseButtons BrowserApplication::eventMouseButtons() const
{
    return m_eventMouseButtons;
}

Qt::KeyboardModifiers BrowserApplication::eventKeyboardModifiers() const
{
    return m_eventKeyboardModifiers;
}

void BrowserApplication::setEventMouseButtons(Qt::MouseButtons buttons)
{
    m_eventMouseButtons = buttons;
}

void BrowserApplication::setEventKeyboardModifiers(Qt::KeyboardModifiers modifiers)
{
    m_eventKeyboardModifiers = modifiers;
}

