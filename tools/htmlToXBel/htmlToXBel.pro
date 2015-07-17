TEMPLATE = app
TARGET = htmlToXBel
DEPENDPATH += .
INCLUDEPATH += .

win32|os2: CONFIG += console
mac:CONFIG -= app_bundle

contains(QT_VERSION, ^5.*) {
    QT += network widgets webkitwidgets printsupport
} else {
    QT += network webkit
}

# Input
SOURCES += main.cpp

RCC_DIR     = $$PWD/.rcc
UI_DIR      = $$PWD/.ui
MOC_DIR     = $$PWD/.moc
OBJECTS_DIR = $$PWD/.obj

RESOURCES += source.qrc

include(../../install.pri)
include(../../webkittrunk.pri)

!mac {
unix {
    INSTALLS += man man-compress

    man.path = $$DATADIR/man/man1
    man.files += data/htmlToXBel.1

    man-compress.path = $$DATADIR/man/man1
    man-compress.extra = "" "gzip -9 -f \$(INSTALL_ROOT)/$$DATADIR/man/man1/htmlToXBel.1" ""
    man-compress.depends = install_man
}
}
