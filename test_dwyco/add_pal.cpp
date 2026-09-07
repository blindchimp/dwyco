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
    int hlen = strlen(peer_hex);
    if (hlen != 20) { fprintf(stderr, "peer must be 20 hex chars\n"); return 1; }
    char uid[10];
    for (int i = 0; i < 10; i++) {
        unsigned b;
        if (sscanf(peer_hex + i * 2, "%2x", &b) != 1) { fprintf(stderr, "bad hex\n"); return 1; }
        uid[i] = (char)b;
    }
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
    dwyco_pal_add(uid, 10);
    int spin;
    for (int i = 0; i < 10; i++) dwyco_service_channels(&spin);
    printf("is_pal=%d\n", dwyco_is_pal(uid, 10));
    dwyco_exit();
    return 0;
}
