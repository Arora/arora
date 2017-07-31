/*
 * Copyright 2009 Jonas Gehring <jonas.gehring@boolsoft.org>
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

#include "fileaccesshandler.h"

#include <qapplication.h>
#include <qcryptographichash.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qfileiconprovider.h>
#include <qhash.h>
#include <qstyle.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qwebsettings.h>

FileAccessHandler::FileAccessHandler(QObject *parent)
    : SchemeAccessHandler(parent)
{
}

QNetworkReply *FileAccessHandler::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    Q_UNUSED(outgoingData);

    switch (op) {
    case QNetworkAccessManager::GetOperation:
        break;
    default:
        return 0;
    }

    // This handler can only list directories yet, so pass anything
    // else back to the manager
    QString path = request.url().toLocalFile();
    if (!QFileInfo(path).isDir()) {
        return 0;
    }

    FileAccessReply *reply = new FileAccessReply(request, this);
    return reply;
}


FileAccessReply::FileAccessReply(const QNetworkRequest &request, QObject *parent)
    : QNetworkReply(parent)
{
    setOperation(QNetworkAccessManager::GetOperation);
    setRequest(request);
    setUrl(request.url());

    buffer.open(QIODevice::ReadWrite);
    setError(QNetworkReply::NoError, tr("No Error"));

    QTimer::singleShot(0, this, SLOT(listDirectory()));

    open(QIODevice::ReadOnly);
}

FileAccessReply::~FileAccessReply()
{
    close();
}

qint64 FileAccessReply::bytesAvailable() const
{
    return buffer.bytesAvailable() + QNetworkReply::bytesAvailable();
}

void FileAccessReply::close()
{
    buffer.close();
}

static QString cssLinkClass(const QIcon &icon, int size = 32)
{
    // The CSS class generation is a bit tricky, because QIcon/QPixmap's
    // cacheKey() returns the different values for the same icons on my Windows
    // box (tested with XP). Thus, the checksum of the actual text of the CSS class
    // is used for the class name.
    QString data = QLatin1String("a.%3 {\n\
  padding-left: %1px;\n\
  background: transparent url(data:image/png;base64,%2) no-repeat center left;\n\
  font-weight: bold;\n\
}\n");
    QPixmap pixmap = icon.pixmap(QSize(size, size));
    QBuffer imageBuffer;
    imageBuffer.open(QBuffer::ReadWrite);
    if (!pixmap.save(&imageBuffer, "PNG")) {
        // If an error occured, write a blank pixmap
        pixmap = QPixmap(size, size);
        pixmap.fill(Qt::transparent);
        imageBuffer.buffer().clear();
        pixmap.save(&imageBuffer, "PNG");
    }
    return data.arg(size+4).arg(QLatin1String(imageBuffer.buffer().toBase64()));
}

void FileAccessReply::listDirectory()
{
    QDir dir(url().toLocalFile());
    if (!dir.exists()) {
        setError(QNetworkReply::ContentNotFoundError, tr("Error opening: %1: No such file or directory").arg(dir.absolutePath()));
        emit error(QNetworkReply::ContentNotFoundError);
        emit finished();
        return;
    }
    if (!dir.isReadable()) {
        setError(QNetworkReply::ContentAccessDenied, tr("Unable to read %1").arg(dir.absolutePath()));
        emit error(QNetworkReply::ContentAccessDenied);
        emit finished();
        return;
    }

    // Format a html page for the directory contents
    QFile dirlistFile(QLatin1String(":/dirlist.html"));
    if (!dirlistFile.open(QIODevice::ReadOnly))
        return;
    QString html = QLatin1String(dirlistFile.readAll());
    html = html.arg(dir.absolutePath(), tr("Contents of %1").arg(dir.absolutePath()));

    // Templates for the listing
    QString link = QLatin1String("<a class=\"%1\" href=\"%2\">%3</a>");
    QString row = QLatin1String("<tr%1> <td class=\"name\">%2</td> <td class=\"size\">%3</td> <td class=\"modified\">%4</td> </tr>\n");

    QFileIconProvider iconProvider;
    QHash<QString, bool> existingClasses;
    int iconSize = QWebSettings::globalSettings()->fontSize(QWebSettings::DefaultFontSize);
    QFileInfoList list = dir.entryInfoList(QDir::AllEntries | QDir::Hidden, QDir::Name | QDir::DirsFirst);
    QString dirlist, classes;

    // Write link to parent directory first
    if (!dir.isRoot()) {
        QIcon icon = qApp->style()->standardIcon(QStyle::SP_FileDialogToParent);
        classes += cssLinkClass(icon, iconSize).arg(QLatin1String("link_parent"));

        QString addr = QString::fromUtf8(QUrl::fromLocalFile(QFileInfo(dir.absoluteFilePath(QLatin1String(".."))).canonicalFilePath()).toEncoded());
        QString size, modified; // Empty by intention
        dirlist += row.arg(QString()).arg(link.arg(QLatin1String("link_parent")).arg(addr).arg(QLatin1String(".."))).arg(size).arg(modified);
    }

    for (int i = 0; i < list.count(); ++i) {
        // Skip '.' and '..'
        if (list[i].fileName() == QLatin1String(".") || list[i].fileName() == QLatin1String("..")) {
            continue;
        }

        // Fetch file icon and generate a corresponding CSS class if necessary
        QIcon icon = iconProvider.icon(list[i]);
        QString cssClass = cssLinkClass(icon, iconSize);
        QByteArray cssData = cssClass.toLatin1();
        QString className = QString(QLatin1String("link_%1")).arg(QLatin1String(QCryptographicHash::hash(cssData, QCryptographicHash::Md4).toHex()));
        if (!existingClasses.contains(className)) {
            classes += cssClass.arg(className);
            existingClasses.insert(className, true);
        }

        QString addr = QString::fromUtf8(QUrl::fromLocalFile(list[i].canonicalFilePath()).toEncoded());
        QString size, modified;
        if (list[i].fileName() != QLatin1String("..")) {
            if (list[i].isFile())
                size = tr("%1 KB").arg(QString::number(list[i].size()/1024));
            modified = list[i].lastModified().toString(Qt::SystemLocaleShortDate);
        }

        QString classes;
        if (list[i].isHidden())
            classes = QLatin1String(" class=\"hidden\"");
        dirlist += row.arg(classes).arg(link.arg(className).arg(addr).arg(list[i].fileName())).arg(size).arg(modified);
    }

    html = html.arg(classes).arg(dirlist).arg(tr("Show Hidden Files"));

    // Save result to buffer
    QTextStream stream(&buffer);
    stream << html;
    stream.flush();
    buffer.reset();

    // Publish result
    setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("text/html"));
    setHeader(QNetworkRequest::ContentLengthHeader, buffer.bytesAvailable());
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
    setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, QByteArray("Ok"));
    emit metaDataChanged();
    emit downloadProgress(buffer.size(), buffer.size());
    QNetworkReply::NetworkError errorCode = error();
    if (errorCode != QNetworkReply::NoError) {
        emit error(errorCode);
    } else if (buffer.size() > 0) {
        emit readyRead();
    }

    emit finished();
}

qint64 FileAccessReply::readData(char *data, qint64 maxSize)
{
    return buffer.read(data, maxSize);
}
