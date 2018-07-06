TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

INCLUDEPATH += 
DEFINES += 
macx-g++|linux-g++|linux-g++-64 {
#QMAKE_CXXFLAGS += -fpermissive
#QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder
}

SOURCES = \
	libpbm1.c \
	libpbm2.c \
    libpbm4.c
