#!/bin/sh

export NDK=$HOME/Android/Sdk/ndk/28.2.13676358
SRC_DIR=$PWD
BUILD_DIR=$SRC_DIR/build
#ABIS=("armeabi-v7a" "arm64-v8a" "x86" "x86_64")

for ABI in "armeabi-v7a" "arm64-v8a" "x86" "x86_64"
do
  echo "===== Building for $ABI ====="
  cmake \
    -S "$SRC_DIR" \
    -B "$BUILD_DIR/$ABI" \
    -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=$ABI \
    -DANDROID_PLATFORM=android-26 \
    -DANDROID_STL=c++_shared \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja
  cmake --build "$BUILD_DIR/$ABI"
done
