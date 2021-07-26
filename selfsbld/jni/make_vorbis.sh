#!/bin/bash
pushd `dirname $0`
. settings.sh
pushd libvorbis
make clean
make
make install
popd; popd
