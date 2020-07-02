#!/bin/bash
D=$HOME
SHADOW_NAME=$D/build-cdcx

mkdir $SHADOW_NAME
mkdir $SHADOW_NAME/lib
mkdir $SHADOW_NAME/include

(
opwd=$PWD
cd $SHADOW_NAME
qmake CONFIG+=release DWYCO_CONFDIR=cdcxbld $opwd/dwycore.pro
make -j 8
)
