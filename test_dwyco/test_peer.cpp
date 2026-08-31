#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>

#include "dlli.h"
#include "dwycolist2.h"
#include "test_common.h"
#include "dwyco_new_msg.h"

#define ASSERT(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL at %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        exit(1); \
    } \
} while(0)

// FNV-1a 64-bit hash over a file's bytes. Returns the file size on
// success, or -1 if the file cannot be opened/read.
static long
file_hash(const char *path, unsigned long long *hash_out)
{
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    unsigned long long h = 14695981039346656037ULL;
    long size = 0;
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        size += (long)n;
        for (size_t i = 0; i < n; ++i) {
            h ^= (unsigned char)buf[i];
            h *= 1099511628211ULL;
        }
    }
    fclose(f);
    if (hash_out) *hash_out = h;
    return size;
}

// Event recording
struct Event {
    int cmd;
    std::string uid;
    std::string value;
};
static std::vector<Event> g_events;
static int g_login_done;

static void DWYCOCALLCONV
sys_event_cb(int cmd, int ctx_id, const char *uid, int len_uid,
    const char *name, int len_name, int type, const char *value, int val_len,
    int qid, int extra_arg)
{
    Event e;
    e.cmd = cmd;
    if (uid && len_uid > 0) e.uid = std::string(uid, len_uid);
    if (value && val_len > 0) e.value = std::string(value, val_len);
    g_events.push_back(e);
    if (cmd == DWYCO_SE_SERVER_LOGIN) g_login_done = 1;
}

static void DWYCOCALLCONV
login_cb(const char *str, int what)
{
    // 1 = login ok, 2 = new account created (and logged in)
    if (what == 1 || what == 2) g_login_done = 1;
}

static void DWYCOCALLCONV
emergency_cb(int problem, int must_exit, const char *msg)
{
    fprintf(stderr, "EMERGENCY: problem=%d must_exit=%d msg=%s\n",
        problem, must_exit, msg ? msg : "null");
}

static void DWYCOCALLCONV
service_once(void)
{
    int spin;
    dwyco_service_channels(&spin);
}

static int
wait_login(int timeout_ms)
{
    int elapsed = 0;
    while (elapsed < timeout_ms) {
        int spin, next;
        next = dwyco_service_channels(&spin);
        if (next <= 0 || next > 50) next = 50;
        usleep(next * 1000);
        elapsed += next;
        if (g_login_done) return 1;
    }
    return 0;
}

static void
clear_events(void)
{
    g_events.clear();
}

static void
init(const char *user_dir, const char *account_name)
{
    char sys_dir[512], tmp_dir[512];
    snprintf(sys_dir, sizeof(sys_dir), "%s/sys", user_dir);
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", user_dir);
    mkdir(sys_dir, 0755);
    mkdir(user_dir, 0755);
    mkdir(tmp_dir, 0755);
    install_app_files(user_dir);

    dwyco_set_fn_prefixes(sys_dir, user_dir, tmp_dir);
    dwyco_set_system_event_callback(sys_event_cb);
    dwyco_set_login_result_callback(login_cb);
    dwyco_set_emergency_callback(emergency_cb);
    dwyco_set_client_version("dwytest", 7);

    ASSERT(dwyco_init() != 0);
    std::string desc = std::string(account_name) + " test account (" + user_dir + ")";
    test_bootstrap_profile(account_name, desc.c_str());
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);

    printf("  Waiting for login...\n");
    if (!wait_login(5000)) {
        fprintf(stderr, "Login timeout\n");
        exit(1);
    }
    printf("  Login OK\n");
}

static int g_server_mode;

static void
shutdown(void)
{
    dwyco_exit();
}

// Wait up to timeout_ms for the given peer (binary uid) to come online,
// servicing channels so the discovery/broadcast state stays current.
// Returns 1 once the peer is online, 0 on timeout.
static int
wait_peer_online(const char *peer_uid, int peer_uid_len, int timeout_ms)
{
    int elapsed = 0;
    while (elapsed < timeout_ms) {
        int spin, next;
        next = dwyco_service_channels(&spin);
        if (next <= 0 || next > 50) next = 50;
        usleep(next * 1000);
        elapsed += next;
        if (dwyco_uid_online(peer_uid, peer_uid_len)) return 1;
    }
    return 0;
}

