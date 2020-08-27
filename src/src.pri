CONFIG += qt warn_on
contains(QT_BUILD_PARTS, tools): CONFIG += uitools
else : DEFINES += QT_NO_UITOOLS

win32|os2 : Debug : CONFIG += console

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

QT += webkit network webkitwidgets printsupport uitools

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

VPATH += $$PWD

FORMS += \
    aboutdialog.ui \
    autofilldialog.ui \
    acceptlanguagedialog.ui \
    downloaditem.ui \
    downloads.ui \
    searchbanner.ui \
    settings.ui

HEADERS += \
    aboutdialog.h \
    acceptlanguagedialog.h \
    autosaver.h \
    autofilldialog.h \
    autofillmanager.h \
    browserapplication.h \
    browsermainwindow.h \
    clearprivatedata.h \
    clearbutton.h \
    downloadmanager.h \
    modelmenu.h \
    modeltoolbar.h \
    plaintexteditsearch.h \
    searchbar.h \
    searchbutton.h \
    searchlineedit.h \
    settings.h \
    sourcehighlighter.h \
    sourceviewer.h \
    tabbar.h \
    tabwidget.h \
    toolbarsearch.h \
    webactionmapper.h \
    webpage.h \
    webview.h \
    webviewsearch.h

SOURCES += \
    aboutdialog.cpp \
    acceptlanguagedialog.cpp \
    autosaver.cpp \
    autofilldialog.cpp \
    autofillmanager.cpp \
    browserapplication.cpp \
    browsermainwindow.cpp \
    clearprivatedata.cpp \
    clearbutton.cpp \
    downloadmanager.cpp \
    modelmenu.cpp \
    modeltoolbar.cpp \
    plaintexteditsearch.cpp \
    searchbar.cpp \
    searchbutton.cpp \
    searchlineedit.cpp \
    settings.cpp \
    sourcehighlighter.cpp \
    sourceviewer.cpp \
    tabbar.cpp \
    tabwidget.cpp \
    toolbarsearch.cpp \
    webactionmapper.cpp \
    webpage.cpp \
    webview.cpp \
    webviewsearch.cpp

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

mac {
    ICON = browser.icns
    QMAKE_INFO_PLIST = $$PWD/Info_mac.plist
}

include(../webkittrunk.pri)

unix {
    PKGDATADIR = $$DATADIR/endorphin
    DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"
}

win32 {
    LIBS += -ladvapi32
}
