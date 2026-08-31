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

static void
shutdown(void)
{
    dwyco_exit();
}

// Coordination file format:
//   pers_id=<hex>\n
//   text=<message text>\n
//   no_forward=<0|1>\n
//   msg_id=<server msg id>\n
//   att_size=<bytes>\n           (only for attachment sends)
//   att_hash=<hex fnv1a>\n       (only for attachment sends)
//   att_name=<user filename>\n   (only for attachment sends)

static void
write_coord2(const char *fn, const char *pers_id, int pers_len,
    const char *text, int text_len, int no_forward, const char *msg_id,
    long att_size, unsigned long long att_hash, const char *att_name)
{
    // write to a temp file and rename so the receiver never
    // observes a partially-written coord file.
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s.tmp", fn);
    FILE *f = fopen(tmp, "w");
    ASSERT(f);
    if (pers_id && pers_len > 0)
        fprintf(f, "pers_id=%.*s\n", pers_len, pers_id);
    if (text && text_len > 0)
        fprintf(f, "text=%.*s\n", text_len, text);
    fprintf(f, "no_forward=%d\n", no_forward);
    if (msg_id)
        fprintf(f, "msg_id=%s\n", msg_id);
    if (att_size >= 0) {
        fprintf(f, "att_size=%ld\n", att_size);
        fprintf(f, "att_hash=%llx\n", att_hash);
        if (att_name)
            fprintf(f, "att_name=%s\n", att_name);
    }
    fclose(f);
    rename(tmp, fn);
}

static void
write_coord(const char *fn, const char *pers_id, int pers_len,
    const char *text, int text_len, int no_forward, const char *msg_id)
{
    write_coord2(fn, pers_id, pers_len, text, text_len, no_forward, msg_id,
        -1, 0, 0);
}

static int
read_coord(const char *fn, std::string &pers_id, std::string &text,
    int &no_forward, std::string &msg_id, long &att_size,
    unsigned long long &att_hash, std::string &att_name)
{
    FILE *f = fopen(fn, "r");
    if (!f) return 0;
    char buf[1024];
    while (fgets(buf, sizeof(buf), f)) {
        char *eq = strchr(buf, '=');
        if (!eq) continue;
        *eq = 0;
        char *val = eq + 1;
        // strip newline
        size_t len = strlen(val);
        while (len > 0 && (val[len-1] == '\n' || val[len-1] == '\r'))
            val[--len] = 0;
        if (strcmp(buf, "pers_id") == 0) pers_id = val;
        else if (strcmp(buf, "text") == 0) text = val;
        else if (strcmp(buf, "no_forward") == 0) no_forward = atoi(val);
        else if (strcmp(buf, "msg_id") == 0) msg_id = val;
        else if (strcmp(buf, "att_size") == 0) att_size = atol(val);
        else if (strcmp(buf, "att_hash") == 0) sscanf(val, "%llx", &att_hash);
        else if (strcmp(buf, "att_name") == 0) att_name = val;
    }
    fclose(f);
    return 1;
}

// ===== SEND MODE =====
// Usage: dwytest_peer send <user_dir> <coord_file> <peer_uid> <text> [no_forward] [attachment_path]
static int
mode_send(int argc, char **argv)
{
    if (argc < 6) {
        fprintf(stderr, "Usage: dwytest_peer send <user_dir> <coord_file> <peer_uid_hex> <text> [no_forward] [attachment_path]\n");
        return 1;
    }
    const char *user_dir = argv[2];
    const char *coord_fn = argv[3];
    const char *peer_hex = argv[4];
    const char *msg_text = argv[5];
    int no_forward = (argc > 6) ? atoi(argv[6]) : 0;
    const char *att_path = (argc > 7 && strlen(argv[7]) > 0) ? argv[7] : 0;

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

    // If sending an attachment, compute its size/hash so the receiver
    // can verify the copied-out file matches byte-for-byte.
    long att_size = -1;
    unsigned long long att_hash = 0;
    std::string att_name;
    if (att_path) {
        att_size = file_hash(att_path, &att_hash);
        if (att_size < 0) {
            fprintf(stderr, "Cannot read attachment '%s'\n", att_path);
            free(peer_uid);
            shutdown();
            return 1;
        }
        const char *base = strrchr(att_path, '/');
        att_name = base ? base + 1 : att_path;
        printf("  Attachment: '%s' size=%ld hash=%llx\n",
            att_name.c_str(), att_size, att_hash);
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
        if (next <= 0 || next > 100) next = 100;
        usleep(next * 1000);
        elapsed += next;
        for (auto &e : g_events) {
            if (e.cmd == DWYCO_SE_MSG_SEND_SUCCESS) {
                printf("  Send success\n");
                got_result = 1;
                // e.value may contain the server msg_id
                write_coord2(coord_fn, pers_id, pers_len,
                    msg_text, strlen(msg_text), no_forward,
                    e.value.c_str(), att_size, att_hash, att_name.c_str());
                break;
            }
            if (e.cmd == DWYCO_SE_MSG_SEND_FAIL) {
                printf("  Send fail\n");
                got_result = 1;
                write_coord2(coord_fn, pers_id, pers_len,
                    msg_text, strlen(msg_text), no_forward, 0,
                    att_size, att_hash, att_name.c_str());
                break;
            }
        }
        if (got_result) break;
    }
    if (!got_result) {
        printf("  Send timed out\n");
        write_coord2(coord_fn, pers_id, pers_len,
            msg_text, strlen(msg_text), no_forward, 0,
            att_size, att_hash, att_name.c_str());
    }

    dwyco_delete_zap_composition(cid);
    free(peer_uid);
    shutdown();
    return got_result ? 0 : 1;
}

