lessThan(QT_VERSION, 5.0) {
    error("Arora requires Qt 5.0 or greater")
}

TEMPLATE = subdirs
SUBDIRS  = src tools
CONFIG += ordered

unix {
    warning("It is recommended to use CMake instead of QMake")
    # this is an ugly work around to do .PHONY: doc
    doxygen.target = doc dox
    doxygen.commands = doxygen Doxyfile
    doxygen.depends = Doxyfile
    QMAKE_EXTRA_TARGETS += doxygen
}

