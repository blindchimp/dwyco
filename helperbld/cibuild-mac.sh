#!/bin/bash
D=$HOME
SHADOW_NAME=$D/build-helpers
mkdir $SHADOW_NAME

(
opwd=$PWD
cd $SHADOW_NAME
qmake CONFIG+=release -spec macx-g++ DWYCO_CONFDIR=helperbld $opwd/helpers.pro
# this is goofy because qt4 has some old flags that
# we need to override, but i don't want to tweak the
# qt4 install's mkspecs
make qmake
for i in xferdl dwycobg qdwyrun
do
sed 's/10.5/10.9/g' <$i/Makefile >/tmp/kk
mv /tmp/kk $i/Makefile
done

make -j 8
)
