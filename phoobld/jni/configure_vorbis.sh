#!/bin/bash
pushd `dirname $0`
. settings.sh
. ndk_autoconf.sh

export CFLAGS="-fPIC -DANDROID"

maindir=`pwd`
oggpath="/libogg/output"
oggdir=$maindir$oggpath

pushd libvorbis

thisdir=`pwd`
prefixpath="/output"
prefixdir=$thisdir$prefixpath

export OGG_CFLAGS=-I$oggdir/include

autoreconf -if
./configure \
--prefix=$prefixdir \
--host=$TARGET_TAG \
--enable-static \
--with-ogg-libraries=$oggdir/lib \
--with-ogg-includes=$oggdir/include \
--disable-oggtest \
--disable-shared \
--disable-examples 


popd;popd
