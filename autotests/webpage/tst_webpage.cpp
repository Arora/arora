/*
 * Copyright 2009 Benjamin C. Meyer <ben@meyerhome.net>
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
#include <QtWebKit/QtWebKit>
#include <QtNetwork/QtNetwork>
#include <QWebFrame>

#include <webpage.h>
#include "qtest_arora.h"

class tst_WebPage : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void webpage_data();
    void webpage();

    void loadSettings_data();
    void loadSettings();
    void webPluginFactory_data();
    void webPluginFactory();
    void acceptNavigationRequest_data();
    void acceptNavigationRequest();
    void createPlugin_data();
    void createPlugin();
    void createWindow_data();
    void createWindow();
    void handleUnsupportedContent();
    void linkedResources();
    void javaScriptObjects_data();
    void javaScriptObjects();
    void userAgent();
};

// Subclass that exposes the protected functions.
class SubWebPage : public WebPage
{
public:
    QString call_userAgentForUrl(const QUrl &url) const
        { return SubWebPage::userAgentForUrl(url); }

    void call_aboutToLoadUrl(QUrl const &url)
        { return SubWebPage::aboutToLoadUrl(url); }

    bool call_acceptNavigationRequest(QWebFrame *frame, QNetworkRequest const &request, NavigationType type)
        { return SubWebPage::acceptNavigationRequest(frame, request, type); }

    QObject *call_createPlugin(QString const &classId, QUrl const &url, QStringList const &paramNames, QStringList const &paramValues)
        { return SubWebPage::createPlugin(classId, url, paramNames, paramValues); }

    QWebPage *call_createWindow(QWebPage::WebWindowType type)
        { return SubWebPage::createWindow(type); }
};

// This will be called before the first test function is executed.
// It is only called once.
void tst_WebPage::initTestCase()
{
    QCoreApplication::setApplicationName("tst_webpage");

    QDesktopServices::setUrlHandler(QLatin1String("mailto"), this, "openUrl");
    QDesktopServices::setUrlHandler(QLatin1String("ftp"), this, "openUrl");
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_WebPage::cleanupTestCase()
{
    QSettings settings;
    settings.setValue("userAgent", QString());
}

// This will be called before each test function is executed.
void tst_WebPage::init()
{
}

// This will be called after every test function.
void tst_WebPage::cleanup()
{
}

void tst_WebPage::webpage_data()
{
}

void tst_WebPage::webpage()
{
    SubWebPage page;
    page.loadSettings();
    QVERIFY(page.webPluginFactory());
    page.call_aboutToLoadUrl(QUrl());
    QCOMPARE(page.call_acceptNavigationRequest((QWebFrame*)0, QNetworkRequest(), QWebPage::NavigationTypeLinkClicked), true);
    QCOMPARE(page.call_createPlugin(QString(), QUrl(), QStringList(), QStringList()), (QObject*)0);
    QCOMPARE(page.call_createWindow(QWebPage::WebBrowserWindow), (QWebPage*)0);
}

void tst_WebPage::loadSettings_data()
{
    QTest::addColumn<int>("foo");
    QTest::newRow("0") << 0;
    QTest::newRow("-1") << -1;
}

// public void loadSettings()
void tst_WebPage::loadSettings()
{
#if 0
    QFETCH(int, foo);

    SubWebPage page;

    QSignalSpy spy0(&page, SIGNAL(aboutToLoadUrl(QUrl const&)));

    page.loadSettings();

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

// Q_DECLARE_METATYPE(WebPluginFactory*)
void tst_WebPage::webPluginFactory_data()
{
#if 0
    QTest::addColumn<WebPluginFactory*>("webPluginFactory");
    QTest::newRow("null") << WebPluginFactory*();
#endif
}

// public WebPluginFactory *webPluginFactory()
void tst_WebPage::webPluginFactory()
{
#if 0
    QFETCH(WebPluginFactory*, webPluginFactory);

    SubWebPage page;

    QSignalSpy spy0(&page, SIGNAL(aboutToLoadUrl(QUrl const&)));

    QCOMPARE(page.webPluginFactory(), webPluginFactory);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

Q_DECLARE_METATYPE(QWebPage::WebWindowType)
Q_DECLARE_METATYPE(QWebPage::NavigationType)
Q_DECLARE_METATYPE(Qt::MouseButton)
Q_DECLARE_METATYPE(Qt::KeyboardModifier)
void tst_WebPage::acceptNavigationRequest_data()
{
    QTest::addColumn<Qt::MouseButton>("pressedButton");
    QTest::addColumn<Qt::KeyboardModifier>("pressedKeys");
    QTest::addColumn<bool>("validFrame");
    QTest::addColumn<QNetworkRequest>("request");
    QTest::addColumn<QWebPage::NavigationType>("type");
    QTest::addColumn<bool>("acceptNavigationRequest");
    QTest::addColumn<int>("spyCount");

    QTest::newRow("null-noframe") << Qt::NoButton << Qt::NoModifier << false << QNetworkRequest() << QWebPage::NavigationTypeLinkClicked << true << 0;
    QTest::newRow("null-frame")   << Qt::NoButton << Qt::NoModifier << true << QNetworkRequest() << QWebPage::NavigationTypeLinkClicked << true << 1;

    QTest::newRow("mailto-0") << Qt::NoButton << Qt::NoModifier << true << QNetworkRequest(QUrl("mailto:foo@bar.com")) << QWebPage::NavigationTypeLinkClicked << false << 0;
    QTest::newRow("mailto-1") << Qt::NoButton << Qt::NoModifier << false << QNetworkRequest(QUrl("mailto:foo@bar.com")) << QWebPage::NavigationTypeLinkClicked << false << 0;
    QTest::newRow("ftp-0") << Qt::NoButton << Qt::NoModifier << true << QNetworkRequest(QUrl("ftp:foo@bar.com")) << QWebPage::NavigationTypeLinkClicked << false << 0;
    QTest::newRow("ftp-1") << Qt::NoButton << Qt::NoModifier << false << QNetworkRequest(QUrl("ftp:foo@bar.com")) << QWebPage::NavigationTypeLinkClicked << false << 0;


    QTest::newRow("normal-0") << Qt::NoButton << Qt::NoModifier << false << QNetworkRequest(QUrl("http://www.foo.com")) << QWebPage::NavigationTypeLinkClicked << true << 0;
    QTest::newRow("normal-1") << Qt::NoButton << Qt::NoModifier << true << QNetworkRequest(QUrl("http://www.foo.com")) << QWebPage::NavigationTypeLinkClicked << true << 1;

    QTest::newRow("midclick-0") << Qt::MidButton << Qt::NoModifier << true << QNetworkRequest(QUrl("http://www.foo.com")) << QWebPage::NavigationTypeLinkClicked << false << 0;
    QTest::newRow("midclick-1") << Qt::MidButton << Qt::ShiftModifier << true << QNetworkRequest(QUrl("http://www.foo.com")) << QWebPage::NavigationTypeLinkClicked << false << 0;
    QTest::newRow("midclick-2") << Qt::MidButton << Qt::AltModifier << true << QNetworkRequest(QUrl("http://www.foo.com")) << QWebPage::NavigationTypeLinkClicked << false << 0;
}

// protected bool acceptNavigationRequest(QWebFrame *frame, QNetworkRequest const &request, NavigationType type)
void tst_WebPage::acceptNavigationRequest()
{
    QFETCH(Qt::MouseButton, pressedButton);
    QFETCH(Qt::KeyboardModifier, pressedKeys);
    QFETCH(bool, validFrame);
    QFETCH(QNetworkRequest, request);
    QFETCH(QWebPage::NavigationType, type);
    QFETCH(bool, acceptNavigationRequest);
    QFETCH(int, spyCount);

    BrowserApplication::instance()->setEventMouseButtons(pressedButton);
    BrowserApplication::instance()->setEventKeyboardModifiers(pressedKeys);
    SubWebPage page;
    QSignalSpy spy0(&page, SIGNAL(aboutToLoadUrl(QUrl const&)));

    QWebFrame *frame = validFrame ? page.mainFrame() : (QWebFrame*)0;
    QCOMPARE(page.call_acceptNavigationRequest(frame, request, type), acceptNavigationRequest);

    QCOMPARE(spy0.count(), spyCount);
    BrowserApplication::instance()->setEventMouseButtons(Qt::NoButton);
    BrowserApplication::instance()->setEventKeyboardModifiers(Qt::NoModifier);
}

Q_DECLARE_METATYPE(QStringList)
Q_DECLARE_METATYPE(QObject*)
void tst_WebPage::createPlugin_data()
{
#if 0
    QTest::addColumn<QString>("classId");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QStringList>("paramNames");
    QTest::addColumn<QStringList>("paramValues");
    QTest::addColumn<QObject*>("createPlugin");
    QTest::newRow("null") << QString() << QUrl() << QStringList() << QStringList() << QObject*();
#endif
}

// protected QObject *createPlugin(QString const &classId, QUrl const &url, QStringList const &paramNames, QStringList const &paramValues)
void tst_WebPage::createPlugin()
{
#if 0
    QFETCH(QString, classId);
    QFETCH(QUrl, url);
    QFETCH(QStringList, paramNames);
    QFETCH(QStringList, paramValues);
    QFETCH(QObject*, createPlugin);

    SubWebPage page;

    QSignalSpy spy0(&page, SIGNAL(aboutToLoadUrl(QUrl const&)));

    QCOMPARE(page.call_createPlugin(classId, url, paramNames, paramValues), createPlugin);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

Q_DECLARE_METATYPE(QWebPage*)
void tst_WebPage::createWindow_data()
{
#if 0
    QTest::addColumn<QWebPage::WebWindowType>("type");
    QTest::addColumn<QWebPage*>("createWindow");
    QTest::newRow("null") << QWebPage::WebWindowType() << QWebPage*();
#endif
}

// protected QWebPage *createWindow(QWebPage::WebWindowType type)
void tst_WebPage::createWindow()
{
#if 0
    QFETCH(QWebPage::WebWindowType, type);
    QFETCH(QWebPage*, createWindow);

    SubWebPage page;

    QSignalSpy spy0(&page, SIGNAL(aboutToLoadUrl(QUrl const&)));

    QCOMPARE(page.call_createWindow(type), createWindow);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_WebPage::handleUnsupportedContent()
{
    SubWebPage page;
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));
    page.mainFrame()->load(QUrl("http://exampletesttesttesttesttesttes.com/test.html"));
    QTRY_COMPARE(spy.count(), 1);
}

void tst_WebPage::linkedResources()
{
    SubWebPage page;

    QString html = "<html>"
        "<head>"
            "<link rel=\"stylesheet\" type=\"text/css\" href=\"styles/common.css\" />"
            "<link rel=\"alternate\" type=\"application/rss+xml\" href=\"./rss.xml\" />"
            "<link rel=\"alternate\" type=\"application/atom+xml\" href=\"../atom.xml\" title=\"Feed\" />"
            "<link rel=\"search\" type=\"application/opensearchdescription+xml\" href=\"http://external.foo/search.xml\" />"
        "</head>"
        "<body>"
            "<link rel=\"stylesheet\" type=\"text/css\" href=\"styles/ie.css\" />"
        "</body>"
    "</html>";

    page.mainFrame()->setHtml(html, QUrl("http://foobar.baz/foo/"));

    QList<WebPageLinkedResource> resources = page.linkedResources();
    QCOMPARE(resources.count(), 4);

    QCOMPARE(resources.at(0).rel, QString("stylesheet"));
    QCOMPARE(resources.at(0).type, QString("text/css"));
    QCOMPARE(resources.at(0).href, QUrl("http://foobar.baz/foo/styles/common.css"));
    QCOMPARE(resources.at(0).title, QString());

    QCOMPARE(resources.at(1).rel, QString("alternate"));
    QCOMPARE(resources.at(1).type, QString("application/rss+xml"));
    QCOMPARE(resources.at(1).href, QUrl("http://foobar.baz/foo/rss.xml"));

    QCOMPARE(resources.at(2).href, QUrl("http://foobar.baz/atom.xml"));
    QCOMPARE(resources.at(2).title, QString("Feed"));

    QCOMPARE(resources.at(3).rel, QString("search"));
    QCOMPARE(resources.at(3).type, QString("application/opensearchdescription+xml"));
    QCOMPARE(resources.at(3).href, QUrl("http://external.foo/search.xml"));

    QString js = "var base = document.createElement('base');"
                 "base.setAttribute('href', 'http://barbaz.foo/bar/');"
                 "document.getElementsByTagName('head')[0].appendChild(base);";

    page.mainFrame()->evaluateJavaScript(js);

    resources = page.linkedResources();
    QCOMPARE(resources.count(), 4);

    QCOMPARE(resources.at(0).href, QUrl("http://barbaz.foo/bar/styles/common.css"));
    QCOMPARE(resources.at(1).href, QUrl("http://barbaz.foo/bar/rss.xml"));
    QCOMPARE(resources.at(2).href, QUrl("http://barbaz.foo/atom.xml"));
    QCOMPARE(resources.at(3).href, QUrl("http://external.foo/search.xml"));
}

void tst_WebPage::javaScriptObjects_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<bool>("windowExternal");
    QTest::addColumn<bool>("windowArora");

    QTest::newRow("qrc:/notfound.html") << QUrl("qrc:/notfound.html") << true << false;
    QTest::newRow("qrc:/startpage.html") << QUrl("qrc:/startpage.html") << true << true;
}

void tst_WebPage::javaScriptObjects()
{
    QFETCH(QUrl, url);
    QFETCH(bool, windowExternal);
    QFETCH(bool, windowArora);

    SubWebPage page;
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));
    page.mainFrame()->load(url);
    QTRY_COMPARE(spy.count(), 1);

    QVariant windowExternalVariant = page.mainFrame()->evaluateJavaScript(QLatin1String("window.external"));
    QVariant windowAroraVariant = page.mainFrame()->evaluateJavaScript(QLatin1String("window.arora"));

    QCOMPARE(windowExternal, !windowExternalVariant.isNull());
    QCOMPARE(windowArora, !windowAroraVariant.isNull());
}

void tst_WebPage::userAgent()
{
    QSettings settings;
    settings.setValue("userAgent", QString());
    SubWebPage page;
    QString defaultUserAgent = page.call_userAgentForUrl(QUrl());
    QVERIFY(!defaultUserAgent.isEmpty());
    QVERIFY(defaultUserAgent.contains("tst_webpage"));
    settings.setValue("userAgent", "ben");
    page.loadSettings();
    QString customUserAgent = page.call_userAgentForUrl(QUrl());
    QVERIFY(!customUserAgent.isEmpty());
    QCOMPARE(customUserAgent, QString("ben"));
}


QTEST_MAIN(tst_WebPage)
#include "tst_webpage.moc"

