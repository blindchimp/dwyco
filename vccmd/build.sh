#!/bin/sh
D=$HOME
SHADOW_NAME=$D/git/build-lh

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

if [ ! -e $SHADOW_NAME/include/sp.h ]
then
( 
cd bld/spread-hacked 
cd `cat curver` 
./configure --prefix=$SHADOW_NAME 
make -j 8
make install 
) 
fi

(
opwd=$PWD
cd $SHADOW_NAME
qmake DWYCO_CONFDIR=vccmd $opwd/lh.pro
make -j 8
)
