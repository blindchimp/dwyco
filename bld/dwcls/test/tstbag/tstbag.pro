TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

linux-*: DEFINES += LINUX
INCLUDEPATH += ../..

SOURCES += \
    ../tstbag.cpp \
    ../../useful.cpp \
    ../../dwhash.cpp \
    ../../dwstr.cpp
