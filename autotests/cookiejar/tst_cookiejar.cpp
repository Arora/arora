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
#include <cookiejar.h>

class tst_CookieJar : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void cookiejar_data();
    void cookiejar();

    void acceptPolicy_data();
    void acceptPolicy();
    void allowedCookies_data();
    void allowedCookies();
    void allowForSessionCookies_data();
    void allowForSessionCookies();
    void blockedCookies_data();
    void blockedCookies();
    void clear_data();
    void clear();
    void cookiesForUrl_data();
    void cookiesForUrl();
    void keepPolicy_data();
    void keepPolicy();
    void loadSettings_data();
    void loadSettings();
    void setAcceptPolicy_data();
    void setAcceptPolicy();
    void setAllowedCookies_data();
    void setAllowedCookies();
    void setAllowForSessionCookies_data();
    void setAllowForSessionCookies();
    void setBlockedCookies_data();
    void setBlockedCookies();
    void setCookiesFromUrl_data();
    void setCookiesFromUrl();
    void setKeepPolicy_data();
    void setKeepPolicy();
    void cookiesChanged_data();
    void cookiesChanged();
    void isOnDomainList_data();
    void isOnDomainList();
};

// Subclass that exposes the protected functions.
class SubCookieJar : public CookieJar
{
public:
    void call_cookiesChanged()
        { return SubCookieJar::cookiesChanged(); }

    static bool call_isOnDomainList(QStringList const &list, QString const &domain)
        { return SubCookieJar::isOnDomainList(list, domain); }
};

