/*
 * Copyright 2008-2009 Benjamin C. Meyer <ben@meyerhome.net>
 * Copyright 2008 Ariya Hidayat <ariya.hidayat@gmail.com>
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

#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <qwebview.h>

#include "tabwidget.h"

#include <qwebelement.h>
class QLabel;


class BrowserMainWindow;
class TabWidget;
class WebPage;
class WebView : public QWebView
{
    Q_OBJECT

public:
    WebView(QWidget *parent = 0);
    WebPage *webPage() const { return m_page; }

    void loadSettings();

    void keyReleaseEvent(QKeyEvent *event);
    void focusOutEvent(QFocusEvent *event);

    void loadUrl(const QUrl &url, const QString &title = QString());
    QUrl url() const;

    QString lastStatusBarText() const;
    inline int progress() const { return m_progress; }
    TabWidget *tabWidget() const;

signals:
    void search(const QUrl &searchUrl, TabWidget::OpenUrlIn openIn);

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void applyZoom();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    int levelForZoom(int zoom);

private slots:
    void setProgress(int progress);
    void loadFinished();
    void setStatusBarText(const QString &string);
    void downloadRequested(const QNetworkRequest &request);
    void openActionUrlInNewTab();
    void openActionUrlInNewWindow();
    void downloadLinkToDisk();
    void copyLinkToClipboard();
    void openImageInNewWindow();
    void downloadImageToDisk();
    void copyImageToClipboard();
    void copyImageLocationToClipboard();
    void blockImage();
    void bookmarkLink();
    void searchRequested(QAction *action);
    void addSearchEngine();
    void hideAccessKeys();
    void accessKeyShortcut();

private:
    QString m_statusBarText;
    QUrl m_initialUrl;
    int m_progress;
    int m_currentZoom;
    QList<int> m_zoomLevels;
    WebPage *m_page;

    bool m_enableAccessKeys;
    bool checkForAccessKey(QKeyEvent *event);
    void showAccessKeys();
    void makeAccessKeyLabel(const QChar &accessKey, const QWebElement &element);
    QList<QLabel*> m_accessKeyLabels;
    QHash<QChar, QWebElement> m_accessKeyNodes;
    bool m_accessKeysPressed;
};

#endif

