#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>

#include "dlli.h"
#include "test_common.h"

int
main(int argc, char **argv)
{
    if (argc < 3) { fprintf(stderr, "usage: %s <user_dir> <peer_uid_hex>\n", argv[0]); return 1; }
    const char *user_dir = argv[1];
    const char *peer_hex = argv[2];
    char uid[64];
    int uid_len;
    if (!test_uid_arg(peer_hex, "Peer UID", uid, sizeof(uid), &uid_len))
        return 1;
    if (uid_len != 10) { fprintf(stderr, "peer must be 10 bytes\n"); return 1; }
    char tmp_dir[512];
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", user_dir);
    mkdir(user_dir, 0755);
    mkdir(tmp_dir, 0755);
    install_app_files(user_dir);
    dwyco_set_fn_prefixes(user_dir, user_dir, tmp_dir);
    test_bootstrap_profile("dwytest", "dwytest");
    if (!dwyco_init()) { fprintf(stderr, "init failed\n"); return 1; }
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);
    dwyco_pal_add(uid, uid_len);
    service_ms(500);
    printf("is_pal=%d\n", dwyco_is_pal(uid, uid_len));
    dwyco_exit();
    return 0;
}
