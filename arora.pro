lessThan(QT_VERSION, 5.0) {
    error("Arora requires Qt 5.0 or greater")
}

TEMPLATE = subdirs
SUBDIRS  = src tools
CONFIG += ordered

unix {
    # this is an ugly work around to do .PHONY: doc
    doxygen.target = doc dox
    doxygen.commands = doxygen Doxyfile
    doxygen.depends = Doxyfile
    QMAKE_EXTRA_TARGETS += doxygen
}

