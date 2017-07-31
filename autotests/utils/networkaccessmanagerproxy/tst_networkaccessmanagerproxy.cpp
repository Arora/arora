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

#include <qtest.h>

#include <networkaccessmanagerproxy.h>
#include <webpageproxy.h>

#include <qnetworkcookie.h>
#include <qnetworkrequest.h>
#include <qnetworkreply.h>
#include <QNetworkCookieJar>

class tst_NetworkAccessManagerProxy : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void networkaccessmanagerproxy_data();
    void networkaccessmanagerproxy();

    void primaryNetworkAccessManager();
    void webPage();

    void createRequest_data();
    void createRequest();
};

// Subclass that exposes the protected functions.
class SubNetworkAccessManagerProxy : public NetworkAccessManagerProxy
{
public:
    QNetworkReply *call_createRequest(QNetworkAccessManager::Operation op, QNetworkRequest const &request, QIODevice *outgoingData = 0)
        { return SubNetworkAccessManagerProxy::createRequest(op, request, outgoingData); }
};

// This will be called before the first test function is executed.
// It is only called once.
void tst_NetworkAccessManagerProxy::initTestCase()
{
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_NetworkAccessManagerProxy::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_NetworkAccessManagerProxy::init()
{
}

// This will be called after every test function.
void tst_NetworkAccessManagerProxy::cleanup()
{
}

void tst_NetworkAccessManagerProxy::networkaccessmanagerproxy_data()
{
}

void tst_NetworkAccessManagerProxy::networkaccessmanagerproxy()
{
    SubNetworkAccessManagerProxy proxy;
    QVERIFY(proxy.call_createRequest(QNetworkAccessManager::GetOperation,
            QNetworkRequest(),
            (QIODevice*)0));
}

// public void setPrimaryNetworkAccessManager(NetworkAccessManagerProxy *primaryManager)
void tst_NetworkAccessManagerProxy::primaryNetworkAccessManager()
{
    SubNetworkAccessManagerProxy proxy;

    NetworkAccessManagerProxy primaryManager;

    proxy.setPrimaryNetworkAccessManager(&primaryManager);
    QCOMPARE(&primaryManager, proxy.primaryNetworkAccessManager());
    QVERIFY(primaryManager.cookieJar()->parent() == &primaryManager);
}

// public void setWebPage(WebPageProxy *page)
void tst_NetworkAccessManagerProxy::webPage()
{
    SubNetworkAccessManagerProxy proxy;

    WebPageProxy webPage;
    proxy.setWebPage(&webPage);
    QCOMPARE(&webPage, proxy.webPage());
}

void tst_NetworkAccessManagerProxy::createRequest_data()
{
    QTest::addColumn<bool>("primary");
    QTest::newRow("true") << true;
    QTest::newRow("false") << false;
}

// protected QNetworkReply *createRequest(QNetworkAccessManager::Operation op, QNetworkRequest const &request, QIODevice *outgoingData = 0)
void tst_NetworkAccessManagerProxy::createRequest()
{
    QFETCH(bool, primary);

    SubNetworkAccessManagerProxy proxy;
    SubNetworkAccessManagerProxy primaryManager;
    WebPageProxy webPage;
    if (!primary) {
        proxy.setPrimaryNetworkAccessManager(&primaryManager);
        proxy.setWebPage(&webPage);
    }

    QIODevice *outgoingData = 0;
    QNetworkRequest request;
    QNetworkReply *reply = proxy.call_createRequest(QNetworkAccessManager::GetOperation, request, outgoingData);
    QVERIFY(reply);
    if (primary)
        QCOMPARE(reply->request(), request);
    else
        QVERIFY(reply->request() != request);
}

QTEST_MAIN(tst_NetworkAccessManagerProxy)
#include "tst_networkaccessmanagerproxy.moc"

