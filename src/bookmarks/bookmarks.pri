INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/addbookmarkdialog.h \
    $$PWD/bookmarksdialog.h \
    $$PWD/bookmarksmanager.h \
    $$PWD/bookmarksmenu.h \
    $$PWD/bookmarksmodel.h \
    $$PWD/bookmarkstoolbar.h \
    $$PWD/bookmarknode.h

SOURCES += \
    $$PWD/addbookmarkdialog.cpp \
    $$PWD/bookmarksdialog.cpp \
    $$PWD/bookmarksmanager.cpp \
    $$PWD/bookmarksmenu.cpp \
    $$PWD/bookmarksmodel.cpp \
    $$PWD/bookmarkstoolbar.cpp \
    $$PWD/bookmarknode.cpp

FORMS += \
    $$PWD/addbookmarkdialog.ui \
    $$PWD/bookmarksdialog.ui

include(xbel/xbel.pri)
