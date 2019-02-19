TEMPLATE = subdirs
CONFIG += ordered

android-* {
SUBDIRS=bld/qt-qml-models bld/qt-supermacros
}

!android {
SUBDIRS=bld/jhead bld/kazlib bld/zlib bld/dwcls bld/crypto5 bld/jenkins bld/vc bld/pbm bld/pgm bld/ppm bld/cdc32 bld/qt-qml-models bld/qt-supermacros
}

SUBDIRS += rando
