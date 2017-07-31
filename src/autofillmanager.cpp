/**
 * Copyright (c) 2009, Benjamin C. Meyer  <ben@meyerhome.net>
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

#include "autofillmanager.h"

#include "autosaver.h"
#include "browserapplication.h"
#include "browsermainwindow.h"
#include "networkaccessmanagerproxy.h"
#include "webpageproxy.h"
#include "webview.h"

#include <qdesktopservices.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qurl.h>
#include <qwebframe.h>
#include <qwebpage.h>
#include <qwebsettings.h>
#include <QUrlQuery>

#include <qdebug.h>

// #define AUTOFILL_DEBUG

AutoFillManager::AutoFillManager(QObject *parent)
    : QObject(parent)
    , m_savePasswordForms(true)
    , m_allowAutoCompleteOff(true)
    , m_saveTimer(new AutoSaver(this))
{
    connect(this, SIGNAL(autoFillChanged()),
            m_saveTimer, SLOT(changeOccurred()));
    loadSettings();
    loadFormData();
}

AutoFillManager::~AutoFillManager()
{
    m_saveTimer->saveIfNeccessary();
}

void AutoFillManager::save() const
{
    saveFormData();
}

void AutoFillManager::setForms(const QList<Form> &forms)
{
    m_forms = forms;
    emit autoFillChanged();
}

QList<AutoFillManager::Form> AutoFillManager::forms() const
{
    return m_forms;
}

void AutoFillManager::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("autofill"));
    m_savePasswordForms = settings.value(QLatin1String("passwordForms"), m_savePasswordForms).toBool();
    m_allowAutoCompleteOff = settings.value(QLatin1String("ignoreAutoCompleteOff"), m_allowAutoCompleteOff).toBool();
}

QString AutoFillManager::autoFillDataFile()
{
    QString fileName = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/data/Arora";
    fileName += QLatin1String("/autofill.dat");
    return fileName;
}

void AutoFillManager::saveFormData() const
{
    QString fileName = autoFillDataFile();
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "Unable to open" << fileName << "to store autofill data";
        return;
    }

    QDataStream stream(&file);
    stream << m_forms;
}

void AutoFillManager::loadFormData()
{
    QString fileName = autoFillDataFile();
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return;

    QDataStream stream(&file);
    stream >> m_forms;
}

void AutoFillManager::post(const QNetworkRequest &request, const QByteArray &outgoingData)
{
#ifdef AUTOFILL_DEBUG
    qDebug() << "AutoFillManager::" << __FUNCTION__ << outgoingData << request.url();
#endif

    // Don't even give the options to save this site user name & password.
    if (QWebSettings::globalSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled)) {
        return;
    }

    // Determine the url
    QByteArray refererHeader = request.rawHeader("Referer");
    if (refererHeader.isEmpty()) {
        // XXX We could store the frame url in the request if this is a common problem
        qWarning() << "AutoFillManager:" << "Unable to determine the request Referer";
        return;
    }
    QUrl url = QUrl::fromEncoded(refererHeader);
    url = stripUrl(url);

    // Check that the url isn't in m_never
    if (m_never.contains(url))
        return;

    // Check the request type
    QVariant typeVariant = request.attribute((QNetworkRequest::Attribute)(WebPageProxy::pageAttributeId() + 1));
    QWebPage::NavigationType type = (QWebPage::NavigationType)typeVariant.toInt();
    if (typeVariant.isValid() && type != QWebPage::NavigationTypeFormSubmitted) {
        // XXX Does this occur normally?
        qWarning() << "AutoFillManager:" << "Type is not FormSubmitted" << type
                   << "expected:" << QWebPage::NavigationTypeFormSubmitted;
        return;
    }

    // Determine the QWebView
    QVariant v = request.attribute((QNetworkRequest::Attribute)(WebPageProxy::pageAttributeId()));
    QWebPage *webPage = (QWebPage*)(v.value<void*>());
    if (!webPage) {
        qWarning() << "AutoFillManager:" << "QWebPage is not set in QNetworkRequest.";
        return;
    }
#if 0
    // TODO CHECK reply ownership
    if (!NetworkAccessManagerProxy::exists(webPage)) {
        qWarning() << "AutoFillManager:" << "QWebPage no longer exists.";
        return;
    }
#endif

    // Find the matching form on the webpage
    Form form = findForm(webPage, outgoingData);
    if (!form.isValid()) {
#if 0
        qWarning() << "AutoFillManager:" << "Unable to find matching form on the webpage.";
#endif
        return;
    }
    form.url = url;

    // Check if we allow storing this form if it has a password
    if (!allowedToAutoFill(form.hasAPassword))
        return;

    // Prompt if we have never seen this password
    int alreadyAccepted = -1;
    for (int i = 0; i < m_forms.count(); ++i) {
        const Form &form = m_forms.at(i);
        if (form.url == url) {
            alreadyAccepted = i;
            break;
        }
    }
    if (form.hasAPassword && alreadyAccepted == -1) {
        QMessageBox messageBox;
        messageBox.setText(tr("<b>Would you like to save this password?</b><br> \
        To review passwords you have saved and remove them, open the AutoFill panel of preferences."));
        messageBox.addButton(tr("Never for this site"), QMessageBox::DestructiveRole);
        messageBox.addButton(tr("Not now"), QMessageBox::RejectRole);
        messageBox.addButton(QMessageBox::Yes);
        messageBox.setDefaultButton(QMessageBox::Yes);
        messageBox.exec();
        switch (messageBox.buttonRole(messageBox.clickedButton())) {
        case QMessageBox::DestructiveRole:
            m_never.append(url);
            return;
        case QMessageBox::RejectRole:
            return;
        default:
            break;
        }
    }

#ifdef AUTOFILL_DEBUG
    qDebug() << "AutoFillManager:" << "Saving" << form.url;
#endif
    // TODO When we can hook into element events we can save multiple passwords for different users
    if (alreadyAccepted != -1)
        m_forms.removeAt(alreadyAccepted);
    m_forms.append(form);
    emit autoFillChanged();
}

AutoFillManager::Form AutoFillManager::findForm(QWebPage *webPage, const QByteArray &outgoingData) const
{
    Form form;
    QUrlQuery argsUrl(outgoingData);
    QList<QPair<QString, QString> > encodedArgs = argsUrl.queryItems();
    QSet<QPair<QString, QString> > args;
    // XXX Is there a Qt function to do this? (unencode '+' to ' ')
    for (int i = 0; i < encodedArgs.count(); ++i) {
        QString key = encodedArgs[i].first;
        QString value = encodedArgs[i].second.replace(QLatin1String("+"), QLatin1String(" "));
        QPair<QString, QString> p(key, value);
        args.insert(p);
    }

    QFile file(QLatin1String(":parseForms.js"));
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "AutoFillManager:" << "Unable to open js form parsing file";
        return form;
    }
    QString script = QLatin1String(file.readAll());

    // XXX Do I need to do this on subframes?
    QVariant r = webPage->mainFrame()->evaluateJavaScript(script);
    QVariantList list = r.toList();
    foreach (const QVariant &formVariant, list) {
        QVariantMap map = formVariant.toMap();
        bool formHasPasswords = false;
        QString formName = map[QLatin1String("name")].toString();
        QVariantList elements = map[QLatin1String("elements")].toList();
        QSet<QPair<QString, QString> > formElements;
        QSet<QPair<QString, QString> > deadElements;
        foreach (const QVariant &element, elements) {
            QVariantMap elementMap = element.toMap();
            QString name = elementMap[QLatin1String("name")].toString();
            QString value = elementMap[QLatin1String("value")].toString();
            QString type = elementMap[QLatin1String("type")].toString();
            if (type == QLatin1String("password"))
                formHasPasswords = true;
            QPair<QString, QString> p(name, value);
            if ((elementMap[QLatin1String("autocomplete")].toString()) == QLatin1String("off"))
                deadElements.insert(p);
            if (!name.isEmpty())
                formElements.insert(p);
        }
#if 0
        QT_VERSION >= 0x040600
        if (formElements.contains(args)) {
#else
        if (formElements.intersect(args) == args) {
#endif
            form.hasAPassword = formHasPasswords;
            form.name = formName;
            if (m_allowAutoCompleteOff)
                args = args.subtract(deadElements);
            form.elements = args.toList();
            break;
        }
#ifdef AUTOFILL_DEBUG
        qDebug() << formName;
        qDebug() << formElements;
        qDebug() << args;
        qDebug() << formElements.count() << args.count() << formElements.subtract(args);
#endif
    }
    return form;
}

QUrl AutoFillManager::stripUrl(const QUrl &url)
{
    QUrl cleanUrl = url;
    cleanUrl.setQuery(QUrlQuery());
    cleanUrl.setFragment(QString());
    cleanUrl.setUserInfo(QString());
    return cleanUrl;
}

bool AutoFillManager::allowedToAutoFill(bool password) const
{
    if (password && m_savePasswordForms)
        return true;
    return false;
}

QList<AutoFillManager::Form> AutoFillManager::fetchForms(const QUrl &url) const
{
    QList<Form> forms;
    foreach (const Form &form, m_forms)
        if (form.url == url)
            forms.append(form);
#ifdef AUTOFILL_DEBUG
    qDebug() << "AutoFillManager::" << __FUNCTION__ << url << m_forms.count() << "found:" << forms.count();
#endif
    return forms;
}

void AutoFillManager::fill(QWebPage *page) const
{
#ifdef AUTOFILL_DEBUG
    qDebug() << "AutoFillManager::" << __FUNCTION__ << page;
#endif
    if (!page || !page->mainFrame())
        return;

    QUrl url = page->mainFrame()->url();
    url = stripUrl(url);

    const QList<Form> forms = fetchForms(url);
    if (forms.isEmpty())
        return;

    foreach (const Form &form, forms) {
        QString formName = QString(QLatin1String("\"%1\"")).arg(form.name);
        if (form.name.isEmpty())
            formName = QLatin1String("0");
        foreach (const AutoFillManager::Element &element, form.elements) {
            const QString key = element.first;
            const QString value = element.second;

            // When we drop 4.5 migrate this to the 4.6 dom API
            bool disabled = page->mainFrame()->evaluateJavaScript(QString(QLatin1String("document.forms[%1].elements[\"%2\"].disabled")).arg(formName).arg(key)).toBool();
            if (disabled) {
#ifdef AUTOFILL_DEBUG
                qDebug() << formName << "is disabled";
#endif
                continue;
            }
            bool readOnly = page->mainFrame()->evaluateJavaScript(QString(QLatin1String("document.forms[%1].elements[\"%2\"].readonly")).arg(formName).arg(key)).toBool();
            if (readOnly) {
#ifdef AUTOFILL_DEBUG
                qDebug() << formName << "is readOnly";
#endif
                continue;
            }

            QString type = page->mainFrame()->evaluateJavaScript(QString(QLatin1String("document.forms[%1].elements[\"%2\"].type")).arg(formName).arg(key)).toString();
            if (type.isEmpty()
                || type == QLatin1String("hidden")
                || type == QLatin1String("reset")
                || type == QLatin1String("submit")) {
#ifdef AUTOFILL_DEBUG
                qDebug() << formName << key << "is hidden, reset or submit";
#endif
                continue;
            }
#ifdef AUTOFILL_DEBUG
            qDebug() << "type:" << type << "readonly" << readOnly << "disabled" << disabled << key << value;
#endif
            QString setType = (type == QLatin1String("checkbox"))
                                ? QLatin1String("checked")
                                : QLatin1String("value");

            // XXX is there a cleaner way to do this?
            QString jsValue = value;
            jsValue.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
            jsValue.replace(QLatin1Char('\"'), QLatin1String("\\\""));
            QString javascript = QString(QLatin1String("document.forms[%1].elements[\"%2\"].%3=\"%4\";"))
                    .arg(formName)
                    .arg(key)
                    .arg(setType)
                    .arg(jsValue);
            page->mainFrame()->evaluateJavaScript(javascript);
        }
    }
}

QDataStream &operator>>(QDataStream &in, AutoFillManager::Form &form)
{
    AutoFillManager::Form::load(in, form);
    return in;
}

QDataStream &operator<<(QDataStream &out, const AutoFillManager::Form &form)
{
    AutoFillManager::Form::save(out, form);
    return out;
}

void AutoFillManager::Form::load(QDataStream &in, AutoFillManager::Form &form)
{
    in >> form.elements;
    in >> form.url;
    in >> form.name;
    in >> form.hasAPassword;
}

void AutoFillManager::Form::save(QDataStream &out, const AutoFillManager::Form &form)
{
    out << form.elements;
    out << form.url;
    out << form.name;
    out << form.hasAPassword;
}

