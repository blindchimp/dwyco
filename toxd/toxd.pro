TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
include(../$$DWYCO_CONFDIR/conf.pri)

INCLUDEPATH += $${PWD}/../bld/vc
INCLUDEPATH += $${PWD}/../bld/dwcls
INCLUDEPATH += $${PWD}/../bld/kazlib


SOURCES += \
    toxd.cpp

macx-* {
INCLUDEPATH += /opt/homebrew/include
#QMAKE_LFLAGS += /opt/homebrew/lib
D = $${OUT_PWD}/../bld
LIBS += \
$${D}/vc/libvc.a \
$${D}/dwcls/libdwcls.a \
$${D}/kazlib/libkazlib.a \
$${D}/jenkins/libjenkins.a \
/opt/homebrew/lib/libtoxcore.a \
/opt/homebrew/lib/libsodium.a \
-lpthread
}

linux-* {
DEFINES += LOCAL_TOXCORE #TOXD_STANDALONE
INCLUDEPATH += /home/dwight/git/c-toxcore/toxcore
D = $${OUT_PWD}/../bld
LIBS += \
$${D}/vc/libvc.a \
$${D}/dwcls/libdwcls.a \
$${D}/kazlib/libkazlib.a \
$${D}/jenkins/libjenkins.a \
/home/dwight/git/c-toxcore/build/libtoxcore.a \
-lsodium \
-lpthread \
-ldl
}

win32* {
D=$${OUT_PWD}/../bld
LIBS += \
$${D}/vc/$${S}/vc.lib \
$${D}/dwcls/$${S}/dwcls.lib \
$${D}/kazlib/$${S}/kazlib.lib \
toxcore.lib \
libsodium.lib \
ws2_32.lib
}
