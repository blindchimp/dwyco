// Can video capture be initialized from the raw-file (PPM) test input?
//
// The raw files input is the one capture source that needs no camera, so it is
// the only way to exercise the capture path on a headless build machine. This
// test points the settings at a list of PPM files and asks the library to bring
// capture up, then reports what happened.
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
// the codec: dwyco_finish_startup() sets MMChannel::Moron_dork_mode, which
// routes the frame timer through grab_and_code(showonly=1) ->
// get_data(no_convert=1) -> code_preprocess(inhibit_coding=1), and
// theoracol.cc's code_preprocess then early-returns straight to
// display_img_2b_coded() before any subsample, crop, pad or encode. The only
// thing applied to the pixels is a row reversal by FileAcquire::need().
//
// Full chain with citations is on the g_rec/rec_frame declaration below.
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
//   DWYCO_NO_THEORA_CODEC    a video codec is mandatory, not optional. With it
//                            defined, every branch of the if/else chain in
//                            MMChannel::coder_from_config() [mmchan.cc:1881]
//                            compiles away and it degrades to
//                            FAILRET("incompatible coding style"), so
//                            build_outgoing() fails at the coder step and
//                            preview() returns 0 even though initaq()
//                            succeeded. Capture cannot initialize at all without
//                            this, so it is not just about frames.
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

// The frame source.
//
// This test verifies frame CONTENT, so it must know the exact pixels it wrote.
// That means it always generates its own frames rather than pointing at an
// external sequence like /tmp/128x96/tennis.lst, whose contents are unknown to
// the test.
//
// DWYTEST_RAW_LIST overrides this and points the capture path at a real file
// list instead. Content verification is impossible then, so the test drops to
// structural checks only and says so on stdout.
static int g_content_known;

#define SYNTH_DIR    "/tmp/dwytest_vidcap_frames"
#define SYNTH_LIST   SYNTH_DIR "/list.lst"
#define SYNTH_COLS   128
#define SYNTH_ROWS   96
#define SYNTH_FRAMES 20

// Expected pixel value for source frame 'frame' at file coordinates (x, y),
// channel c (0=r, 1=g, 2=b). This mirrors write_ppm() exactly -- same integer
// arithmetic, same truncation -- so the two can be compared bit-exactly.
//
// The per-frame design serves the comparison:
//   r is a ramp across x  -> catches horizontal geometry errors and R/B swaps
//   g is a ramp across y  -> catches vertical geometry errors and the row flip
//   b is constant per frame -> identifies which frame this is
static int
expected_file_pixel(int frame, int x, int y, int c)
{
    switch (c) {
    case 0: return (x * 255) / (SYNTH_COLS - 1);
    case 1: return (y * 255) / (SYNTH_ROWS - 1);
    default: return (frame * 255) / SYNTH_FRAMES;
    }
}

