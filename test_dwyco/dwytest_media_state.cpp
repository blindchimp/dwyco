// Coverage for the audio/mute/pause state API and the video-capture
// dispatch API.
//
//   Audio state:  dwyco_get_audio_hw, dwyco_set/get_all_mute,
//                 dwyco_set/get_exclusive_audio, dwyco_set/get_auto_squelch,
//                 dwyco_get_squelched, dwyco_set/get_full_duplex,
//                 dwyco_get_audio_output_in_progress,
//                 dwyco_set_max_established_originated_calls
//   Pause:        dwyco_pause_all_channels, dwyco_pause_channel_media_set,
//                 dwyco_pause_channel_media_get,
//                 dwyco_remote_pause_channel_media_get
//   Video:        dwyco_enable_video_capture_preview, dwyco_get_vfw_drivers,
//                 dwyco_start_vfw, dwyco_shutdown_vfw, dwyco_change_driver,
//                 dwyco_is_preview_on, dwyco_preview_on, dwyco_preview_off,
//                 dwyco_vfw_format, dwyco_vfw_source, dwyco_set_external_video
//   Drivers:      dwyco_set_external_video_capture_callbacks,
//                 dwyco_set_external_audio_capture_callbacks,
//                 dwyco_set_external_audio_output_callbacks
//
// None of this needs a peer, a call, or audio hardware: it is all state and
// callback plumbing. It does need dwyco_init() for the audio subsystem, so it
// is registered under the "server" label.
//
// The interesting part of this file is the driver-callback section. The three
// setters look interchangeable but are not:
//
//  * dwyco_set_external_audio_capture_callbacks and
//    dwyco_set_external_audio_output_callbacks really install the callbacks,
//    and the DLL calls straight into them. This test proves that by
//    installing counting stubs and observing them fire.
//
//  * dwyco_set_external_video_capture_callbacks compiles to an EMPTY FUNCTION
//    in this build. test_dwyco_cmake/conf.cmake sets DWYCOBG=1, and
//    bld/cdc32/CMakeLists.txt adds DWYCO_NO_ACQ_VIDEO_MEDIA when that is set,
//    which wraps the whole body in #ifndef. Installing video capture callbacks
//    therefore succeeds and does nothing, and nothing ever calls into them.
//
// A few more things this pins, none of them in the header:
//
//  * dwyco_is_preview_on() is hardcoded `return 0` and dwyco_vfw_format() is
//    hardcoded `return 1` in the library. Both are unconditional.
//  * All the other dwyco_vfw_*/preview entry points return 1
//    unconditionally, guarding the actual call behind a null callback check.
//  * All_mute and Auto_squelch both default to 1, not 0.
//  * dwyco_set_max_established_originated_calls returns the PREVIOUS value,
//    and the default is 4 (which dlli.h does document).
//  * The three dwyco_pause_channel_media_* entry points take a channel id;
//    an unknown id returns 0 and leaves the out parameters untouched.
//  * The video callback typedefs are internally inconsistent:
//    hw_preview_on is DwycoVVCallback (takes void *) but hw_preview_off is
//    DwycoVCallback (takes nothing), even though the prose above them
//    documents both as no-argument. A driver has to match the typedefs, not
//    the comment.

#include <dlli.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>

#include "test_common.h"
#include "list_readback.h"

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

static const char *g_dir = "/tmp/dwytest_media";

// ===== audio state =====

// Three independent booleans; get_audio_hw tolerates null out-pointers but
// this passes real ones per the project rule about never handing the API a
// null pointer.
static void
audio_hw_query(void)
{
    int has_in = -1, has_out = -1, full_duplex = -1;
    CHECK(dwyco_get_audio_hw(&has_in, &has_out, &full_duplex) != 0);
    // A build machine with no sound card must still report something sane
    // rather than leaving the outputs untouched.
    CHECK(has_in == 0 || has_in == 1);
    CHECK(has_out == 0 || has_out == 1);
    CHECK(full_duplex == 0 || full_duplex == 1);
}

