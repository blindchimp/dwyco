#!/bin/sh
export PATH=$PATH:$HOME/Qt/5.15.2/clang_64/bin
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
