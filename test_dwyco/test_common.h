#ifndef DWYCO_TEST_COMMON_H
#define DWYCO_TEST_COMMON_H

// Shared test harness. Assumes "dlli.h" has already been included.
//
// This header is deliberately header-only with static functions, matching
// the existing convention in this directory. The build passes
// -Wno-unused-function, so helpers that a given binary does not use are
// fine.

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <functional>
#include <string>
#include <cerrno>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>

// ===== uid hex codec =====
//
// A dwyco uid is 10 raw bytes. It travels on the command line, and shows
// up in log/print output, as 20 ascii hex chars -- the same hex form the
// hand-written database code stores uids in. These two helpers are the
// only place that conversion is spelled out; they used to be duplicated
// in test_main.cpp, test_peer.cpp (twice) and add_pal.cpp.

static int
dwyco_test_hexval(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Decode an ascii-hex uid into a binary buffer. Returns 1 on success and
// sets *len_out to the byte count; returns 0 for anything malformed (odd
// length, non-hex char, or an out buffer too small to NUL-terminate).
// Note this deliberately rejects what sscanf("%2x") would happily accept,
// like " 1", "0x" or a trailing sign.
static int
uid_from_hex(const char *hex, char *out, int out_size, int *len_out)
{
    int hlen, i;
    if (!hex || !out || !len_out || out_size < 1)
        return 0;
    hlen = (int)strlen(hex);
    if (hlen <= 0 || (hlen % 2) != 0 || hlen / 2 >= out_size)
        return 0;
    for (i = 0; i < hlen; i += 2) {
        int hi = dwyco_test_hexval(hex[i]);
        int lo = dwyco_test_hexval(hex[i + 1]);
        if (hi < 0 || lo < 0)
            return 0;
        out[i / 2] = (char)((hi << 4) | lo);
    }
    *len_out = hlen / 2;
    return 1;
}

// Encode a binary uid as ascii hex. 'out' must have room for len*2+1
// bytes; it is always NUL-terminated.
static void
uid_to_hex(const char *uid, int len, char *out)
{
    static const char hexd[] = "0123456789abcdef";
    int i;
    if (!out)
        return;
    for (i = 0; i < len; i++) {
        unsigned char c = (unsigned char)uid[i];
        out[i * 2] = hexd[c >> 4];
        out[i * 2 + 1] = hexd[c & 0xf];
    }
    out[len * 2] = 0;
}

// ===== peer / seed accounts =====
//
// g_peer_uid is the account under test: sends go here, and it is the uid
// that appears on received messages.
//
// g_seed_uid is a third, long-running account. It exists because several
// tests need a pal AND a non-pal at the same time -- dwyco_set_pals_only
// filtering is only provable if something gets rejected -- or a group
// member that is not the peer.
//
// Both are optional; tests that need one skip when it is absent.

static char g_peer_uid[64];
static int g_peer_uid_len;
static int g_has_peer;
static char g_seed_uid[64];
static int g_seed_uid_len;
static int g_has_seed;

// Decode an argv-supplied uid into one of the slots above. Prints a
// diagnostic and returns 0 if the string is malformed, so callers can
// just bail out of main().
static int
test_uid_arg(const char *hex, const char *what,
    char *out, int out_size, int *len_out)
{
    if (uid_from_hex(hex, out, out_size, len_out))
        return 1;
    fprintf(stderr, "%s must be an even-length hex string"
        " (20 chars == a 10 byte uid), got: %s\n",
        what, hex ? hex : "(null)");
    return 0;
}

// ===== channel service loop =====
//
// dwyco_service_channels is the single call every test binary has to make
// to pump network events, login, sends and fetches. It returns how long
// it thinks the caller should sleep before calling again, in ms. The
// "clamp that to 1..50ms" rule used to be copy-pasted into five polling
// loops across three files; it lives in service_step() now.
//
// Note the elapsed accounting below counts the *requested* sleep, not
// real wall clock. That is what the original loops did, so timeout
// behavior is unchanged, but it means a timeout can overrun slightly if
// service calls themselves are slow.

static void
service_once(void)
{
    int spin;
    dwyco_service_channels(&spin);
}

static void
service_step(int *elapsed_ms)
{
    int spin;
    int next = dwyco_service_channels(&spin);
    if (next <= 0 || next > 50)
        next = 50;
    usleep(next * 1000);
    if (elapsed_ms)
        *elapsed_ms += next;
}

// Pump the service loop for a fixed budget. Returns ms consumed.
static int
service_ms(int budget_ms)
{
    int elapsed = 0;
    while (elapsed < budget_ms)
        service_step(&elapsed);
    return elapsed;
}

// Pump the service loop until pred() returns true or timeout_ms elapses.
// pred() is checked before the first service pass, so an already-satisfied
// predicate costs zero service iterations and a zero timeout is a single
// non-blocking check. Returns 1 if pred() was satisfied, 0 on timeout.
static int
wait_for(const std::function<bool()> &pred, int timeout_ms)
{
    int elapsed = 0;
    for (;;) {
        if (pred())
            return 1;
        if (elapsed >= timeout_ms)
            return 0;
        service_step(&elapsed);
    }
}

// Same, but takes a plain function pointer for callers that would rather
// not build a std::function.
static int
wait_for_fn(bool (*pred)(void *), void *arg, int timeout_ms)
{
    if (!pred)
        return 0;
    return wait_for([=]() { return pred(arg) != 0; }, timeout_ms);
}

// ===== subprocess driver =====

// fork/exec argv, wait, and return the child's exit status (128+signal if
// it died on one, -1 if fork/exec/waitpid failed).
//
// This exists for the handful of APIs that can terminate the calling
// process from the inside: dwyco_update_server_list and
// dwyco_restore_from_backup both call exit() if the server list changed
// or a restore completed. Those can only be tested out-of-process, and
// what such a test asserts is the child's exit status, not in-process
// state. The child inherits stderr so library panics stay visible.
static int
run_subprocess(char *const argv[])
{
    pid_t pid;
    int status;

    if (!argv || !argv[0]) {
        fprintf(stderr, "run_subprocess: no argv\n");
        return -1;
    }
    pid = fork();
    if (pid < 0) {
        perror("run_subprocess: fork");
        return -1;
    }
    if (pid == 0) {
        execvp(argv[0], argv);
        perror("run_subprocess: execvp");
        _exit(127);
    }
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            perror("run_subprocess: waitpid");
            return -1;
        }
    }
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);
    return -1;
}

// ===== argv / environment =====

// Return argv[idx] if present, else the value of 'envname', else NULL.
// The env fallback is what lets ctest drive these binaries (see
// CMakeLists.txt) without the uids having to be known at configure time.
static const char *
arg_or_env(int argc, char **argv, int idx, const char *envname)
{
    const char *v;
    if (argc > idx && argv[idx] && *argv[idx])
        return argv[idx];
    v = envname ? getenv(envname) : NULL;
    return (v && *v) ? v : NULL;
}

// ===== system event names =====

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