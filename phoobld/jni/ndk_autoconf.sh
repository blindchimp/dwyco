#!/bin/bash
. settings.sh

if [ `uname` = "Darwin" ]
then
export HOST_TAG="darwin-x86_64"
else
export HOST_TAG="linux-x86_64"
fi

export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/$HOST_TAG
NDK_API_LEVEL=17
if [ $NDK_ABI = "arm64" -o $NDK_ABI = "x86_64" ]
then
NDK_API_LEVEL=21
fi

export CC=$TOOLCHAIN/bin/${TARGET_TAG}${NDK_API_LEVEL}-clang
export CXX=$TOOLCHAIN/bin/${TARGET_TAG}${NDK_API_LEVEL}-clang++

AR=`ndk-which --abi ${NDK_ABI_NAME} ar`
AS=`ndk-which --abi ${NDK_ABI_NAME} as`
LD=`ndk-which --abi ${NDK_ABI_NAME} ld`
RANLIB=`ndk-which --abi ${NDK_ABI_NAME} ranlib`
STRIP=`ndk-which --abi ${NDK_ABI_NAME} strip`

export AR
export AS
export LD
export RANLIB
export STRIP

#echo -n $AR | od -t x1
#echo -n $AS | od -t x1
#echo -n $RANLIB | od -t x1
#echo -n $LD | od -t x1
