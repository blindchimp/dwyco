// Coverage for the settings / codec / runtime-state / contacts API.
//
//   dwyco_set_setting, dwyco_get_setting, dwyco_set_codec_data,
//   dwyco_get_codec_data, dwyco_get_fn_prefixes, dwyco_get_authenticator,
//   dwyco_get_create_new_account, dwyco_get_invisible_state,
//   dwyco_set_invisible_state, dwyco_set_initial_invis,
//   dwyco_get_suspend_state, dwyco_suspend, dwyco_resume,
//   dwyco_get_refresh_users, dwyco_set_refresh_users,
//   dwyco_get_moron_dork_mode, dwyco_set_moron_dork_mode,
//   dwyco_add_entropy_timer, dwyco_trace_init, dwyco_field_debug,
//   dwyco_debug_dump, dwyco_app_debug1, dwyco_app_debug2,
//   dwyco_add_contact, dwyco_get_contact_list, dwyco_clear_contact_list,
//   dwyco_set_aux_string, dwyco_get_aux_string, dwyco_write_token
//
// These need dwyco_init() -- the settings live in a set.sql that init
// creates -- but they do NOT need a server login. The test is therefore
// registered under the "server" label but only asserts local behavior.
//
// Contracts encoded here were established by probing the library, because
// several are not in the header and two contradict it:
//
//  * dwyco_set_setting("user/...", ...) ALWAYS returns 0; user/* settings
//    are profile-managed now and are only reachable through the profile API.
//    (The header used to give exactly this call as its worked example.)
//
//  * There is no "display" setting group -- nothing is ever registered under
//    it. The documented groups are net, call_acceptance, raw_files,
//    video_format, video_input, zap; rate, auth, group, sync, server, app
//    also exist, plus a typo'd "vid_input".
//
//  * dwyco_get_setting / dwyco_set_setting call oopanic() (exit(1)) for an
//    unknown setting name. Only the two names handled before the lookup --
//    one with no '/' at all, and anything under "user" -- fail safely.
//
//  * dwyco_get_setting's *value_out must be copied out immediately and must
//    not be freed -- same rule as the dwyco_list_get family. Only
//    dwyco_get_authenticator and dwyco_get_aux_string hand back buffers
//    that must be freed.
//
//  * dwyco_get_setting on an int setting returns decimal ASCII with
//    DWYCO_TYPE_INT, matching the dwyco_list_get convention.
//
//  * dwyco_get_fn_prefixes returns void, normalizes every prefix to end in
//    '/', and has no "how much space do you need" query: pass buffers that
//    are already big enough. Too-small buffers are silently skipped with the
//    lengths left untouched.
//
//  * dwyco_set_codec_data coerces agc/denoise with !!, so any nonzero value
//    reads back as 1.
//
//  * dwyco_get_refresh_users / dwyco_set_refresh_users are both no-ops.

#include <dlli.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
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

// argv[0], for the subprocess tests.
static const char *g_argv0 = 0;
static const char *g_dir = "/tmp/dwytest_settings";

// Forward declared so unknown_setting_child()'s subprocess can bring the
// client up before poking at the settings database.
static int init_test(void);

// Reading a setting into freshly initialized locals. get_setting writes
// through value_out, so all four must be valid on entry.
static int
get_setting(const char *name, const char **val, int *len, int *type)
{
    *val = 0;
    *len = -1;
    *type = -1;
    return dwyco_get_setting(name, val, len, type);
}

// ===== settings =====

// An int setting round-trips as decimal ASCII with DWYCO_TYPE_INT.
//
// Deliberately does NOT assert a specific default: net/primary_port is
// overwritten during init with whatever port the listener actually bound,
// and that has been observed to differ between runs.
static void
setting_int_round_trip(void)
{
    const char *val;
    int len, type;

    CHECK(get_setting("net/primary_port", &val, &len, &type) != 0);
    CHECK(type == DWYCO_TYPE_INT);

    CHECK(dwyco_set_setting("net/primary_port", "12345") != 0);
    CHECK(get_setting("net/primary_port", &val, &len, &type) != 0);
    CHECK(type == DWYCO_TYPE_INT);
    CHECK(len == 5);
    CHECK(val && memcmp(val, "12345", 5) == 0);

    // A negative int must survive too.
    CHECK(dwyco_set_setting("net/primary_port", "-7") != 0);
    CHECK(get_setting("net/primary_port", &val, &len, &type) != 0);
    CHECK(len == 2);
    CHECK(val && memcmp(val, "-7", 2) == 0);
}

