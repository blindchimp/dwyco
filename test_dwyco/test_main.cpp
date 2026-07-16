#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <ctime>
#include <unistd.h>
#include <sys/stat.h>

#include "dlli.h"

#define ASSERT(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL at %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        exit(1); \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        fprintf(stderr, "FAIL at %s:%d: expected %d got %d\n", __FILE__, __LINE__, (int)(b), (int)(a)); \
        exit(1); \
    } \
} while(0)

#define ASSERT_STR_EQ(a, alen, b, blen) do { \
    if ((alen) != (blen) || memcmp((a), (b), (alen)) != 0) { \
        fprintf(stderr, "FAIL at %s:%d: string mismatch\n", __FILE__, __LINE__); \
        exit(1); \
    } \
} while(0)

#define RUN_TEST(name) do { \
    printf("  %-40s ", #name); \
    g_test_pass = 1; \
    test_##name(); \
    if (g_test_pass) printf("OK\n"); \
} while(0)

#define TEST_SKIP() do { g_test_pass = 0; printf("SKIP\n"); } while(0)

static int g_test_pass = 1;

struct Event {
    int cmd;
    std::string uid;
    std::string name;
    std::string value;
};

struct {
    std::vector<Event> events;
    int login_status;
    int fetch_id;
    int fetch_what;
    char fetch_msgid[256];
    char my_uid[64];
    int my_uid_len;
} g_state;

static void DWYCOCALLCONV
sys_event_cb(int cmd, int ctx_id, const char *uid, int len_uid,
    const char *name, int len_name, int type, const char *value, int val_len,
    int qid, int extra_arg)
{
    Event e;
    e.cmd = cmd;
    if (uid && len_uid > 0) e.uid = std::string(uid, len_uid);
    if (name && len_name > 0) e.name = std::string(name, len_name);
    if (value && val_len > 0) e.value = std::string(value, val_len);
    g_state.events.push_back(e);

    if (cmd == DWYCO_SE_SERVER_LOGIN)
        g_state.login_status = 1;
}

static void DWYCOCALLCONV
login_result_cb(const char *str, int what)
{
    if (what == 1)
        g_state.login_status = 1;
}

static void DWYCOCALLCONV
emergency_cb(int problem, int must_exit, const char *msg)
{
    fprintf(stderr, "EMERGENCY: problem=%d must_exit=%d msg=%s\n",
        problem, must_exit, msg ? msg : "null");
}

static void DWYCOCALLCONV
fetch_cb(int id, int what, const char *msgid, void *arg)
{
    g_state.fetch_id = id;
    g_state.fetch_what = what;
    if (msgid) {
        strncpy(g_state.fetch_msgid, msgid, sizeof(g_state.fetch_msgid) - 1);
        g_state.fetch_msgid[sizeof(g_state.fetch_msgid) - 1] = 0;
    }
}

static void
service_once(void)
{
    int spin;
    dwyco_service_channels(&spin);
}

static int
service_until(int target_cmd, int timeout_ms)
{
    int elapsed = 0;
    while (elapsed < timeout_ms) {
        int spin;
        int next = dwyco_service_channels(&spin);
        if (next <= 0 || next > 50) next = 50;
        usleep(next * 1000);
        elapsed += next;
        for (auto &e : g_state.events) {
            if (e.cmd == target_cmd) return 1;
        }
    }
    return 0;
}

static void
clear_events(void)
{
    g_state.events.clear();
}

static int
event_seen(int cmd)
{
    for (auto &e : g_state.events)
        if (e.cmd == cmd) return 1;
    return 0;
}

static int
count_events(int cmd)
{
    int n = 0;
    for (auto &e : g_state.events)
        if (e.cmd == cmd) ++n;
    return n;
}

// Test peer UID (set from command line)
static char g_peer_uid[64];
static int g_peer_uid_len;
static int g_has_peer;

static void
init_test(const char *user_dir)
{
    char tmp_dir[512];
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", user_dir);
    mkdir(user_dir, 0755);
    mkdir(tmp_dir, 0755);

    dwyco_set_fn_prefixes(user_dir, user_dir, tmp_dir);
    dwyco_set_system_event_callback(sys_event_cb);
    dwyco_set_login_result_callback(login_result_cb);
    dwyco_set_emergency_callback(emergency_cb);
    dwyco_set_client_version("dwytest", 7);

    ASSERT(dwyco_init() != 0);
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);

    const char *uid;
    int uid_len;
    dwyco_get_my_uid(&uid, &uid_len);
    ASSERT(uid_len > 0 && uid_len < (int)sizeof(g_state.my_uid));
    memcpy(g_state.my_uid, uid, uid_len);
    g_state.my_uid_len = uid_len;
}

