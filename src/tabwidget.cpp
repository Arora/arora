/*
 * Copyright 2008 Benjamin C. Meyer <ben@meyerhome.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the iQ_WS_X11mplied warranty of
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

#include "tabwidget.h"

#include "addbookmarkdialog.h"
#include "bookmarknode.h"
#include "bookmarksmanager.h"
#include "bookmarksmodel.h"
#include "browserapplication.h"
#include "browsermainwindow.h"
#include "history.h"
#include "historycompleter.h"
#include "historymanager.h"
#include "locationbar.h"
#include "opensearchengine.h"
#include "opensearchmanager.h"
#include "tabbar.h"
#include "toolbarsearch.h"
#include "webactionmapper.h"
#include "webpage.h"
#include "webview.h"
#include "webviewsearch.h"

#include <qcompleter.h>
#include <qdir.h>
#include <qevent.h>
#include <qlistview.h>
#include <qmenu.h>
#include <qmessagebox.h>
#include <qmovie.h>
#include <qsettings.h>
#include <qstackedwidget.h>
#include <qstyle.h>
#include <qtoolbutton.h>
#include <qwebhistory.h>

#include <qdebug.h>

//#define USERMODIFIEDBEHAVIOR_DEBUG

TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent)
    , m_recentlyClosedTabsAction(0)
    , m_newTabAction(0)
    , m_closeTabAction(0)
    , m_bookmarkTabsAction(0)
    , m_nextTabAction(0)
    , m_previousTabAction(0)
    , m_recentlyClosedTabsMenu(0)
    , m_lineEditCompleter(0)
    , m_locationBars(0)
    , m_tabBar(new TabBar(this))
    , addTabButton(0)
    , closeTabButton(0)
{
    setElideMode(Qt::ElideRight);

    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T), this, SLOT(openLastTab()));
    new QShortcut(QKeySequence::Undo, this, SLOT(openLastTab()));

    connect(m_tabBar, SIGNAL(loadUrl(const QUrl&, TabWidget::OpenUrlIn)),
            this, SLOT(loadUrl(const QUrl&, TabWidget::OpenUrlIn)));
    connect(m_tabBar, SIGNAL(newTab()), this, SLOT(newTab()));
    connect(m_tabBar, SIGNAL(closeTab(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(cloneTab(int)), this, SLOT(cloneTab(int)));
    connect(m_tabBar, SIGNAL(closeOtherTabs(int)), this, SLOT(closeOtherTabs(int)));
    connect(m_tabBar, SIGNAL(reloadTab(int)), this, SLOT(reloadTab(int)));
    connect(m_tabBar, SIGNAL(reloadAllTabs()), this, SLOT(reloadAllTabs()));
    setTabBar(m_tabBar);
    setDocumentMode(true);
    connect(m_tabBar, SIGNAL(tabMoved(int, int)),
            this, SLOT(moveTab(int, int)));

    // Actions
    m_newTabAction = new QAction(this);
    m_newTabAction->setShortcuts(QKeySequence::AddTab);
    connect(m_newTabAction, SIGNAL(triggered()), this, SLOT(newTab()));

    m_closeTabAction = new QAction(this);
    m_closeTabAction->setShortcuts(QKeySequence::Close);
    m_closeTabAction->setIcon(QIcon(QLatin1String(":graphics/closetab.png")));
#if QT_VERSION < 0x040600 || (QT_VERSION >= 0x040600 && !defined(Q_WS_X11))
    m_closeTabAction->setIconVisibleInMenu(false);
#endif
    connect(m_closeTabAction, SIGNAL(triggered()), this, SLOT(closeTab()));

    m_bookmarkTabsAction = new QAction(this);
    connect(m_bookmarkTabsAction, SIGNAL(triggered()), this, SLOT(bookmarkTabs()));

    m_newTabAction->setIcon(QIcon(QLatin1String(":graphics/addtab.png")));
#if QT_VERSION < 0x040600 || (QT_VERSION >= 0x040600 && !defined(Q_WS_X11))
    m_newTabAction->setIconVisibleInMenu(false);
#endif

    m_nextTabAction = new QAction(this);
    connect(m_nextTabAction, SIGNAL(triggered()), this, SLOT(nextTab()));

    m_previousTabAction = new QAction(this);
    connect(m_previousTabAction, SIGNAL(triggered()), this, SLOT(previousTab()));
#if QT_VERSION >= 0x040600 && defined(Q_WS_X11)
    m_previousTabAction->setIcon(QIcon::fromTheme(QLatin1String("go-previous")));
    m_nextTabAction->setIcon(QIcon::fromTheme(QLatin1String("go-next")));
#endif

    m_recentlyClosedTabsMenu = new QMenu(this);
    connect(m_recentlyClosedTabsMenu, SIGNAL(aboutToShow()),
            this, SLOT(aboutToShowRecentTabsMenu()));
    connect(m_recentlyClosedTabsMenu, SIGNAL(triggered(QAction *)),
            this, SLOT(aboutToShowRecentTriggeredAction(QAction *)));
    m_recentlyClosedTabsAction = new QAction(this);
    m_recentlyClosedTabsAction->setMenu(m_recentlyClosedTabsMenu);
    m_recentlyClosedTabsAction->setEnabled(false);

#ifndef Q_WS_MAC // can't seem to figure out the background color :(
    addTabButton = new QToolButton(this);
    addTabButton->setDefaultAction(m_newTabAction);
    addTabButton->setAutoRaise(true);
    addTabButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
#endif

    connect(m_tabBar, SIGNAL(tabCloseRequested(int)),
            this, SLOT(closeTab(int)));
    connect(this, SIGNAL(currentChanged(int)),
            this, SLOT(currentChanged(int)));

    m_locationBars = new QStackedWidget(this);

    connect(BrowserApplication::historyManager(), SIGNAL(historyCleared()),
        this, SLOT(historyCleared()));

    // Initialize Actions' labels
    retranslate();
    loadSettings();
}

void TabWidget::historyCleared()
{
    m_recentlyClosedTabs.clear();
    m_recentlyClosedTabsAction->setEnabled(false);
}

void TabWidget::clear()
{
    // clear the recently closed tabs
    m_recentlyClosedTabs.clear();
    m_recentlyClosedTabsAction->setEnabled(false);
    // clear the line edit history
    for (int i = 0; i < m_locationBars->count(); ++i) {
        QLineEdit *qLineEdit = locationBar(i);
        qLineEdit->setText(qLineEdit->text());
        webViewSearch(i)->clear();
    }
}

// When index is -1 index chooses the current tab
void TabWidget::reloadTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    if (WebView *tab = webView(index)) {
        tab->reload();
    }
}

void TabWidget::moveTab(int fromIndex, int toIndex)
{
    QWidget *lineEdit = m_locationBars->widget(fromIndex);
    m_locationBars->removeWidget(lineEdit);
    m_locationBars->insertWidget(toIndex, lineEdit);
}

void TabWidget::addWebAction(QAction *action, QWebPage::WebAction webAction)
{
    if (!action)
        return;
    m_actions.append(new WebActionMapper(action, webAction, this));
}

void TabWidget::currentChanged(int index)
{
    WebView *webView = this->webView(index);
    if (!webView)
        return;

    Q_ASSERT(m_locationBars->count() == count());

    WebView *oldWebView = this->webView(m_locationBars->currentIndex());
    if (oldWebView) {
        disconnect(oldWebView, SIGNAL(statusBarMessage(const QString&)),
                   this, SIGNAL(showStatusBarMessage(const QString&)));
        disconnect(oldWebView->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)),
                   this, SIGNAL(linkHovered(const QString&)));
        disconnect(oldWebView, SIGNAL(loadProgress(int)),
                   this, SIGNAL(loadProgress(int)));
    }

    connect(webView, SIGNAL(statusBarMessage(const QString&)),
            this, SIGNAL(showStatusBarMessage(const QString&)));
    connect(webView->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)),
            this, SIGNAL(linkHovered(const QString&)));
    connect(webView, SIGNAL(loadProgress(int)),
            this, SIGNAL(loadProgress(int)));

    for (int i = 0; i < m_actions.count(); ++i) {
        WebActionMapper *mapper = m_actions[i];
        mapper->updateCurrent(webView->page());
    }
    emit setCurrentTitle(webView->title());
    m_locationBars->setCurrentIndex(index);
    emit loadProgress(webView->progress());
    emit showStatusBarMessage(webView->lastStatusBarText());
    if (webView->url().isEmpty() && webView->hasFocus()) {
        m_locationBars->currentWidget()->setFocus();
    } else if (!webView->url().isEmpty()) {
        webView->setFocus();
    }
}

QAction *TabWidget::newTabAction() const
{
    return m_newTabAction;
}

QAction *TabWidget::closeTabAction() const
{
    return m_closeTabAction;
}

QAction *TabWidget::bookmarkTabsAction() const
{
    return m_bookmarkTabsAction;
}

QAction *TabWidget::recentlyClosedTabsAction() const
{
    return m_recentlyClosedTabsAction;
}

QAction *TabWidget::nextTabAction() const
{
    return m_nextTabAction;
}

QAction *TabWidget::previousTabAction() const
{
    return m_previousTabAction;
}

QWidget *TabWidget::locationBarStack() const
{
    return m_locationBars;
}

QLineEdit *TabWidget::currentLocationBar() const
{
    return locationBar(m_locationBars->currentIndex());
}

WebView *TabWidget::currentWebView() const
{
    return webView(currentIndex());
}

QLineEdit *TabWidget::locationBar(int index) const
{
    return qobject_cast<LocationBar*>(m_locationBars->widget(index));
}

WebView *TabWidget::webView(int index) const
{
    QWidget *widget = this->widget(index);
    if (WebViewWithSearch *webViewWithSearch = qobject_cast<WebViewWithSearch*>(widget)) {
        return webViewWithSearch->m_webView;
    }
    return 0;
}

WebViewSearch *TabWidget::webViewSearch(int index) const
{
    QWidget *widget = this->widget(index);
    if (WebViewWithSearch *webViewWithSearch = qobject_cast<WebViewWithSearch*>(widget)) {
        return webViewWithSearch->m_webViewSearch;
    }
    return 0;
}

int TabWidget::webViewIndex(WebView *webView) const
{
    for (int i = 0; i < count(); ++i) {
        QWidget *widget = this->widget(i);
        if (WebViewWithSearch *webViewWithSearch = qobject_cast<WebViewWithSearch*>(widget)) {
            if (webViewWithSearch->m_webView == webView)
                return i;
        }
    }
    return -1;
}

void TabWidget::newTab()
{
    makeNewTab(true);
}

WebView *TabWidget::makeNewTab(bool makeCurrent)
{
    // line edit
    LocationBar *locationBar = new LocationBar;
    if (!m_lineEditCompleter) {
        HistoryCompletionModel *completionModel = new HistoryCompletionModel(this);
        completionModel->setSourceModel(BrowserApplication::historyManager()->historyFilterModel());
        m_lineEditCompleter = new HistoryCompleter(completionModel, this);
        connect(m_lineEditCompleter, SIGNAL(activated(const QString &)),
                this, SLOT(loadString(const QString &)));
        // Should this be in Qt by default?
        QAbstractItemView *popup = m_lineEditCompleter->popup();
        QListView *listView = qobject_cast<QListView*>(popup);
        if (listView) {
            // Urls are always LeftToRight
            listView->setLayoutDirection(Qt::LeftToRight);
            listView->setUniformItemSizes(true);
        }
    }
    locationBar->setCompleter(m_lineEditCompleter);
    connect(locationBar, SIGNAL(returnPressed()), this, SLOT(lineEditReturnPressed()));
    m_locationBars->addWidget(locationBar);
    m_locationBars->setSizePolicy(locationBar->sizePolicy());

#ifndef AUTOTESTS
    QWidget::setTabOrder(locationBar, BrowserMainWindow::parentWindow(this)->findChild<ToolbarSearch*>());
#endif

    // webview
    WebView *webView = new WebView;
    locationBar->setWebView(webView);
    connect(webView, SIGNAL(loadStarted()),
            this, SLOT(webViewLoadStarted()));
    connect(webView, SIGNAL(loadProgress(int)),
                this, SLOT(webViewLoadProgress(int)));
    connect(webView, SIGNAL(loadFinished(bool)),
            this, SLOT(webViewLoadFinished(bool)));
    connect(webView, SIGNAL(iconChanged()),
            this, SLOT(webViewIconChanged()));
    connect(webView, SIGNAL(titleChanged(const QString &)),
            this, SLOT(webViewTitleChanged(const QString &)));
    connect(webView, SIGNAL(urlChanged(const QUrl &)),
            this, SLOT(webViewUrlChanged(const QUrl &)));
    connect(webView, SIGNAL(search(const QUrl&, TabWidget::OpenUrlIn)),
            this, SLOT(loadUrl(const QUrl&, TabWidget::OpenUrlIn)));
    connect(webView->page(), SIGNAL(windowCloseRequested()),
            this, SLOT(windowCloseRequested()));
    connect(webView->page(), SIGNAL(printRequested(QWebFrame *)),
            this, SIGNAL(printRequested(QWebFrame *)));
    connect(webView->page(), SIGNAL(geometryChangeRequested(const QRect &)),
            this, SLOT(geometryChangeRequestedCheck(const QRect &)));
    connect(webView->page(), SIGNAL(menuBarVisibilityChangeRequested(bool)),
            this, SLOT(menuBarVisibilityChangeRequestedCheck(bool)));
    connect(webView->page(), SIGNAL(statusBarVisibilityChangeRequested(bool)),
            this, SLOT(statusBarVisibilityChangeRequestedCheck(bool)));
    connect(webView->page(), SIGNAL(toolBarVisibilityChangeRequested(bool)),
            this, SLOT(toolBarVisibilityChangeRequestedCheck(bool)));

    WebViewWithSearch *webViewWithSearch = new WebViewWithSearch(webView, this);
    addTab(webViewWithSearch, tr("Untitled"));
    if (makeCurrent)
        setCurrentWidget(webViewWithSearch);

    // webview actions
    for (int i = 0; i < m_actions.count(); ++i) {
        WebActionMapper *mapper = m_actions[i];
        mapper->addChild(webView->page()->action(mapper->webAction()));
    }

    if (count() == 1)
        currentChanged(currentIndex());
    emit tabsChanged();
    return webView;
}

void TabWidget::geometryChangeRequestedCheck(const QRect &geometry)
{
    if (count() == 1)
        emit geometryChangeRequested(geometry);
}

void TabWidget::menuBarVisibilityChangeRequestedCheck(bool visible)
{
    if (count() == 1)
        emit menuBarVisibilityChangeRequested(visible);
}

void TabWidget::statusBarVisibilityChangeRequestedCheck(bool visible)
{
    if (count() == 1)
        emit statusBarVisibilityChangeRequested(visible);
}

void TabWidget::toolBarVisibilityChangeRequestedCheck(bool visible)
{
    if (count() == 1)
        emit toolBarVisibilityChangeRequested(visible);
}

void TabWidget::reloadAllTabs()
{
    for (int i = 0; i < count(); ++i) {
        if (WebView *tab = webView(i)) {
            tab->reload();
        }
    }
}

void TabWidget::bookmarkTabs()
{
    AddBookmarkDialog dialog;
    dialog.setFolder(true);
    dialog.setTitle(tr("Saved Tabs"));
    dialog.exec();

    BookmarkNode *folder = dialog.addedNode();
    if (!folder)
        return;

    for (int i = 0; i < count(); ++i) {
        WebView *tab = webView(i);
        if (!tab)
            continue;

        QString title = tab->title();
        QString url = QString::fromUtf8(tab->url().toEncoded());
        BookmarkNode *bookmark = new BookmarkNode(BookmarkNode::Bookmark);
        bookmark->url = url;
        bookmark->title = title;
        BrowserApplication::bookmarksManager()->addBookmark(folder, bookmark);
    }
}

void TabWidget::lineEditReturnPressed()
{
    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(sender())) {
        OpenUrlIn tab = CurrentTab;
        if (qApp->keyboardModifiers() == Qt::AltModifier)
            tab = NewSelectedTab;

        loadString(lineEdit->text(), tab);
        if (m_locationBars->currentWidget() == lineEdit)
            currentWebView()->setFocus();
    }
}

void TabWidget::windowCloseRequested()
{
    WebPage *webPage = qobject_cast<WebPage*>(sender());
    if (!webPage)
        return;
    WebView *webView = qobject_cast<WebView*>(webPage->view());
    int index = webViewIndex(webView);
    if (index >= 0) {
        if (count() == 1)
            BrowserMainWindow::parentWindow(this)->close();
        else
            closeTab(index);
    }
}

void TabWidget::closeOtherTabs(int index)
{
    if (-1 == index)
        return;
    for (int i = count() - 1; i > index; --i)
        closeTab(i);
    for (int i = index - 1; i >= 0; --i)
        closeTab(i);
}

// When index is -1 index chooses the current tab
void TabWidget::cloneTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;
    QUrl url = webView(index)->url();
    WebView *tab = makeNewTab();
    tab->loadUrl(url);
}

// When index is -1 index chooses the current tab
void TabWidget::closeTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    bool hasFocus = false;
    WebView *tab = webView(index);

    if (tab && !tab->url().isEmpty()) {
        if (tab->isModified()) {
            QMessageBox closeConfirmation(tab);
            closeConfirmation.setWindowFlags(Qt::Sheet);
            closeConfirmation.setWindowTitle(tr("Do you really want to close this page?"));
            closeConfirmation.setInformativeText(tr("You have modified this page and when closing it you would lose the modification.\n"
                                                    "Do you really want to close this page?\n"));
            closeConfirmation.setIcon(QMessageBox::Question);
            closeConfirmation.addButton(QMessageBox::Yes);
            closeConfirmation.addButton(QMessageBox::No);
            closeConfirmation.setEscapeButton(QMessageBox::No);
            if (closeConfirmation.exec() == QMessageBox::No)
                return;
        }
        hasFocus = tab->hasFocus();

        m_recentlyClosedTabsAction->setEnabled(true);
        m_recentlyClosedTabs.prepend(tab->url());
#if QT_VERSION >= 0x040600
        QByteArray tabHistory;
        QDataStream tabHistoryStream(&tabHistory, QIODevice::WriteOnly);
        tabHistoryStream << *tab->history();
        m_recentlyClosedTabsHistory.prepend(tabHistory);
#else
        m_recentlyClosedTabsHistory.prepend(QByteArray());
#endif
        if (m_recentlyClosedTabs.size() >= TabWidget::m_recentlyClosedTabsSize)
            m_recentlyClosedTabs.removeLast();
    }
    QWidget *lineEdit = m_locationBars->widget(index);
    m_locationBars->removeWidget(lineEdit);
    lineEdit->deleteLater();

    QWidget *webViewWithSearch = widget(index);
    removeTab(index);
    webViewWithSearch->setParent(0);
    webViewWithSearch->deleteLater();

    emit tabsChanged();
    if (hasFocus && count() > 0 && currentWebView())
        currentWebView()->setFocus();
    if (count() == 0)
        emit lastTabClosed();
}

QLabel *TabWidget::animationLabel(int index, bool addMovie)
{
    if (-1 == index)
        return 0;
    QTabBar::ButtonPosition side = m_tabBar->freeSide();
    QLabel *loadingAnimation = qobject_cast<QLabel*>(m_tabBar->tabButton(index, side));
    if (!loadingAnimation) {
        loadingAnimation = new QLabel(this);
    }
    if (addMovie && !loadingAnimation->movie()) {
        QMovie *movie = new QMovie(QLatin1String(":graphics/loading.gif"), QByteArray(), loadingAnimation);
        movie->setSpeed(50);
        loadingAnimation->setMovie(movie);
        movie->start();
    }
    m_tabBar->setTabButton(index, side, 0);
    m_tabBar->setTabButton(index, side, loadingAnimation);
    return loadingAnimation;
}

void TabWidget::webViewLoadStarted()
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) {
        QLabel *label = animationLabel(index, true);
        if (label->movie())
            label->movie()->start();
    }

    if (index != currentIndex())
        return;

    emit showStatusBarMessage(tr("Loading..."));
}

void TabWidget::webViewLoadProgress(int progress)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);

    if (index != currentIndex()
        || index < 0)
        return;

    double totalBytes = (double) webView->webPage()->totalBytes() / 1024;

    QString message = tr("Loading %1% (%2 %3)...").arg(progress).arg(totalBytes, 0, 'f', 2).arg(QLatin1String("kB"));
    emit showStatusBarMessage(message);
}

void TabWidget::webViewLoadFinished(bool ok)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);

    if (-1 != index) {
        QLabel *label = animationLabel(index, true);
        if (label->movie())
            label->movie()->stop();
#if defined(Q_WS_MAC)
        QTabBar::ButtonPosition side = m_tabBar->freeSide();
        m_tabBar->setTabButton(index, side, 0);
        delete label;
#endif
    }
    webViewIconChanged();

    if (index != currentIndex())
        return;

    if (ok)
        emit showStatusBarMessage(tr("Finished loading"));
    else
        emit showStatusBarMessage(tr("Failed to load"));
}

void TabWidget::webViewIconChanged()
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) {
#if !defined(Q_WS_MAC)
        QIcon icon = BrowserApplication::instance()->icon(webView->url());
        QLabel *label = animationLabel(index, false);
        QMovie *movie = label->movie();
        delete movie;
        label->setMovie(0);
        label->setPixmap(icon.pixmap(16, 16));
#endif
    }
}

void TabWidget::webViewTitleChanged(const QString &title)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 == index)
        return;
    QString tabTitle = title;
    if (title.isEmpty())
        tabTitle = QString::fromUtf8(webView->url().toEncoded());
    tabTitle.replace(QLatin1Char('&'), QLatin1String("&&"));
    setTabText(index, tabTitle);
    setTabToolTip(index, tabTitle);
    if (currentIndex() == index)
        emit setCurrentTitle(title);
    BrowserApplication::historyManager()->updateHistoryEntry(webView->url(), title);
}

void TabWidget::webViewUrlChanged(const QUrl &url)
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 == index)
        return;
    m_tabBar->setTabData(index, url);
    emit tabsChanged();
}

void TabWidget::openLastTab()
{
    if (m_recentlyClosedTabs.isEmpty())
        return;
    QUrl url = m_recentlyClosedTabs.takeFirst();
    QByteArray historyState = m_recentlyClosedTabsHistory.takeFirst();
#if QT_VERSION >= 0x040600
    createTab(historyState, NewTab);
#else
    loadUrl(url, NewTab);
#endif
    m_recentlyClosedTabsAction->setEnabled(!m_recentlyClosedTabs.isEmpty());
}

void TabWidget::aboutToShowRecentTabsMenu()
{
    m_recentlyClosedTabsMenu->clear();
    for (int i = 0; i < m_recentlyClosedTabs.count(); ++i) {
        QAction *action = new QAction(m_recentlyClosedTabsMenu);
#if QT_VERSION >= 0x040600
        action->setData(m_recentlyClosedTabsHistory.at(i));
#else
        action->setData(m_recentlyClosedTabs.at(i));
#endif
        QIcon icon = BrowserApplication::instance()->icon(m_recentlyClosedTabs.at(i));
        action->setIcon(icon);
        action->setText(m_recentlyClosedTabs.at(i).toString());
        m_recentlyClosedTabsMenu->addAction(action);
    }
}

void TabWidget::aboutToShowRecentTriggeredAction(QAction *action)
{
    if (!action)
        return;

#if QT_VERSION >= 0x040600
    QByteArray historyState = action->data().toByteArray();
    createTab(historyState, NewTab);
#else
    QUrl url = action->data().toUrl();
    loadUrl(url, NewTab);
#endif
}

void TabWidget::retranslate()
{
    m_nextTabAction->setText(tr("Show Next Tab"));
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BraceRight));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_PageDown));
    shortcuts.append(tr("Ctrl-]"));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Less));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Tab));
    m_nextTabAction->setShortcuts(shortcuts);
    m_previousTabAction->setText(tr("Show Previous Tab"));
    shortcuts.clear();
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BraceLeft));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_PageUp));
    shortcuts.append(tr("Ctrl-["));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Greater));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Tab));
    m_previousTabAction->setShortcuts(shortcuts);
    m_recentlyClosedTabsAction->setText(tr("Recently Closed Tabs"));
    m_newTabAction->setText(tr("New &Tab"));
    m_closeTabAction->setText(tr("&Close Tab"));
    m_bookmarkTabsAction->setText(tr("Bookmark All Tabs"));
    m_tabBar->updateViewToolBarAction();
}

void TabWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslate();
    QTabWidget::changeEvent(event);
}

/*
    Transform string into a QUrl and then load it.

    When you already have a QUrl call loadUrl()
 */
