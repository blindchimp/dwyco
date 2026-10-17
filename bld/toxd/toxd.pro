TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt
include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

INCLUDEPATH += $${PWD}/../vc
INCLUDEPATH += $${PWD}/../dwcls
INCLUDEPATH += $${PWD}/../kazlib
INCLUDEPATH += $${PWD}/../jenkins
INCLUDEPATH += $${PWD}/../cdc32


SOURCES += \
    toxd.cpp

macx-* {
INCLUDEPATH += /opt/homebrew/include
D = $${OUT_PWD}/..
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
# note: DWYCO_TOXCORE is what actually turns on the real plugin in toxd.cpp
# (the whole implementation is inside that ifdef). LOCAL_TOXCORE only picks
# the header layout (tox/tox.h vs tox.h).
DEFINES += DWYCO_TOXCORE LOCAL_TOXCORE #TOXD_STANDALONE
INCLUDEPATH += /home/dwight/git/c-toxcore/toxcore
# toxencryptsave.h lives in the sibling dir, not in toxcore/
INCLUDEPATH += /home/dwight/git/c-toxcore/toxencryptsave
D = $${OUT_PWD}/..
# note: qmake ignores LIBS in a staticlib project. this is here to document
# what the app that links this has to link, nothing more.
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
D=$${OUT_PWD}/..
LIBS += \
$${D}/vc/$${S}/vc.lib \
$${D}/dwcls/$${S}/dwcls.lib \
$${D}/kazlib/$${S}/kazlib.lib \
toxcore.lib \
libsodium.lib \
ws2_32.lib
}
