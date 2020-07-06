TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)


SOURCES = \
	libpbm1.c \
	libpbm2.c \
    libpbm4.c
