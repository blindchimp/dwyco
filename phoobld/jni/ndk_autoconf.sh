#!/bin/bash
. settings.sh

export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/$HOST_TAG
NDK_API_LEVEL=17
if [ $NDK_ABI = "arm64" ]
then
NDK_API_LEVEL=21
fi

if [ $NDK_ABI = "arm" ]
then
# dumb
export CC=$TOOLCHAIN/bin/${TARGET_TAG}${NDK_API_LEVEL}-clang
export CXX=$TOOLCHAIN/bin/${TARGET_TAG}${NDK_API_LEVEL}-clang++
export AR=$TOOLCHAIN/bin/arm-linux-androideabi-ar
export AS=$TOOLCHAIN/bin/arm-linux-androideabi-as
export LD=$TOOLCHAIN/bin/arm-linux-androideabi-ld
export RANLIB=$TOOLCHAIN/bin/arm-linux-androideabi-ranlib
export STRIP=$TOOLCHAIN/bin/arm-linux-androideabi-strip
else
export CC=$TOOLCHAIN/bin/${TARGET_TAG}${NDK_API_LEVEL}-clang
export CXX=$TOOLCHAIN/bin/${TARGET_TAG}${NDK_API_LEVEL}-clang++
export AR=$TOOLCHAIN/bin/${TARGET_TAG}-ar
export AS=$TOOLCHAIN/bin/${TARGET_TAG}-as
export LD=$TOOLCHAIN/bin/${TARGET_TAG}-ld
export RANLIB=$TOOLCHAIN/bin/${TARGET_TAG}-ranlib
export STRIP=$TOOLCHAIN/bin/${TARGET_TAG}-strip
fi
