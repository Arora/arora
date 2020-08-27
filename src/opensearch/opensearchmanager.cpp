/*
 * Copyright 2009 Jakub Wieczorek <faw217@gmail.com>
 * Copyright 2009 Christian Franke <cfchris6@ts2server.com>
 * Copyright 2009 Christopher Eby <kreed@kreed.org>
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

#include "opensearchmanager.h"

#include "autosaver.h"
#include "browserapplication.h"
#include "networkaccessmanager.h"
#include "opensearchengine.h"
#include "opensearchreader.h"
#include "opensearchwriter.h"

#include <qdesktopservices.h>
#include <qdir.h>
#include <qdiriterator.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qnetworkreply.h>
#include <qnetworkrequest.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qstringlist.h>

OpenSearchManager::OpenSearchManager(QObject *parent)
    : QObject(parent)
    , m_autoSaver(new AutoSaver(this))
{
    connect(this, SIGNAL(changed()),
            m_autoSaver, SLOT(changeOccurred()));

    load();
}

OpenSearchManager::~OpenSearchManager()
{
    m_autoSaver->saveIfNeccessary();
    qDeleteAll(m_engines.values());
    m_engines.clear();
}

QString OpenSearchManager::currentEngineName() const
{
    return m_current;
}

void OpenSearchManager::setCurrentEngineName(const QString &name)
{
    if (!m_engines.contains(name))
        return;

    m_current = name;
    emit currentEngineChanged();
    emit changed();
}

OpenSearchEngine *OpenSearchManager::currentEngine() const
{
    if (m_current.isEmpty() || !m_engines.contains(m_current))
        return 0;

    return m_engines[m_current];
}

void OpenSearchManager::setCurrentEngine(OpenSearchEngine *engine)
{
    if (!engine)
        return;

    setCurrentEngineName(m_engines.key(engine));
}

OpenSearchEngine *OpenSearchManager::engine(const QString &name)
{
    if (!m_engines.contains(name))
        return 0;

    return m_engines[name];
}

bool OpenSearchManager::engineExists(const QString &name)
{
    return m_engines.contains(name);
}

QStringList OpenSearchManager::allEnginesNames() const
{
    return m_engines.keys();
}

int OpenSearchManager::enginesCount() const
{
    return m_engines.count();
}

void OpenSearchManager::addEngine(const QUrl &url)
{
    if (!url.isValid())
        return;

    QNetworkReply *reply = BrowserApplication::networkAccessManager()->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(engineFromUrlAvailable()));
    reply->setParent(this);
}

bool OpenSearchManager::addEngine(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    OpenSearchReader reader;
    OpenSearchEngine *engine = reader.read(&file);

    if (!addEngine(engine)) {
        delete engine;
        return false;
    }

    return true;
}

bool OpenSearchManager::addEngine(OpenSearchEngine *engine)
{
    if (!engine)
        return false;

    if (!engine->isValid())
        return false;

    if (m_engines.contains(engine->name()))
        return false;

    m_engines[engine->name()] = engine;

    emit changed();

    return true;
}

void OpenSearchManager::removeEngine(const QString &name)
{
    if (m_engines.count() <= 1)
        return;

    if (!m_engines.contains(name))
        return;

    OpenSearchEngine *engine = m_engines[name];
    foreach (const QString &keyword, m_keywords.keys(engine))
        m_keywords.remove(keyword);
    engine->deleteLater();

    m_engines[name] = 0;
    m_engines.remove(name);

    QString file = QDir(enginesDirectory()).filePath(generateEngineFileName(name));
    QFile::remove(file);

    if (name == m_current)
        setCurrentEngineName(m_engines.keys().at(0));

    emit changed();
}

QString OpenSearchManager::generateEngineFileName(const QString &engineName) const
{
    QString fileName;

    // Strip special characters from the name.
    for (int i = 0; i < engineName.count(); ++i) {
        if (engineName.at(i).isSpace()) {
            fileName.append(QLatin1Char('_'));
            continue;
        }

        if (engineName.at(i).isLetterOrNumber())
            fileName.append(engineName.at(i));
    }

    fileName.append(QLatin1String(".xml"));

    return fileName;
}

void OpenSearchManager::saveDirectory(const QString &dirName)
{
    QDir dir;
    if (!dir.mkpath(dirName))
        return;
    dir.setPath(dirName);

    OpenSearchWriter writer;

    foreach (OpenSearchEngine *engine, m_engines.values()) {
        QString name = generateEngineFileName(engine->name());
        QString fileName = dir.filePath(name);

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
            continue;

        writer.write(&file, engine);
    }
}

void OpenSearchManager::save()
{
    saveDirectory(enginesDirectory());

    QSettings settings;
    settings.beginGroup(QLatin1String("openSearch"));
    settings.setValue(QLatin1String("engine"), m_current);

    settings.beginWriteArray(QLatin1String("keywords"), m_keywords.count());
    QHash<QString, OpenSearchEngine*>::const_iterator i = m_keywords.constBegin();
    QHash<QString, OpenSearchEngine*>::const_iterator end = m_keywords.constEnd();
    int j = 0;
    for (; i != end; ++i) {
        settings.setArrayIndex(j++);
        settings.setValue(QLatin1String("keyword"), i.key());
        settings.setValue(QLatin1String("engine"), i.value()->name());
    }
    settings.endArray();

    settings.endGroup();
}

bool OpenSearchManager::loadDirectory(const QString &dirName)
{
    if (!QFile::exists(dirName))
        return false;

    QDirIterator iterator(dirName, QStringList() << QLatin1String("*.xml"));

    if (!iterator.hasNext())
        return false;

    bool success = false;

    while (iterator.hasNext()) {
        if (addEngine(iterator.next()))
            success = true;
    }

    return success;
}

void OpenSearchManager::load()
{
    if (!loadDirectory(enginesDirectory()))
        loadDirectory(QLatin1String(":/searchengines"));

    // get current engine
    QSettings settings;
    settings.beginGroup(QLatin1String("openSearch"));
    m_current = settings.value(QLatin1String("engine"), QLatin1String("Google")).toString();

    int size = settings.beginReadArray(QLatin1String("keywords"));
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString keyword = settings.value(QLatin1String("keyword")).toString();
        QString engineName = settings.value(QLatin1String("engine")).toString();
        m_keywords.insert(keyword, engine(engineName));
    }
    settings.endArray();

    settings.endGroup();

    if (!m_engines.contains(m_current) && m_engines.count() > 0)
        m_current = m_engines.keys().at(0);

    emit currentEngineChanged();
}

void OpenSearchManager::restoreDefaults()
{
    loadDirectory(QLatin1String(":/searchengines"));
}

QString OpenSearchManager::enginesDirectory() const
{
    QDir directory(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/data/Endorphin");
    return directory.filePath(QLatin1String("searchengines"));
}

bool OpenSearchManager::confirmAddition(OpenSearchEngine *engine)
{
    if (!engine || !engine->isValid())
        return false;

    QString host = QUrl(engine->searchUrlTemplate()).host();

    QMessageBox::StandardButton button = QMessageBox::question(0, QString(),
            tr("Do you want to add the following engine to your list of search engines?<br /><br />"
               "Name: %1<br />Searches on: %2").arg(engine->name(), host),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    return (button == QMessageBox::Yes);
}

void OpenSearchManager::engineFromUrlAvailable()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply)
        return;

    if (reply->error() != QNetworkReply::NoError) {
        reply->close();
        reply->deleteLater();
        return;
    }

    OpenSearchReader reader;
    OpenSearchEngine *engine = reader.read(reply);

    reply->close();
    reply->deleteLater();

    if (!engine->isValid()) {
        delete engine;
        return;
    }

    if (engineExists(engine->name())) {
        delete engine;
        return;
    }

    if (!confirmAddition(engine)) {
        delete engine;
        return;
    }

    if (!addEngine(engine)) {
        delete engine;
        return;
    }
}

QUrl OpenSearchManager::convertKeywordSearchToUrl(const QString &string)
{
    int i = string.indexOf(QLatin1Char(' '));
    if (i <= 0)
        return QUrl();

    const QString keyword = string.left(i);
    const QString terms = string.mid(i + 1);
    if (terms.isEmpty())
        return QUrl();

    if (OpenSearchEngine *engine = engineForKeyword(keyword))
        return engine->searchUrl(terms);

    return QUrl();
}

OpenSearchEngine *OpenSearchManager::engineForKeyword(const QString &keyword) const
{
    if (keyword.isEmpty())
        return 0;
    if (!m_keywords.contains(keyword))
        return 0;
    return m_keywords.value(keyword);
}

void OpenSearchManager::setEngineForKeyword(const QString &keyword, OpenSearchEngine *engine)
{
    if (keyword.isEmpty())
        return;

    if (!engine)
        m_keywords.remove(keyword);
    else
        m_keywords.insert(keyword, engine);

    emit changed();
}

QStringList OpenSearchManager::keywordsForEngine(OpenSearchEngine *engine) const
{
    return m_keywords.keys(engine);
}

void OpenSearchManager::setKeywordsForEngine(OpenSearchEngine *engine, const QStringList &keywords)
{
    if (!engine)
        return;

    foreach (const QString &keyword, keywordsForEngine(engine))
        m_keywords.remove(keyword);

    foreach (const QString &keyword, keywords) {
        if (keyword.isEmpty())
            continue;

        m_keywords.insert(keyword, engine);
    }

    emit changed();
}
