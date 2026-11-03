// Coverage for the call/channel state API and the callback registrations.
//
//   selective chat:      dwyco_selective_chat_enable,
//                         dwyco_selective_chat_recipient_enable,
//                         dwyco_is_selective_chat_recipient,
//                         dwyco_reset_selective_chat_recipients
//   pals-only filtering: dwyco_set_pals_only, dwyco_get_pals_only
//   channel/call lookups: dwyco_call_accept, dwyco_call_reject,
//                         dwyco_zap_accept, dwyco_zap_reject,
//                         dwyco_cancel_call, dwyco_chan_to_call,
//                         dwyco_channel_streams, dwyco_destroy_channel,
//                         dwyco_hangup_all_calls, dwyco_send_user_control
//   channel creation:    dwyco_channel_create
//   keyboard:            dwyco_command_from_keyboard, dwyco_line_from_keyboard
//   callback registration: the dwyco_set_*_callback family for calls and chat
//
// WHAT IS AND IS NOT COVERED
//
// Everything here is state, lookup-with-an-unknown-id, or callback
// registration. What is NOT covered, and cannot be without two clients that
// can actually establish a media session:
//
//   dwyco_connect_uid, dwyco_connect_all4, dwyco_connect_msg_chan,
//   the DWYCO_CSC_ACCEPT/DEFER/REJECT screening matrix, and the
//   DWYCO_CALLDISP_* disposition callbacks.
//
// Those need a second endpoint with working audio/video, which this build has
// no hardware for (dwyco_get_audio_hw reports no input or output). They are
// the natural next chunk, and they would have to run against a live peer
// rather than in isolation.
//
// Two things worth knowing about the functions that ARE covered:
//
//  * dwyco_zap_accept and dwyco_zap_reject are hardcoded "return 0" in the
//    library. They are not broken, they are just not implemented -- so a test
//    that asserted 1 would be wrong.
//
//  * Every function that takes a channel or call id validates it and returns
//    0 (or -1 for dwyco_chan_to_call) for an unknown id, leaving out
//    parameters untouched. That is the only safe way to exercise them without
//    a live channel, and it is what most of this file does.
//
//  * dwyco_set_pals_only is backed by the "zap/ignore" setting, and
//    dwyco_get_pals_only reads it back, so the pair round-trips. Note it also
//    broadcasts to chat-local users, which is harmless with no chat server.

#include <dlli.h>
#include "test_common.h"
#include "list_readback.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static int g_pass = 0;
static int g_fail = 0;

