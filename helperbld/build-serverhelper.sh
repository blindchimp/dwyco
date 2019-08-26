#!/bin/sh
D=$HOME
SHADOW_NAME=$D/git/build-serverhelper
mkdir -p $SHADOW_NAME

(
opwd=$PWD
cd $SHADOW_NAME
qmake DWYCO_CONFDIR=helperbld $opwd/serverhelper.pro
make -j 8
)
