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

#include "opensearchwriter.h"
#include "opensearchengine.h"

class tst_OpenSearchWriter : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void write_data();
    void write();
};

// This will be called before the first test function is executed.
// It is only called once.
void tst_OpenSearchWriter::initTestCase()
{
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_OpenSearchWriter::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_OpenSearchWriter::init()
{
}

// This will be called after every test function.
void tst_OpenSearchWriter::cleanup()
{
}

Q_DECLARE_METATYPE(OpenSearchEngine::Parameters)
void tst_OpenSearchWriter::write_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("description");
    QTest::addColumn<QString>("searchUrlTemplate");
    QTest::addColumn<QString>("suggestionsUrlTemplate");
    QTest::addColumn<QString>("imageUrl");
    QTest::addColumn<OpenSearchEngine::Parameters>("searchParameters");
    QTest::addColumn<OpenSearchEngine::Parameters>("suggestionsParameters");
    QTest::addColumn<QString>("searchMethod");
    QTest::addColumn<QString>("suggestionsMethod");
    QTest::addColumn<QString>("fileName");

    QTest::newRow("testfile1") << QString("Foo Bar") << QString("Bar Foo") << QString("http://foobar.barfoo/search") << QString()
            << QString() << OpenSearchEngine::Parameters() << OpenSearchEngine::Parameters()
            << QString() << QString("get") << QString(":/testfile1.xml");

    QTest::newRow("testfile2") << QString("Endorphin!") << QString("a cross platform web browser built using Qt and WebKit")
            << QString("http://foobar.barfoo/search") << QString("http://foobar.barfoo/suggest") << QString()
            << OpenSearchEngine::Parameters() << OpenSearchEngine::Parameters()
            << QString("get") << QString("post") << QString(":/testfile2.xml");

    QTest::newRow("testile3") << QString("Foo Bar") << QString("Bar Foo") << QString("http://foobar.barfoo/search")
            << QString("http://foobar.barfoo/suggest") << QString()
            << (OpenSearchEngine::Parameters() << OpenSearchEngine::Parameter("q", "{searchTerms}") << OpenSearchEngine::Parameter("a", "foo"))
            << (OpenSearchEngine::Parameters() << OpenSearchEngine::Parameter("q", "{searchTerms}"))
            << QString("post") << QString("foo") << QString(":/testfile3.xml");
}

void tst_OpenSearchWriter::write()
{
    QFETCH(QString, name);
    QFETCH(QString, description);
    QFETCH(QString, searchUrlTemplate);
    QFETCH(QString, suggestionsUrlTemplate);
    QFETCH(QString, imageUrl);
    QFETCH(OpenSearchEngine::Parameters, searchParameters);
    QFETCH(OpenSearchEngine::Parameters, suggestionsParameters);
    QFETCH(QString, searchMethod);
    QFETCH(QString, suggestionsMethod);
    QFETCH(QString, fileName);

    OpenSearchEngine engine;
    OpenSearchWriter writer;

    engine.setName(name);
    engine.setDescription(description);
    engine.setSearchUrlTemplate(searchUrlTemplate);
    engine.setSuggestionsUrlTemplate(suggestionsUrlTemplate);
    engine.setImageUrl(imageUrl);
    engine.setSearchParameters(searchParameters);
    engine.setSuggestionsParameters(suggestionsParameters);
    engine.setSearchMethod(searchMethod);
    engine.setSuggestionsMethod(suggestionsMethod);

    QByteArray output;
    QBuffer buffer(&output);
    writer.write(&buffer, &engine);

    QFile expected(fileName);
    expected.open(QIODevice::ReadOnly);

    QCOMPARE(output, expected.readAll());
}

QTEST_MAIN(tst_OpenSearchWriter)

#include "tst_opensearchwriter.moc"

