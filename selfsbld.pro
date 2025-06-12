TEMPLATE = subdirs
CONFIG += ordered

android-* {
SUBDIRS=bld/qt-qml-models bld/qt-supermacros bld/qt6drv
}

!android {
SUBDIRS=bld/jhead bld/ogg bld/vorbis112 bld/theora.1.2.x bld/speex bld/kazlib \
    bld/dwcls bld/crypto8 bld/jenkins bld/vc bld/pbm bld/pgm bld/ppm bld/gsm bld/cdc32 \
    bld/qt-qml-models bld/qt-supermacros bld/uv
SUBDIRS += bld/miniupnp/miniupnp-master/miniupnpc
}

linux-*:SUBDIRS +=  bld/qt6drv
macx-clang: SUBDIRS += bld/qt6drv
macx-ios-clang: SUBDIRS += bld/qt6drv
SUBDIRS += selfs
#android: include(/home/dwight/android/astudio/android_openssl/openssl.pri)
#android: include(/Users/dwight/android/astudio/android_openssl/openssl.pri)
