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

#include "webpluginfactory.h"

#include <qwidget.h>
#include <qurl.h>
#include <qdebug.h>

// #define WEBPLUGINFACTORY_DEBUG

WebPluginFactory::WebPluginFactory(QObject *parent)
    : QWebPluginFactory(parent)
    , m_loaded(false)
{
}

WebPluginFactory::~WebPluginFactory()
{
    qDeleteAll(m_plugins);
    m_plugins.clear();
}

QObject *WebPluginFactory::create(const QString &mimeType, const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues) const
{
    if (!m_loaded)
        init();

#ifdef WEBPLUGINFACTORY_DEBUG
    qDebug() << "WebPluginFactory::" << __FUNCTION__ << mimeType << url << argumentNames << argumentValues;
#endif
    AroraWebPlugin *plugin = m_pluginsCache[mimeType];
    if (plugin)
        return plugin->create(mimeType, url, argumentNames, argumentValues);
#ifdef WEBPLUGINFACTORY_DEBUG
    qDebug() << "WebPluginFactory::" << __FUNCTION__ << "No match";
#endif
    return 0;
}

QList<QWebPluginFactory::Plugin> WebPluginFactory::plugins() const
{
#ifdef WEBPLUGINFACTORY_DEBUG
    qDebug() << "WebPluginFactory::" << __FUNCTION__;
#endif
    if (!m_loaded)
        init();
    QList<QWebPluginFactory::Plugin> plugins;
    foreach (AroraWebPlugin *plugin, m_plugins) {
        QWebPluginFactory::Plugin pluginInfo = plugin->metaPlugin();
        if (!plugin->isAnonymous())
            plugins.append(pluginInfo);
    }
    return plugins;
}

void WebPluginFactory::refreshPlugins()
{
#ifdef WEBPLUGINFACTORY_DEBUG
    qDebug() << "WebPluginFactory::" << __FUNCTION__;
#endif
    init();
    QWebPluginFactory::refreshPlugins();
}

void WebPluginFactory::init() const
{
    m_loaded = true;
    m_pluginsCache.clear();
    qDeleteAll(m_plugins);
    m_plugins.clear();
    foreach (AroraWebPlugin *plugin, m_plugins) {
        foreach (const QWebPluginFactory::MimeType &pluginMimeType, plugin->metaPlugin().mimeTypes)
            m_pluginsCache.insert(pluginMimeType.name, plugin);
    }
}