static void
setting_string_round_trip(void)
{
    const char *val;
    int len, type;

    CHECK(get_setting("net/app_id", &val, &len, &type) != 0);
    CHECK(type == DWYCO_TYPE_STRING);

    static const char want[] = "dwytest-app-id";
    CHECK(dwyco_set_setting("net/app_id", want) != 0);
    CHECK(get_setting("net/app_id", &val, &len, &type) != 0);
    CHECK(type == DWYCO_TYPE_STRING);
    CHECK(len == (int)strlen(want));
    CHECK(val && memcmp(val, want, strlen(want)) == 0);

    // An empty string is a legitimate value.
    CHECK(dwyco_set_setting("net/app_id", "") != 0);
    CHECK(get_setting("net/app_id", &val, &len, &type) != 0);
    CHECK(len == 0);
}

// Every group the header lists as valid must have at least one gettable
// setting, except "display" which the header lists but which nothing
// registers. See the file header comment.
static void
setting_groups(void)
{
    static const char *names[] = {
        "net/app_id",
        "call_acceptance/auto_accept",
        "raw_files/preload",
        "user/email",
        "video_format/flip",
        "video_input/source",
        "zap/save_sent",
        // Not in the header's list at all, but real:
        "rate/max_fps",
        "sync/eager",
        "server/invis",
        "app/nicename",
        "group/alt_name",
    };
    const char *val;
    int len, type;
    for (unsigned i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        if (get_setting(names[i], &val, &len, &type) != 0) {
            CHECK(type == DWYCO_TYPE_INT || type == DWYCO_TYPE_STRING);
        } else {
            printf("[FAIL] get_setting(%s) failed\n", names[i]);
            g_fail++;
        }
    }
}

// The two cases that fail safely rather than aborting.
static void
setting_rejected_names(void)
{
    // No '/' at all.
    CHECK(dwyco_set_setting("bogus", "x") == 0);
    CHECK(dwyco_set_setting("", "x") == 0);

    // user/* is explicitly refused -- the header's own example at
    // dlli.h:2261 claims this works. It does not.
    CHECK(dwyco_set_setting("user/email", "foo@bar.com") == 0);
    CHECK(dwyco_set_setting("user/username", "someone") == 0);
    // But reading a user setting is fine.
    const char *val;
    int len, type;
    CHECK(get_setting("user/email", &val, &len, &type) != 0);
    CHECK(type == DWYCO_TYPE_STRING);
}

// get_setting's value_out points into the setting's own storage: copy it out
// before the next call and do NOT free it. Reading it twice must give the
// same bytes, and the pointer must stay stable as long as the setting is
// unchanged.
static void
setting_output_is_borrowed(void)
{
    CHECK(dwyco_set_setting("net/app_id", "borrowed-check") != 0);

    const char *a;
    int alen, atype;
    const char *b;
    int blen, btype;
    CHECK(get_setting("net/app_id", &a, &alen, &atype) != 0);
    CHECK(get_setting("net/app_id", &b, &blen, &btype) != 0);
    CHECK(a == b);
    CHECK(alen == blen);
    // No dwyco_free_array here on purpose: the pointer is borrowed, so
    // freeing it would corrupt the settings db. This mirrors how every other
    // dwyco_list_get-style accessor has to be used.
}

// Unknown names abort inside the library (get_settings_value calls
// oopanic("bad setting"), which is [[noreturn]] and exits 1). Verified in a
// subprocess because that cannot be caught in-process.
static void
unknown_setting_child(const char *which)
{
    const char *val = 0;
    int len = 0;
    int type = 0;

    // init_test() is required first: the settings Map is built during
    // dwyco_init, and looking a name up in an unbuilt Map would segfault
    // instead of hitting the oopanic we are trying to observe.
    if (!init_test()) {
        printf("      child init failed\n");
        _exit(99);
    }

    if (strcmp(which, "get") == 0) {
        printf("      about to get an unknown setting...\n");
        fflush(stdout);
        printf("      get rc=%d\n",
            dwyco_get_setting("bogus/nosuch", &val, &len, &type));
    } else {
        printf("      about to set an unknown setting...\n");
        fflush(stdout);
        printf("      set rc=%d\n", dwyco_set_setting("bogus/nosuch", "1"));
    }
    // Only reached if the library stops panicking, which would be a fix.
    exit(0);
}

