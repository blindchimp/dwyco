#ifndef DWYCO_TEST_LIST_READBACK_H
#define DWYCO_TEST_LIST_READBACK_H

#include <dlli.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

// Read-back helpers for the standalone list tests. All return 1 on
// success (value read back and matching), 0 on any mismatch, printing
// a [FAIL] diagnostic. Lists are single-column, so DWYCO_NO_COLUMN is
// used for every dwyco_list_get call.

static int
lr_rows(DWYCO_LIST l, int expected_rows)
{
    int r = -1, c = -1;
    if (dwyco_list_numelems(l, &r, &c) != 1) {
        printf("[FAIL] dwyco_list_numelems failed\n");
        return 0;
    }
    if (r != expected_rows) {
        printf("[FAIL] rows=%d expected=%d\n", r, expected_rows);
        return 0;
    }
    return 1;
}

static int
lr_int(DWYCO_LIST l, int row, int expected)
{
    const char *val;
    int len, type;
    if (!dwyco_list_get(l, row, DWYCO_NO_COLUMN, &val, &len, &type)) {
        printf("[FAIL] dwyco_list_get row %d failed\n", row);
        return 0;
    }
    if (type != DWYCO_TYPE_INT) {
        printf("[FAIL] row %d type=%d expected DWYCO_TYPE_INT\n", row, type);
        return 0;
    }
    char expected_str[64];
    snprintf(expected_str, sizeof(expected_str), "%d", expected);
    if (len != (int)strlen(expected_str) ||
        strncmp(val, expected_str, (size_t)len) != 0) {
        printf("[FAIL] row %d value='%.*s'(%d) expected=%d\n",
            row, len, val, len, expected);
        return 0;
    }
    return 1;
}

static int
lr_str(DWYCO_LIST l, int row, const char *expected, int expected_len)
{
    const char *val;
    int len, type;
    if (!dwyco_list_get(l, row, DWYCO_NO_COLUMN, &val, &len, &type)) {
        printf("[FAIL] dwyco_list_get row %d failed\n", row);
        return 0;
    }
    if (type != DWYCO_TYPE_STRING) {
        printf("[FAIL] row %d type=%d expected DWYCO_TYPE_STRING\n", row, type);
        return 0;
    }
    if (len != expected_len ||
        memcmp(val, expected, (size_t)expected_len) != 0) {
        printf("[FAIL] row %d value='%.*s'(%d) expected='%.*s'(%d)\n",
            row, len, val, len, expected_len, expected, expected_len);
        return 0;
    }
    return 1;
}

static int
lr_nil(DWYCO_LIST l, int row)
{
    const char *val;
    int len, type;
    if (!dwyco_list_get(l, row, DWYCO_NO_COLUMN, &val, &len, &type)) {
        printf("[FAIL] dwyco_list_get row %d failed\n", row);
        return 0;
    }
    if (type != DWYCO_TYPE_NIL) {
        printf("[FAIL] row %d type=%d expected DWYCO_TYPE_NIL\n", row, type);
        return 0;
    }
    return 1;
}

#endif