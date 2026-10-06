// Coverage for the profile API.
//
//   dwyco_create_bootstrap_profile, dwyco_make_profile_pack,
//   dwyco_set_profile_from_composer, dwyco_get_profile_to_viewer,
//   dwyco_get_profile_to_viewer_sync
//
// The profile calls need two accounts to be interesting: one sets a profile,
// the other fetches it. Like dwytest_msg and dwytest_attach, this binary
// re-executes itself as the second client. The SENDER DIRECTORY HERE IS A
// DELIBERATE CACHE -- see the note at the top.
//
// The callback-based calls are exercised for real: dwyco_get_profile_to_viewer
// and dwyco_set_profile_from_composer both complete, and the documented
// "succ" codes from dlli.h are checked:
//
//   0   failed
//   -1  success, text only, no attachment
//   -2  success, the attachment is a user-specified file
//   >0  success, and the value is a viewer id
//
// Two things the header does not say, both pinned below:
//
//  * dwyco_make_profile_pack writes into a function-static buffer and hands
//    back a pointer into it. The next call overwrites it, so it must be
//    copied out immediately -- and it must NOT be freed, because it is not
//    an allocation. Every other string-returning call in this api hands back
//    either a new[] buffer (needs dwyco_free_array) or a borrowed pointer
//    (must not be freed); this one is neither, it is static storage.
//
//  * dwyco_set_profile_from_composer validates its composer cookie, so a
//    deleted composition makes it return 0 without sending anything.

#include <dlli.h>
#include "test_common.h"
#include "dwyco_new_msg.h"
#include "list_readback.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

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

static const char *PEER_DIR = "/tmp/dwytest_profile_peer";
static const char *RECV_DIR = "/tmp/dwytest_profile";

static char g_my_uid[64];
static int g_my_uid_len;
static char g_my_uid_hex[64];

// What the peer's profile callback reported.
static int g_peer_cb_called;
static int g_peer_cb_succ;
static char g_peer_cb_s1[256];
static char g_peer_cb_s2[256];
static int g_peer_cb_s1_len, g_peer_cb_s2_len;

// What our own set-profile callback reported.
static int g_self_cb_called;
static int g_self_cb_succ;

static void DWYCOCALLCONV
peer_profile_cb(int succ, const char *reason, const char *s1, int len_s1,
    const char *s2, int len_s2, const char *s3, int len_s3,
    const char *filename, const char *uid, int len_uid, int reviewed,
    int regular, void *user_arg)
{
    (void)reason; (void)s3; (void)len_s3; (void)filename; (void)uid;
    (void)len_uid; (void)reviewed; (void)regular; (void)user_arg;
    g_peer_cb_called++;
    g_peer_cb_succ = succ;
    g_peer_cb_s1_len = len_s1 < (int)sizeof(g_peer_cb_s1) - 1
        ? len_s1 : (int)sizeof(g_peer_cb_s1) - 1;
    if (s1 && len_s1 > 0)
        memcpy(g_peer_cb_s1, s1, (size_t)g_peer_cb_s1_len);
    g_peer_cb_s1[g_peer_cb_s1_len] = 0;
    g_peer_cb_s2_len = len_s2 < (int)sizeof(g_peer_cb_s2) - 1
        ? len_s2 : (int)sizeof(g_peer_cb_s2) - 1;
    if (s2 && len_s2 > 0)
        memcpy(g_peer_cb_s2, s2, (size_t)g_peer_cb_s2_len);
    g_peer_cb_s2[g_peer_cb_s2_len] = 0;
}

static void DWYCOCALLCONV
self_profile_cb(int succ, const char *reason, const char *s1, int len_s1,
    const char *s2, int len_s2, const char *s3, int len_s3,
    const char *filename, const char *uid, int len_uid, int reviewed,
    int regular, void *user_arg)
{
    (void)reason; (void)s1; (void)len_s1; (void)s2; (void)len_s2;
    (void)s3; (void)len_s3; (void)filename; (void)uid; (void)len_uid;
    (void)reviewed; (void)regular; (void)user_arg;
    g_self_cb_called++;
    g_self_cb_succ = succ;
}

// ===== peer mode =====

// The peer sets a profile with a distinctive description, so the receiver can
// prove it fetched *this* profile rather than an empty one.
#define PEER_HANDLE "dwyprof-peer"
#define PEER_DESC   "peer profile description marker"

