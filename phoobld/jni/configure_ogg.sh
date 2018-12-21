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

pushd libogg

thisdir=`pwd`
prefixpath="/output"
prefixdir=$thisdir$prefixpath

#export CC="$abi-gcc"
#export LD="$abi-ld"
#export RANLIB="$abi-ranlib"
#export AR="$abi-ar"

./configure \
--prefix=$prefixdir \
--host=$host \
--disable-shared \
--enable-static 

popd;popd
