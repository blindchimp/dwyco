#!/bin/sh
D=$HOME
SHADOW_NAME=$D/git/build-phoo

mkdir $SHADOW_NAME
mkdir $SHADOW_NAME/lib
mkdir $SHADOW_NAME/include

(
cd bld/libuv
make -j 8
mkdir -p $SHADOW_NAME/bld/libuv
cp libuv.a $SHADOW_NAME/bld/libuv
#cp libuv.a $SHADOW_NAME/lib
cp include/uv.h $SHADOW_NAME/include
)

(
opwd=$PWD
cd $SHADOW_NAME
qmake DWYCO_CONFDIR=phoobld $opwd/phoobld.pro
make -j 8
)
