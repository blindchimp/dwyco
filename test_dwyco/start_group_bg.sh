#!/bin/bash
# start_group_bg.sh - start dwycobg in group sync mode on a given client directory.
#
# The client directory must already have an account created (e.g. via create_account).
#
# There are two cases:
#   - If the client is NOT yet in the group, dwycobg joins it (exits when the
#     join completes), then this script restarts it for ongoing sync.
#   - If the client IS already in the same group, dwycobg just runs doing eager sync.
#
# In both cases the final long-running process is backgrounded and its PID/port
# are printed to stdout. The process uses a FIXED port (9797) as its "funny
# mutex", so a caller can force it to exit by connecting to that port
# (e.g. `python3 -c 'import socket;s=socket.socket();s.connect(("127.0.0.1",9797))'`),
# or use the DWYCOBG_PID to signal it.
#
# Usage:
#   ./start_group_bg.sh <client_dir> <group_name> [password] [build_dir]
#
#   client_dir:   path to the client directory (e.g. /tmp/dwyABC)
#   group_name:   name of the group to join / sync with
#   password:     group password (default: "foofoo")
#   build_dir:    directory containing built binaries (default: auto-detect)
set -e
set -x

PORT=9797

CLIENT_DIR="${1:?Usage: $0 <client_dir> <group_name> [password] [build_dir]}"
GROUP_NAME="${2:?Usage: $0 <client_dir> <group_name> [password] [build_dir]}"
PASSWORD="${3:-foofoo}"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${4:-${BUILD_DIR:-$SCRIPT_DIR/build}}"
if [ ! -x "$BUILD_DIR/dwycobg" ]; then
    if [ -x "$SCRIPT_DIR/build/dwycobg" ]; then
        BUILD_DIR="$SCRIPT_DIR/build"
    elif [ -x "/tmp/dwy_test_build/dwycobg" ]; then
        BUILD_DIR="/tmp/dwy_test_build"
    fi
fi

DWYCOBG_BIN="$BUILD_DIR/dwycobg"
if [ ! -x "$DWYCOBG_BIN" ]; then
    echo "Error: dwycobg not found in $BUILD_DIR. Build test_dwyco first." >&2
    exit 1
fi

if [ ! -d "$CLIENT_DIR" ]; then
    echo "Error: client directory $CLIENT_DIR does not exist." >&2
    exit 1
fi

echo "Starting dwycobg group mode:" >&2
echo "  client:  $CLIENT_DIR" >&2
echo "  group:   $GROUP_NAME" >&2
echo "  port:    $PORT" >&2

# If the client is already in a group matching GROUP_NAME, dwycobg will
# just run doing sync (it will NOT exit), so we launch it directly in the
# background. Otherwise (not in a group) dwycobg joins, exits, and we
# restart it after the join completes.
ALT_NAME=$(python3 - "$CLIENT_DIR" <<'EOF'
import sys, sqlite3, os
d = sys.argv[1]
p = os.path.join(d, "set.sql")
if not os.path.exists(p):
    print("")
    sys.exit(0)
try:
    con = sqlite3.connect(p)
    row = con.execute("select value from settings where name='group/alt_name'").fetchone()
    con.close()
    print(row[0] if row else "")
except Exception:
    print("")
EOF
)

if [ -n "$ALT_NAME" ] && [ "$ALT_NAME" = "$GROUP_NAME" ]; then
    # already in this group - just run doing eager sync
    (cd "$CLIENT_DIR" && exec "$DWYCOBG_BIN" "$PORT" "$GROUP_NAME" "$PASSWORD") &
    BG_PID=$!
    echo "DWYCOBG_PID=$BG_PID"
    echo "DWYCOBG_PORT=$PORT"
    exit 0
fi

# not in this group (or in a different one) - dwycobg will attempt the join
# and exit when done, then we restart for ongoing sync.
(cd "$CLIENT_DIR" && exec "$DWYCOBG_BIN" "$PORT" "$GROUP_NAME" "$PASSWORD")
RC=$?

if [ "$RC" -ne 0 ]; then
    echo "Error: dwycobg exited with status $RC (group join may have failed)." >&2
    exit 1
fi

echo "Join completed, restarting for sync..." >&2
(cd "$CLIENT_DIR" && exec "$DWYCOBG_BIN" "$PORT" "$GROUP_NAME" "$PASSWORD") &
BG_PID=$!
echo "DWYCOBG_PID=$BG_PID"
echo "DWYCOBG_PORT=$PORT"
