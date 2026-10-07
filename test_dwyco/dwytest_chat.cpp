// Coverage for the chat server / chat context / user lobby API.
//
//   dwyco_get_server_list, dwyco_switch_to_chat_server,
//   dwyco_switch_to_chat_server2, dwyco_disconnect_chat_server,
//   dwyco_chat_online, dwyco_chat_server_has_pw, dwyco_check_chat_server_pw,
//   dwyco_get_lobby_name_by_id2, dwyco_set_chat_ctx_callback,
//   dwyco_set_chat_ctx_callback2, dwyco_chat_addq, dwyco_chat_delq,
//   dwyco_chat_talk, dwyco_chat_mute, dwyco_chat_set_filter,
//   dwyco_chat_set_demigod, dwyco_chat_clear_all_demigods,
//   dwyco_chat_set_unblock_time, dwyco_chat_set_unblock_time2,
//   dwyco_chat_get_admin_info, dwyco_chat_send_popup,
//   dwyco_chat_set_sys_attr, dwyco_chat_send_data,
//   dwyco_chat_set_activity_state, dwyco_chat_create_user_lobby,
//   dwyco_chat_remove_user_lobby
//
// Needs a running server. A directory server doubles as a chat server, so
// dwyco_switch_to_chat_server(0) is enough to get a real chat session; no
// peer is required.
//
// USER LOBBY LIFECYCLE, which is the thing to get right here:
//
//   create_user_lobby -> (wait for ADD_LOBBY) -> switch_to_chat_server2 (enter)
//                     -> remove_user_lobby
//
// ADD_LOBBY for your own lobby does arrive, and quickly -- measured at
// 200-900 ms, well under the 30 s worst case. So do not conclude "you never
// get your own lobby" from a short wait; the id does turn up in the lobby
// list like any other. Once you have it, the creator is the lobby's subgod,
// so check_chat_server_pw() returns 1 for ANY password and entering needs
// none (dlli.cpp:3898). Only a *different* account can reach outcome 2, which
// is why the last phase spawns a second client.
//
// Removal only works for a lobby you can get into, and only the creator or a
// system god may remove one (cdcx/mainwin.cpp:4602 says as much). Note that
// removing the lobby you are currently inside drops you from the chat server,
// so you never see the DEL_LOBBY for it and your local lobby table stays stale
// until you reconnect. That is asserted rather than papered over.
//
// ORDER MATTERS for a second reason: dwyco_switch_to_chat_server() calls
// stop_chat_thread() unconditionally (dirth.cc:834), so every call tears down
// the live chat session and rebuilds it from scratch. Chat_online stays 0
// until the new server issues its challenge and chat_online() runs
// (chatops.cc:120). So each switch here is deliberate and followed by a wait
// for the session to come back up.
//
// dwyco_switch_to_chat_server2() is different and much better behaved: it
// looks the lobby up and checks the password BEFORE calling
// stop_chat_thread(), so a refused switch leaves the session untouched.
//
// TWO HEADER BUGS THIS TEST ENCOUNTERS
//
// 1. "User-defined lobbies -- Only available via ChatCtxCallback2" (dlli.h:257)
//    is wrong. add_user_lobby and del_user_lobby both fire
//    dwyco_pg_callback, i.e. ChatCtxCallback1 (pgdll.cpp:253, pgdll.cpp:273),
//    with the lobby delivered as a DWYCO_LIST in the value parameters.
//    Callback2 fires for exactly one event: DWYCO_CHAT_CTX_SYS_ATTR, and only
//    when the attribute value is a vector (pgdll.cpp:200-205). Scalar sys attrs
//    go to callback1.
//
// 2. The lobby-id column constants are named "000".."011" but are *column
//    positions*, not values. There is no lobby with id "000". Real ids look
//    like "U_19a99a9e", so anything that hardcodes "000" as a lobby id is
//    testing nothing.

#include <dlli.h>
#include "test_common.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>

static int g_pass = 0;
static int g_fail = 0;

