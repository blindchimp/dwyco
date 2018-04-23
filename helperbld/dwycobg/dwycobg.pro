TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
include(../conf.pro)

#QMAKE_LFLAGS_RELEASE += -static

SOURCES += \
    dwycobg.cpp

linux-* {
D = $${OUT_PWD}/../bld

LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto5/libcrypto5.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/zlib/libzlib.a \
$${D}/theora/libtheora.a \
$${D}/speex/libspeex.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
-lsqlite3 \
-lpthread \
-ldl
}

macx-g++|macx-clang {
D = $${OUT_PWD}/../bld
QMAKE_LFLAGS += -mmacosx-version-min=10.9
QMAKE_LFLAGS_X86_64 += -mmacosx-version-min=10.9
QMAKE_LFLAGS -= -mmacosx-version-min=10.5
LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto5/libcrypto5.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/zlib/libzlib.a \
$${D}/theora/libtheora.a \
$${D}/speex/libspeex.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
-lpthread \
-ldl
}

win32* {
DEFINES += CDCCORE_STATIC
D = $$replace(OUT_PWD, /, \\)\\..\\bld
S = release

LIBS += \
$${D}\\cdc32\\$${S}\\cdc32.lib \
$${D}\\vc\\$${S}\\vc.lib \
$${D}\\crypto5\\$${S}\\crypto5.lib \
$${D}\\dwcls\\$${S}\\dwcls.lib \
$${D}\\gsm\\$${S}\\gsm.lib \
$${D}\\kazlib\\$${S}\\kazlib.lib \
$${D}\\ppm\\$${S}\\ppm.lib \
$${D}\\pgm\\$${S}\\pgm.lib \
$${D}\\pbm\\$${S}\\pbm.lib \
$${D}\\zlib\\$${S}\\zlib.lib \
$${D}\\theora\\$${S}\\theora.lib \
$${D}\\speex\\$${S}\\speex.lib \
$${D}\\vorbis112\\$${S}\\vorbis.lib \
$${D}\\ogg\\$${S}\\ogg.lib \
$${D}\\jenkins\\$${S}\\jenkins.lib \
user32.lib kernel32.lib wsock32.lib winmm.lib vfw32.lib advapi32.lib binmode.obj

DEFINES += USE_VFW  MINGW_CLIENT VCCFG_FILE _CRT_SECURE_NO_WARNINGS __WIN32__ _Windows WIN32

# use this for linking to dynamic cdcdll
#LIBS +=  $${PWD}/cdcdll8.lib winmm.lib user32.lib kernel32.lib

}