static void
unknown_setting_aborts(void)
{
    if (!g_argv0) {
        printf("[FAIL] no argv[0] recorded for subprocess test\n");
        g_fail++;
        return;
    }
    char *set_argv[] = { (char *)g_argv0, (char *)"--unknown-setting",
        (char *)"set", 0 };
    CHECK(run_subprocess(set_argv) == 1);

    char *get_argv[] = { (char *)g_argv0, (char *)"--unknown-setting",
        (char *)"get", 0 };
    CHECK(run_subprocess(get_argv) == 1);
}

// ===== codec data =====

static void
codec_data_round_trip(void)
{
    int agc, denoise;
    double delay;

    CHECK(dwyco_set_codec_data(1, 0, 0.25) != 0);
    CHECK(dwyco_get_codec_data(&agc, &denoise, &delay) != 0);
    CHECK(agc == 1);
    CHECK(denoise == 0);
    CHECK(delay > 0.24 && delay < 0.26);

    CHECK(dwyco_set_codec_data(0, 1, 0.5) != 0);
    CHECK(dwyco_get_codec_data(&agc, &denoise, &delay) != 0);
    CHECK(agc == 0);
    CHECK(denoise == 1);
    CHECK(delay > 0.49 && delay < 0.51);
}

// agc and denoise are stored with !!, so any nonzero value collapses to 1
// rather than being preserved.
static void
codec_data_coerces_booleans(void)
{
    int agc, denoise;
    double delay;
    CHECK(dwyco_set_codec_data(42, 99, 0.0) != 0);
    CHECK(dwyco_get_codec_data(&agc, &denoise, &delay) != 0);
    CHECK(agc == 1);
    CHECK(denoise == 1);
}

// ===== fn prefixes =====

// dwyco_get_fn_prefixes returns void and normalizes every prefix to end in
// '/', so what comes back is not necessarily byte-identical to what was set.
static void
fn_prefixes_round_trip(void)
{
    char sys_pfx[512], user_pfx[512], tmp_pfx[512];
    int sys_len = (int)sizeof(sys_pfx);
    int user_len = (int)sizeof(user_pfx);
    int tmp_len = (int)sizeof(tmp_pfx);

    dwyco_get_fn_prefixes(sys_pfx, &sys_len, user_pfx, &user_len,
        tmp_pfx, &tmp_len);

    CHECK(sys_pfx[0] != 0);
    CHECK(user_pfx[0] != 0);
    CHECK(tmp_pfx[0] != 0);

    // All three are normalized with a trailing slash.
    CHECK(sys_pfx[sys_len - 2] == '/');
    CHECK(user_pfx[user_len - 2] == '/');
    CHECK(tmp_pfx[tmp_len - 2] == '/');

    // Lengths come back as strlen + 1.
    CHECK(sys_len == (int)strlen(sys_pfx) + 1);
    CHECK(user_len == (int)strlen(user_pfx) + 1);
    CHECK(tmp_len == (int)strlen(tmp_pfx) + 1);

    // The values we asked for, with the trailing slash the library adds.
    char want[512];
    snprintf(want, sizeof(want), "%s/sys/", g_dir);
    CHECK(strcmp(sys_pfx, want) == 0);
    snprintf(want, sizeof(want), "%s/", g_dir);
    CHECK(strcmp(user_pfx, want) == 0);
    snprintf(want, sizeof(want), "%s/tmp/", g_dir);
    CHECK(strcmp(tmp_pfx, want) == 0);
}

// There is no "tell me the size" query: a too-small buffer is skipped and the
// corresponding length is left exactly as passed in.
static void
fn_prefixes_small_buffer_is_ignored(void)
{
    char a[4] = { 0 }, b[4] = { 0 }, c[4] = { 0 };
    int la = 4, lb = 4, lc = 4;

    dwyco_get_fn_prefixes(a, &la, b, &lb, c, &lc);

    CHECK(la == 4);
    CHECK(lb == 4);
    CHECK(lc == 4);
    CHECK(a[0] == 0);
    CHECK(b[0] == 0);
    CHECK(c[0] == 0);
}

// ===== authenticator =====