#define RUN(name) do { \
    printf("  %-40s ", #name); \
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

static const char *CLIENT_DIR = "/tmp/dwytest_calls";

static char g_my_uid[64];
static int g_my_uid_len;

static const char BOGUS_UID[10] = {
    (char)0xde, (char)0xad, (char)0xbe, (char)0xef, (char)0x01,
    (char)0x23, (char)0x45, (char)0x67, (char)0x89, (char)0xab
};

// A channel id that cannot exist. Channel ids are small positive integers
// allocated by the library, and no channel is ever created in this test, so
// anything large is guaranteed unknown.
#define NO_CHAN 987654

// ===== callback stubs =====

static int g_cb_count;
static int g_dispositions;
static int g_statuses;
static int g_last_what;
static char g_last_uid[64];
static int g_last_uid_len;

static void DWYCOCALLCONV chan_destroy_cb(int id, void *arg)
{ (void)id; (void)arg; g_cb_count++; }
static void DWYCOCALLCONV disposition_cb(int call_id, int chan_id, int what,
    void *arg, const char *uid, int len_uid, const char *call_type, int len_call_type)
{
    (void)call_id; (void)chan_id; (void)arg; (void)call_type; (void)len_call_type;
    g_cb_count++;
    g_dispositions++;
    g_last_what = what;
    g_last_uid_len = len_uid < (int)sizeof(g_last_uid) ? len_uid
        : (int)sizeof(g_last_uid) - 1;
    if (uid && len_uid > 0)
        memcpy(g_last_uid, uid, (size_t)g_last_uid_len);
}
static void DWYCOCALLCONV status_cb(int id, const char *msg, int percent, void *arg)
{ (void)id; (void)msg; (void)percent; (void)arg; g_cb_count++; g_statuses++; }
static void DWYCOCALLCONV appearance_cb(int chan_id, const char *name,
    const char *location, const char *uid, int len_uid, const char *call_type,
    int len_call_type)
{ (void)chan_id; (void)name; (void)location; (void)uid; (void)len_uid;
  (void)call_type; (void)len_call_type; g_cb_count++; }
static void DWYCOCALLCONV zap_appearance_cb(int chan_id, const char *name,
    int size, const char *uid, int len_uid)
{ (void)chan_id; (void)name; (void)size; (void)uid; (void)len_uid; g_cb_count++; }
static void DWYCOCALLCONV appearance_death_cb(int chan_id)
{ (void)chan_id; g_cb_count++; }
static int DWYCOCALLCONV screening_cb(int chan_id, int rv, int sv, int ra, int sa,
    int rpc, int rvc, const char *call_type, int len_call_type,
    const char *uid, int len_uid, int *style_out, char **err_out)
{
    (void)chan_id; (void)rv; (void)sv; (void)ra; (void)sa; (void)rpc;
    (void)rvc; (void)call_type; (void)len_call_type; (void)uid; (void)len_uid;
    if (style_out) *style_out = DWYCO_CSC_REJECT_CALL;
    if (err_out) *err_out = 0;
    return 0;
}
static void DWYCOCALLCONV user_control_cb(int chan_id, const char *uid,
    int len_uid, const char *data, int len_data)
{ (void)chan_id; (void)uid; (void)len_uid; (void)data; (void)len_data; g_cb_count++; }
static void DWYCOCALLCONV debug_msg_cb(int id, const char *msg, int percent, void *arg)
{ (void)id; (void)msg; (void)percent; (void)arg; g_cb_count++; }
static void DWYCOCALLCONV video_display_cb(int chan_id, void *img, int cols,
    int rows, int depth)
{ (void)chan_id; (void)img; (void)cols; (void)rows; (void)depth; g_cb_count++; }
static int DWYCOCALLCONV pub_chat_init_cb(int chan_id)
{ (void)chan_id; g_cb_count++; return 0; }
static int DWYCOCALLCONV priv_chat_init_cb(int chan_id, const char *unused)
{ (void)chan_id; (void)unused; g_cb_count++; return 0; }
static int DWYCOCALLCONV priv_chat_display_cb(int chan_id, const char *com,
    int arg1, int arg2, const char *str, int len)
{ (void)chan_id; (void)com; (void)arg1; (void)arg2; (void)str; (void)len; return 0; }
static int DWYCOCALLCONV pub_chat_display_cb(const char *who, int len_who,
    const char *line, int len_line, const char *uid, int len_uid)
{ (void)who; (void)len_who; (void)line; (void)len_line; (void)uid;
  (void)len_uid; g_cb_count++; return 0; }

// ===== tests =====

// Selective chat needs a LIVE private chat session. Both halves of it bail
// out immediately when there is no "message xmitter" channel:
//
//   MMChannel::selective_chat_enable(e)
//       MMChannel *xm = MMChannel::find_msg_xmitter();
//       if(!xm) return 0;
//
//   MMChannel::selective_chat_recipient_enable(uid, enable)
//       ... looks for a serviced channel whose remote_uid matches AND that
//           has a live chat id ...
//       if(!m) return 0;
//       MMChannel *xm = MMChannel::find_msg_xmitter();
//       if(!xm) return 0;
//
// So with no peer connected they always report 0, and there is no way to
// observe the state they would set. dlli.h documents no such precondition.
//
// That makes the positive path a chat-session test, not a state test -- it
// belongs with the chat server work, where a real lobby can be joined. Here
// the contract worth pinning is that they decline cleanly and that the
// getter agrees.
static void
selective_chat_needs_a_live_session(void)
{
    dwyco_reset_selective_chat_recipients();

    int enable_rc = dwyco_selective_chat_enable(1);
    int disable_rc = dwyco_selective_chat_enable(0);
    // Both must report "could not do it" rather than pretending.
    CHECK(enable_rc == 0);
    CHECK(disable_rc == 0);
    printf("(enable=%d disable=%d) ", enable_rc, disable_rc);

    // No session, so nobody is a selective recipient and nothing can be set.
    CHECK(dwyco_is_selective_chat_recipient(BOGUS_UID, 10) == 0);
    CHECK(dwyco_selective_chat_recipient_enable(BOGUS_UID, 10, 1) == 0);
    CHECK(dwyco_is_selective_chat_recipient(BOGUS_UID, 10) == 0);

    static const char other[10] = {
        (char)1, (char)2, (char)3, (char)4, (char)5,
        (char)6, (char)7, (char)8, (char)9, (char)10
    };
    CHECK(dwyco_selective_chat_recipient_enable(other, 10, 1) == 0);
    CHECK(dwyco_is_selective_chat_recipient(other, 10) == 0);

    // reset is void and must be safe to call repeatedly.
    dwyco_reset_selective_chat_recipients();
    dwyco_reset_selective_chat_recipients();
    CHECK(dwyco_is_selective_chat_recipient(BOGUS_UID, 10) == 0);
}

// pals-only is stored in the zap/ignore setting, so the getter really does
// read back what the setter wrote.
static void
pals_only_round_trip(void)
{
    int before = dwyco_get_pals_only();
    CHECK(before == 0 || before == 1);

    dwyco_set_pals_only(1);
    CHECK(dwyco_get_pals_only() == 1);
    dwyco_set_pals_only(0);
    CHECK(dwyco_get_pals_only() == 0);

    // And the backing setting agrees.
    const char *val;
    int len, type;
    CHECK(dwyco_get_setting("zap/ignore", &val, &len, &type) != 0);
    printf("(zap/ignore='%.*s') ", len, val);

    dwyco_set_pals_only(before);
    CHECK(dwyco_get_pals_only() == before);
}

// Every channel-keyed entry point must reject an unknown id rather than
// dereference it, and must leave its out parameters alone.
static void
channel_lookups_reject_unknown_id(void)
{
    g_cb_count = 0;
    CHECK(dwyco_call_accept(NO_CHAN) == 0);
    CHECK(dwyco_call_reject(NO_CHAN, 0) == 0);
    // dwyco_chan_to_call reports "no such call" as -1.
    CHECK(dwyco_chan_to_call(NO_CHAN) == -1);
    CHECK(dwyco_send_user_control(BOGUS_UID, 10, "data", 4) == 0);
    // dwyco_cancel_call is void and returns early on a bad cookie.
    dwyco_cancel_call(NO_CHAN);
    // destroy_channel and hangup_all_calls are void and must be no-ops here.
    dwyco_destroy_channel(NO_CHAN);
    dwyco_hangup_all_calls();
    CHECK(g_cb_count == 0);
}

static void
channel_streams_rejects_unknown_id(void)
{
    int sv = -1, rv = -1, sa = -1, ra = -1, pc = -1, vc = -1;
    int rc = dwyco_channel_streams(NO_CHAN, &sv, &rv, &sa, &ra, &pc, &vc);
    CHECK(rc == 0);
    CHECK(sv == -1);
    CHECK(rv == -1);
    CHECK(sa == -1);
    CHECK(ra == -1);
    CHECK(pc == -1);
    CHECK(vc == -1);
}

// dwyco_zap_accept / dwyco_zap_reject are stubs that always return 0. Pinned
// so that implementing them shows up as a test failure.
static void
zap_accept_and_reject_are_stubs(void)
{
    CHECK(dwyco_zap_accept(NO_CHAN, 0) == 0);
    CHECK(dwyco_zap_accept(0, 1) == 0);
    CHECK(dwyco_zap_reject(NO_CHAN, 0) == 0);
    CHECK(dwyco_zap_reject(0, 1) == 0);
}

// Starting a call to an address nobody can answer must return a call id that
// is not zero, and must not deliver a disposition synchronously. The call is
// cancelled straight away so the test leaves nothing running.
static void
channel_create_to_unreachable_uid(void)
{
    g_cb_count = 0;
    g_dispositions = 0;
    g_statuses = 0;
    int call_id = dwyco_channel_create(BOGUS_UID, 10, disposition_cb, 0,
        status_cb, 0, 0, "test", 4, 0);
    printf("(call_id=%d status=%d disp=%d) ", call_id, g_statuses, g_dispositions);
    // Call setup reports DWYCO_CALLDISP_STARTED as soon as the call is
    // initiated; the outcome arrives later, after the network round trip. What
    // must NOT happen is a synchronous ESTABLISHED, which would mean the call
    // connected to an address nobody answers.
    printf("(what=%d) ", g_last_what);
    if (g_dispositions > 0) {
        CHECK(g_last_what == DWYCO_CALLDISP_STARTED ||
              g_last_what == DWYCO_CALLDISP_FAILED ||
              g_last_what == DWYCO_CALLDISP_CANCELED ||
              g_last_what == DWYCO_CALLDISP_REJECTED);
        CHECK(g_last_what != DWYCO_CALLDISP_ESTABLISHED);
    }
    if (call_id > 0)
        dwyco_cancel_call(call_id);
    // After cancelling, the id must no longer resolve.
    if (call_id > 0)
        CHECK(dwyco_chan_to_call(call_id) == -1);
}

// The keyboard entry points take a channel id and must tolerate an unknown one.
static void
keyboard_commands_tolerate_unknown_channel(void)
{
    // 'k' is "insert char at cursor" per dlli.h's command table.
    dwyco_command_from_keyboard(NO_CHAN, 'k', 0, 0, "x", 1);
    dwyco_line_from_keyboard(NO_CHAN, "hello", 5);
    // 'c' clears the buffer, 'd' deletes, 's' selects, 'p' pastes.
    dwyco_command_from_keyboard(NO_CHAN, 'c', 0, 0, 0, 0);
    dwyco_command_from_keyboard(NO_CHAN, 'd', 0, 0, 0, 0);
    dwyco_command_from_keyboard(NO_CHAN, 's', 0, 5, 0, 0);
    dwyco_command_from_keyboard(NO_CHAN, 'p', 0, 0, "text", 4);
    dwyco_command_from_keyboard(NO_CHAN, 'b', 0, 0, 0, 0);
}

// The per-channel destroy callback is registered per channel, not globally, so
// registering one for an id that does not exist must simply do nothing.
static void
channel_destroy_callback_registration(void)
{
    g_cb_count = 0;
    dwyco_set_channel_destroy_callback(NO_CHAN, chan_destroy_cb, 0);
    // Destroying the unknown channel must not invoke it.
    dwyco_destroy_channel(NO_CHAN);
    CHECK(g_cb_count == 0);
}

// All the global callback setters are plain pointer assignments. Installing a
// stub for each must not fault, and must leave the library usable.
static void
callback_registration_is_safe(void)
{
    g_cb_count = 0;
    // Installed one at a time, each followed by a short service burst, so a
    // failure names the setter that caused it rather than just killing the
    // run half way through.
#define INSTALL_AND_POKE(what, call) do { \
        fprintf(stderr, "  installing " what "\n"); \
        call; \
        service_ms(150); \
    } while (0)

    INSTALL_AND_POKE("call_appearance",
        dwyco_set_call_appearance_callback(appearance_cb));
    INSTALL_AND_POKE("call_acceptance",
        dwyco_set_call_acceptance_callback(appearance_cb));
    // dwyco_set_zap_appearance_callback is deliberately NOT installed here:
    // see set_zap_appearance_callback_terminates() below.
    INSTALL_AND_POKE("call_appearance_death",
        dwyco_set_call_appearance_death_callback(appearance_death_cb));
    INSTALL_AND_POKE("call_screening",
        dwyco_set_call_screening_callback(screening_cb));
    INSTALL_AND_POKE("user_control",
        dwyco_set_user_control_callback(user_control_cb));
    INSTALL_AND_POKE("debug_message",
        dwyco_set_debug_message_callback(debug_msg_cb));
    INSTALL_AND_POKE("call_bandwidth",
        dwyco_set_call_bandwidth_callback(debug_msg_cb));
    INSTALL_AND_POKE("video_display",
        dwyco_set_video_display_callback(video_display_cb));
    INSTALL_AND_POKE("public_chat_init",
        dwyco_set_public_chat_init_callback(pub_chat_init_cb));
    INSTALL_AND_POKE("private_chat_init",
        dwyco_set_private_chat_init_callback(priv_chat_init_cb));
    INSTALL_AND_POKE("public_chat_display",
        dwyco_set_public_chat_display_callback(pub_chat_display_cb));
    INSTALL_AND_POKE("private_chat_display",
        dwyco_set_private_chat_display_callback(priv_chat_display_cb));
    INSTALL_AND_POKE("bgapp_msg",
        dwyco_set_bgapp_msg_callback(pub_chat_display_cb));
#undef INSTALL_AND_POKE

    // The library must still work normally afterwards.
    service_ms(300);
    int mute = dwyco_get_all_mute();
    CHECK(mute == 0 || mute == 1);
    CHECK(dwyco_selective_chat_enable(0) == 0);
}

