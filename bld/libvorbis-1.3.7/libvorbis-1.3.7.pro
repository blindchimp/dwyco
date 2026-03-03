TARGET = vorbis
TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

INCLUDEPATH += include lib ../ogg/include

SOURCES += \
    lib/window.c \
    lib/vorbisenc.c \
    lib/synthesis.c \
    lib/smallft.c \
    lib/sharedbook.c \
    lib/res0.c \
    lib/registry.c \
    lib/psy.c \
    lib/mdct.c \
    lib/mapping0.c \
    lib/lsp.c \
    lib/lpc.c \
    lib/lookup.c \
    lib/info.c \
    lib/floor1.c \
    lib/floor0.c \
    lib/envelope.c \
    lib/codebook.c \
    lib/block.c \
    lib/bitrate.c \
    lib/analysis.c

#lib/vorbisfile.c

win32: SOURCES += radxe/alloca.c
win32: QMAKE_CFLAGS += /wd4244 /wd4305

HEADERS += \
    include/vorbis/vorbisfile.h \
    include/vorbis/vorbisenc.h \
    include/vorbis/codec.h
