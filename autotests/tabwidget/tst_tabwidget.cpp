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
#include <QtGui/QtGui>
#include <QAction>
#include <QLineEdit>
#include "qtest_arora.h"

#include <tabwidget.h>
#include <webview.h>

class tst_TabWidget : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void tabwidget_data();
    void tabwidget();
    void addWebAction_data();
    void addWebAction();
    void closeTab_data();
    void closeTab();
    void currentLocationBar_data();
    void currentLocationBar();
    void currentWebView_data();
    void currentWebView();
    void locationBarStack_data();
    void locationBarStack();
    void loadUrl_data();
    void loadUrl();
    void newTab_data();
    void newTab();
    void nextTab_data();
    void nextTab();
    void previousTab_data();
    void previousTab();
    void recentlyClosedTabsAction_data();
    void recentlyClosedTabsAction();
    void linkHovered_data();
    void linkHovered(const QString &);
    void loadProgress_data();
    void loadProgress(int);
    void setCurrentTitle_data();
    void setCurrentTitle(const QString &);
    void showStatusBarMessage_data();
    void showStatusBarMessage(const QString &);
    void tabsChanged_data();
    void tabsChanged();

    void saveState();
};

// Subclass that exposes the protected functions.
class SubTabWidget : public TabWidget
{
public:
    void call_linkHovered(QString const &link)
        { return SubTabWidget::linkHovered(link); }

    void call_loadProgress(int progress)
        { return SubTabWidget::loadProgress(progress); }

    void call_setCurrentTitle(QString const &url)
        { return SubTabWidget::setCurrentTitle(url); }

    void call_showStatusBarMessage(QString const &message)
        { return SubTabWidget::showStatusBarMessage(message); }

    void call_tabsChanged()
        { return SubTabWidget::tabsChanged(); }
};

// This will be called before the first test function is executed.
// It is only called once.
void tst_TabWidget::initTestCase()
{
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_TabWidget::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_TabWidget::init()
{
}

// This will be called after every test function.
void tst_TabWidget::cleanup()
{
}

void tst_TabWidget::tabwidget_data()
{
}

void tst_TabWidget::tabwidget()
{
    SubTabWidget widget;
    widget.addWebAction((QAction*)0, QWebPage::Back);
    widget.closeTab();
    QVERIFY(widget.closeTabAction());
    widget.currentWebView();
    widget.locationBarStack();
    widget.loadUrl(QUrl());
    widget.newTab();
    QVERIFY(widget.newTabAction());
    widget.nextTab();
    QVERIFY(widget.nextTabAction());
    widget.previousTab();
    QVERIFY(widget.previousTabAction());
    QVERIFY(widget.recentlyClosedTabsAction());
    QVERIFY(widget.currentLocationBar());
}

Q_DECLARE_METATYPE(QWebPage::WebAction)
void tst_TabWidget::addWebAction_data()
{
    QTest::addColumn<QWebPage::WebAction>("webAction");
    QTest::newRow("back") << QWebPage::Back;
}

// public void addWebAction(QAction *action, QWebPage::WebAction webAction)
void tst_TabWidget::addWebAction()
{
    QFETCH(QWebPage::WebAction, webAction);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    QAction *action = new QAction(&widget);
    widget.addWebAction(action, webAction);

    widget.newTab();
    QVERIFY(!action->isEnabled());

    widget.loadUrl(QUrl("about:config"));
    QUrl url1(":/notfound.html"); //QUrl("http://www.google.com/"));
    QUrl url2(":/notfound2.html"); //QUrl("http://www.yahoo.com/"));
    widget.loadUrl(url1);
    widget.loadUrl(url2);

    QTRY_VERIFY(action->isEnabled());
    widget.newTab();
    QVERIFY(!action->isEnabled());

    QCOMPARE(spy0.count(), 0);
    QVERIFY(spy2.count() > 0);
    QCOMPARE(spy3.count(), 8);
    QVERIFY(spy4.count() > 0);
    QCOMPARE(spy5.count(), 6);
    QCOMPARE(spy6.count(), 0);
}

void tst_TabWidget::closeTab_data()
{
    QTest::addColumn<int>("index");
    QTest::newRow("null") << 0;
}

// public void closeTab(int index = -1)
void tst_TabWidget::closeTab()
{
    QFETCH(int, index);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    widget.closeTab(index);
    widget.newTab();
    widget.newTab();
    widget.loadUrl(QUrl("about:config"));
    widget.newTab();
    qDebug() << "TODO";
    return;

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 4);
    QCOMPARE(spy3.count(), 2);
    QCOMPARE(spy4.count(), 4);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
}

Q_DECLARE_METATYPE(QLineEdit*)
void tst_TabWidget::currentLocationBar_data()
{
    /*
    QTest::addColumn<QLineEdit*>("currentLocationBar");
    QTest::newRow("null") << QLineEdit*();
    */
}

// public QLineEdit *currentLocationBar() const
void tst_TabWidget::currentLocationBar()
{
    /*
    QFETCH(QLineEdit*, currentLocationBar);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    QCOMPARE(widget.currentLocationBar(), currentLocationBar);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.");
}

Q_DECLARE_METATYPE(WebView*)
void tst_TabWidget::currentWebView_data()
{
    /*
    QTest::addColumn<WebView*>("currentWebView");
    QTest::newRow("null") << WebView*();
    */
}

// public WebView *currentWebView() const
void tst_TabWidget::currentWebView()
{
    /*
    QFETCH(WebView*, currentWebView);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    QCOMPARE(widget.currentWebView(), currentWebView);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.");
}

Q_DECLARE_METATYPE(QWidget*)
void tst_TabWidget::locationBarStack_data()
{
    /*
    QTest::addColumn<QWidget*>("locationBarStack");
    QTest::newRow("null") << QWidget*();
    */
}

