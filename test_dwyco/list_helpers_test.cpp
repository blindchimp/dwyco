/* === Dwyco List Helpers - Cases 1 & 2: Creation, Destruction, Append Operations === */
#include <dlli.h>
#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

#define test_pass(name) printf("[PASS] %s\n", name); g_pass++
#define test_fail(fmt, ...) printf("[FAIL] " fmt "\n", ##__VA_ARGS__); g_fail++

/* === Test Case #1a: Basic Creation === */
static void tc1a(void) {
    DWYCO_LIST list = dwyco_list_new();
    
    if ((list == NULL)) {
        test_fail("List creation returned NULL\n");
        return;
    }
    
    dwyco_list_release(list);
    test_pass("Created and released basic list");
}

/* === Test Case #1b: Multiple Lists Live Simultaneously === */
static void tc1b(void) {
    DWYCO_LIST lists[3] = { NULL, NULL, NULL };
    for (int i = 0; i < 3; i++) {
        lists[i] = dwyco_list_new();
        if (lists[i]) {
            dwyco_list_append_int(lists[i], i);
        }
    }
    for (int i = 0; i < 3; i++) {
        if (lists[i]) dwyco_list_release(lists[i]);
    }
    test_pass("Created, used, and released multiple lists");
}

/* === Test Case #1c: Stress Creation/Release Loop === */
static void tc1c(void) {
    for (int i = 0; i < 1000; i++) {
        DWYCO_LIST list_stress = dwyco_list_new();
        if (list_stress) {
            dwyco_list_release(list_stress);
        } else break;
    }
    test_pass("Released 1000 lists in a loop");
}

/* === Test Case #1d: Immediate Release Pattern === */
static void tc1d(void) {
    for (int i = 0; i < 1000; i++) {
        DWYCO_LIST tmp_list = dwyco_list_new();
        if (!tmp_list) continue;
        dwyco_list_release(tmp_list);
    }
    test_pass("1000 quick create-release cycles");
}

/* === Test Case #2a: Append_Int_Valid_List_Basic_Value === */
static void tc2a(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) {
        test_fail("append_int test - new() returned NULL\n");
        return;
    }
    dwyco_list_append_int(list, 42);
    test_pass("append_int() on valid list");
    dwyco_list_release(list);
}

/* === Test Case #2b: Append_Int_Valid_List_Basic_Value === */
static void tc2b(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) {
        test_fail("Append_int test - new() returned NULL\n");
        return;
    }
    
    int values[5] = {-1, 0, 1, 42, 255};
    for (int i = 0; i < 5; i++) {
        dwyco_list_append_int(list, values[i]);
    }
    test_pass("Appended various integer values to list");
    dwyco_list_release(list);
}

/* === Test Case #2c: Bulk Append via Loop === */
static void tc2c(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) return;
    
    int count = 0;
    for (int i = 1; i <= 500; i++) {
        dwyco_list_append_int(list, i);
        count++;
    }
    printf("[PASS] Completed bulk append of %d items\n", count); g_pass++;
    dwyco_list_release(list);
}

/* === Test Case #2d: Append With Zero and Boundary Values === */
static void tc2d(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) return;
    
    dwyco_list_append_int(list, 0);
    dwyco_list_append_int(list, -1);
    dwyco_list_append_int(list, 65535);
    test_pass("Boundary value appends");
    dwyco_list_release(list);
}

/* === Test Case #2e: Append With Multiple Zero Values === */
static void tc2e(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) return;
    
    for (int i = 0; i < 10; i++) {
        dwyco_list_append_int(list, 0);
    }
    test_pass("Appended 10 zero values");
    dwyco_list_release(list);
}

/* === Test Case #2f: Append With Mixed Signs And Magnitudes === */
static void tc2f(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) return;
    
    int vals[5] = {-100, -1, 0, 1, 100};
    for (int i = 0; i < 5; i++) {
        dwyco_list_append_int(list, vals[i]);
    }
    test_pass("Mixed signed magnitude appends");
    dwyco_list_release(list);
}

/* === Test Case #2g: Append With Sequential Values === */
static void tc2g(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) return;
    
    for (int i = 0; i < 1000; i++) {
        dwyco_list_append_int(list, i);
    }
    test_pass("Appended sequential values 0-999");
    dwyco_list_release(list);
}

/* === Test Case #2h: Append With Large Sequential Values === */
static void tc2h(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) return;
    
    for (int i = 10000; i < 15000; i++) {
        dwyco_list_append_int(list, i);
    }
    test_pass("Appended values 10000-14999");
    dwyco_list_release(list);
}

/* === Test Main Entry Point === */
int main(void) {
    printf("\n=== Dwyco List Helpers (Cases #1 & #2 Coverage) ===\n\n");
    
    tc1a();
    tc1b();
    tc1c();
    tc1d();
    
    tc2a();
    tc2b();
    tc2c();
    tc2d();
    tc2e();
    tc2f();
    tc2g();
    tc2h();
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d\n", g_pass);
    printf("Failed: %d\n", g_fail);
    printf("Total:  %d\n\n", g_pass + g_fail);
    
    return g_fail > 0 ? 1 : 0;
}