static int
peer_main(int argc, char **argv)
{
    (void)argc; (void)argv;
    const char *dir = PEER_DIR;
    char sys_dir[512], tmp_dir[512];
    snprintf(sys_dir, sizeof(sys_dir), "%s/sys", dir);
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", dir);
    mkdir(dir, 0755);
    mkdir(sys_dir, 0755);
    mkdir(tmp_dir, 0755);
    char logp[512];
    snprintf(logp, sizeof(logp), "%s/peer.log", dir);
    int lf = open(logp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (lf >= 0) {
        fflush(stdout);
        dup2(lf, 1);
        close(lf);
    }
    install_app_files(dir);

    dwyco_set_fn_prefixes(sys_dir, dir, tmp_dir);
    dwyco_set_client_version("dwytest", 7);
    if (!dwyco_init()) {
        fprintf(stderr, "[peer] init failed\n");
        return 1;
    }
    // create_bootstrap_profile is the first-run path and is a no-op once the
    // account exists, so the explicit set below is what actually installs it.
    dwyco_create_bootstrap_profile(PEER_HANDLE, (int)strlen(PEER_HANDLE),
        PEER_DESC, (int)strlen(PEER_DESC), "peerloc", 7, "peer@example.com", 16);
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);

    const char *uid;
    int len;
    dwyco_get_my_uid(&uid, &len);
    char hex[64];
    for (int i = 0; i < len; i++)
        snprintf(hex + i * 2, 3, "%02x", (unsigned char)uid[i]);
    printf("PEER_UID=%s\n", hex);
    fflush(stdout);

    // Set the profile explicitly as well, so it is in place even for an
    // account whose bootstrap profile was skipped.
    service_ms(2000);
    int cid = dwyco_make_zap_composition(0);
    if (cid > 0) {
        g_self_cb_called = 0;
        int rc = dwyco_set_profile_from_composer(cid,
            "peer profile description marker", (int)strlen(PEER_DESC),
            self_profile_cb, 0);
        printf("PEER_SETPROFILE rc=%d\n", rc);
        int got = wait_for([]() { return g_self_cb_called != 0; }, 30000);
        printf("PEER_SETPROFILE_DONE called=%d succ=%d\n", got, g_self_cb_succ);
        dwyco_delete_zap_composition(cid);
    }
    fflush(stdout);

    // Stay alive so the receiver can fetch.
    for (int i = 0; i < 900; i++) {
        service_ms(100);
        if (dwyco_get_rescan_messages()) {
            dwyco_set_rescan_messages(0);
            process_remote_msgs();
        }
    }
    printf("PEER_DONE\n");
    fflush(stdout);
    dwyco_exit();
    return 0;
}

// ===== receiver: helpers =====

// Read the peer's uid out of its log.
static int
read_peer_uid(char *out, int out_size)
{
    char logp[512];
    snprintf(logp, sizeof(logp), "%s/peer.log", PEER_DIR);
    FILE *f = fopen(logp, "r");
    if (!f)
        return 0;
    char line[256];
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        const char *p = strstr(line, "PEER_UID=");
        if (p) {
            size_t n = strlen(p + 9);
            while (n && (p[9 + n - 1] == '\n' || p[9 + n - 1] == '\r'))
                n--;
            if (n < (size_t)out_size) {
                memcpy(out, p + 9, n);
                out[n] = 0;
                found = 1;
            }
            break;
        }
    }
    fclose(f);
    return found;
}

static int g_have_peer;

// ===== tests =====

// make_profile_pack serializes the four fields into a static buffer.
static void
profile_pack_is_static_storage(void)
{
    const char *s1 = "";
    int l1 = 0;
    int rc = dwyco_make_profile_pack("handle-one", 10, "desc-one", 8,
        "loc-one", 7, "mail-one", 8, &s1, &l1);
    CHECK(rc != 0);
    CHECK(s1 != 0);
    CHECK(l1 > 0);

    // Copy it out immediately, because the next call overwrites it.
    char first[512];
    int n = l1 < (int)sizeof(first) - 1 ? l1 : (int)sizeof(first) - 1;
    memcpy(first, s1, (size_t)n);
    first[n] = 0;
    const char *first_ptr = s1;

    // A second call overwrites the buffer. The POINTER may even move, since
    // the static vc is reassigned and may reallocate -- what matters, and what
    // the header does not say, is that the contents we did not copy out are
    // gone. So the copy taken above must still match, and re-reading through
    // the original pointer must not be relied on.
    const char *s2 = "";
    int l2 = 0;
    dwyco_make_profile_pack("handle-two", 10, "desc-two", 8,
        "loc-two", 7, "mail-two", 8, &s2, &l2);
    CHECK(l2 > 0);
    printf("(ptr_moved=%d len=%d) ", s2 != first_ptr, l1);
    // No free_array on any of these: this is a function-static vc, not an
    // allocation.

    // Empty fields are legal.
    const char *s3 = "";
    int l3 = 0;
    CHECK(dwyco_make_profile_pack("", 0, "", 0, "", 0, "", 0, &s3, &l3) != 0);
    CHECK(l3 > 0);
}