// Before a successful login Current_authenticator is nil, so this returns 0
// with a zero-length buffer. The buffer is still a real new[] allocation and
// must be freed with dwyco_free_array, which the header does not mention.
static void
authenticator_returns_freeable_buffer(void)
{
    const char *a;
    int len = -1;
    int rc = dwyco_get_authenticator(&a, &len);
    CHECK(rc == 0 || rc == 1);
    CHECK(len >= 0);
    if (a)
        dwyco_free_array((char *)a);
}

// ===== state flags =====

static void
invisible_state_round_trip(void)
{
    int before = dwyco_get_invisible_state();
    dwyco_set_invisible_state(1);
    CHECK(dwyco_get_invisible_state() == 1);
    dwyco_set_invisible_state(0);
    CHECK(dwyco_get_invisible_state() == 0);
    dwyco_set_invisible_state(before);
    CHECK(dwyco_get_invisible_state() == before);
}

// dwyco_set_initial_invis is documented as "only before dwyco_init" and its
// body is entirely commented out in the library, so calling it after init
// must be a harmless no-op rather than a crash.
static void
initial_invis_is_a_noop(void)
{
    int before = dwyco_get_invisible_state();
    dwyco_set_initial_invis(1);
    CHECK(dwyco_get_invisible_state() == before);
    dwyco_set_initial_invis(0);
    CHECK(dwyco_get_invisible_state() == before);
}

static void
suspend_resume_round_trip(void)
{
    CHECK(dwyco_get_suspend_state() == 0);
    dwyco_suspend();
    CHECK(dwyco_get_suspend_state() == 1);
    dwyco_resume();
    CHECK(dwyco_get_suspend_state() == 0);
    // Both are idempotent.
    dwyco_resume();
    CHECK(dwyco_get_suspend_state() == 0);
}

// Both halves of the refresh_users flag are empty in the library
// (dlli.cpp: get returns a literal 0, set has its assignment commented out),
// so the getter must keep reporting 0 no matter what is set.
static void
refresh_users_is_a_noop(void)
{
    dwyco_set_refresh_users(1);
    CHECK(dwyco_get_refresh_users() == 0);
    dwyco_set_refresh_users(0);
    CHECK(dwyco_get_refresh_users() == 0);
}

// The default is 1, so probe both directions rather than assuming 0.
static void
moron_dork_round_trip(void)
{
    int before = dwyco_get_moron_dork_mode();
    dwyco_set_moron_dork_mode(0);
    CHECK(dwyco_get_moron_dork_mode() == 0);
    dwyco_set_moron_dork_mode(1);
    CHECK(dwyco_get_moron_dork_mode() == 1);
    dwyco_set_moron_dork_mode(before);
    CHECK(dwyco_get_moron_dork_mode() == before);
}

// ===== contacts =====

// This is the one genuinely multi-column list the server-free tests can
// reach: there is no dwyco_list_append_col, so multi-column lists only come
// out of real API calls. dwyco_get_contact_list hands back a
// name/phone/email matrix, which is what lr_col_* exist for.
static void
contacts_round_trip(void)
{
    dwyco_clear_contact_list();

    CHECK(dwyco_add_contact("Alice", "555-1111", "alice@example.com") != 0);
    CHECK(dwyco_add_contact("Bob", "555-2222", "") != 0);

    DWYCO_LIST l = 0;
    CHECK(dwyco_get_contact_list(&l) != 0);
    if (!l)
        return;

    CHECK(lr_rows(l, 2));
    CHECK(lr_cols(l, 3));

    CHECK(lr_col_str(l, 0, DWYCO_CONTACT_LIST_NAME, "Alice", 5));
    CHECK(lr_col_str(l, 0, DWYCO_CONTACT_LIST_PHONE, "555-1111", 8));
    CHECK(lr_col_str(l, 0, DWYCO_CONTACT_LIST_EMAIL, "alice@example.com", 17));

    CHECK(lr_col_str(l, 1, DWYCO_CONTACT_LIST_NAME, "Bob", 3));
    CHECK(lr_col_str(l, 1, DWYCO_CONTACT_LIST_PHONE, "555-2222", 8));
    // An empty email is stored as an empty STRING, not as nil:
    // dwyco_add_contact does v[2] = email unconditionally.
    CHECK(lr_col_str(l, 1, DWYCO_CONTACT_LIST_EMAIL, "", 0));

    dwyco_list_release(l);
}