void TabWidget::loadString(const QString &string, OpenUrlIn tab)
{
    if (string.isEmpty())
        return;

    QUrl url = guessUrlFromString(string);
    loadUrl(url, tab);
}

QUrl TabWidget::guessUrlFromString(const QString &string)
{
    OpenSearchManager *manager = ToolbarSearch::openSearchManager();
    QUrl url = manager->convertKeywordSearchToUrl(string);
    if (url.isValid())
        return url;

#if QT_VERSION >= 0x040600
    url = QUrl::fromUserInput(string);
#else
    url = WebView::guessUrlFromString(string);
#endif

    if (url.scheme() == QLatin1String("about")
        && url.path() == QLatin1String("home"))
        url = QUrl(QLatin1String("qrc:/startpage.html"));

    // QUrl::isValid() is too much tolerant.
    // We actually want to check if the url conforms to the RFC, which QUrl::isValid() doesn't state.
    if (!url.scheme().isEmpty() && (!url.host().isEmpty() || !url.path().isEmpty()))
        return url;

    QSettings settings;
    settings.beginGroup(QLatin1String("urlloading"));
    bool search = settings.value(QLatin1String("searchEngineFallback"), false).toBool();

    if (search) {
        url = ToolbarSearch::openSearchManager()->currentEngine()->searchUrl(string.trimmed());
    } else {
        QString urlString = QLatin1String("http://") + string.trimmed();
        url = QUrl::fromEncoded(urlString.toUtf8(), QUrl::TolerantMode);
    }

    return url;
}

