#!/bin/bash
pushd `dirname $0`
. settings.sh
. ndk_autoconf.sh

if [ "$NDK_ABI" = "arm" ]
then
	export CFLAGS="-fPIC -DANDROID -march=armv7-a"
else
	export CFLAGS="-fPIC -DANDROID"
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

export OGG_CFLAGS=-I$oggdir/include

autoreconf -if
./configure \
--prefix=`pwd`/output \
--host=$TARGET_TAG \
--disable-shared \
--enable-static \
--with-ogg=$oggdir \
--with-vorbis=$vorbisdir  \
--disable-examples \
--disable-asm

popd;popd