// ===== SEND MODE =====
// Usage: dwytest_peer send <user_dir> <peer_uid> <text> [no_forward] [attachment_path]
static int
mode_send(int argc, char **argv)
{
    if (argc < 5) {
        fprintf(stderr, "Usage: dwytest_peer send <user_dir> <peer_uid_hex> <text> [no_forward] [attachment_path]\n");
        return 1;
    }
    const char *user_dir = argv[2];
    const char *peer_hex = argv[3];
    const char *msg_text = argv[4];
    int no_forward = (argc > 5) ? atoi(argv[5]) : 0;
    const char *att_path = (argc > 6 && strlen(argv[6]) > 0) ? argv[6] : 0;

    // Convert hex peer UID to binary
    int peer_hex_len = strlen(peer_hex);
    if (peer_hex_len != 20) {
        fprintf(stderr, "Peer UID must be 20 hex chars (10 bytes), got %d: %s\n",
            peer_hex_len, peer_hex);
        return 1;
    }
    int peer_uid_len = peer_hex_len / 2;
    char *peer_uid = (char *)malloc(peer_uid_len + 1);
    if (peer_uid_len != 10) {
        fprintf(stderr, "Peer UID must be 10 bytes\n");
        free(peer_uid);
        return 1;
    }
    for (int i = 0; i < peer_uid_len; i++) {
        unsigned int b;
        if (sscanf(peer_hex + i * 2, "%2x", &b) != 1) {
            fprintf(stderr, "Invalid hex peer UID: %s\n", peer_hex);
            free(peer_uid);
            return 1;
        }
        peer_uid[i] = (char)b;
    }
    peer_uid[peer_uid_len] = 0;

    init(user_dir, "dwytest-send");

    // Get my UID
    const char *my_uid;
    int my_uid_len;
    dwyco_get_my_uid(&my_uid, &my_uid_len);
    if (my_uid_len != 10) {
        fprintf(stderr, "My UID is not 10 bytes (got %d)\n", my_uid_len);
        free(peer_uid);
        shutdown();
        return 1;
    }
    printf("  My UID (hex): ");
    for (int i = 0; i < my_uid_len; i++)
        printf("%02x", (unsigned char)my_uid[i]);
    printf("\n");

    // Sanity-check the attachment file is readable before sending.
    if (att_path) {
        unsigned long long h = 0;
        if (file_hash(att_path, &h) < 0) {
            fprintf(stderr, "Cannot read attachment '%s'\n", att_path);
            free(peer_uid);
            shutdown();
            return 1;
        }
        printf("  Attachment: '%s'\n", att_path);
    }

    // Wait until the receiver is online before sending, so the message is
    // delivered directly rather than parked on the server.
    if (!g_server_mode) {
        printf("  Waiting for peer to come online...\n");
        if (!wait_peer_online(peer_uid, peer_uid_len, 90000)) {
            fprintf(stderr, "Peer never came online\n");
            free(peer_uid);
            shutdown();
            return 1;
        }
        printf("  Peer online\n");
    } else {
        printf("  Server mode: skipping peer online check\n");
    }

    // Send
    int cid;
    if (att_path)
        cid = dwyco_make_file_zap_composition(att_path, strlen(att_path));
    else
        cid = dwyco_make_zap_composition(0);
    ASSERT(cid > 0);

    const char *pers_id;
    int pers_len;
    int res = dwyco_zap_send6(cid, peer_uid, peer_uid_len,
        msg_text, strlen(msg_text), no_forward, 0, 0,
        &pers_id, &pers_len);
    ASSERT(res != 0);
    printf("  Send initiated, pers_id=%.*s\n", pers_len, pers_id);

    // Poll until send completes or fails
    printf("  Waiting for send result...\n");
    clear_events();
    int got_result = 0;
    int elapsed = 0;
    while (elapsed < 90000) {
        int spin, next;
        next = dwyco_service_channels(&spin);
        if (next <= 0 || next > 50) next = 50;
        usleep(next * 1000);
        elapsed += next;
        for (auto &e : g_events) {
            if (e.cmd == DWYCO_SE_MSG_SEND_SUCCESS) {
                printf("  Send success\n");
                got_result = 1;
                break;
            }
            if (e.cmd == DWYCO_SE_MSG_SEND_FAIL) {
                printf("  Send fail\n");
                got_result = 1;
                break;
            }
        }
        if (got_result) break;
    }
    if (!got_result)
        printf("  Send timed out\n");

    dwyco_delete_zap_composition(cid);
    free(peer_uid);
    shutdown();
    return got_result ? 0 : 1;
}

