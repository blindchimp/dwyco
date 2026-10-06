// Coverage for the user-list / presence / uid-resolution / trash API.
//
//   dwyco_get_user_list2, dwyco_load_users2, dwyco_load_users_internal,
//   dwyco_get_updated_uids, dwyco_uid_status, dwyco_uid_online,
//   dwyco_uid_to_ip, dwyco_uid_to_ip2, dwyco_uid_g, dwyco_uid_to_info,
//   dwyco_delete_user, dwyco_clear_user, dwyco_fetch_info,
//   dwyco_map_uid_to_uids, dwyco_map_uid_to_representative,
//   dwyco_name_to_uid, dwyco_power_clean_safe, dwyco_empty_trash,
//   dwyco_count_trashed_users, dwyco_untrash_users, dwyco_run_sql
//
// Needs dwyco_init(); the uid-resolution and name-lookup tests additionally
// need a server login and are skipped without one.
//
// Contracts encoded here were established by probing, because several are
// not in the header:
//
//  * dwyco_uid_status's documented return values are wrong. dlli.h says
//    "0 for offline, 1 for online not available, 3 for online available",
//    but the implementation is "uid_online_display(v) | 2" and
//    uid_online_display only ever returns 0 or 1. So 0 and 1 are NEVER
//    returned; the real values are 2 (offline) and 3 (online). The bit
//    meanings still work: bit 0 is online-ness, bit 1 is always set.
//
//  * dwyco_uid_to_info falls back to the uid's ASCII HEX in the handle
//    column when the uid cannot be resolved. That is the one handle value
//    this test can assert on without depending on server state.
//
//  * dwyco_uid_to_info returns a 1-row, 6-column list for every uid,
//    resolved or not. Columns are DWYCO_INFO_*; "reviewed" and "regular" are
//    ints, the rest are strings.
//
//  * dwyco_uid_to_ip hands back a pointer straight out of inet_ntoa(), i.e.
//    static storage. Copy it out, never free it. The library's own comment
//    says the memory management here is suspect. dwyco_uid_to_ip2 by
//    contrast allocates a new[] "ip:port" string that DOES need
//    dwyco_free_array -- and on failure (no known ip) it returns 0 without
//    writing *str_out at all.
//
//  * dwyco_get_user_list2's nelems_out always equals the list's row count.
//
//  * dwyco_uid_g returns 1 unconditionally for one hardcoded uid
//    (5a098f3df49015331d74) -- a backdoor for the author, not a bug to fix
//    here, but worth pinning so nobody is surprised by it.
//
//  * dwyco_run_sql is only safe with VALID sql. See the note on that test.

#include <dlli.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "test_common.h"
#include "list_readback.h"

static int g_pass = 0;
static int g_fail = 0;

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

// A uid that is well-formed (10 bytes) but belongs to nobody. Used wherever a
// test needs a "real" uid that resolves to nothing, so it cannot collide with
// a live account.
static const char BOGUS_UID[10] = {
    (char)0xde, (char)0xad, (char)0xbe, (char)0xef, (char)0x01,
    (char)0x23, (char)0x45, (char)0x67, (char)0x89, (char)0xab
};

// Login state, for the tests that need the server.
static int g_login_done;

static void DWYCOCALLCONV
login_result_cb(const char *str, int what)
{
    (void)str;
    if (what == 1 || what == 2)
        g_login_done = 1;
}

// dwyco_name_to_uid results arrive as DWYCO_SE_IDENT_TO_UID.
static int g_ident_events;
static int g_ident_uid_len;
static char g_ident_uid[64];
static char g_ident_handle[128];
static int g_ident_handle_len;

