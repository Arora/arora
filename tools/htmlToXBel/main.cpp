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

#include <QApplication>
#include <QtWebKitWidgets>

/*!
    A tool to convert html bookmark files into the xbel format.

    The html bookmark files should be DOCTYPE: NETSCAPE-Bookmark-file-1

    More information about XBel can be found here: http://pyxml.sourceforge.net/topics/xbel/
*/
int main(int argc, char **argv)
{
    QApplication application(argc, argv);

    QFile inFile;
    QFile outFile;
    QTextStream in(&inFile);
    QTextStream out(&outFile);

    // Either read in from stdin and output to stdout
    // or read in from a file and output to a file
    // Example: ./app foo.html -o bar.xbel
    bool setInput = false;
    bool setOutput = false;
    QStringList args = application.arguments();
    args.pop_front();
    foreach (const QString &arg, args) {
        if (arg == QLatin1String("-o")) {
            setOutput = true;
        } else if (setOutput) {
            outFile.setFileName(arg);
            outFile.open(QIODevice::WriteOnly);
        } else if (QFile::exists(arg)) {
            setInput = true;
            inFile.setFileName(arg);
            inFile.open(QIODevice::ReadOnly);
        } else {
            qWarning() << "Usage: htmlToXBel"
                        << "[stdin|htmlfile]" << "[stdout|-o outFile]";
            return 1;
        }
    }

    if (!setInput)
        inFile.open(stdin, QIODevice::ReadOnly);
    if (!setOutput)
        outFile.open(stdout, QIODevice::WriteOnly);
    if (inFile.openMode() == QIODevice::NotOpen
        || outFile.openMode() == QIODevice::NotOpen) {
        qWarning() << "Unable to open streams";
        return 1;
    }

    QWebPage webPage;
    webPage.mainFrame()->setHtml(inFile.readAll());
    QFile jsFile(":/extract.js");
    if (!jsFile.open(QFile::ReadOnly)) {
        qWarning() << "Unable to load javascript to extract bookmarks.";
        return 1;
    }
    QString result = webPage.mainFrame()->evaluateJavaScript(jsFile.readAll()).toString();
    if (result.isEmpty()) {
        qWarning() << "Error while extracting bookmarks.";
        return 1;
    }
    out << result;
    return 0;
}
