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

#ifndef BROWSERAPPLICATION_H
#define BROWSERAPPLICATION_H

#include "singleapplication.h"

#include <qpointer.h>
#include <qurl.h>
#include <qdatetime.h>

class AutoFillManager;
class BookmarksManager;
class BrowserMainWindow;
class CookieJar;
class DownloadManager;
class HistoryManager;
class NetworkAccessManager;
class LanguageManager;
class QLocalSocket;
class BrowserApplication : public SingleApplication
{
    Q_OBJECT

public:
    BrowserApplication(int &argc, char **argv);
    ~BrowserApplication();
    static BrowserApplication *instance();
    void loadSettings();

    BrowserMainWindow *mainWindow();
    QList<BrowserMainWindow*> mainWindows();
    bool allowToCloseWindow(BrowserMainWindow *window);

    static QIcon icon(const QUrl &url);

    void saveSession();
    bool canRestoreSession() const;

    static HistoryManager *historyManager();
    static CookieJar *cookieJar();
    static DownloadManager *downloadManager();
    static NetworkAccessManager *networkAccessManager();
    static BookmarksManager *bookmarksManager();
    static LanguageManager *languageManager();
    static AutoFillManager *autoFillManager();

    static QString installedDataDirectory();
    static QString dataFilePath(const QString &fileName);

    Qt::MouseButtons eventMouseButtons() const;
    Qt::KeyboardModifiers eventKeyboardModifiers() const;
    void setEventMouseButtons(Qt::MouseButtons buttons);
    void setEventKeyboardModifiers(Qt::KeyboardModifiers modifiers);

    static bool zoomTextOnly();

    static bool isPrivate();
    static void setPrivate(bool isPrivate);


public slots:
    BrowserMainWindow *newMainWindow();
    bool restoreLastSession();
    void quitBrowser();

    static void setZoomTextOnly(bool textOnly);

    void askDesktopToOpenUrl(const QUrl &url);

private slots:
    void retranslate();
    void messageReceived(QLocalSocket *socket);
    void postLaunch();
    void openUrl(const QUrl &url);

signals:
    void zoomTextOnlyChanged(bool textOnly);
    void privacyChanged(bool isPrivate);

private:
    QString parseArgumentUrl(const QString &string) const;
    void clean();

    static HistoryManager *s_historyManager;
    static DownloadManager *s_downloadManager;
    static NetworkAccessManager *s_networkAccessManager;
    static BookmarksManager *s_bookmarksManager;
    static LanguageManager *s_languageManager;
    static AutoFillManager *s_autoFillManager;

    QList<QPointer<BrowserMainWindow> > m_mainWindows;
    QByteArray m_lastSession;
    bool quitting;

    Qt::MouseButtons m_eventMouseButtons;
    Qt::KeyboardModifiers m_eventKeyboardModifiers;

    QUrl m_lastAskedUrl;
    QDateTime m_lastAskedUrlDateTime;
};

#endif // BROWSERAPPLICATION_H

