#!/bin/bash
pushd `dirname $0`
. settings.sh

pushd jpeg

thisdir=`pwd`
prefixpath="/output"
prefixdir=$thisdir$prefixpath

export CC="arm-linux-androideabi-gcc"
export LD="arm-linux-androideabi-ld"
export RANLIB="arm-linux-androideabi-ranlib"
export AR="arm-linux-androideabi-ar"
export CFLAGS="-fPIC -DANDROID -D__thumb__ -mthumb -Wfatal-errors -Wno-deprecated"

./configure \
--prefix=$prefixdir \
--host=arm-linux \
--disable-shared \
--enable-static 

popd;popd
