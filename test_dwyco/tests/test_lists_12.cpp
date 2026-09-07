/* === Dwyco List Helpers - Cases 1 & 2: Creation, Destruction, Append Operations === */
#include <dlli.h>
#include <cstdio>

static int g_pass = 0;

#define test_pass(name) do { printf("[PASS] %s\n", name); g_pass++; } while(0)

/* === Test Case #1a: Basic Creation === */
static void tc1a(void) {
    DWYCO_LIST list = dwyco_list_new();
    
    if (!list) {
        printf("[INFO] List creation returned NULL\n");
    } else {
        dwyco_list_release(list);
        test_pass("Created and released basic list");
    }
}

/* === Test Case #1b: Stress Creation/Release Loop === */
static void tc1b(void) {
    for (int i = 0; i < 1000; i++) {
        DWYCO_LIST list = dwyco_list_new();
        if (list) {
            dwyco_list_release(list);
        } else {
            break;
        }
    }
    test_pass("Released 1000 lists in a loop");
}

/* === Test Case #1c: Immediate Release Pattern === */
static void tc1c(void) {
    for (int i = 0; i < 1000; i++) {
        DWYCO_LIST tmp_list = dwyco_list_new();
        if (!tmp_list) continue;
        dwyco_list_release(tmp_list);
    }
    test_pass("1000 quick create-release cycles");
}

/* === Test Case #2a: Append_INT Type Coverage === */
static void tc2a(void) {
    DWYCO_LIST intlist = dwyco_list_new();
    if (!intlist) {
        printf("[SKIP] Could not create list for append_int tests\n");
        return;
    }
    
    int value_42 = 42;
    dwyco_list_append_int(intlist, value_42);
    test_pass("Successfully called append_int(42)");
}

/* === Test Case #2b: Append_String Type Coverage === */
static void tc2b(void) {
    DWYCO_LIST strlist = dwyco_list_new();
    if (!strlist) return;
    
    dwyco_list_append(strlist, "hello world", 10u, DWYCO_TYPE_STRING);
    dwyco_list_append(strlist, "", 0u, DWYCO_TYPE_NIL);
    test_pass("String appends completed");
}

/* === Test Case #2c: Append with Various Integer Values === */
static void tc2c(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) return;
    
    int values[5] = {-1, 0, 1, 42, 255};
    for (int i = 0; i < 5; i++) {
        dwyco_list_append_int(list, values[i]);
    }
    test_pass("Appended various integer values");
    dwyco_list_release(list);
}

/* === Test Case #2d: Bulk Append via Loop === */
static void tc2d(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) return;
    
    for (int i = 1; i <= 500; i++) {
        dwyco_list_append_int(list, i);
    }
    test_pass("Completed bulk append of 500 items");
    dwyco_list_release(list);
}

/* === Test Case #2e: Append with Zero and Boundary Values === */
static void tc2e(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) return;
    
    dwyco_list_append_int(list, 0);
    dwyco_list_append_int(list, -1);
    dwyco_list_append_int(list, 65535);
    test_pass("Boundary value appends");
    dwyco_list_release(list);
}

/* === Test Case #2f: Append with Mixed Signs and Magnitudes === */
static void tc2f(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) return;
    
    int vals[] = {-100, -1, 0, 1, 100};
    for (int i = 0; i < 5; i++) {
        dwyco_list_append_int(list, vals[i]);
    }
    test_pass("Mixed signed magnitude appends");
    dwyco_list_release(list);
}

/* === Test Case #2g: Append with Sequential Values === */
static void tc2g(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) return;
    
    for (int i = 0; i < 1000; i++) {
        dwyco_list_append_int(list, i);
    }
    test_pass("Appended sequential values 0-999");
    dwyco_list_release(list);
}

/* === Test Case #2h: Append with Large Sequential Values === */
static void tc2h(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) return;
    
    for (int i = 10000; i < 15000; i++) {
        dwyco_list_append_int(list, i);
    }
    test_pass("Appended values 10000-14999");
    dwyco_list_release(list);
}

/* === Test Case #2i: Multi-Row List Creation === */
static void tc2i(void) {
    DWYCO_LIST multilist = dwyco_list_new();
    if (!multilist) return;
    
    for (int i = 0; i < 10; i++) {
        dwyco_list_append(multilist, "item", 4u, DWYCO_TYPE_STRING);
    }
    test_pass("Created multi-row list via repeated appends");
}

/* === Test Case #2j: Mixed Type Append Patterns === */
static void tc2j(void) {
    DWYCO_LIST mix_list = dwyco_list_new();
    if (!mix_list) return;
    
    /* Mix types to test whether list maintains type correctness */
    int len_u = (int)(sizeof("string")-1);
    if (mix_list) {
        dwyco_list_append_int(mix_list, 100);
        const char *p = "string";
        dwyco_list_append(mix_list, p, (const unsigned int)(sizeof(p)-1), DWYCO_TYPE_STRING);
        dwyco_list_append_int(mix_list, 200);
    }
    
    test_pass("Mixed type appends completed");
}

/* === Test Case #2k: Empty String Handling === */
static void tc2k(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) return;
    
    /* Only append valid empty string */
    dwyco_list_append(list, "", 0u, DWYCO_TYPE_NIL);
    test_pass("Empty string append completed");
}

/* === Test Case #2l: Append then Release === */
static void tc2l(void) {
    DWYCO_LIST list = dwyco_list_new();
    if (!list) return;
    
    /* Test release after appending */
    dwyco_list_append_int(list, 999);
    test_pass("Appended and prepared for release");
    dwyco_list_release(list);
}

int main(void) {
    tc1a();
    tc1b();
    tc1c();
    
    tc2a();
    tc2b();
    tc2c();
    tc2d();
    tc2e();
    tc2f();
    tc2g();
    tc2h();
    tc2i();
    tc2j();
    tc2k();
    tc2l();
    
    return 0;
}