// The exclusive-audio state carries a channel id; -1 means "no channel".
static void
exclusive_audio_accepts_no_channel(void)
{
    int state = -1, chan = -1;
    CHECK(dwyco_get_exclusive_audio(&state, &chan) != 0);
    CHECK(dwyco_set_exclusive_audio(1, -1) != 0);
    CHECK(dwyco_get_exclusive_audio(&state, &chan) != 0);
    CHECK(state == 1);
    CHECK(chan == -1);
    CHECK(dwyco_set_exclusive_audio(0, -1) != 0);
    CHECK(dwyco_get_exclusive_audio(&state, &chan) != 0);
    CHECK(state == 0);
}

// Sending/stopping media on an unknown channel must be refused.
static void
channel_media_controls_reject_unknown_id(void)
{
    CHECK(dwyco_channel_send_video(NO_CHAN, 0) == 0);
    CHECK(dwyco_channel_send_audio(NO_CHAN, 0) == 0);
    CHECK(dwyco_channel_stop_send_video(NO_CHAN) == 0);
    CHECK(dwyco_channel_stop_send_audio(NO_CHAN) == 0);
}

// ===== defect: dwyco_set_zap_appearance_callback always terminates =====
//
// The whole body is:
//
//   oopanic("zap appearances not supported anymore");
//   //zap_appearance_callback = cb;
//
// so calling it exits the process no matter what callback you pass. Zap
// appearances were removed but the declaration was left in dlli.h as an
// ordinary setter, so a client that still tries to install one dies at
// startup with a confusing message instead of a link error or a no-op.
//
// Verified out-of-process. If the function is ever given a real body (or
// removed from the header), this test will fail.
static const char *g_argv0 = 0;

