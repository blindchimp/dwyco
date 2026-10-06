// Coverage for the DWYCO_LIST serialization and copy API.
//
// The other list tests in this directory (test_lists_12,
// test_append_operations, test_creation_destruction, list_helpers_test,
// list_test_suite_part1) cover creation, release and append thoroughly, but
// only ever build flat single-column lists, and they never touch these four
// functions at all:
//
//   dwyco_list_print, dwyco_list_copy, dwyco_list_to_string,
//   dwyco_list_from_string
//
// So this binary exists to cover those, plus the type/column-count behavior
// of the accessors that the existing tests do not pin down.
//
// No account and no network: dwyco_list_* is pure data-structure API.

#include <dlli.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

#include "test_common.h"
#include "list_readback.h"

static int g_pass = 0;
static int g_fail = 0;

// argv[0], needed so the out-of-range test can re-exec this binary.
static const char *g_argv0 = 0;

#define RUN(name) do { \
    printf("  %-40s ", #name); \
    int before = g_fail; \
    name(); \
    if (g_fail == before) { printf("OK\n"); g_pass++; } \
} while (0)

#define CHECK(cond) do { \
    if (!(cond)) { \
        printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        g_fail++; \
    } \
} while (0)

// ===== dwyco_list_numelems =====

// A flat list built with dwyco_list_append reports cols == -1, not 1: the
// implementation only reports a real column count when row 0 is itself a
// vector. An empty list reports rows == 0 and cols == -1. Pin both, because
// callers that assume cols is 1 for flat lists are wrong.
static void
flat_list_reports_minus_one_col(void)
{
    DWYCO_LIST l = dwyco_list_new();
    CHECK(l != 0);
    if (!l) return;

    CHECK(lr_rows(l, 0));
    CHECK(lr_cols(l, -1));

    dwyco_list_append_int(l, 7);
    CHECK(lr_rows(l, 1));
    CHECK(lr_cols(l, -1));
    CHECK(lr_int(l, 0, 7));

    dwyco_list_release(l);
}

// ===== dwyco_list_append =====

// dwyco_list_append(..., DWYCO_TYPE_STRING) builds a VC_BSTRING, so it is
// length-based: embedded NULs and high bytes must survive. This is the
// property AGENTS.md calls out -- VC_BSTRING is only meaningful during
// construction, and on the way out the type reads back as DWYCO_TYPE_STRING.
static void
append_string_is_binary_safe(void)
{
    // "a\0b\xff\x00" -- NUL in the middle and a high byte
    static const char raw[] = { 'a', 0, 'b', (char)0xff, 0 };
    const int raw_len = 5;

    DWYCO_LIST l = dwyco_list_new();
    CHECK(l != 0);
    if (!l) return;

    dwyco_list_append(l, raw, raw_len, DWYCO_TYPE_STRING);
    CHECK(lr_rows(l, 1));
    CHECK(lr_str(l, 0, raw, raw_len));

    // A zero-length string is still a string, not nil.
    dwyco_list_append(l, "", 0, DWYCO_TYPE_STRING);
    CHECK(lr_rows(l, 2));
    CHECK(lr_str(l, 1, "", 0));

    dwyco_list_release(l);
}

// DWYCO_TYPE_NIL appends a nil element.
static void
append_nil(void)
{
    DWYCO_LIST l = dwyco_list_new();
    CHECK(l != 0);
    if (!l) return;

    dwyco_list_append(l, 0, 0, DWYCO_TYPE_NIL);
    CHECK(lr_rows(l, 1));
    CHECK(lr_nil(l, 0));

    dwyco_list_release(l);
}

// DWYCO_TYPE_INT takes the value as a decimal ASCII string, unlike
// dwyco_list_append_int which takes an int. This is a separate code path
// in the library (vc(VC_INT, val, 0) parses the string), so cover it,
// including a negative value.
static void
append_int_from_string(void)
{
    DWYCO_LIST l = dwyco_list_new();
    CHECK(l != 0);
    if (!l) return;

    dwyco_list_append(l, "42", 2, DWYCO_TYPE_INT);
    dwyco_list_append(l, "-7", 2, DWYCO_TYPE_INT);
    dwyco_list_append(l, "0", 1, DWYCO_TYPE_INT);
    CHECK(lr_rows(l, 3));
    CHECK(lr_int(l, 0, 42));
    CHECK(lr_int(l, 1, -7));
    CHECK(lr_int(l, 2, 0));

    dwyco_list_release(l);
}