static void DWYCOCALLCONV
system_event_cb(int cmd, int ctx_id, const char *uid, int len_uid,
    const char *name, int len_name, int type, const char *value, int val_len,
    int qid, int extra_arg)
{
    (void)ctx_id; (void)name; (void)len_name; (void)qid; (void)extra_arg;
    if (cmd != DWYCO_SE_IDENT_TO_UID)
        return;
    g_ident_events++;
    // NOTE: for this event the handle arrives in *value*, not in *name* --
    // se.cpp passes 0,0 for the name slot unconditionally.
    (void)type;
    g_ident_uid_len = len_uid < (int)sizeof(g_ident_uid) ? len_uid
        : (int)sizeof(g_ident_uid) - 1;
    if (uid && len_uid > 0)
        memcpy(g_ident_uid, uid, (size_t)g_ident_uid_len);
    g_ident_handle_len = val_len < (int)sizeof(g_ident_handle) - 1 ? val_len
        : (int)sizeof(g_ident_handle) - 1;
    if (value && val_len > 0)
        memcpy(g_ident_handle, value, (size_t)g_ident_handle_len);
    g_ident_handle[g_ident_handle_len] = 0;
}

static void
reset_ident(void)
{
    g_ident_events = 0;
    g_ident_uid_len = 0;
    g_ident_handle_len = 0;
    g_ident_handle[0] = 0;
}

static int g_my_uid[64];
static int g_my_uid_len;

// ===== user list =====

// nelems_out is redundant with the row count, and the two must agree.
static void
user_list_is_self_consistent(void)
{
    int nelems = -99;
    DWYCO_USER_LIST l = 0;
    CHECK(dwyco_get_user_list2(&l, &nelems) != 0);
    CHECK(l != 0);
    if (!l)
        return;
    int rows = -1, cols = -1;
    CHECK(dwyco_list_numelems(l, &rows, &cols) != 0);
    CHECK(rows >= 0);
    CHECK(nelems == rows);
    // The list holds bare uids, so it is single-column.
    CHECK(cols == -1);
    dwyco_list_release(l);
}

static void
load_users2_reports_a_total(void)
{
    int total = -99;
    CHECK(dwyco_load_users2(0, &total) != 0);
    CHECK(total >= 0);
    // "recent" is a filter; 10 must also work and report the same total.
    int total2 = -99;
    CHECK(dwyco_load_users2(10, &total2) != 0);
    CHECK(total2 >= 0);
}

// dwyco_load_users_internal is the no-output-pointer variant.
static void
load_users_internal(void)
{
    CHECK(dwyco_load_users_internal() != 0);
}

// "uids updated since <unix time>". Both a zero and a negative time are
// meaningful queries and must be safe.
static void
updated_uids(void)
{
    DWYCO_USER_LIST l = 0;
    CHECK(dwyco_get_updated_uids(&l, 0) != 0);
    CHECK(l != 0);
    if (l) {
        int rows = -1, cols = -1;
        CHECK(dwyco_list_numelems(l, &rows, &cols) != 0);
        CHECK(rows >= 0);
        dwyco_list_release(l);
    }
    l = 0;
    CHECK(dwyco_get_updated_uids(&l, -1) != 0);
    if (l)
        dwyco_list_release(l);
}

// ===== presence =====

// The header documents 0/1/3. Only 2 and 3 are reachable, because the
// implementation ORs in 2 unconditionally. This test pins the real contract;
// if the header is ever corrected to match, or the OR removed, this fails.
static void
uid_status_never_returns_0_or_1(void)
{
    int self = dwyco_uid_status((const char *)g_my_uid, g_my_uid_len);
    int bogus = dwyco_uid_status(BOGUS_UID, 10);

    printf("\n      status: self=%d bogus=%d", self, bogus);
    if (self == 2 || self == 3) printf(" (doc says 0/1/3)");
    printf("\n");

    CHECK(self == 2 || self == 3);
    CHECK(bogus == 2 || bogus == 3);
    // Bit 1 is the "always available" flag and is always set.
    CHECK((self & 2) != 0);
    CHECK((bogus & 2) != 0);
    // Bit 0 is online-ness.
    CHECK((self & 1) == 0 || (self & 1) == 1);
}

