#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/stat.h>

#include "dlli.h"
#include "test_common.h"

static int g_login_done;
static int g_join_result = -1; // -1 pending, 0 fail, 1 ok
static std::string g_join_name;

static void DWYCOCALLCONV
sys_event_cb(int cmd, int ctx_id, const char *uid, int len_uid,
    const char *name, int len_name, int type, const char *value, int val_len,
    int qid, int extra_arg)
{
    if (cmd == DWYCO_SE_SERVER_LOGIN)
        g_login_done = 1;
    if (cmd == DWYCO_SE_GRP_JOIN_OK) {
        g_join_result = 1;
        if (name && len_name > 0)
            g_join_name = std::string(name, len_name);
    } else if (cmd == DWYCO_SE_GRP_JOIN_FAIL) {
        g_join_result = 0;
        if (name && len_name > 0)
            g_join_name = std::string(name, len_name);
    }
}

static void DWYCOCALLCONV
login_cb(const char *str, int what)
{
    if (what == 1 || what == 2)
        g_login_done = 1;
}

static void DWYCOCALLCONV
emergency_cb(int problem, int must_exit, const char *msg)
{
    fprintf(stderr, "EMERGENCY: problem=%d must_exit=%d msg=%s\n",
        problem, must_exit, msg ? msg : "null");
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

static int
wait_join(int timeout_ms)
{
    int elapsed = 0;
    while (elapsed < timeout_ms) {
        int spin, next;
        next = dwyco_service_channels(&spin);
        if (next <= 0 || next > 50) next = 50;
        usleep(next * 1000);
        elapsed += next;
        if (g_join_result != -1) return 1;
    }
    return 0;
}

int
main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "usage: %s <user_dir> <group_name> [password]\n", argv[0]);
        fprintf(stderr, "  user_dir: client data directory (created if needed)\n");
        fprintf(stderr, "  group_name: short random string, e.g. 8 chars\n");
        fprintf(stderr, "  password: group password (default: foofoo)\n");
        return 1;
    }
    const char *user_dir = argv[1];
    const char *group_name = argv[2];
    const char *password = (argc >= 4) ? argv[3] : "foofoo";

    if (strlen(group_name) == 0) {
        fprintf(stderr, "group_name must be non-empty\n");
        return 1;
    }
    if (strlen(password) == 0) {
        fprintf(stderr, "password must be non-empty\n");
        return 1;
    }

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

    if (!dwyco_init()) { fprintf(stderr, "init failed\n"); return 1; }
    test_bootstrap_profile("dwytest", "join_group");
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);

    printf("Waiting for login...\n");
    if (!wait_login(120000)) {
        fprintf(stderr, "Login timeout\n");
        dwyco_exit();
        return 1;
    }
    printf("Login OK\n");

    const char *uid;
    int uid_len;
    dwyco_get_my_uid(&uid, &uid_len);
    printf("UID_HEX=");
    for (int i = 0; i < uid_len; i++)
        printf("%02x", (unsigned char)uid[i]);
    printf("\n");

    // Check current group status before attempting join
    {
        DWYCO_LIST gs = 0;
        if (dwyco_get_group_status(&gs) && gs) {
            int rows, cols;
            dwyco_list_numelems(gs, &rows, &cols);
            if (rows > 0) {
                const char *val; int len, type;
                if (dwyco_list_get(gs, 0, DWYCO_GS_GNAME, &val, &len, &type) == 1 && type == DWYCO_TYPE_STRING && len > 0) {
                    std::string cur(val, len);
                    printf("Already in group: %s\n", cur.c_str());
                    if (cur == group_name) {
                        printf("Already joined requested group %s\n", group_name);
                        dwyco_list_release(gs);
                        dwyco_exit();
                        return 0;
                    } else {
                        fprintf(stderr, "In different group %s, must leave first\n", cur.c_str());
                        dwyco_list_release(gs);
                        dwyco_exit();
                        return 1;
                    }
                }
            }
            dwyco_list_release(gs);
        }
    }

    printf("Joining group '%s' with password '%s'...\n", group_name, password);
    g_join_result = -1;
    g_join_name.clear();
    int rc = dwyco_start_gj2(group_name, password);
    if (!rc) {
        fprintf(stderr, "dwyco_start_gj2 failed immediately (already in group or invalid args)\n");
        dwyco_exit();
        return 1;
    }

    printf("Waiting for group join result...\n");
    if (!wait_join(120000)) {
        fprintf(stderr, "Group join timeout (no JOIN_OK/FAIL event)\n");
        dwyco_exit();
        return 1;
    }

    if (g_join_result == 1 && !g_join_name.empty()) {
        printf("Group join OK: %s\n", g_join_name.c_str());
        // Verify group status: GNAME must be a string of length > 0
        DWYCO_LIST gs = 0;
        if (dwyco_get_group_status(&gs) && gs) {
            int rows, cols;
            dwyco_list_numelems(gs, &rows, &cols);
            const char *val; int len, type;
            if (dwyco_list_get(gs, 0, DWYCO_GS_GNAME, &val, &len, &type) == 1 && type == DWYCO_TYPE_STRING && len > 0)
                printf("Group status GNAME=%.*s\n", len, val);
            else
                printf("Group status: no valid GNAME (type=%d len=%d)\n", type, len);
            dwyco_list_release(gs);
        }
        // Final success requires GNAME is a string of length > 0
        {
            DWYCO_LIST gs2 = 0;
            bool ok = false;
            if (dwyco_get_group_status(&gs2) && gs2) {
                const char *val; int len, type;
                if (dwyco_list_get(gs2, 0, DWYCO_GS_GNAME, &val, &len, &type) == 1 && type == DWYCO_TYPE_STRING && len > 0)
                    ok = true;
                dwyco_list_release(gs2);
            }
            if (!ok) {
                fprintf(stderr, "Group join reported OK but group name is not a string of length > 0\n");
                dwyco_exit();
                return 1;
            }
        }
        dwyco_exit();
        return 0;
    } else {
        fprintf(stderr, "Group join FAIL: %s\n", g_join_name.c_str());
        dwyco_exit();
        return 1;
    }
}