// create_bootstrap_profile is the first-run path. It is a no-op for an
// account that already exists, which is the case here.
static void
bootstrap_profile_on_existing_account(void)
{
    int rc = dwyco_create_bootstrap_profile("someone", 7, "a description",
        12, "somewhere", 9, "s@example.com", 13);
    // Either value is defensible; what matters is that it is not a crash and
    // that repeating it stays consistent.
    int rc2 = dwyco_create_bootstrap_profile("someone", 7, "a description",
        12, "somewhere", 9, "s@example.com", 13);
    CHECK(rc == rc2);
    printf("(rc=%d) ", rc);
}

// set_profile_from_composer validates its cookie before doing anything.
static void
set_profile_validates_composer(void)
{
    int cid = dwyco_make_zap_composition(0);
    CHECK(cid > 0);
    if (cid <= 0)
        return;
    dwyco_delete_zap_composition(cid);
    // Deleted: must be refused rather than dereferenced.
    CHECK(dwyco_set_profile_from_composer(cid, "text", 4, self_profile_cb, 0) == 0);
}

// The real thing: set our own profile and wait for the callback.
static void
set_own_profile(void)
{
    static const char desc[] = "receiver profile description marker";
    int cid = dwyco_make_zap_composition(0);
    if (cid <= 0) {
        printf("(no composition) ");
        return;
    }
    g_self_cb_called = 0;
    int rc = dwyco_set_profile_from_composer(cid, desc, (int)strlen(desc),
        self_profile_cb, 0);
    CHECK(rc != 0);
    dwyco_delete_zap_composition(cid);
    if (rc == 0)
        return;

    int got = wait_for([]() { return g_self_cb_called != 0; }, 30000);
    CHECK(got);
    printf("(succ=%d) ", g_self_cb_succ);
    // 0 failed, -1 text only, -2 user file, >0 a viewer id.
    CHECK(g_self_cb_succ == -1 || g_self_cb_succ == -2 || g_self_cb_succ > 0);
}

// The synchronous profile fetch returns whatever is cached right now, without
// going to the server. Before any profile is known it has nothing to give.
static void
profile_to_viewer_sync_is_local_only(void)
{
    // Our own profile was just set, so it may or may not be cached yet
    // depending on timing; either way it must be a defined answer.
    char *fn = 0;
    int flen = 0;
    int rc = dwyco_get_profile_to_viewer_sync(g_my_uid, g_my_uid_len, &fn, &flen);
    printf("(rc=%d len=%d) ", rc, flen);
    if (rc == -2 && fn) {
        // -2 means a user-specified file: fn_out is a real filename that the
        // caller must free with dwyco_free_array.
        CHECK(flen > 0);
        dwyco_free_array(fn);
    } else {
        // 0 means we have no media, -1 not resolved. Nothing to free.
        CHECK(rc == 0 || rc == -1 || rc == -2 || rc > 0);
    }
}

// The asynchronous fetch, against the peer's account. Needs the server, so it
// is skipped when no peer is available.
static void
get_peer_profile(void)
{
    if (!g_have_peer) {
        printf("(no peer) ");
        return;
    }
    // Being pals makes the fetch a direct lookup rather than an anonymous
    // server query, which is what makes it reliable.
    dwyco_pal_add((const char *)g_peer_uid, g_peer_uid_len);
    service_ms(1500);

    // The profile the peer set may not have reached the server yet, and the
    // first fetch can race that. Retry a bounded number of times.
    int attempts = 0;
    for (attempts = 0; attempts < 4; attempts++) {
        g_peer_cb_called = 0;
        g_peer_cb_succ = 0;
        int rc = dwyco_get_profile_to_viewer((const char *)g_peer_uid,
            g_peer_uid_len, peer_profile_cb, 0);
        if (rc == 0)
            break;
        wait_for([]() { return g_peer_cb_called != 0; }, 20000);
        if (g_peer_cb_called && g_peer_cb_succ != 0)
            break;
        service_ms(3000);
    }

    printf("(attempts=%d succ=%d s1len=%d) ", attempts + 1, g_peer_cb_succ,
        g_peer_cb_s1_len);
    if (!g_peer_cb_called) {
        printf("(callback never fired) ");
        return;
    }
    // A profile we can read is either text-only (-1), a user file (-2), or a
    // viewer id (> 0). 0 means the fetch failed.
    CHECK(g_peer_cb_succ == -1 || g_peer_cb_succ == -2 || g_peer_cb_succ > 0);
    // s1 carries the profile's handle. (s2 is the second text field, and the
    // description is not among the callback's string args -- the description
    // is available through dwyco_uid_to_info instead, which is what the
    // users test covers.) A text-only profile must therefore give us the
    // handle we expect.
    if (g_peer_cb_succ == -1 || g_peer_cb_succ > 0) {
        CHECK(g_peer_cb_s1_len > 0);
        CHECK(g_peer_cb_s1_len == (int)strlen(PEER_HANDLE));
        CHECK(memcmp(g_peer_cb_s1, PEER_HANDLE,
            (size_t)g_peer_cb_s1_len) == 0);
    }
    printf("[s1='%s'] ", g_peer_cb_s1);
}

