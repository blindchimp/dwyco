# note: there is an incompatibility with the system installed
# sqlite development files on the mac, so the wrapper doesn't compile.
# don't use this on the mac for anything much, so just ignoring it for
# now.

TEMPLATE=app
CONFIG -= app_bundle
CONFIG -= qt
include(../$$DWYCO_CONFDIR/conf.pri)

SOURCES = \
    fuzz.cpp

INCLUDEPATH += ../$$DWYCO_CONFDIR  ../bld/dwcls ../bld/vc $${OUT_PWD}/../include

D=$${OUT_PWD}/..

LIBS += \
$${D}/bld/vc/libvc.a \
$${D}/bld/dwcls/libdwcls.a \
$${D}/bld/crypto5/libcrypto5.a \
$${D}/bld/jenkins/libjenkins.a \
$${D}/bld/kazlib/libkazlib.a \
$${D}/bld/zlib/libzlib.a \
$${D}/bld/uv/libuv.a

PRE_TARGETDEPS += \
$${D}/bld/vc/libvc.a \
$${D}/bld/dwcls/libdwcls.a \
$${D}/bld/crypto5/libcrypto5.a \
$${D}/bld/jenkins/libjenkins.a \
$${D}/bld/kazlib/libkazlib.a \
$${D}/bld/zlib/libzlib.a \
$${D}/bld/uv/libuv.a

linux-* {
#DEFINES += DWYCO_USE_STATIC_SQLITE
#DEFINES += LH_WRAP_SPREAD LH_WRAP_SQLITE3
SOURCES +=

#LIBS += \
#$${D}/lib/libspread.a

LIBS += -lpthread -ldl
}

macx-* {
DEFINES +=  LH_WRAP_SQLITE3
SOURCES +=
LIBS += -framework Foundation -framework CoreServices -lsqlite3 -lpthread -ldl
}

