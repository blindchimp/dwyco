TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

linux-*|macx-*: DEFINES += LINUX
INCLUDEPATH += ../..

SOURCES += \
    ../tqmap.cpp \
    ../../useful.cpp \
    ../../dwhash.cpp \
    ../../dwstr.cpp
