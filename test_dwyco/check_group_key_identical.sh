#!/bin/bash
# Check that the dhg.sql private_key records are identical for all 500 clients
# in /tmp/dwy0 - /tmp/dwy499.
#
# The `keys` table may have multiple rows; we only compare privkey values for
# rows whose gname (alt_name) matches the requested group name. Clients whose
# dhg.sql is absent are reported as MISSING (not compared). Clients that have
# no matching gname row are reported as NO_MATCHING_ROW (also not identical).
set -e

GNAME="${1:-barfybarf}"

GOOD=0
DIFF=0
MISSING=0
NOMATCH=0
REF=""

for i in $(seq 0 499); do
    db="/tmp/dwy${i}/dhg.sql"
    if [ ! -f "$db" ]; then
        echo "MISSING_DB  $i ($db)"
        MISSING=$((MISSING + 1))
        continue
    fi
    # privkey values (hex) for rows whose gname matches; multiple matching
    # rows are kept in rowid order and compared as a set
    val=$(sqlite3 "$db" "select hex(privkey) from keys where alt_name='$GNAME' order by rowid;" | tr -d '\n')
    if [ -z "$val" ]; then
        echo "NO_MATCHING_ROW  $i (gname=$GNAME)"
        NOMATCH=$((NOMATCH + 1))
        continue
    fi
    if [ -z "$REF" ]; then
        REF="$val"
    fi
    if [ "$val" = "$REF" ]; then
        GOOD=$((GOOD + 1))
    else
        echo "DIFF  $i"
        DIFF=$((DIFF + 1))
    fi
done

echo "----------------------------------------"
echo "identical:         $GOOD"
echo "different:         $DIFF"
echo "missing db:        $MISSING"
echo "no matching gname: $NOMATCH"
if [ "$DIFF" -eq 0 ] && [ "$MISSING" -eq 0 ] && [ "$NOMATCH" -eq 0 ]; then
    echo "RESULT: all 500 clients have identical private_key records for gname=$GNAME"
    exit 0
else
    echo "RESULT: NOT all identical"
    exit 1
fi
