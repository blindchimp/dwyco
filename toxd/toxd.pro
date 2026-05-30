TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
include(../$$DWYCO_CONFDIR/conf.pri)

INCLUDEPATH += $${PWD}/../bld/vc
INCLUDEPATH += $${PWD}/../bld/dwcls
INCLUDEPATH += $${PWD}/../bld/kazlib
INCLUDEPATH += /opt/homebrew/include

SOURCES += \
    toxd.cpp

macx-* {
D = $${OUT_PWD}/../bld
LIBS += \
$${D}/vc/libvc.a \
$${D}/dwcls/libdwcls.a \
$${D}/kazlib/libkazlib.a \
-ltoxcore \
-lsodium \
-lpthread
}

linux-* {
D = $${OUT_PWD}/../bld
LIBS += \
$${D}/vc/libvc.a \
$${D}/dwcls/libdwcls.a \
$${D}/kazlib/libkazlib.a \
-ltoxcore \
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
