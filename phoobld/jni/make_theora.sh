#!/bin/bash
pushd `dirname $0`
. settings.sh
pushd libtheora
make clean
make
make install
popd; popd
