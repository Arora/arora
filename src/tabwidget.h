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

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <qtabwidget.h>

#include <qwebpage.h>
#include <qurl.h>

QT_BEGIN_NAMESPACE
class QCompleter;
class QLabel;
class QLineEdit;
class QMenu;
class QStackedWidget;
QT_END_NAMESPACE

class BrowserMainWindow;
class NewTabButton;
class TabBar;
class WebView;
class WebActionMapper;
class WebViewSearch;
class QToolButton;

/*!
    TabWidget that contains WebViews and a stack widget of associated line edits.

    Connects up the current tab's signals to this class's signal and uses WebActionMapper
    to proxy the actions.
 */
class TabWidget : public QTabWidget
{
    Q_OBJECT

signals:
    // tab widget signals
    void tabsChanged();
    void lastTabClosed();

    // current tab signals
    void setCurrentTitle(const QString &url);
    void showStatusBarMessage(const QString &message);
    void linkHovered(const QString &link);
    void loadProgress(int progress);
    void geometryChangeRequested(const QRect &geometry);
    void menuBarVisibilityChangeRequested(bool visible);
    void statusBarVisibilityChangeRequested(bool visible);
    void toolBarVisibilityChangeRequested(bool visible);
    void printRequested(QWebFrame *frame);

public:
    enum OpenUrlIn {
        NewWindow,
        NewSelectedTab,
        NewNotSelectedTab,
        CurrentTab,
        UserOrCurrent,
        NewTab = NewNotSelectedTab
    };

    TabWidget(QWidget *parent = 0);

    void loadSettings();
    TabBar *tabBar() { return m_tabBar; }
    void clear();
    void addWebAction(QAction *action, QWebPage::WebAction webAction);

    QAction *newTabAction() const;
    QAction *closeTabAction() const;
    QAction *bookmarkTabsAction() const;
    QAction *recentlyClosedTabsAction() const;
    QAction *nextTabAction() const;
    QAction *previousTabAction() const;

    QWidget *locationBarStack() const;
    QLineEdit *currentLocationBar() const;
    WebView *currentWebView() const;
    WebView *webView(int index) const;
    WebViewSearch *webViewSearch(int index) const;
    QLineEdit *locationBar(int index) const;
    int webViewIndex(WebView *webView) const;
    WebView *makeNewTab(bool makeCurrent = false);

    QByteArray saveState() const;
    bool restoreState(const QByteArray &state);

    static OpenUrlIn modifyWithUserBehavior(OpenUrlIn tab);
    WebView *getView(OpenUrlIn tab, WebView *currentView);

protected:
    void changeEvent(QEvent *event);

public slots:
    void loadString(const QString &string, OpenUrlIn tab = CurrentTab);
    void loadUrlFromUser(const QUrl &url, const QString &title = QString());
    void loadUrl(const QUrl &url, TabWidget::OpenUrlIn tab = CurrentTab, const QString &title = QString());
    void createTab(const QByteArray &historyState, TabWidget::OpenUrlIn tab = CurrentTab);
    void newTab();
    void newTabFromClipboard();
    void cloneTab(int index = -1);
    void closeTab(int index = -1);
    void closeOtherTabs(int index);
    void reloadTab(int index = -1);
    void reloadAllTabs();
    void nextTab();
    void previousTab();
    void bookmarkTabs();

private slots:
    void currentChanged(int index);
    void openLastTab();
    void aboutToShowRecentTabsMenu();
    void aboutToShowRecentTriggeredAction(QAction *action);
    void webViewLoadStarted();
    void webViewLoadProgress(int progress);
    void webViewLoadFinished(bool ok);
    void webViewIconChanged();
    void webViewTitleChanged(const QString &title);
    void webViewUrlChanged(const QUrl &url);
    void lineEditReturnPressed();
    void windowCloseRequested();
    void moveTab(int fromIndex, int toIndex);
    void geometryChangeRequestedCheck(const QRect &geometry);
    void menuBarVisibilityChangeRequestedCheck(bool visible);
    void statusBarVisibilityChangeRequestedCheck(bool visible);
    void toolBarVisibilityChangeRequestedCheck(bool visible);
    void historyCleared();

private:
    static QUrl guessUrlFromString(const QString &url);
    QLabel *animationLabel(int index, bool addMovie);
    void retranslate();

    QAction *m_recentlyClosedTabsAction;
    QAction *m_newTabAction;
    QAction *m_closeTabAction;
    QAction *m_bookmarkTabsAction;
    QAction *m_nextTabAction;
    QAction *m_previousTabAction;

    QMenu *m_recentlyClosedTabsMenu;
    static const int m_recentlyClosedTabsSize = 10;
    QList<QUrl> m_recentlyClosedTabs;
    QList<QByteArray> m_recentlyClosedTabsHistory;
    QList<WebActionMapper*> m_actions;

    QCompleter *m_lineEditCompleter;
    QStackedWidget *m_locationBars;
    TabBar *m_tabBar;
    NewTabButton *addTabButton;
    QToolButton *closeTabButton;
};

#endif // TABWIDGET_H

