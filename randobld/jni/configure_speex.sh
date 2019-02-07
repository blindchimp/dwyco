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

#export CC="$abi-gcc"
#export LD="$abi-ld"
#export RANLIB="$abi-ranlib"
#export AR="$abi-ar"

autoreconf -if
./configure \
--prefix=`pwd`/output \
--host=$host \
--disable-shared \
--enable-static \
--with-ogg=$oggdir \
--with-vorbis=$vorbisdir \
--enable-fixed-point

popd;popd
