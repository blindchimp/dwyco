QT = core
#QT -= gui
CONFIG -= app_bundle

include(../$$DWYCO_CONFDIR/conf.pri)

SOURCES += \
    randobot.cpp \
    ../bld/miscsrc/dwyco_new_msg.cpp

DEFINES += DWYCO_THROW DWYCO_USE_STATIC_SQLITE
INCLUDEPATH += $${PWD}/../bld/dwcls $${PWD}/../bld/vc $${PWD}/../bld/cdc32 ../bld/miscsrc $${VCCFG_COMP}

linux-*|macx-* {

D = $${OUT_PWD}/../bld

LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto8/libcrypto8.a \
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
-ldl

PRE_TARGETDEPS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto8/libcrypto8.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/zlib/libzlib.a \
$${D}/theora/libtheora.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
$${D}/speex/libspeex.a


QMAKE_CXX=ccache g++
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder
DEFINES += LINUX VCCFG_FILE
#QMAKE_CXXFLAGS += -fsanitize=address
#QMAKE_LFLAGS += -fsanitize=address
#QMAKE_LFLAGS += --coverage
}

HEADERS += \
    dwyco_new_msg.h