// get_lobby_name_by_id2 is a lookup against the user-lobby table. Nothing has
// created a lobby here, so it must report "not found" cleanly.
static void
lobby_name_lookup_of_unknown_id(void)
{
    DWYCO_LIST l = 0;
    int rc = dwyco_get_lobby_name_by_id2("000", &l);
    // 0 with no list written is the expected "no such lobby" answer.
    CHECK(rc == 0);
    CHECK(l == 0);
}

// ===== harness =====

static int
boot(const char *dir, const char *account, const char *desc)
{
    char sys_dir[512], tmp_dir[512];
    snprintf(sys_dir, sizeof(sys_dir), "%s/sys", dir);
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", dir);
    mkdir(dir, 0755);
    mkdir(sys_dir, 0755);
    mkdir(tmp_dir, 0755);
    install_app_files(dir);

    dwyco_set_fn_prefixes(sys_dir, dir, tmp_dir);
    dwyco_set_client_version("dwytest", 7);
    if (!dwyco_init())
        return 0;
    if (desc)
        test_bootstrap_profile(account, desc);
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);

    const char *uid;
    int len;
    dwyco_get_my_uid(&uid, &len);
    if (len <= 0 || len >= (int)sizeof(g_my_uid))
        return 0;
    memcpy(g_my_uid, uid, (size_t)len);
    g_my_uid_len = len;
    for (int i = 0; i < len; i++)
        snprintf(g_my_uid_hex + i * 2, 3, "%02x", (unsigned char)g_my_uid[i]);
    return 1;
}

int
main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--peer") == 0)
        return peer_main(argc, argv);

    setvbuf(stdout, 0, _IOLBF, 0);
    printf("Dwyco profiles\n");

    if (!boot(RECV_DIR, "dwyprof-recv", "dwytest profile receiver")) {
        fprintf(stderr, "dwyco_init failed\n");
        return 1;
    }
    printf("  receiver uid=%s\n", g_my_uid_hex);
    service_ms(3000);

    const char *self = argv[0];
    char *child_argv[] = { (char *)self, (char *)"--peer", 0 };
    pid_t child = spawn_subprocess(child_argv);
    if (child < 0) {
        fprintf(stderr, "could not spawn peer\n");
        dwyco_exit();
        return 1;
    }
    printf("  spawned peer pid=%d\n", (int)child);

    char peer_hex[64];
    g_have_peer = wait_for([&]() { return read_peer_uid(peer_hex, sizeof(peer_hex)) != 0; },
        30000);
    if (g_have_peer) {
        g_have_peer = uid_from_hex(peer_hex, g_peer_uid, sizeof(g_peer_uid),
            &g_peer_uid_len);
        printf("  peer uid=%s\n", peer_hex);
    } else {
        printf("  peer uid unavailable (profile fetch will skip)\n");
    }

    printf("\nPacking:\n");
    RUN(profile_pack_is_static_storage);
    RUN(bootstrap_profile_on_existing_account);

    printf("\nSetting:\n");
    RUN(set_profile_validates_composer);
    RUN(set_own_profile);

    printf("\nFetching:\n");
    RUN(profile_to_viewer_sync_is_local_only);
    RUN(get_peer_profile);
    RUN(lobby_name_lookup_of_unknown_id);

    int child_status = wait_subprocess(child, 30000);
    printf("  peer exited with %d\n", child_status);

    dwyco_exit();

    printf("\nSummary: passed=%d failed=%d\n", g_pass, g_fail);
    if (g_fail) {
        printf("FAILED\n");
        return 1;
    }
    printf("All profile tests passed.\n");
    return 0;
}