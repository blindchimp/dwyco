#!/bin/sh
# Build dwycobg with system cmake (no Qt)
# Usage: ./build.sh [Debug|Release]

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DWYCO_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_TYPE="${1:-Release}"
BUILD_DIR="$HOME/git/build-dwycobg"

cmake -S "$DWYCO_DIR/dwycobg" -B "$BUILD_DIR" \
    -D CMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -D DWYCO_TOXCORE=ON \
    -G Ninja

cmake --build "$BUILD_DIR" 