static void
contacts_clear(void)
{
    dwyco_add_contact("Temp", "555-3333", "temp@example.com");
    dwyco_clear_contact_list();

    DWYCO_LIST l = 0;
    CHECK(dwyco_get_contact_list(&l) != 0);
    if (!l)
        return;
    CHECK(lr_rows(l, 0));
    dwyco_list_release(l);
}

// ===== aux string =====

// get_aux_string is the only one of these that validates its out-pointers,
// and it returns 0 until something has been set. The returned buffer is
// NUL-terminated AND length-reported, and must be freed with
// dwyco_free_array as the header says.
static void
aux_string_round_trip(void)
{
    static const char *k_set = "dwytest-aux-value";
    dwyco_set_aux_string(k_set);

    const char *s;
    int len = -1;
    int rc = dwyco_get_aux_string(&s, &len);
    CHECK(rc != 0);
    CHECK(len == (int)strlen(k_set));
    CHECK(s != 0);
    if (s) {
        // Both a length and a NUL terminator are provided.
        CHECK(memcmp(s, k_set, strlen(k_set)) == 0);
        CHECK(s[len] == 0);
        dwyco_free_array((char *)s);
    }
}

// ===== debug / diagnostics =====

// These are no-ops or logging-only, so the test is that they do not crash.
//
// Note dwyco_app_debug1/2 look printf-shaped but their data arguments are
// declared int, not varargs -- passing a string does not compile.
static void
debug_helpers_do_not_crash(void)
{
    dwyco_trace_init();
    dwyco_field_debug("field", 1);
    dwyco_add_entropy_timer("entropy", 7);
    dwyco_app_debug1(__FILE__, __LINE__, "test %s %d", 65, 49);
    dwyco_app_debug2(__FILE__, __LINE__, "test %s %s %s %s %s", 1, 2, 3, 4, 5);
    // debug_dump writes to the log; just make sure it survives.
    dwyco_debug_dump();
}

// ===== harness =====

static int
init_test(void)
{
    char sys_dir[512], tmp_dir[512];
    snprintf(sys_dir, sizeof(sys_dir), "%s/sys", g_dir);
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", g_dir);
    mkdir(g_dir, 0755);
    mkdir(sys_dir, 0755);
    mkdir(tmp_dir, 0755);
    install_app_files(g_dir);

    dwyco_set_fn_prefixes(sys_dir, g_dir, tmp_dir);
    dwyco_set_client_version("dwytest", 7);
    if (dwyco_init() == 0)
        return 0;
    test_bootstrap_profile("dwytest-settings", "dwytest settings account");
    dwyco_finish_startup();
    return 1;
}

int
main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--unknown-setting") == 0) {
        g_argv0 = argv[0];
        unknown_setting_child(argc > 2 ? argv[2] : "set");
        return 0;
    }
    g_argv0 = argv[0];
    setvbuf(stdout, 0, _IOLBF, 0);

    printf("Dwyco settings / codec / runtime state\n");

    if (!init_test()) {
        fprintf(stderr, "dwyco_init failed\n");
        return 1;
    }

    printf("\nSettings:\n");
    RUN(setting_int_round_trip);
    RUN(setting_string_round_trip);
    RUN(setting_groups);
    RUN(setting_rejected_names);
    RUN(setting_output_is_borrowed);
    RUN(unknown_setting_aborts);

    printf("\nCodec:\n");
    RUN(codec_data_round_trip);
    RUN(codec_data_coerces_booleans);

    printf("\nPrefixes:\n");
    RUN(fn_prefixes_round_trip);
    RUN(fn_prefixes_small_buffer_is_ignored);

    printf("\nAccount state:\n");
    RUN(authenticator_returns_freeable_buffer);
    RUN(invisible_state_round_trip);
    RUN(initial_invis_is_a_noop);
    RUN(suspend_resume_round_trip);
    RUN(refresh_users_is_a_noop);
    RUN(moron_dork_round_trip);

    printf("\nContacts:\n");
    RUN(contacts_round_trip);
    RUN(contacts_clear);

    printf("\nAux string:\n");
    RUN(aux_string_round_trip);

    printf("\nDebug:\n");
    RUN(debug_helpers_do_not_crash);

    dwyco_exit();

    printf("\nSummary: passed=%d failed=%d\n", g_pass, g_fail);
    if (g_fail) {
        printf("FAILED\n");
        return 1;
    }
    printf("All settings tests passed.\n");
    return 0;
}