// ===== RECEIVE MODE =====
// Usage: dwytest_peer recv <user_dir> <coord_file> <peer_uid_hex>
static int
mode_recv(int argc, char **argv)
{
    if (argc < 5) {
        fprintf(stderr, "Usage: dwytest_peer recv <user_dir> <coord_file> <peer_uid_hex>\n");
        return 1;
    }
    const char *user_dir = argv[2];
    const char *coord_fn = argv[3];
    const char *peer_hex = argv[4];

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

    // The sender writes the coord file (atomically) once the message
    // has been sent. Wait for it to appear before reading it.
    printf("  Waiting for coord file...\n");
    int elapsed = 0;
    while (elapsed < 90000) {
        FILE *cf = fopen(coord_fn, "r");
        if (cf) { fclose(cf); break; }
        usleep(250 * 1000);
        elapsed += 250;
    }
    if (elapsed >= 90000) {
        fprintf(stderr, "Timed out waiting for coord file\n");
        free(peer_uid);
        shutdown();
        return 1;
    }

    // Read expected text from coord file
    std::string expected_pers_id, expected_text;
    int expected_no_forward = 0;
    std::string sent_msg_id;
    long expected_att_size = -1;
    unsigned long long expected_att_hash = 0;
    std::string expected_att_name;
    read_coord(coord_fn, expected_pers_id, expected_text,
        expected_no_forward, sent_msg_id, expected_att_size,
        expected_att_hash, expected_att_name);

    printf("  Expected text: '%s'\n", expected_text.c_str());
    printf("  Expected no_forward: %d\n", expected_no_forward);
    if (expected_att_size >= 0)
        printf("  Expected attachment: '%s' size=%ld hash=%llx\n",
            expected_att_name.c_str(), expected_att_size, expected_att_hash);

    // Wait for a message. Messages are only delivered while both peers
    // are online, so the receiver must stay connected and service
    // channels until the send happens. New server messages are handled
    // via the canonical dwyco_new_msg flow (rescan -> process remote
    // msgs -> hand out the _inbox-tagged message).
    printf("  Waiting for message...\n");
    elapsed = 0;
    while (elapsed < 120000) {
        int spin, next;
        next = dwyco_service_channels(&spin);
        if (next <= 0 || next > 100) next = 100;
        usleep(next * 1000);
        elapsed += next;

        for (size_t gi = 0; gi < g_events.size(); ++gi) {
            static size_t last_printed = 0;
            if (gi < last_printed) continue;
            int c = g_events[gi].cmd;
            printf("  DBG ev[%d]=%d val=%.24s\n", (int)gi, c, g_events[gi].value.c_str());
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
            int from_expected = (ruid == expected_bin);

            int ok = from_expected;
            if (from_expected && !expected_text.empty()) {
                if (txt.length() != (int)expected_text.length() ||
                        memcmp(txt.c_str(), expected_text.c_str(),
                            txt.length()) != 0) {
                    // Not what we sent this round: likely a leftover from a
                    // previous (failed) run. Note it and move on.
                    printf("  Text MISMATCH (stale msg?), skipping\n");
                    ok = 0;
                } else {
                    printf("  Text MATCH\n");
                }
            }

            if (ok && expected_att_size >= 0) {
                // The attachment must have been declared and be a file zap.
                if (!has_att || !is_file) {
                    printf("  Attachment MISMATCH (stale msg?), skipping\n");
                    ok = 0;
                } else {
                    if (!expected_att_name.empty())
                        printf("  Attachment name: '%s'\n", expected_att_name.c_str());
                    // Copy out the full file (unlike the _buf2 variant, this
                    // has no size cap) and verify size + content hash.
                    char dst[1024];
                    snprintf(dst, sizeof(dst), "%s/tmp/att_recv_%d.bin",
                        user_dir, getpid());
                    unsigned long long got_hash = 0;
                    long got_size =
                        dwyco_copy_out_file_zap2(mid.c_str(), dst) ? file_hash(dst, &got_hash) : -1;
                    unlink(dst);
                    if (got_size < 0 || got_size != expected_att_size ||
                            got_hash != expected_att_hash) {
                        printf("  Attachment MISMATCH (stale msg?), skipping\n");
                        ok = 0;
                    } else {
                        printf("  Attachment MATCH (size=%ld)\n", got_size);
                    }
                }
            }

            // Mark processed and clean up, as the bots do
            processed_msg(mid);
            dwyco_delete_saved_message(ruid.c_str(), ruid.length(), mid.c_str());
            dwyco_delete_unfetched_message(mid.c_str());

            if (!ok) {
                printf("  Skipped msg (expected from this peer only)\n");
                continue;
            }

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
        fprintf(stderr, "  %s send <user_dir> <coord_file> <peer_uid_hex> <text> [no_forward] [attachment_path]\n", argv[0]);
        fprintf(stderr, "  %s recv <user_dir> <coord_file> <peer_uid_hex>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "send") == 0)
        return mode_send(argc, argv);
    else if (strcmp(argv[1], "recv") == 0)
        return mode_recv(argc, argv);
    else {
        fprintf(stderr, "Unknown mode: %s\n", argv[1]);
        return 1;
    }
}