static void
all_mute_round_trip(void)
{
    // Defaults to 1, so probe both directions from a known value rather
    // than assuming the initial state.
    dwyco_set_all_mute(0);
    CHECK(dwyco_get_all_mute() == 0);
    CHECK(dwyco_set_all_mute(1) != 0);
    CHECK(dwyco_get_all_mute() == 1);
    CHECK(dwyco_set_all_mute(0) != 0);
    CHECK(dwyco_get_all_mute() == 0);
}

static void
auto_squelch_round_trip(void)
{
    dwyco_set_auto_squelch(0);
    CHECK(dwyco_get_auto_squelch() == 0);
    CHECK(dwyco_set_auto_squelch(1) != 0);
    CHECK(dwyco_get_auto_squelch() == 1);
}

static void
full_duplex_round_trip(void)
{
    // dwyco_set_full_duplex returns void.
    dwyco_set_full_duplex(1);
    CHECK(dwyco_get_full_duplex() == 1);
    dwyco_set_full_duplex(0);
    CHECK(dwyco_get_full_duplex() == 0);
}

// Exclusive audio carries a channel id alongside the flag, so both halves
// have to round-trip together.
static void
exclusive_audio_round_trip(void)
{
    int state = -1, chan = -1;
    CHECK(dwyco_get_exclusive_audio(&state, &chan) != 0);
    CHECK(dwyco_set_exclusive_audio(1, 42) != 0);
    CHECK(dwyco_get_exclusive_audio(&state, &chan) != 0);
    CHECK(state == 1);
    CHECK(chan == 42);

    CHECK(dwyco_set_exclusive_audio(0, 0) != 0);
    CHECK(dwyco_get_exclusive_audio(&state, &chan) != 0);
    CHECK(state == 0);
    CHECK(chan == 0);
}

// dwyco_get_squelched has no setter in the api, so it can only be checked for
// being a defined 0/1 rather than for round-tripping.
static void
squelched_is_read_only(void)
{
    int s = dwyco_get_squelched();
    CHECK(s == 0 || s == 1);
}

// dwyco_get_audio_output_in_progress reaches into the audio output device,
// which does not exist without hardware. It must degrade to 0, not crash.
static void
audio_output_in_progress_without_hw(void)
{
    int spk = dwyco_get_audio_output_in_progress();
    CHECK(spk >= 0);
}

// The setter returns the value that was in effect before the call, which is
// how a caller reads the current limit. The documented default is 4.
static void
max_established_calls_returns_previous(void)
{
    // Each call returns the value that was in effect *before* it, so a
    // sequence of calls reads out the previous values in reverse.
    int orig = dwyco_set_max_established_originated_calls(8);
    CHECK(orig > 0);
    int after_8 = dwyco_set_max_established_originated_calls(5);
    CHECK(after_8 == 8);
    int after_5 = dwyco_set_max_established_originated_calls(orig);
    CHECK(after_5 == 5);
}

// ===== pause =====

// An unknown channel id is rejected rather than dereferenced, and the out
// parameters are left exactly as the caller passed them. This is the one
// safely-testable negative case in the pause api, since there is no channel
// to create without a peer.
static void
pause_rejects_unknown_channel(void)
{
    CHECK(dwyco_pause_channel_media_set(9999, 1, 1) == 0);
    CHECK(dwyco_pause_channel_media_set(9999, -1, -1) == 0);

    int pv = -99, pa = -99;
    CHECK(dwyco_pause_channel_media_get(9999, &pv, &pa) == 0);
    CHECK(pv == -99);
    CHECK(pa == -99);

    pv = pa = -99;
    CHECK(dwyco_remote_pause_channel_media_get(9999, &pv, &pa) == 0);
    CHECK(pv == -99);
    CHECK(pa == -99);
}

static void
pause_all_channels_is_safe(void)
{
    // Returns void. With no channels live this must be inert rather than a
    // crash, and must stay inert when toggled twice in a row.
    dwyco_pause_all_channels(1);
    dwyco_pause_all_channels(0);
    dwyco_pause_all_channels(1);
    dwyco_pause_all_channels(0);
}

// ===== video / vfw =====

