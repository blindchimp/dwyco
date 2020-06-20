TEMPLATE = subdirs
CONFIG += ordered

android-* {
SUBDIRS=bld/qt-qml-models bld/qt-supermacros bld/qtdrv
}

!android {
SUBDIRS=bld/jhead bld/ogg bld/vorbis112 bld/theora.1.2.x bld/speex bld/kazlib bld/zlib \
    bld/dwcls bld/crypto5 bld/jenkins bld/vc bld/pbm bld/pgm bld/ppm bld/gsm bld/cdc32 \
    bld/qt-qml-models bld/qt-supermacros bld/uv
SUBDIRS += bld/miniupnp/miniupnp-master/miniupnpc
}

linux-*:SUBDIRS += bld/v4lcap bld/qtdrv
macx-clang: SUBDIRS += bld/qtdrv
SUBDIRS += phoo
