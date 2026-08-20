#!/bin/sh

if [ "$(uname)" = "Darwin" ]; then
    NDK_BASE="$HOME/Library/Android/sdk/ndk"
else
    NDK_BASE="$HOME/Android/Sdk/ndk"
fi
export NDK="$NDK_BASE/28.2.13676358"
SRC_DIR=$PWD
BUILD_DIR=$SRC_DIR/build

if [ ! -d "$SRC_DIR/tox-deps" ]; then
    echo "ERROR: tox-deps not found. Run ./build-tox-deps.sh first."
    exit 1
fi

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
    -DDWYCO_TOXCORE=ON \
    -GNinja
  cmake --build "$BUILD_DIR/$ABI"
done