/*
   Somewhere in the browser interface a users wants to open a url

   By default open this url in the current tab, unless mouse or keyboard
   modifiers are set.
 */
void TabWidget::loadUrlFromUser(const QUrl &url, const QString &title)
{
    loadUrl(url, modifyWithUserBehavior(CurrentTab), title);
}

void TabWidget::loadSettings()
{
    for (int i = 0; i < count(); ++i) {
        WebView *v = webView(i);
        if (v && v->page())
            v->loadSettings();
    }

    QSettings settings;
    settings.beginGroup(QLatin1String("tabs"));
    bool newTabButtonInRightCorner = settings.value(QLatin1String("newTabButtonInRightCorner"), true).toBool();
#ifndef Q_WS_MAC
    setCornerWidget(addTabButton, newTabButtonInRightCorner ? Qt::TopRightCorner : Qt::TopLeftCorner);
    addTabButton->show();
#endif

    bool oneCloseButton = settings.value(QLatin1String("oneCloseButton"), false).toBool();
    if (oneCloseButton) {
        if (!closeTabButton) {
            closeTabButton = new QToolButton(this);
            closeTabButton->setDefaultAction(m_closeTabAction);
            closeTabButton->setAutoRaise(true);
            closeTabButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        }
        setCornerWidget(closeTabButton, newTabButtonInRightCorner ? Qt::TopLeftCorner : Qt::TopRightCorner);
        closeTabButton->setVisible(oneCloseButton);
    } else {
        setCornerWidget(0, newTabButtonInRightCorner ? Qt::TopLeftCorner : Qt::TopRightCorner);
    }
    m_tabBar->setTabsClosable(!oneCloseButton);
}

