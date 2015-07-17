INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

FORMS += \
    $$PWD/passworddialog.ui \
    $$PWD/proxy.ui

HEADERS += \
    $$PWD/fileaccesshandler.h \
    $$PWD/networkaccessmanager.h \
    $$PWD/networkdiskcache.h \
    $$PWD/networkproxyfactory.h \
    $$PWD/schemeaccesshandler.h

SOURCES += \
    $$PWD/fileaccesshandler.cpp \
    $$PWD/networkaccessmanager.cpp \
    $$PWD/networkdiskcache.cpp \
    $$PWD/networkproxyfactory.cpp \
    $$PWD/schemeaccesshandler.cpp

include(cookiejar/cookiejar.pri)