// ===== dwyco_list_copy =====

// The copy must be independent: appending to it must not disturb the
// original.
static void
copy_is_independent(void)
{
    DWYCO_LIST orig = dwyco_list_new();
    CHECK(orig != 0);
    if (!orig) return;
    dwyco_list_append_int(orig, 1);
    dwyco_list_append_int(orig, 2);

    DWYCO_LIST copy = dwyco_list_copy(orig);
    CHECK(copy != 0);
    if (!copy) {
        dwyco_list_release(orig);
        return;
    }

    CHECK(lr_rows(copy, 2));
    CHECK(lr_int(copy, 0, 1));
    CHECK(lr_int(copy, 1, 2));

    // Mutate the copy only.
    dwyco_list_append_int(copy, 99);
    CHECK(lr_rows(copy, 3));
    CHECK(lr_int(copy, 2, 99));

    // Original is untouched.
    CHECK(lr_rows(orig, 2));
    CHECK(lr_int(orig, 0, 1));
    CHECK(lr_int(orig, 1, 2));

    dwyco_list_release(copy);
    dwyco_list_release(orig);
}

// Copying an empty list, and copying binary payloads, are both plausible
// places for an implementation to fall over.
static void
copy_empty_and_binary(void)
{
    DWYCO_LIST empty = dwyco_list_new();
    DWYCO_LIST empty_copy = dwyco_list_copy(empty);
    CHECK(empty_copy != 0);
    if (empty_copy) {
        CHECK(lr_rows(empty_copy, 0));
        dwyco_list_release(empty_copy);
    }
    dwyco_list_release(empty);

    static const char raw[] = { 0x00, (char)0x80, 0x00, (char)0xff, 'z' };
    DWYCO_LIST bin = dwyco_list_new();
    dwyco_list_append(bin, raw, 5, DWYCO_TYPE_STRING);
    dwyco_list_append(bin, "tail", 4, DWYCO_TYPE_STRING);

    DWYCO_LIST bin_copy = dwyco_list_copy(bin);
    CHECK(bin_copy != 0);
    if (bin_copy) {
        CHECK(lr_rows(bin_copy, 2));
        CHECK(lr_str(bin_copy, 0, raw, 5));
        CHECK(lr_str(bin_copy, 1, "tail", 4));
        dwyco_list_release(bin_copy);
    }
    dwyco_list_release(bin);
}

// Releasing the copy must not invalidate the original, and vice versa.
static void
copy_release_order(void)
{
    DWYCO_LIST a = dwyco_list_new();
    DWYCO_LIST b = dwyco_list_copy(a);
    CHECK(b != 0);
    if (!b) {
        dwyco_list_release(a);
        return;
    }
    dwyco_list_release(a);
    // a is gone; b must still be readable and independently releasable.
    CHECK(lr_rows(b, 0));
    dwyco_list_release(b);
}

// ===== dwyco_list_print =====

// print() writes to VcError (stderr) and always returns 1. It is a debug
// aid, so the only thing worth asserting is that it does not crash and
// reports success. stderr noise is redirected to /dev/null for the duration.
static void
print_returns_one(void)
{
    DWYCO_LIST l = dwyco_list_new();
    CHECK(l != 0);
    if (!l) return;
    dwyco_list_append(l, "hello", 5, DWYCO_TYPE_STRING);
    dwyco_list_append_int(l, 3);

    fflush(stderr);
    int saved = dup(2);
    int null_fd = open("/dev/null", O_WRONLY);
    if (null_fd >= 0) {
        dup2(null_fd, 2);
    }
    int rc = dwyco_list_print(l);
    fflush(stderr);
    if (null_fd >= 0) {
        dup2(saved, 2);
        close(null_fd);
    }
    close(saved);

    CHECK(rc == 1);
    // Printing must not have disturbed the list.
    CHECK(lr_rows(l, 2));
    CHECK(lr_str(l, 0, "hello", 5));
    CHECK(lr_int(l, 1, 3));

    dwyco_list_release(l);
}

// ===== dwyco_list_to_string / dwyco_list_from_string =====

