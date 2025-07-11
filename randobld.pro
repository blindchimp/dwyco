TEMPLATE = subdirs
CONFIG += ordered

android-* {
SUBDIRS=bld/qt-qml-models bld/qt-supermacros
}

linux-*|macx-clang*|win32-msvc* {
SUBDIRS=bld/jhead bld/kazlib bld/dwcls bld/crypto8 bld/jenkins bld/vc bld/pbm bld/cdc32 bld/qt-qml-models bld/qt-supermacros \
 bld/uv
}

macx-ios* {
SUBDIRS=bld/jhead bld/kazlib bld/dwcls bld/crypto8 bld/jenkins bld/vc bld/pbm bld/cdc32 bld/qt-qml-models bld/qt-supermacros bld/uv
}

SUBDIRS += rando