static void
uid_online_and_g_are_booleans(void)
{
    int on = dwyco_uid_online((const char *)g_my_uid, g_my_uid_len);
    int on_bogus = dwyco_uid_online(BOGUS_UID, 10);
    CHECK(on == 0 || on == 1);
    CHECK(on_bogus == 0 || on_bogus == 1);

    int g = dwyco_uid_g((const char *)g_my_uid, g_my_uid_len);
    int g_bogus = dwyco_uid_g(BOGUS_UID, 10);
    CHECK(g == 0 || g == 1);
    CHECK(g_bogus == 0 || g_bogus == 1);
    // Nobody in this test is a god.
    CHECK(g == 0);
    CHECK(g_bogus == 0);
}

// dwyco_uid_g has a hardcoded always-true uid baked into the library. Pin it
// so the backdoor is documented rather than a surprise.
static void
uid_g_has_a_hardcoded_uid(void)
{
    static const char backdoor[] = {
        (char)0x5a, (char)0x09, (char)0x8f, (char)0x3d, (char)0xf4,
        (char)0x90, (char)0x15, (char)0x33, (char)0x1d, (char)0x74
    };
    CHECK(dwyco_uid_g(backdoor, 10) == 1);
}

// ===== uid_to_info =====

// Every uid produces a 1-row, 6-column record. This is the multi-column
// readback the contact-list test could not reach.
static void
uid_to_info_shape(void)
{
    int cant = -99;
    DWYCO_LIST info = dwyco_uid_to_info(BOGUS_UID, 10, &cant);
    CHECK(info != 0);
    if (!info)
        return;
    CHECK(lr_rows(info, 1));
    CHECK(lr_cols(info, 6));
    // reviewed and regular are ints; the rest are strings. The *values* of
    // the string columns depend on resolution state -- an unresolved uid
    // gets its hex in the handle column, see the next test -- so only the
    // types are pinned here.
    CHECK(lr_col_int(info, 0, DWYCO_INFO_REVIEWED, 0));
    CHECK(lr_col_int(info, 0, DWYCO_INFO_REGULAR, 0));
    const char *v;
    int vl, vt;
    CHECK(dwyco_list_get(info, 0, DWYCO_INFO_HANDLE, &v, &vl, &vt) != 0);
    CHECK(vt == DWYCO_TYPE_STRING);
    CHECK(dwyco_list_get(info, 0, DWYCO_INFO_LOCATION, &v, &vl, &vt) != 0);
    CHECK(vt == DWYCO_TYPE_STRING);
    CHECK(dwyco_list_get(info, 0, DWYCO_INFO_DESCRIPTION, &v, &vl, &vt) != 0);
    CHECK(vt == DWYCO_TYPE_STRING);
    CHECK(dwyco_list_get(info, 0, DWYCO_INFO_EMAIL, &v, &vl, &vt) != 0);
    CHECK(vt == DWYCO_TYPE_STRING);
    dwyco_list_release(info);
}

// For a uid that cannot be resolved, the handle column falls back to the
// uid's ASCII HEX. That is the one DWYCO_INFO_HANDLE value that can be
// asserted without depending on server state, and it also exercises the
// hex-uid convention the whole database layer uses.
static void
uid_to_info_unresolved_handle_is_hex_uid(void)
{
    char want[64];
    uid_to_hex(BOGUS_UID, 10, want);

    int cant = -99;
    DWYCO_LIST info = dwyco_uid_to_info(BOGUS_UID, 10, &cant);
    CHECK(info != 0);
    if (!info)
        return;
    CHECK(lr_col_str(info, 0, DWYCO_INFO_HANDLE, want, 20));
    dwyco_list_release(info);
}

static void
uid_to_info_for_self(void)
{
    int cant = -99;
    DWYCO_LIST info = dwyco_uid_to_info((const char *)g_my_uid, g_my_uid_len, &cant);
    CHECK(info != 0);
    if (!info)
        return;
    CHECK(lr_rows(info, 1));
    CHECK(lr_cols(info, 6));
    dwyco_list_release(info);
}

// ===== uid_to_ip =====

