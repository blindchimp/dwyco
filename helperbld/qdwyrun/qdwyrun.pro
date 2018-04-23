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

include(../conf.pro)

linux-* {
QMAKE_CXXFLAGS += -fpermissive
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder
}

macx-* {
QMAKE_CXXFLAGS_X86_64 += -mmacosx-version-min=10.9
QMAKE_LFLAGS += -mmacosx-version-min=10.9
}

#win32:QMAKE_CXXFLAGS += /MT
#win32:QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd.LIB

D=$${PWD}/../bld
L=$${OUT_PWD}/../bld

INCLUDEPATH += $$D/dwcls $$D/vc $$D/crypto5

linux-*|macx-* {
LIBS += \
$$L/vc/libvc.a \
$$L/dwcls/libdwcls.a \
$$L/jenkins/libjenkins.a \
$$L/kazlib/libkazlib.a \
$$L/crypto5/libcrypto5.a \
$$L/zlib/libzlib.a
}

win32* {
DEFINES += CDCCORE_STATIC
DEFINES += VCCFG_FILE _CRT_SECURE_NO_WARNINGS __WIN32__ _Windows WIN32 
D = $$replace(OUT_PWD, /, \\)\\..\\bld
S = release

LIBS += \
$${D}\\vc\\$${S}\\vc.lib \
$${D}\\crypto5\\$${S}\\crypto5.lib \
$${D}\\dwcls\\$${S}\\dwcls.lib \
$${D}\\kazlib\\$${S}\\kazlib.lib \
$${D}\\jenkins\\$${S}\\jenkins.lib \
user32.lib kernel32.lib wsock32.lib advapi32.lib Shell32.lib
}

RESOURCES += \
    icons.qrc
