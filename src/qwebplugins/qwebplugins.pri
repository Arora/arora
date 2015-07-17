INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
  $$PWD/arorawebplugin.h \
  $$PWD/webpluginfactory.h

SOURCES += \
  $$PWD/arorawebplugin.cpp \
  $$PWD/webpluginfactory.cpp

include(clicktoflash/clicktoflash.pri)

