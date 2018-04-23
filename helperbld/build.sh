#!/bin/sh
D=$HOME
SHADOW_NAME=$D/git/build-helpers
mkdir -p $SHADOW_NAME

(
opwd=$PWD
cd $SHADOW_NAME
qmake DWYCO_CONFDIR=helperbld $opwd/helpers.pro
make -j 8
)