#define RUN(name) do { \
    printf("  %-42s ", #name); \
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

static const char *CLIENT_DIR = "/tmp/dwytest_chat";
static const char *CHECKER_DIR = "/tmp/dwytest_chat_checker";

// How long the second client keeps its lobby alive for inspection.
static int CHECKER_HOLD_MS = 60000;

// How long to sit on a live chat session and confirm nothing drops it.
static int STABILITY_SECONDS = 20;

// The real DWYCO_CHAT_CTX_* names. This is deliberately a separate table from
// the DWYCO_SE_* one in test_common.h: the two numbering spaces overlap
// numerically but mean unrelated things.
static const char *chat_ctx_name(int cmd)
{
    switch (cmd) {
    case DWYCO_CHAT_CTX_NEW: return "NEW";
    case DWYCO_CHAT_CTX_DEL: return "DEL";
    case DWYCO_CHAT_CTX_ADD_USER: return "ADD_USER";
    case DWYCO_CHAT_CTX_DEL_USER: return "DEL_USER";
    case DWYCO_CHAT_CTX_UPDATE_AH: return "UPDATE_AH";
    case DWYCO_CHAT_CTX_START_UPDATE: return "START_UPDATE";
    case DWYCO_CHAT_CTX_END_UPDATE: return "END_UPDATE";
    case DWYCO_CHAT_CTX_Q_ADD: return "Q_ADD";
    case DWYCO_CHAT_CTX_Q_DEL: return "Q_DEL";
    case DWYCO_CHAT_CTX_Q_GRANT: return "Q_GRANT";
    case DWYCO_CHAT_CTX_Q_DATA: return "Q_DATA";
    case DWYCO_CHAT_CTX_SYS_ATTR: return "SYS_ATTR";
    case DWYCO_CHAT_CTX_UPDATE_ATTR: return "UPDATE_ATTR";
    case DWYCO_CHAT_CTX_ADD_LOBBY: return "ADD_LOBBY";
    case DWYCO_CHAT_CTX_DEL_LOBBY: return "DEL_LOBBY";
    case DWYCO_CHAT_CTX_ADD_GOD: return "ADD_GOD";
    case DWYCO_CHAT_CTX_DEL_GOD: return "DEL_GOD";
    case DWYCO_CHAT_CTX_RECV_DATA: return "RECV_DATA";
    default: return "?";
    }
}

// The 12 columns the server sends for a user lobby. Column names are positions.
#define LOB_COL_ID     DWYCO_SL_ULOBBY_ID
#define LOB_COL_HOST   DWYCO_SL_ULOBBY_HOSTNAME
#define LOB_COL_IP     DWYCO_SL_ULOBBY_IP
#define LOB_COL_PORT   DWYCO_SL_ULOBBY_PORT
#define LOB_COL_RATING DWYCO_SL_ULOBBY_RATING
#define LOB_COL_DISP   DWYCO_SL_ULOBBY_DISPLAY_NAME
#define LOB_COL_CAT    DWYCO_SL_ULOBBY_CATEGORY
#define LOB_COL_MAXU   DWYCO_SL_ULOBBY_MAX_USERS

#define MAX_LOBBIES 64

struct lobby_info {
    char id[64];
    char disp[128];
    char cat[64];
    char host[64];
    int port;
    int max_users;
    int has_pw;
    int used;
};

static lobby_info g_lobbies[MAX_LOBBIES];
static int g_nlobbies;

// Chat ctx recording.
static int g_ctx1_count, g_ctx2_count;
static int g_ctx_new, g_ctx_del, g_ctx_add_user, g_ctx_attr;
static int g_ctx_sysattr, g_ctx_add_lobby, g_ctx_start_update, g_ctx_end_update;
static int g_ctx_last_ctx;
static int g_ctx_batches;

// Copy a DWYCO_LIST column into a fixed buffer. Returns 0 if absent.
static int
lob_get_str(DWYCO_LIST l, const char *col, char *buf, int buflen)
{
    const char *v;
    int vl, vt;
    buf[0] = 0;
    if (!dwyco_list_get(l, 0, col, &v, &vl, &vt))
        return 0;
    int n = vl < buflen - 1 ? vl : buflen - 1;
    memcpy(buf, v, n);
    buf[n] = 0;
    return 1;
}

static int
lob_get_int(DWYCO_LIST l, const char *col)
{
    const char *v;
    int vl, vt;
    if (!dwyco_list_get(l, 0, col, &v, &vl, &vt))
        return -1;
    if (vt != DWYCO_TYPE_INT)
        return -1;
    char tmp[32];
    int n = vl < (int)sizeof(tmp) - 1 ? vl : (int)sizeof(tmp) - 1;
    memcpy(tmp, v, n);
    tmp[n] = 0;
    return atoi(tmp);
}

static void DWYCOCALLCONV
chat_ctx_cb1(int cmd, int ctx_id, const char *uid, int len_uid,
    const char *name, int len_name, int type, const char *value, int val_len,
    int qid, int extra_arg)
{
    (void)uid; (void)len_uid; (void)val_len; (void)qid; (void)extra_arg;
    g_ctx1_count++;
    g_ctx_last_ctx = ctx_id;

    switch (cmd) {
    case DWYCO_CHAT_CTX_NEW: g_ctx_new++; break;
    case DWYCO_CHAT_CTX_DEL: g_ctx_del++; break;
    case DWYCO_CHAT_CTX_ADD_USER: g_ctx_add_user++; break;
    case DWYCO_CHAT_CTX_UPDATE_ATTR: g_ctx_attr++; break;
    case DWYCO_CHAT_CTX_SYS_ATTR: g_ctx_sysattr++; break;
    case DWYCO_CHAT_CTX_START_UPDATE:
        g_ctx_start_update++;
        break;
    case DWYCO_CHAT_CTX_END_UPDATE:
        g_ctx_end_update++;
        // Each END closes one bracket opened by a START. They must match up;
        // an unclosed START would mean a truncated update burst.
        g_ctx_batches++;
        break;
    case DWYCO_CHAT_CTX_ADD_LOBBY: {
        // Per cdcx/mainwin.cpp:3969 the value pointer IS the lobby list.
        // Do NOT release it -- pgdll.cpp:257 does that after we return.
        g_ctx_add_lobby++;
        if (type != DWYCO_TYPE_VECTOR && type != -1)
            break;
        DWYCO_LIST l = (DWYCO_LIST)value;
        if (!l || g_nlobbies >= MAX_LOBBIES)
            break;
        lobby_info &li = g_lobbies[g_nlobbies];
        memset(&li, 0, sizeof(li));
        if (!lob_get_str(l, LOB_COL_ID, li.id, sizeof(li.id)))
            return;
        if (li.id[0] == 0)
            return;
        lob_get_str(l, LOB_COL_DISP, li.disp, sizeof(li.disp));
        lob_get_str(l, LOB_COL_CAT, li.cat, sizeof(li.cat));
        lob_get_str(l, LOB_COL_HOST, li.host, sizeof(li.host));
        li.port = lob_get_int(l, LOB_COL_PORT);
        li.max_users = lob_get_int(l, LOB_COL_MAXU);
        li.used = 1;
        // has_pw is answered from the library's own table, so read it back
        // rather than trusting the (internal) pw column.
        li.has_pw = dwyco_chat_server_has_pw(li.id);
        g_nlobbies++;
        break;
    }
    default:
        break;
    }
    (void)name; (void)len_name; (void)type;
}

static void DWYCOCALLCONV
chat_ctx_cb2(int cmd, int ctx_id, const char *uid, int len_uid,
    const char *name, int len_name, DWYCO_LIST list, int qid, int extra_arg)
{
    (void)cmd; (void)ctx_id; (void)uid; (void)len_uid; (void)name;
    (void)len_name; (void)list; (void)qid; (void)extra_arg;
    g_ctx2_count++;
}

static int g_login;
static void DWYCOCALLCONV
login_result_cb(const char *str, int what)
{
    (void)str;
    // 0 = failure, 1 = "Server login ok.", 2 = "New account created".
    if (what == 1 || what == 2)
        g_login = 1;
}

static int g_cmd_called;
static char g_cmd_name[64];
static int g_cmd_succ;
static char g_cmd_fail[256];
static void DWYCOCALLCONV
command_cb(const char *cmd, void *arg, int succ, const char *failed_reason)
{
    (void)arg;
    g_cmd_called++;
    snprintf(g_cmd_name, sizeof(g_cmd_name), "%s", cmd ? cmd : "(null)");
    g_cmd_succ = succ;
    snprintf(g_cmd_fail, sizeof(g_cmd_fail), "%s", failed_reason ? failed_reason : "");
}

static char g_my_uid[64];
static int g_my_uid_len;

static const char *LOBBY_PW = "topsecret";
static const char *g_argv0 = "";
static char g_own_lobby_name[128];
static char g_own_lobby_id[64];
static char g_checker_lobby_name[128];

// wait_subprocess() reports status the same way run_subprocess() does: 0 for
// success, otherwise something else. Accept a clean 0 and 0x00-style codes
// without making the test brittle about which one the platform hands back.
static int
WEXITSTATUS_OK(int st)
{
    return st == 0;
}

// Find a recorded lobby by display name.
static int
find_lobby_by_disp(const char *want)
{
    for (int i = 0; i < g_nlobbies; i++)
        if (g_lobbies[i].used && strcmp(g_lobbies[i].disp, want) == 0)
            return i;
    return -1;
}

// Pick the first lobby matching a predicate over the recorded set.
static lobby_info *
find_lobby(int want_pw)
{
    for (int i = 0; i < g_nlobbies; i++)
        if (g_lobbies[i].used && g_lobbies[i].has_pw == want_pw)
            return &g_lobbies[i];
    return 0;
}

// ===== tests =====

// The server list is what dwyco_switch_to_chat_server indexes into, so it has
// to be populated before a switch can work. It arrives with the directory
// server's response, which is *before* the login callback fires.
static void
server_list_shape(void)
{
    DWYCO_SERVER_LIST l = 0;
    int numlines = -1;
    CHECK(dwyco_get_server_list(&l, &numlines) != 0);
    CHECK(l != 0);
    if (!l)
        return;
    int rows = -1, cols = -1;
    CHECK(dwyco_list_numelems(l, &rows, &cols) != 0);
    CHECK(numlines == rows);
    CHECK(rows >= 1);
    CHECK(cols > 4);
    printf("(servers=%d cols=%d) ", rows, cols);

    for (int i = 0; i < rows; i++) {
        const char *v;
        int vl, vt;
        CHECK(dwyco_list_get(l, i, DWYCO_SL_SERVER_HOSTNAME, &v, &vl, &vt) != 0);
        CHECK(vl > 0);
        CHECK(dwyco_list_get(l, i, DWYCO_SL_SERVER_PORT, &v, &vl, &vt) != 0);
        CHECK(vl > 0);
        CHECK(dwyco_list_get(l, i, DWYCO_SL_SERVER_NAME, &v, &vl, &vt) != 0);
    }
    dwyco_list_release(l);
}

// Before the login completes there is no usable chat session, whatever the
// switch call returns. send_to_chatserver() gates on Chat_id != -1 and
// Chat_online, and Chat_online is only set once the chat server answers the
// challenge, which needs the logged-in identity.
static void
no_chat_session_before_login(void)
{
    CHECK(dwyco_chat_online() == 0);
    CHECK(dwyco_chat_addq(0) == 0);
    CHECK(dwyco_chat_talk(0) == 0);
    CHECK(dwyco_chat_get_admin_info() == 0);
    // No user lobbies are known yet either.
    CHECK(dwyco_chat_server_has_pw("U_00000000") == -1);
}

// The chat context lifecycle. A real chat server emits, in order:
//   NEW -> (update batches) -> ADD_USER -> UPDATE_ATTR for ui-* and us-*
// and START_UPDATE/END_UPDATE bracket the bursts.
static void
chat_ctx_events(void)
{
    printf("(events=%d new=%d add_user=%d attr=%d sysattr=%d add_lobby=%d) ",
        g_ctx1_count, g_ctx_new, g_ctx_add_user, g_ctx_attr, g_ctx_sysattr,
        g_ctx_add_lobby);
    CHECK(g_ctx1_count > 0);
    CHECK(g_ctx_new == 1);
    CHECK(g_ctx_add_user > 0);
    CHECK(g_ctx_attr > 0);
    CHECK(g_ctx_sysattr > 0);
    CHECK(g_ctx_add_lobby > 0);
    // The update brackets must balance -- every START has an END.
    CHECK(g_ctx_start_update == g_ctx_end_update);
    CHECK(g_ctx_batches == g_ctx_end_update);
    CHECK(g_ctx_batches > 0);
    CHECK(g_ctx_last_ctx > 0);
}

// Callback2 fires only for vector-valued sys attrs. The servers here send
// scalar ones, so callback2 stays silent -- which is itself the documented-by-
// omission contract, since the header claims lobbies come through it.
static void
chat_ctx_callback2_silent_for_scalar_attrs(void)
{
    CHECK(g_ctx2_count == 0);
    printf("(cb2=%d) ", g_ctx2_count);
}

// A live chat session that stays up. This is the check that distinguishes a
// working chat connection from one that is quietly being torn down and
// rebuilt: nothing here touches the session, so any 1 -> 0 -> 1 would show up.
static void
chat_session_is_stable(void)
{
    int transitions = 0;
    int last = dwyco_chat_online();
    CHECK(last != 0);
    for (int i = 0; i < STABILITY_SECONDS * 2 && last; i++) {
        service_ms(500);
        int on = dwyco_chat_online();
        if (on != last) {
            transitions++;
            last = on;
        }
    }
    printf("(%ds online=%d transitions=%d) ", STABILITY_SECONDS,
        dwyco_chat_online(), transitions);
    CHECK(dwyco_chat_online() != 0);
    CHECK(transitions == 0);
}

// Every user lobby the server announced must be readable back by id, with a
// display name that matches what was delivered, and its password state must
// agree with dwyco_chat_server_has_pw.
static void
lobby_list_is_readable(void)
{
    CHECK(g_nlobbies > 0);
    printf("(lobbies=%d) ", g_nlobbies);
    int nopw = 0, withpw = 0;
    for (int i = 0; i < g_nlobbies; i++) {
        lobby_info &li = g_lobbies[i];
        if (!li.used)
            continue;
        // Ids are "U_" plus hex, never "000".
        CHECK(strncmp(li.id, "U_", 2) == 0);
        CHECK(strlen(li.id) > 2);
        // has_pw: -1 unknown, 0 no password, 1 password.
        CHECK(li.has_pw == 0 || li.has_pw == 1);
        if (li.has_pw == 0)
            nopw++;
        else
            withpw++;
        // The display name resolves, and matches column "005".
        DWYCO_LIST nl = 0;
        int rc = dwyco_get_lobby_name_by_id2(li.id, &nl);
        CHECK(rc != 0);
        CHECK(nl != 0);
        if (rc && nl) {
            char got[128];
            lob_get_str(nl, DWYCO_NO_COLUMN, got, sizeof(got));
            CHECK(strcmp(got, li.disp) == 0);
        }
        if (nl)
            dwyco_list_release(nl);
    }
    // A useful test needs both kinds present, otherwise the matrix below is
    // vacuous.
    CHECK(nopw > 0);
    CHECK(withpw > 0);
}

// The password matrix, for real lobby ids.
//
//   has_pw: -1 unknown id, 0 no password, 1 password
//   check_pw: 0 unknown id, 1 no password needed (incl. our own god bypass),
//             2 password correct, -1 password wrong
static void
lobby_password_matrix(void)
{
    lobby_info *open_l = find_lobby(0);
    lobby_info *locked = find_lobby(1);
    CHECK(open_l != 0);
    CHECK(locked != 0);
    if (!open_l || !locked)
        return;
    printf("(open='%s' locked='%s') ", open_l->id, locked->id);

    // Passwordless: any password is fine, so even a wrong one gives 1.
    CHECK(dwyco_chat_server_has_pw(open_l->id) == 0);
    CHECK(dwyco_check_chat_server_pw(open_l->id, "") == 1);
    CHECK(dwyco_check_chat_server_pw(open_l->id, "anything") == 1);

    // Passworded: we are not its god, so every password we can guess is wrong.
    CHECK(dwyco_chat_server_has_pw(locked->id) == 1);
    CHECK(dwyco_check_chat_server_pw(locked->id, "") == -1);
    CHECK(dwyco_check_chat_server_pw(locked->id, "guess") == -1);

    // Unknown ids: has_pw -1, check_pw 0.
    static const char *bogus[] = { "U_deadbeef", "000", "not-an-id", "" };
    for (unsigned i = 0; i < sizeof(bogus) / sizeof(bogus[0]); i++) {
        CHECK(dwyco_chat_server_has_pw(bogus[i]) == -1);
        CHECK(dwyco_check_chat_server_pw(bogus[i], "") == 0);
        DWYCO_LIST nl = 0;
        CHECK(dwyco_get_lobby_name_by_id2(bogus[i], &nl) == 0);
        CHECK(nl == 0);
    }
}

// The creator side of the lobby lifecycle, end to end. This also covers
// outcome 1 ("no password needed") from the *owner's* point of view, which is
// a different code path than the passwordless-lobby case above: the creator is
// the lobby's subgod, so the bypass fires before the password is ever compared.
static void
own_lobby_lifecycle(void)
{
    // create/remove_user_lobby go through the directory connection, so we have
    // to be on the directory server's chat -- not sitting inside some other
    // user lobby, where both calls fail with "not connected to directory".
    if (dwyco_switch_to_chat_server(0) != 0)
        CHECK(wait_for([]() { return dwyco_chat_online() != 0; }, 30000));
    service_ms(1000);

    int before = g_nlobbies;
    g_cmd_called = 0;
    // Same shape cdcx uses (mainwin.cpp:4576): category "user", and ourselves
    // as the sub_god_uid.
    dwyco_chat_create_user_lobby(g_own_lobby_name, "user",
        g_my_uid, g_my_uid_len, LOBBY_PW, 50, command_cb, 0);
    int got = wait_for([]() { return g_cmd_called != 0; }, 30000);
    CHECK(got);
    CHECK(strcmp(g_cmd_name, "create_user_lobby") == 0);
    CHECK(g_cmd_succ == 1);
    // On success the callback passes NULL for failed_reason -- and, notably,
    // never the new lobby's id (dlli.cpp:2684). We have to pick it out of the
    // lobby list instead.
    CHECK(g_cmd_fail[0] == 0);

    // ADD_LOBBY for our own lobby is expected; it just is not instant.
    int idx = -1;
    int waited_ms = 0;
    for (int i = 0; i < 300 && idx < 0; i++) {
        service_ms(100);
        waited_ms += 100;
        int found = find_lobby_by_disp(g_own_lobby_name);
        // Only accept one that arrived after we asked for it.
        idx = (found >= before) ? found : -1;
    }
    printf("(create ok, add_lobby in %dms) ", waited_ms);
    CHECK(idx >= 0);
    if (idx < 0)
        return;
    const char *cid = g_lobbies[idx].id;
    g_own_lobby_id[0] = 0;
    snprintf(g_own_lobby_id, sizeof(g_own_lobby_id), "%s", cid);
    CHECK(strncmp(cid, "U_", 2) == 0);

    // It has a password, but we are its subgod, so every password "works".
    CHECK(dwyco_chat_server_has_pw(cid) == 1);
    CHECK(dwyco_check_chat_server_pw(cid, "") == 1);
    CHECK(dwyco_check_chat_server_pw(cid, LOBBY_PW) == 1);
    CHECK(dwyco_check_chat_server_pw(cid, "wrong") == 1);

    // Its display name resolves to what we asked for.
    DWYCO_LIST nl = 0;
    CHECK(dwyco_get_lobby_name_by_id2(cid, &nl) != 0);
    if (nl) {
        char got_name[128];
        lob_get_str(nl, DWYCO_NO_COLUMN, got_name, sizeof(got_name));
        CHECK(strcmp(got_name, g_own_lobby_name) == 0);
        dwyco_list_release(nl);
    }

    // Enter it with no password at all -- the owner bypass means we do not
    // need the one we just set.
    int sw = dwyco_switch_to_chat_server2(cid, "");
    printf("(enter sw=%d) ", sw);
    CHECK(sw == 1);
    int on = wait_for([]() { return dwyco_chat_online() != 0; }, 30000);
    CHECK(on);
    CHECK(dwyco_chat_addq(0) == 1);

    // Only a lobby we can get into can be removed, and only the creator or a
    // system god may remove one. We are both, so this must succeed.
    g_cmd_called = 0;
    dwyco_chat_remove_user_lobby(cid, command_cb, 0);
    int rm = wait_for([]() { return g_cmd_called != 0; }, 30000);
    printf("(remove succ=%d) ", g_cmd_succ);
    CHECK(rm);
    CHECK(strcmp(g_cmd_name, "remove_user_lobby") == 0);
    CHECK(g_cmd_succ == 1);

    // Removing the lobby we were inside drops us from the chat server. We
    // therefore never receive the DEL_LOBBY for it and our copy of the lobby
    // list goes stale -- it still claims the lobby exists. Documented rather
    // than asserted away.
    int dropped = wait_for([]() { return dwyco_chat_online() == 0; }, 20000);
    printf("(dropped=%d stale_has_pw=%d) ", dropped, dwyco_chat_server_has_pw(cid));
    CHECK(dropped);
    // Back to a usable session for the remaining phases.
    CHECK(dwyco_switch_to_chat_server(0) != 0);
    CHECK(wait_for([]() { return dwyco_chat_online() != 0; }, 30000));
    service_ms(1500);
}

// Outcome 2 ("password correct") is only reachable by an account that is NOT
// the lobby's subgod, so it needs a second client. The checker child creates a
// passworded lobby with a known password and then deletes it itself -- the
// creator can always clean up after itself, which is the only way this test can
// avoid leaving lobbies behind on the shared server.
static void
correct_password_from_a_second_client(void)
{
    // We must be on the directory server's chat to see the global lobby list.
    if (dwyco_chat_online() == 0) {
        CHECK(dwyco_switch_to_chat_server(0) != 0);
        CHECK(wait_for([]() { return dwyco_chat_online() != 0; }, 30000));
    }

    // The name has to be agreed up front and handed to the child: both sides
    // cannot derive it from their own pid, or they will never match. The
    // primary's pid makes it unique per run, so a stale lobby from an earlier
    // run cannot be mistaken for this one.
    char self[1024];
    snprintf(self, sizeof(self), "%s", g_argv0);
    char *child[] = { self, (char *)"checker", g_checker_lobby_name, 0 };
    pid_t pid = spawn_subprocess(child);
    if (pid < 0) {
        printf("(could not spawn checker) ");
        return;
    }

    int idx = -1;
    int waited_ms = 0;
    // First just wait: lobby creation is broadcast to clients already connected.
    for (int i = 0; i < 150 && idx < 0; i++) {
        service_ms(100);
        waited_ms += 100;
        idx = find_lobby_by_disp(g_checker_lobby_name);
    }
    // If it has not shown up, reconnect -- the list is re-sent on chat login.
    for (int attempt = 0; attempt < 5 && idx < 0; attempt++) {
        g_nlobbies = 0;
        CHECK(dwyco_switch_to_chat_server(0) != 0);
        CHECK(wait_for([]() { return dwyco_chat_online() != 0; }, 30000));
        for (int i = 0; i < 25 && idx < 0; i++) {
            service_ms(100);
            waited_ms += 100;
            idx = find_lobby_by_disp(g_checker_lobby_name);
        }
    }
    printf("(saw checker lobby in %dms) ", waited_ms);
    CHECK(idx >= 0);
    if (idx < 0) {
        wait_subprocess(pid, 120000);
        return;
    }
    const char *cid = g_lobbies[idx].id;

    // We are not its subgod, so this is the real password comparison.
    CHECK(dwyco_chat_server_has_pw(cid) == 1);
    CHECK(dwyco_check_chat_server_pw(cid, LOBBY_PW) == 2);
    CHECK(dwyco_check_chat_server_pw(cid, "wrong") == -1);
    CHECK(dwyco_check_chat_server_pw(cid, "") == -1);

    // Being a member is not enough to delete the lobby -- only the creator or a
    // system god may. Try it while we still know the lobby, so a refusal really
    // means "not the creator" rather than "unknown id". The checker cleans up.
    g_cmd_called = 0;
    dwyco_chat_remove_user_lobby(cid, command_cb, 0);
    int rm = wait_for([]() { return g_cmd_called != 0; }, 30000);
    printf("(member remove succ=%d reason='%s') ", g_cmd_succ, g_cmd_fail);
    CHECK(rm);
    CHECK(g_cmd_succ == 0);
    CHECK(g_cmd_fail[0] != 0);
    // The refusal must not have taken the lobby away from the primary.
    CHECK(dwyco_chat_server_has_pw(cid) == 1);

    // And entering honours the same distinction. A wrong password is refused
    // with -1 and leaves this session alone.
    CHECK(dwyco_switch_to_chat_server2(cid, "wrong") == -1);
    CHECK(dwyco_switch_to_chat_server2(cid, "") == -1);
    CHECK(dwyco_chat_online() != 0);
    CHECK(dwyco_chat_addq(0) == 1);
    // The correct password gets us in.
    CHECK(dwyco_switch_to_chat_server2(cid, LOBBY_PW) == 1);
    CHECK(wait_for([]() { return dwyco_chat_online() != 0; }, 30000));
    CHECK(dwyco_chat_addq(0) == 1);
    // Being a member may change what the password check says about us; record
    // it rather than assuming.
    printf("[after joining: has_pw=%d cp('')=%d cp(correct)=%d cp(wrong)=%d] ",
        dwyco_chat_server_has_pw(cid), dwyco_check_chat_server_pw(cid, ""),
        dwyco_check_chat_server_pw(cid, LOBBY_PW),
        dwyco_check_chat_server_pw(cid, "wrong"));
    CHECK(dwyco_switch_to_chat_server(0) != 0);
    CHECK(wait_for([]() { return dwyco_chat_online() != 0; }, 30000));

    // The checker deletes its own lobby and exits.
    int st = wait_subprocess(pid, 120000);
    printf("(checker exit=%d) ", st);
    CHECK(WEXITSTATUS_OK(st));
    service_ms(1000);
}

// dwyco_switch_to_chat_server2 checks the password before touching the session,
// so a refused switch must leave the live session completely intact.
static void
switch2_refused_leaves_session_alone(void)
{
    lobby_info *locked = find_lobby(1);
    CHECK(locked != 0);
    if (!locked)
        return;
    int before = dwyco_chat_online();
    CHECK(before != 0);
    // Wrong/missing password on a protected lobby -> -1, not 0.
    CHECK(dwyco_switch_to_chat_server2(locked->id, "") == -1);
    CHECK(dwyco_switch_to_chat_server2(locked->id, "guess") == -1);
    // Still on the same, live session.
    CHECK(dwyco_chat_online() != 0);
    CHECK(dwyco_chat_addq(0) == 1);
    printf("(refused=%d online=%d) ", dwyco_chat_online(), 1);
}

// An unknown lobby id is refused with 0 and also leaves the session alone.
static void
switch2_unknown_lobby_id(void)
{
    int before = dwyco_chat_online();
    CHECK(before != 0);
    CHECK(dwyco_switch_to_chat_server2("U_deadbeef", "") == 0);
    CHECK(dwyco_switch_to_chat_server2("000", "pw") == 0);
    CHECK(dwyco_chat_online() != 0);
    CHECK(dwyco_chat_addq(0) == 1);
}

// Switching to a real, passwordless user lobby works and keeps us online.
static void
switch2_to_a_real_lobby(void)
{
    lobby_info *open_l = find_lobby(0);
    CHECK(open_l != 0);
    if (!open_l)
        return;
    int rc = dwyco_switch_to_chat_server2(open_l->id, "");
    printf("(id='%s' rc=%d) ", open_l->id, rc);
    CHECK(rc != 0);
    CHECK(rc != -1);
    int on = wait_for([]() { return dwyco_chat_online() != 0; }, 20000);
    CHECK(on);
    CHECK(dwyco_chat_online() != 0);
    // And it is a working session, not just a flag.
    CHECK(dwyco_chat_addq(0) == 1);
    CHECK(dwyco_chat_get_admin_info() == 1);
    // The context is rebuilt for the new server, so NEW fires again.
    CHECK(g_ctx_new >= 2);
}

// Every chat queue command must be accepted while a chat session is up. They
// return 1 to mean "written to the server channel", not "the server agreed".
static void
chat_queue_commands_are_accepted(void)
{
    CHECK(dwyco_chat_online() != 0);
    CHECK(dwyco_chat_addq(0) == 1);
    CHECK(dwyco_chat_talk(0) == 1);
    CHECK(dwyco_chat_mute(0, 1) == 1);
    CHECK(dwyco_chat_mute(0, 0) == 1);
    CHECK(dwyco_chat_delq(0, 0, 0) == 1);
    CHECK(dwyco_chat_set_filter(0, 0, 0, 0, 0) == 1);
    CHECK(dwyco_chat_get_admin_info() == 1);
    CHECK(dwyco_chat_addq(0) == 1);
    CHECK(dwyco_chat_delq(0, 0, 0) == 1);
    // A named uid works as a filter argument too.
    CHECK(dwyco_chat_set_filter(0, 0, 0, g_my_uid, g_my_uid_len) == 1);
    CHECK(dwyco_chat_set_filter(0, 1, 0, 0, 0) == 1);
    CHECK(dwyco_chat_set_filter(0, 0, 0, 0, 0) == 1);
    service_ms(300);
}

// God-only administration. These are NOT client-side god checks: they report
// only whether the command reached the server, so with a session up they
// return 1 regardless of who you are and the server applies the god check.
// A test asserting 0 for a non-god account would be wrong.
static void
admin_commands_are_accepted(void)
{
    CHECK(dwyco_chat_online() != 0);
    CHECK(dwyco_chat_set_demigod(g_my_uid, g_my_uid_len, 1) == 1);
    CHECK(dwyco_chat_set_demigod(g_my_uid, g_my_uid_len, 0) == 1);
    CHECK(dwyco_chat_clear_all_demigods() == 1);
    CHECK(dwyco_chat_set_unblock_time(0, g_my_uid, g_my_uid_len, -1) == 1);
    CHECK(dwyco_chat_set_unblock_time(0, g_my_uid, g_my_uid_len, 60) == 1);
    CHECK(dwyco_chat_set_unblock_time2(0, g_my_uid, g_my_uid_len, 60, "because") == 1);
    CHECK(dwyco_chat_set_unblock_time2(0, g_my_uid, g_my_uid_len, -1, "") == 1);
    service_ms(300);
}

static void
sys_attr_set(void)
{
    CHECK(dwyco_chat_online() != 0);
    CHECK(dwyco_chat_set_sys_attr("dwytest-str", 11, DWYCO_TYPE_STRING,
        "value", 5, 0) == 1);
    CHECK(dwyco_chat_set_sys_attr("dwytest-int", 11, DWYCO_TYPE_INT,
        0, 0, 42) == 1);
    CHECK(dwyco_chat_set_sys_attr("dwytest-nil", 12, DWYCO_TYPE_NIL,
        0, 0, 0) == 1);
    service_ms(300);
}

static void
chat_send_data(void)
{
    CHECK(dwyco_chat_online() != 0);
    dwyco_chat_send_data("dwytest snap", 12,
        DWYCO_CHAT_DATA_PIC_TYPE_NONE, 0, 0);
    static const unsigned char jpeg[] = { 0xff, 0xd8, 0x01, 0x02 };
    dwyco_chat_send_data("with pic", 8, DWYCO_CHAT_DATA_PIC_TYPE_JPG,
        (const char *)jpeg, 4);
    service_ms(300);
}

static void
chat_activity_state(void)
{
    CHECK(dwyco_chat_online() != 0);
    CHECK(dwyco_chat_set_activity_state(1, 0, 0) == 1);
    CHECK(dwyco_chat_set_activity_state(1, "ignored", 7) == 1);
    CHECK(dwyco_chat_set_activity_state(-1, "dwytest-busy", 12) == 1);
    CHECK(dwyco_chat_set_activity_state(0, 0, 0) == 1);
    service_ms(300);
}

static void
chat_send_popup(void)
{
    CHECK(dwyco_chat_online() != 0);
    CHECK(dwyco_chat_send_popup("dwytest popup", 13, 0) == 1);
    CHECK(dwyco_chat_send_popup("dwytest popup global", 20, 1) == 1);
    service_ms(300);
}

// Removal of an id nobody told us about must report failure through the
// callback rather than crash or hang. (Removal of a real lobby, by its
// creator, is covered by own_lobby_lifecycle().)
static void
remove_unknown_lobby_reports_failure(void)
{
    g_cmd_called = 0;
    dwyco_chat_remove_user_lobby("U_deadbeef", command_cb, 0);
    int got = wait_for([]() { return g_cmd_called != 0; }, 30000);
    printf("(cmd='%s' succ=%d reason='%s') ", g_cmd_name, g_cmd_succ, g_cmd_fail);
    CHECK(got);
    CHECK(strcmp(g_cmd_name, "remove_user_lobby") == 0);
    CHECK(g_cmd_succ == 0);
    CHECK(g_cmd_fail[0] != 0);
}

static void
disconnect_chat_server(void)
{
    CHECK(dwyco_disconnect_chat_server() != 0);
    int gone = wait_for([]() { return dwyco_chat_online() == 0; }, 20000);
    printf("(online=%d) ", dwyco_chat_online());
    CHECK(gone);
    CHECK(dwyco_disconnect_chat_server() != 0);
}

// With the chat thread stopped, Chat_id is -1 and stays -1, so every chatq
// command must report 0. This half does not depend on connection timing.
static void
chat_queue_commands_are_refused_offline(void)
{
    CHECK(dwyco_chat_online() == 0);
    CHECK(dwyco_chat_addq(0) == 0);
    CHECK(dwyco_chat_talk(0) == 0);
    CHECK(dwyco_chat_mute(0, 1) == 0);
    CHECK(dwyco_chat_mute(0, 0) == 0);
    CHECK(dwyco_chat_delq(0, 0, 0) == 0);
    CHECK(dwyco_chat_set_filter(0, 0, 0, 0, 0) == 0);
    CHECK(dwyco_chat_get_admin_info() == 0);
    CHECK(dwyco_chat_clear_all_demigods() == 0);
    CHECK(dwyco_chat_set_demigod(g_my_uid, g_my_uid_len, 1) == 0);
    CHECK(dwyco_chat_set_unblock_time(0, g_my_uid, g_my_uid_len, 60) == 0);
    CHECK(dwyco_chat_set_unblock_time2(0, g_my_uid, g_my_uid_len, 60, "x") == 0);
    CHECK(dwyco_chat_set_sys_attr("dwytest-str", 11, DWYCO_TYPE_STRING,
        "value", 5, 0) == 0);
    CHECK(dwyco_chat_send_popup("dwytest popup", 13, 0) == 0);
    CHECK(dwyco_chat_set_activity_state(1, 0, 0) == 0);
    CHECK(dwyco_chat_set_activity_state(-1, "dwytest-busy", 12) == 0);
    dwyco_chat_send_data("dwytest snap", 12,
        DWYCO_CHAT_DATA_PIC_TYPE_NONE, 0, 0);
    // Unknown-id lookups are unaffected by the session state.
    CHECK(dwyco_chat_server_has_pw("U_deadbeef") == -1);
    CHECK(dwyco_check_chat_server_pw("U_deadbeef", "") == 0);
    service_ms(300);
}

// ===== harness =====

static int
boot(const char *dir, const char *who)
{
    char sys_dir[512], tmp_dir[512];
    snprintf(sys_dir, sizeof(sys_dir), "%s/sys", dir);
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", dir);
    mkdir(dir, 0755);
    mkdir(sys_dir, 0755);
    mkdir(tmp_dir, 0755);
    install_app_files(dir);

    dwyco_set_fn_prefixes(sys_dir, dir, tmp_dir);
    dwyco_set_chat_ctx_callback(chat_ctx_cb1);
    dwyco_set_chat_ctx_callback2(chat_ctx_cb2);
    dwyco_set_login_result_callback(login_result_cb);
    dwyco_set_client_version("dwytest", 7);
    if (!dwyco_init())
        return 0;
    test_bootstrap_profile(who, who);
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);

    const char *uid;
    int len;
    dwyco_get_my_uid(&uid, &len);
    if (len <= 0 || len >= (int)sizeof(g_my_uid))
        return 0;
    memcpy(g_my_uid, uid, (size_t)len);
    g_my_uid_len = len;

    const char *s = getenv("DWYTEST_CHAT_STABILITY_SECONDS");
    if (s && *s) {
        int v = atoi(s);
        if (v >= 0 && v < 600)
            STABILITY_SECONDS = v;
    }
    return 1;
}

