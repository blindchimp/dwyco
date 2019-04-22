#-------------------------------------------------
#
# Project created by QtCreator 2018-03-17T11:44:41
#
#-------------------------------------------------

QT       -= core gui

TARGET = dwyco_jni
TEMPLATE = lib
CONFIG -= qt

DEFINES += DWYCO_JNI_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
SOURCES += dummy.cpp

android-g++ {
#QMAKE_LFLAGS += -Wl,--whole-archive
DEFINES += LINUX VCCFG_FILE CDCCORE_STATIC ANDROID

D = $${OUT_PWD}/../bld
equals(ANDROID_TARGET_ARCH, x86) {
L = $$PWD/../libs/x86
} else {
L = $$PWD/../libs/armeabi-v7a
}


# link against shared lib that is also used by the background, saves a bit of
# code but renders debugger useless. also NOTE: none of the JNI stuff will
# work in the main executable if it is statically linked. that is a
# limitation of java as far as i can tell.
#LIBS += $${L}/libdwyco_jni.so
#ANDROID_EXTRA_LIBS += $${L}/libdwyco_jni.so
LIBS += \
$${D}/cdc32/libcdc32.a \
$${D}/vc/libvc.a \
$${D}/crypto5/libcrypto5.a \
$${D}/dwcls/libdwcls.a \
$${D}/gsm/libgsm.a \
$${D}/kazlib/libkazlib.a \
$${D}/ppm/libppm.a \
$${D}/pgm/libpgm.a \
$${D}/pbm/libpbm.a \
$${D}/zlib/libzlib.a \
$${D}/theora/libtheora.a \
$${D}/vorbis112/libvorbis.a \
$${D}/ogg/libogg.a \
$${D}/jenkins/libjenkins.a \
$${D}/speex/libspeex.a


}
