#!/bin/sh
# do an in-tree build of phoo
# then create an AppDir in phoo/appdir
(
cd bld/libuv
make -j 4
)
qmake CONFIG+=release CONFIG+=appdir DWYCO_CONFDIR=phoobld phoobld.pro
make -j 8
make INSTALL_ROOT=appdir install