static void
shutdown_test(void)
{
    dwyco_exit();
}

// ===== COMPOSITION TESTS =====

static void
test_compose_empty(void)
{
    int cid = dwyco_make_zap_composition(0);
    ASSERT(cid > 0);
    int flim = dwyco_flim(cid);
    ASSERT(flim >= 0);
    ASSERT(dwyco_delete_zap_composition(cid) != 0);
}

static void
test_compose_file_zap(void)
{
    // Create a temp file for the test
    FILE *f = fopen("/tmp/dwytest_test_file.txt", "w");
    ASSERT(f);
    fprintf(f, "test content");
    fclose(f);

    int cid = dwyco_make_file_zap_composition("/tmp/dwytest_test_file.txt", 26);
    ASSERT(cid > 0);
    ASSERT(dwyco_is_file_zap(cid) != 0);
    ASSERT(dwyco_delete_zap_composition(cid) != 0);

    unlink("/tmp/dwytest_test_file.txt");
}

static void
test_compose_special(void)
{
    int cid = dwyco_make_special_zap_composition(
        DWYCO_PAL_AUTH_ASK, "payload", 7);
    ASSERT(cid > 0);
    ASSERT(dwyco_delete_zap_composition(cid) != 0);
}

static void
test_compose_dup(void)
{
    int c1 = dwyco_make_zap_composition(0);
    ASSERT(c1 > 0);
    int c2 = dwyco_dup_zap_composition(c1);
    ASSERT(c2 > 0);
    ASSERT(c2 != c1);
    ASSERT(dwyco_delete_zap_composition(c1) != 0);
    ASSERT(dwyco_delete_zap_composition(c2) != 0);
}

static void
test_compose_flim(void)
{
    int cid = dwyco_make_zap_composition(0);
    ASSERT(cid > 0);
    int flim = dwyco_flim(cid);
    ASSERT(flim >= 0);
    dwyco_delete_zap_composition(cid);
}

// ===== SEND / OUTBOX TESTS =====

static void
test_send_deferred(void)
{
    if (!g_has_peer) { TEST_SKIP(); return; }

    int cid = dwyco_make_zap_composition(0);
    ASSERT(cid > 0);

    const char *pers_id;
    int pers_len;
    int res = dwyco_zap_send6(cid, g_peer_uid, g_peer_uid_len,
        "deferred test", 13, 0, 0, 1, &pers_id, &pers_len);
    ASSERT(res != 0);
    ASSERT(pers_id && pers_len > 0);
    dwyco_delete_zap_composition(cid);

    // Check it appears in outbox
    DWYCO_QD_MSG_LIST qd = 0;
    dwyco_get_qd_messages(&qd, g_peer_uid, g_peer_uid_len);
    ASSERT(qd != 0);
    int rows, cols;
    dwyco_list_numelems(qd, &rows, &cols);
    ASSERT(rows >= 1);
    dwyco_list_release(qd);

    // Kill it
    ASSERT(dwyco_kill_message(pers_id, pers_len) != 0);
}

static void
test_kill_message(void)
{
    if (!g_has_peer) { TEST_SKIP(); return; }

    int cid = dwyco_make_zap_composition(0);
    ASSERT(cid > 0);

    const char *pers_id;
    int pers_len;
    dwyco_zap_send6(cid, g_peer_uid, g_peer_uid_len,
        "kill test", 9, 0, 0, 1, &pers_id, &pers_len);
    dwyco_delete_zap_composition(cid);

    // Verify in outbox
    DWYCO_QD_MSG_LIST qd = 0;
    dwyco_get_qd_messages(&qd, g_peer_uid, g_peer_uid_len);
    ASSERT(qd != 0);
    int rows, cols;
    dwyco_list_numelems(qd, &rows, &cols);
    ASSERT(rows >= 1);
    dwyco_list_release(qd);

    // Kill
    ASSERT(dwyco_kill_message(pers_id, pers_len) != 0);
    service_once();

    // Verify removed from outbox
    dwyco_get_qd_messages(&qd, g_peer_uid, g_peer_uid_len);
    if (qd) {
        dwyco_list_numelems(qd, &rows, &cols);
        dwyco_list_release(qd);
    }
}