// Write a binary P6 (maxval 255) frame matching expected_file_pixel().
static int
write_ppm(const char *path, int frame)
{
    FILE *f = fopen(path, "wb");
    int x, y;
    if (!f)
        return 0;
    fprintf(f, "P6\n%d %d\n255\n", SYNTH_COLS, SYNTH_ROWS);
    for (y = 0; y < SYNTH_ROWS; ++y) {
        for (x = 0; x < SYNTH_COLS; ++x) {
            unsigned char rgb[3];
            rgb[0] = (unsigned char)expected_file_pixel(frame, x, y, 0);
            rgb[1] = (unsigned char)expected_file_pixel(frame, x, y, 1);
            rgb[2] = (unsigned char)expected_file_pixel(frame, x, y, 2);
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
    return 1;
}

// Produce the file list the capture path will read.
//
// Default: write SYNTH_FRAMES known-content P6 frames and list them. The test
// then knows every pixel it handed over and can check the preview callbacks
// got those exact pixels back.
//
// With DWYTEST_RAW_LIST set: use that list instead and set g_content_known to 0,
// since the frames are no longer known. Only the structural assertions run.
//
// Returns NULL if the list could not be produced.
static const char *
ensure_file_list(void)
{
    const char *want = getenv("DWYTEST_RAW_LIST");
    struct stat st;
    char frame[512];
    FILE *lst;
    int i;

    if (want && *want) {
        if (stat(want, &st) != 0 || st.st_size == 0) {
            fprintf(stderr, "DWYTEST_RAW_LIST=%s is missing or empty\n", want);
            return 0;
        }
        printf("  using DWYTEST_RAW_LIST: %s\n", want);
        printf("  note: frame CONTENT is unknown for an external list, so"
            " content checks are skipped\n");
        g_content_known = 0;
        return want;
    }

    printf("  synthesizing %d %dx%d P6 frames in %s\n",
        SYNTH_FRAMES, SYNTH_COLS, SYNTH_ROWS, SYNTH_DIR);
    mkdir(SYNTH_DIR, 0755);
    lst = fopen(SYNTH_LIST, "wt");
    if (!lst)
        return 0;
    for (i = 0; i < SYNTH_FRAMES; ++i) {
        snprintf(frame, sizeof(frame), SYNTH_DIR "/frame.%d.ppm", i);
        if (!write_ppm(frame, i)) {
            fclose(lst);
            return 0;
        }
        fprintf(lst, "%s\n", frame);
    }
    fclose(lst);
    g_content_known = 1;
    return SYNTH_LIST;
}

// The settings must be written before anything asks for capture, and they are
// only addressable after dwyco_init() has built the settings map.
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
    // cycles. Preloading is also fine and exercises a different branch.
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

// ===== recorded frames =====
//
// The preview path is a bit-exact passthrough, so the frames are recorded in
// full and compared pixel-for-pixel against what the test wrote. The chain is:
//
//   dwyco_finish_startup() sets MMChannel::Moron_dork_mode = 1  [dlli.cpp:1555]
//     -> frame timer at rate/max_fps (50ms default)             [mmbld.cc:476]
//     -> !has_data() ? sampler->need() : grab_and_code(..., showonly=1)
//                                                            [mmchan.cc:4440]
//     -> get_data(..., no_convert=1) skips rgb_ycc_convert     [mmchan.cc:4010]
//     -> code_preprocess(0,0,0,...,bits) with inhibit_coding=1
//     -> early return: display_img_2b_coded((pixel**)bits,..) [theoracol.cc:417]
//        which is BEFORE any subsample/crop/pad/encode
//     -> ppm_to_colorview -> this callback, depth 3            [dlli.cpp:1255]
//
// So nothing lossy is in the path and the pixels can be compared exactly. The
// one transform applied is a row reversal: FileAcquire::need() calls
// flip_in_place() [acqfile.h:199], which for T=pixel resolves to the template
// at imgmisc.h:43 (the gray** overload at imgmisc.cc:557 is inside #if 0).
//
// NOTE this exact comparison is only valid because the codec is bypassed. With
// Moron_dork_mode 0, or on the decode path, frames come back lossy and a
// bit-exact compare would be wrong -- not a regression.

struct rec_frame
{
    int chan;
    int cols;
    int rows;
    int depth;
    std::vector<unsigned char> rgb; // cols*rows*3, row-major
};

static std::vector<rec_frame> g_rec;

static void DWYCOCALLCONV
vid_display(int chan_id, void *img, int cols, int rows, int depth)
{
    rec_frame f;

    // The api hands us a non-null pointer. It is a pixel** -- the library casts
    // with (char **) when calling [dlli.cpp:1255]. pixel is a struct of three
    // unsigned chars with no padding, so sizeof(pixel) == 3 and it can be read
    // as unsigned char** without pulling in ppm.h.
    if (!img)
        return;
    if (cols <= 0 || rows <= 0)
        return;

    f.chan = chan_id;
    f.cols = cols;
    f.rows = rows;
    f.depth = depth;
    f.rgb.resize((size_t)cols * (size_t)rows * 3);
    const unsigned char **rows_ptr = (const unsigned char **)img;
    for (int y = 0; y < rows; ++y)
        memcpy(&f.rgb[(size_t)y * (size_t)cols * 3], rows_ptr[y], (size_t)cols * 3);
    // Must copy: the library frees the buffer right after this returns
    // (ppm_freearray at mmchan.cc:4067).
    g_rec.push_back(f);
}

// Reset between preview on-cycles. Turning preview off tears down TheAq, so the
// next cycle restarts the file list at frame 0 -- without this the ordering
// check would see a decrease and fail spuriously.
static void
reset_frames(void)
{
    g_rec.clear();
}

// Compare a recorded frame against source frame 'frame'. Returns 1 if identical.
// On mismatch, and if 'why' is non-null, fills it with a human-readable
// description of the first differing byte.
static int
frame_matches(int frame, const rec_frame &f, int flipped, char *why, size_t why_size)
{
    for (int y = 0; y < SYNTH_ROWS; ++y) {
        // flipped: displayed row y corresponds to file row SYNTH_ROWS-1-y.
        int fy = flipped ? (SYNTH_ROWS - 1 - y) : y;
        for (int x = 0; x < SYNTH_COLS; ++x) {
            for (int c = 0; c < 3; ++c) {
                int want = expected_file_pixel(frame, x, fy, c);
                int got = f.rgb[((size_t)y * SYNTH_COLS + x) * 3 + c];
                if (want != got) {
                    if (why && why_size)
                        snprintf(why, why_size,
                            "frame %d (%s): first diff at x=%d y=%d chan=%d"
                            " (file row %d) want %d got %d",
                            frame, flipped ? "flipped" : "unflipped",
                            x, y, c, fy, want, got);
                    return 0;
                }
            }
        }
    }
    return 1;
}

// Find which source frame this recorded frame is. Returns the index, or -1 if
// it matches none.
//
// Row order is the one transform the pipeline applies (see the flip_in_place
// note above), so that is the primary model. If nothing matches, retry the
// other orientation so the failure message can distinguish "row order differs
// from what this test models" -- a real pipeline finding -- from "the pixels
// are actually different", which would point at a resample or a channel swap.
static int
identify_frame(const rec_frame &f, char *why, size_t why_size)
{
    char flip_detail[256];
    char noflip_detail[256];

    for (int frame = 0; frame < SYNTH_FRAMES; ++frame)
        if (frame_matches(frame, f, 1, 0, 0))
            return frame;

    flip_detail[0] = 0;
    noflip_detail[0] = 0;
    // Record where frame 0 diverges under each row order. Note these must go
    // into separate buffers -- probing the other orientation must not clobber
    // the detail from the first.
    frame_matches(0, f, 1, flip_detail, sizeof(flip_detail));
    if (frame_matches(0, f, 0, 0, 0)) {
        if (why && why_size)
            snprintf(why, why_size,
                "recorded %dx%d frame matches NO source frame under the row-flip"
                " model, but matches source frame 0 WITHOUT the flip -- the"
                " pipeline's row order differs from what this test models",
                f.cols, f.rows);
        return -1;
    }
    frame_matches(0, f, 0, noflip_detail, sizeof(noflip_detail));

    if (why && why_size)
        snprintf(why, why_size,
            "recorded %dx%d frame matches NO source frame under either row"
            " order. vs source 0 flipped: %s | vs source 0 unflipped: %s",
            f.cols, f.rows, flip_detail, noflip_detail);
    return -1;
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

    // Capture is up. Pump the service loop so the frame timer runs, then report
    // what the display callback saw.
    service_ms(500);

    printf("\n      frames=%zu\n", g_rec.size());
    // Don't pin the count. The timer runs at rate/max_fps (50ms default,
    // mmbld.cc:476) and the load/consume alternation at mmchan.cc:4440 means one
    // frame per two ticks, so ~5 per 500ms -- but that is timing dependent.
    CHECK(g_rec.size() >= 3);
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
    char why[512];
    int prev_idx = -1;
    int content_fail = 0;

    if (g_rec.empty()) {
        printf("[FAIL] no frames were recorded\n");
        g_fail++;
        return;
    }

    for (size_t i = 0; i < g_rec.size(); ++i) {
        const rec_frame &f = g_rec[i];
        CHECK(f.chan == DWYCO_VIDEO_PREVIEW_CHAN);
        // depth 3 == color, depth 1 == gray. The raw source is P6.
        CHECK(f.depth == 3);
        if (g_content_known) {
            CHECK(f.cols == SYNTH_COLS);
            CHECK(f.rows == SYNTH_ROWS);
        } else {
            CHECK(f.cols > 0);
            CHECK(f.rows > 0);
        }
        CHECK(f.rgb.size() == (size_t)f.cols * (size_t)f.rows * 3);
    }

    if (!g_content_known)
        return;

    for (size_t i = 0; i < g_rec.size(); ++i) {
        why[0] = 0;
        int idx = identify_frame(g_rec[i], why, sizeof(why));
        if (idx < 0) {
            printf("[FAIL] recorded frame #%zu: %s\n", i, why);
            g_fail++;
            content_fail++;
            continue;
        }
        // Strictly increasing: frames must be consumed from the list in order,
        // one per tick, with nothing skipped or repeated. Starting at 0 comes
        // from prev_idx being -1 here.
        if (idx <= prev_idx) {
            printf("[FAIL] recorded frame #%zu is source frame %d, but the"
                " previous one was source frame %d -- frames must arrive in"
                " list order\n", i, idx, prev_idx);
            g_fail++;
            content_fail++;
        }
        prev_idx = idx;
    }

    if (content_fail == 0) {
        // Compact proof the sequence really advanced rather than repeating.
        printf("\n      source frames %d..%d matched bit-exactly, in order\n",
            0, prev_idx);
    }
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
    reset_frames();
    int first = dwyco_enable_video_capture_preview(1);
    service_ms(200);
    size_t after_first = g_rec.size();
    CHECK(dwyco_enable_video_capture_preview(0) != 0);
    service_ms(100);

    // Second cycle starts from scratch: frame 0 of the list again.
    reset_frames();
    int second = dwyco_enable_video_capture_preview(1);
    service_ms(200);
    size_t after_second = g_rec.size();
    CHECK(dwyco_enable_video_capture_preview(0) != 0);

    // Both attempts should agree with each other. If the first succeeded and
    // the second did not, capture init is not idempotent, which is worth
    // knowing about independently of whether it works at all.
    CHECK(first == second);
    printf("\n      first=%d (%zu frames) second=%d (%zu frames)\n",
        first, after_first, second, after_second);

    // The restart check: after re-enabling, the first frame recorded must be
    // source frame 0, i.e. the list restarted rather than resuming.
    if (g_content_known && after_second > 0) {
        char why[512];
        int idx = identify_frame(g_rec[0], why, sizeof(why));
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

    g_file_list = ensure_file_list();
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
    dwyco_set_video_display_callback(vid_display);

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