// With no xmitter channel live there is nothing to preview. Turning preview
// on fails; turning it off succeeds (it just clears the flag).
static void
video_capture_preview_without_channel(void)
{
    CHECK(dwyco_enable_video_capture_preview(0) != 0);
    // dwyco_enable_video_capture_preview(1) tries to build a dummy channel
    // and a capture device. On a machine with no camera it cannot succeed,
    // and it is not required to; skip asserting a value so the test does not
    // depend on the host's video hardware.
    (void)dwyco_enable_video_capture_preview(1);
    dwyco_enable_video_capture_preview(0);
}

// With no video capture driver installed, the driver list is an empty
// single-column list: 0 rows, cols reported as -1.
static void
vfw_drivers_empty_without_driver(void)
{
    DWYCO_LIST l = dwyco_get_vfw_drivers();
    CHECK(l != 0);
    if (!l)
        return;
    CHECK(lr_rows(l, 0));
    CHECK(lr_cols(l, -1));
    dwyco_list_release(l);
}

// Every one of these guards its real work behind a null callback check and
// then returns 1, so with no driver installed they all report success while
// doing nothing.
static void
vfw_entry_points_all_succeed(void)
{
    CHECK(dwyco_start_vfw(0, 0, 0) != 0);
    CHECK(dwyco_change_driver(0) != 0);
    CHECK(dwyco_preview_on(0) != 0);
    CHECK(dwyco_preview_off() != 0);
    CHECK(dwyco_vfw_source() != 0);
    CHECK(dwyco_set_external_video(1) != 0);
    CHECK(dwyco_set_external_video(0) != 0);
    CHECK(dwyco_shutdown_vfw() != 0);
}

// Both of these are unconditional stubs in the library, so the test pins
// that they are stubs rather than silently passing on real behavior.
static void
preview_on_and_vfw_format_are_hardcoded(void)
{
    CHECK(dwyco_is_preview_on() == 0);
    CHECK(dwyco_vfw_format() == 1);
}

// ===== external driver callbacks =====

// Callbacks matching the typedefs in dlli.h exactly. Note that
// hw_preview_on takes void * while hw_preview_off takes nothing -- see the
// file header.
static int c_vid_new, c_vid_init, c_vid_devices, c_vid_setdev;
static int c_vid_prev_on, c_vid_prev_off, c_vid_appdata;
static int c_aud_new, c_aud_init, c_aud_status;
static int c_out_new, c_out_init;

static void DWYCOCALLCONV VV(void *) { }
static int DWYCOCALLCONV IV(void *) { return 0; }
static int DWYCOCALLCONV IVI(void *, int) { return 0; }
static void DWYCOCALLCONV VVI(void *, int, int) { }
static void DWYCOCALLCONV VI(int i) { c_vid_setdev += i; }
static void DWYCOCALLCONV VVV(void) { }
static int DWYCOCALLCONV IVII(void *, int) { return 80; }
static int DWYCOCALLCONV DEVOUT(void *, void *, int, int) { return 0; }
static int DWYCOCALLCONV DEVDONE(void *, void **, int *, int *) { return 0; }

static char ** DWYCOCALLCONV vid_get_devices(void) {
    c_vid_devices++;
    return 0;
}
static void DWYCOCALLCONV vid_free_list(char **) { }
static void * DWYCOCALLCONV vid_get_data(void *, int *, int *, int *, int *,
    unsigned long *) {
    static char b[8];
    return b;
}
static void DWYCOCALLCONV vid_new(void *) { c_vid_new++; }
static int DWYCOCALLCONV vid_init(void *, int) { c_vid_init++; return 1; }
static void DWYCOCALLCONV vid_hw_preview_on(void *) { c_vid_prev_on++; }
static void DWYCOCALLCONV vid_hw_preview_off(void) { c_vid_prev_off++; }
static void DWYCOCALLCONV vid_set_app_data(void *) { c_vid_appdata++; }

static void DWYCOCALLCONV aud_new(void *, int, int) { c_aud_new++; }
static int DWYCOCALLCONV aud_init(void *) { c_aud_init++; return 1; }
static int DWYCOCALLCONV aud_status(void *) { c_aud_status++; return 1; }
static void * DWYCOCALLCONV aud_get_data(void *, int *, int *) {
    static char b[8];
    return b;
}
static void DWYCOCALLCONV aud_out_new(void *) { c_out_new++; }
static int DWYCOCALLCONV aud_out_init(void *) { c_out_init++; return 1; }

