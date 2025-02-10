#!/bin/sh
export PATH=~/Qt/6.8.2/clang_64/bin:$PATH
D=$HOME
SHADOW_NAME=$D/git/build-phoo

mkdir $SHADOW_NAME
mkdir $SHADOW_NAME/lib
mkdir $SHADOW_NAME/include

(
opwd=$PWD
cd $SHADOW_NAME
qmake -spec macx-clang DWYCO_CONFDIR=phoobld QMAKE_APPLE_DEVICE_ARCHS="x86_64 arm64" $opwd/phoobld.pro
make -j 8
)
