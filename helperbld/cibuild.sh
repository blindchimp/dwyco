#!/bin/bash
D=$HOME
SHADOW_NAME=$D/build-helpers
mkdir $SHADOW_NAME

(
opwd=$PWD
cd $SHADOW_NAME
qmake CONFIG+=release DWYCO_CONFDIR=helperbld $opwd/helpers.pro
make -j 8
)
