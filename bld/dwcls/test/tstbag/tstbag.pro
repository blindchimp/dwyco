TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../..

SOURCES += \
    ../tstbag.cpp \
    ../../useful.cpp \
    ../../dwhash.cpp
