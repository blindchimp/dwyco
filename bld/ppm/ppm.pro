TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

INCLUDEPATH += ../pgm

SOURCES = \
        ppm.cpp
