#!/bin/bash
# create_and_enter_group.sh - create an account then enter a group
# Group name is a short random string (8 hex chars by default), password is fixed "foofoo".
# Usage: ./create_and_enter_group.sh [user_dir] [group_name]
#   user_dir:   optional, defaults to mktemp -d /tmp/dwyXXXXXX
#   group_name: optional, defaults to random 8-char string
# Requires built binaries: create_account, join_group (and dwycobg for alternative flow)
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$SCRIPT_DIR/build}"
# fallback to /tmp build used during dev, and to build dir relative to script
if [ ! -x "$BUILD_DIR/create_account" ]; then
    if [ -x "$SCRIPT_DIR/build/create_account" ]; then
        BUILD_DIR="$SCRIPT_DIR/build"
    elif [ -x "/tmp/dwy_test_build/create_account" ]; then
        BUILD_DIR="/tmp/dwy_test_build"
    fi
fi

CREATE_BIN="$BUILD_DIR/create_account"
JOIN_BIN="$BUILD_DIR/join_group"
DWYCOBG_BIN="$BUILD_DIR/dwycobg"

if [ ! -x "$CREATE_BIN" ]; then
    echo "Missing $CREATE_BIN - build test_dwyco first (cmake --build $BUILD_DIR)" >&2
    exit 1
fi
if [ ! -x "$JOIN_BIN" ]; then
    echo "Missing $JOIN_BIN - rebuild after adding join_group.cpp" >&2
    exit 1
fi

USER_DIR="${1:-}"
GROUP_NAME="${2:-}"
PASSWORD="foofoo"

if [ -z "$USER_DIR" ]; then
    USER_DIR=$(mktemp -d /tmp/dwyXXXXXX)
    echo "Created temp dir: $USER_DIR" >&2
else
    mkdir -p "$USER_DIR"
fi

if [ -z "$GROUP_NAME" ]; then
    # short random string: 8 hex chars (4 random bytes)
    if command -v openssl >/dev/null 2>&1; then
        GROUP_NAME=$(openssl rand -hex 4)
    else
        GROUP_NAME=$(tr -dc 'a-z0-9' </dev/urandom | head -c8)
    fi
fi

echo "=== create account ===" >&2
echo "DIR=$USER_DIR" >&2
echo "GROUP=$GROUP_NAME PW=$PASSWORD" >&2

# create_account: 3 args means exit immediately after login (no 2-min wait)
# With 2 args it waits 2 minutes; we want fast path.
echo "Running $CREATE_BIN $USER_DIR dummy" >&2
"$CREATE_BIN" "$USER_DIR" dummy

echo "=== enter group ===" >&2
echo "Running $JOIN_BIN $USER_DIR $GROUP_NAME $PASSWORD" >&2
"$JOIN_BIN" "$USER_DIR" "$GROUP_NAME" "$PASSWORD"

echo "=== done ===" >&2
echo "DIR=$USER_DIR"
echo "GROUP=$GROUP_NAME"
echo "PASSWORD=$PASSWORD"

# Optional dwycobg alternative (commented):
#   The dwycobg helper can also perform the join via background_sync:
#   PORT=$(python3 -c 'import socket; s=socket.socket(); s.bind(("",0)); print(s.getsockname()[1])')
#   (cd "$USER_DIR" && "$DWYCOBG_BIN" "$PORT" "$GROUP_NAME" "$PASSWORD")
#   Using join_group is more direct for tests and gives explicit JOIN_OK/FAIL events.

