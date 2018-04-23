QT -= core gui
include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)
#DESTDIR = $$PWD/../

TARGET = ogg
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += include

SOURCES += \
src/framing.c \
src/bitwise.c

HEADERS += \
include/ogg/os_types.h \
include/ogg/ogg.h
