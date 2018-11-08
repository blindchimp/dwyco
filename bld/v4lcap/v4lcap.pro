# note: this component is only built for LINUX
TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

linux-g++-*:QMAKE_CXXFLAGS += -fpermissive -Wno-unused-parameter
INCLUDEPATH += ../dwcls ../cdc32 ../cdc32/winemu ../speex/include


# note: this has to be the same as in cdc32.pro until we can
# figure out a good way to set this properly
DEFINES += UWB_SAMPLING UWB_SAMPLE_RATE=44100

SOURCES = \
v4l2cap.cpp

equals(DWYCO_USE_LINUX_AUDIO, 1) {
SOURCES += aextsdl.cc \
esdaqaud.cc \
esdaudin.cpp
}

