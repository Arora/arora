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

#ifndef WEBPLUGINFACTORY_H
#define WEBPLUGINFACTORY_H

#include <qwebpluginfactory.h>
#include <qhash.h>

#include "endorphinwebplugin.h"

class WebPluginFactory : public QWebPluginFactory
{
    Q_OBJECT

public:
    WebPluginFactory(QObject *parent = 0);
    ~WebPluginFactory();

    QObject *create(const QString &mimeType, const QUrl &url,
                    const QStringList &argumentNames, const QStringList &argumentValues) const;
    QList<QWebPluginFactory::Plugin> plugins() const;
    void refreshPlugins();

private:
    void init() const;
    mutable bool m_loaded;
    mutable QList<EndorphinWebPlugin*> m_plugins;
    mutable QHash<QString, EndorphinWebPlugin*> m_pluginsCache;
};

#endif // WEBPLUGINFACTORY_H

