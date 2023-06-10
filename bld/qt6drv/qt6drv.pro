QT -= gui
QT += multimedia concurrent qml multimediawidgets
android: QT += androidextras
android: TARGET="qt6drv_$${QT_ARCH}"
TEMPLATE = lib
DEFINES += QTDRV_LIBRARY

CONFIG += c++11
CONFIG += static

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include($$PWD/../../$$DWYCO_CONFDIR/conf.pri)

INCLUDEPATH += ../miscsrc ../ppm ../pgm ../pbm ../cdc32

SOURCES += \
    audi_qt6.cpp \
    audo_qt6.cpp \
    vgqt6.cpp

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
