#!/bin/bash
# Round-trip message test coordinator
# Requires local servers with test accounts UserA and UserB
#
# Usage: ./test_roundtrip.sh <user_a_dir> <user_a_uid_hex> <user_b_dir> <user_b_uid_hex>
#   UIDs must be in hex format (lowercase, no separators)
#
# Covers simple text messages plus attachments:
#   - both clients started together; they rendezvous by polling each
#     other's online status (dwyco_uid_online) before exchanging
#   - small attachments (<10k, inline) and large attachments (>1MB,
#     out of line)

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
RECV_LOG="/tmp/dwytest_roundtrip_recv.$$.log"
SEND_LOG="/tmp/dwytest_roundtrip_send.$$.log"
SMALL_ATT="/tmp/dwytest_small_att.$$.bin"
LARGE_ATT="/tmp/dwytest_large_att.$$.bin"
PEER_BIN="${PEER_BIN:-./dwytest_peer}"

cleanup() {
    rm -f "$RECV_LOG" "$SEND_LOG" "$SMALL_ATT" "$LARGE_ATT"
}
trap cleanup EXIT

# Small attachment: under 10k so the direct-send path delivers it inline.
# Large attachment: over 1 megabyte so the direct-send path transfers the
# file out of line on a separate channel.
make_attachments() {
    # deterministic-ish content
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

# Both clients are started together; they rendezvous by polling each
# other's online status (via dwyco_uid_online) rather than using a coord
# file or a fixed sleep. The sender waits for the receiver to come online
# before firing; the receiver waits for the sender before polling for the
# incoming message. Optional trailing args are passed to the sender
# (no_forward, attachment path).
run_roundtrip_test() {
    local name="$1"
    local text="$2"
    local no_forward="${3:-0}"
    local att="${4:-}"

    echo ""
    echo "=== $name ==="
    echo "  Text: '$text'"
    echo "  No forward: $no_forward"
    if [ -n "$att" ]; then
        echo "  Attachment: $att ($(stat -c %s "$att") bytes)"
    else
        echo "  Attachment: none"
    fi

    echo "--- Starting receiver ---"
    "$PEER_BIN" recv "$USERA_DIR" "$USERB_UID" > "$RECV_LOG" 2>&1 &
    RECV_PID=$!

    echo "--- Starting sender ---"
    if ! "$PEER_BIN" send "$USERB_DIR" "$USERA_UID" "$text" "$no_forward" "$att" > "$SEND_LOG" 2>&1; then
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
run_roundtrip_test "Simple text" "Hello from UserB"
run_roundtrip_test "No forward" "Secret message" 1
run_roundtrip_test "Multi-word text" "The quick brown fox jumps over the lazy dog" 0
run_roundtrip_test "Numeric text" "12345 67890 111213" 0
run_roundtrip_test "Special chars" "Line one\nLine two\nLine three" 0

echo ""
echo "----------------"
echo " Attachments"
echo "----------------"
run_roundtrip_test "Inline small attach" "small attachment" 0 "$SMALL_ATT"
run_roundtrip_test "Out-of-line large attach" "large attachment" 0 "$LARGE_ATT"

echo ""
echo "============================================"
echo " Results: $PASS passed, $FAIL failed"
echo "============================================"

exit $FAIL