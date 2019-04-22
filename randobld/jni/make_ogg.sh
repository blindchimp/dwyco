#!/bin/bash
pushd `dirname $0`
. settings.sh
pushd libogg
make clean
make -i
make -i install
popd; popd
