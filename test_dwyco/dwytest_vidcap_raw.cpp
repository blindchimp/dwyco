// Can video capture be initialized from the raw-file (PPM) test input, and do
// the preview callbacks get the same frames the source produced?
//
// The raw files input is the one capture source that needs no camera, so it is
// the only way to exercise the capture path on a headless build machine.
//
// How capture actually gets initialized
// -------------------------------------
// There is no dwyco_start_video_capture(). Capture is initialized implicitly,
// on first use, by the acquisition-object machinery:
//
//   dwyco_enable_video_capture_preview(1)
//     -> builds a throwaway MMChannel + DummyTube
//     -> MMChannel::build_outgoing(1, 1, 12)          [mmbld.cc:422]
//     -> initaq(mbox, fail_reason)                     [aq.cc:120]
//          if video_input/source == "raw" -> init_raw_files()   [aq.cc:63]
//              -> FileAcquire<pixel>::init()           [acqfile.h:92]
//          else                          -> init_external_video() [aq.cc:92]
//
// So the return value of dwyco_enable_video_capture_preview(1) IS the answer to
// "did capture initialize": build_outgoing FAILRETs when initaq() fails, and
// preview() then returns 0. That is the load-bearing assertion here.
//
// What this test checks
// ---------------------
// Capture initializing at all (the preview() return value), and then that the
// frames arriving at dwyco_set_video_display_callback are the SAME frames the
// synthetic PPM source produced -- same pixels, same size, in list order.
//
// The frames can be compared bit-exactly because the preview path never touches
// the codec. Full chain with citations is on raw_video_display() in
// raw_frames.h.
//
// The frame source itself lives in raw_frames.h, shared with dwytest_zap.cpp.
//
// Settings that matter
// ---------------------
//   video_input/source        "raw"  -- picks the raw-file path over the camera
//   raw_files/raw_files_list        -- the list of PPM files (newline separated)
//   raw_files/use_pattern    0      -- use the list; 1 means treat the setting
//                                       above as a printf pattern instead
//   raw_files/preload        0/1    -- read every file up front (1) or lazily (0)
//   video_input/no_video     0      -- only consulted during call negotiation,
//                                       NOT by initaq, but set anyway since it
//                                       is the master "video off" switch
//
// Note raw_files/use_list_of_files is declared in ezset2.cpp but is dead:
// init_raw_files() only ever reads raw_files/use_pattern, which selects
// list-vs-pattern and defaults to 0 (i.e. "use the list") anyway. It is still
// set here so the settings match the documented test recipe, but changing it
// has no effect.
//
// What makes this path available at all
// ------------------------------------
// This only works because test_dwyco_cmake/conf.cmake sets DWYCO_TESTING=1,
// which takes the DWYCO_TESTING branch in bld/cdc32/CMakeLists.txt. That branch
// trims the same things DWYCOBG=1 does, EXCEPT it keeps three of them compiled
// in:
//
//   DWYCO_NO_THEORA_CODEC  a video codec is mandatory, not optional. With it
//                         defined, every branch of the if/else chain in
//                         MMChannel::coder_from_config() [mmchan.cc:1881]
//                         compiles away and it degrades to
//                         FAILRET("incompatible coding style"), so
//                         build_outgoing() fails at the coder step and
//                         preview() returns 0 even though initaq()
//                         succeeded. Capture cannot initialize at all without
//                         this, so it is not just about frames.
//   DWYCO_NO_VIDEO_FROM_PPM  init_raw_files()/FileAcquire<>/readfile().
//   DWYCO_NO_VIDEO_MSGS      dwyco_zap_create_preview(), the PNG-writing zap
//                            preview api.
//
// Still stripped, deliberately: DWYCO_NO_ACQ_VIDEO_MEDIA (the external camera
// driver path). That one only affects init_external_video(), i.e. the *other*
// capture source, and dwytest_media_state.cpp asserts those driver callbacks
// are compiled out -- so leaving it off keeps that test honest.
//
// If this test ever fails with preview() returning 0 again, check those three
// defines before suspecting the capture code: the distinction is whether
// fail_reason is left at its aq.cc:122 initial value ("unknown video acq init
// failure", meaning it never reached the raw-file branch) or set to a real
// reason like "can't open list of files".

#include <dlli.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>

