#!/bin/bash
pushd `dirname $0`
. settings.sh
. ndk_autoconf.sh

export CFLAGS="-fPIC -DANDROID"

pushd libogg

thisdir=`pwd`
prefixpath="/output"
prefixdir=$thisdir$prefixpath


autoreconf -if
./configure \
--prefix=$prefixdir \
--host=$TARGET_TAG \
--disable-shared \
--enable-static 

popd;popd
