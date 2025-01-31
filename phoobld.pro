TEMPLATE = subdirs
CONFIG += ordered

equals(QT_MAJOR_VERSION, 6) {
QTDRV=qt6drv
}
!equals(QT_MAJOR_VERSION, 6) {
QTDRV=qtdrv
}

android-* {
SUBDIRS=bld/qt-qml-models bld/qt-supermacros bld/$$QTDRV
}

!android {
SUBDIRS=bld/jhead bld/ogg bld/vorbis112 bld/theora.1.2.x bld/speex bld/kazlib \
    bld/dwcls bld/crypto5 bld/jenkins bld/vc bld/pbm bld/pgm bld/ppm bld/gsm bld/cdc32 \
    bld/qt-qml-models bld/qt-supermacros bld/uv
SUBDIRS += bld/miniupnp/miniupnp-master/miniupnpc
}

linux-*:SUBDIRS += bld/v4lcap bld/$$QTDRV
macx-clang|macx-g++: SUBDIRS += bld/$$QTDRV
macx-ios-clang: SUBDIRS += bld/$$QTDRV
win*: SUBDIRS += bld/$$QTDRV
SUBDIRS += phoo
#android: include(/home/dwight/android/astudio/android_openssl/openssl.pri)
#android: include(/Users/dwight/android/astudio/android_openssl/openssl.pri)
