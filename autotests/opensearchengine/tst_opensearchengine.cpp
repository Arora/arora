/*
 * Copyright 2009 Benjamin C. Meyer <ben@meyerhome.net>
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
#include "qtry.h"
#include "opensearchengine.h"
#include "opensearchenginedelegate.h"

#include <qbuffer.h>
#include <qfile.h>
#include <qlocale.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkreply.h>
#include <qnetworkrequest.h>
#include <qtimer.h>

typedef OpenSearchEngine::Parameters Parameters;
typedef OpenSearchEngine::Parameter Parameter;

class tst_OpenSearchEngine : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void opensearchengine_data();
    void opensearchengine();

    void description_data();
    void description();
    void image();
    void imageUrl_data();
    void imageUrl();
    void isValid_data();
    void isValid();
    void name_data();
    void name();
    void operatorlessthan();
    void operatorequal_data();
    void operatorequal();
    void providesSuggestions_data();
    void providesSuggestions();
    void requestSuggestions_data();
    void requestSuggestions();
    void requestSuggestionsCrash();
    void searchParameters_data();
    void searchParameters();
    void searchUrl_data();
    void searchUrl();
    void searchUrlTemplate_data();
    void searchUrlTemplate();
    void suggestionsParameters_data();
    void suggestionsParameters();
    void suggestionsUrl_data();
    void suggestionsUrl();
    void suggestionsUrlTemplate_data();
    void suggestionsUrlTemplate();
    void parseTemplate_data();
    void parseTemplate();
    void languageCodes_data();
    void languageCodes();
    void requestMethods();
    void delegate();
};

// Subclass that exposes the protected functions.
class SubOpenSearchEngine : public OpenSearchEngine
{
public:
    void call_imageChanged()
        { return SubOpenSearchEngine::imageChanged(); }

    void call_loadImage() const
        { return SubOpenSearchEngine::loadImage(); }

    QString call_parseTemplate(QString const &searchTerm, QString const &searchTemplate) const
        { return SubOpenSearchEngine::parseTemplate(searchTerm, searchTemplate); }

    void call_suggestions(QStringList const &suggestions)
        { return SubOpenSearchEngine::suggestions(suggestions); }
};

class SuggestionsTestNetworkReply : public QNetworkReply
{
    Q_OBJECT

public:
    SuggestionsTestNetworkReply(const QNetworkRequest &request, QObject *parent = 0)
        : QNetworkReply(parent)
    {
        setOperation(QNetworkAccessManager::GetOperation);
        setRequest(request);
        setUrl(request.url());
        setOpenMode(QIODevice::ReadOnly);

        expectedResult.setFileName(":/suggestions.txt");
        expectedResult.open(QIODevice::ReadOnly);
        setError(QNetworkReply::NoError, tr("No Error"));

        QTimer::singleShot(50, this, SLOT(sendSuggestions()));
    }

    ~SuggestionsTestNetworkReply()
    {
        close();
    }

    qint64 bytesAvailable() const
    {
        return expectedResult.bytesAvailable() + QNetworkReply::bytesAvailable();
    }

    void close()
    {
        expectedResult.close();
    }

    qint64 readData(char *data, qint64 maxSize)
    {
        return expectedResult.read(data, maxSize);
    }

    void abort()
    {
    }

private slots:
    void sendSuggestions()
    {
        // Publish result
        setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("text/html"));
        setHeader(QNetworkRequest::ContentLengthHeader, expectedResult.bytesAvailable());
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
        setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, QByteArray("Ok"));

        emit metaDataChanged();
        emit readyRead();
        emit downloadProgress(expectedResult.size(), expectedResult.size());
        emit finished();
    }

private:
    QFile expectedResult;
};

class SuggestionsTestNetworkAccessManager : public QNetworkAccessManager
{
public:
    SuggestionsTestNetworkAccessManager(QObject *parent = 0)
        : QNetworkAccessManager(parent)
    {
    }

    QNetworkRequest lastRequest;
    Operation lastOperation;
    bool lastOutgoingData;

protected:
    QNetworkReply *createRequest(QNetworkAccessManager::Operation operation, const QNetworkRequest &request, QIODevice *outgoingData = 0)
    {
        lastOperation = operation;
        lastRequest = request;
        lastOutgoingData = (bool)outgoingData;

        return new SuggestionsTestNetworkReply(request, 0);
    }
};

class Delegate : public OpenSearchEngineDelegate
{
    public:
        Delegate()
            : OpenSearchEngineDelegate()
            , callsCount(0)
        {
        }

        ~Delegate()
        {
        }

        void performSearchRequest(const QNetworkRequest &request,
                                  QNetworkAccessManager::Operation operation,
                                  const QByteArray &data)
        {
            ++callsCount;
            lastRequest = request;
            lastOperation = operation;
            lastData = data;
        }

        QNetworkRequest lastRequest;
        QNetworkAccessManager::Operation lastOperation;
        QByteArray lastData;
        int callsCount;
};

// This will be called before the first test function is executed.
// It is only called once.
void tst_OpenSearchEngine::initTestCase()
{
    QCoreApplication::setApplicationName("tst_opensearchengine");
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_OpenSearchEngine::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_OpenSearchEngine::init()
{
}

// This will be called after every test function.
void tst_OpenSearchEngine::cleanup()
{
}

void tst_OpenSearchEngine::opensearchengine_data()
{
}

void tst_OpenSearchEngine::opensearchengine()
{
    SubOpenSearchEngine engine;
    QCOMPARE(engine.description(), QString());
    QCOMPARE(engine.image(), QImage());
    QCOMPARE(engine.imageUrl(), QString());
    QCOMPARE(engine.isValid(), false);
    QCOMPARE(engine.name(), QString());
    OpenSearchEngine other;
    QCOMPARE(engine.operator<(other), false);
    QCOMPARE(engine.operator==(other), true);
    QCOMPARE(engine.providesSuggestions(), false);
    engine.requestSuggestions(QString());
    QCOMPARE(engine.searchParameters(), QList<Parameter>());
    QCOMPARE(engine.searchUrl(QString()), QUrl());
    QCOMPARE(engine.searchUrlTemplate(), QString());
    engine.setDescription(QString());
    engine.setImage(QImage());
    engine.setImageUrl(QString());
    engine.setName(QString());
    engine.setSearchParameters(QList<Parameter>());
    engine.setSearchUrlTemplate(QString());
    engine.setSuggestionsParameters(QList<Parameter>());
    engine.setSuggestionsUrlTemplate(QString());
    QCOMPARE(engine.suggestionsParameters(), QList<Parameter>());
    QCOMPARE(engine.suggestionsUrl(QString()), QUrl());
    QCOMPARE(engine.suggestionsUrlTemplate(), QString());
    engine.call_imageChanged();
    engine.call_loadImage();
    QVERIFY(!engine.networkAccessManager());
    QNetworkAccessManager manager;
    engine.setNetworkAccessManager(&manager);
    QCOMPARE(engine.networkAccessManager(), &manager);
    QCOMPARE(engine.call_parseTemplate(QString(), QString()), QString());
    engine.call_suggestions(QStringList());
}

void tst_OpenSearchEngine::description_data()
{
    QTest::addColumn<QString>("description");
    QTest::newRow("null") << QString();
    QTest::newRow("foo") << QString("foo");
}

// public QString description() const
void tst_OpenSearchEngine::description()
{
    QFETCH(QString, description);

    SubOpenSearchEngine engine;

    QSignalSpy spy0(&engine, SIGNAL(imageChanged()));
    QSignalSpy spy1(&engine, SIGNAL(suggestions(QStringList const&)));

    engine.setDescription(description);
    QCOMPARE(engine.description(), description);
    QCOMPARE(engine.property("description").toString(), description);
    engine.setProperty("description", QString());
    QCOMPARE(engine.property("description").toString(), QString());

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy1.count(), 0);
}

// public QImage image() const
void tst_OpenSearchEngine::image()
{
    SubOpenSearchEngine engine;

    QNetworkAccessManager manager;
    engine.setNetworkAccessManager(&manager);

    QSignalSpy spy0(&engine, SIGNAL(imageChanged()));
    QSignalSpy spy1(&engine, SIGNAL(suggestions(QStringList const&)));

    QBuffer imageBuffer;
    imageBuffer.open(QBuffer::ReadWrite);
    QPixmap image(1, 1);
    image.fill();
    image.save(&imageBuffer, "PNG");
    QString imageUrl = QString("data:image/png;base64,").append(imageBuffer.buffer().toBase64());
    engine.setImageUrl(imageUrl);
    QCOMPARE(engine.image(), QImage());

    QTRY_COMPARE(spy0.count(), 1);
    QCOMPARE(spy1.count(), 0);
    QVERIFY(engine.image() != QImage());

    SubOpenSearchEngine engine2;
    QSignalSpy spy2(&engine2, SIGNAL(imageChanged()));

    QVERIFY(engine2.imageUrl().isEmpty());
    engine2.setImage(engine.image());
    QCOMPARE(engine2.imageUrl(), imageUrl);

    QCOMPARE(spy2.count(), 1);
}

void tst_OpenSearchEngine::imageUrl_data()
{
    QTest::addColumn<QString>("imageUrl");
    QTest::newRow("null") << QString();
    QTest::newRow("foo") << QString("foo");
}

// public QString imageUrl() const
void tst_OpenSearchEngine::imageUrl()
{
    QFETCH(QString, imageUrl);

    SubOpenSearchEngine engine;

    QSignalSpy spy0(&engine, SIGNAL(imageChanged()));
    QSignalSpy spy1(&engine, SIGNAL(suggestions(QStringList const&)));

    engine.setImageUrl(imageUrl);
    QCOMPARE(engine.imageUrl(), imageUrl);
    QCOMPARE(engine.property("imageUrl").toString(), imageUrl);
    engine.setProperty("imageUrl", QString());
    QCOMPARE(engine.property("imageUrl").toString(), QString());

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy1.count(), 0);
}

void tst_OpenSearchEngine::isValid_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("searchUrlTemplate");
    QTest::addColumn<bool>("isValid");
    QTest::newRow("false-0") << QString() << QString() << false;
    QTest::newRow("false-1") << QString() << QString("x") << false;
    QTest::newRow("false-2") << QString("x") << QString() << false;
    QTest::newRow("true") << QString("x") << QString("y") << true;
}

// public bool isValid() const
void tst_OpenSearchEngine::isValid()
{
    QFETCH(QString, name);
    QFETCH(QString, searchUrlTemplate);
    QFETCH(bool, isValid);

    SubOpenSearchEngine engine;

    QSignalSpy spy0(&engine, SIGNAL(imageChanged()));
    QSignalSpy spy1(&engine, SIGNAL(suggestions(QStringList const&)));

    engine.setName(name);
    engine.setSearchUrlTemplate(searchUrlTemplate);
    QCOMPARE(engine.isValid(), isValid);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy1.count(), 0);
}

void tst_OpenSearchEngine::name_data()
{
    QTest::addColumn<QString>("name");
    QTest::newRow("null") << QString();
    QTest::newRow("foo") << QString("foo");
}

// public QString name() const
void tst_OpenSearchEngine::name()
{
    QFETCH(QString, name);

    SubOpenSearchEngine engine;

    QSignalSpy spy0(&engine, SIGNAL(imageChanged()));
    QSignalSpy spy1(&engine, SIGNAL(suggestions(QStringList const&)));

    engine.setName(name);
    QCOMPARE(engine.name(), name);
    QCOMPARE(engine.property("name").toString(), name);
    engine.setProperty("name", QString());
    QCOMPARE(engine.property("name").toString(), QString());

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy1.count(), 0);
}

// public bool operator<(OpenSearchEngine const &other) const
void tst_OpenSearchEngine::operatorlessthan()
{
    SubOpenSearchEngine engine1;
    engine1.setName("a");
    SubOpenSearchEngine engine2;
    engine2.setName("b");

    QVERIFY(engine1 < engine2);
}

Q_DECLARE_METATYPE(Parameters)
void tst_OpenSearchEngine::operatorequal_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("description");
    QTest::addColumn<QString>("imageUrl");
    QTest::addColumn<QString>("searchUrlTemplate");
    QTest::addColumn<QString>("suggestionsUrlTemplate");
    QTest::addColumn<Parameters>("searchParameters");
    QTest::addColumn<Parameters>("suggestionsParameters");
    QTest::addColumn<bool>("operatorequal");
    QTest::newRow("null") << QString() << QString() << QString() << QString() << QString()
                          << Parameters() << Parameters()
                          << true;
    QTest::newRow("name") << QString("x") << QString() << QString() << QString() << QString()
                          << Parameters() << Parameters()
                          << false;
    QTest::newRow("description") << QString() << QString("x") << QString() << QString() << QString()
                          << Parameters() << Parameters()
                          << false;
    QTest::newRow("imageUrl") << QString() << QString() << QString("x") << QString() << QString()
                          << Parameters() << Parameters()
                          << false;
    QTest::newRow("parameters") << QString() << QString() << QString() << QString() << QString()
                          << (Parameters() << Parameter("a", "b")) << Parameters()
                          << false;
}

// public bool operator==(OpenSearchEngine const &other) const
void tst_OpenSearchEngine::operatorequal()
{
    QFETCH(QString, name);
    QFETCH(QString, description);
    QFETCH(QString, imageUrl);
    QFETCH(QString, searchUrlTemplate);
    QFETCH(QString, suggestionsUrlTemplate);
    QFETCH(Parameters, searchParameters);
    QFETCH(Parameters, suggestionsParameters);
    QFETCH(bool, operatorequal);

    SubOpenSearchEngine engine;
    SubOpenSearchEngine other;
    other.setName(name);
    other.setDescription(description);
    other.setImageUrl(imageUrl);
    other.setSearchUrlTemplate(searchUrlTemplate);
    other.setSuggestionsUrlTemplate(suggestionsUrlTemplate);
    other.setSearchParameters(searchParameters);
    other.setSuggestionsParameters(suggestionsParameters);

    QCOMPARE(engine.operator==(other), operatorequal);
}

void tst_OpenSearchEngine::providesSuggestions_data()
{
    QTest::addColumn<QString>("suggestionsUrlTemplate");
    QTest::addColumn<bool>("providesSuggestions");
    QTest::newRow("false") << QString() << false;
    QTest::newRow("true") << QString("foo") << true;
}

// public bool providesSuggestions() const
void tst_OpenSearchEngine::providesSuggestions()
{
    QFETCH(QString, suggestionsUrlTemplate);
    QFETCH(bool, providesSuggestions);

    SubOpenSearchEngine engine;

    QSignalSpy spy0(&engine, SIGNAL(imageChanged()));
    QSignalSpy spy1(&engine, SIGNAL(suggestions(QStringList const&)));

    engine.setSuggestionsUrlTemplate(suggestionsUrlTemplate);
    QCOMPARE(engine.providesSuggestions(), providesSuggestions);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy1.count(), 0);
}

void tst_OpenSearchEngine::requestSuggestions_data()
{
    QTest::addColumn<QString>("searchTerm");
    QTest::addColumn<QString>("method");
    QTest::addColumn<QNetworkAccessManager::Operation>("lastOperation");
    QTest::newRow("null") << QString() << QString() << QNetworkAccessManager::GetOperation;
    QTest::newRow("foo") << QString("foo") << QString("get") << QNetworkAccessManager::GetOperation;
    QTest::newRow("bar") << QString("bar") << QString("post") << QNetworkAccessManager::PostOperation;
    QTest::newRow("baz") << QString("baz") << QString("put") << QNetworkAccessManager::GetOperation;
}

Q_DECLARE_METATYPE(QNetworkAccessManager::Operation)
// public void requestSuggestions(QString const &searchTerm)
void tst_OpenSearchEngine::requestSuggestions()
{
    QFETCH(QString, searchTerm);
    QFETCH(QString, method);
    QFETCH(QNetworkAccessManager::Operation, lastOperation);

    SuggestionsTestNetworkAccessManager manager;
    SubOpenSearchEngine engine;
    engine.setNetworkAccessManager(&manager);
    engine.setSuggestionsMethod(method);
    engine.setSuggestionsUrlTemplate("http://foobar.baz");
    engine.setSuggestionsParameters(Parameters() << Parameter("a", "b"));

    QVERIFY(engine.providesSuggestions());

    QSignalSpy spy(&engine, SIGNAL(suggestions(QStringList const&)));

    engine.requestSuggestions(searchTerm);

    if (searchTerm.isEmpty()) {
        QTest::qWait(200);
        QCOMPARE(spy.count(), 0);
    } else {
        QTRY_COMPARE(spy.count(), 1);

        QStringList suggestions;
        suggestions << "sears" << "search engines" << "search engine" << "search" << "sears.com" << "seattle times";
        QCOMPARE(spy.at(0).at(0).toStringList(), suggestions);

        QCOMPARE(manager.lastOperation, lastOperation);
        QCOMPARE(manager.lastOutgoingData, lastOperation == QNetworkAccessManager::PostOperation);
    }
}

void tst_OpenSearchEngine::requestSuggestionsCrash()
{
    SuggestionsTestNetworkAccessManager manager;
    SubOpenSearchEngine engine;
    engine.setNetworkAccessManager(&manager);
    engine.setSuggestionsUrlTemplate("http://foobar.baz");

    QVERIFY(engine.providesSuggestions());

    QSignalSpy spy(&engine, SIGNAL(suggestions(QStringList const&)));

    QStringList colors = QColor::colorNames();

    for (int i = 0; i < colors.count(); ++i)
        engine.requestSuggestions(colors.at(i));

    QTRY_COMPARE(spy.count(), 1);

    QStringList suggestions;
    suggestions << "sears" << "search engines" << "search engine" << "search" << "sears.com" << "seattle times";
    QCOMPARE(spy.at(0).at(0).toStringList(), suggestions);
}

void tst_OpenSearchEngine::searchParameters_data()
{
    QTest::addColumn<Parameters>("searchParameters");
    QTest::newRow("null") << QList<Parameter>();
    QTest::newRow("something") << (Parameters() << Parameter("a", "b"));
}

// public QList<Parameter> searchParameters() const
void tst_OpenSearchEngine::searchParameters()
{
    QFETCH(Parameters, searchParameters);

    SubOpenSearchEngine engine;

    QSignalSpy spy0(&engine, SIGNAL(imageChanged()));
    QSignalSpy spy1(&engine, SIGNAL(suggestions(QStringList const&)));

    engine.setSearchParameters(searchParameters);
    QCOMPARE(engine.searchParameters(), searchParameters);
    QCOMPARE(qvariant_cast<Parameters>(engine.property("searchParameters")), searchParameters);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy1.count(), 0);
}

void tst_OpenSearchEngine::searchUrl_data()
{
    QTest::addColumn<QString>("searchTerm");
    QTest::addColumn<QString>("searchUrlTemplate");
    QTest::addColumn<Parameters>("parameters");
    QTest::addColumn<QUrl>("searchUrl");
    QTest::newRow("null") << QString() << QString() << Parameters() << QUrl();
    QTest::newRow("foo") << QString("foo") << QString("http://foobar.baz/?q={searchTerms}")
                    << Parameters() << QUrl(QString("http://foobar.baz/?q=foo"));
    QTest::newRow("empty") << QString() << QString("http://foobar.baz/?q={searchTerms}")
                    << Parameters() << QUrl(QString("http://foobar.baz/?q="));
    QTest::newRow("parameters") << QString("baz") << QString("http://foobar.baz/?q={searchTerms}")
                    << (Parameters() << Parameter("abc", "{searchTerms}") << Parameter("x", "yz"))
                    << QUrl(QString("http://foobar.baz/?q=baz&abc=baz&x=yz"));
}

// public QUrl searchUrl(QString const &searchTerm) const
void tst_OpenSearchEngine::searchUrl()
{
    QFETCH(QString, searchTerm);
    QFETCH(QString, searchUrlTemplate);
    QFETCH(Parameters, parameters);
    QFETCH(QUrl, searchUrl);

    SubOpenSearchEngine engine;
    engine.setSearchParameters(parameters);
    engine.setSearchUrlTemplate(searchUrlTemplate);

    QCOMPARE(engine.searchUrl(searchTerm), searchUrl);
}

void tst_OpenSearchEngine::searchUrlTemplate_data()
{
    QTest::addColumn<QString>("searchUrlTemplate");
    QTest::newRow("null") << QString();
    QTest::newRow("foo") << QString("foo");
}

// public QString searchUrlTemplate() const
void tst_OpenSearchEngine::searchUrlTemplate()
{
    QFETCH(QString, searchUrlTemplate);

    SubOpenSearchEngine engine;

    QSignalSpy spy0(&engine, SIGNAL(imageChanged()));
    QSignalSpy spy1(&engine, SIGNAL(suggestions(QStringList const&)));

    engine.setSearchUrlTemplate(searchUrlTemplate);
    QCOMPARE(engine.property("searchUrlTemplate").toString(), searchUrlTemplate);
    engine.setProperty("searchUrlTemplate", QString());
    QCOMPARE(engine.property("searchUrlTemplate").toString(), QString());
    engine.setProperty("searchUrlTemplate", searchUrlTemplate);
    QCOMPARE(engine.searchUrlTemplate(), searchUrlTemplate);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy1.count(), 0);
}

void tst_OpenSearchEngine::suggestionsParameters_data()
{
    QTest::addColumn<Parameters>("suggestionsParameters");
    QTest::newRow("null") << QList<Parameter>();
    QTest::newRow("something") << (Parameters() << Parameter("a", "b"));
}

// public QList<Parameter> suggestionsParameters() const
void tst_OpenSearchEngine::suggestionsParameters()
{
    QFETCH(Parameters, suggestionsParameters);

    SubOpenSearchEngine engine;

    QSignalSpy spy0(&engine, SIGNAL(imageChanged()));
    QSignalSpy spy1(&engine, SIGNAL(suggestions(QStringList const&)));

    engine.setSuggestionsParameters(suggestionsParameters);
    QCOMPARE(engine.suggestionsParameters(), suggestionsParameters);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy1.count(), 0);
}

void tst_OpenSearchEngine::suggestionsUrl_data()
{
    QTest::addColumn<QString>("searchTerm");
    QTest::addColumn<QString>("suggestionsUrlTemplate");
    QTest::addColumn<Parameters>("parameters");
    QTest::addColumn<QUrl>("suggestionsUrl");
    QTest::newRow("null") << QString() << QString() << Parameters() << QUrl();
    QTest::newRow("foo") << QString("foo") << QString("http://foobar.baz/?q={searchTerms}")
                    << Parameters() << QUrl(QString("http://foobar.baz/?q=foo"));
    QTest::newRow("empty") << QString() << QString("http://foobar.baz/?q={searchTerms}")
                    << Parameters() << QUrl(QString("http://foobar.baz/?q="));
    QTest::newRow("parameters") << QString("baz") << QString("http://foobar.baz/?q={searchTerms}")
                    << (Parameters() << Parameter("a", "bc"))
                    << QUrl(QString("http://foobar.baz/?q=baz&a=bc"));
}

// public QUrl suggestionsUrl(QString const &searchTerm) const
void tst_OpenSearchEngine::suggestionsUrl()
{
    QFETCH(QString, searchTerm);
    QFETCH(QString, suggestionsUrlTemplate);
    QFETCH(Parameters, parameters);
    QFETCH(QUrl, suggestionsUrl);

    SubOpenSearchEngine engine;
    engine.setSuggestionsParameters(parameters);
    engine.setSuggestionsUrlTemplate(suggestionsUrlTemplate);

    QCOMPARE(engine.suggestionsUrl(searchTerm), suggestionsUrl);
}

void tst_OpenSearchEngine::suggestionsUrlTemplate_data()
{
    QTest::addColumn<QString>("suggestionsUrlTemplate");
    QTest::newRow("null") << QString();
    QTest::newRow("foo") << QString("foo");
}

// public QString suggestionsUrlTemplate() const
void tst_OpenSearchEngine::suggestionsUrlTemplate()
{
    QFETCH(QString, suggestionsUrlTemplate);

    SubOpenSearchEngine engine;

    QSignalSpy spy0(&engine, SIGNAL(imageChanged()));
    QSignalSpy spy1(&engine, SIGNAL(suggestions(QStringList const&)));

    engine.setSuggestionsUrlTemplate(suggestionsUrlTemplate);
    QCOMPARE(engine.suggestionsUrlTemplate(), suggestionsUrlTemplate);

    QCOMPARE(spy0.count(), 0);
    QCOMPARE(spy1.count(), 0);
}

void tst_OpenSearchEngine::parseTemplate_data()
{
    QString lang = QLocale().name().replace(QLatin1Char('_'), QLatin1Char('-'));

    QTest::addColumn<QString>("searchTerm");
    QTest::addColumn<QString>("searchTemplate");
    QTest::addColumn<QString>("parseTemplate");
    QTest::addColumn<bool>("valid");
    QTest::newRow("null") << QString() << QString() << QString() << false;
    QTest::newRow("foo") << QString("foo") << QString("http://foobar.baz/?q={searchTerms}")
                    << QString("http://foobar.baz/?q=foo") << true;
    QTest::newRow("allParameters") << QString("bar")
                    << QString("http://foobar.baz/?st={searchTerms}&amp;c={count}"
                               "&amp;si={startIndex}&amp;sp={startPage}&amp;l={language}"
                               "&amp;ie={inputEncoding}&amp;oe={outputEncoding}")
                    << QString("http://foobar.baz/?st=bar&amp;c=20&amp;si=0&amp;"
                               "sp=0&amp;l=%1&amp;ie=UTF-8&amp;oe=UTF-8").arg(lang)
                    << true;
    QTest::newRow("tricky") << QString("{count}") << QString("http://foobar.baz/q={searchTerms}&amp;count={count}")
                    << QString("http://foobar.baz/q=%7Bcount%7D&amp;count=20") << true;
    QTest::newRow("multiple") << QString("abc") << QString("http://foobar.baz/?q={searchTerms}&amp;x={searchTerms}")
                    << QString("http://foobar.baz/?q=abc&amp;x=abc") << true;
    QTest::newRow("referrer") << QString("foo")
                    << QString("http://foobar.baz/?q={searchTerms}&amp;a={source}&amp;b={ref:source}&amp;c={referrer:source?}")
                    << QString("http://foobar.baz/?q=foo&amp;a=tst_opensearchengine"
                               "&amp;b=tst_opensearchengine&amp;c=tst_opensearchengine")
                    << true;
    QTest::newRow("inputEncoding") << QString("c++") << QString("http://foobar.baz/?q={searchTerms}")
                    << QString("http://foobar.baz/?q=c%2B%2B") << true;
}

// protected QString parseTemplate(QString const &searchTerm, QString const &searchTemplate) const
void tst_OpenSearchEngine::parseTemplate()
{
    QFETCH(QString, searchTerm);
    QFETCH(QString, searchTemplate);
    QFETCH(QString, parseTemplate);
    QFETCH(bool, valid);

    SubOpenSearchEngine engine;
    QString url = engine.call_parseTemplate(searchTerm, searchTemplate);
    QCOMPARE(url, parseTemplate);
    QCOMPARE(QUrl(url).isValid(), valid);
}

void tst_OpenSearchEngine::languageCodes_data()
{
    QTest::addColumn<QString>("languageCode");
    QTest::addColumn<QString>("url");

    QTest::newRow("es") << QString("es") << QString("http://foobar.baz/?l=es-ES");
    QTest::newRow("pt_BR") << QString("pt_BR") << QString("http://foobar.baz/?l=pt-BR");
}

void tst_OpenSearchEngine::languageCodes()
{
    QFETCH(QString, languageCode);
    QFETCH(QString, url);

    QLocale::setDefault(QLocale(languageCode));

    SubOpenSearchEngine engine;
    QCOMPARE(engine.call_parseTemplate(QString("foo"), QString("http://foobar.baz/?l={language}")), url);
}

void tst_OpenSearchEngine::requestMethods()
{
    SubOpenSearchEngine engine;

    QCOMPARE(engine.searchMethod(), QString("get"));
    QCOMPARE(engine.suggestionsMethod(), QString("get"));

    engine.setSearchMethod("post");
    QCOMPARE(engine.searchMethod(), QString("post"));
    QCOMPARE(engine.suggestionsMethod(), QString("get"));

    engine.setSearchMethod("get");
    QCOMPARE(engine.searchMethod(), QString("get"));
    QCOMPARE(engine.suggestionsMethod(), QString("get"));

    engine.setSuggestionsMethod("PoSt");
    QCOMPARE(engine.searchMethod(), QString("get"));
    QCOMPARE(engine.suggestionsMethod(), QString("post"));

    engine.setSearchMethod("foo");
    QCOMPARE(engine.searchMethod(), QString("get"));
    QCOMPARE(engine.suggestionsMethod(), QString("post"));

    engine.setSuggestionsMethod("bar");
    QCOMPARE(engine.searchMethod(), QString("get"));
    QCOMPARE(engine.suggestionsMethod(), QString("post"));
}

void tst_OpenSearchEngine::delegate()
{
    SubOpenSearchEngine engine;
    engine.setName(QString("foo"));
    engine.setDescription(QString("bar"));
    engine.setSearchUrlTemplate(QString("http://foobar.baz/?q={searchTerms}"));

    QCOMPARE(engine.delegate(), (Delegate*)0);
    engine.setDelegate(0);
    QCOMPARE(engine.delegate(), (Delegate*)0);
    engine.requestSearchResults(QString("baz"));

    Delegate delegate;
    engine.setDelegate(&delegate);
    QCOMPARE(engine.delegate(), &delegate);
    engine.requestSearchResults(QString("baz"));

    QCOMPARE(delegate.callsCount, 1);
    QCOMPARE(delegate.lastOperation, QNetworkAccessManager::GetOperation);
    QCOMPARE(delegate.lastData, QByteArray());
    QNetworkRequest request(QUrl(engine.call_parseTemplate(QString("baz"), engine.searchUrlTemplate())));
    QCOMPARE(delegate.lastRequest, request);

    QUrlQuery urlQuery(delegate.lastRequest.url().query());
    QVERIFY(urlQuery.hasQueryItem("q"));
    QCOMPARE(urlQuery.queryItemValue("q"), QString("baz"));

    engine.setSearchParameters(Parameters() << Parameter("a", "b") << Parameter("b", "c"));
    engine.requestSearchResults(QString("baz"));

    QCOMPARE(delegate.callsCount, 2);
    QCOMPARE(delegate.lastOperation, QNetworkAccessManager::GetOperation);
    QCOMPARE(delegate.lastData, QByteArray());

    QUrlQuery q2(delegate.lastRequest.url().query());
    QVERIFY(q2.hasQueryItem("a"));
    QCOMPARE(q2.queryItemValue("a"), QString("b"));
    QVERIFY(q2.hasQueryItem("b"));
    QCOMPARE(q2.queryItemValue("b"), QString("c"));
    QVERIFY(q2.hasQueryItem("q"));
    QCOMPARE(q2.queryItemValue("q"), QString("baz"));

    QUrl url(engine.call_parseTemplate(QString("baz"), engine.searchUrlTemplate()));
    QCOMPARE(delegate.lastRequest.url().toString(QUrl::RemoveQuery), url.toString(QUrl::RemoveQuery));

    engine.setSearchMethod(QString("post"));
    engine.requestSearchResults(QString("baz"));

    QCOMPARE(delegate.callsCount, 3);
    QCOMPARE(delegate.lastOperation, QNetworkAccessManager::PostOperation);
    request = QNetworkRequest(QUrl(engine.call_parseTemplate(QString("baz"), engine.searchUrlTemplate())));
    QCOMPARE(delegate.lastRequest, request);

    QUrlQuery q3(delegate.lastRequest.url().query());
    QVERIFY(q3.hasQueryItem("q"));
    QCOMPARE(q3.queryItemValue("q"), QString("baz"));

    QVERIFY(!delegate.lastData.isEmpty());
    QStringList query = QString(delegate.lastData).split('&');
    QCOMPARE(query.count(), 2);
    QCOMPARE(query, QStringList() << "a=b" << "b=c");
}

QTEST_MAIN(tst_OpenSearchEngine)
#include "tst_opensearchengine.moc"

