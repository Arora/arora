TEMPLATE = app
TARGET = endorphin-placesimport
DEPENDPATH += .
INCLUDEPATH += .

win32|os2: CONFIG += console
mac:CONFIG -= app_bundle

QT += sql

# Input
SOURCES += main_placesimport.cpp

include(../../install.pri)
# This can be improved to just include the history manager and utility classes
# once the history classes are separated out into individual files.
include(../../src/src.pri)

!mac {
unix {
    INSTALLS += man man-compress

    man.path = $$DATADIR/man/man1
    man.files += data/endorphin-placesimport.1

    man-compress.path = $$DATADIR/man/man1
    man-compress.extra = "" "gzip -9 -f \$(INSTALL_ROOT)/$$DATADIR/man/man1/endorphin-placesimport.1" ""
    man-compress.depends = install_man
}
}
