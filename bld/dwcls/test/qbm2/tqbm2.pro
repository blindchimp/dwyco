TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

linux-*|macx-*: DEFINES += LINUX
INCLUDEPATH += ../.. ../../../kazlib

linux:LIBS += /home/dwight/Downloads/libkazlib.a /home/dwight/Downloads/libdwcls.a
macx:LIBS += /Users/dwight/Downloads/libkazlib.a /Users/dwight/Downloads/libdwcls.a


SOURCES += \
    ../dwqbm2.cpp \
    ../../useful.cpp \
    ../../dwhash.cpp \
    ../../dwstr.cpp

HEADERS += \
    ../../dwqbm2.h
