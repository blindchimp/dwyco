#!/bin/bash
pushd `dirname $0`
. settings.sh

if [ "$NDK_ABI" = "arm" ]
then
	abi="arm-linux-androideabi"
	host="arm-linux"
	export CFLAGS="-fPIC -DANDROID -D__thumb__ -mthumb -Wfatal-errors -Wno-deprecated"
else
	abi="i686-linux-android"
	host="x86-linux"
	export CFLAGS="-fPIC -DANDROID -Wfatal-errors -Wno-deprecated"
fi

maindir=`pwd`
oggpath="/libogg/output"
oggdir=$maindir$oggpath

pushd libvorbis

thisdir=`pwd`
prefixpath="/output"
prefixdir=$thisdir$prefixpath

export CC="$abi-gcc"
export LD="$abi-ld"
export RANLIB="$abi-ranlib"
export AR="$abi-ar"
export OGG_CFLAGS=-I$oggdir/include

./configure \
--prefix=$prefixdir \
--host=$host \
--enable-static \
--with-ogg-libraries=$oggdir/lib \
--with-ogg-includes=$oggdir/include \
--disable-oggtest \
--disable-shared \
--disable-examples 


popd;popd
