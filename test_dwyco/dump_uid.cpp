#include <cstdio>
#include <cstring>
#include <sys/stat.h>

#include "dlli.h"
#include "test_common.h"

int
main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: %s <user_dir>\n", argv[0]); return 1; }
    const char *user_dir = argv[1];
    char tmp_dir[512];
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", user_dir);
    mkdir(user_dir, 0755);
    mkdir(tmp_dir, 0755);
    install_app_files(user_dir);

    dwyco_set_fn_prefixes(user_dir, user_dir, tmp_dir);
    if (!dwyco_init()) { fprintf(stderr, "init failed\n"); return 1; }
    test_bootstrap_profile("dwytest", "dwytest");
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);

    const char *uid;
    int uid_len;
    dwyco_get_my_uid(&uid, &uid_len);
    printf("UID_LEN=%d\n", uid_len);
    printf("UID_HEX=");
    for (int i = 0; i < uid_len; i++)
        printf("%02x", (unsigned char)uid[i]);
    printf("\n");
    dwyco_exit();
    return 0;
}
