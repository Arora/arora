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

#ifndef WEBPAGE_H
#define WEBPAGE_H

#include "webpageproxy.h"
#include "tabwidget.h"

#include <qlist.h>
#include <qnetworkrequest.h>

class WebPageLinkedResource
{
public:
    QString rel;
    QString type;
    QUrl href;
    QString title;
};

class OpenSearchEngine;
class QNetworkReply;
class WebPluginFactory;
// See https://developer.mozilla.org/en/adding_search_engines_from_web_pages
class JavaScriptExternalObject : public QObject
{
    Q_OBJECT

public:
    JavaScriptExternalObject(QObject *parent = 0);

public slots:
    void AddSearchProvider(const QString &url);
};

class JavaScriptEndorphinObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject *currentEngine READ currentEngine)

public:
    JavaScriptEndorphinObject(QObject *parent = 0);

public slots:
    QString translate(const QString &string);
    QObject *currentEngine() const;
    QString searchUrl(const QString &string) const;
};

class WebPage : public WebPageProxy
{
    Q_OBJECT

signals:
    void aboutToLoadUrl(const QUrl &url);

public:
    WebPage(QObject *parent = 0);
    ~WebPage();

    void loadSettings();

    static WebPluginFactory *webPluginFactory();
    QList<WebPageLinkedResource> linkedResources(const QString &relation = QString());

    static QString userAgent();
    static void setUserAgent(const QString &userAgent);

protected:
    QString userAgentForUrl(const QUrl &url) const;
    bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request,
                                 NavigationType type);
    QObject *createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);
    QWebPage *createWindow(QWebPage::WebWindowType type);

protected slots:
    void handleUnsupportedContent(QNetworkReply *reply);
    void addExternalBinding(QWebFrame *frame = 0);

protected:
    void populateNetworkRequest(QNetworkRequest &request);
    static QString s_userAgent;
    static WebPluginFactory *s_webPluginFactory;
    TabWidget::OpenUrlIn m_openTargetBlankLinksIn;
    QUrl m_requestedUrl;
    JavaScriptExternalObject *m_javaScriptExternalObject;
    JavaScriptEndorphinObject *m_javaScriptEndorphinObject;

private:
    QNetworkRequest lastRequest;
    QWebPage::NavigationType lastRequestType;

};

#endif // WEBPAGE_H

