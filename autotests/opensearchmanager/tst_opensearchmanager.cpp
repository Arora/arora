/*
 * Copyright 2009 Jakub Wieczorek <faw217@gmail.com>
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

#include "qtest_endorphin.h"

#include "opensearchengine.h"
#include "opensearchmanager.h"

class tst_OpenSearchManager : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void addRemoveEngine_data();
    void addRemoveEngine();
    void setCurrentEngine_data();
    void setCurrentEngine();
    void generateEngineFileName_data();
    void generateEngineFileName();
    void restoreDefaults();
    void keywords();
    void convertKeywordSearchToUrl();
    void convertKeywordSearchToUrl_data();
};

class SubOpenSearchManager : public OpenSearchManager
{
public:
    QString generateEngineFileName(const QString &engineName)
    {
        return OpenSearchManager::generateEngineFileName(engineName);
    }

    static int defaultCount()
    {
        return QDir(":/searchengines/").count();
    }
};

// This will be called before the first test function is executed.
// It is only called once.
void tst_OpenSearchManager::initTestCase()
{
    QCoreApplication::setApplicationName("opensearchtest");

    SubOpenSearchManager manager;
    foreach (const QString &name, manager.allEnginesNames())
        manager.removeEngine(name);
    QCOMPARE(manager.enginesCount(), 1);
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_OpenSearchManager::cleanupTestCase()
{
    SubOpenSearchManager manager;
    QCOMPARE(manager.enginesCount(), 1);
}

// This will be called before each test function is executed.
void tst_OpenSearchManager::init()
{
    SubOpenSearchManager manager;
    QCOMPARE(manager.enginesCount(), 1);
}

// This will be called after every test function.
void tst_OpenSearchManager::cleanup()
{
    SubOpenSearchManager manager;
    foreach (const QString &name, manager.allEnginesNames())
        manager.removeEngine(name);
    QCOMPARE(manager.enginesCount(), 1);
}

void tst_OpenSearchManager::addRemoveEngine_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("description");
    QTest::addColumn<QString>("searchUrlTemplate");
    QTest::addColumn<bool>("valid");

    QTest::newRow("valid") << "Foo" << "Bar" << "http://foobaz.bar" << true;
    QTest::newRow("invalid") << "Baz" << "Foo" << "" << false;
}

void tst_OpenSearchManager::addRemoveEngine()
{
    QFETCH(QString, name);
    QFETCH(QString, description);
    QFETCH(QString, searchUrlTemplate);
    QFETCH(bool, valid);

    SubOpenSearchManager manager;

    QSignalSpy signalSpy(&manager, SIGNAL(changed()));

    OpenSearchEngine *engine = new OpenSearchEngine();
    engine->setName(name);
    engine->setDescription(description);
    engine->setSearchUrlTemplate(searchUrlTemplate);

    QCOMPARE(manager.enginesCount(), 1);
    QVERIFY(!manager.engineExists(name));

    bool result = manager.addEngine(engine);

    QCOMPARE(result, valid);
    QCOMPARE(manager.enginesCount(), (valid ? 2 : 1));
    QCOMPARE(manager.engineExists(name), valid);
    QCOMPARE(signalSpy.count(), (valid ? 1 : 0));

    manager.removeEngine(engine->name());

    QCOMPARE(manager.enginesCount(), 1);
    QVERIFY(!manager.engineExists(name));
    QCOMPARE(signalSpy.count(), (valid ? 2 : 0));
}

void tst_OpenSearchManager::setCurrentEngine_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("description");
    QTest::addColumn<QString>("searchUrlTemplate");
    QTest::addColumn<bool>("valid");

    QTest::newRow("valid") << "Foo" << "Bar" << "http://foobaz.bar" << true;
    QTest::newRow("invalid") << "Baz" << "Foo" << "" << false;
}

void tst_OpenSearchManager::setCurrentEngine()
{
    QFETCH(QString, name);
    QFETCH(QString, description);
    QFETCH(QString, searchUrlTemplate);
    QFETCH(bool, valid);

    SubOpenSearchManager manager;

    QCOMPARE(manager.enginesCount(), 1);

    QString oldCurrentEngineName = manager.currentEngineName();
    OpenSearchEngine *oldCurrentEngine = manager.currentEngine();

    QSignalSpy signalSpy(&manager, SIGNAL(currentEngineChanged()));

    OpenSearchEngine *engine = new OpenSearchEngine();
    engine->setName(name);
    engine->setDescription(description);
    engine->setSearchUrlTemplate(searchUrlTemplate);

    bool result = manager.addEngine(engine);
    QCOMPARE(result, valid);

    manager.setCurrentEngineName(name);
    QCOMPARE(manager.currentEngineName(), (valid ? name : oldCurrentEngineName));
    QCOMPARE(*manager.currentEngine(), (valid ? *engine : *oldCurrentEngine));
    QCOMPARE(signalSpy.count(), (valid ? 1 : 0));

    manager.removeEngine(engine->name());
    QCOMPARE(signalSpy.count(), (valid ? 2 : 0));
}

void tst_OpenSearchManager::generateEngineFileName_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("fileName");

    QTest::newRow("simple") << "FooBar" << "FooBar.xml";
    QTest::newRow("with-spaces") << "Foo Bar" << "Foo_Bar.xml";
    QTest::newRow("with-special-chars") << ":Foo&Bar*Baz-" << "FooBarBaz.xml";
    QTest::newRow("with-special-chars-and-spaces") << ": Foo & Bar -" << "_Foo__Bar_.xml";
}

void tst_OpenSearchManager::generateEngineFileName()
{
    QFETCH(QString, name);
    QFETCH(QString, fileName);

    SubOpenSearchManager manager;

    QCOMPARE(manager.generateEngineFileName(name), fileName);
}

void tst_OpenSearchManager::restoreDefaults()
{
    SubOpenSearchManager manager;

    QCOMPARE(manager.enginesCount(), 1);
    manager.restoreDefaults();
    QCOMPARE(manager.enginesCount(), manager.defaultCount());

    foreach (const QString &name, manager.allEnginesNames())
        manager.removeEngine(name);

    // Never let the manager have no engines.
    QCOMPARE(manager.enginesCount(), 1);

    OpenSearchEngine *engine = new OpenSearchEngine();
    engine->setName("Foobarbaz");
    engine->setSearchUrlTemplate("http://foobarbaz.baz");

    manager.addEngine(engine);
    manager.restoreDefaults();
    QCOMPARE(manager.enginesCount(), manager.defaultCount() + 1);

    manager.removeEngine(engine->name());
}

void tst_OpenSearchManager::keywords()
{
    {
        SubOpenSearchManager manager;

        QVERIFY(!manager.engineForKeyword("foo"));
        QVERIFY(!manager.engineForKeyword(QString()));

        manager.setEngineForKeyword("foo", 0);
        manager.setEngineForKeyword(QString(), 0);
        QVERIFY(!manager.engineForKeyword("foo"));
        QVERIFY(!manager.engineForKeyword(QString()));
        QCOMPARE(manager.keywordsForEngine(0), QStringList());

        manager.setKeywordsForEngine(0, QStringList() << "foo");
        QCOMPARE(manager.keywordsForEngine(0), QStringList());

        manager.restoreDefaults();

        OpenSearchEngine *engine1 = manager.engine(manager.allEnginesNames().at(0));
        OpenSearchEngine *engine2 = manager.engine(manager.allEnginesNames().at(1));

        QCOMPARE(manager.keywordsForEngine(engine1), QStringList());
        QCOMPARE(manager.keywordsForEngine(engine2), QStringList());

        manager.setEngineForKeyword("foo", engine1);
        manager.setEngineForKeyword("bar", engine1);
        manager.setEngineForKeyword("baz", engine2);

        QCOMPARE(manager.engineForKeyword("foo"), engine1);
        QCOMPARE(manager.engineForKeyword("bar"), engine1);
        QCOMPARE(manager.engineForKeyword("baz"), engine2);

        QCOMPARE(manager.keywordsForEngine(engine1), QStringList() << "foo" << "bar");
        QCOMPARE(manager.keywordsForEngine(engine2), QStringList() << "baz");

        manager.setKeywordsForEngine(engine1, QStringList() << "baz");
        manager.setKeywordsForEngine(engine2, QStringList() << "foo" << "bar");

        QCOMPARE(manager.engineForKeyword("foo"), engine2);
        QCOMPARE(manager.engineForKeyword("bar"), engine2);
        QCOMPARE(manager.engineForKeyword("baz"), engine1);

        QCOMPARE(manager.keywordsForEngine(engine2), QStringList() << "foo" << "bar");
        QCOMPARE(manager.keywordsForEngine(engine1), QStringList() << "baz");
    }

    {
        SubOpenSearchManager manager;

        manager.restoreDefaults();

        OpenSearchEngine *engine1 = manager.engine(manager.allEnginesNames().at(0));
        OpenSearchEngine *engine2 = manager.engine(manager.allEnginesNames().at(1));

        QCOMPARE(*manager.engineForKeyword("foo"), *engine2);
        QCOMPARE(*manager.engineForKeyword("bar"), *engine2);
        QCOMPARE(*manager.engineForKeyword("baz"), *engine1);

        QCOMPARE(manager.keywordsForEngine(engine2), QStringList() << "foo" << "bar");
        QCOMPARE(manager.keywordsForEngine(engine1), QStringList() << "baz");

        manager.setEngineForKeyword("foo", 0);

        QVERIFY(!manager.engineForKeyword("foo"));
        QCOMPARE(*manager.engineForKeyword("bar"), *engine2);
        QCOMPARE(*manager.engineForKeyword("baz"), *engine1);

        QCOMPARE(manager.keywordsForEngine(engine2), QStringList() << "bar");
        QCOMPARE(manager.keywordsForEngine(engine1), QStringList() << "baz");

        manager.setKeywordsForEngine(engine1, QStringList());
        manager.setKeywordsForEngine(engine2, QStringList());
    }
}

void tst_OpenSearchManager::convertKeywordSearchToUrl_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<bool>("valid");

    QTest::newRow("invalid-0") << "null" << false;
    QTest::newRow("invalid-1") << "foo" << false;
    QTest::newRow("invalid-2") << "bar" << false;
    QTest::newRow("invalid-3") << "baz" << false;
    QTest::newRow("invalid-4") << "foo " << false;
    QTest::newRow("invalid-5") << "foobar" << false;
    QTest::newRow("valid-0") << "foo searchstring" << true;
}

void tst_OpenSearchManager::convertKeywordSearchToUrl()
{
    QFETCH(QString, string);
    QFETCH(bool, valid);

    SubOpenSearchManager manager;
    manager.restoreDefaults();
    OpenSearchEngine *engine1 = manager.engine(manager.allEnginesNames().at(0));
    manager.setEngineForKeyword("foo", engine1);
    manager.setEngineForKeyword("bar", engine1);
    OpenSearchEngine *engine2 = manager.engine(manager.allEnginesNames().at(1));
    manager.setEngineForKeyword("baz", engine2);

    QCOMPARE(manager.convertKeywordSearchToUrl(string).isValid(), valid);
}

QTEST_MAIN(tst_OpenSearchManager)

#include "tst_opensearchmanager.moc"

