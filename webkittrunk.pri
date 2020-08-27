# To build Endorphin against WebKit trunk:
#   export QT_WEBKIT=webkit_trunk
#   export WEBKITDIR=$HOME/dev/webkit
#
# Optional:
#   export WEBKITOUTPUTDIR=$HOME/build/webkit
#   export WEBKITBRANCH=some_cool_git_branch_of_webkit
#
CONFIG += $$(QT_WEBKIT)
webkit_trunk {
    WEBKITDIR = $$(WEBKITDIR)
    isEmpty(WEBKITDIR): error(Please point WEBKITDIR at the root of your WebKit source tree)

    OUTPUT_DIR = $$(WEBKITOUTPUTDIR)
    isEmpty(OUTPUT_DIR): OUTPUT_DIR = $$WEBKITDIR/WebKitBuild

    WEBKITBRANCH = $$(WEBKITBRANCH)
    !isEmpty(WEBKITBRANCH) {
        OUTPUT_DIR = $$OUTPUT_DIR/$$WEBKITBRANCH
    }

    # When building in debug try to link to the debug version of webkit
    # and vice versa in release, but when they can not be paired up
    # because webkit only has release or debug libs fall back to the other one
    CONFIG(debug) {
        exists($$OUTPUT_DIR/Debug) {
            OUTPUT_DIR = $$OUTPUT_DIR/Debug
        } else {
            OUTPUT_DIR = $$OUTPUT_DIR/Release
        }
    } else:CONFIG(release) {
        exists($$OUTPUT_DIR/Release) {
            OUTPUT_DIR = $$OUTPUT_DIR/Release
        } else {
            OUTPUT_DIR = $$OUTPUT_DIR/Debug
        }
    }

    message(Using WebKit Trunk at $$WEBKITDIR)
    message(Using WebKit Build at $$OUTPUT_DIR)

    QT -= webkit
    DEFINES += WEBKIT_TRUNK
    include($$WEBKITDIR/WebKit.pri)
    QMAKE_RPATHDIR = $$OUTPUT_DIR/lib $$QMAKE_RPATHDIR
}

