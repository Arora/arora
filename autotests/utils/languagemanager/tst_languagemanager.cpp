/**
 * Copyright (c) 2008, Benjamin C. Meyer
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

#include <qtest.h>
#include <qsignalspy.h>

#include <languagemanager.h>
#include <qlocale.h>

class tst_LanguageManager : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void languagemanager_data();
    void languagemanager();

    void isLanguageAvailable_data();
    void isLanguageAvailable();
    void chooseNewLanguage_data();
    void chooseNewLanguage();
    void setCurrentLanguage_data();
    void setCurrentLanguage();
};

// Subclass that exposes the protected functions.
class SubLanguageManager : public LanguageManager
{
public:
    SubLanguageManager() : LanguageManager() {
        addLocaleDirectory(qApp->applicationDirPath() + QLatin1String("/.qm/locale"));
    }

};

class TestWidget : public QWidget
{
public:
    void changeEvent(QEvent *event) {
        if (event->type() == QEvent::LanguageChange)
            retranslate = true;
        QWidget::changeEvent(event);
    }
    bool retranslate;
};


// This will be called before the first test function is executed.
// It is only called once.
void tst_LanguageManager::initTestCase()
{
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_LanguageManager::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_LanguageManager::init()
{
    SubLanguageManager manager;
    manager.setCurrentLanguage(QString());
}

// This will be called after every test function.
void tst_LanguageManager::cleanup()
{
}

void tst_LanguageManager::languagemanager_data()
{
}

void tst_LanguageManager::languagemanager()
{
    SubLanguageManager manager;
    manager.languages();
    // spawns an event loop...
    // manager.chooseNewLanguage();

    QString fallbackLanguage;
    if (manager.isLanguageAvailable(QLocale::system().name()))
        fallbackLanguage = QLocale::system().name();

    QCOMPARE(manager.currentLanguage(), fallbackLanguage);
    manager.setCurrentLanguage(QString());
}

void tst_LanguageManager::isLanguageAvailable_data()
{
    QTest::addColumn<QString>("language");
    QTest::addColumn<bool>("available");
    QTest::newRow("null") << QString() << true;
    QTest::newRow("fallback0") << "ca_ES" << true;
    QTest::newRow("fallback1") << "ca_ES.UTF-8" << true;
    QTest::newRow("fallback2") << "de_AT" << true;
}

void tst_LanguageManager::isLanguageAvailable()
{
    QFETCH(QString, language);
    QFETCH(bool, available);
    SubLanguageManager manager;
    QCOMPARE(manager.isLanguageAvailable(language), available);
}

void tst_LanguageManager::chooseNewLanguage_data()
{
    QTest::addColumn<int>("foo");
    QTest::newRow("null") << 0;
}

// public void chooseNewLanguage()
void tst_LanguageManager::chooseNewLanguage()
{
    // how do you test this?
#if 0
    QFETCH(int, foo);

    SubLanguageManager manager;

    manager.chooseNewLanguage();
#endif
    QSKIP("Test is not implemented.");
}

void tst_LanguageManager::setCurrentLanguage_data()
{
    SubLanguageManager manager;

    QString fallbackLanguage;
    if (manager.isLanguageAvailable(QLocale::system().name()))
        fallbackLanguage = QLocale::system().name();

    QTest::addColumn<QString>("language");
    QTest::addColumn<bool>("success");
    QTest::addColumn<QString>("result");
    QTest::newRow("null-foo") << QString("foo") << false << fallbackLanguage;
    QTest::newRow("null-null") << QString() << false << fallbackLanguage;

    QString validLanguage = manager.languages().value(0);
    if (validLanguage.isEmpty())
        QSKIP("no languages to test with");
    QTest::newRow(validLanguage.toLatin1()) << validLanguage << true << validLanguage;
    QTest::newRow("fallback") << "ca_ES" << true << "ca_ES";
}

// public void setCurrentLanguage(QString const &language)
void tst_LanguageManager::setCurrentLanguage()
{
    QFETCH(QString, language);
    QFETCH(bool, success);
    QFETCH(QString, result);

    SubLanguageManager manager;
    QString initialLanguage = manager.currentLanguage();
    QSignalSpy spy(&manager, SIGNAL(languageChanged(const QString &)));

    TestWidget widget;
    widget.retranslate = false;
    QCOMPARE(manager.setCurrentLanguage(language), success);
    QCOMPARE(manager.currentLanguage(), result);

    // test if we set the default locale properly
    // this is essential for opensearch localization
    QLocale currentLocale(manager.currentLanguage());
    if (manager.currentLanguage().isEmpty())
        currentLocale = QLocale();
    QCOMPARE(currentLocale, QLocale());

    qApp->processEvents();
    QCOMPARE(widget.retranslate, success);
    QCOMPARE(spy.count(), success ? 1 : 0);
    if (success) {
        QVERIFY(manager.setCurrentLanguage(QString()));
        QCOMPARE(spy.count(), 2);
    }
}

QTEST_MAIN(tst_LanguageManager)
#include "tst_languagemanager.moc"

