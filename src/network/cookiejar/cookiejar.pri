INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
  $$PWD/cookiedialog.h \
  $$PWD/cookieexceptionsdialog.h \
  $$PWD/cookieexceptionsmodel.h \
  $$PWD/cookiejar.h \
  $$PWD/cookiemodel.h

SOURCES += \
  $$PWD/cookiedialog.cpp \
  $$PWD/cookieexceptionsmodel.cpp \
  $$PWD/cookiemodel.cpp \
  $$PWD/cookieexceptionsdialog.cpp \
  $$PWD/cookiejar.cpp

FORMS += \
    $$PWD/cookies.ui \
    $$PWD/cookiesexceptions.ui

include($$PWD/networkcookiejar/networkcookiejar.pri)