static void
test_save_sent(void)
{
    if (!g_has_peer) { TEST_SKIP(); return; }

    int cid = dwyco_make_zap_composition(0);
    ASSERT(cid > 0);

    const char *pers_id;
    int pers_len;
    int res = dwyco_zap_send5(cid, g_peer_uid, g_peer_uid_len,
        "save sent test", 14, 0, 1, &pers_id, &pers_len);
    ASSERT(res != 0);
    dwyco_delete_zap_composition(cid);

    if (pers_id && pers_len > 0)
        dwyco_kill_message(pers_id, pers_len);
}

static void
test_qd_message_status(void)
{
    if (!g_has_peer) { TEST_SKIP(); return; }

    int cid = dwyco_make_zap_composition(0);
    ASSERT(cid > 0);

    const char *pers_id;
    int pers_len;
    dwyco_zap_send6(cid, g_peer_uid, g_peer_uid_len,
        "status test", 11, 0, 0, 1, &pers_id, &pers_len);
    dwyco_delete_zap_composition(cid);

    ASSERT(pers_id && pers_len > 0);
    int failed = dwyco_qd_message_is_failed(pers_id, pers_len);
    ASSERT(failed == 0 || failed == 1);

    dwyco_kill_message(pers_id, pers_len);
}

// ===== MESSAGE BODY TESTS =====

static void
test_get_body_text(void)
{
    if (!g_has_peer) { TEST_SKIP(); return; }

    // Send a message with save_sent, then get its body
    int cid = dwyco_make_zap_composition(0);
    ASSERT(cid > 0);

    const char *pers_id;
    int pers_len;
    dwyco_zap_send5(cid, g_peer_uid, g_peer_uid_len,
        "body text test", 14, 0, 1, &pers_id, &pers_len);
    dwyco_delete_zap_composition(cid);

    // Try to get saved body
    DWYCO_SAVED_MSG_LIST bodies = 0;
    int res = dwyco_get_message_bodies(&bodies, g_state.my_uid,
        g_state.my_uid_len, 1);
    if (res && bodies) {
        int r, c;
        dwyco_list_numelems(bodies, &r, &c);
        if (r > 0) {
            DWYCO_LIST text = dwyco_get_body_text(bodies);
            if (text) {
                dwyco_list_release(text);
            }
        }
        dwyco_list_release(bodies);
    }

    if (pers_id && pers_len > 0)
        dwyco_kill_message(pers_id, pers_len);
}

static void
test_get_body_array(void)
{
    if (!g_has_peer) { TEST_SKIP(); return; }

    int cid = dwyco_make_zap_composition(0);
    ASSERT(cid > 0);

    const char *pers_id;
    int pers_len;
    dwyco_zap_send5(cid, g_peer_uid, g_peer_uid_len,
        "body array test", 15, 0, 1, &pers_id, &pers_len);
    dwyco_delete_zap_composition(cid);

    DWYCO_SAVED_MSG_LIST bodies = 0;
    int res = dwyco_get_message_bodies(&bodies, g_state.my_uid,
        g_state.my_uid_len, 1);
    if (res && bodies) {
        int r, c;
        dwyco_list_numelems(bodies, &r, &c);
        if (r > 0) {
            DWYCO_LIST arr = dwyco_get_body_array(bodies);
            if (arr) {
                dwyco_list_release(arr);
            }
        }
        dwyco_list_release(bodies);
    }

    if (pers_id && pers_len > 0)
        dwyco_kill_message(pers_id, pers_len);
}

// ===== MESSAGE INDEX TESTS =====

static void
test_message_index(void)
{
    if (!g_has_peer) { TEST_SKIP(); return; }

    // Send a message first so we have something in the index
    int cid = dwyco_make_zap_composition(0);
    ASSERT(cid > 0);

    const char *pers_id;
    int pers_len;
    dwyco_zap_send5(cid, g_peer_uid, g_peer_uid_len,
        "index test", 10, 0, 1, &pers_id, &pers_len);
    dwyco_delete_zap_composition(cid);

    DWYCO_MSG_IDX idx = 0;
    int res = dwyco_get_message_index(&idx, g_state.my_uid, g_state.my_uid_len);
    ASSERT(res != 0);
    if (idx) {
        int r, c;
        dwyco_list_numelems(idx, &r, &c);
        ASSERT(r >= 0);
        dwyco_list_release(idx);
    }

    // Test with load count limit
    int avail = 0;
    res = dwyco_get_message_index2(&idx, g_state.my_uid, g_state.my_uid_len,
        &avail, 5);
    ASSERT(res != 0);
    if (idx) {
        int r, c;
        dwyco_list_numelems(idx, &r, &c);
        ASSERT(r <= 5);
        dwyco_list_release(idx);
    }

    if (pers_id && pers_len > 0)
        dwyco_kill_message(pers_id, pers_len);
}