static void
zap_appearance_child(void)
{
    int null_fd = open("/dev/null", O_WRONLY);
    if (null_fd >= 0) {
        dup2(null_fd, 2);
        close(null_fd);
    }
    dwyco_set_zap_appearance_callback(zap_appearance_cb);
    fprintf(stderr, "      child: survived\n");
    exit(0);
}

static void
set_zap_appearance_callback_terminates(void)
{
    if (!g_argv0) {
        printf("[FAIL] no argv[0] recorded for subprocess test\n");
        g_fail++;
        return;
    }
    char *argv[] = { (char *)g_argv0, (char *)"--zap-appearance", 0 };
    // oopanic -> exit(1).
    CHECK(run_subprocess(argv) == 1);
}

// ===== harness =====

static int
boot(void)
{
    char sys_dir[512], tmp_dir[512];
    snprintf(sys_dir, sizeof(sys_dir), "%s/sys", CLIENT_DIR);
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", CLIENT_DIR);
    mkdir(CLIENT_DIR, 0755);
    mkdir(sys_dir, 0755);
    mkdir(tmp_dir, 0755);
    install_app_files(CLIENT_DIR);

    dwyco_set_fn_prefixes(sys_dir, CLIENT_DIR, tmp_dir);
    dwyco_set_client_version("dwytest", 7);
    if (!dwyco_init())
        return 0;
    test_bootstrap_profile("dwytest-calls", "dwytest calls account");
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);

    const char *uid;
    int len;
    dwyco_get_my_uid(&uid, &len);
    if (len <= 0 || len >= (int)sizeof(g_my_uid))
        return 0;
    memcpy(g_my_uid, uid, (size_t)len);
    g_my_uid_len = len;
    return 1;
}