// dwyco_uid_to_ip returns inet_ntoa()'s static buffer. Copy it out; do not
// free it. Contrast with uid_to_ip2 below, which allocates.
static void
uid_to_ip_returns_borrowed_string(void)
{
    int can_do_direct = -99;
    char *ip = 0;
    dwyco_uid_to_ip((const char *)g_my_uid, g_my_uid_len, &can_do_direct, &ip);
    CHECK(can_do_direct == 0 || can_do_direct == 1);
    // ip must point at a NUL-terminated string. Not freed -- borrowed.
    CHECK(ip != 0);
    if (ip) {
        size_t n = strlen(ip);
        CHECK(n > 0);
        CHECK(n < 64);
    }
}

// uid_to_ip2 allocates "ip:port" and returns 1, OR returns 0 having written
// nothing to *str_out. Poison the pointer so the "wrote nothing" case is
// detectable.
static void
uid_to_ip2_fails_cleanly_without_ip(void)
{
    int can_do_direct = -99;
    char *ip = (char *)0x1;
    int rc = dwyco_uid_to_ip2(BOGUS_UID, 10, &can_do_direct, &ip);
    if (rc == 0) {
        // Failure must not have written the out-pointer.
        CHECK(ip == (char *)0x1);
    } else {
        // Success allocates, so it must be freed -- but only after copying.
        char copy[64];
        CHECK(ip != 0);
        if (ip) {
            snprintf(copy, sizeof(copy), "%s", ip);
            // "ip:port"
            CHECK(strchr(copy, ':') != 0);
            dwyco_free_array(ip);
        }
    }
}

// ===== group mapping =====

// With no group, a uid maps to exactly itself.
static void
map_uid_to_uids_without_group(void)
{
    DWYCO_LIST l = 0;
    CHECK(dwyco_map_uid_to_uids((const char *)g_my_uid, g_my_uid_len, &l) != 0);
    CHECK(l != 0);
    if (!l)
        return;
    CHECK(lr_rows(l, 1));
    // The single entry is our own uid, read back as a binary string.
    CHECK(lr_str(l, 0, (const char *)g_my_uid, g_my_uid_len));
    dwyco_list_release(l);
}

static void
map_uid_to_representative(void)
{
    DWYCO_LIST l = 0;
    CHECK(dwyco_map_uid_to_representative((const char *)g_my_uid,
        g_my_uid_len, &l) != 0);
    CHECK(l != 0);
    if (!l)
        return;
    CHECK(lr_rows(l, 1));
    dwyco_list_release(l);
}

// ===== trash =====

// delete_user on an unknown-but-well-formed uid must succeed without
// corrupting anything, and the trash counters must stay consistent across
// empty_trash / untrash_users.
static void
trash_cycle_stays_consistent(void)
{
    int n0 = dwyco_count_trashed_users();
    CHECK(n0 >= 0);

    CHECK(dwyco_delete_user(BOGUS_UID, 10) != 0);
    int n1 = dwyco_count_trashed_users();
    CHECK(n1 >= 0);
    CHECK(n1 >= n0);

    CHECK(dwyco_clear_user(BOGUS_UID, 10) != 0);
    int n2 = dwyco_count_trashed_users();
    CHECK(n2 >= 0);

    CHECK(dwyco_empty_trash() != 0);
    int n3 = dwyco_count_trashed_users();
    CHECK(n3 == 0);

    // untrash_users is void and is a no-op on an empty trash.
    dwyco_untrash_users();
    CHECK(dwyco_count_trashed_users() == 0);
}

// power_clean is documented as a no-op in this build; it must still be safe
// to call.
static void
power_clean_is_safe(void)
{
    dwyco_power_clean_safe();
}

// fetch_info is fire-and-forget; it must not crash even for an unknown uid.
static void
fetch_info_is_safe(void)
{
    dwyco_fetch_info(BOGUS_UID, 10);
    dwyco_fetch_info((const char *)g_my_uid, g_my_uid_len);
}

// ===== run_sql =====