/*
    Replace the openIn behavior with the behavior the user wants.

    // ctrl open in new tab
    // ctrl-shift open in new tab and select
    // ctrl-alt open in new window
 */
TabWidget::OpenUrlIn TabWidget::modifyWithUserBehavior(OpenUrlIn tab) {
    Qt::KeyboardModifiers modifiers = BrowserApplication::instance()->eventKeyboardModifiers();
    Qt::MouseButtons buttons = BrowserApplication::instance()->eventMouseButtons();
#ifdef USERMODIFIEDBEHAVIOR_DEBUG
    qDebug() << __FUNCTION__ << "start" << modifiers << buttons << tab;
#endif
    if (modifiers & Qt::ControlModifier || buttons == Qt::MidButton) {
        if (modifiers & Qt::AltModifier) {
            tab = NewWindow;
        } else {
            QSettings settings;
            settings.beginGroup(QLatin1String("tabs"));
            bool select = settings.value(QLatin1String("selectNewTabs"), false).toBool();
            if (modifiers & Qt::ShiftModifier)
                tab = !select ? NewSelectedTab : NewNotSelectedTab;
            else
                tab = select ? NewSelectedTab : NewNotSelectedTab;
        }
    }
#ifdef USERMODIFIEDBEHAVIOR_DEBUG
    qDebug() << __FUNCTION__ << "end" << modifiers << buttons << tab;
#endif
    BrowserApplication::instance()->setEventKeyboardModifiers(0);
    BrowserApplication::instance()->setEventMouseButtons(Qt::NoButton);
    return tab;
}

