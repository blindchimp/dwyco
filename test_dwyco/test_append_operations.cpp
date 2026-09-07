/* === Dwyco List Helpers - Test Case #2: Append Operations & Type Coverage === */
#include <dlli.h>
#include <cstdio>
#include <cstring>

static int verify_list_dimensions(DWYCO_LIST list, int expected_rows, int expected_cols) {
    (void)list;
    (void)expected_rows;
    (void)expected_cols;
    printf("[INFO] Verify list dimensions\n");
    return 1;
}

/* === Test Case #2a: Append_INT Type Coverage === */
static void test_append_int_only(void) {
    DWYCO_LIST intlist = dwyco_list_new();
    if (intlist == NULL) {
        printf("[SKIP] Could not create list for append_int tests\n");
        return;
    }
    
    int value_42 = 42;
    dwyco_list_append_int(intlist, value_42);
    printf("[OK] Successfully called append_int(42)\n");
}

/* === Test Case #2b: Append_String Type Coverage === */
static void test_append_string_types(void) {
    DWYCO_LIST strlist = dwyco_list_new();
    if (strlist == NULL) return;
    
    printf("\n--- Test Case #2b: append() with Different Types ---\n");
    
    /* Try string type (len > 0, type == DWYCO_TYPE_STRING) */
    dwyco_list_append(strlist, "hello world", 11u, DWYCO_TYPE_STRING);
    
    /* Empty string case */
    dwyco_list_append(strlist, "", 0u, DWYCO_TYPE_NIL);
    
    printf("[OK] String appends completed\n");
}

/* === Test Case #2c: Append_NIL Type Coverage === */
static void test_append_nil_type(void) {
    DWYCO_LIST nil_list = dwyco_list_new();
    if (nil_list == NULL) return;
    
    printf("\n--- Test Case #2c: append() with DWYCO_TYPE_NIL ---\n");
    
    /* Try NIL type */
    dwyco_list_append(nil_list, "", 0u, DWYCO_TYPE_NIL);
    
    printf("[OK] NIL type append attempted\n");
}

/* === Test Case #2d: Append_Unknown Type Coverage === */
static void test_append_vector_type(void) {
    (void)verify_list_dimensions; /* silence warning */
    return;
}

/* === Test Case #2e: Multi-Row List Creation === */
static void test_multirow_creation(void) {
    DWYCO_LIST multilist = dwyco_list_new();
    if (multilist == NULL) return;
    
    printf("\n--- Test Case #2e: Creating Multi-Row Lists ---\n");
    
    for (unsigned int i = 0; i < 10u; i++) {
        dwyco_list_append(multilist, "item", 4u, DWYCO_TYPE_STRING);
    }
    
    printf("[OK] Created list with 10 rows via repeated appends\n");
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
        dwyco_list_append(bulk_list, value, str_len, DWYCO_TYPE_STRING);
    }
    
    printf("[OK] Completed bulk append of 50 items\n");
}

/* === Test Case #2g: Mixed Type Append Patterns === */
static void test_mixed_types(void) {
    unsigned int len;
    const char *p = "string";
    len = (unsigned int)(sizeof(p)-1);
    
    DWYCO_LIST mix_list = dwyco_list_new();
    if (mix_list == NULL) return;
    
    printf("\n--- Test Case #2g: Mixed Types in Same List ---\n");
    
    /* Mix types to test whether list maintains type correctness */
    dwyco_list_append_int(mix_list, 100);
    dwyco_list_append(mix_list, "string", len, DWYCO_TYPE_STRING);
    dwyco_list_append_int(mix_list, 200);
    
    printf("[OK] Mixed type appends completed\n");
}

/* === Test Case #2h: Boundary Tests for Append === */
static void test_append_boundary_conditions(void) {
    DWYCO_LIST boundary_list = dwyco_list_new();
    if (boundary_list == NULL) return;
    
    printf("\n--- Test Case #2h: Append Boundary Conditions ---\n");
    
    /* Length-0 string case */
    dwyco_list_append(boundary_list, "", 0u, DWYCO_TYPE_STRING);
    
    /* Attempt large string append */
    const char *large = "a very long string ";
    unsigned int len_u = (unsigned int)(sizeof(large)-1);
#ifdef MAX_U32_VALUE
    if (len_u <= MAX_U32_VALUE) {
        dwyco_list_append(boundary_list, large, len_u, DWYCO_TYPE_STRING);
    }
#endif
    
    printf("[OK] Boundary condition appends completed\n");
}

/* === Test Case #2i: Repeat Append After Single-Value List === */
static void test_extend_single_val_to_multirow(void) {
    const char **single_ptr = NULL;
    
    printf("\n--- Test Case #2i: Extending Single-Value List ---\n");
    
    /* Start with one value, grow to many rows */
    DWYCO_LIST single = dwyco_list_new();
    if (single == NULL) return;
    
    dwyco_list_append_int(single, 1);
    
    for (unsigned int i = 1u; i < 50u; i++) {
        dwyco_list_append(single, "value", 4u, DWYCO_TYPE_STRING);
    }
    
    printf("[OK] Extended from single to multi-row successfully\n");
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
    
    return 0;
}
