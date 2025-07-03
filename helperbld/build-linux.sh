#!/bin/sh
D=$HOME
export PATH=$D/Qt/6.8.3/gcc_64/bin:$PATH
SHADOW_NAME=$D/git/build-helpers
mkdir -p $SHADOW_NAME

(
opwd=$PWD
cd $SHADOW_NAME
qmake DWYCO_CONFDIR=helperbld QMAKE_APPLE_DEVICE_ARCHS="x86_64 arm64" $opwd/helpers.pro
make -j 8
)