/*
   Somewhere in the browser interface a users wants to open a url in a specific tab.
 */
void TabWidget::loadUrl(const QUrl &url, OpenUrlIn tab, const QString &title)
{
    if (tab == UserOrCurrent) {
        loadUrlFromUser(url, title);
        return;
    }
    if (!url.isValid())
        return;
    WebView *webView = getView(tab, currentWebView());
    if (webView) {
        int index = webViewIndex(webView);
        if (index != -1)
            locationBar(index)->setText(QString::fromUtf8(url.toEncoded()));
        webView->loadUrl(url, title);
    }
}

/*
    Return the view that matches the openIn behavior creating
    a new view/window if necessary.
 */
WebView *TabWidget::getView(OpenUrlIn tab, WebView *currentView)
{
    WebView *webView = 0;
    switch (tab) {
        case NewWindow: {
#ifdef USERMODIFIEDBEHAVIOR_DEBUG
            qDebug() << __FUNCTION__ << "NewWindow";
#endif
            BrowserMainWindow *newMainWindow = BrowserApplication::instance()->newMainWindow();
            webView = newMainWindow->currentTab();
            webView->setFocus();
            break;
        }

        case NewSelectedTab: {
#ifdef USERMODIFIEDBEHAVIOR_DEBUG
            qDebug() << __FUNCTION__ << "NewSelectedTab";
#endif
            webView = makeNewTab(true);
            webView->setFocus();
            break;
        }

        case NewNotSelectedTab: {
#ifdef USERMODIFIEDBEHAVIOR_DEBUG
            qDebug() << __FUNCTION__ << "NewNotSelectedTab";
#endif
            webView = makeNewTab(false);
            break;
        }

        case CurrentTab:
        default:
#ifdef USERMODIFIEDBEHAVIOR_DEBUG
            qDebug() << __FUNCTION__ << "CurrentTab";
#endif
            webView = currentView;
            if (!webView)
                return 0;
            webView->setFocus();
            break;
    }
    return webView;
}

