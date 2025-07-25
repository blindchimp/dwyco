#-------------------------------------------------
#
# Project created by QtCreator 2014-02-18T13:22:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qdwyrun
TEMPLATE = app


SOURCES += main.cpp\
        qdwyrun.cpp

HEADERS  += qdwyrun.h

FORMS    += qdwyrun.ui

include(../$$DWYCO_CONFDIR/conf.pri)

linux-* {
QMAKE_CXXFLAGS += -fpermissive
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder
}

macx-* {
#QMAKE_CXXFLAGS_X86_64 += -mmacosx-version-min=10.9
#QMAKE_LFLAGS += -mmacosx-version-min=10.9
}

#win32:QMAKE_CXXFLAGS += /MT
#win32:QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd.LIB

D=$${PWD}/../bld
L=$${OUT_PWD}/../bld

INCLUDEPATH += $$D/dwcls $$D/vc $$D/crypto8

linux-*|macx-* {
LIBS += \
$$L/vc/libvc.a \
$$L/dwcls/libdwcls.a \
$$L/jenkins/libjenkins.a \
$$L/kazlib/libkazlib.a \
$$L/crypto8/libcrypto8.a \
$$L/zlib/libzlib.a

#$$L/uv/libuv.a
}

win32* {
DEFINES += CDCCORE_STATIC
DEFINES += VCCFG_FILE _CRT_SECURE_NO_WARNINGS __WIN32__ _Windows WIN32 
#D = $$replace(OUT_PWD, /, \\)\\..\\bld
#S = debug
D=$${OUT_PWD}/../bld
CONFIG(debug, debug|release) {
    S=debug
}
CONFIG(release, debug|release) {
    S=release
}

LIBS += \
$${D}/vc/$${S}/vc.lib \
$${D}/crypto8/$${S}/crypto8.lib \
$${D}/dwcls/$${S}/dwcls.lib \
$${D}/kazlib/$${S}/kazlib.lib \
$${D}/jenkins/$${S}/jenkins.lib \
$${D}/zlib/$${S}/zlib.lib \
user32.lib kernel32.lib Ws2_32.lib advapi32.lib Shell32.lib
}

RESOURCES += \
    icons.qrc
