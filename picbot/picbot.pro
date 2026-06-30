QT += core
QT -= gui

include(../$$DWYCO_CONFDIR/conf.pri)
INCLUDEPATH += ../bld/dwcls ../bld/miscsrc
SOURCES += \
    picbot.cpp \
    ../bld/miscsrc/dwyco_new_msg.cpp

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
$${D}/zlib/libzlib.a \
$${D}/jenkins/libjenkins.a \
-ldl


QMAKE_CXX=ccache g++
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder
DEFINES += LINUX VCCFG_FILE

#QMAKE_CXXFLAGS += -fsanitize=address
#QMAKE_LFLAGS += -fsanitize=address
#QMAKE_LFLAGS += --coverage
}