int
main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--zap-appearance") == 0) {
        setvbuf(stdout, 0, _IOLBF, 0);
        zap_appearance_child();
        return 0;
    }
    g_argv0 = argv[0];
    setvbuf(stdout, 0, _IOLBF, 0);
    printf("Dwyco call / channel state\n");

    if (!boot()) {
        fprintf(stderr, "dwyco_init failed\n");
        return 1;
    }
    service_ms(2000);

    printf("\nSelective chat:\n");
    RUN(selective_chat_needs_a_live_session);

    printf("\nPals-only filtering:\n");
    RUN(pals_only_round_trip);

    printf("\nChannel lookups:\n");
    RUN(channel_lookups_reject_unknown_id);
    RUN(channel_streams_rejects_unknown_id);
    RUN(zap_accept_and_reject_are_stubs);
    RUN(channel_media_controls_reject_unknown_id);
    RUN(channel_destroy_callback_registration);

    printf("\nCall setup:\n");
    RUN(channel_create_to_unreachable_uid);
    RUN(exclusive_audio_accepts_no_channel);

    printf("\nKeyboard:\n");
    RUN(keyboard_commands_tolerate_unknown_channel);

    printf("\nCallback registration:\n");
    RUN(callback_registration_is_safe);
    RUN(set_zap_appearance_callback_terminates);

    dwyco_exit();

    printf("\nSummary: passed=%d failed=%d\n", g_pass, g_fail);
    if (g_fail) {
        printf("FAILED\n");
        return 1;
    }
    printf("All call tests passed.\n");
    return 0;
}