void TabWidget::nextTab()
{
    int next = currentIndex() + 1;
    if (next == count())
        next = 0;
    setCurrentIndex(next);
}

void TabWidget::previousTab()
{
    int next = currentIndex() - 1;
    if (next < 0)
        next = count() - 1;
    setCurrentIndex(next);
}

static const qint32 TabWidgetMagic = 0xaa;

QByteArray TabWidget::saveState() const
{
    int version = 1;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << qint32(TabWidgetMagic);
    stream << qint32(version);

    QStringList tabs;
    QList<QByteArray> tabsHistory;
    for (int i = 0; i < count(); ++i) {
        if (WebView *tab = webView(i)) {
            tabs.append(QString::fromUtf8(tab->url().toEncoded()));
#if QT_VERSION >= 0x040600
            if (tab->history()->count() != 0) {
                QByteArray tabHistory;
                QDataStream tabHistoryStream(&tabHistory, QIODevice::WriteOnly);
                tabHistoryStream << *tab->history();
                tabsHistory.append(tabHistory);
            } else {
                tabsHistory << QByteArray();
            }
#else
            tabsHistory.append(QByteArray());
#endif
        } else {
            tabs.append(QString::null);
            tabsHistory.append(QByteArray());
        }
    }
    stream << tabs;
    stream << currentIndex();
    stream << tabsHistory;

    return data;
}