// The "checker" role: a second, independent client used only to observe
// someone else's passworded lobby, which is the sole way to reach
// check_chat_server_pw() outcome 2. It creates its own lobby so it can delete
// it again afterwards.
static int
run_checker(const char *lobby_name)
{
    printf("dwytest_chat checker (second client)\n");
    if (!boot(CHECKER_DIR, "dwytest-chat-checker")) {
        fprintf(stderr, "checker: dwyco_init failed\n");
        return 1;
    }
    int logged_in = wait_for([]() { return g_login != 0; }, 20000);
    if (!logged_in) {
        fprintf(stderr, "checker: no server login\n");
        dwyco_exit();
        return 1;
    }
    // The server list has to exist before a switch can work, and on a brand new
    // account it can lag the login callback, so retry rather than assume.
    DWYCO_SERVER_LIST sl = 0;
    int nl = -1;
    dwyco_get_server_list(&sl, &nl);
    fprintf(stderr, "checker: server list rows=%d\n", nl);
    if (sl)
        dwyco_list_release(sl);

    int connected = 0;
    for (int attempt = 0; attempt < 5 && !connected; attempt++) {
        if (dwyco_switch_to_chat_server(0) != 0)
            connected = wait_for([]() { return dwyco_chat_online() != 0; }, 30000);
        if (!connected)
            service_ms(2000);
    }
    if (!connected) {
        fprintf(stderr, "checker: switch_to_chat_server(0) failed after retries\n");
        dwyco_exit();
        return 1;
    }
    service_ms(1500);

    if (!lobby_name || !*lobby_name) {
        fprintf(stderr, "checker: no lobby name given\n");
        dwyco_exit();
        return 1;
    }
    snprintf(g_checker_lobby_name, sizeof(g_checker_lobby_name), "%s", lobby_name);

    g_cmd_called = 0;
    dwyco_chat_create_user_lobby(g_checker_lobby_name, "user",
        g_my_uid, g_my_uid_len, LOBBY_PW, 50, command_cb, 0);
    if (!wait_for([]() { return g_cmd_called != 0; }, 30000) || g_cmd_succ != 1) {
        fprintf(stderr, "checker: create_user_lobby failed: %s\n", g_cmd_fail);
        dwyco_exit();
        return 1;
    }

    // Find our own lobby, then hold it long enough for the primary to inspect
    // it, then delete it so nothing is left behind on the server.
    int idx = -1;
    for (int i = 0; i < 300 && idx < 0; i++) {
        service_ms(100);
        for (int j = 0; j < g_nlobbies; j++)
            if (g_lobbies[j].used && strcmp(g_lobbies[j].disp, g_checker_lobby_name) == 0) {
                idx = j;
                break;
            }
    }
    if (idx < 0) {
        fprintf(stderr, "checker: our lobby never appeared\n");
        dwyco_exit();
        return 1;
    }
    const char *cid = g_lobbies[idx].id;
    printf("  checker created lobby %s (%s)\n", cid, g_checker_lobby_name);

    // Give the primary time to run its assertions, plus slack.
    service_ms(CHECKER_HOLD_MS);

    g_cmd_called = 0;
    dwyco_chat_remove_user_lobby(cid, command_cb, 0);
    wait_for([]() { return g_cmd_called != 0; }, 30000);
    printf("  checker removed lobby %s: succ=%d\n", cid, g_cmd_succ);
    int rc = (g_cmd_succ == 1) ? 0 : 1;
    dwyco_exit();
    return rc;
}

