TEMPLATE = app

TARGET = endorphin
mac {
    TARGET = Endorphin
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
}

DEFINES += \
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
    desktop.files += endorphin.desktop

    iconxpm.path = $$DATADIR/pixmaps
    iconxpm.files += data/endorphin.xpm

    iconsvg.path = $$DATADIR/icons/hicolor/scalable/apps
    iconsvg.files += data/endorphin.svg

    icon16.path = $$DATADIR/icons/hicolor/16x16/apps
    icon16.files += data/16x16/endorphin.png

    icon32.path = $$DATADIR/icons/hicolor/32x32/apps
    icon32.files += data/32x32/endorphin.png

    icon128.path = $$DATADIR/icons/hicolor/128x128/apps
    icon128.files += data/128x128/endorphin.png

    man.path = $$DATADIR/man/man1
    man.files += data/endorphin.1

    man-compress.path = $$DATADIR/man/man1
    man-compress.extra = "" "gzip -9 -f \$(INSTALL_ROOT)/$$DATADIR/man/man1/endorphin.1" ""
    man-compress.depends = install_man

    GNOME_DEFAULT_APPS_PATH = $$system(pkg-config --variable=defappsdir gnome-default-applications)

    !isEmpty(GNOME_DEFAULT_APPS_PATH) {
        INSTALLS += gnome-default-app

        gnome-default-app.path = $$GNOME_DEFAULT_APPS_PATH
        gnome-default-app.files = data/endorphin.xml
    }
}
}
