#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>

#include "dlli.h"
#include "test_common.h"

static int g_login_done;

static void DWYCOCALLCONV
sys_event_cb(int cmd, int ctx_id, const char *uid, int len_uid,
    const char *name, int len_name, int type, const char *value, int val_len,
    int qid, int extra_arg)
{
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

int
main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: %s <user_dir> [-] (wait 2 minutes before exiting, 3 args means exit immediately)\n", argv[0]); return 1; }
    const char *user_dir = argv[1];
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
    test_bootstrap_profile("dwytest", "create_account");
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);

    printf("  Waiting for login...\n");
    if (!wait_login(120000)) {
        fprintf(stderr, "Login timeout\n");
        dwyco_exit();
        return 1;
    }
    printf("  Login OK\n");

    const char *uid;
    int uid_len;
    dwyco_get_my_uid(&uid, &uid_len);
    printf("DIR=%s\n", user_dir);
    printf("UID_HEX=");
    for (int i = 0; i < uid_len; i++)
        printf("%02x", (unsigned char)uid[i]);
    printf("\n");
    if(argc < 3)
    {
        g_login_done = 0;
        wait_login(120000);
    }
    dwyco_exit();
    return 0;
}
