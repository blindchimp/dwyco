TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

INCLUDEPATH +=  ../pbm ../pgm

SOURCES = \
	libppm1.c \
    libppm2.c
