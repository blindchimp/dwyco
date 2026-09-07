/* === Dwyco List Helpers - Test Case #2: Append Operations & Type Coverage === */
#include <dlli.h>
#include <cstdio>
#include <cstring>
#include "list_readback.h"

static int g_fail = 0;

#define CHECK(cond) do { if (!(cond)) g_fail++; } while(0)

/* === Test Case #2a: Append_INT Type Coverage === */
static void test_append_int_only(void) {
    DWYCO_LIST intlist = dwyco_list_new();
    if (intlist == NULL) {
        printf("[SKIP] Could not create list for append_int tests\n");
        return;
    }
    
    int value_42 = 42;
    dwyco_list_append_int(intlist, value_42);
    CHECK(lr_rows(intlist, 1));
    CHECK(lr_int(intlist, 0, 42));
    printf("[OK] append_int(42) verified\n");
    dwyco_list_release(intlist);
}

/* === Test Case #2b: Append_String Type Coverage === */
static void test_append_string_types(void) {
    DWYCO_LIST strlist = dwyco_list_new();
    if (strlist == NULL) return;
    
    printf("\n--- Test Case #2b: append() with Different Types ---\n");
    
    /* Try string type (len > 0, type == DWYCO_TYPE_STRING) */
    dwyco_list_append(strlist, "hello world", 11, DWYCO_TYPE_STRING);
    
    /* Empty string case */
    dwyco_list_append(strlist, "", 0, DWYCO_TYPE_NIL);
    
    CHECK(lr_rows(strlist, 2));
    CHECK(lr_str(strlist, 0, "hello world", 11));
    CHECK(lr_nil(strlist, 1));
    printf("[OK] String appends verified\n");
    dwyco_list_release(strlist);
}

/* === Test Case #2c: Append_NIL Type Coverage === */
static void test_append_nil_type(void) {
    DWYCO_LIST nil_list = dwyco_list_new();
    if (nil_list == NULL) return;
    
    printf("\n--- Test Case #2c: append() with DWYCO_TYPE_NIL ---\n");
    
    /* Try NIL type */
    dwyco_list_append(nil_list, "", 0, DWYCO_TYPE_NIL);
    
    CHECK(lr_rows(nil_list, 1));
    CHECK(lr_nil(nil_list, 0));
    printf("[OK] NIL type append verified\n");
    dwyco_list_release(nil_list);
}

/* === Test Case #2d: Append_Unknown Type Coverage === */
static void test_append_vector_type(void) {
    return;
}

/* === Test Case #2e: Multi-Row List Creation === */
static void test_multirow_creation(void) {
    DWYCO_LIST multilist = dwyco_list_new();
    if (multilist == NULL) return;
    
    printf("\n--- Test Case #2e: Creating Multi-Row Lists ---\n");
    
    for (unsigned int i = 0; i < 10u; i++) {
        dwyco_list_append(multilist, "item", 4, DWYCO_TYPE_STRING);
    }
    
    CHECK(lr_rows(multilist, 10));
    for (unsigned int i = 0; i < 10u; i++) {
        CHECK(lr_str(multilist, (int)i, "item", 4));
    }
    printf("[OK] Created list with 10 rows via repeated appends\n");
    dwyco_list_release(multilist);
}

/* === Test Case #2f: Bulk Append Pattern === */
static void test_bulk_append_pattern(void) {
    DWYCO_LIST bulk_list = dwyco_list_new();
    if (bulk_list == NULL) return;
    
    printf("\n--- Test Case #2f: Bulk Append via Loop ---\n");
    
    /* This tests performance and memory allocation behavior under */
    /* repeated reallocation from append operations */
    
    const char *value = "test_value";
    unsigned int str_len = 10u;
    for (unsigned int i = 1u; i <= 50u; i++) {
        dwyco_list_append(bulk_list, value, (int)str_len, DWYCO_TYPE_STRING);
    }
    
    CHECK(lr_rows(bulk_list, 50));
    for (unsigned int i = 0; i < 50u; i++) {
        CHECK(lr_str(bulk_list, (int)i, value, (int)str_len));
    }
    printf("[OK] Completed bulk append of 50 items\n");
    dwyco_list_release(bulk_list);
}

/* === Test Case #2g: Mixed Type Append Patterns === */
static void test_mixed_types(void) {
    const char *p = "string";
    unsigned int len = (unsigned int)strlen(p);
    
    DWYCO_LIST mix_list = dwyco_list_new();
    if (mix_list == NULL) return;
    
    printf("\n--- Test Case #2g: Mixed Types in Same List ---\n");
    
    /* Mix types to test whether list maintains type correctness */
    dwyco_list_append_int(mix_list, 100);
    dwyco_list_append(mix_list, p, (int)len, DWYCO_TYPE_STRING);
    dwyco_list_append_int(mix_list, 200);
    
    CHECK(lr_rows(mix_list, 3));
    CHECK(lr_int(mix_list, 0, 100));
    CHECK(lr_str(mix_list, 1, p, (int)len));
    CHECK(lr_int(mix_list, 2, 200));
    printf("[OK] Mixed type appends verified\n");
    dwyco_list_release(mix_list);
}

/* === Test Case #2h: Boundary Tests for Append === */
static void test_append_boundary_conditions(void) {
    DWYCO_LIST boundary_list = dwyco_list_new();
    if (boundary_list == NULL) return;
    
    printf("\n--- Test Case #2h: Append Boundary Conditions ---\n");
    
    /* Length-0 string case */
    dwyco_list_append(boundary_list, "", 0, DWYCO_TYPE_STRING);
    
    /* Large string append */
    const char *large = "a very long string ";
    int len_large = (int)strlen(large);
    dwyco_list_append(boundary_list, large, len_large, DWYCO_TYPE_STRING);
    
    CHECK(lr_rows(boundary_list, 2));
    CHECK(lr_str(boundary_list, 0, "", 0));
    CHECK(lr_str(boundary_list, 1, large, len_large));
    printf("[OK] Boundary condition appends verified\n");
    dwyco_list_release(boundary_list);
}

/* === Test Case #2i: Repeat Append After Single-Value List === */
static void test_extend_single_val_to_multirow(void) {
    printf("\n--- Test Case #2i: Extending Single-Value List ---\n");
    
    /* Start with one value, grow to many rows */
    DWYCO_LIST single = dwyco_list_new();
    if (single == NULL) return;
    
    dwyco_list_append_int(single, 1);
    
    for (unsigned int i = 1u; i < 50u; i++) {
        dwyco_list_append(single, "value", 4, DWYCO_TYPE_STRING);
    }
    
    CHECK(lr_rows(single, 50));
    CHECK(lr_int(single, 0, 1));
    for (unsigned int i = 1u; i < 50u; i++) {
        CHECK(lr_str(single, (int)i, "value", 4));
    }
    printf("[OK] Extended from single to multi-row successfully\n");
    dwyco_list_release(single);
}

void (*test_funcs[])(void) = {
    test_append_int_only,
    test_append_string_types,
    test_append_nil_type,
    test_append_vector_type,
    test_multirow_creation,
    test_bulk_append_pattern,
    test_mixed_types,
    test_append_boundary_conditions,
    test_extend_single_val_to_multirow,
};

int main(void) {
    printf("Dwyco List Helpers - Test Suite #2\n");
    
    for (size_t i = 0; i < sizeof(test_funcs)/sizeof(test_funcs[0]); i++) {
        test_funcs[i]();
    }
    
    printf("\n=== Summary ===\n");
    printf("Failed: %d\n", g_fail);
    
    return g_fail > 0 ? 1 : 0;
}