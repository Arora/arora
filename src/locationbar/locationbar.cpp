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

#include "locationbar.h"

#include "browserapplication.h"
#include "clearbutton.h"
#include "locationbarsiteicon.h"
#include "privacyindicator.h"
#include "searchlineedit.h"
#include "webview.h"

#include <qdrag.h>
#include <qevent.h>
#include <qpainter.h>
#include <qstyleoption.h>
#include <QMimeData>

#include <qdebug.h>

LocationBar::LocationBar(QWidget *parent)
    : LineEdit(parent)
    , m_webView(0)
    , m_siteIcon(0)
    , m_privacyIndicator(0)
{
    // Urls are always LeftToRight
    setLayoutDirection(Qt::LeftToRight);

    setUpdatesEnabled(false);
    // site icon on the left
    m_siteIcon = new LocationBarSiteIcon(this);
    addWidget(m_siteIcon, LeftSide);

    // privacy indicator at rightmost position
    m_privacyIndicator = new PrivacyIndicator(this);
    addWidget(m_privacyIndicator, RightSide);

    // clear button on the right
    ClearButton *m_clearButton = new ClearButton(this);
    connect(m_clearButton, SIGNAL(clicked()),
            this, SLOT(clear()));
    connect(this, SIGNAL(textChanged(const QString&)),
            m_clearButton, SLOT(textChanged(const QString&)));
    addWidget(m_clearButton, RightSide);

    updateTextMargins();
    setUpdatesEnabled(true);
}

void LocationBar::setWebView(WebView *webView)
{
    Q_ASSERT(webView);
    m_webView = webView;
    m_siteIcon->setWebView(webView);
    connect(webView, SIGNAL(urlChanged(const QUrl &)),
            this, SLOT(webViewUrlChanged(const QUrl &)));
    connect(webView, SIGNAL(loadProgress(int)),
            this, SLOT(update()));
}

WebView *LocationBar::webView() const
{
    return m_webView;
}

void LocationBar::webViewUrlChanged(const QUrl &url)
{
    if (hasFocus())
        return;
    setText(QString::fromUtf8(url.toEncoded()));
    setCursorPosition(0);
}

void LocationBar::paintEvent(QPaintEvent *event)
{
    QPalette p = palette();
    QColor defaultBaseColor = QApplication::palette().color(QPalette::Base);
    QColor backgroundColor = defaultBaseColor;
    if (m_webView && m_webView->url().scheme() == QLatin1String("https")
        && p.color(QPalette::Text).value() < 128) {
        QColor lightYellow(248, 248, 210);
        backgroundColor = lightYellow;
    }

    // set the progress bar
    if (m_webView) {
        int progress = m_webView->progress();
        if (progress == 0) {
            p.setBrush(QPalette::Base, backgroundColor);
        } else {
            QColor loadingColor = QColor(116, 192, 250);
            if (p.color(QPalette::Text).value() >= 128)
                loadingColor = defaultBaseColor.darker(200);

            QLinearGradient gradient(0, 0, width(), 0);
            gradient.setColorAt(0, loadingColor);
            gradient.setColorAt(((double)progress)/100, backgroundColor);
            p.setBrush(QPalette::Base, gradient);
        }
        setPalette(p);
    }

    LineEdit::paintEvent(event);
}

void LocationBar::focusOutEvent(QFocusEvent *event)
{
    if (text().isEmpty() && m_webView)
        webViewUrlChanged(m_webView->url());
    QLineEdit::focusOutEvent(event);
}

void LocationBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        selectAll();
    else
        QLineEdit::mouseDoubleClickEvent(event);
}

void LocationBar::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && m_webView) {
        setText(QString::fromUtf8(m_webView->url().toEncoded()));
        selectAll();
        return;
    }

    QString currentText = text().trimmed();
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        && !currentText.startsWith(QLatin1String("http://"), Qt::CaseInsensitive)) {
        QString append;
        if (event->modifiers() == Qt::ControlModifier)
            append = QLatin1String(".com");
        else if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
            append = QLatin1String(".org");
        else if (event->modifiers() == Qt::ShiftModifier)
            append = QLatin1String(".net");
        QUrl url(QLatin1String("http://") + currentText);
        QString host = url.host();
        if (!host.endsWith(append, Qt::CaseInsensitive)) {
            host += append;
            url.setHost(host);
            setText(url.toString());
        }
    }

    LineEdit::keyPressEvent(event);
}

void LocationBar::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls() || mimeData->hasText())
        event->acceptProposedAction();

    LineEdit::dragEnterEvent(event);
}

void LocationBar::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    QUrl url;
    if (mimeData->hasUrls())
        url = mimeData->urls().at(0);
    else if (mimeData->hasText())
        url = QUrl::fromEncoded(mimeData->text().toUtf8(), QUrl::TolerantMode);

    if (url.isEmpty() || !url.isValid()) {
        LineEdit::dropEvent(event);
        return;
    }

    setText(QString::fromUtf8(url.toEncoded()));
    selectAll();

    event->acceptProposedAction();
}
