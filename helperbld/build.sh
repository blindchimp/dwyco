#!/bin/sh
D=$HOME
export PATH=$D/Qt/5.15.2/clang_64/bin:$PATH
SHADOW_NAME=$D/git/build-helpers
mkdir -p $SHADOW_NAME

(
opwd=$PWD
cd $SHADOW_NAME
qmake DWYCO_CONFDIR=helperbld $opwd/helpers.pro
make -j 8
)