// public QWidget *locationBarStack() const
void tst_TabWidget::locationBarStack()
{
    /*
    QFETCH(QWidget*, locationBarStack);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    QCOMPARE(widget.locationBarStack(), locationBarStack);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.");
}

void tst_TabWidget::loadUrl_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::newRow("null") << QUrl();
}

// public void loadUrl(QUrl const &url)
void tst_TabWidget::loadUrl()
{
    /*
    QFETCH(QUrl, url);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    widget.loadUrl(url);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.");
}

void tst_TabWidget::newTab_data()
{
    QTest::addColumn<int>("foo");
    QTest::newRow("null") << 0;
}

// public void newTab()
void tst_TabWidget::newTab()
{
    /*
    QFETCH(int, foo);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    widget.newTab();

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.");
}

void tst_TabWidget::nextTab_data()
{
    QTest::addColumn<int>("foo");
    QTest::newRow("null") << 0;
}

// public void nextTab()
void tst_TabWidget::nextTab()
{
    /*
    QFETCH(int, foo);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    widget.nextTab();

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.");
}

void tst_TabWidget::previousTab_data()
{
    QTest::addColumn<int>("foo");
    QTest::newRow("null") << 0;
}

// public void previousTab()
void tst_TabWidget::previousTab()
{
    /*
    QFETCH(int, foo);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    widget.previousTab();

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.");
}

void tst_TabWidget::recentlyClosedTabsAction_data()
{
    /*
    QTest::addColumn<QAction*>("recentlyClosedTabsAction");
    QTest::newRow("null") << QAction*();
    */
}

// public QAction *recentlyClosedTabsAction() const
void tst_TabWidget::recentlyClosedTabsAction()
{
    /*
    QFETCH(QAction*, recentlyClosedTabsAction);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    QCOMPARE(widget.recentlyClosedTabsAction(), recentlyClosedTabsAction);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.");
}

void tst_TabWidget::linkHovered_data()
{
    QTest::addColumn<QString>("link");
    QTest::newRow("null") << QString("foo");
}

// protected void linkHovered(QString const &link)
void tst_TabWidget::linkHovered(const QString &)
{
    /*
    QFETCH(QString, link);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    widget.call_linkHovered(link);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.");
}

void tst_TabWidget::loadProgress_data()
{
    QTest::addColumn<int>("progress");
    QTest::newRow("null") << 0;
}

// protected void loadProgress(int progress)
void tst_TabWidget::loadProgress(int)
{
    /*
    QFETCH(int, progress);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    widget.call_loadProgress(progress);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.");
}

void tst_TabWidget::setCurrentTitle_data()
{
    QTest::addColumn<QString>("url");
    QTest::newRow("null") << QString("foo");
}

// protected void setCurrentTitle(QString const &url)
void tst_TabWidget::setCurrentTitle(const QString &)
{
    /*
    QFETCH(QString, url);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    widget.call_setCurrentTitle(url);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.");
}

void tst_TabWidget::showStatusBarMessage_data()
{
    QTest::addColumn<QString>("message");
    QTest::newRow("null") << QString("foo");
}

// protected void showStatusBarMessage(QString const &message)
void tst_TabWidget::showStatusBarMessage(const QString &)
{
    /*
    QFETCH(QString, message);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    widget.call_showStatusBarMessage(message);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.");
}

void tst_TabWidget::tabsChanged_data()
{
    QTest::addColumn<int>("foo");
    QTest::newRow("null") << 0;
}

// protected void tabsChanged()
void tst_TabWidget::tabsChanged()
{
    /*
    QFETCH(int, foo);

    SubTabWidget widget;

    QSignalSpy spy0(&widget, SIGNAL(linkHovered(const QString &)));
    QSignalSpy spy2(&widget, SIGNAL(loadProgress(int)));
    QSignalSpy spy3(&widget, SIGNAL(setCurrentTitle(const QString &)));
    QSignalSpy spy4(&widget, SIGNAL(showStatusBarMessage(const QString &)));
    QSignalSpy spy5(&widget, SIGNAL(tabsChanged()));
    QSignalSpy spy6(&widget, SIGNAL(lastTabClosed()));

    widget.call_tabsChanged();

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy2.count(), 0);
    QCOMPARE(spy3.count(), 0);
    QCOMPARE(spy4.count(), 0);
    QCOMPARE(spy5.count(), 0);
    QCOMPARE(spy6.count(), 0);
    */
    QSKIP("Test is not implemented.");
}

void tst_TabWidget::saveState()
{
    SubTabWidget widget;
    widget.newTab();
    QCOMPARE(widget.count(), 1);

    QUrl url = QUrl("data:text/html;base32,Hello%20World");
    widget.loadUrl(url, TabWidget::CurrentTab);
    QCOMPARE(widget.count(), 1);
    QCOMPARE(widget.webView(0)->url(), url);

    widget.loadUrl(url, TabWidget::NewTab);
    QCOMPARE(widget.count(), 2);
    QCOMPARE(widget.webView(1)->url(), url);

    QByteArray state = widget.saveState();

    widget.closeTab();
    QCOMPARE(widget.count(), 1);
    widget.closeTab();
    QCOMPARE(widget.count(), 0);

    widget.newTab();
    widget.restoreState(state);
    QCOMPARE(widget.count(), 2);
    QVERIFY(widget.webView(0));
    QCOMPARE(widget.webView(0)->url(), url);
    QVERIFY(widget.webView(1));
    QCOMPARE(widget.webView(1)->url(), url);

    widget.closeTab();
    widget.closeTab();
}


QTEST_MAIN(tst_TabWidget)
#include "tst_tabwidget.moc"

