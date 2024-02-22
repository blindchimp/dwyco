#!/bin/bash
. settings.sh

if [ `uname` = "Darwin" ]
then
export HOST_TAG="darwin-x86_64"
else
export HOST_TAG="linux-x86_64"
fi

export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/$HOST_TAG

export CC=$TOOLCHAIN/bin/${TARGET_TAG}21-clang
export CXX=$TOOLCHAIN/bin/${TARGET_TAG}21-clang++

AR=$TOOLCHAIN/bin/llvm-ar
AS=$TOOLCHAIN/bin/llvm-as
LD=$TOOLCHAIN/bin/ld
RANLIB=$TOOLCHAIN/bin/llvm-ranlib
STRIP=$TOOLCHAIN/bin/llvm-strip

#AR=`ndk-which --abi ${NDK_ABI_NAME} ar`
#AS=`ndk-which --abi ${NDK_ABI_NAME} as`
#LD=`ndk-which --abi ${NDK_ABI_NAME} ld`
#RANLIB=`ndk-which --abi ${NDK_ABI_NAME} ranlib`
#STRIP=`ndk-which --abi ${NDK_ABI_NAME} strip`

export AR
export AS
export LD
export RANLIB
export STRIP

#echo -n $AR | od -t x1
#echo -n $AS | od -t x1
#echo -n $RANLIB | od -t x1
#echo -n $LD | od -t x1
