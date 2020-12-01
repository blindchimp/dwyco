TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

linux-*|macx-*: DEFINES += LINUX
INCLUDEPATH += ../.. ../../../kazlib

SOURCES += \
    ../dwqbm2.cpp \
    ../../useful.cpp \
    ../../dwhash.cpp \
    ../../dwstr.cpp