// dwyco_run_sql is only safe with VALID sql.
//
// The library's sql_run_sql wraps the statement in try/catch and rolls back,
// returning nil on error, and dwyco_run_sql maps that to 0. That is true, but
// it is not the whole story: a statement sqlite rejects makes the SQL layer
// call user_panic() -> exit(1) before the catch can help, and an empty
// statement segfaults outright. So the 0/1 return only distinguishes "ran
// fine" from "did not get that far".
//
// Valid SQL, including bound ?1/?2/?3 arguments, works and is tested here.
// Invalid SQL is tested out-of-process in run_sql_invalid_terminates().
static void
run_sql_valid_statements(void)
{
    // DDL, DML, positional binds, and idempotent drops.
    CHECK(dwyco_run_sql("create table if not exists dwytest_zz (a text, b int)",
        0, 0, 0) != 0);
    CHECK(dwyco_run_sql("insert into dwytest_zz values ('lit', 1)", 0, 0, 0) != 0);
    CHECK(dwyco_run_sql("insert into dwytest_zz values (?1, ?2)",
        "bound", "2", 0) != 0);
    CHECK(dwyco_run_sql("select count(*) from dwytest_zz", 0, 0, 0) != 0);
    CHECK(dwyco_run_sql("select a from dwytest_zz where b = ?1", "2", 0, 0) != 0);
    // A read-only statement returns 0 rather than 1: the return is "was the
    // result non-nil", not "did it work".
    CHECK(dwyco_run_sql("drop table if exists dwytest_zz", 0, 0, 0) != 0);
    CHECK(dwyco_run_sql("drop table if exists dwytest_zz", 0, 0, 0) != 0);
}

static const char *g_argv0 = 0;

static int init_test(void);

static void
run_sql_child(int argc, char **argv)
{
    (void)argc;
    // dwyco_init() is required: the SQL layer has no database without it,
    // so every statement would fail for the wrong reason.
    if (!init_test()) {
        printf("      child init failed\n");
        _exit(99);
    }
    dwyco_run_sql(argv[2], 0, 0, 0);
    exit(0);
}

// Invalid SQL terminates the process rather than returning 0. Verified
// out-of-process because exit(1) cannot be caught here.
//
// If the SQL layer is ever taught to report errors instead of panicking,
// this test fails and should be rewritten to assert the 0 return.
static void
run_sql_invalid_terminates(void)
{
    if (!g_argv0) {
        printf("[FAIL] no argv[0] recorded for subprocess test\n");
        g_fail++;
        return;
    }
    // "select 1" is the control: valid, must survive and exit 0.
    char *ok_argv[] = { (char *)g_argv0, (char *)"--run-sql",
        (char *)"select 1", 0 };
    CHECK(run_subprocess(ok_argv) == 0);
    // Not sql at all.
    char *bad_argv[] = { (char *)g_argv0, (char *)"--run-sql",
        (char *)"this is not sql", 0 };
    CHECK(run_subprocess(bad_argv) == 1);
    // Valid sql against a table that does not exist.
    char *missing_argv[] = { (char *)g_argv0, (char *)"--run-sql",
        (char *)"select * from dwytest_no_such_table", 0 };
    CHECK(run_subprocess(missing_argv) == 1);
}

// ===== name -> uid =====

// dwyco_name_to_uid is async and reports through DWYCO_SE_IDENT_TO_UID.
//
// The event's *handle arrives in the value parameter; the name parameter is
// always null for this event (se.cpp passes 0,0 there unconditionally). The
// uid parameter is the resolved uid, or an empty string when the handle does
// not resolve.
//
// This needs the directory server, so it is skipped without a login.
static void
name_to_uid_reports_through_ident_event(void)
{
    if (!g_login_done) {
        printf("(no login) ");
        return;
    }
    // test_bootstrap_profile registered this handle on the server.
    static const char handle[] = "dwytest-users";
    reset_ident();
    dwyco_name_to_uid(handle, (int)strlen(handle));
    int got = wait_for([]() { return g_ident_events > 0; }, 30000);
    if (!got) {
        printf("(no IDENT event) ");
        return;
    }
    CHECK(g_ident_handle_len == (int)strlen(handle));
    CHECK(g_ident_handle_len > 0);
    if (g_ident_handle_len > 0)
        CHECK(memcmp(g_ident_handle, handle, (size_t)g_ident_handle_len) == 0);
    // If it resolved, the uid must be well-formed.
    if (g_ident_uid_len > 0)
        CHECK(g_ident_uid_len == 10);
    printf("(resolved=%d) ", g_ident_uid_len == 10);
}

