/*
 * Copyright 2008-2009 Diego Iastrubni, elcuco, at, kde.org
 * Copyright 2008-2009 Benjamin C. Meyer <ben@meyerhome.net>
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

#include "languagemanager.h"

#include <qapplication.h>
#include <qdir.h>
#include <qdiriterator.h>
#include <qfileinfo.h>
#include <qinputdialog.h>
#include <qlibraryinfo.h>
#include <qmessagebox.h>
#include <qlocale.h>
#include <qsettings.h>
#include <qtranslator.h>

#include <qdebug.h>

// #define LANGUAGEMANAGER_DEBUG

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
    , m_sysTranslator(0)
    , m_appTranslator(0)
    , m_loaded(false)
{
#ifdef LANGUAGEMANAGER_DEBUG
    qDebug() << "LanguageManager::" << __FUNCTION__;
#endif
}

void LanguageManager::addLocaleDirectory(const QString &directory)
{
    m_localeDirectories.append(directory);
}

QStringList LanguageManager::localeDirectories() const
{
    return m_localeDirectories;
}

QString LanguageManager::currentLanguage() const
{
#ifdef LANGUAGEMANAGER_DEBUG
    qDebug() << "LanguageManager::" << __FUNCTION__;
#endif
    if (!m_currentLanguage.isEmpty())
        return m_currentLanguage;

    const QString sysLanguage = QLocale::system().name();
    if (isLanguageAvailable(sysLanguage))
        return sysLanguage;
    return QString();
}

bool LanguageManager::isLanguageAvailable(const QString &language) const
{
#ifdef LANGUAGEMANAGER_DEBUG
    qDebug() << "LanguageManager::" << __FUNCTION__ << language;
#endif
    if (language.isEmpty())
        return true;

    // optimization so we don't have to load all the languages
    if (!m_loaded) {
        foreach (const QString &dir, m_localeDirectories) {
            QString file = dir + QLatin1Char('/') + language + QLatin1String(".qm");
            if (QFile::exists(file))
                return true;
        }
    }

    return !(convertStringToLanguageFile(language).isEmpty());
}

// Return an empty string if we do not have the language file for string
// If we don't have an exact ma
// - Fall back to country
// - Fall back to the first country_ match if there is one
QString LanguageManager::convertStringToLanguageFile(const QString &string) const
{
#ifdef LANGUAGEMANAGER_DEBUG
    qDebug() << "LanguageManager::" << __FUNCTION__ << string;
#endif
    loadAvailableLanguages();
    if (m_languages.contains(string))
       return string;
    QLocale locale(string);
    QString fallback = locale.name().split(QLatin1Char('_')).value(0);
    if (!string.contains(fallback))
        return QString();
#ifdef LANGUAGEMANAGER_DEBUG
    qDebug() << "LanguageManager::" << __FUNCTION__ << "fallback" << fallback;
#endif
    if (m_languages.contains(fallback)) // fallback to the country
        return fallback;

    // See if any language file matches the country
    foreach (const QString &language, m_languages) {
        QString country = QLocale(language).name().split(QLatin1Char('_')).value(0);
        if (country == fallback)
            return language;
    }

    return QString();
}

bool LanguageManager::setCurrentLanguage(const QString &language)
{
#ifdef LANGUAGEMANAGER_DEBUG
    qDebug() << "LanguageManager::" << __FUNCTION__ << language;
#endif
    if (m_currentLanguage == language
        || !isLanguageAvailable(language))
        return false;

    m_currentLanguage = language;

    QSettings settings;
    settings.beginGroup(QLatin1String("LanguageManager"));
    settings.setValue(QLatin1String("language"), m_currentLanguage);

    if (m_currentLanguage.isEmpty()) {
        delete m_appTranslator;
        delete m_sysTranslator;
        m_appTranslator = 0;
        m_sysTranslator = 0;
        emit languageChanged(currentLanguage());
        return true;
    }

    QTranslator *newAppTranslator = new QTranslator(this);
    QString resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    QString languageFile = convertStringToLanguageFile(m_currentLanguage);
    bool loaded = false;
    foreach (const QString &dir, m_localeDirectories) {
        loaded = newAppTranslator->load(languageFile, dir);
        if (loaded)
            break;
    }

    QTranslator *newSysTranslator = new QTranslator(this);
    QString translatorFileName = QLatin1String("qt_") + languageFile;
    if (!newSysTranslator->load(translatorFileName, resourceDir)) {
        delete newSysTranslator;
        newSysTranslator = 0;
    }

    if (!loaded) {
        qWarning() << "Failed to load translation:" << currentLanguage();
        delete newAppTranslator;
        delete newSysTranslator;
        return false;
    }

    QLocale::setDefault(QLocale(m_currentLanguage));

    // A new language event is sent to all widgets in the application
    // They need to catch it and re-translate
    delete m_appTranslator;
    delete m_sysTranslator;
    qApp->installTranslator(newAppTranslator);
    qApp->installTranslator(newSysTranslator);
    m_appTranslator = newAppTranslator;
    m_sysTranslator = newSysTranslator;
    emit languageChanged(currentLanguage());
    return true;
}

QStringList LanguageManager::languages() const
{
#ifdef LANGUAGEMANAGER_DEBUG
    qDebug() << "LanguageManager::" << __FUNCTION__;
#endif
    loadAvailableLanguages();
    return m_languages;
}

void LanguageManager::loadLanguageFromSettings()
{
#ifdef LANGUAGEMANAGER_DEBUG
    qDebug() << "LanguageManager::" << __FUNCTION__;
#endif

    QSettings settings;
    settings.beginGroup(QLatin1String("LanguageManager"));
    if (settings.contains(QLatin1String("language"))) {
        QString selectedLanguage = settings.value(QLatin1String("language")).toString();
#ifdef LANGUAGEMANAGER_DEBUG
        qDebug() << "LanguageManager::" << __FUNCTION__ << "Loading language from settings" << selectedLanguage;
#endif
        // When a translation fails to load remove it from the settings
        // to prevent it from being loaded every time.
        if (!setCurrentLanguage(selectedLanguage)) {
#ifdef LANGUAGEMANAGER_DEBUG
            qDebug() << "LanguageManager::" << __FUNCTION__ << "Failed to load language";
#endif
            settings.remove(QLatin1String("language"));
        }
    } else if (!currentLanguage().isEmpty()) {
        setCurrentLanguage(currentLanguage());
    }
}

void LanguageManager::chooseNewLanguage()
{
#ifdef LANGUAGEMANAGER_DEBUG
    qDebug() << "LanguageManager::" << __FUNCTION__;
#endif
    loadAvailableLanguages();
    if (m_languages.isEmpty()) {
        QMessageBox messageBox;
        QLatin1String separator = QLatin1String(", ");
        messageBox.setText(tr("No translation files are installed at %1.")
            .arg(m_localeDirectories.join(separator)));
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();
        return;
    }

    QStringList items;
    int defaultItem = -1;
    QString current = currentLanguage();
    foreach (const QString &name, m_languages) {
        QLocale locale(name);
        QString string = QString(QLatin1String("%1, %2 (%3) %4"))
            .arg(QLocale::languageToString(locale.language()))
            .arg(QLocale::countryToString(locale.country()))
            .arg(name)
            // this is for pretty RTL support
            .arg(QChar(0x200E)); // LRM = 0x200E
        if (name == current)
            defaultItem = items.count();
        items << string;
    }
    items << QLatin1String("English (en_US)");
    if (defaultItem == -1)
        defaultItem = items.count() - 1;

    bool ok;
    QString item = QInputDialog::getItem(0,
        tr("Choose language"),
        tr("<p>You can run with a different language than<br>"
        "the operating system default.</p>"
        "<p>Please choose the language which should be used</p>"),
        items, defaultItem, false, &ok);
    if (!ok)
        return;

    int selection = items.indexOf(item);
    setCurrentLanguage(m_languages.value(selection));
}

/*!
    Find all *.qm files in the data directory that have a Qt translation.
 */
void LanguageManager::loadAvailableLanguages() const
{
    if (m_loaded)
        return;
    m_loaded = true;
#ifdef LANGUAGEMANAGER_DEBUG
    qDebug() << "LanguageManager::" << __FUNCTION__;
#endif

    foreach (const QString &dir, m_localeDirectories) {
        QDirIterator it(dir);
        while (it.hasNext()) {
            QString fileName = it.next();
            if (!fileName.endsWith(QLatin1String(".qm")))
                continue;
            const QFileInfo info = it.fileInfo();
            QString language = info.completeBaseName();
            m_languages.append(language);
        }
    }
}
