#!/bin/bash
D=$HOME
SHADOW_NAME=$D/build-cdcx

mkdir $SHADOW_NAME
mkdir $SHADOW_NAME/lib
mkdir $SHADOW_NAME/include

(
opwd=$PWD
cd $SHADOW_NAME
qmake CONFIG+=release -spec macx-g++ DWYCO_CONFDIR=cdcxbld $opwd/dwycore.pro
# this is goofy because qt4 has some old flags that
# we need to override, but i don't want to tweak the
# qt4 install's mkspecs
make qmake
sed 's/10.5/10.9/g' <cdcx/Makefile >/tmp/kk
mv /tmp/kk cdcx/Makefile
make -j 8
)
