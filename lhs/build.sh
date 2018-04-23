#!/bin/sh
D=$HOME
mkdir $D/git/build-lh
mkdir $D/git/build-lh/lib
mkdir $D/git/build-lh/include

(cd bld/libuv
make -j 8
cp libuv.a $D/git/build-lh/lib
cp include/uv.h $D/git/build-lh/include
)
(cd bld/spread-hacked
cd `cat curver`
./configure --prefix=$D/git/build-lh
make -j 8 && make install
)

(
opwd=$PWD
cd $D/git/build-lh
qmake DWYCO_CONFDIR=lhs $opwd/lh.pro
make -j 8
)
