CONFIG += qt warn_on
contains(QT_BUILD_PARTS, tools): CONFIG += uitools
else : DEFINES += QT_NO_UITOOLS

win32|os2 : Debug : CONFIG += console

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

QT += network
contains(QT_VERSION, ^5.*) {
    QT += widgets webkitwidgets printsupport
} else {
    QT += webkit
}

# Share object files for faster compiling
RCC_DIR     = $$PWD/.rcc
UI_DIR      = $$PWD/.ui
MOC_DIR     = $$PWD/.moc
OBJECTS_DIR = $$PWD/.obj

exists(../.git/HEAD) {
    GITVERSION=$$system(git log -n1 --pretty=format:%h)
    !isEmpty(GITVERSION) {
        GITCHANGENUMBER=$$system(git log --pretty=format:%h | wc -l)
        DEFINES += GITVERSION=\"\\\"$$GITVERSION\\\"\"
        DEFINES += GITCHANGENUMBER=\"\\\"$$GITCHANGENUMBER\\\"\"
    }
}

FORMS += \
    $$PWD/aboutdialog.ui \
    $$PWD/autofilldialog.ui \
    $$PWD/acceptlanguagedialog.ui \
    $$PWD/downloaditem.ui \
    $$PWD/downloads.ui \
    $$PWD/searchbanner.ui \
    $$PWD/settings.ui

HEADERS += \
    $$PWD/aboutdialog.h \
    $$PWD/acceptlanguagedialog.h \
    $$PWD/autosaver.h \
    $$PWD/autofilldialog.h \
    $$PWD/autofillmanager.h \
    $$PWD/browserapplication.h \
    $$PWD/browsermainwindow.h \
    $$PWD/clearprivatedata.h \
    $$PWD/clearbutton.h \
    $$PWD/downloadmanager.h \
    $$PWD/modelmenu.h \
    $$PWD/modeltoolbar.h \
    $$PWD/plaintexteditsearch.h \
    $$PWD/searchbar.h \
    $$PWD/searchbutton.h \
    $$PWD/searchlineedit.h \
    $$PWD/settings.h \
    $$PWD/sourcehighlighter.h \
    $$PWD/sourceviewer.h \
    $$PWD/tabbar.h \
    $$PWD/tabwidget.h \
    $$PWD/toolbarsearch.h \
    $$PWD/webactionmapper.h \
    $$PWD/webpage.h \
    $$PWD/webview.h \
    $$PWD/webviewsearch.h

SOURCES += \
    $$PWD/aboutdialog.cpp \
    $$PWD/acceptlanguagedialog.cpp \
    $$PWD/autosaver.cpp \
    $$PWD/autofilldialog.cpp \
    $$PWD/autofillmanager.cpp \
    $$PWD/browserapplication.cpp \
    $$PWD/browsermainwindow.cpp \
    $$PWD/clearprivatedata.cpp \
    $$PWD/clearbutton.cpp \
    $$PWD/downloadmanager.cpp \
    $$PWD/modelmenu.cpp \
    $$PWD/modeltoolbar.cpp \
    $$PWD/plaintexteditsearch.cpp \
    $$PWD/searchbar.cpp \
    $$PWD/searchbutton.cpp \
    $$PWD/searchlineedit.cpp \
    $$PWD/settings.cpp \
    $$PWD/sourcehighlighter.cpp \
    $$PWD/sourceviewer.cpp \
    $$PWD/tabbar.cpp \
    $$PWD/tabwidget.cpp \
    $$PWD/toolbarsearch.cpp \
    $$PWD/webactionmapper.cpp \
    $$PWD/webpage.cpp \
    $$PWD/webview.cpp \
    $$PWD/webviewsearch.cpp

include(adblock/adblock.pri)
include(bookmarks/bookmarks.pri)
include(history/history.pri)
include(locationbar/locationbar.pri)
include(network/network.pri)
include(opensearch/opensearch.pri)
include(qwebplugins/qwebplugins.pri)
include(utils/utils.pri)
include(useragent/useragent.pri)

RESOURCES += \
    $$PWD/data/data.qrc \
    $$PWD/data/graphics/graphics.qrc \
    $$PWD/data/searchengines/searchengines.qrc \
    $$PWD/htmls/htmls.qrc

DISTFILES += ../AUTHORS \
    ../ChangeLog \
    ../LICENSE.GPL2 \
    ../LICENSE.GPL3 \
    ../README

win32 {
    RC_FILE = $$PWD/browser.rc
    LIBS += -luser32
}

os2 {
    RC_FILE = $$PWD/browser_os2.rc
}

mac {
    ICON = browser.icns
    QMAKE_INFO_PLIST = Info_mac.plist
}

include(../webkittrunk.pri)

# To support both Qt4 and Qt5
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000

unix {
    PKGDATADIR = $$DATADIR/arora
    DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"
}

win32 {
    LIBS += -ladvapi32
}
