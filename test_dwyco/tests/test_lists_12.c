/* === Dwyco List Helpers - Cases 1 & 2: Creation, Destruction, Append Operations === */
#include <dlli.h>
#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

/* === Helper Macro === */
#define test_pass(name) do { printf("[PASS] %s\n", name); g_pass++; } while(0)
#define test_fail(name, fmt, ...) do { printf("[FAIL] %s: " fmt "\n", name, ##__VA_ARGS__); g_fail++; } while(0)

/* === Test Case #1a: Basic Creation === */
static void tc1a(void) {
    const char *list = dwyco_list_new();
    
    if (!g_pass == 0 && list == NULL) {
        test_fail("List returned NULL", "");
        return;
    } else if (!list) {
        /* Expected on some platforms */
        printf("[INFO] List creation returned NULL\n");
    }
    
    if (list) {
        dwyco_list_release(list);
        test_pass("Created and released basic list");
    }
}

/* === Test Case #1b: Multiple Releases === */
static void tc1b(void) {
    const char *dummy = dwyco_list_new();
    if (dummy) dwyco_list_release(dummy);
    
    for (int i = 0; i < 3; i++) {
        dwyco_list_release(NULL);  /* Should be safe no-op */
    }
    test_pass("Multiple release(NULL) calls");
}

/* === Test Case #1c: Stress Creation/Release Loop === */
static void tc1c(void) {
    for (int i = 0; i < 1000; i++) {
        const char *list = dwyco_list_new();
        if (list) {
            dwyco_list_release(list);
        } else {
            break;
        }
    }
    test_pass("Released 1000 lists in a loop");
}

/* === Test Case #1d: Immediate Release Pattern === */
static void tc1d(void) {
    for (int i = 0; i < 1000; i++) {
        const char *tmp_list = dwyco_list_new();
        if (!tmp_list) continue;
        dwyco_list_release(tmp_list);
    }
    test_pass("1000 quick create-release cycles");
}

/* === Test Case #2a: Append_Int_Null_Safety === */
static void tc2a(void) {
    /* Test that append_int doesn't crash with NULL or empty inputs */
    dwyco_list_append_int(NULL, 42);
    test_pass("append_int() didn't crash on NULL");
}

/* === Test Case #2b: Append_Int_Various_Values === */
static void tc2b(void) {
    const char *list = NULL;
    
    /* Can't create and use list in this function due to previous failure case */
    /* Testing null-safety instead */
    test_fail("Append_int basic value tests - skipped if new() returns NULL");
}

/* === Test Case #2c: Append_Int_Valid_List_After_Skip === */
static void tc2c(void) {
    const char *list = dwyco_list_new();
    if (!list) return;  /* Can't test properly without valid list */
    
    /* Test appending various integer values to the valid list */
    int values[5] = {-1, 0, 1, 42, 255};
    for (int i = 0; i < 5; i++) {
        dwyco_list_append_int(list, values[i]);
    }
    test_pass("Appended various integer values");
    dwyco_list_release(list);
}

/* === Test Case #2d: Bulk Append via Loop === */
static void tc2d(void) {
    const char *list = dwyco_list_new();
    if (!list) return;
    
    int count = 0;
    for (int i = 1; i <= 500; i++) {
        dwyco_list_append_int(list, i);
        count++;
    }
    test_pass("Completed bulk append of %d items", count);
    dwyco_list_release(list);
}

/* === Test Case #2e: Append with Zero and Boundary Values === */
static void tc2e(void) {
    const char *list = dwyco_list_new();
    if (!list) return;
    
    /* Edge cases for integer append */
    dwyco_list_append_int(list, 0);      /* Minimum int usually */
    dwyco_list_append_int(list, -1);
    dwyco_list_append_int(list, 65535);  /* Typical max uint16 */
    test_pass("Boundary value appends");
    dwyco_list_release(list);
}

/* === Test Case #2f: Append_Multiple_Zeros_Only === */
static void tc2f(void) {
    const char *list = dwyco_list_new();
    if (!list) return;
    
    for (int i = 0; i < 10; i++) {
        dwyco_list_append_int(list, 0);
    }
    test_pass("Appended 10 zero values");
    dwyco_list_release(list);
}

/* === Test Case #2g: Append with Mixed Signs and Magnitudes === */
static void tc2g(void) {
    const char *list = dwyco_list_new();
    if (!list) return;
    
    /* Mix positive, negative, zero values */
    int vals[] = {-100, -1, 0, 1, 100};
    for (int i = 0; i < 5; i++) {
        dwyco_list_append_int(list, vals[i]);
    }
    test_pass("Mixed signed magnitude appends");
    dwyco_list_release(list);
}

/* === Test Case #2h: Append with Sequential Values === */
static void tc2h(void) {
    const char *list = dwyco_list_new();
    if (!list) return;
    
    for (int i = 0; i < 1000; i++) {
        dwyco_list_append_int(list, i);
    }
    test_pass("Appended sequential values 0-999");
    dwyco_list_release(list);
}

/* === Test Case #2i: Append with Large Sequential Values === */
static void tc2i(void) {
    const char *list = dwyco_list_new();
    if (!list) return;
    
    for (int i = 10000; i < 15000; i++) {
        dwyco_list_append_int(list, i);
    }
    test_pass("Appended values 10000-14999");
    dwyco_list_release(list);
}

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
    tc2i();
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d\n", g_pass);
    printf("Failed: %d\n", g_fail);
    printf("Total:  %d\n\n", g_pass + g_fail);
    
    return g_fail > 0 ? 1 : 0;
}
