#!/bin/sh
#
# Clone the source trees that build-tox-deps.sh expects, including the
# submodules needed to cross-compile for Android:
#   c-toxcore       (TokTok/c-toxcore, third_party/cmp submodule)
#   libsodium-cmake (robinlinden/libsodium-cmake, libsodium submodule)
#
# Existing checkouts are left alone; their submodules are just updated.
#
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

TC_DIR="${C_TOXCORE_DIR:-$SCRIPT_DIR/../../c-toxcore}"
SODIUM_DIR="${LIBSODIUM_CMAKE_DIR:-$HOME/git/libsodium-cmake}"

TC_REPO="${C_TOXCORE_REPO:-https://github.com/TokTok/c-toxcore.git}"
TC_REF="${C_TOXCORE_REF:-v0.2.24-rc.4}"

SODIUM_REPO="${LIBSODIUM_CMAKE_REPO:-https://github.com/robinlinden/libsodium-cmake.git}"
SODIUM_REF="${LIBSODIUM_CMAKE_REF:-9b2848dfc1b917a9410f0de9d81059b26cbfaa8d}"

clone_repo() {
    url=$1
    dest=$2
    ref=$3

    if [ -d "$dest/.git" ]; then
        echo "=== $dest already exists, skipping clone ==="
    else
        echo "=== Cloning $url -> $dest ==="
        mkdir -p "$(dirname "$dest")"
        git clone "$url" "$dest"
        git -C "$dest" checkout "$ref"
    fi
    git -C "$dest" submodule update --init --recursive
}

clone_repo "$TC_REPO" "$TC_DIR" "$TC_REF"
clone_repo "$SODIUM_REPO" "$SODIUM_DIR" "$SODIUM_REF"

echo "=== Tox dependency sources ready; now run ./build-tox-deps.sh ==="
