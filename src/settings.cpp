/*
 * Copyright 2008-2009 Benjamin C. Meyer <ben@meyerhome.net>
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

/****************************************************************************
**
** Copyright (C) 2007-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.2, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "settings.h"

#include "acceptlanguagedialog.h"
#include "autofilldialog.h"
#include "autofillmanager.h"
#include "browserapplication.h"
#include "browsermainwindow.h"
#include "cookiedialog.h"
#include "cookieexceptionsdialog.h"
#include "cookiejar.h"
#include "historymanager.h"
#include "networkaccessmanager.h"
#include "tabwidget.h"
#include "webpluginfactory.h"
#include "webpage.h"
#include "webview.h"

#include <qdesktopservices.h>
#include <qfile.h>
#include <qfontdialog.h>
#include <qmetaobject.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qfiledialog.h>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_cacheEnabled(false)
{
    setupUi(this);
    connect(exceptionsButton, SIGNAL(clicked()), this, SLOT(showExceptions()));
    connect(setHomeToCurrentPageButton, SIGNAL(clicked()), this, SLOT(setHomeToCurrentPage()));
    connect(cookiesButton, SIGNAL(clicked()), this, SLOT(showCookies()));
    connect(standardFontButton, SIGNAL(clicked()), this, SLOT(chooseFont()));
    connect(fixedFontButton, SIGNAL(clicked()), this, SLOT(chooseFixedFont()));
    connect(languageButton, SIGNAL(clicked()), this, SLOT(chooseAcceptLanguage()));
    connect(downloadDirectoryButton, SIGNAL(clicked()), this, SLOT(chooseDownloadDirectory()));
    connect(externalDownloadBrowse, SIGNAL(clicked()), this, SLOT(chooseDownloadProgram()));
    connect(styleSheetBrowseButton, SIGNAL(clicked()), this, SLOT(chooseStyleSheet()));

    connect(editAutoFillUserButton, SIGNAL(clicked()), this, SLOT(editAutoFillUser()));

    // As network cache has too many bugs in 4.5.1, do not allow to enable it.
    if (QLatin1String(qVersion()) == QLatin1String("4.5.1"))
        networkCache->setVisible(false);

    loadDefaults();
    loadFromSettings();
}

void SettingsDialog::loadDefaults()
{
    QWebSettings *defaultSettings = QWebSettings::globalSettings();
    QString standardFontFamily = defaultSettings->fontFamily(QWebSettings::StandardFont);
    int standardFontSize = defaultSettings->fontSize(QWebSettings::DefaultFontSize);
    m_standardFont = QFont(standardFontFamily, standardFontSize);
    standardLabel->setText(QString(QLatin1String("%1 %2")).arg(m_standardFont.family()).arg(m_standardFont.pointSize()));

    QString fixedFontFamily = defaultSettings->fontFamily(QWebSettings::FixedFont);
    int fixedFontSize = defaultSettings->fontSize(QWebSettings::DefaultFixedFontSize);
    m_fixedFont = QFont(fixedFontFamily, fixedFontSize);
    fixedLabel->setText(QString(QLatin1String("%1 %2")).arg(m_fixedFont.family()).arg(m_fixedFont.pointSize()));

    downloadsLocation->setText(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));

    blockPopupWindows->setChecked(!defaultSettings->testAttribute(QWebSettings::JavascriptCanOpenWindows));
    enableJavascript->setChecked(defaultSettings->testAttribute(QWebSettings::JavascriptEnabled));
    enablePlugins->setChecked(defaultSettings->testAttribute(QWebSettings::PluginsEnabled));
    enableImages->setChecked(defaultSettings->testAttribute(QWebSettings::AutoLoadImages));
    enableLocalStorage->setChecked(defaultSettings->testAttribute(QWebSettings::LocalStorageEnabled));
    clickToFlash->setChecked(false);
    cookieSessionCombo->setCurrentIndex(0);
    filterTrackingCookiesCheckbox->setChecked(false);

    autoFillPasswordFormsCheckBox->setChecked(false);
    minimFontSizeCheckBox->setChecked(false);
    minimumFontSizeSpinBox->setValue(9);
}

void SettingsDialog::loadFromSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("Settings"));
    tabWidget->setCurrentIndex(settings.value(QLatin1String("currentTab"), 0).toInt());
    settings.endGroup();

    settings.beginGroup(QLatin1String("MainWindow"));
    QString defaultHome = QLatin1String("about:home");
    homeLineEdit->setText(settings.value(QLatin1String("home"), defaultHome).toString());
    startupBehavior->setCurrentIndex(settings.value(QLatin1String("startupBehavior"), 0).toInt());
    settings.endGroup();

    settings.beginGroup(QLatin1String("history"));
    int historyExpire = settings.value(QLatin1String("historyLimit")).toInt();
    int idx = 0;
    switch (historyExpire) {
    case 1: idx = 0; break;
    case 7: idx = 1; break;
    case 14: idx = 2; break;
    case 30: idx = 3; break;
    case 365: idx = 4; break;
    case -1: idx = 5; break;
    case -2: idx = 6; break;
    default:
        idx = 5;
    }
    expireHistory->setCurrentIndex(idx);
    settings.endGroup();

    settings.beginGroup(QLatin1String("urlloading"));
    bool search = settings.value(QLatin1String("searchEngineFallback"), false).toBool();
    searchEngineFallback->setChecked(search);
    settings.endGroup();

    settings.beginGroup(QLatin1String("downloadmanager"));
    bool alwaysPromptForFileName = settings.value(QLatin1String("alwaysPromptForFileName"), false).toBool();
    downloadAsk->setChecked(alwaysPromptForFileName);
    QString downloadDirectory = settings.value(QLatin1String("downloadDirectory"), downloadsLocation->text()).toString();
    downloadsLocation->setText(downloadDirectory);
    externalDownloadButton->setChecked(settings.value(QLatin1String("external"), false).toBool());
    externalDownloadPath->setText(settings.value(QLatin1String("externalPath")).toString());
    settings.endGroup();

    // Appearance
    settings.beginGroup(QLatin1String("websettings"));
    m_fixedFont = qVariantValue<QFont>(settings.value(QLatin1String("fixedFont"), m_fixedFont));
    m_standardFont = qVariantValue<QFont>(settings.value(QLatin1String("standardFont"), m_standardFont));

    standardLabel->setText(QString(QLatin1String("%1 %2")).arg(m_standardFont.family()).arg(m_standardFont.pointSize()));
    fixedLabel->setText(QString(QLatin1String("%1 %2")).arg(m_fixedFont.family()).arg(m_fixedFont.pointSize()));

    blockPopupWindows->setChecked(settings.value(QLatin1String("blockPopupWindows"), blockPopupWindows->isChecked()).toBool());
    enableJavascript->setChecked(settings.value(QLatin1String("enableJavascript"), enableJavascript->isChecked()).toBool());
    enablePlugins->setChecked(settings.value(QLatin1String("enablePlugins"), enablePlugins->isChecked()).toBool());
    enableImages->setChecked(settings.value(QLatin1String("enableImages"), enableImages->isChecked()).toBool());
    enableLocalStorage->setChecked(settings.value(QLatin1String("enableLocalStorage"), enableLocalStorage->isChecked()).toBool());
    userStyleSheet->setText(QString::fromUtf8(settings.value(QLatin1String("userStyleSheet")).toUrl().toEncoded()));
    clickToFlash->setChecked(settings.value(QLatin1String("enableClickToFlash"), clickToFlash->isChecked()).toBool());
    int minimumFontSize = settings.value(QLatin1String("minimumFontSize"), 0).toInt();
    minimFontSizeCheckBox->setChecked(minimumFontSize != 0);
    if (minimumFontSize != 0)
        minimumFontSizeSpinBox->setValue(minimumFontSize);
    settings.endGroup();

    // Privacy
    settings.beginGroup(QLatin1String("cookies"));

    QByteArray value = settings.value(QLatin1String("acceptCookies"), QLatin1String("AcceptOnlyFromSitesNavigatedTo")).toByteArray();
    QMetaEnum acceptPolicyEnum = CookieJar::staticMetaObject.enumerator(CookieJar::staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    CookieJar::AcceptPolicy acceptCookies = acceptPolicyEnum.keyToValue(value) == -1 ?
                        CookieJar::AcceptOnlyFromSitesNavigatedTo :
                        static_cast<CookieJar::AcceptPolicy>(acceptPolicyEnum.keyToValue(value));
    switch (acceptCookies) {
    case CookieJar::AcceptAlways:
        acceptCombo->setCurrentIndex(0);
        break;
    case CookieJar::AcceptNever:
        acceptCombo->setCurrentIndex(1);
        break;
    case CookieJar::AcceptOnlyFromSitesNavigatedTo:
        acceptCombo->setCurrentIndex(2);
        break;
    }

    value = settings.value(QLatin1String("keepCookiesUntil"), QLatin1String("Expire")).toByteArray();
    QMetaEnum keepPolicyEnum = CookieJar::staticMetaObject.enumerator(CookieJar::staticMetaObject.indexOfEnumerator("KeepPolicy"));
    CookieJar::KeepPolicy keepCookies = keepPolicyEnum.keyToValue(value) == -1 ?
                        CookieJar::KeepUntilExpire :
                        static_cast<CookieJar::KeepPolicy>(keepPolicyEnum.keyToValue(value));
    switch (keepCookies) {
    case CookieJar::KeepUntilExpire:
        keepUntilCombo->setCurrentIndex(0);
        break;
    case CookieJar::KeepUntilExit:
        keepUntilCombo->setCurrentIndex(1);
        break;
    case CookieJar::KeepUntilTimeLimit:
        keepUntilCombo->setCurrentIndex(2);
        break;
    }
    int sessionLength = settings.value(QLatin1String("sessionLength"), -1).toInt();
    switch (sessionLength) {
    case 1: cookieSessionCombo->setCurrentIndex(1); break;
    case 2: cookieSessionCombo->setCurrentIndex(2); break;
    case 3: cookieSessionCombo->setCurrentIndex(3); break;
    case 7: cookieSessionCombo->setCurrentIndex(4); break;
    case 30: cookieSessionCombo->setCurrentIndex(5); break;
    default:
    case 0: cookieSessionCombo->setCurrentIndex(0); break;
    }
    filterTrackingCookiesCheckbox->setChecked(settings.value(QLatin1String("filterTrackingCookies"), false).toBool());
    settings.endGroup();

    // Network
    settings.beginGroup(QLatin1String("network"));
    m_cacheEnabled = settings.value(QLatin1String("cacheEnabled"), true).toBool();
    networkCache->setChecked(m_cacheEnabled);
    networkCacheMaximumSizeSpinBox->setValue(settings.value(QLatin1String("maximumCacheSize"), 50).toInt());
    settings.endGroup();

    // Proxy
    settings.beginGroup(QLatin1String("proxy"));
    proxySupport->setChecked(settings.value(QLatin1String("enabled"), false).toBool());
    proxyType->setCurrentIndex(settings.value(QLatin1String("type"), 0).toInt());
    proxyHostName->setText(settings.value(QLatin1String("hostName")).toString());
    proxyPort->setValue(settings.value(QLatin1String("port"), 1080).toInt());
    proxyUserName->setText(settings.value(QLatin1String("userName")).toString());
    proxyPassword->setText(settings.value(QLatin1String("password")).toString());
    settings.endGroup();

    // Tabs
    settings.beginGroup(QLatin1String("tabs"));
    selectTabsWhenCreated->setChecked(settings.value(QLatin1String("selectNewTabs"), false).toBool());
    confirmClosingMultipleTabs->setChecked(settings.value(QLatin1String("confirmClosingMultipleTabs"), true).toBool());
    oneCloseButton->setChecked(settings.value(QLatin1String("oneCloseButton"),false).toBool());
    quitAsLastTabClosed->setChecked(settings.value(QLatin1String("quitAsLastTabClosed"), true).toBool());
    openTargetBlankLinksIn->setCurrentIndex(settings.value(QLatin1String("openTargetBlankLinksIn"), TabWidget::NewSelectedTab).toInt());
    openLinksFromAppsIn->setCurrentIndex(settings.value(QLatin1String("openLinksFromAppsIn"), TabWidget::NewSelectedTab).toInt());
    settings.endGroup();

    // Accessibility
#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    settings.beginGroup(QLatin1String("WebView"));
    enableAccessKeys->setChecked(settings.value(QLatin1String("enableAccessKeys"), true).toBool());
    settings.endGroup();
#else
    enableAccessKeys->setEnabled(false);
#endif

    settings.beginGroup(QLatin1String("autofill"));
    autoFillPasswordFormsCheckBox->setChecked(settings.value(QLatin1String("passwordForms"), true).toBool());
    settings.endGroup();
}

void SettingsDialog::saveToSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("Settings"));
    settings.setValue(QLatin1String("currentTab"), tabWidget->currentIndex());
    settings.endGroup();

    settings.beginGroup(QLatin1String("MainWindow"));
    settings.setValue(QLatin1String("home"), homeLineEdit->text());
    settings.setValue(QLatin1String("startupBehavior"), startupBehavior->currentIndex());
    settings.endGroup();

    settings.beginGroup(QLatin1String("downloadmanager"));
    settings.setValue(QLatin1String("alwaysPromptForFileName"), downloadAsk->isChecked());
    settings.setValue(QLatin1String("downloadDirectory"), downloadsLocation->text());
    settings.setValue(QLatin1String("external"), externalDownloadButton->isChecked());
    settings.setValue(QLatin1String("externalPath"), externalDownloadPath->text());
    settings.endGroup();

    settings.beginGroup(QLatin1String("history"));
    int historyExpire = expireHistory->currentIndex();
    int idx = -1;
    switch (historyExpire) {
    case 0: idx = 1; break;
    case 1: idx = 7; break;
    case 2: idx = 14; break;
    case 3: idx = 30; break;
    case 4: idx = 365; break;
    case 5: idx = -1; break;
    case 6: idx = -2; break;
    }
    settings.setValue(QLatin1String("historyLimit"), idx);
    settings.endGroup();

    settings.beginGroup(QLatin1String("urlloading"));
    settings.setValue(QLatin1String("searchEngineFallback"), searchEngineFallback->isChecked());
    settings.endGroup();

    // Appearance
    settings.beginGroup(QLatin1String("websettings"));
    settings.setValue(QLatin1String("fixedFont"), m_fixedFont);
    settings.setValue(QLatin1String("standardFont"), m_standardFont);

    settings.setValue(QLatin1String("blockPopupWindows"), blockPopupWindows->isChecked());
    settings.setValue(QLatin1String("enableJavascript"), enableJavascript->isChecked());
    settings.setValue(QLatin1String("enablePlugins"), enablePlugins->isChecked());
    settings.setValue(QLatin1String("enableImages"), enableImages->isChecked());
    settings.setValue(QLatin1String("enableLocalStorage"), enableLocalStorage->isChecked());
    QString userStyleSheetString = userStyleSheet->text();
    if (QFile::exists(userStyleSheetString))
        settings.setValue(QLatin1String("userStyleSheet"), QUrl::fromLocalFile(userStyleSheetString));
    else
        settings.setValue(QLatin1String("userStyleSheet"), QUrl::fromEncoded(userStyleSheetString.toUtf8()));
    settings.setValue(QLatin1String("enableClickToFlash"), clickToFlash->isChecked());

    if (minimFontSizeCheckBox->isChecked())
        settings.setValue(QLatin1String("minimumFontSize"), minimumFontSizeSpinBox->value());
    else
        settings.setValue(QLatin1String("minimumFontSize"), 0);
    settings.endGroup();

    // Privacy
    settings.beginGroup(QLatin1String("cookies"));
    CookieJar::AcceptPolicy acceptCookies;
    switch (acceptCombo->currentIndex()) {
    default:
    case 0:
        acceptCookies = CookieJar::AcceptAlways;
        break;
    case 1:
        acceptCookies = CookieJar::AcceptNever;
        break;
    case 2:
        acceptCookies = CookieJar::AcceptOnlyFromSitesNavigatedTo;
        break;
    }

    QMetaEnum acceptPolicyEnum = CookieJar::staticMetaObject.enumerator(CookieJar::staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    settings.setValue(QLatin1String("acceptCookies"), QLatin1String(acceptPolicyEnum.valueToKey(acceptCookies)));

    CookieJar::KeepPolicy keepPolicy;
    switch (keepUntilCombo->currentIndex()) {
    default:
    case 0:
        keepPolicy = CookieJar::KeepUntilExpire;
        break;
    case 1:
        keepPolicy = CookieJar::KeepUntilExit;
        break;
    case 2:
        keepPolicy = CookieJar::KeepUntilTimeLimit;
        break;
    }

    QMetaEnum keepPolicyEnum = CookieJar::staticMetaObject.enumerator(CookieJar::staticMetaObject.indexOfEnumerator("KeepPolicy"));
    settings.setValue(QLatin1String("keepCookiesUntil"), QLatin1String(keepPolicyEnum.valueToKey(keepPolicy)));
    int sessionLength = cookieSessionCombo->currentIndex();
    switch (sessionLength) {
    case 1: sessionLength = 1; break;
    case 2: sessionLength = 2; break;
    case 3: sessionLength = 3; break;
    case 4: sessionLength = 7; break;
    case 5: sessionLength = 30; break;
    default:
    case 0: sessionLength = -1; break;
    }
    settings.setValue(QLatin1String("sessionLength"), sessionLength);
    settings.setValue(QLatin1String("filterTrackingCookies"), filterTrackingCookiesCheckbox->isChecked());
    settings.endGroup();

    // Network
    settings.beginGroup(QLatin1String("network"));
    settings.setValue(QLatin1String("cacheEnabled"), networkCache->isChecked());
    settings.setValue(QLatin1String("maximumCacheSize"), networkCacheMaximumSizeSpinBox->value());
    settings.endGroup();

    // proxy
    settings.beginGroup(QLatin1String("proxy"));
    settings.setValue(QLatin1String("enabled"), proxySupport->isChecked());
    settings.setValue(QLatin1String("type"), proxyType->currentIndex());
    settings.setValue(QLatin1String("hostName"), proxyHostName->text());
    settings.setValue(QLatin1String("port"), proxyPort->text());
    settings.setValue(QLatin1String("userName"), proxyUserName->text());
    settings.setValue(QLatin1String("password"), proxyPassword->text());
    settings.endGroup();

    // Tabs
    settings.beginGroup(QLatin1String("tabs"));
    settings.setValue(QLatin1String("selectNewTabs"), selectTabsWhenCreated->isChecked());
    settings.setValue(QLatin1String("confirmClosingMultipleTabs"), confirmClosingMultipleTabs->isChecked());
    settings.setValue(QLatin1String("oneCloseButton"), oneCloseButton->isChecked());
    settings.setValue(QLatin1String("quitAsLastTabClosed"), quitAsLastTabClosed->isChecked());
    settings.setValue(QLatin1String("openTargetBlankLinksIn"), openTargetBlankLinksIn->currentIndex());
    settings.setValue(QLatin1String("openLinksFromAppsIn"), openLinksFromAppsIn->currentIndex());
    settings.endGroup();

    settings.beginGroup(QLatin1String("autofill"));
    settings.setValue(QLatin1String("passwordForms"), autoFillPasswordFormsCheckBox->isChecked());
    settings.endGroup();

    // Accessibility
#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    settings.beginGroup(QLatin1String("WebView"));
    settings.setValue(QLatin1String("enableAccessKeys"), enableAccessKeys->isChecked());
    settings.endGroup();
#endif

    BrowserApplication::instance()->loadSettings();
    BrowserApplication::networkAccessManager()->loadSettings();
    BrowserApplication::cookieJar()->loadSettings();
    BrowserApplication::historyManager()->loadSettings();
    BrowserApplication::autoFillManager()->loadSettings();

    WebPage::webPluginFactory()->refreshPlugins();

    QList<BrowserMainWindow*> list = BrowserApplication::instance()->mainWindows();
    foreach (BrowserMainWindow *mainWindow, list) {
        mainWindow->tabWidget()->loadSettings();
    }
}

void SettingsDialog::accept()
{
    saveToSettings();
    // Due to a bug in Qt <= 4.5.1, enabling/disabling cache requires the browser to be restarted.
    if (QLatin1String(qVersion()) <= QLatin1String("4.5.1") && networkCache->isChecked() != m_cacheEnabled) {
        QMessageBox::information(this, tr("Restart required"),
                                 tr("The network cache configuration has changed. "
                                    "So that it can be taken into account, the browser has to be restarted."));
    }
    QDialog::accept();
}

void SettingsDialog::showCookies()
{
    CookieDialog dialog(BrowserApplication::cookieJar(), this);
    dialog.exec();
}

void SettingsDialog::showExceptions()
{
    CookieExceptionsDialog dialog(BrowserApplication::cookieJar(), this);
    dialog.exec();
}

void SettingsDialog::chooseDownloadDirectory()
{
    QString fileName = QFileDialog::getExistingDirectory(this, tr("Choose Directory"), downloadsLocation->text());
    downloadsLocation->setText(fileName);
}

void SettingsDialog::chooseDownloadProgram()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose Program"), externalDownloadPath->text());
    if (fileName.contains(QLatin1Char(' ')))
        fileName = QString(QLatin1String("\"%1\"")).arg(fileName);
    externalDownloadPath->setText(fileName);
}

void SettingsDialog::chooseFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, m_standardFont, this);
    if (ok) {
        m_standardFont = font;
        standardLabel->setText(QString(QLatin1String("%1 %2")).arg(font.family()).arg(font.pointSize()));
    }
}

void SettingsDialog::chooseFixedFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, m_fixedFont, this);
    if (ok) {
        m_fixedFont = font;
        fixedLabel->setText(QString(QLatin1String("%1 %2")).arg(font.family()).arg(font.pointSize()));
    }
}

void SettingsDialog::setHomeToCurrentPage()
{
    BrowserMainWindow *mw = static_cast<BrowserMainWindow*>(parent());
    WebView *webView = mw->currentTab();
    if (webView)
        homeLineEdit->setText(QString::fromUtf8(webView->url().toEncoded()));
}

void SettingsDialog::chooseAcceptLanguage()
{
    AcceptLanguageDialog dialog;
    dialog.exec();
}

void SettingsDialog::chooseStyleSheet()
{
    QUrl url = QUrl::fromEncoded(userStyleSheet->text().toUtf8());
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose CSS File"), url.toLocalFile());
    userStyleSheet->setText(QString::fromUtf8(QUrl::fromLocalFile(fileName).toEncoded()));
}

void SettingsDialog::editAutoFillUser()
{
    AutoFillDialog dialog;
    dialog.exec();
}

