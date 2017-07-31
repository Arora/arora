/*
 * Copyright 2008 Benjamin C. Meyer <ben@meyerhome.net>
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

#include "locationbarsiteicon.h"

#include <qevent.h>
#include <qurl.h>
#include <QDrag>
#include <QMimeData>

#include "browserapplication.h"
#include "webview.h"

LocationBarSiteIcon::LocationBarSiteIcon(QWidget *parent)
    : QLabel(parent)
    , m_webView(0)
{
    resize(QSize(16, 16));
    webViewSiteIconChanged();
    setCursor(Qt::ArrowCursor);
    show();
}

void LocationBarSiteIcon::setWebView(WebView *webView)
{
    m_webView = webView;
    connect(webView, SIGNAL(loadFinished(bool)),
            this, SLOT(webViewSiteIconChanged()));
    connect(webView, SIGNAL(iconChanged()),
            this, SLOT(webViewSiteIconChanged()));
}

void LocationBarSiteIcon::webViewSiteIconChanged()
{
    QUrl url;
    if (m_webView)
        url = m_webView->url();
    setPixmap(BrowserApplication::instance()->icon(url).pixmap(16, 16));
}

void LocationBarSiteIcon::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragStartPos = event->pos();
    QLabel::mousePressEvent(event);
}

void LocationBarSiteIcon::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton
        && (event->pos() - m_dragStartPos).manhattanLength() > QApplication::startDragDistance()
        && m_webView) {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        QString title = m_webView->title();
        if (title.isEmpty())
            title = QString::fromUtf8(m_webView->url().toEncoded());
        mimeData->setText(title);
        QList<QUrl> urls;
        urls.append(m_webView->url());
        mimeData->setUrls(urls);
        const QPixmap *p = pixmap();
        if (p)
            drag->setPixmap(*p);
        drag->setMimeData(mimeData);
        drag->exec();
    }
}
