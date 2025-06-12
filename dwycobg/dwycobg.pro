TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
include(../$$DWYCO_CONFDIR/conf.pri)

#QMAKE_LFLAGS_RELEASE += -static
INCLUDEPATH += $${PWD}/../bld/miscsrc
SOURCES += \
    dwycobg.cpp

linux-* {
D = $${OUT_PWD}/../bld

LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto8/libcrypto8.a \
$${D}/dwcls/libdwcls.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/zlib/libzlib.a \
$${D}/jenkins/libjenkins.a \
$${D}/uv/libuv.a \
-lpthread \
-ldl

PRE_TARGETDEPS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto8/libcrypto8.a \
$${D}/dwcls/libdwcls.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/zlib/libzlib.a \
$${D}/jenkins/libjenkins.a

}

macx-* {
D = $${OUT_PWD}/../bld
LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto8/libcrypto8.a \
$${D}/dwcls/libdwcls.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/zlib/libzlib.a \
$${D}/jenkins/libjenkins.a \
-lpthread \
-ldl
}

win32* {
DEFINES += CDCCORE_STATIC
#D = $$replace(OUT_PWD, /, \\)\\..\\bld
D=$${OUT_PWD}/../bld
CONFIG(debug, debug|release) {
    S=debug
}
CONFIG(release, debug|release) {
    S=release
}

#S = release

LIBS += \
$${D}/cdc32/$${S}/cdc32.lib \
$${D}/vc/$${S}/vc.lib \
$${D}/crypto8/$${S}/crypto8.lib \
$${D}/dwcls/$${S}/dwcls.lib \
$${D}/kazlib/$${S}/kazlib.lib \
$${D}/ppm/$${S}/ppm.lib \
$${D}/pgm/$${S}/pgm.lib \
$${D}/pbm/$${S}/pbm.lib \
$${D}/zlib/$${S}/zlib.lib \
$${D}/jenkins/$${S}/jenkins.lib \
user32.lib kernel32.lib Ws2_32.lib winmm.lib vfw32.lib advapi32.lib binmode.obj

DEFINES += USE_VFW  MINGW_CLIENT VCCFG_FILE _CRT_SECURE_NO_WARNINGS __WIN32__ _Windows WIN32

# use this for linking to dynamic cdcdll
#LIBS +=  $${PWD}/cdcdll8.lib winmm.lib user32.lib kernel32.lib

}
