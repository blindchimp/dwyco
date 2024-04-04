#!/bin/sh
# note: qt4 qmake doesn't work for this
export QT_SELECT=5
D=/src
SHADOW_NAME=$D/git/build-afl

mkdir $SHADOW_NAME
mkdir $SHADOW_NAME/lib
mkdir $SHADOW_NAME/include

#if [ ! -e $SHADOW_NAME/include/sp.h ]
#then
#( 
#cd bld/spread-hacked 
#cd `cat curver` 
#./configure --prefix=$SHADOW_NAME 
#make -j 8
#make install 
#) 
#fi

(
opwd=$PWD
cd $SHADOW_NAME
qmake DWYCO_CONFDIR=fuzz $opwd/afl.pro
make -j 8
)
