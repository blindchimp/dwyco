#!/bin/bash
#
# Build libsodium and c-toxcore as static libraries for Android.
# Produces per-ABI artifacts in phoo_dynbld/tox-deps/<ABI>/
#   lib/libsodium.a
#   lib/libtoxcore.a
#   include/sodium.h, sodium/*
#   include/tox/tox.h, toxencryptsave.h
#
set -eu

if [ "$(uname)" = "Darwin" ]; then
    NDK_BASE="$HOME/Library/Android/sdk/ndk"
else
    NDK_BASE="$HOME/Android/Sdk/ndk"
fi
NDK="$NDK_BASE/28.2.13676358"
API=21

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SRC_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
DEPS_DIR="$SCRIPT_DIR/tox-deps"
TC_SRC="$SRC_DIR/../c-toxcore"
SODIUM_SRC="$HOME/git/libsodium-cmake"

ABIS="armeabi-v7a arm64-v8a x86 x86_64"

if [ ! -d "$TC_SRC" ]; then
    echo "ERROR: c-toxcore not found at $TC_SRC"
    exit 1
fi

# --- libsodium via robinlinden/libsodium-cmake (pure CMake, no autotools) ---
if [ ! -d "$SODIUM_SRC" ]; then
    echo "ERROR: libsodium-cmake not found at $SODIUM_SRC — please ensure it exists before running this script."
    exit 1
fi

for ABI in $ABIS; do
    PREFIX="$DEPS_DIR/$ABI"
    if [ -f "$PREFIX/lib/libsodium.a" ]; then
        echo "=== libsodium already built for $ABI, skipping ==="
        continue
    fi
    echo "=== Building libsodium for $ABI ==="
    cmake -S "$SODIUM_SRC" -B "$SODIUM_SRC/build-$ABI" \
        -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
        -DANDROID_ABI="$ABI" \
        -DANDROID_PLATFORM="android-$API" \
        -DANDROID_STL=none \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DBUILD_SHARED_LIBS=OFF \
        -DSODIUM_DISABLE_TESTS=ON \
        -GNinja
    cmake --build "$SODIUM_SRC/build-$ABI"
    # robinlinden/libsodium-cmake has no install target; copy manually
    mkdir -p "$PREFIX/lib" "$PREFIX/include"
    cp "$SODIUM_SRC/build-$ABI/libsodium.a" "$PREFIX/lib/"
    cp "$SODIUM_SRC/libsodium/src/libsodium/include/sodium.h" "$PREFIX/include/"
    cp -r "$SODIUM_SRC/libsodium/src/libsodium/include/sodium" "$PREFIX/include/"
done

# --- c-toxcore (static, no toxav, no tests, no daemon) ---
TC_WRAPPER="$SCRIPT_DIR/toxcore-cmake"
for ABI in $ABIS; do
    PREFIX="$DEPS_DIR/$ABI"
    if [ -f "$PREFIX/lib/libtoxcore.a" ]; then
        echo "=== c-toxcore already built for $ABI, skipping ==="
        continue
    fi
    echo "=== Building c-toxcore for $ABI ==="

    TC_BUILD="$TC_SRC/build-$ABI"
    rm -rf "$TC_BUILD"
    cmake -S "$TC_WRAPPER" -B "$TC_BUILD" \
        -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
        -DANDROID_ABI="$ABI" \
        -DANDROID_PLATFORM="android-$API" \
        -DANDROID_STL=none \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DTC_SRC="$TC_SRC" \
        -DSODIUM_INCLUDE_DIR="$PREFIX/include" \
        -GNinja
    cmake --build "$TC_BUILD"
    cmake --install "$TC_BUILD"
done

echo "=== All tox deps built ==="
