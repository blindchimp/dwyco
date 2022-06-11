TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

SOURCES = \
dwhash.cpp \
useful.cpp \
dwstr.cpp \
dwtree2.cpp \
dwvp.cpp \
dwdate.cpp \
dwhist2.cc \
uricodec.cpp \
dwcls_timer.cpp

INCLUDEPATH = ../kazlib


