TEMPLATE = app
TARGET = arora-cacheinfo
DEPENDPATH += .
INCLUDEPATH += .

win32: CONFIG += console
mac:CONFIG -= app_bundle

QT += network

# Input
SOURCES += main.cpp

RCC_DIR     = $$PWD/.rcc
UI_DIR      = $$PWD/.ui
MOC_DIR     = $$PWD/.moc
OBJECTS_DIR = $$PWD/.obj

include(../../install.pri)

!mac {
unix {
    INSTALLS += man man-compress

    man.path = $$DATADIR/man/man1
    man.files += data/arora-cacheinfo.1

    man-compress.path = $$DATADIR/man/man1
    man-compress.extra = "" "gzip -9 -f \$(INSTALL_ROOT)/$$DATADIR/man/man1/arora-cacheinfo.1" ""
    man-compress.depends = install_man
}
}