// ===== RECEIVE MODE =====
// Usage: dwytest_peer recv <user_dir> <peer_uid_hex>
static int
mode_recv(int argc, char **argv)
{
    if (argc < 4) {
        fprintf(stderr, "Usage: dwytest_peer recv <user_dir> <peer_uid_hex>\n");
        return 1;
    }
    const char *user_dir = argv[2];
    const char *peer_hex = argv[3];

    int peer_hex_len = strlen(peer_hex);
    if (peer_hex_len != 20) {
        fprintf(stderr, "Peer UID must be 20 hex chars (10 bytes), got %d: %s\n",
            peer_hex_len, peer_hex);
        return 1;
    }
    int peer_uid_len = peer_hex_len / 2;
    char *peer_uid = (char *)malloc(peer_uid_len + 1);
    if (peer_uid_len != 10) {
        fprintf(stderr, "Peer UID must be 10 bytes\n");
        free(peer_uid);
        return 1;
    }
    for (int i = 0; i < peer_uid_len; i++) {
        unsigned int b;
        if (sscanf(peer_hex + i * 2, "%2x", &b) != 1) {
            fprintf(stderr, "Invalid hex peer UID: %s\n", peer_hex);
            free(peer_uid);
            return 1;
        }
        peer_uid[i] = (char)b;
    }
    peer_uid[peer_uid_len] = 0;

    init(user_dir, "dwytest-recv");

    // Stay online so the sender can see us and deliver the message
    // directly. Wait for the sender to come online before polling for
    // the incoming message.
    if (!g_server_mode) {
        printf("  Waiting for sender to come online...\n");
        if (!wait_peer_online(peer_uid, peer_uid_len, 90000)) {
            fprintf(stderr, "Sender never came online\n");
            free(peer_uid);
            shutdown();
            return 1;
        }
        printf("  Sender online\n");
    } else {
        printf("  Server mode: skipping sender online check\n");
    }

    // Wait for a message. New server messages are handled via the
    // canonical dwyco_new_msg flow (rescan -> process remote msgs ->
    // hand out the _inbox-tagged message).
    printf("  Waiting for message...\n");
    int elapsed = 0;
    while (elapsed < 120000) {
        int spin, next;
        next = dwyco_service_channels(&spin);
        if (next <= 0 || next > 50) next = 50;
        usleep(next * 1000);
        elapsed += next;

        for (size_t gi = 0; gi < g_events.size(); ++gi) {
            static size_t last_printed = 0;
            if (gi < last_printed) continue;
            int c = g_events[gi].cmd;
            printf("  DBG ev[%d]=%d (%s) val=%.24s\n", (int)gi, c, dwyco_se_name_lookup(c), g_events[gi].value.c_str());
            last_printed = gi + 1;
        }

        if (dwyco_get_rescan_messages())
            printf("  DBG rescan flag SET\n");

        if (dwyco_get_rescan_messages()) {
            dwyco_set_rescan_messages(0);
            DWYCO_UNFETCHED_MSG_LIST ufml;
            if (dwyco_get_unfetched_messages(&ufml, 0, 0)) {
                dwyco_list dl(ufml);
                printf("  DBG process_remote_msgs nrows=%d\n", dl.rows());
            } else {
                printf("  DBG process_remote_msgs: no unfetched\n");
            }
            process_remote_msgs();
        }

        int zviewer, has_att, is_file;
        DwString ruid, txt, mid, creator_uid;
        if (dwyco_new_msg2(ruid, txt, zviewer, mid, has_att, is_file,
            creator_uid)) {
            printf("  New msg from=%s text='%.*s'\n",
                DwString::to_hex(ruid).c_str(), txt.length(), txt.c_str());

            DwString expected_bin(peer_uid, peer_uid_len);
            if (ruid != expected_bin) {
                printf("  Skipped msg (expected from this peer only)\n");
                processed_msg(mid);
                dwyco_delete_saved_message(ruid.c_str(), ruid.length(), mid.c_str());
                dwyco_delete_unfetched_message(mid.c_str());
                continue;
            }

            // Mark processed and clean up, as the bots do
            processed_msg(mid);
            dwyco_delete_saved_message(ruid.c_str(), ruid.length(), mid.c_str());
            dwyco_delete_unfetched_message(mid.c_str());

            free(peer_uid);
            shutdown();
            printf("  Receive OK\n");
            return 0;
        }
    }
    fprintf(stderr, "Timed out waiting for message\n");
    free(peer_uid);
    shutdown();
    return 1;
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s [--server] send <user_dir> <peer_uid_hex> <text> [no_forward] [attachment_path]\n", argv[0]);
        fprintf(stderr, "  %s [--server] recv <user_dir> <peer_uid_hex>\n", argv[0]);
        return 1;
    }

    int arg_start = 1;
    if (argc > 1 && strcmp(argv[1], "--server") == 0) {
        g_server_mode = 1;
        arg_start = 2;
    }

    if (arg_start >= argc) {
        fprintf(stderr, "Missing mode after --server\n");
        return 1;
    }

    if (strcmp(argv[arg_start], "send") == 0)
        return mode_send(argc - arg_start + 1, argv + arg_start - 1);
    else if (strcmp(argv[arg_start], "recv") == 0)
        return mode_recv(argc - arg_start + 1, argv + arg_start - 1);
    else {
        fprintf(stderr, "Unknown mode: %s\n", argv[arg_start]);
        return 1;
    }
}