#include "test_common.h"
#include "raw_frames.h"

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

static const char *g_dir = "/tmp/dwytest_vidcap";
static const char *g_file_list;

static void
configure_raw_input(void)
{
    // Point at the file list. The "raw" source is what makes initaq() take the
    // FileAcquire branch at all.
    CHECK(dwyco_set_setting("video_input/source", "raw") != 0);
    CHECK(dwyco_set_setting("raw_files/raw_files_list", g_file_list) != 0);

    // 0 == use raw_files_list. init_raw_files() reads use_pattern and ignores
    // use_list_of_files; see the file header.
    CHECK(dwyco_set_setting("raw_files/use_pattern", "0") != 0);
    CHECK(dwyco_set_setting("raw_files/use_list_of_files", "1") != 0);

    // Lazy loading keeps init cheap; the frame timer pulls each file in as it
    // cycles.
    CHECK(dwyco_set_setting("raw_files/preload", "0") != 0);

    // Not consulted by initaq, but it is the master video switch during call
    // negotiation, so pin it to "video allowed" for completeness.
    CHECK(dwyco_set_setting("video_input/no_video", "0") != 0);
}

// Read the settings back. A silent set failure would make the capture attempt
// below fail for the wrong reason, so confirm the round trip first.
static void
raw_input_settings_read_back(void)
{
    const char *v = 0;
    int len = 0, type = 0;

    CHECK(dwyco_get_setting("video_input/source", &v, &len, &type) != 0);
    if (v && type == DWYCO_TYPE_STRING)
        CHECK(strcmp(v, "raw") == 0);
    else
        printf("[note] source read back as type %d\n", type);

    v = 0;
    CHECK(dwyco_get_setting("raw_files/raw_files_list", &v, &len, &type) != 0);
    if (v && type == DWYCO_TYPE_STRING)
        CHECK(strcmp(v, g_file_list) == 0);
}

// THE TEST.
//
// dwyco_enable_video_capture_preview(1) returning nonzero means initaq() came up
// AND build_outgoing() got as far as building a coder -- see the file header
// for the call chain. It is the one call that covers both, so it is the whole
// assertion. A zero here means something regressed in the build config or in
// the file list; the message points at the most likely cause of each.
static void
capture_initializes_from_raw_files(void)
{
    int on;

    on = dwyco_enable_video_capture_preview(1);
    if (!on) {
        printf("[FAIL] dwyco_enable_video_capture_preview(1) returned 0.\n"
            "       Two distinct things can cause this:\n"
            "       1. DWYCO_TESTING is not set in test_dwyco_cmake/conf.cmake,\n"
            "          so DWYCO_NO_VIDEO_FROM_PPM compiled out the raw-file path\n"
            "          and initaq() fell through to init_external_video() -- a bare\n"
            "          `return 0` under DWYCO_NO_ACQ_VIDEO_MEDIA.\n"
            "       2. initaq() succeeded but build_outgoing() then failed at\n"
            "          coder_from_config(), which happens whenever\n"
            "          DWYCO_NO_THEORA_CODEC is defined and there is no other\n"
            "          codec compiled in. Both defines are named in the header.\n"
            "       If it is neither of those, suspect the file list -- the\n"
            "       settings were verified above, so a bad path or unreadable\n"
            "       PPM would fail here instead.\n");
        g_fail++;
        return;
    }

    // Capture is up. Pump the service loop so the frame timer runs.
    service_ms(500);

    // Don't pin the count. The timer runs at rate/max_fps (50ms default,
    // mmbld.cc:476) and the load/consume alternation at mmchan.cc:4440 means one
    // frame per two ticks, so ~5 per 500ms -- but that is timing dependent.
    printf("\n      frames=%zu\n", raw_rec.size());
    CHECK(raw_rec.size() >= 3);
}