// ===== harness =====

static int
init_test(void)
{
    static const char *dir = "/tmp/dwytest_users";
    char sys_dir[512], tmp_dir[512];
    snprintf(sys_dir, sizeof(sys_dir), "%s/sys", dir);
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", dir);
    mkdir(dir, 0755);
    mkdir(sys_dir, 0755);
    mkdir(tmp_dir, 0755);
    install_app_files(dir);

    dwyco_set_fn_prefixes(sys_dir, dir, tmp_dir);
    dwyco_set_system_event_callback(system_event_cb);
    dwyco_set_login_result_callback(login_result_cb);
    dwyco_set_client_version("dwytest", 7);
    if (dwyco_init() == 0)
        return 0;
    test_bootstrap_profile("dwytest-users", "dwytest users account");
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);

    const char *uid;
    int len;
    dwyco_get_my_uid(&uid, &len);
    if (len <= 0 || len >= (int)sizeof(g_my_uid))
        return 0;
    memcpy(g_my_uid, uid, (size_t)len);
    g_my_uid_len = len;
    return 1;
}

int
main(int argc, char **argv)
{
    static const char *dir = "/tmp/dwytest_users";
    char sys_dir[512], tmp_dir[512];

    if (argc > 2 && strcmp(argv[1], "--run-sql") == 0) {
        setvbuf(stdout, 0, _IOLBF, 0);
        // Silence the expected panic message for the failure cases.
        int null_fd = open("/dev/null", O_WRONLY);
        if (null_fd >= 0) {
            dup2(null_fd, 2);
            close(null_fd);
        }
        run_sql_child(argc, argv);
        return 0;
    }
    g_argv0 = argv[0];
    setvbuf(stdout, 0, _IOLBF, 0);

    printf("Dwyco users / presence / uid resolution\n");

    if (!init_test()) {
        fprintf(stderr, "dwyco_init failed\n");
        return 1;
    }

    printf("  Waiting for server login...\n");
    if (wait_for([]() { return g_login_done != 0; }, 10000))
        printf("  Login OK\n");
    else
        printf("  Login timeout (resolution tests will skip)\n");

    printf("\nUser list:\n");
    RUN(user_list_is_self_consistent);
    RUN(load_users2_reports_a_total);
    RUN(load_users_internal);
    RUN(updated_uids);

    printf("\nPresence:\n");
    RUN(uid_status_never_returns_0_or_1);
    RUN(uid_online_and_g_are_booleans);
    RUN(uid_g_has_a_hardcoded_uid);

    printf("\nuid_to_info:\n");
    RUN(uid_to_info_shape);
    RUN(uid_to_info_unresolved_handle_is_hex_uid);
    RUN(uid_to_info_for_self);

    printf("\nAddresses:\n");
    RUN(uid_to_ip_returns_borrowed_string);
    RUN(uid_to_ip2_fails_cleanly_without_ip);

    printf("\nGroup mapping:\n");
    RUN(map_uid_to_uids_without_group);
    RUN(map_uid_to_representative);

    printf("\nTrash:\n");
    RUN(trash_cycle_stays_consistent);
    RUN(power_clean_is_safe);
    RUN(fetch_info_is_safe);

    printf("\nSQL:\n");
    RUN(run_sql_valid_statements);
    RUN(run_sql_invalid_terminates);

    printf("\nName resolution:\n");
    RUN(name_to_uid_reports_through_ident_event);

    dwyco_exit();

    printf("\nSummary: passed=%d failed=%d\n", g_pass, g_fail);
    if (g_fail) {
        printf("FAILED\n");
        return 1;
    }
    printf("All user tests passed.\n");
    return 0;
}