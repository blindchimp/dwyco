#!/bin/sh
D=$HOME
SHADOW_NAME=$D/git/build-helpers
mkdir -p $SHADOW_NAME

(
opwd=$PWD
cd $SHADOW_NAME
qmake $opwd/bots.pro
make -j 8
)
