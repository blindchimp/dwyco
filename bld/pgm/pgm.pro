TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

INCLUDEPATH +=  ../pbm
DEFINES += 
macx-g++|linux-g++|linux-g++-64 {
#QMAKE_CXXFLAGS += -fpermissive
#QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder
}

SOURCES = \
	libpgm1.c
