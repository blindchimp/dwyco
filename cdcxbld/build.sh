#!/bin/sh
export PATH=$HOME/Qt/6.7.2/macos/bin:$PATH

U=`uname`

D=$HOME
SHADOW_NAME=$D/git/build-cdcx

mkdir $SHADOW_NAME
mkdir $SHADOW_NAME/lib
mkdir $SHADOW_NAME/include

if [ $U = Darwin ]
then
	$D/git/dwyco/cdcx/dumptime-mac $D/git/dwyco/cdcx/main.cpp $D/git/dwyco/cdcx/buildtime.h
else
	$D/git/dwyco/cdcx/dumptime $D/git/dwyco/cdcx/main.cpp $D/git/dwyco/cdcx/buildtime.h
fi


(
opwd=$PWD
cd $SHADOW_NAME
qmake CONFIG+=release DWYCO_CONFDIR=cdcxbld QMAKE_APPLE_DEVICE_ARCHS="x86_64 arm64" $opwd/dwycore.pro
make -j 8
)