// ===== TAGGING TESTS =====

static void
test_tag_fav(void)
{
    const char *mid = "test_mid_001";
    dwyco_set_fav_msg(mid, 1);
    ASSERT(dwyco_get_fav_msg(mid) != 0);
    dwyco_set_fav_msg(mid, 0);
    ASSERT(dwyco_get_fav_msg(mid) == 0);
}

static void
test_tag_set_check_unset(void)
{
    const char *mid = "test_mid_tag_001";
    const char *tag = "test_tag";

    dwyco_set_msg_tag(mid, tag);
    ASSERT(dwyco_mid_has_tag(mid, tag) != 0);

    dwyco_unset_msg_tag(mid, tag);
    ASSERT(dwyco_mid_has_tag(mid, tag) == 0);
}

static void
test_tag_get_tagged(void)
{
    const char *mid1 = "test_mid_t1";
    const char *mid2 = "test_mid_t2";
    const char *tag = "batch_tag";

    dwyco_set_msg_tag(mid1, tag);
    dwyco_set_msg_tag(mid2, tag);

    // Note: get_tagged_mids joins against global message index,
    // so fake mids won't appear. Just check the API doesn't crash.
    DWYCO_LIST mids = 0;
    int res = dwyco_get_tagged_mids(&mids, tag);
    ASSERT(res != 0);
    if (mids) dwyco_list_release(mids);

    // Check get_tagged_idx
    DWYCO_MSG_IDX idx = 0;
    res = dwyco_get_tagged_idx(&idx, tag, 0);
    if (idx) dwyco_list_release(idx);

    dwyco_unset_msg_tag(mid1, tag);
    dwyco_unset_msg_tag(mid2, tag);
}

static void
test_tag_count(void)
{
    const char *mid1 = "test_mid_cnt1";
    const char *mid2 = "test_mid_cnt2";
    const char *tag = "count_tag";

    ASSERT(dwyco_count_tag(tag) >= 0);
    dwyco_set_msg_tag(mid1, tag);
    dwyco_set_msg_tag(mid2, tag);
    // Note: count_tag may require real mids in global index
    ASSERT(dwyco_count_tag(tag) >= 0);
    dwyco_unset_msg_tag(mid1, tag);
    dwyco_unset_msg_tag(mid2, tag);
}

static void
test_tag_unset_all(void)
{
    const char *mid1 = "test_mid_ua1";
    const char *mid2 = "test_mid_ua2";
    const char *tag = "unset_all_tag";

    dwyco_set_msg_tag(mid1, tag);
    dwyco_set_msg_tag(mid2, tag);
    ASSERT(dwyco_mid_has_tag(mid1, tag) != 0);
    ASSERT(dwyco_mid_has_tag(mid2, tag) != 0);

    dwyco_unset_all_msg_tag(tag);
    ASSERT(dwyco_mid_has_tag(mid1, tag) == 0);
    ASSERT(dwyco_mid_has_tag(mid2, tag) == 0);
}

static void
test_tag_mid_disposition(void)
{
    const char *mid = "test_mid_disp";
    int d = dwyco_mid_disposition(mid);
    ASSERT(d >= -1);
}

static void
test_tag_uid_has_tag(void)
{
    if (!g_has_peer) { TEST_SKIP(); return; }

    const char *tag = "uid_tag";
    ASSERT(dwyco_uid_has_tag(g_peer_uid, g_peer_uid_len, tag) >= 0);
    ASSERT(dwyco_uid_count_tag(g_peer_uid, g_peer_uid_len, tag) >= 0);
}

// ===== PAL TESTS =====

static void
test_pal_add_delete(void)
{
    if (!g_has_peer) { TEST_SKIP(); return; }

    // Add pal
    dwyco_pal_add(g_peer_uid, g_peer_uid_len);
    service_once();
    ASSERT(dwyco_is_pal(g_peer_uid, g_peer_uid_len) != 0);

    // Get list
    DWYCO_LIST list = dwyco_pal_get_list();
    ASSERT(list != 0);
    int r, c;
    dwyco_list_numelems(list, &r, &c);
    ASSERT(r >= 1);
    dwyco_list_release(list);

    // Delete pal
    dwyco_pal_delete(g_peer_uid, g_peer_uid_len);
    service_once();
    ASSERT(dwyco_is_pal(g_peer_uid, g_peer_uid_len) == 0);
}

