#!/bin/bash
pushd `dirname $0`
. settings.sh
pushd jpeg
make clean
make -i
make -i install
popd; popd
