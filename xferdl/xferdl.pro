TEMPLATE = app
#CONFIG += console
win32: CONFIG += windows
CONFIG -= app_bundle
CONFIG -= qt
include(../$$DWYCO_CONFDIR/conf.pri)

SOURCES += \
    xferdl.cpp

macx-* {
QMAKE_CXXFLAGS_X86_64 += -mmacosx-version-min=10.9
QMAKE_LFLAGS += -mmacosx-version-min=10.9
}

D=$${PWD}/../bld
L=$${OUT_PWD}/../bld

INCLUDEPATH += $$D/dwcls $$D/vc $$D/crypto5

linux-*|macx-* {
LIBS += \
$$L/vc/libvc.a \
$$L/dwcls/libdwcls.a \
$$L/jenkins/libjenkins.a \
$$L/kazlib/libkazlib.a \
$$L/crypto5/libcrypto5.a

#$$L/uv/libuv.a \
#-pthread
}

win32* {
DEFINES += CDCCORE_STATIC
DEFINES += VCCFG_FILE _CRT_SECURE_NO_WARNINGS __WIN32__ _Windows WIN32 
#D = $$replace(OUT_PWD, /, \\)\\..\\bld
#S = release
D=$${OUT_PWD}/../bld
CONFIG(debug, debug|release) {
    S=debug
}
CONFIG(release, debug|release) {
    S=release
}

LIBS += \
$${D}/vc/$${S}/vc.lib \
$${D}/crypto5/$${S}/crypto5.lib \
$${D}/dwcls/$${S}/dwcls.lib \
$${D}/kazlib/$${S}/kazlib.lib \
$${D}/jenkins/$${S}/jenkins.lib \
user32.lib kernel32.lib ws2_32.lib wsock32.lib advapi32.lib Shell32.lib
}
