TEMPLATE = app

TARGET = fram
mac {
    TARGET = Fram
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
}

DEFINES += \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII \
    QT_STRICT_ITERATORS \

include(../install.pri)

include(src.pri)

SOURCES += main.cpp

DESTDIR = ../

include(locale/locale.pri)

!mac {
unix {
    INSTALLS += translations desktop iconxpm iconsvg icon16 icon32 icon128 man man-compress

    translations.path = $$PKGDATADIR
    translations.files += .qm/locale

    desktop.path = $$DATADIR/applications
    desktop.files += arora.desktop

    iconxpm.path = $$DATADIR/pixmaps
    iconxpm.files += data/arora.xpm

    iconsvg.path = $$DATADIR/icons/hicolor/scalable/apps
    iconsvg.files += data/arora.svg

    icon16.path = $$DATADIR/icons/hicolor/16x16/apps
    icon16.files += data/16x16/arora.png

    icon32.path = $$DATADIR/icons/hicolor/32x32/apps
    icon32.files += data/32x32/arora.png

    icon128.path = $$DATADIR/icons/hicolor/128x128/apps
    icon128.files += data/128x128/arora.png

    man.path = $$DATADIR/man/man1
    man.files += data/arora.1

    man-compress.path = $$DATADIR/man/man1
    man-compress.extra = "" "gzip -9 -f \$(INSTALL_ROOT)/$$DATADIR/man/man1/arora.1" ""
    man-compress.depends = install_man

    GNOME_DEFAULT_APPS_PATH = $$system(pkg-config --variable=defappsdir gnome-default-applications)

    !isEmpty(GNOME_DEFAULT_APPS_PATH) {
        INSTALLS += gnome-default-app

        gnome-default-app.path = $$GNOME_DEFAULT_APPS_PATH
        gnome-default-app.files = data/arora.xml
    }
}
}
