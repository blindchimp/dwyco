#!/bin/sh
U=`uname`
if [ $U = Darwin ]
then
	export PATH=$HOME/Qt/6.8.2/macos/bin:$PATH
else
	export PATH=$HOME/Qt/6.8.2/gcc_64/bin:$PATH
fi


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
