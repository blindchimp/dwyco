#!/bin/sh
D=$HOME
SHADOW_NAME=$D/git/build-cdcx

mkdir $SHADOW_NAME
mkdir $SHADOW_NAME/lib
mkdir $SHADOW_NAME/include

(cd bld/libuv
make -j 8
cp libuv.a $SHADOW_NAME/lib
cp include/uv.h $SHADOW_NAME/include
)

(
opwd=$PWD
cd $SHADOW_NAME
qmake CONFIG+=debug -spec macx-g++ DWYCO_CONFDIR=cdcxbld $opwd/dwycore.pro
make -j 8
)