bool TabWidget::restoreState(const QByteArray &state)
{
    int version = 1;
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    if (stream.atEnd())
        return false;

    qint32 marker;
    qint32 v;
    stream >> marker;
    stream >> v;
    if (marker != TabWidgetMagic || v != version)
        return false;

    QStringList openTabs;
    stream >> openTabs;

    int currentTab;
    stream >> currentTab;
    setCurrentIndex(currentTab);
    QList<QByteArray> tabHistory;
    stream >> tabHistory;

    for (int i = 0; i < openTabs.count(); ++i) {
        QUrl url = QUrl::fromEncoded(openTabs.at(i).toUtf8());
        TabWidget::OpenUrlIn tab = i == 0 && currentWebView()->url() == QUrl() ? CurrentTab : NewTab;
#if QT_VERSION >= 0x040600
        QByteArray historyState = tabHistory.value(i);
        if (!historyState.isEmpty()) {
            createTab(historyState, tab);
        } else {
#endif
            if (WebView *webView = getView(tab, currentWebView()))
                webView->loadUrl(url);
#if QT_VERSION >= 0x040600
        }
#endif

    }
    return true;
}

void TabWidget::createTab(const QByteArray &historyState, TabWidget::OpenUrlIn tab)
{
#if QT_VERSION >= 0x040600
    if (WebView *webView = getView(tab, currentWebView())) {
        QDataStream historyStream(historyState);
        historyStream >> *webView->history();
    }
#else
    qWarning() << "Warning: TabWidget::createTab should not be called, but it is...";
    Q_UNUSED(historyState);
    Q_UNUSED(tab);
#endif
}

