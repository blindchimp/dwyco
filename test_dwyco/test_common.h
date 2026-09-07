#ifndef DWYCO_TEST_COMMON_H
#define DWYCO_TEST_COMMON_H

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

// Map DWYCO_SE* event numbers to human-readable names.
// Keep the entries sorted by value and leave gaps where the
// original numbering has them (e.g. 25 is missing).
struct dwyco_se_name
{
    int         id;
    const char *name;
};

static const dwyco_se_name dwyco_se_names[] =
{
    {  1, "DWYCO_SE_USER_STATUS_CHANGE" },
    {  2, "DWYCO_SE_USER_ADD" },
    {  3, "DWYCO_SE_USER_DEL" },
    {  4, "DWYCO_SE_SERVER_CONNECTING" },
    {  5, "DWYCO_SE_SERVER_CONNECTION_SUCCESSFUL" },
    {  6, "DWYCO_SE_SERVER_DISCONNECT" },
    {  7, "DWYCO_SE_SERVER_LOGIN" },
    {  8, "DWYCO_SE_SERVER_LOGIN_FAILED" },
    {  9, "DWYCO_SE_USER_MSG_RECEIVED" },
    { 10, "DWYCO_SE_USER_UID_RESOLVED" },
    { 11, "DWYCO_SE_USER_PROFILE_INVALIDATE" },
    { 12, "DWYCO_SE_USER_MSG_IDX_UPDATED" },
    { 13, "DWYCO_SE_USER_MSG_IDX_UPDATED_PREPEND" },
    { 14, "DWYCO_SE_MSG_SEND_START" },
    { 15, "DWYCO_SE_MSG_SEND_FAIL" },
    { 16, "DWYCO_SE_MSG_SEND_SUCCESS" },
    { 17, "DWYCO_SE_MSG_SEND_STATUS" },
    { 18, "DWYCO_SE_MSG_SEND_DELIVERY_SUCCESS" },
    { 19, "DWYCO_SE_MSG_SEND_CANCELED" },
    { 20, "DWYCO_SE_MSG_DOWNLOAD_START" },
    { 21, "DWYCO_SE_MSG_DOWNLOAD_FAILED" },
    { 22, "DWYCO_SE_MSG_DOWNLOAD_FETCHING_ATTACHMENT" },
    { 23, "DWYCO_SE_MSG_DOWNLOAD_ATTACHMENT_FETCH_FAILED" },
    { 24, "DWYCO_SE_MSG_DOWNLOAD_OK" },
    { 26, "DWYCO_SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED" },
    { 27, "DWYCO_SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED_DECRYPT_FAILED" },
    { 28, "DWYCO_SE_CHAT_SERVER_CONNECTING" },
    { 29, "DWYCO_SE_CHAT_SERVER_CONNECTION_SUCCESSFUL" },
    { 30, "DWYCO_SE_CHAT_SERVER_DISCONNECT" },
    { 31, "DWYCO_SE_CHAT_SERVER_LOGIN" },
    { 32, "DWYCO_SE_CHAT_SERVER_LOGIN_FAILED" },
    { 33, "DWYCO_SE_GRP_JOIN_OK" },
    { 34, "DWYCO_SE_GRP_JOIN_FAIL" },
    { 35, "DWYCO_SE_MSG_DOWNLOAD_PROGRESS" },
    { 36, "DWYCO_SE_MSG_PULL_OK" },
    { 37, "DWYCO_SE_MSG_TAG_CHANGE" },
    { 38, "DWYCO_SE_GRP_STATUS_CHANGE" },
    { 39, "DWYCO_SE_IGNORE_LIST_CHANGE" },
    { 40, "DWYCO_SE_IDENT_TO_UID" },
    { 41, "DWYCO_SE_SERVER_ATTR" },
    { 50, "DWYCO_SE_TOX_FRIEND_REQUEST" },
    { 51, "DWYCO_SE_TOX_MESSAGE" },
    { 52, "DWYCO_SE_TOX_READ_RECEIPT" },
    { 53, "DWYCO_SE_TOX_FRIEND_STATUS" },
    { 54, "DWYCO_SE_TOX_FRIEND_NAME" },
    { 55, "DWYCO_SE_TOX_FILE_REQUEST" },
    { 56, "DWYCO_SE_TOX_FILE_CHUNK" },
    { 57, "DWYCO_SE_TOX_SELF_CONNECTION_STATUS" },
    { 58, "DWYCO_SE_TOX_READY" },
    { 59, "DWYCO_SE_TOX_CRASHED" },
    { 60, "DWYCO_SE_TOX_TYPING" },
    { 61, "DWYCO_SE_TOX_FRIEND_USER_STATUS" },
    { 62, "DWYCO_SE_TOX_AVATAR" },
    {  0, nullptr }
};

static const char *
dwyco_se_name_lookup(int id)
{
    for (const dwyco_se_name *e = dwyco_se_names; e->name; ++e)
        if (e->id == id)
            return e->name;
    return "(unknown)";
}

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