#!/bin/bash
pushd `dirname $0`
. settings.sh
. ndk_autoconf.sh

# asm is currently only for arm32
if [ $NDK_ABI = "arm64" ]
then
	disable_neon="--disable-neon"
else
	disable_neon=""
fi

export CFLAGS="-fPIC -DANDROID"

thisdir=`pwd`

oggpath="/libogg/output"
oggdir=$thisdir$oggpath

vorbispath="/libvorbis/output"
vorbisdir=$thisdir$vorbispath

echo "**********"
echo $oggdir
echo $vorbisdir
echo "**********"

pushd libspeex

autoreconf -if
./configure \
--prefix=`pwd`/output \
--host=$TARGET_TAG \
--disable-shared \
--enable-static \
--with-ogg=$oggdir \
--with-vorbis=$vorbisdir \
--enable-fixed-point \
--disable-examples \
$disable_neon


popd;popd
