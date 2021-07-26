#!/bin/sh
D=$HOME
SHADOW_NAME=$D/git/build-selfs

mkdir $SHADOW_NAME
mkdir $SHADOW_NAME/lib
mkdir $SHADOW_NAME/include

(
opwd=$PWD
cd $SHADOW_NAME
qmake -spec macx-clang DWYCO_CONFDIR=selfsbld $opwd/selfsbld.pro
make -j 8
)
