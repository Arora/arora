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
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWebKit/QtWebKit>
#include <QAction>

#include <webactionmapper.h>
#include <webview.h>

#include "qtest_arora.h"

class tst_WebActionMapper : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void webactionmapper_data();
    void webactionmapper();
    void addChild();
    void updateCurrent_data();
    void updateCurrent();

    void triggerRoot();
    void destroyRoot();
    void destroyCurrent();
    void childChanged_data();
    void childChanged();
};

// Subclass that exposes the protected functions.
class SubWebActionMapper : public WebActionMapper
{
public:
    SubWebActionMapper(QAction *root, QWebPage::WebAction webAction, QObject *parent)
        : WebActionMapper(root, webAction, parent) {}

};

// This will be called before the first test function is executed.
// It is only called once.
void tst_WebActionMapper::initTestCase()
{
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_WebActionMapper::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_WebActionMapper::init()
{
}

// This will be called after every test function.
void tst_WebActionMapper::cleanup()
{
}

void tst_WebActionMapper::webactionmapper_data()
{
}

void tst_WebActionMapper::webactionmapper()
{
    SubWebActionMapper mapper(0, QWebPage::Stop, 0);
    mapper.addChild(0);
    mapper.updateCurrent(0);
    QCOMPARE(mapper.webAction(), QWebPage::Stop);
}

// public void addChild(QAction *action)
void tst_WebActionMapper::addChild()
{
    QAction *root = new QAction(this);
    SubWebActionMapper mapper(root, QWebPage::Stop, 0);
    QVERIFY(!root->isEnabled());

    QAction *child = new QAction(this);
    mapper.addChild(child);

    child->setEnabled(true);
    QVERIFY(!root->isEnabled());
}

void tst_WebActionMapper::updateCurrent_data()
{
    QTest::addColumn<int>("currentParentCount");
    QTest::newRow("null") << 0;
}

// public void updateCurrent(WebView *currentParent)
void tst_WebActionMapper::updateCurrent()
{
    QAction *root = new QAction(this);
    SubWebActionMapper mapper(root, QWebPage::Stop, 0);
    QVERIFY(!root->isEnabled());

    WebView webView;
    QAction *childAction = webView.page()->action(mapper.webAction());
    mapper.addChild(childAction);

    mapper.updateCurrent(webView.page());
    QCOMPARE(root->isChecked(), childAction->isChecked());
    QCOMPARE(root->isEnabled(), childAction->isEnabled());

    mapper.updateCurrent(0);
    QCOMPARE(root->isChecked(), false);
    QCOMPARE(root->isEnabled(), false);
}

void tst_WebActionMapper::triggerRoot()
{
    QAction *root = new QAction(this);
    SubWebActionMapper mapper(root, QWebPage::Reload, 0);

    WebView webView;
    QAction *childAction = webView.page()->action(mapper.webAction());
    mapper.addChild(childAction);

    mapper.updateCurrent(webView.page());
    QCOMPARE(root->isEnabled(), true);

    QSignalSpy spy(childAction, SIGNAL(triggered()));
    root->trigger();
    QCOMPARE(spy.count(), 1);
}

void tst_WebActionMapper::destroyRoot()
{
    QAction *root = new QAction(this);
    SubWebActionMapper mapper(root, QWebPage::Reload, 0);

    WebView webView;
    QAction *childAction = webView.page()->action(mapper.webAction());
    mapper.addChild(childAction);

    mapper.updateCurrent(webView.page());
    QCOMPARE(root->isEnabled(), true);

    delete root;
    mapper.updateCurrent(webView.page());
    webView.load(QUrl("data:text/html;<p>hello world"));
    childAction->setEnabled(false);
    childAction->setEnabled(true);
}

void tst_WebActionMapper::destroyCurrent()
{
    QAction *root = new QAction(this);
    SubWebActionMapper mapper(root, QWebPage::Reload, 0);

    WebView *webView = new WebView;
    QAction *childAction = webView->page()->action(mapper.webAction());
    mapper.addChild(childAction);

    mapper.updateCurrent(webView->page());

    delete webView;
    root->trigger();
}

void tst_WebActionMapper::childChanged_data()
{
    QTest::addColumn<bool>("isCurrent");
    QTest::newRow("true") << true;
    QTest::newRow("false") << false;
}

void tst_WebActionMapper::childChanged()
{
    QFETCH(bool, isCurrent);
    QAction *root = new QAction(this);
    SubWebActionMapper mapper(root, QWebPage::Reload, 0);

    WebView webView;
    QAction *childAction = webView.page()->action(mapper.webAction());
    mapper.addChild(childAction);

    if (isCurrent)
        mapper.updateCurrent(webView.page());

    childAction->setEnabled(false);
    QSignalSpy spy(root, SIGNAL(changed()));
    webView.load(QUrl("data:text/html;<p>hello world"));
    QCOMPARE(spy.count(), isCurrent ? 1 : 0);
}

QTEST_MAIN(tst_WebActionMapper)
#include "tst_webactionmapper.moc"

