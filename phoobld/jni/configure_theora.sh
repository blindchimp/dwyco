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

thisdir=`pwd`

oggpath="/libogg/output"
oggdir=$thisdir$oggpath

vorbispath="/libvorbis/output"
vorbisdir=$thisdir$vorbispath

echo "**********"
echo $oggdir
echo $vorbisdir
echo "**********"

pushd libtheora

export CC="$abi-gcc"
export LD="$abi-ld"
export RANLIB="$abi-ranlib"
export AR="$abi-ar"
export OGG_CFLAGS=-I$oggdir/include

./configure \
--prefix=`pwd`/output \
--host=$host \
--disable-shared \
--enable-static \
--with-ogg=$oggdir \
--with-vorbis=$vorbisdir  \
--disable-examples

popd;popd