// Every recorded frame must be structurally correct and, when the content is
// known, bit-identical to the source frame it came from.
//
// Two separate claims are being checked:
//
//  structure  -- cols/rows/depth/chan are exactly what the preview path should
//                hand over, on every frame, not just the first.
//  content    -- each frame equals one of the frames this test wrote, and the
//                frames arrive in list order with none skipped or repeated.
//
// The content check is what proves the passthrough is really a passthrough and
// not a resample, a channel swap, or frames delivered out of order.
static void
frames_match_the_synthesized_source(void)
{
    if (raw_rec.empty()) {
        printf("[FAIL] no frames were recorded\n");
        g_fail++;
        return;
    }

    for (size_t i = 0; i < raw_rec.size(); ++i) {
        const raw_captured_frame &f = raw_rec[i];
        CHECK(f.chan == DWYCO_VIDEO_PREVIEW_CHAN);
        // depth 3 == color, depth 1 == gray. The raw source is P6.
        CHECK(f.depth == 3);
        CHECK(f.rgb.size() == (size_t)f.cols * (size_t)f.rows * 3);
    }

    if (!raw_frame_content_known)
        return;

    int cols = raw_frame_content_known ? RAW_FRAME_COLS : 0;
    int rows = raw_frame_content_known ? RAW_FRAME_ROWS : 0;
    g_fail += raw_check_frame_sequence(cols, rows);
}

// Turning preview back off must always succeed, whether or not the on-call
// worked, and must be safe to repeat.
static void
preview_off_is_safe(void)
{
    CHECK(dwyco_enable_video_capture_preview(0) != 0);
    CHECK(dwyco_enable_video_capture_preview(0) != 0);
}

// A second preview attempt after the first has come up and gone down. This is
// a distinct code path in dwyco_enable_video_capture_preview: if a channel was
// left behind it takes the "mc = find_xmitter(0)" branch and just flips the
// coder's gv_id instead of building a new one.
//
// Also re-checks that capture restarts cleanly: exitaq() drops the FileAcquire,
// so the second cycle must replay the list from frame 0.
static void
preview_can_be_cycled(void)
{
    raw_rec_reset();
    int first = dwyco_enable_video_capture_preview(1);
    service_ms(200);
    size_t after_first = raw_rec.size();
    CHECK(dwyco_enable_video_capture_preview(0) != 0);
    service_ms(100);

    // Second cycle starts from scratch: frame 0 of the list again.
    raw_rec_reset();
    int second = dwyco_enable_video_capture_preview(1);
    service_ms(200);
    size_t after_second = raw_rec.size();
    CHECK(dwyco_enable_video_capture_preview(0) != 0);

    // Both attempts should agree with each other. If the first succeeded and
    // the second did not, capture init is not idempotent, which is worth
    // knowing about independently of whether it works at all.
    CHECK(first == second);
    printf("\n      first=%d (%zu frames) second=%d (%zu frames)\n",
        first, after_first, second, after_second);

    // The restart check: after re-enabling, the first frame recorded must be
    // source frame 0, i.e. the list restarted rather than resuming.
    if (raw_frame_content_known && after_second > 0) {
        char why[512];
        int idx = raw_identify_frame(raw_rec[0], why, sizeof(why));
        if (idx != 0) {
            printf("[FAIL] after cycling preview, the first frame of the second"
                " cycle is source frame %d, expected 0 (the list should"
                " restart). %s\n", idx, why);
            g_fail++;
        }
    }
}

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
    test_bootstrap_profile("dwytest-vidcap", "dwytest video capture account");
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);
    return 1;
}

int
main(void)
{
    setvbuf(stdout, 0, _IOLBF, 0);
    printf("Dwyco video capture from raw (PPM) files\n");

    g_file_list = raw_ensure_file_list();
    if (!g_file_list) {
        fprintf(stderr, "no usable raw file list\n");
        return 1;
    }
    printf("  file list: %s\n", g_file_list);

    if (!init_test()) {
        fprintf(stderr, "dwyco_init failed\n");
        return 1;
    }

    // Before any capture request, so the frames land somewhere.
    dwyco_set_video_display_callback(raw_video_display);

    printf("\nSettings:\n");
    RUN(configure_raw_input);
    RUN(raw_input_settings_read_back);

    printf("\nCapture init:\n");
    RUN(capture_initializes_from_raw_files);
    // Must run before preview_can_be_cycled, which resets the recording.
    RUN(frames_match_the_synthesized_source);
    RUN(preview_off_is_safe);
    RUN(preview_can_be_cycled);

    dwyco_exit();

    printf("\nSummary: passed=%d failed=%d\n", g_pass, g_fail);
    if (g_fail) {
        printf("FAILED\n");
        return 1;
    }
    printf("Raw-file video capture initialized.\n");
    return 0;
}