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

#include <QtGui/QtGui>
#include <QtTest/QtTest>
#include <QAction>
#include <tabbar.h>

class tst_TabBar : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void tabbar_data();
    void tabbar();

    void showTabBarWhenOneTab_data();
    void showTabBarWhenOneTab();

    void tabSizeHint_data();
    void tabSizeHint();
};

// Subclass that exposes the protected functions.
class SubTabBar : public TabBar
{
public:
    void call_cloneTab(int index)
        { return SubTabBar::cloneTab(index); }

    void call_closeOtherTabs(int index)
        { return SubTabBar::closeOtherTabs(index); }

    void call_closeTab(int index)
        { return SubTabBar::closeTab(index); }

    void call_dragEnterEvent(QDragEnterEvent *event)
        { return SubTabBar::dragEnterEvent(event); }

    void call_dropEvent(QDropEvent *event)
        { return SubTabBar::dropEvent(event); }

    void call_mouseMoveEvent(QMouseEvent *event)
        { return SubTabBar::mouseMoveEvent(event); }

    void call_mousePressEvent(QMouseEvent *event)
        { return SubTabBar::mousePressEvent(event); }

    void call_newTab()
        { return SubTabBar::newTab(); }

    void call_reloadAllTabs()
        { return SubTabBar::reloadAllTabs(); }

    void call_reloadTab(int index)
        { return SubTabBar::reloadTab(index); }

    void call_tabLayoutChange()
        { return SubTabBar::tabLayoutChange(); }

    QSize call_tabSizeHint(int index) const
        { return SubTabBar::tabSizeHint(index); }
};

// This will be called before the first test function is executed.
// It is only called once.
void tst_TabBar::initTestCase()
{
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_TabBar::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_TabBar::init()
{
}

// This will be called after every test function.
void tst_TabBar::cleanup()
{
}

void tst_TabBar::tabbar_data()
{
}

void tst_TabBar::tabbar()
{
    SubTabBar bar;
    QCOMPARE(bar.showTabBarWhenOneTab(), true);
    bar.setShowTabBarWhenOneTab(false);
    QVERIFY(bar.viewTabBarAction() != 0);
    bar.call_tabLayoutChange();
}

void tst_TabBar::showTabBarWhenOneTab_data()
{
    QTest::addColumn<bool>("showTabBarWhenOneTab");
    QTest::newRow("true") << false;
    QTest::newRow("false") << false;
}

// public bool showTabBarWhenOneTab() const
void tst_TabBar::showTabBarWhenOneTab()
{
    QFETCH(bool, showTabBarWhenOneTab);

    SubTabBar bar;
    bar.show();
    bar.setShowTabBarWhenOneTab(showTabBarWhenOneTab);
    QAction *action = bar.viewTabBarAction();
    QVERIFY(action);
    QCOMPARE(action->text(), (showTabBarWhenOneTab ? QString("Hide Tab Bar") : QString("Show Tab Bar")));
    QCOMPARE(bar.showTabBarWhenOneTab(), showTabBarWhenOneTab);

    bar.addTab("one");
    QCOMPARE(bar.count(), 1);
    QCOMPARE(bar.isVisible(), showTabBarWhenOneTab);

    bar.addTab("two");
    QCOMPARE(bar.count(), 2);
    QCOMPARE(bar.isVisible(), true);
    QCOMPARE(action->text(), QString("Hide Tab Bar"));
    QCOMPARE(action->isEnabled(), false);

    bar.removeTab(0);
    QCOMPARE(bar.count(), 1);
    QCOMPARE(bar.isVisible(), showTabBarWhenOneTab);
    QCOMPARE(action->isEnabled(), true);
    QCOMPARE(action->text(), (showTabBarWhenOneTab ? QString("Hide Tab Bar") : QString("Show Tab Bar")));
}

void tst_TabBar::tabSizeHint_data()
{
    QTest::addColumn<int>("index");
    QTest::newRow("0") << 0;
}

// protected QSize tabSizeHint(int index) const
void tst_TabBar::tabSizeHint()
{
    QFETCH(int, index);

    SubTabBar bar;

    QVERIFY(bar.call_tabSizeHint(index).width() <= 250);
}

QTEST_MAIN(tst_TabBar)
#include "tst_tabbar.moc"

