TEMPLATE = app
CONFIG += console
CONFIG -= qt
include(../$$DWYCO_CONFDIR/conf.pri)

SOURCES += \
    xfer.cc

D=$${PWD}/../bld
L=$${OUT_PWD}/../bld

INCLUDEPATH += /home/dwight/syncdev/keys $$D/dwcls $$D/vc

linux-* {
LIBS += \
$$L/vc/libvc.a \
$$L/dwcls/libdwcls.a \
$$L/jenkins/libjenkins.a \
$$L/kazlib/libkazlib.a \
$$L/crypto5/libcrypto5.a
QMAKE_LFLAGS += #-static #-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder -Wno-unused-variable
#QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
}

