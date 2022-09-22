QT       -= core gui

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

TARGET = theora
TEMPLATE = lib
CONFIG += staticlib
QMAKE_CFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-shift-op-parentheses

INCLUDEPATH += include ../ogg/include ../vorbis/include

SOURCES += \
    lib/tokenize.c \
    lib/state.c \
    lib/rate.c \
    lib/quant.c \
    lib/mcenc.c \
    lib/mathops.c \
    lib/internal.c \
    lib/info.c \
    lib/idct.c \
    lib/huffenc.c \
    lib/huffdec.c \
    lib/fragment.c \
    lib/fdct.c \
    lib/enquant.c \
    lib/encode.c \
    lib/encinfo.c \
    lib/encfrag.c \
    lib/dequant.c \
    lib/decode.c \
    lib/decinfo.c \
    lib/bitpack.c \
    lib/apiwrapper.c \
    lib/analyze.c \
    lib/encapiwrapper.c \
    lib/decapiwrapper.c
HEADERS +=