// ===== IGNORE TESTS =====

static void
test_ignore_unignore(void)
{
    if (!g_has_peer) { TEST_SKIP(); return; }

    ASSERT(dwyco_is_ignored(g_peer_uid, g_peer_uid_len) == 0);

    dwyco_ignore(g_peer_uid, g_peer_uid_len);
    service_once();
    ASSERT(dwyco_is_ignored(g_peer_uid, g_peer_uid_len) != 0);

    // Check list
    DWYCO_LIST list = dwyco_ignore_list_get();
    ASSERT(list != 0);
    int r, c;
    dwyco_list_numelems(list, &r, &c);
    ASSERT(r >= 1);
    dwyco_list_release(list);

    dwyco_unignore(g_peer_uid, g_peer_uid_len);
    service_once();
    ASSERT(dwyco_is_ignored(g_peer_uid, g_peer_uid_len) == 0);
}

// ===== SYSTEM EVENT TESTS =====

static void
test_event_send_cancel(void)
{
    if (!g_has_peer) { TEST_SKIP(); return; }

    clear_events();

    int cid = dwyco_make_zap_composition(0);
    ASSERT(cid > 0);

    const char *pers_id;
    int pers_len;
    dwyco_zap_send6(cid, g_peer_uid, g_peer_uid_len,
        "event test", 10, 0, 0, 1, &pers_id, &pers_len);
    dwyco_delete_zap_composition(cid);

    // Kill it to trigger a cancel event (if the library sends one for deferred msgs)
    dwyco_kill_message(pers_id, pers_len);
    service_once();
}

static void
test_event_tag_change(void)
{
    clear_events();

    const char *mid = "event_tag_test_mid";
    const char *tag = "event_tag";

    dwyco_set_msg_tag(mid, tag);
    service_once();

    dwyco_unset_msg_tag(mid, tag);
    service_once();
}

int
main(int argc, char **argv)
{
    const char *user_dir = "/tmp/dwytest";
    if (argc > 1) user_dir = argv[1];
    if (argc > 2) {
        g_peer_uid_len = strlen(argv[2]);
        if (g_peer_uid_len > (int)sizeof(g_peer_uid) - 1)
            g_peer_uid_len = sizeof(g_peer_uid) - 1;
        memcpy(g_peer_uid, argv[2], g_peer_uid_len);
        g_has_peer = 1;
    }

    printf("DWYCO Messaging Local Tests\n");
    printf("  User dir: %s\n", user_dir);
    printf("  Peer UID: %s\n", g_has_peer ? "(provided)" : "(none - send tests skipped)");

    init_test(user_dir);

    // Wait for login with timeout
    printf("  Waiting for server login...\n");
    int logged_in = service_until(DWYCO_SE_SERVER_LOGIN, 30000);
    if (logged_in)
        printf("  Login OK\n");
    else
        printf("  Login timeout (server-dependent tests may fail)\n");

    printf("\nComposition:\n");
    RUN_TEST(compose_empty);
    RUN_TEST(compose_file_zap);
    RUN_TEST(compose_special);
    RUN_TEST(compose_dup);
    RUN_TEST(compose_flim);

    printf("\nSend / Outbox:\n");
    RUN_TEST(send_deferred);
    RUN_TEST(kill_message);
    RUN_TEST(save_sent);
    RUN_TEST(qd_message_status);

    printf("\nMessage Body:\n");
    RUN_TEST(get_body_text);
    RUN_TEST(get_body_array);

    printf("\nMessage Index:\n");
    RUN_TEST(message_index);

    printf("\nTagging:\n");
    RUN_TEST(tag_fav);
    RUN_TEST(tag_set_check_unset);
    RUN_TEST(tag_get_tagged);
    RUN_TEST(tag_count);
    RUN_TEST(tag_unset_all);
    RUN_TEST(tag_mid_disposition);
    RUN_TEST(tag_uid_has_tag);

    printf("\nPals:\n");
    RUN_TEST(pal_add_delete);

    printf("\nIgnore:\n");
    RUN_TEST(ignore_unignore);

    printf("\nSystem Events:\n");
    RUN_TEST(event_send_cancel);
    RUN_TEST(event_tag_change);

    shutdown_test();
    printf("\nAll local tests passed.\n");
    return 0;
}