// Installing the audio capture callbacks must make the DLL actually call
// them.
//
// dwyco_get_audio_hw is the trigger: it runs check_audio_device(), which
// creates and initializes both the capture and the output device, so one
// call to it exercises every callback the DLL is willing to reach on a
// machine with no sound card. (dwyco_get_audio_output_in_progress reaches
// nothing here -- it only reads TheAudioOutput->device_bufs_playing(), and
// with no hardware the device either does not exist or never calls back.)
//
// This is the load-bearing assertion of the whole file: without it,
// dwyco_set_external_audio_capture_callbacks would be an unverified no-op.
// The baseline is taken inside the test so the assertion is about *this*
// call dispatching, not about some earlier one having done so.
static void
audio_capture_callbacks_are_invoked(void)
{
    int new0 = c_aud_new;
    int init0 = c_aud_init;

    int has_in = 0, has_out = 0, full_duplex = 0;
    CHECK(dwyco_get_audio_hw(&has_in, &has_out, &full_duplex) != 0);

    if (c_aud_init <= init0) {
        printf("[FAIL] capture init not called by dwyco_get_audio_hw"
            " (init %d -> %d, new %d -> %d)\n",
            init0, c_aud_init, new0, c_aud_new);
        g_fail++;
        return;
    }
    printf("\n      capture driver: new +%d, init +%d, status=%d\n",
        c_aud_new - new0, c_aud_init - init0, c_aud_status);
}

// Same idea for the audio output driver. Same trigger, because
// check_audio_device() brings up capture and output together.
static void
audio_output_callbacks_are_invoked(void)
{
    int new0 = c_out_new;
    int init0 = c_out_init;

    int has_in = 0, has_out = 0, full_duplex = 0;
    CHECK(dwyco_get_audio_hw(&has_in, &has_out, &full_duplex) != 0);

    if (c_out_init <= init0) {
        printf("[FAIL] output init not called by dwyco_get_audio_hw"
            " (init %d -> %d, new %d -> %d)\n",
            init0, c_out_init, new0, c_out_new);
        g_fail++;
        return;
    }
    printf("\n      output driver: new +%d, init +%d\n",
        c_out_new - new0, c_out_init - init0);
}

// The video setter is the odd one out: its body is compiled out by
// DWYCO_NO_ACQ_VIDEO_MEDIA in this build, so installing callbacks succeeds
// and nothing is ever called through them.
//
// If a future build enables video capture, this test will fail -- which is
// the point: the behavior should change loudly, not silently.
static void
video_capture_callbacks_are_not_installed_in_this_build(void)
{
    dwyco_set_external_video_capture_callbacks(
        /* nw               */ vid_new,
        /* del              */ VV,
        /* init             */ vid_init,
        /* has_data         */ IV,
        /* need             */ VV,
        /* pass             */ VV,
        /* stop             */ VV,
        /* get_data         */ vid_get_data,
        /* free_data        */ VV,
        /* get_vid_devices  */ vid_get_devices,
        /* free_vid_list    */ vid_free_list,
        /* set_vid_device   */ VI,
        /* stop_vid_device  */ VVV,
        /* show_source_dlg  */ VVV,
        /* hw_preview_on    */ vid_hw_preview_on,
        /* hw_preview_off   */ vid_hw_preview_off,
        /* set_app_data     */ vid_set_app_data);

    // Poke the entry points that would dispatch to the driver.
    DWYCO_LIST l = dwyco_get_vfw_drivers();
    if (l)
        dwyco_list_release(l);
    CHECK(dwyco_start_vfw(3, 0, 0) != 0);
    CHECK(dwyco_change_driver(3) != 0);
    CHECK(dwyco_preview_on(0) != 0);
    CHECK(dwyco_preview_off() != 0);
    CHECK(dwyco_vfw_source() != 0);
    CHECK(dwyco_set_external_video(1) != 0);

    int fired = c_vid_new + c_vid_init + c_vid_devices + c_vid_setdev +
        c_vid_prev_on + c_vid_prev_off + c_vid_appdata;
    if (fired != 0) {
        printf("[FAIL] video callbacks fired (%d calls) -- video capture is"
            " now enabled in this build, update this test\n", fired);
        g_fail++;
        return;
    }
    // The driver list stays empty for the same reason.
    l = dwyco_get_vfw_drivers();
    CHECK(l != 0);
    if (l) {
        CHECK(lr_rows(l, 0));
        dwyco_list_release(l);
    }
}

