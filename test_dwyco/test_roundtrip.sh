#!/bin/bash
# Round-trip message test coordinator
# Requires local servers with test accounts UserA and UserB
#
# Usage: ./test_roundtrip.sh <user_a_dir> <user_a_uid_hex> <user_b_dir> <user_b_uid_hex>
#   UIDs must be in hex format (lowercase, no separators)
#
# Covers simple text messages plus attachments:
#   - both clients online at the same time (direct sends)
#   - clients NOT online at the same time (exercise the server store & forward)
#   - small attachments (<10k, inline) and large attachments (>1MB, out of line)

set -e

if [ $# -lt 4 ]; then
    echo "Usage: $0 <user_a_dir> <user_a_uid_hex> <user_b_dir> <user_b_uid_hex>"
    echo ""
    echo "  Runs round-trip message tests: UserB sends to UserA, UserA receives and verifies."
    echo "  Requires local servers with both accounts registered and pals."
    exit 1
fi

USERA_DIR="$1"
USERA_UID="$2"
USERB_DIR="$3"
USERB_UID="$4"
COORD_FILE="/tmp/dwytest_roundtrip_coord.$$"
RECV_LOG="/tmp/dwytest_roundtrip_recv.$$.log"
SEND_LOG="/tmp/dwytest_roundtrip_send.$$.log"
SMALL_ATT="/tmp/dwytest_small_att.$$.bin"
LARGE_ATT="/tmp/dwytest_large_att.$$.bin"
PEER_BIN="${PEER_BIN:-./dwytest_peer}"

cleanup() {
    rm -f "$COORD_FILE" "$RECV_LOG" "$SEND_LOG" "$SMALL_ATT" "$LARGE_ATT"
}
trap cleanup EXIT

# Small attachment: under 10k so the direct-send path delivers it inline.
# Large attachment: over 1 megabyte so the direct-send path transfers the
# file out of line on a separate channel.
make_attachments() {
    # deterministic-ish content; the size/hash travel in the coord file so
    # the receiver can verify the copied-out bytes match exactly.
    python3 - "$SMALL_ATT" "$LARGE_ATT" <<'EOF'
import sys
small = open(sys.argv[1], "wb")
small.write(bytes((i * 7 + i // 251) & 0xff for i in range(9000)))
small.close()
large = open(sys.argv[2], "wb")
pat = bytes(range(256)) * 4300 + b"dwyco out-of-line tail"
large.write(pat)
large.close()
EOF
}

PASS=0
FAIL=0

# Direct send: start the receiver first so both clients are online when the
# sender fires. Optional trailing args are passed to the sender
# (attachment path).
run_direct_test() {
    local name="$1"
    local text="$2"
    local no_forward="${3:-0}"
    local delay="${4:-3}"
    local att="${5:-}"

    echo ""
    echo "=== $name (both clients online) ==="
    echo "  Text: '$text'"
    echo "  No forward: $no_forward"
    if [ -n "$att" ]; then
        echo "  Attachment: $att ($(stat -c %s "$att") bytes)"
    else
        echo "  Attachment: none"
    fi

    rm -f "$COORD_FILE"

    # Receiver must be online when the message is sent (messages are
    # only delivered to a connected peer), so start it first. It waits
    # for the coord file, then polls for the incoming message.
    echo "--- Starting receiver ---"
    "$PEER_BIN" recv "$USERA_DIR" "$COORD_FILE" "$USERB_UID" > "$RECV_LOG" 2>&1 &
    RECV_PID=$!

    echo "--- Waiting ${delay}s for receiver to come online ---"
    sleep "$delay"

    echo "--- Sending ---"
    if ! "$PEER_BIN" send "$USERB_DIR" "$COORD_FILE" "$USERA_UID" "$text" "$no_forward" "$att" > "$SEND_LOG" 2>&1; then
        cat "$SEND_LOG"
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

# Server send: the receiver is OFFLINE while the sender sends, so the
# message must ride the server (store & forward). The receiver is started
# afterward and fetches the queued message from the server.
run_server_test() {
    local name="$1"
    local text="$2"
    local att="${3:-}"

    echo ""
    echo "=== $name (receiver offline, server store & forward) ==="
    echo "  Text: '$text'"
    if [ -n "$att" ]; then
        echo "  Attachment: $att ($(stat -c %s "$att") bytes)"
    else
        echo "  Attachment: none"
    fi

    rm -f "$COORD_FILE"

    echo "--- Sending while receiver is offline ---"
    if ! "$PEER_BIN" send "$USERB_DIR" "$COORD_FILE" "$USERA_UID" "$text" 0 "$att" > "$SEND_LOG" 2>&1; then
        cat "$SEND_LOG"
        echo "FAIL: send failed"
        FAIL=$((FAIL + 1))
        return
    fi

    echo "--- Starting receiver (fetches queued message from server) ---"
    "$PEER_BIN" recv "$USERA_DIR" "$COORD_FILE" "$USERB_UID" > "$RECV_LOG" 2>&1 &
    RECV_PID=$!

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

make_attachments

echo ""
echo "----------------"
echo " Simple text"
echo "----------------"
run_direct_test "Simple text" "Hello from UserB"
run_direct_test "No forward" "Secret message" 1
run_direct_test "Multi-word text" "The quick brown fox jumps over the lazy dog" 0
run_direct_test "Numeric text" "12345 67890 111213" 0
run_direct_test "Special chars" "Line one\nLine two\nLine three" 0

echo ""
echo "----------------"
echo " Attachments"
echo "----------------"
run_direct_test "Direct inline small attach" "small direct attachment" 0 3 "$SMALL_ATT"
run_direct_test "Direct out-of-line large attach" "large direct attachment" 0 3 "$LARGE_ATT"
run_server_test "Server small attach" "small via server" "$SMALL_ATT"
run_server_test "Server large attach" "large via server" "$LARGE_ATT"

echo ""
echo "============================================"
echo " Results: $PASS passed, $FAIL failed"
echo "============================================"

exit $FAIL