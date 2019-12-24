# note: there is an incompatibility with the system installed
# sqlite development files on the mac, so the wrapper doesn't compile.
# don't use this on the mac for anything much, so just ignoring it for
# now.

TEMPLATE=app
CONFIG -= app_bundle
CONFIG -= qt
include(../$$DWYCO_CONFDIR/conf.pri)

SOURCES = \
../bld/vc/vctrun.cpp \
../bld/vc/cunit.cpp \
../bld/vc/vctfun.cpp

linux-g++* {
SOURCES += \
../bld/vc/hacked_sqlite3.cpp \
../bld/vc/hacked_spread.xml.cpp 
}

macx-* {
SOURCES += \
../bld/vc/hacked_sqlite3.cpp \
../bld/vc/hacked_spread.xml.cpp 
}

INCLUDEPATH += ../$$DWYCO_CONFDIR  ../bld/dwcls ../bld/vc $${OUT_PWD}/../include

linux-g++*: DEFINES += LH_WRAP_SPREAD LH_WRAP_SQLITE3 
macx-*: DEFINES += LH_WRAP_SPREAD LH_WRAP_SQLITE3

D=$${OUT_PWD}/..
LIBS += \
$${D}/bld/vc/libvc.a \
$${D}/bld/dwcls/libdwcls.a \
$${D}/bld/crypto5/libcrypto5.a \
$${D}/bld/jenkins/libjenkins.a \
$${D}/bld/kazlib/libkazlib.a \
$${D}/bld/zlib/libzlib.a \
$${D}/lib/libuv.a



macx-* {
LIBS += \
$${D}/lib/libspread.a

LIBS += -framework Foundation -framework CoreServices -lsqlite3 -lpthread -ldl
}

linux-g++* {
LIBS += \
$${D}/lib/libspread.a

LIBS += -lsqlite3 -lpthread -ldl
}