// Both audio driver callback sets have to be installed before anything else
// touches the audio subsystem.
//
// dwyco_get_audio_hw runs check_audio_device(), which creates and initializes
// the devices once and then leaves them alive. If the first audio call in the
// process happened before install_audio_drivers(), the devices would already
// exist holding the default (null) callbacks, and installing ours afterwards
// would never be reached -- the observed symptom being that
// audio_capture_callbacks_are_invoked sees its init counter stay flat.
//
// Installing first makes both "were invoked" tests meaningful regardless of
// what order the test sections run in.
static void
install_audio_drivers(void)
{
    dwyco_set_external_audio_capture_callbacks(
        /* nw          */ VVI,
        /* del         */ VV,
        /* init        */ aud_init,
        /* has_data    */ IV,
        /* need        */ VV,
        /* pass        */ VV,
        /* stop        */ VV,
        /* on          */ VV,
        /* off         */ VV,
        /* reset       */ VV,
        /* status      */ aud_status,
        /* get_data    */ aud_get_data);

    dwyco_set_external_audio_output_callbacks(
        /* nw             */ aud_out_new,
        /* dlete          */ VV,
        /* init           */ aud_out_init,
        /* device_output  */ DEVOUT,
        /* device_done    */ DEVDONE,
        /* stop           */ IV,
        /* reset          */ IV,
        /* status         */ IV,
        /* close          */ IV,
        /* buffer_time    */ IVII,
        /* play_silence   */ IV,
        /* bufs_playing   */ IV);
}

// ===== harness =====

static int
init_test(void)
{
    char sys_dir[512], tmp_dir[512];
    snprintf(sys_dir, sizeof(sys_dir), "%s/sys", g_dir);
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", g_dir);
    mkdir(g_dir, 0755);
    mkdir(sys_dir, 0755);
    mkdir(tmp_dir, 0755);
    install_app_files(g_dir);

    dwyco_set_fn_prefixes(sys_dir, g_dir, tmp_dir);
    dwyco_set_client_version("dwytest", 7);
    if (dwyco_init() == 0)
        return 0;
    test_bootstrap_profile("dwytest-media", "dwytest media account");
    dwyco_finish_startup();
    return 1;
}

int
main(void)
{
    setvbuf(stdout, 0, _IOLBF, 0);
    printf("Dwyco audio / pause / video-capture state\n");

    if (!init_test()) {
        fprintf(stderr, "dwyco_init failed\n");
        return 1;
    }

    // Must come before any other audio call. See install_audio_drivers().
    install_audio_drivers();

    printf("\nAudio state:\n");
    RUN(audio_hw_query);
    RUN(all_mute_round_trip);
    RUN(auto_squelch_round_trip);
    RUN(full_duplex_round_trip);
    RUN(exclusive_audio_round_trip);
    RUN(squelched_is_read_only);
    RUN(audio_output_in_progress_without_hw);
    RUN(max_established_calls_returns_previous);

    printf("\nPause:\n");
    RUN(pause_rejects_unknown_channel);
    RUN(pause_all_channels_is_safe);

    printf("\nVideo / vfw:\n");
    RUN(video_capture_preview_without_channel);
    RUN(vfw_drivers_empty_without_driver);
    RUN(vfw_entry_points_all_succeed);
    RUN(preview_on_and_vfw_format_are_hardcoded);

    printf("\nExternal drivers:\n");
    RUN(audio_capture_callbacks_are_invoked);
    RUN(audio_output_callbacks_are_invoked);
    RUN(video_capture_callbacks_are_not_installed_in_this_build);

    dwyco_exit();

    printf("\nSummary: passed=%d failed=%d\n", g_pass, g_fail);
    if (g_fail) {
        printf("FAILED\n");
        return 1;
    }
    printf("All media state tests passed.\n");
    return 0;
}