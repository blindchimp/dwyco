#!/bin/bash
. settings.sh
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/$HOST_TAG
export AR=$TOOLCHAIN/bin/${TARGET_TAG}-ar
export AS=$TOOLCHAIN/bin/${TARGET_TAG}-as
export CC=$TOOLCHAIN/bin/${TARGET_TAG}21-clang
export CXX=$TOOLCHAIN/bin/${TARGET_TAG}21-clang++
export LD=$TOOLCHAIN/bin/${TARGET_TAG}-ld
export RANLIB=$TOOLCHAIN/bin/${TARGET_TAG}-ranlib
export STRIP=$TOOLCHAIN/bin/${TARGET_TAG}-strip
