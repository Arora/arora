
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

TRANSLATIONS += \
    $$PWD/ast.ts \
    $$PWD/ca.ts \
    $$PWD/cs_CZ.ts \
    $$PWD/da_DK.ts \
    $$PWD/de_DE.ts \
    $$PWD/el_GR.ts \
    $$PWD/es.ts \
    $$PWD/es_CR.ts \
    $$PWD/et_EE.ts \
    $$PWD/fi_FI.ts \
    $$PWD/fr_CA.ts \
    $$PWD/fr_FR.ts \
    $$PWD/gl.ts \
    $$PWD/he_IL.ts \
    $$PWD/hu_HU.ts \
    $$PWD/it_IT.ts \
    $$PWD/ja_JP.ts \
    $$PWD/ms.ts \
    $$PWD/nb_NO.ts \
    $$PWD/nl.ts \
    $$PWD/pl_PL.ts \
    $$PWD/pt_PT.ts \
    $$PWD/pt_BR.ts \
    $$PWD/ru_RU.ts \
    $$PWD/sk_SK.ts \
    $$PWD/sr_RS.ts \
    $$PWD/sr_RS@latin.ts \
    $$PWD/tr_TR.ts \
    $$PWD/uk.ts \
    $$PWD/zh_CN.ts \
    $$PWD/zh_TW.ts

isEmpty(QMAKE_LRELEASE) {
    win32|os2:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
    unix {
        !exists($$QMAKE_LRELEASE) { QMAKE_LRELEASE = lrelease-qt4 }
    } else {
        !exists($$QMAKE_LRELEASE) { QMAKE_LRELEASE = lrelease }
    }
}

updateqm.input = TRANSLATIONS
updateqm.output = .qm/locale/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE -silent ${QMAKE_FILE_IN} -qm .qm/locale/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm
