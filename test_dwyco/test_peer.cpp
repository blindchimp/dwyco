#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>

#include "dlli.h"

#define ASSERT(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL at %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        exit(1); \
    } \
} while(0)

// Event recording
struct Event {
    int cmd;
    std::string uid;
    std::string value;
};
static std::vector<Event> g_events;
static int g_login_done;
static int g_fetch_id;
static int g_fetch_what;
static char g_fetch_msgid[256];

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
    if (what == 1) g_login_done = 1;
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
    g_fetch_id = id;
    g_fetch_what = what;
    if (msgid) {
        strncpy(g_fetch_msgid, msgid, sizeof(g_fetch_msgid) - 1);
        g_fetch_msgid[sizeof(g_fetch_msgid) - 1] = 0;
    }
}

static void
service_once(void)
{
    int spin;
    dwyco_service_channels(&spin);
}

static int
wait_for(int target_cmd, int timeout_ms)
{
    int elapsed = 0;
    while (elapsed < timeout_ms) {
        int spin, next;
        next = dwyco_service_channels(&spin);
        if (next <= 0 || next > 50) next = 50;
        usleep(next * 1000);
        elapsed += next;
        for (auto &e : g_events)
            if (e.cmd == target_cmd) return 1;
    }
    return 0;
}

static int
wait_for_fetch(int timeout_ms)
{
    int elapsed = 0;
    while (elapsed < timeout_ms) {
        int spin, next;
        next = dwyco_service_channels(&spin);
        if (next <= 0 || next > 50) next = 50;
        usleep(next * 1000);
        elapsed += next;
        if (g_fetch_id) return 1;
    }
    return 0;
}

static void
clear_events(void)
{
    g_events.clear();
}

static void
init(const char *user_dir)
{
    char sys_dir[512], tmp_dir[512];
    snprintf(sys_dir, sizeof(sys_dir), "%s/sys", user_dir);
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", user_dir);
    mkdir(sys_dir, 0755);
    mkdir(user_dir, 0755);
    mkdir(tmp_dir, 0755);

    dwyco_set_fn_prefixes(sys_dir, user_dir, tmp_dir);
    dwyco_set_system_event_callback(sys_event_cb);
    dwyco_set_login_result_callback(login_cb);
    dwyco_set_emergency_callback(emergency_cb);
    dwyco_set_client_version("dwytest", 7);

    ASSERT(dwyco_init() != 0);
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);

    printf("  Waiting for login...\n");
    if (!wait_for(DWYCO_SE_SERVER_LOGIN, 30000)) {
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

static void
write_coord(const char *fn, const char *pers_id, int pers_len,
    const char *text, int text_len, int no_forward, const char *msg_id)
{
    FILE *f = fopen(fn, "w");
    ASSERT(f);
    if (pers_id && pers_len > 0)
        fprintf(f, "pers_id=%.*s\n", pers_len, pers_id);
    if (text && text_len > 0)
        fprintf(f, "text=%.*s\n", text_len, text);
    fprintf(f, "no_forward=%d\n", no_forward);
    if (msg_id)
        fprintf(f, "msg_id=%s\n", msg_id);
    fclose(f);
}

static int
read_coord(const char *fn, std::string &pers_id, std::string &text,
    int &no_forward, std::string &msg_id)
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
    }
    fclose(f);
    return 1;
}

// ===== SEND MODE =====
// Usage: dwytest_peer send <user_dir> <coord_file> <peer_uid> <text> [no_forward]
static int
mode_send(int argc, char **argv)
{
    if (argc < 6) {
        fprintf(stderr, "Usage: dwytest_peer send <user_dir> <coord_file> <peer_uid_hex> <text> [no_forward]\n");
        return 1;
    }
    const char *user_dir = argv[2];
    const char *coord_fn = argv[3];
    const char *peer_hex = argv[4];
    const char *msg_text = argv[5];
    int no_forward = (argc > 6) ? atoi(argv[6]) : 0;

    // Convert hex peer UID to binary
    int peer_hex_len = strlen(peer_hex);
    int peer_uid_len = peer_hex_len / 2;
    char *peer_uid = (char *)malloc(peer_uid_len + 1);
    for (int i = 0; i < peer_uid_len; i++) {
        unsigned int b;
        sscanf(peer_hex + i * 2, "%2x", &b);
        peer_uid[i] = (char)b;
    }
    peer_uid[peer_uid_len] = 0;

    init(user_dir);

    // Get my UID
    const char *my_uid;
    int my_uid_len;
    dwyco_get_my_uid(&my_uid, &my_uid_len);
    printf("  My UID (hex): ");
    for (int i = 0; i < my_uid_len; i++)
        printf("%02x", (unsigned char)my_uid[i]);
    printf("\n");

    // Send
    int cid = dwyco_make_zap_composition(0);
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
    while (elapsed < 30000) {
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
                write_coord(coord_fn, pers_id, pers_len,
                    msg_text, strlen(msg_text), no_forward,
                    e.value.c_str());
                break;
            }
            if (e.cmd == DWYCO_SE_MSG_SEND_FAIL) {
                printf("  Send fail\n");
                got_result = 1;
                write_coord(coord_fn, pers_id, pers_len,
                    msg_text, strlen(msg_text), no_forward, 0);
                break;
            }
        }
        if (got_result) break;
    }
    if (!got_result) {
        printf("  Send timed out\n");
        write_coord(coord_fn, pers_id, pers_len,
            msg_text, strlen(msg_text), no_forward, 0);
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
    int peer_uid_len = peer_hex_len / 2;
    char *peer_uid = (char *)malloc(peer_uid_len + 1);
    for (int i = 0; i < peer_uid_len; i++) {
        unsigned int b;
        sscanf(peer_hex + i * 2, "%2x", &b);
        peer_uid[i] = (char)b;
    }
    peer_uid[peer_uid_len] = 0;

    init(user_dir);

    // Read expected text from coord file
    std::string expected_pers_id, expected_text;
    int expected_no_forward = 0;
    std::string sent_msg_id;
    read_coord(coord_fn, expected_pers_id, expected_text,
        expected_no_forward, sent_msg_id);

    printf("  Expected text: '%s'\n", expected_text.c_str());
    printf("  Expected no_forward: %d\n", expected_no_forward);

    // Wait for SE_USER_MSG_RECEIVED
    printf("  Waiting for message...\n");
    clear_events();
    if (!wait_for(DWYCO_SE_USER_MSG_RECEIVED, 60000)) {
        fprintf(stderr, "Timed out waiting for message\n");
        free(peer_uid);
        shutdown();
        return 1;
    }
    printf("  Message received event\n");

    // Get unfetched messages from peer
    DWYCO_UNFETCHED_MSG_LIST unfetched = 0;
    int res = dwyco_get_unfetched_messages(&unfetched, peer_uid, peer_uid_len);
    ASSERT(res != 0);
    ASSERT(unfetched != 0);

    int rows, cols;
    dwyco_list_numelems(unfetched, &rows, &cols);
    printf("  Unfetched messages: %d\n", rows);
    ASSERT(rows >= 1);

    // Fetch each message
    for (int i = 0; i < rows; i++) {
        const char *msg_id;
        int msg_id_len, type;
        res = dwyco_list_get(unfetched, i, DWYCO_QMS_ID, &msg_id, &msg_id_len, &type);
        if (!res || !msg_id) continue;

        printf("  Fetching message %d: %.*s\n", i, msg_id_len, msg_id);

        g_fetch_id = 0;
        g_fetch_what = 0;
        memset(g_fetch_msgid, 0, sizeof(g_fetch_msgid));

        int fid = dwyco_fetch_server_message(std::string(msg_id, msg_id_len).c_str(),
            fetch_cb, 0, 0, 0);
        ASSERT(fid != 0);

        if (!wait_for_fetch(30000)) {
            fprintf(stderr, "Fetch timed out\n");
            continue;
        }

        printf("  Fetch result: what=%d msgid=%s\n", g_fetch_what, g_fetch_msgid);
        ASSERT(g_fetch_what == DWYCO_MSG_DOWNLOAD_OK ||
               g_fetch_what == DWYCO_MSG_DOWNLOAD_SAVE_FAILED);

        // Save the message
        res = dwyco_save_message(std::string(msg_id, msg_id_len).c_str());
        ASSERT(res != 0);
        printf("  Message saved\n");

        // Now retrieve the saved message body
        DWYCO_SAVED_MSG_LIST body = 0;
        res = dwyco_get_saved_message(&body, peer_uid, peer_uid_len,
            std::string(msg_id, msg_id_len).c_str());
        if (res && body) {
            // Get body text
            DWYCO_LIST text_list = dwyco_get_body_text(body);
            if (text_list) {
                int text_rows, text_cols;
                dwyco_list_numelems(text_list, &text_rows, &text_cols);
                printf("  Body text rows: %d\n", text_rows);

                // Get the text string
                const char *body_text;
                int body_text_len, body_type;
                res = dwyco_list_get(text_list, 0, DWYCO_NO_COLUMN,
                    &body_text, &body_text_len, &body_type);
                if (res && body_text) {
                    printf("  Body text: '%.*s'\n", body_text_len, body_text);
                    // Verify against expected
                    if (!expected_text.empty()) {
                        ASSERT(body_text_len == (int)expected_text.length());
                        ASSERT(memcmp(body_text, expected_text.c_str(),
                               body_text_len) == 0);
                        printf("  Text MATCH\n");
                    }
                }
                dwyco_list_release(text_list);
            }
            dwyco_list_release(body);
        }

        // Delete saved message
        dwyco_delete_saved_message(peer_uid, peer_uid_len,
            std::string(msg_id, msg_id_len).c_str());

        // Delete unfetched
        dwyco_delete_unfetched_message(std::string(msg_id, msg_id_len).c_str());
    }

    dwyco_list_release(unfetched);
    free(peer_uid);
    shutdown();
    printf("  Receive OK\n");
    return 0;
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s send <user_dir> <coord_file> <peer_uid_hex> <text> [no_forward]\n", argv[0]);
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
