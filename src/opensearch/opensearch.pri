INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/opensearchdialog.h \
    $$PWD/opensearchengine.h \
    $$PWD/opensearchenginedelegate.h \
    $$PWD/opensearchengineaction.h \
    $$PWD/opensearchenginemodel.h \
    $$PWD/opensearchmanager.h \
    $$PWD/opensearchreader.h \
    $$PWD/opensearchwriter.h

SOURCES += \
    $$PWD/opensearchdialog.cpp \
    $$PWD/opensearchengine.cpp \
    $$PWD/opensearchenginedelegate.cpp \
    $$PWD/opensearchengineaction.cpp \
    $$PWD/opensearchenginemodel.cpp \
    $$PWD/opensearchmanager.cpp \
    $$PWD/opensearchreader.cpp \
    $$PWD/opensearchwriter.cpp

FORMS += \
    $$PWD/opensearchdialog.ui

QT += script
