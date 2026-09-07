#!/bin/bash
# Create 500 client accounts and have each join group "barfybarf" (password "foofoo").
# Assumes a group member is already alive and running in "barfybarf".
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
GROUP_NAME="barfybarf"
MAX_PARALLEL=${MAX_PARALLEL:-50}

run_one() {
    local i=$1
    local dir="/tmp/dwy${i}"
    mkdir -p "$dir"
    if "$SCRIPT_DIR/create_and_enter_group.sh" "$dir" "$GROUP_NAME" 2>&1; then
        echo "OK  $i"
    else
        echo "FAIL $i"
    fi
}

i=0
while [ $i -lt 500 ]; do
    run_one $i &
    # throttle concurrency
    if [ $(( (i + 1) % MAX_PARALLEL )) -eq 0 ]; then
        wait
    fi
    i=$((i + 1))
done

echo "Waiting for remaining jobs..."
wait
echo "Done. All 500 clients launched."
