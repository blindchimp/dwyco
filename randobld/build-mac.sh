#!/bin/sh
export PATH=~/Qt/5.15.2/clang_64/bin:$PATH
D=$HOME
SHADOW_NAME=$D/git/build-rando

mkdir $SHADOW_NAME
mkdir $SHADOW_NAME/lib
mkdir $SHADOW_NAME/include

(
opwd=$PWD
cd $SHADOW_NAME
qmake -spec macx-clang DWYCO_CONFDIR=randobld $opwd/randobld.pro
make -j 8
)
