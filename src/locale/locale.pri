
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

TRANSLATIONS += \
    ast.ts \
    ca.ts \
    cs_CZ.ts \
    da_DK.ts \
    de_DE.ts \
    el_GR.ts \
    es.ts \
    es_CR.ts \
    et_EE.ts \
    fi_FI.ts \
    fr_CA.ts \
    fr_FR.ts \
    gl.ts \
    he_IL.ts \
    hu_HU.ts \
    it_IT.ts \
    ja_JP.ts \
    ms.ts \
    nb_NO.ts \
    nl.ts \
    pl_PL.ts \
    pt_PT.ts \
    pt_BR.ts \
    ru_RU.ts \
    sk_SK.ts \
    sr_RS.ts \
    sr_RS@latin.ts \
    tr_TR.ts \
    uk.ts \
    zh_CN.ts \
    zh_TW.ts

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
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