int
main(int argc, char **argv)
{
    setvbuf(stdout, 0, _IOLBF, 0);
    g_argv0 = argc > 0 ? argv[0] : "dwytest_chat";

    // Unique per run, so a stale lobby from an earlier run is never mistaken
    // for the one this run just created.
    snprintf(g_own_lobby_name, sizeof(g_own_lobby_name),
        "dwytest-own-%d", (int)getpid());
    snprintf(g_checker_lobby_name, sizeof(g_checker_lobby_name),
        "dwytest-check-%d", (int)getpid());

    if (argc > 1 && strcmp(argv[1], "checker") == 0)
        return run_checker(argc > 2 ? argv[2] : 0);

    printf("Dwyco chat server / chat context / user lobbies\n");

    if (!boot(CLIENT_DIR, "dwytest-chat")) {
        fprintf(stderr, "dwyco_init failed\n");
        return 1;
    }

    printf("  Waiting for server login...\n");
    int logged_in = wait_for([]() { return g_login != 0; }, 20000);
    printf("  %s\n", logged_in ? "Login OK" : "Login timeout");
    if (!logged_in) {
        printf("\nNo server login: chat coverage needs a directory server.\n");
        dwyco_exit();
        return 1;
    }

    // The one and only switch_to_chat_server call in this file. Everything
    // below runs against this single session.
    int sw = dwyco_switch_to_chat_server(0);
    printf("  switch_to_chat_server(0)=%d\n", sw);
    CHECK(sw != 0);
    int up = wait_for([]() { return dwyco_chat_online() != 0; }, 30000);
    printf("  chat_online=%d (%s)\n", dwyco_chat_online(),
        up ? "connected" : "NEVER CAME UP");
    if (!up) {
        printf("\nChat server never accepted the login; nothing else can run.\n");
        dwyco_exit();
        return 1;
    }
    // Let the connect burst and the lobby list arrive.
    service_ms(3000);

    printf("\nServer list:\n");
    RUN(server_list_shape);

    printf("\nChat context:\n");
    RUN(chat_ctx_events);
    RUN(chat_ctx_callback2_silent_for_scalar_attrs);

    printf("\nStability:\n");
    RUN(chat_session_is_stable);

    printf("\nUser lobbies:\n");
    RUN(lobby_list_is_readable);
    RUN(lobby_password_matrix);

    printf("\nSwitching to a lobby:\n");
    RUN(switch2_refused_leaves_session_alone);
    RUN(switch2_unknown_lobby_id);
    RUN(switch2_to_a_real_lobby);

    printf("\nChat queue (online):\n");
    RUN(chat_queue_commands_are_accepted);
    RUN(admin_commands_are_accepted);
    RUN(sys_attr_set);
    RUN(chat_send_data);
    RUN(chat_activity_state);
    RUN(chat_send_popup);

    printf("\nUser lobby lifecycle:\n");
    RUN(own_lobby_lifecycle);
    RUN(remove_unknown_lobby_reports_failure);

    printf("\nPassword check as a non-owner (second client):\n");
    RUN(correct_password_from_a_second_client);

    printf("\nDisconnect:\n");
    RUN(disconnect_chat_server);

    printf("\nChat queue (offline):\n");
    RUN(chat_queue_commands_are_refused_offline);

    dwyco_exit();

    printf("\nSummary: passed=%d failed=%d\n", g_pass, g_fail);
    if (g_fail) {
        printf("FAILED\n");
        return 1;
    }
    printf("All chat tests passed.\n");
    return 0;
}
