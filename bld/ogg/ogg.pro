
TARGET = ogg
TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

INCLUDEPATH += include

SOURCES += \
src/framing.c \
src/bitwise.c

HEADERS += \
include/ogg/os_types.h \
include/ogg/ogg.h
