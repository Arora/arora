INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/editlistview.h \
    $$PWD/edittableview.h \
    $$PWD/edittreeview.h \
    $$PWD/languagemanager.h \
    $$PWD/lineedit.h \
    $$PWD/lineedit_p.h \
    $$PWD/networkaccessmanagerproxy.h \
    $$PWD/networkaccessmanagerproxy_p.h \
    $$PWD/singleapplication.h \
    $$PWD/squeezelabel.h \
    $$PWD/treesortfilterproxymodel.h \
    $$PWD/webpageproxy.h

SOURCES += \
    $$PWD/editlistview.cpp \
    $$PWD/edittableview.cpp \
    $$PWD/edittreeview.cpp \
    $$PWD/languagemanager.cpp \
    $$PWD/lineedit.cpp \
    $$PWD/networkaccessmanagerproxy.cpp \
    $$PWD/singleapplication.cpp \
    $$PWD/squeezelabel.cpp \
    $$PWD/treesortfilterproxymodel.cpp \
    $$PWD/webpageproxy.cpp

win32 {
    HEADERS += \
        $$PWD/explorerstyle.h
    SOURCES += \
        $$PWD/explorerstyle.cpp
    LIBS += -lgdi32
}

