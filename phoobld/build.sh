#!/bin/sh
D=$HOME
SHADOW_NAME=$D/git/build-phoo

mkdir $SHADOW_NAME
mkdir $SHADOW_NAME/lib
mkdir $SHADOW_NAME/include

(
opwd=$PWD
cd $SHADOW_NAME
qmake DWYCO_CONFDIR=phoobld $opwd/phoobld.pro
make -j 8
)
