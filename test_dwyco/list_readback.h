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

// ===== column-addressed readback =====
//
// The helpers above use DWYCO_NO_COLUMN, which is only correct for the
// flat single-column lists these tests build with dwyco_list_append.
//
// Multi-column lists cannot be constructed through the public API -- there
// is no dwyco_list_append_col -- so they only ever come out of the real API
// (dwyco_uid_to_info, dwyco_get_server_list, ...). These variants read them
// by the "000"/"001" column-name constants the header defines.
//
// NOTE: never call dwyco_list_get with an out-of-range row. It calls
// oopanic(), which is [[noreturn]] and exits(1) -- see the
// oob_row_exits test in dwytest_list.cpp, which has to run in a subprocess
// for exactly this reason.

// Assert the reported column count. Note dwyco_list_numelems reports -1
// when the list is empty or when row 0 is not itself a vector, which is the
// normal result for a flat list built by dwyco_list_append.
static int
lr_cols(DWYCO_LIST l, int expected_cols)
{
    int r = -1, c = -1;
    if (dwyco_list_numelems(l, &r, &c) != 1) {
        printf("[FAIL] dwyco_list_numelems failed\n");
        return 0;
    }
    if (c != expected_cols) {
        printf("[FAIL] cols=%d expected=%d\n", c, expected_cols);
        return 0;
    }
    return 1;
}

static int
lr_col_str(DWYCO_LIST l, int row, const char *col,
    const char *expected, int expected_len)
{
    const char *val;
    int len, type;
    if (!dwyco_list_get(l, row, col, &val, &len, &type)) {
        printf("[FAIL] dwyco_list_get row %d col %s failed\n", row, col);
        return 0;
    }
    if (type != DWYCO_TYPE_STRING) {
        printf("[FAIL] row %d col %s type=%d expected DWYCO_TYPE_STRING\n",
            row, col, type);
        return 0;
    }
    if (len != expected_len ||
        memcmp(val, expected, (size_t)expected_len) != 0) {
        printf("[FAIL] row %d col %s value='%.*s'(%d) expected='%.*s'(%d)\n",
            row, col, len, val, len, expected_len, expected, expected_len);
        return 0;
    }
    return 1;
}

static int
lr_col_int(DWYCO_LIST l, int row, const char *col, int expected)
{
    const char *val;
    int len, type;
    if (!dwyco_list_get(l, row, col, &val, &len, &type)) {
        printf("[FAIL] dwyco_list_get row %d col %s failed\n", row, col);
        return 0;
    }
    if (type != DWYCO_TYPE_INT) {
        printf("[FAIL] row %d col %s type=%d expected DWYCO_TYPE_INT\n",
            row, col, type);
        return 0;
    }
    char expected_str[64];
    snprintf(expected_str, sizeof(expected_str), "%d", expected);
    if (len != (int)strlen(expected_str) ||
        strncmp(val, expected_str, (size_t)len) != 0) {
        printf("[FAIL] row %d col %s value='%.*s'(%d) expected=%d\n",
            row, col, len, val, len, expected);
        return 0;
    }
    return 1;
}

static int
lr_col_nil(DWYCO_LIST l, int row, const char *col)
{
    const char *val;
    int len, type;
    if (!dwyco_list_get(l, row, col, &val, &len, &type)) {
        printf("[FAIL] dwyco_list_get row %d col %s failed\n", row, col);
        return 0;
    }
    if (type != DWYCO_TYPE_NIL) {
        printf("[FAIL] row %d col %s type=%d expected DWYCO_TYPE_NIL\n",
            row, col, type);
        return 0;
    }
    return 1;
}

#endif