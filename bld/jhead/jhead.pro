TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt
android|linux-g++:QMAKE_CXXFLAGS += -fpermissive
INCLUDEPATH=cjhead

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

SOURCES = \
exif.cpp \
iptc.cpp \
jhead.cpp \
jpgfile.cpp \
makernote.cpp \
jpgqguess.cpp \
gpsinfo.cpp \
paths.cpp

