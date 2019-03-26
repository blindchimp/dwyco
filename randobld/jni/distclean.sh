#!/bin/bash
pushd `dirname $0`
. settings.sh
for i in libogg libvorbis libspeex libtheora
do

pushd $i
make -i mostlyclean clean distclean maintainer-clean
find . -name Makefile.in -delete -print
popd

done
popd