// Serialize a list, deserialize it, and check that every element came back
// with the same type and the same bytes. This is the round trip that makes
// to_string/from_string useful for persisting a list, so it has to be
// lossless for ints, nils, and binary strings.
static void
serialize_round_trip(void)
{
    static const char bin[] = { 'x', 0, (char)0xff, 0, 'y' };
    DWYCO_LIST l = dwyco_list_new();
    CHECK(l != 0);
    if (!l) return;

    dwyco_list_append_int(l, 0);
    dwyco_list_append(l, "", 0, DWYCO_TYPE_STRING);
    dwyco_list_append(l, "plain ascii", 11, DWYCO_TYPE_STRING);
    dwyco_list_append(l, bin, 5, DWYCO_TYPE_STRING);
    dwyco_list_append_int(l, -2147483647);
    dwyco_list_append(l, 0, 0, DWYCO_TYPE_NIL);
    dwyco_list_append_int(l, 2147483647);

    const char *ser;
    int ser_len;
    dwyco_list_to_string(l, &ser, &ser_len);
    // The serialized form is binary and can contain NULs, so it must be
    // measured with ser_len, not strlen.
    CHECK(ser != 0);
    CHECK(ser_len > 0);

    DWYCO_LIST back = 0;
    int rc = dwyco_list_from_string(&back, ser, ser_len);
    CHECK(rc != 0);
    if (rc && back) {
        CHECK(lr_rows(back, 7));
        CHECK(lr_int(back, 0, 0));
        CHECK(lr_str(back, 1, "", 0));
        CHECK(lr_str(back, 2, "plain ascii", 11));
        CHECK(lr_str(back, 3, bin, 5));
        CHECK(lr_int(back, 4, -2147483647));
        CHECK(lr_nil(back, 5));
        CHECK(lr_int(back, 6, 2147483647));
        dwyco_list_release(back);
    }

    // to_string allocated this with new[]; the header requires
    // dwyco_free_array (not dwyco_free) on it.
    dwyco_free_array((char *)ser);
    dwyco_list_release(l);
}

// An empty list still round-trips.
static void
serialize_empty_round_trip(void)
{
    DWYCO_LIST l = dwyco_list_new();
    CHECK(l != 0);
    if (!l) return;

    const char *ser;
    int ser_len;
    dwyco_list_to_string(l, &ser, &ser_len);
    CHECK(ser != 0);
    CHECK(ser_len > 0);

    DWYCO_LIST back = 0;
    int rc = dwyco_list_from_string(&back, ser, ser_len);
    CHECK(rc != 0);
    if (rc && back) {
        CHECK(lr_rows(back, 0));
        dwyco_list_release(back);
    }
    dwyco_free_array((char *)ser);
    dwyco_list_release(l);
}

// Round-tripping twice must be a fixed point: serialize(deserialize(x)) has
// to equal serialize(x) byte for byte, or repeated use would drift.
static void
serialize_round_trip_is_stable(void)
{
    DWYCO_LIST l = dwyco_list_new();
    CHECK(l != 0);
    if (!l) return;
    dwyco_list_append_int(l, 11);
    dwyco_list_append(l, "z", 1, DWYCO_TYPE_STRING);
    dwyco_list_append(l, 0, 0, DWYCO_TYPE_NIL);

    const char *ser1;
    int len1;
    dwyco_list_to_string(l, &ser1, &len1);
    CHECK(len1 > 0);

    DWYCO_LIST back = 0;
    CHECK(dwyco_list_from_string(&back, ser1, len1) != 0);
    if (back) {
        const char *ser2;
        int len2;
        dwyco_list_to_string(back, &ser2, &len2);
        CHECK(len2 == len1);
        if (len2 == len1)
            CHECK(memcmp(ser1, ser2, (size_t)len1) == 0);
        dwyco_free_array((char *)ser2);
        dwyco_list_release(back);
    }
    dwyco_free_array((char *)ser1);
    dwyco_list_release(l);
}

