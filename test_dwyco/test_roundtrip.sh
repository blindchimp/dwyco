#!/bin/bash
# Round-trip message test coordinator
# Requires local servers with test accounts UserA and UserB
#
# Usage: ./test_roundtrip.sh <user_a_dir> <user_a_uid_hex> <user_b_dir> <user_b_uid_hex>
#   UIDs must be in hex format (lowercase, no separators)

set -e

if [ $# -lt 4 ]; then
    echo "Usage: $0 <user_a_dir> <user_a_uid_hex> <user_b_dir> <user_b_uid_hex>"
    echo ""
    echo "  Runs a round-trip message test: UserA sends to UserB, UserB receives and verifies."
    echo "  Requires local servers with both accounts registered and pals."
    exit 1
fi

USERA_DIR="$1"
USERA_UID="$2"
USERB_DIR="$3"
USERB_UID="$4"
COORD_FILE="/tmp/dwytest_roundtrip_coord.$$"
RECV_LOG="/tmp/dwytest_roundtrip_recv.$$.log"

cleanup() {
    rm -f "$COORD_FILE" "$RECV_LOG"
}
trap cleanup EXIT

PASS=0
FAIL=0

run_test() {
    local name="$1"
    local text="$2"
    local no_forward="${3:-0}"
    local delay="${4:-3}"

    echo ""
    echo "=== $name ==="
    echo "  Text: '$text'"
    echo "  No forward: $no_forward"

    rm -f "$COORD_FILE"

    # Receiver must be online when the message is sent (messages are
    # only delivered to a connected peer), so start it first. It waits
    # for the coord file, then polls for the incoming message.
    echo "--- Starting receiver ---"
    ./dwytest_peer recv "$USERA_DIR" "$COORD_FILE" "$USERB_UID" > "$RECV_LOG" 2>&1 &
    RECV_PID=$!

    echo "--- Waiting ${delay}s for receiver to come online ---"
    sleep "$delay"

    echo "--- Sending ---"
    if ! ./dwytest_peer send "$USERB_DIR" "$COORD_FILE" "$USERA_UID" "$text" "$no_forward"; then
        wait "$RECV_PID" 2>/dev/null
        cat "$RECV_LOG"
        echo "FAIL: send failed"
        FAIL=$((FAIL + 1))
        return
    fi

    echo "--- Waiting for receiver to verify ---"
    if ! wait "$RECV_PID"; then
        cat "$RECV_LOG"
        echo "FAIL: receive failed"
        FAIL=$((FAIL + 1))
        return
    fi
    if ! grep -q "Receive OK" "$RECV_LOG"; then
        cat "$RECV_LOG"
        echo "FAIL: receiver did not confirm"
        FAIL=$((FAIL + 1))
        return
    fi

    PASS=$((PASS + 1))
    echo "PASS"
}

echo "============================================"
echo " DWYCO Round-Trip Message Tests"
echo "============================================"
echo "UserA dir: $USERA_DIR"
echo "UserA UID: $USERA_UID"
echo "UserB dir: $USERB_DIR"
echo "UserB UID: $USERB_UID"

run_test "Simple text" "Hello from UserA"
run_test "No forward" "Secret message" 1
run_test "Multi-word text" "The quick brown fox jumps over the lazy dog" 0
run_test "Numeric text" "12345 67890 111213" 0
run_test "Special chars" "Line one\nLine two\nLine three" 0

echo ""
echo "============================================"
echo " Results: $PASS passed, $FAIL failed"
echo "============================================"

exit $FAIL
