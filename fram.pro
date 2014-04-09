lessThan(QT_VERSION, 5.0) {
    error("Fram requires Qt 5.0 or greater")
}

TEMPLATE = subdirs
SUBDIRS  = src


CONFIG += ordered