// This will be called before the first test function is executed.
// It is only called once.
void tst_CookieJar::initTestCase()
{
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_CookieJar::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_CookieJar::init()
{
}

// This will be called after every test function.
void tst_CookieJar::cleanup()
{
}

void tst_CookieJar::cookiejar_data()
{
}

void tst_CookieJar::cookiejar()
{
    SubCookieJar jar;
#if 0
    QCOMPARE(jar.acceptPolicy(), CookieJar::AcceptPolicy);
    QCOMPARE(jar.allowedCookies(), QStringList);
    QCOMPARE(jar.allowForSessionCookies(), QStringList);
    QCOMPARE(jar.blockedCookies(), QStringList);
    jar.clear();
    QCOMPARE(jar.cookiesForUrl(QUrl()), QList<QNetworkCookie>());
    QCOMPARE(jar.keepPolicy(), CookieJar::KeepPolicy);
    jar.loadSettings();
    jar.setAcceptPolicy(CookieJar::AcceptPolicy);
    jar.setAllowedCookies(QStringList());
    jar.setAllowForSessionCookies(QStringList());
    jar.setBlockedCookies(QStringList());
    QCOMPARE(jar.setCookiesFromUrl(QList<QNetworkCookie>(), QUrl()), false);
    jar.setKeepPolicy(CookieJar::KeepPolicy);
    jar.call_cookiesChanged();
    QCOMPARE(jar.call_isOnDomainList(QStringList(), QString()), false);
#endif
    QSKIP("Test is not implemented.");
}

Q_DECLARE_METATYPE(CookieJar::AcceptPolicy)
void tst_CookieJar::acceptPolicy_data()
{
#if 0
    QTest::addColumn<CookieJar::AcceptPolicy>("acceptPolicy");
    QTest::newRow("null") << CookieJar::AcceptPolicy();
#endif
}

// public CookieJar::AcceptPolicy acceptPolicy() const
void tst_CookieJar::acceptPolicy()
{
#if 0
    QFETCH(CookieJar::AcceptPolicy, acceptPolicy);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    QCOMPARE(jar.acceptPolicy(), acceptPolicy);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_CookieJar::allowedCookies_data()
{
#if 0
    QTest::addColumn<QStringList>("allowedCookies");
    QTest::newRow("null") << QStringList();
#endif
}

// public QStringList allowedCookies() const
void tst_CookieJar::allowedCookies()
{
#if 0
    QFETCH(QStringList, allowedCookies);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    QCOMPARE(jar.allowedCookies(), allowedCookies);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_CookieJar::allowForSessionCookies_data()
{
#if 0
    QTest::addColumn<QStringList>("allowForSessionCookies");
    QTest::newRow("null") << QStringList();
#endif
}

// public QStringList allowForSessionCookies() const
void tst_CookieJar::allowForSessionCookies()
{
#if 0
    QFETCH(QStringList, allowForSessionCookies);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    QCOMPARE(jar.allowForSessionCookies(), allowForSessionCookies);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_CookieJar::blockedCookies_data()
{
#if 0
    QTest::addColumn<QStringList>("blockedCookies");
    QTest::newRow("null") << QStringList();
#endif
}

// public QStringList blockedCookies() const
void tst_CookieJar::blockedCookies()
{
#if 0
    QFETCH(QStringList, blockedCookies);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    QCOMPARE(jar.blockedCookies(), blockedCookies);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_CookieJar::clear_data()
{
    QTest::addColumn<int>("foo");
    QTest::newRow("0") << 0;
    QTest::newRow("-1") << -1;
}

// public void clear()
void tst_CookieJar::clear()
{
#if 0
    QFETCH(int, foo);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    jar.clear();

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_CookieJar::cookiesForUrl_data()
{
#if 0
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QList<QNetworkCookie>>("cookiesForUrl");
    QTest::newRow("null") << QUrl() << QList<QNetworkCookie>();
#endif
}

// public QList<QNetworkCookie> cookiesForUrl(QUrl const &url) const
void tst_CookieJar::cookiesForUrl()
{
#if 0
    QFETCH(QUrl, url);
    QFETCH(QList<QNetworkCookie>, cookiesForUrl);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    QCOMPARE(jar.cookiesForUrl(url), cookiesForUrl);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

Q_DECLARE_METATYPE(CookieJar::KeepPolicy)
void tst_CookieJar::keepPolicy_data()
{
#if 0
    QTest::addColumn<CookieJar::KeepPolicy>("keepPolicy");
    QTest::newRow("null") << CookieJar::KeepPolicy();
#endif
}

// public CookieJar::KeepPolicy keepPolicy() const
void tst_CookieJar::keepPolicy()
{
#if 0
    QFETCH(CookieJar::KeepPolicy, keepPolicy);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    QCOMPARE(jar.keepPolicy(), keepPolicy);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_CookieJar::loadSettings_data()
{
    QTest::addColumn<int>("foo");
    QTest::newRow("0") << 0;
    QTest::newRow("-1") << -1;
}

// public void loadSettings()
void tst_CookieJar::loadSettings()
{
#if 0
    QFETCH(int, foo);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    jar.loadSettings();

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_CookieJar::setAcceptPolicy_data()
{
#if 0
    QTest::addColumn<CookieJar::AcceptPolicy>("policy");
    QTest::newRow("null") << CookieJar::AcceptPolicy();
#endif
}

// public void setAcceptPolicy(CookieJar::AcceptPolicy policy)
void tst_CookieJar::setAcceptPolicy()
{
#if 0
    QFETCH(CookieJar::AcceptPolicy, policy);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    jar.setAcceptPolicy(policy);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_CookieJar::setAllowedCookies_data()
{
#if 0
    QTest::addColumn<QStringList>("list");
    QTest::newRow("null") << QStringList();
#endif
}

// public void setAllowedCookies(QStringList const &list)
void tst_CookieJar::setAllowedCookies()
{
#if 0
    QFETCH(QStringList, list);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    jar.setAllowedCookies(list);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_CookieJar::setAllowForSessionCookies_data()
{
#if 0
    QTest::addColumn<QStringList>("list");
    QTest::newRow("null") << QStringList();
#endif
}

// public void setAllowForSessionCookies(QStringList const &list)
void tst_CookieJar::setAllowForSessionCookies()
{
#if 0
    QFETCH(QStringList, list);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    jar.setAllowForSessionCookies(list);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_CookieJar::setBlockedCookies_data()
{
#if 0
    QTest::addColumn<QStringList>("list");
    QTest::newRow("null") << QStringList();
#endif
}

// public void setBlockedCookies(QStringList const &list)
void tst_CookieJar::setBlockedCookies()
{
#if 0
    QFETCH(QStringList, list);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    jar.setBlockedCookies(list);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_CookieJar::setCookiesFromUrl_data()
{
#if 0
    QTest::addColumn<QList<QNetworkCookie>>("cookieList");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<bool>("setCookiesFromUrl");
    QTest::newRow("null") << QList<QNetworkCookie>() << QUrl() << false;
#endif
}

// public bool setCookiesFromUrl(QList<QNetworkCookie> const &cookieList, QUrl const &url)
void tst_CookieJar::setCookiesFromUrl()
{
#if 0
    QFETCH(QList<QNetworkCookie>, cookieList);
    QFETCH(QUrl, url);
    QFETCH(bool, setCookiesFromUrl);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    QCOMPARE(jar.setCookiesFromUrl(cookieList, url), setCookiesFromUrl);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_CookieJar::setKeepPolicy_data()
{
#if 0
    QTest::addColumn<CookieJar::KeepPolicy>("policy");
    QTest::newRow("null") << CookieJar::KeepPolicy();
#endif
}

// public void setKeepPolicy(CookieJar::KeepPolicy policy)
void tst_CookieJar::setKeepPolicy()
{
#if 0
    QFETCH(CookieJar::KeepPolicy, policy);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    jar.setKeepPolicy(policy);

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_CookieJar::cookiesChanged_data()
{
    QTest::addColumn<int>("foo");
    QTest::newRow("0") << 0;
    QTest::newRow("-1") << -1;
}

// protected void cookiesChanged()
void tst_CookieJar::cookiesChanged()
{
#if 0
    QFETCH(int, foo);

    SubCookieJar jar;

    QSignalSpy spy0(&jar, SIGNAL(cookiesChanged()));

    jar.call_cookiesChanged();

    QCOMPARE(spy0.count(), 0);
#endif
    QSKIP("Test is not implemented.");
}

void tst_CookieJar::isOnDomainList_data()
{
    QTest::addColumn<QStringList>("list");
    QTest::addColumn<QString>("domain");
    QTest::addColumn<bool>("isOnDomainList");

    QTest::newRow("null") << QStringList() << QString() << false;
    QTest::newRow("exact-match") << (QStringList() << "foo.com") << "foo.com" << true;

    QTest::newRow("check-0") << (QStringList() << "foo.com") << "foo.com" << true;
    QTest::newRow("check-1") << (QStringList() << "foo.com") << ".foo.com" << true;
    QTest::newRow("check-2") << (QStringList() << ".foo.com") << "foo.com" << true;
    QTest::newRow("check-3") << (QStringList() << ".foo.com") << ".foo.com" << true;
    QTest::newRow("check-4") << (QStringList() << "foo.com") << "abcfoo.com" << false;
    QTest::newRow("check-5") << (QStringList() << "foo.com") << "abc.foo.com" << true;
    QTest::newRow("check-6") << (QStringList() << ".foo.com") << "abcfoo.com" << false;
    QTest::newRow("check-7") << (QStringList() << ".foo.com") << "abc.foo.com" << true;

    QTest::newRow("check-4") << (QStringList() << "abc.foo.com") << "foo.com" << false;
    QTest::newRow("check-5") << (QStringList() << "abc.foo.com") << ".foo.com" << false;


    QTest::newRow("edgecheck-0") << (QStringList() << "") << ".foo.com" << false;
    QTest::newRow("edgecheck-1") << (QStringList() << "") << "foo.com" << false;
    QTest::newRow("edgecheck-2") << (QStringList() << ".") << ".foo.com" << false;
    QTest::newRow("edgecheck-3") << (QStringList() << ".") << "foo.com" << false;
    QTest::newRow("edgecheck-4") << (QStringList() << "abc.foo.com") << "" << false;
    QTest::newRow("edgecheck-5") << (QStringList() << "a") << "ab" << false;
}

// protected static bool isOnDomainList(QStringList const &list, QString const &domain)
void tst_CookieJar::isOnDomainList()
{
    QFETCH(QStringList, list);
    QFETCH(QString, domain);
    QFETCH(bool, isOnDomainList);

    SubCookieJar jar;

    QCOMPARE(jar.call_isOnDomainList(list, domain), isOnDomainList);
}

QTEST_MAIN(tst_CookieJar)
#include "tst_cookiejar.moc"

