#!/bin/bash
pushd `dirname $0`
. settings.sh

if [ "$NDK_ABI" = "arm" ]
then
#$NDK/build/tools/make-standalone-toolchain.sh --toolchain=arm-linux-androideabi-4.8 --install-dir=./toolchain
$NDK/build/tools/make_standalone_toolchain.py --arch arm -v --api 19 --stl libc++ --install-dir ./toolchain
else
#$NDK/build/tools/make-standalone-toolchain.sh --toolchain=x86-4.8 --install-dir=./toolchain
$NDK/build/tools/make_standalone_toolchain.py --arch x86 -v --api 19 --stl libc++ --install-dir ./toolchain
fi
# --system=linux-x86
