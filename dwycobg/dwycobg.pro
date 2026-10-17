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
# toxd and cdc32 are built with the toxcore headers on linux, so this has
# to link the toxcore library too. see the contains(DWYCO_TOXCORE) block at
# the bottom -- the libs go there, after all the .a's, because static
# archives only satisfy references that come later on the command line.
DEFINES += DWYCO_TOXCORE

LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/toxd/libtoxd.a \
$${D}/vc/libvc.a \
$${D}/crypto8/libcrypto8.a \
$${D}/dwcls/libdwcls.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/zlib/libzlib.a \
$${D}/jenkins/libjenkins.a \
$${D}/uv/libuv.a \
-lpthread \
-ldl

PRE_TARGETDEPS += \
$${D}/cdc32/libcdc32.a \
$${D}/toxd/libtoxd.a \
$${D}/vc/libvc.a \
$${D}/crypto8/libcrypto8.a \
$${D}/dwcls/libdwcls.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/zlib/libzlib.a \
$${D}/jenkins/libjenkins.a \
$${D}/uv/libuv.a

}

macx-* {
D = $${OUT_PWD}/../bld
LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/toxd/libtoxd.a \
$${D}/vc/libvc.a \
$${D}/crypto8/libcrypto8.a \
$${D}/dwcls/libdwcls.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
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
$${D}/toxd/$${S}/toxd.lib \
$${D}/vc/$${S}/vc.lib \
$${D}/crypto8/$${S}/crypto8.lib \
$${D}/dwcls/$${S}/dwcls.lib \
$${D}/kazlib/$${S}/kazlib.lib \
$${D}/ppm/$${S}/ppm.lib \
$${D}/pgm/$${S}/pgm.lib \
$${D}/zlib/$${S}/zlib.lib \
$${D}/jenkins/$${S}/jenkins.lib \
user32.lib kernel32.lib Ws2_32.lib winmm.lib vfw32.lib advapi32.lib binmode.obj

DEFINES += USE_VFW  MINGW_CLIENT VCCFG_FILE _CRT_SECURE_NO_WARNINGS __WIN32__ _Windows WIN32

# use this for linking to dynamic cdcdll
#LIBS +=  $${PWD}/cdcdll8.lib winmm.lib user32.lib kernel32.lib

}

contains(DEFINES, DWYCO_TOXCORE) {
    linux-* {
        LIBS += /home/dwight/git/c-toxcore/build/libtoxcore.a -lsodium
    }
    macx-* {
        LIBS += /opt/homebrew/lib/libtoxcore.a /opt/homebrew/lib/libsodium.a
    }
    win32* {
        LIBS += toxcore.lib libsodium.lib ws2_32.lib
    }
}
