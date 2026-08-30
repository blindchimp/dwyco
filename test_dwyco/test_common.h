#ifndef DWYCO_TEST_COMMON_H
#define DWYCO_TEST_COMMON_H

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#ifndef DWYCO_TEST_APP_DIR
#define DWYCO_TEST_APP_DIR "app"
#endif

// Populate a fresh client dir with the contents of the app dir
// (public key, servers2, no_img.png). Existing files are not
// overwritten so a previously updated servers2 is preserved.
// No-op if the app dir is missing; the core has a compiled-in fallback.

static void
test_copy_file(const char *src, const char *dst)
{
    FILE *in = fopen(src, "rb");
    if (!in) return;
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return; }
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
        fwrite(buf, 1, n, out);
    fclose(out);
    fclose(in);
}

static void
install_app_files(const char *user_dir)
{
    DIR *d = opendir(DWYCO_TEST_APP_DIR);
    if (!d) return;
    struct dirent *e;
    struct stat st;
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.') continue;
        std::string src = std::string(DWYCO_TEST_APP_DIR) + "/" + e->d_name;
        if (stat(src.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) continue;
        std::string dst = std::string(user_dir) + "/" + e->d_name;
        if (stat(dst.c_str(), &st) == 0) continue;
        test_copy_file(src.c_str(), dst.c_str());
    }
    closedir(d);
}

// Set a user-readable name + description for a freshly created
// account so it is easy to identify in server logs. Must be called
// after dwyco_init() and before dwyco_finish_startup(); no-op when
// the account already exists.
static void
test_bootstrap_profile(const char *handle, const char *desc)
{
    if (!dwyco_get_create_new_account())
        return;
    if (!handle) handle = "dwytest";
    if (!desc) desc = "dwytest account";
    dwyco_create_bootstrap_profile(handle, (int)strlen(handle),
        desc, (int)strlen(desc), "", 0, "", 0);
    printf("    Bootstrap profile: %s - %s\n", handle, desc);
}

#endif