/**
 * Copyright (c) 2009, Benjamin C. Meyer <ben@meyerhome.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Benjamin Meyer nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <qtry.h>

#include "adblockdialog.h"
#include "adblockmanager.h"
#include "adblocksubscription.h"

#include <qdebug.h>
#include <QSignalSpy>
#include <QTest>
#include <QUrl>


class tst_AdBlockManager : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void adblockmanager_data();
    void adblockmanager();

    void addSubscription();
    void customRules();
    void isEnabled_data();
    void isEnabled();
    void load();
    void removeSubscription();
    void showDialog();
    void rulesChanged();
};

// Subclass that exposes the protected functions.
class SubAdBlockManager : public AdBlockManager
{
public:
    ~SubAdBlockManager() {
        QList<AdBlockSubscription*> list = subscriptions();
        foreach (AdBlockSubscription *s, list)
            removeSubscription(s);
        setEnabled(false);
    }

    void call_rulesChanged()
        { return SubAdBlockManager::rulesChanged(); }
};

// This will be called before the first test function is executed.
// It is only called once.
void tst_AdBlockManager::initTestCase()
{
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_AdBlockManager::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_AdBlockManager::init()
{
}

// This will be called after every test function.
void tst_AdBlockManager::cleanup()
{
}

void tst_AdBlockManager::adblockmanager_data()
{
}

void tst_AdBlockManager::adblockmanager()
{
    SubAdBlockManager manager;
    manager.addSubscription((AdBlockSubscription*)0);
    QVERIFY(manager.customRules());
    QVERIFY(manager.instance() != (AdBlockManager*)0);
    QCOMPARE(manager.isEnabled(), true);
    manager.load();
    QVERIFY(manager.network());
    QVERIFY(manager.page());
    manager.removeSubscription((AdBlockSubscription*)0);
    manager.setEnabled(false);
    QVERIFY(manager.showDialog());
    QList<AdBlockSubscription*> list;
    list.append(manager.customRules());
    QCOMPARE(manager.subscriptions(), list);
}

// public void addSubscription(AdBlockSubscription *subscription)
void tst_AdBlockManager::addSubscription()
{
    SubAdBlockManager manager;

    QList<AdBlockSubscription*> list = manager.subscriptions();

    QSignalSpy spy0(&manager, SIGNAL(rulesChanged()));

    AdBlockSubscription *subscription = new AdBlockSubscription(QUrl(), &manager);
    manager.addSubscription(subscription);
    QCOMPARE(manager.subscriptions(), (list += subscription));

    QCOMPARE(spy0.count(), 1);
}

// public AdBlockSubscription *customRules()
void tst_AdBlockManager::customRules()
{
    SubAdBlockManager manager;
    QSignalSpy spy0(&manager, SIGNAL(rulesChanged()));

    AdBlockSubscription *subscription = manager.customRules();
    QVERIFY(subscription);
    QVERIFY(!subscription->title().isEmpty());
    QVERIFY(subscription->allRules().isEmpty());

    QCOMPARE(spy0.count(), 1);

    subscription = manager.customRules();
    QCOMPARE(spy0.count(), 1);
}

void tst_AdBlockManager::isEnabled_data()
{
    QTest::addColumn<bool>("isEnabled");
    QTest::newRow("true") << true;
    QTest::newRow("false") << false;
}

// public bool isEnabled() const
void tst_AdBlockManager::isEnabled()
{
    QFETCH(bool, isEnabled);

    SubAdBlockManager manager;

    QSignalSpy spy0(&manager, SIGNAL(rulesChanged()));

    bool before = manager.isEnabled();

    manager.setEnabled(isEnabled);
    manager.setEnabled(isEnabled);
    QCOMPARE(manager.isEnabled(), isEnabled);

    QCOMPARE(spy0.count(), before == isEnabled ? 0 : 1);
}

// public void load()
void tst_AdBlockManager::load()
{
    SubAdBlockManager manager;

    QSignalSpy spy0(&manager, SIGNAL(rulesChanged()));

    manager.load();

    QCOMPARE(spy0.count(), 0);
}

// public void removeSubscription(AdBlockSubscription *subscription)
void tst_AdBlockManager::removeSubscription()
{
    SubAdBlockManager manager;

    QSignalSpy spy0(&manager, SIGNAL(rulesChanged()));

    QList<AdBlockSubscription*> list = manager.subscriptions();
    AdBlockSubscription *subscription = new AdBlockSubscription(QUrl(), &manager);
    manager.addSubscription(subscription);
    manager.removeSubscription(subscription);
    QCOMPARE(manager.subscriptions(), list);

    QCOMPARE(spy0.count(), 2);
}


// public AdBlockDialog *showDialog()
void tst_AdBlockManager::showDialog()
{
    SubAdBlockManager manager;

    QSignalSpy spy0(&manager, SIGNAL(rulesChanged()));

    AdBlockDialog *dialog = manager.showDialog();
    QVERIFY(dialog);
    QTRY_VERIFY(dialog->isVisible());
}

void tst_AdBlockManager::rulesChanged()
{
    SubAdBlockManager manager;

    QSignalSpy spy0(&manager, SIGNAL(rulesChanged()));


    AdBlockSubscription *subscription = new AdBlockSubscription(QUrl(), &manager);
    manager.addSubscription(subscription);
    subscription->setEnabled(true);
    subscription->addRule(AdBlockRule());

    QCOMPARE(spy0.count(), 3);
}

QTEST_MAIN(tst_AdBlockManager)
#include "tst_adblockmanager.moc"