// from_string must reject input that is not a serialized list, and must
// leave list_out completely untouched when it does -- a caller that
// initializes the pointer and checks the return code should never be
// handed a half-initialized value.
//
// Note the serialized form is NOT printable text: it ends with a type byte,
// so it contains NULs/high bytes. That is why every use of str_out from
// dwyco_list_to_string in this file goes through ser_len.
static void
from_string_rejects_garbage(void)
{
    static const char junk[] = "this is definitely not a serialized dwyco vc";
    // Poison the out pointer so we can tell whether it got written to.
    DWYCO_LIST out = (DWYCO_LIST)&from_string_rejects_garbage;

    int rc = dwyco_list_from_string(&out, junk, (int)strlen(junk));
    CHECK(rc == 0);
    CHECK(out == (DWYCO_LIST)&from_string_rejects_garbage);

    // Same for a binary blob that is not a serialized vc.
    static const char blob[] = { (char)0xff, 0x00, 0x01, (char)0x80 };
    DWYCO_LIST out2 = (DWYCO_LIST)&from_string_rejects_garbage;
    int rc2 = dwyco_list_from_string(&out2, blob, 4);
    CHECK(rc2 == 0);
    CHECK(out2 == (DWYCO_LIST)&from_string_rejects_garbage);
}

// A serialized list must be measured by length, not strlen: the encoding
// embeds NULs. Round-tripping a list that itself contains an embedded NUL
// proves the length is carried through, not inferred.
static void
serialize_carries_length(void)
{
    static const char with_nul[] = { 'a', 0, 'b' };
    DWYCO_LIST l = dwyco_list_new();
    CHECK(l != 0);
    if (!l) return;
    dwyco_list_append(l, with_nul, 3, DWYCO_TYPE_STRING);

    const char *ser;
    int ser_len;
    dwyco_list_to_string(l, &ser, &ser_len);
    CHECK(ser != 0);
    // This is the whole point: ser_len must exceed strlen(ser).
    CHECK(ser_len > (int)strlen(ser));
    printf("\n      ser_len=%d strlen=%d\n", ser_len, (int)strlen(ser));

    DWYCO_LIST back = 0;
    CHECK(dwyco_list_from_string(&back, ser, ser_len) != 0);
    if (back) {
        CHECK(lr_rows(back, 1));
        CHECK(lr_str(back, 0, with_nul, 3));
        dwyco_list_release(back);
    }
    dwyco_free_array((char *)ser);
    dwyco_list_release(l);
}

// ===== out-of-range access =====

// dwyco_list_get calls oopanic() for a row past the end, which is
// [[noreturn]] and exits(1) from inside the library. That is an intentional
// guard ("don't allow indexing past the end of dwyco list"), so it has to be
// verified out-of-process -- there is no way to catch exit(1) in-process.
//
// Re-execs this binary with --oob, then checks the child exited 1.
static void
oob_row_in_child(void)
{
    DWYCO_LIST l = dwyco_list_new();
    dwyco_list_append_int(l, 1);
    const char *val;
    int len, type;
    dwyco_list_get(l, 99, DWYCO_NO_COLUMN, &val, &len, &type);
    // Should never get here.
    printf("[FAIL] out-of-range dwyco_list_get returned instead of panicking\n");
    dwyco_list_release(l);
    exit(2);
}

static void
oob_row_exits(void)
{
    const char *self = g_argv0;
    if (!self) {
        printf("[FAIL] no argv[0] recorded for subprocess test\n");
        g_fail++;
        return;
    }
    char *argv[] = { (char *)self, (char *)"--oob", 0 };
    int status = run_subprocess(argv);
    // oopanic() does exit(1); a signal death would be 128+n.
    CHECK(status == 1);
}

int
main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--oob") == 0) {
        oob_row_in_child();
        return 3;
    }
    g_argv0 = argv[0];

    printf("Dwyco list serialization / copy / print\n");

    printf("\nAccessors:\n");
    RUN(flat_list_reports_minus_one_col);

    printf("\nAppend:\n");
    RUN(append_string_is_binary_safe);
    RUN(append_nil);
    RUN(append_int_from_string);

    printf("\nCopy:\n");
    RUN(copy_is_independent);
    RUN(copy_empty_and_binary);
    RUN(copy_release_order);

    printf("\nPrint:\n");
    RUN(print_returns_one);

    printf("\nSerialize:\n");
    RUN(serialize_round_trip);
    RUN(serialize_empty_round_trip);
    RUN(serialize_round_trip_is_stable);
    RUN(serialize_carries_length);
    RUN(from_string_rejects_garbage);

    printf("\nOut-of-range:\n");
    RUN(oob_row_exits);

    printf("\nSummary: passed=%d failed=%d\n", g_pass, g_fail);
    if (g_fail) {
        printf("FAILED\n");
        return 1;
    }
    printf("All list tests passed.\n");
    return 0;
}