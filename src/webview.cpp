/*
 * Copyright 2008-2009 Benjamin C. Meyer <ben@meyerhome.net>
 * Copyright 2008 Jason A. Donenfeld <Jason@zx2c4.com>
 * Copyright 2008 Ariya Hidayat <ariya.hidayat@gmail.com>
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

#include "webview.h"

#include "adblockdialog.h"
#include "adblockmanager.h"
#include "adblockpage.h"
#include "autofillmanager.h"
#include "addbookmarkdialog.h"
#include "bookmarksmanager.h"
#include "browserapplication.h"
#include "browsermainwindow.h"
#include "downloadmanager.h"
#include "opensearchengine.h"
#include "opensearchengineaction.h"
#include "opensearchmanager.h"
#include "toolbarsearch.h"
#include "webpage.h"

#include <qclipboard.h>
#include <qdebug.h>
#include <qevent.h>
#include <qmenubar.h>
#include <qmimedata.h>
#include <qtimer.h>
#include <qwebframe.h>

#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
#if QT_VERSION < 0x040700
Q_DECLARE_METATYPE(QWebElement)
#endif
#include <qinputdialog.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qstylefactory.h>
#include <qtooltip.h>
#include <qwebelement.h>
#endif

#include <qdebug.h>

WebView::WebView(QWidget *parent)
    : QWebView(parent)
    , m_progress(0)
    , m_currentZoom(100)
    , m_page(new WebPage(this))
#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    , m_enableAccessKeys(true)
    , m_accessKeysPressed(false)
#endif
{
    setPage(m_page);
#if QT_VERSION >= 0x040600
    QPalette p;
    if (p.color(QPalette::Window) != Qt::white) {
        QScopedPointer<QStyle> s(QStyleFactory::create(QLatin1String("windows")));
        p = s->standardPalette();
        setPalette(p);
    }
#endif
    connect(page(), SIGNAL(statusBarMessage(const QString&)),
            SLOT(setStatusBarText(const QString&)));
    connect(this, SIGNAL(loadProgress(int)),
            this, SLOT(setProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinished()));
    connect(page(), SIGNAL(aboutToLoadUrl(const QUrl &)),
            this, SIGNAL(urlChanged(const QUrl &)));
    connect(page(), SIGNAL(downloadRequested(const QNetworkRequest &)),
            this, SLOT(downloadRequested(const QNetworkRequest &)));
    connect(BrowserApplication::instance(), SIGNAL(zoomTextOnlyChanged(bool)),
            this, SLOT(applyZoom()));
    page()->setForwardUnsupportedContent(true);
    setAcceptDrops(true);

    // the zoom values (in percent) are chosen to be like in Mozilla Firefox 3
    m_zoomLevels << 30 << 50 << 67 << 80 << 90;
    m_zoomLevels << 100;
    m_zoomLevels << 110 << 120 << 133 << 150 << 170 << 200 << 240 << 300;
#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    connect(m_page, SIGNAL(loadStarted()),
            this, SLOT(hideAccessKeys()));
    connect(m_page, SIGNAL(scrollRequested(int, int, const QRect &)),
            this, SLOT(hideAccessKeys()));
#endif
    loadSettings();
}

void WebView::loadSettings()
{
#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    QSettings settings;
    settings.beginGroup(QLatin1String("WebView"));
    m_enableAccessKeys = settings.value(QLatin1String("enableAccessKeys"), m_enableAccessKeys).toBool();

    if (!m_enableAccessKeys)
        hideAccessKeys();
#endif
    m_page->loadSettings();
}

#if !(QT_VERSION >= 0x040600)
#include <qdir.h>
// DO NOT CHANGE ANYTHING IN THIS FUNCTION
// You want to change TabWidget::guessUrlFromString()
// This is a copy of QWebView::guessUrlFromStringis from
// QtWebKit and should never be out of sync with that code.
// This function is fragile, very easy to break, and tested in QtWebKit.
QUrl WebView::guessUrlFromString(const QString &string)
{
    QString trimmedString = string.trimmed();

    // Check the most common case of a valid url with scheme and host first
    QUrl url = QUrl::fromEncoded(trimmedString.toUtf8(), QUrl::TolerantMode);
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty())
        return url;

    // Absolute files that exists
    if (QDir::isAbsolutePath(trimmedString) && QFile::exists(trimmedString))
        return QUrl::fromLocalFile(trimmedString);

    // If the string is missing the scheme or the scheme is not valid prepend a scheme
    QString scheme = url.scheme();
    if (scheme.isEmpty() || scheme.contains(QLatin1Char('.')) || scheme == QLatin1String("localhost")) {
        // Do not do anything for strings such as "foo", only "foo.com"
        int dotIndex = trimmedString.indexOf(QLatin1Char('.'));
        if (dotIndex != -1 || trimmedString.startsWith(QLatin1String("localhost"))) {
            const QString hostscheme = trimmedString.left(dotIndex).toLower();
            QByteArray scheme = (hostscheme == QLatin1String("ftp")) ? "ftp" : "http";
            trimmedString = QLatin1String(scheme) + QLatin1String("://") + trimmedString;
        }
        url = QUrl::fromEncoded(trimmedString.toUtf8(), QUrl::TolerantMode);
    }

    if (url.isValid())
        return url;

    return QUrl();
}
#endif

TabWidget *WebView::tabWidget() const
{
    QObject *widget = this->parent();
    while (widget) {
        if (TabWidget *tw = qobject_cast<TabWidget*>(widget))
            return tw;
        widget = widget->parent();
    }
    return 0;
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);

    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());

    if (!r.linkUrl().isEmpty()) {
        QAction *newWindowAction = menu->addAction(tr("Open in New &Window"), this, SLOT(openActionUrlInNewWindow()));
        newWindowAction->setData(r.linkUrl());
        QAction *newTabAction = menu->addAction(tr("Open in New &Tab"), this, SLOT(openActionUrlInNewTab()));
        newTabAction->setData(r.linkUrl());
        menu->addSeparator();
        menu->addAction(tr("Save Lin&k"), this, SLOT(downloadLinkToDisk()));
        menu->addAction(tr("&Bookmark This Link"), this, SLOT(bookmarkLink()))->setData(r.linkUrl());
        menu->addSeparator();
        if (!page()->selectedText().isEmpty())
            menu->addAction(pageAction(QWebPage::Copy));
        menu->addAction(tr("&Copy Link Location"), this, SLOT(copyLinkToClipboard()));
    }

    if (!r.imageUrl().isEmpty()) {
        if (!menu->isEmpty())
            menu->addSeparator();
        QAction *newWindowAction = menu->addAction(tr("Open Image in New &Window"), this, SLOT(openActionUrlInNewWindow()));
        newWindowAction->setData(r.imageUrl());
        QAction *newTabAction = menu->addAction(tr("Open Image in New &Tab"), this, SLOT(openActionUrlInNewTab()));
        newTabAction->setData(r.imageUrl());
        menu->addSeparator();
        menu->addAction(tr("&Save Image"), this, SLOT(downloadImageToDisk()));
        menu->addAction(tr("&Copy Image"), this, SLOT(copyImageToClipboard()));
        menu->addAction(tr("C&opy Image Location"), this, SLOT(copyImageLocationToClipboard()))->setData(r.imageUrl().toString());
        menu->addSeparator();
        menu->addAction(tr("Block Image"), this, SLOT(blockImage()))->setData(r.imageUrl().toString());
    }

    if (!page()->selectedText().isEmpty()) {
        if (menu->isEmpty()) {
            menu->addAction(pageAction(QWebPage::Copy));
        } else {
            menu->addSeparator();
        }
        QMenu *searchMenu = menu->addMenu(tr("Search with..."));

        QList<QString> list = ToolbarSearch::openSearchManager()->allEnginesNames();
        for (int i = 0; i < list.count(); ++i) {
            QString name = list.at(i);
            OpenSearchEngine *engine = ToolbarSearch::openSearchManager()->engine(name);
            QAction *action = new OpenSearchEngineAction(engine, searchMenu);
            searchMenu->addAction(action);
            action->setData(name);
        }

        connect(searchMenu, SIGNAL(triggered(QAction *)), this, SLOT(searchRequested(QAction *)));
    }

#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    QWebElement element = r.element();
    if (!element.isNull()
        && element.tagName().toLower() == QLatin1String("input")
        && element.attribute(QLatin1String("type"), QLatin1String("text")) == QLatin1String("text")) {
        if (menu->isEmpty()) {
            menu->addAction(pageAction(QWebPage::Copy));
        } else {
            menu->addSeparator();
        }

        QVariant variant;
        variant.setValue(element);
        menu->addAction(tr("Add to the toolbar search"), this, SLOT(addSearchEngine()))->setData(variant);
    }
#endif

    if (menu->isEmpty()) {
        delete menu;
        menu = page()->createStandardContextMenu();
    } else {
        if (page()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled))
            menu->addAction(pageAction(QWebPage::InspectElement));
    }

    if (!menu->isEmpty()) {
        if (BrowserMainWindow::parentWindow(tabWidget())->menuBar()->isHidden()) {
            menu->addSeparator();
            menu->addAction(BrowserMainWindow::parentWindow(tabWidget())->showMenuBarAction());
        }

        menu->exec(mapToGlobal(event->pos()));
        delete menu;
        return;
    }
    delete menu;

    QWebView::contextMenuEvent(event);
}

void WebView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        m_currentZoom = m_currentZoom + numSteps * 10;
        applyZoom();
        event->accept();
        return;
    }
    QWebView::wheelEvent(event);
}

void WebView::resizeEvent(QResizeEvent *event)
{
    int offset = event->size().height() - event->oldSize().height();
    int currentValue = page()->mainFrame()->scrollBarValue(Qt::Vertical);
    setUpdatesEnabled(false);
    page()->mainFrame()->setScrollBarValue(Qt::Vertical, currentValue - offset);
    setUpdatesEnabled(true);
    QWebView::resizeEvent(event);
}

void WebView::downloadLinkToDisk()
{
    pageAction(QWebPage::DownloadLinkToDisk)->trigger();
}

void WebView::copyLinkToClipboard()
{
    pageAction(QWebPage::CopyLinkToClipboard)->trigger();
}

void WebView::openActionUrlInNewTab()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        QWebPage *page = tabWidget()->getView(TabWidget::NewNotSelectedTab, this)->page();
        QNetworkRequest request(action->data().toUrl());
        request.setRawHeader("Referer", url().toEncoded());
        page->mainFrame()->load(request);
    }
}

void WebView::openActionUrlInNewWindow()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        QWebPage *page = tabWidget()->getView(TabWidget::NewWindow, this)->page();
        QNetworkRequest request(action->data().toUrl());
        request.setRawHeader("Referer", url().toEncoded());
        page->mainFrame()->load(request);
    }
}

void WebView::openImageInNewWindow()
{
    pageAction(QWebPage::OpenImageInNewWindow)->trigger();
}

void WebView::downloadImageToDisk()
{
    pageAction(QWebPage::DownloadImageToDisk)->trigger();
}

void WebView::copyImageToClipboard()
{
    pageAction(QWebPage::CopyImageToClipboard)->trigger();
}

void WebView::copyImageLocationToClipboard()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        BrowserApplication::clipboard()->setText(action->data().toString());
    }
}

void WebView::blockImage()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        QString imageUrl = action->data().toString();
        AdBlockDialog *dialog = AdBlockManager::instance()->showDialog();
        dialog->addCustomRule(imageUrl);
    }
}

void WebView::bookmarkLink()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        AddBookmarkDialog dialog;
        dialog.setUrl(QString::fromUtf8(action->data().toUrl().toEncoded()));
        dialog.exec();
    }
}

void WebView::searchRequested(QAction *action)
{
    QString searchText = page()->selectedText();

    if (searchText.isEmpty())
        return;

    QVariant index = action->data();

    if (index.canConvert<QString>()) {
        OpenSearchEngine *engine = ToolbarSearch::openSearchManager()->engine(index.toString());
        emit search(engine->searchUrl(searchText), TabWidget::NewSelectedTab);
    }
}

#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
void WebView::addSearchEngine()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
        return;

    QVariant variant = action->data();
    if (!variant.canConvert<QWebElement>())
        return;

    QWebElement element = qvariant_cast<QWebElement>(variant);
    QString elementName = element.attribute(QLatin1String("name"));
    QWebElement formElement = element;
    while (formElement.tagName().toLower() != QLatin1String("form"))
        formElement = formElement.parent();

    if (formElement.isNull() || formElement.attribute(QLatin1String("action")).isEmpty())
        return;

    QString method = formElement.attribute(QLatin1String("method"), QLatin1String("get")).toLower();
    if (method != QLatin1String("get")) {
        QMessageBox::warning(this, tr("Method not supported"),
                             tr("%1 method is not supported.").arg(method.toUpper()));
        return;
    }

    QUrl searchUrl(page()->mainFrame()->baseUrl().resolved(QUrl(formElement.attribute(QLatin1String("action")))));
    QMap<QString, QString> searchEngines;
    QWebElementCollection inputFields = formElement.findAll(QLatin1String("input"));
    foreach (QWebElement inputField, inputFields) {
        QString type = inputField.attribute(QLatin1String("type"), QLatin1String("text"));
        QString name = inputField.attribute(QLatin1String("name"));
        QString value = inputField.evaluateJavaScript(QLatin1String("this.value")).toString();

        if (type == QLatin1String("submit")) {
            searchEngines.insert(value, name);
        } else if (type == QLatin1String("text")) {
            if (inputField == element)
                value = QLatin1String("{searchTerms}");

            searchUrl.addQueryItem(name, value);
        } else if (type == QLatin1String("checkbox") || type == QLatin1String("radio")) {
            if (inputField.evaluateJavaScript(QLatin1String("this.checked")).toBool()) {
                searchUrl.addQueryItem(name, value);
            }
        } else if (type == QLatin1String("hidden")) {
            searchUrl.addQueryItem(name, value);
        }
    }

    QWebElementCollection selectFields = formElement.findAll(QLatin1String("select"));
    foreach (QWebElement selectField, selectFields) {
        QString name = selectField.attribute(QLatin1String("name"));
        int selectedIndex = selectField.evaluateJavaScript(QLatin1String("this.selectedIndex")).toInt();
        if (selectedIndex == -1)
            continue;

        QWebElementCollection options = selectField.findAll(QLatin1String("option"));
        QString value = options.at(selectedIndex).toPlainText();
        searchUrl.addQueryItem(name, value);
    }

    bool ok = true;
    if (searchEngines.count() > 1) {
        QString searchEngine = QInputDialog::getItem(this, tr("Search engine"),
                                                    tr("Choose the desired search engine"), searchEngines.keys(),
                                                    0, false, &ok);
        if (!ok)
            return;
        if (!searchEngines[searchEngine].isEmpty())
            searchUrl.addQueryItem(searchEngines[searchEngine], searchEngine);
    }

    QString engineName;
    QWebElementCollection labels = formElement.findAll(QString(QLatin1String("label[for=\"%1\"]")).arg(elementName));
    if (labels.count() > 0)
        engineName = labels.at(0).toPlainText();

    engineName = QInputDialog::getText(this, tr("Engine name"), tr("Type in a name for the engine"),
                                       QLineEdit::Normal, engineName, &ok);
    if (!ok)
        return;

    OpenSearchEngine *engine = new OpenSearchEngine();
    engine->setName(engineName);
    engine->setDescription(engineName);
    engine->setSearchUrlTemplate(searchUrl.toString());
    engine->setImage(icon().pixmap(16, 16).toImage());

    ToolbarSearch::openSearchManager()->addEngine(engine);
}
#endif

void WebView::setProgress(int progress)
{
    m_progress = progress;
}

int WebView::levelForZoom(int zoom)
{
    int i;

    i = m_zoomLevels.indexOf(zoom);
    if (i >= 0)
        return i;

    for (i = 0 ; i < m_zoomLevels.count(); ++i)
        if (zoom <= m_zoomLevels[i])
            break;

    if (i == m_zoomLevels.count())
        return i - 1;
    if (i == 0)
        return i;

    if (zoom - m_zoomLevels[i-1] > m_zoomLevels[i] - zoom)
        return i;
    else
        return i-1;
}

void WebView::applyZoom()
{
    setZoomFactor(qreal(m_currentZoom) / 100.0);
}

void WebView::zoomIn()
{
    int i = levelForZoom(m_currentZoom);

    if (i < m_zoomLevels.count() - 1)
        m_currentZoom = m_zoomLevels[i + 1];
    applyZoom();
}

void WebView::zoomOut()
{
    int i = levelForZoom(m_currentZoom);

    if (i > 0)
        m_currentZoom = m_zoomLevels[i - 1];
    applyZoom();
}

void WebView::resetZoom()
{
    m_currentZoom = 100;
    applyZoom();
}

void WebView::loadFinished()
{
    if (100 != m_progress) {
        qWarning() << "Received finished signal while progress is still:" << progress()
                   << "Url:" << url();
    }
    m_progress = 0;
    AdBlockManager::instance()->page()->applyRulesToPage(page());
    BrowserApplication::instance()->autoFillManager()->fill(page());
}

void WebView::loadUrl(const QUrl &url, const QString &title)
{
    if (url.scheme() == QLatin1String("javascript")) {
        QString scriptSource = QUrl::fromPercentEncoding(url.toString(Q_FLAGS(QUrl::TolerantMode|QUrl::RemoveScheme)).toAscii());
        QVariant result = page()->mainFrame()->evaluateJavaScript(scriptSource);
        return;
    }
    m_initialUrl = url;
    if (!title.isEmpty())
        emit titleChanged(tr("Loading..."));
    else
        emit titleChanged(title);
    load(url);
}

QString WebView::lastStatusBarText() const
{
    return m_statusBarText;
}

QUrl WebView::url() const
{
    QUrl url = QWebView::url();
    if (!url.isEmpty())
        return url;

    return m_initialUrl;
}

void WebView::mousePressEvent(QMouseEvent *event)
{
    BrowserApplication::instance()->setEventMouseButtons(event->buttons());
    BrowserApplication::instance()->setEventKeyboardModifiers(event->modifiers());

    switch (event->button()) {
    case Qt::XButton1:
        pageAction(WebPage::Back)->trigger();
        break;
    case Qt::XButton2:
        pageAction(WebPage::Forward)->trigger();
        break;
    default:
        QWebView::mousePressEvent(event);
        break;
    }
}

void WebView::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void WebView::dragMoveEvent(QDragMoveEvent *event)
{
    event->ignore();
    if (event->source() != this) {
        if (!event->mimeData()->urls().isEmpty()) {
            event->acceptProposedAction();
        } else {
            QUrl url(event->mimeData()->text());
            if (url.isValid())
                event->acceptProposedAction();
        }
    }
    if (!event->isAccepted()) {
        QWebView::dragMoveEvent(event);
    }
}

void WebView::dropEvent(QDropEvent *event)
{
    QWebView::dropEvent(event);
    if (!event->isAccepted()
        && event->source() != this
        && event->possibleActions() & Qt::CopyAction) {

        QUrl url;
        if (!event->mimeData()->urls().isEmpty())
            url = event->mimeData()->urls().first();
        if (!url.isValid())
            url = event->mimeData()->text();
        if (url.isValid()) {
            loadUrl(url);
            event->acceptProposedAction();
        }
    }
}

void WebView::mouseReleaseEvent(QMouseEvent *event)
{
    const bool isAccepted = event->isAccepted();
    m_page->event(event);
    if (!event->isAccepted()
        && (BrowserApplication::instance()->eventMouseButtons() & Qt::MidButton)) {
        QUrl url(QApplication::clipboard()->text(QClipboard::Selection));
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty()) {
            BrowserApplication::instance()->setEventMouseButtons(Qt::NoButton);
            loadUrl(url);
        }
    }
    event->setAccepted(isAccepted);
}

void WebView::setStatusBarText(const QString &string)
{
    m_statusBarText = string;
}

void WebView::downloadRequested(const QNetworkRequest &request)
{
    BrowserApplication::downloadManager()->download(request);
}

void WebView::keyPressEvent(QKeyEvent *event)
{
#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    if (m_enableAccessKeys) {
        m_accessKeysPressed = (event->modifiers() == Qt::ControlModifier
                               && event->key() == Qt::Key_Control);
        if (!m_accessKeysPressed) {
            if (checkForAccessKey(event)) {
                hideAccessKeys();
                event->accept();
                return;
            }
            hideAccessKeys();
        } else {
            QTimer::singleShot(300, this, SLOT(accessKeyShortcut()));
        }
    }
#endif

#if QT_VERSION < 0x040600
    switch (event->key()) {
    case Qt::Key_Back:
        pageAction(WebPage::Back)->trigger();
        event->accept();
        break;
    case Qt::Key_Forward:
        pageAction(WebPage::Forward)->trigger();
        event->accept();
        break;
    case Qt::Key_Stop:
        pageAction(WebPage::Stop)->trigger();
        event->accept();
        break;
    case Qt::Key_Refresh:
        pageAction(WebPage::Reload)->trigger();
        event->accept();
        break;
    default:
        QWebView::keyPressEvent(event);
        return;
    }
#endif
    QWebView::keyPressEvent(event);
}

#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
void WebView::accessKeyShortcut()
{
    if (!hasFocus()
        || !m_accessKeysPressed
        || !m_enableAccessKeys)
        return;
    if (m_accessKeyLabels.isEmpty()) {
        showAccessKeys();
    } else {
        hideAccessKeys();
    }
    m_accessKeysPressed = false;
}

void WebView::keyReleaseEvent(QKeyEvent *event)
{
    if (m_enableAccessKeys)
        m_accessKeysPressed = event->key() == Qt::Key_Control;
    QWebView::keyReleaseEvent(event);
}

void WebView::focusOutEvent(QFocusEvent *event)
{
    if (m_accessKeysPressed) {
        hideAccessKeys();
        m_accessKeysPressed = false;
    }
    QWebView::focusOutEvent(event);
}

bool WebView::checkForAccessKey(QKeyEvent *event)
{
    if (m_accessKeyLabels.isEmpty())
        return false;

    QString text = event->text();
    if (text.isEmpty())
        return false;
    QChar key = text.at(0).toUpper();
    bool handled = false;
    if (m_accessKeyNodes.contains(key)) {
        QWebElement element = m_accessKeyNodes[key];
        QPoint p = element.geometry().center();
        QWebFrame *frame = element.webFrame();
        Q_ASSERT(frame);
        do {
            p -= frame->scrollPosition();
            frame = frame->parentFrame();
        } while (frame && frame != page()->mainFrame());
        QMouseEvent pevent(QEvent::MouseButtonPress, p, Qt::LeftButton, 0, 0);
        qApp->sendEvent(this, &pevent);
        QMouseEvent revent(QEvent::MouseButtonRelease, p, Qt::LeftButton, 0, 0);
        qApp->sendEvent(this, &revent);
        handled = true;
    }
    return handled;
}

void WebView::hideAccessKeys()
{
    if (!m_accessKeyLabels.isEmpty()) {
        for (int i = 0; i < m_accessKeyLabels.count(); ++i) {
            QLabel *label = m_accessKeyLabels[i];
            label->hide();
            label->deleteLater();
        }
        m_accessKeyLabels.clear();
        m_accessKeyNodes.clear();
        update();
    }
}

void WebView::showAccessKeys()
{
    QStringList supportedElement;
    supportedElement << QLatin1String("input")
                     << QLatin1String("a")
                     << QLatin1String("area")
                     << QLatin1String("button")
                     << QLatin1String("label")
                     << QLatin1String("legend")
                     << QLatin1String("textarea");

    QList<QChar> unusedKeys;
    for (char c = 'A'; c <= 'Z'; ++c)
        unusedKeys << QLatin1Char(c);
    for (char c = '0'; c <= '9'; ++c)
        unusedKeys << QLatin1Char(c);

    QRect viewport = QRect(m_page->mainFrame()->scrollPosition(), m_page->viewportSize());
    // Priority first goes to elements with accesskey attributes
    QList<QWebElement> alreadyLabeled;
    foreach (const QString &elementType, supportedElement) {
        QList<QWebElement> result = page()->mainFrame()->findAllElements(elementType).toList();
        foreach (const QWebElement &element, result) {
            const QRect geometry = element.geometry();
            if (geometry.size().isEmpty()
                || !viewport.contains(geometry.topLeft())) {
                continue;
            }
            QString accessKeyAttribute = element.attribute(QLatin1String("accesskey")).toUpper();
            if (accessKeyAttribute.isEmpty())
                continue;
            QChar accessKey;
            for (int i = 0; i < accessKeyAttribute.count(); i+=2) {
                const QChar &possibleAccessKey = accessKeyAttribute[i];
                if (unusedKeys.contains(possibleAccessKey)) {
                    accessKey = possibleAccessKey;
                    break;
                }
            }
            if (accessKey.isNull())
                continue;
            unusedKeys.removeOne(accessKey);
            makeAccessKeyLabel(accessKey, element);
            alreadyLabeled.append(element);
        }
    }

    // Pick an access key first from the letters in the text and then from the
    // list of unused access keys
    foreach (const QString &elementType, supportedElement) {
        QWebElementCollection result = page()->mainFrame()->findAllElements(elementType);
        foreach (const QWebElement &element, result) {
            const QRect geometry = element.geometry();
            if (unusedKeys.isEmpty()
                || alreadyLabeled.contains(element)
                || geometry.size().isEmpty()
                || !viewport.contains(geometry.topLeft())) {
                continue;
            }
            QChar accessKey;
            QString text = element.toPlainText().toUpper();
            for (int i = 0; i < text.count(); ++i) {
                const QChar &c = text.at(i);
                if (unusedKeys.contains(c)) {
                    accessKey = c;
                    break;
                }
            }
            if (accessKey.isNull())
                accessKey = unusedKeys.takeFirst();
            unusedKeys.removeOne(accessKey);
            makeAccessKeyLabel(accessKey, element);
        }
    }
}

void WebView::makeAccessKeyLabel(const QChar &accessKey, const QWebElement &element)
{
    QLabel *label = new QLabel(this);
    label->setText(QString(QLatin1String("<qt><b>%1</b>")).arg(accessKey));

    QPalette p = QToolTip::palette();
    QColor color(Qt::yellow);
    color = color.lighter(150);
    color.setAlpha(175);
    p.setColor(QPalette::Window, color);
    label->setPalette(p);
    label->setAutoFillBackground(true);
    label->setFrameStyle(QFrame::Box | QFrame::Plain);
    QPoint point = element.geometry().center();
    point -= m_page->mainFrame()->scrollPosition();
    label->move(point);
    label->show();
    point.setX(point.x() - label->width() / 2);
    label->move(point);
    m_accessKeyLabels.append(label);
    m_accessKeyNodes[accessKey] = element;
}

#endif
