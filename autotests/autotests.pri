win32: CONFIG += console
mac:CONFIG -= app_bundle

CONFIG += qtestlib

include($$PWD/../src/src.pri)
include($$PWD/modeltest/modeltest.pri)

HEADERS += qtest_endorphin.h

DEFINES += AUTOTESTS

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

RCC_DIR     = $$PWD/.rcc
UI_DIR      = $$PWD/.ui
MOC_DIR     = $$PWD/.moc
OBJECTS_DIR = $$PWD/.obj


