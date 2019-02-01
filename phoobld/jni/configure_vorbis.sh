#!/bin/bash
pushd `dirname $0`
. settings.sh

if [ "$NDK_ABI" = "arm" ]
then
	host="arm-linux-androideabi"
	export CFLAGS="-fPIC -DANDROID"
else
	host="i686-linux-android"
	export CFLAGS="-fPIC -DANDROID"
fi

maindir=`pwd`
oggpath="/libogg/output"
oggdir=$maindir$oggpath

pushd libvorbis

thisdir=`pwd`
prefixpath="/output"
prefixdir=$thisdir$prefixpath

#export CC="$abi-gcc"
#export LD="$abi-ld"
#export RANLIB="$abi-ranlib"
#export AR="$abi-ar"
export OGG_CFLAGS=-I$oggdir/include

autoreconf -if
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
