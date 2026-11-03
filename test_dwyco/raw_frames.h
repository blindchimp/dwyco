#ifndef DWYCO_TEST_RAW_FRAMES_H
#define DWYCO_TEST_RAW_FRAMES_H

// Shared synthetic PPM frame source for the video-capture and zap tests.
//
// Both dwytest_vidcap_raw.cpp and dwytest_zap.cpp need a capture source that
// needs no camera, and both need to know the exact pixels so they can check
// what comes back out. That rules out an external sequence like
// /tmp/128x96/tennis.lst, whose contents the test cannot predict.
//
// Assumes "dlli.h" and <cstdio>/<cstring>/<sys/stat.h> are already included,
// matching the convention of test_common.h.
//
// Requires the DWYCO_TESTING build config (see test_dwyco_cmake/conf.cmake) so
// that DWYCO_NO_VIDEO_FROM_PPM is not defined and the raw-file capture path in
// cdc32 actually exists.

#include <sys/stat.h>

#define RAW_FRAME_DIR    "/tmp/dwytest_raw_frames"
#define RAW_FRAME_LIST   RAW_FRAME_DIR "/list.lst"
#define RAW_FRAME_COLS   128
#define RAW_FRAME_ROWS   96
// Enough frames that a record + play cycle cannot wrap the list, which would
// make frame-identification ambiguous.
#define RAW_FRAME_COUNT  40

// 1 when the capture source is one this test generated (so pixel values are
// known), 0 when DWYTEST_RAW_LIST pointed us at somebody else's frames.
static int raw_frame_content_known;

// Expected pixel value for source frame 'frame' at file coordinates (x, y),
// channel c (0=r, 1=g, 2=b).
//
// The per-frame design serves the comparison:
//   r is a ramp across x  -> catches horizontal geometry errors and R/B swaps
//   g is a ramp across y  -> catches vertical geometry errors and the row flip
//   b is constant per frame -> identifies which frame this is
//
// raw_write_ppm() uses this same function, so the written file and the
// expectation can never drift apart.
static int
raw_frame_expect(int frame, int x, int y, int c)
{
    switch (c) {
    case 0: return (x * 255) / (RAW_FRAME_COLS - 1);
    case 1: return (y * 255) / (RAW_FRAME_ROWS - 1);
    default: return (frame * 255) / RAW_FRAME_COUNT;
    }
}

// Write a binary P6 (maxval 255) frame.
static int
raw_write_ppm(const char *path, int frame)
{
    FILE *f = fopen(path, "wb");
    int x, y;
    if (!f)
        return 0;
    fprintf(f, "P6\n%d %d\n255\n", RAW_FRAME_COLS, RAW_FRAME_ROWS);
    for (y = 0; y < RAW_FRAME_ROWS; ++y) {
        for (x = 0; x < RAW_FRAME_COLS; ++x) {
            unsigned char rgb[3];
            rgb[0] = (unsigned char)raw_frame_expect(frame, x, y, 0);
            rgb[1] = (unsigned char)raw_frame_expect(frame, x, y, 1);
            rgb[2] = (unsigned char)raw_frame_expect(frame, x, y, 2);
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
    return 1;
}

// Produce the file list the capture path will read.
//
// Default: write RAW_FRAME_COUNT known-content P6 frames and list them.
//
// With DWYTEST_RAW_LIST set: use that list instead and clear
// raw_frame_content_known, since those pixels are not known. Callers must then
// skip content comparison.
//
// Returns NULL if the list could not be produced.
static const char *
raw_ensure_file_list(void)
{
    const char *want = getenv("DWYTEST_RAW_LIST");
    struct stat st;
    char path[512];
    FILE *lst;
    int i;

    if (want && *want) {
        if (stat(want, &st) != 0 || st.st_size == 0) {
            fprintf(stderr, "DWYTEST_RAW_LIST=%s is missing or empty\n", want);
            return 0;
        }
        printf("  using DWYTEST_RAW_LIST: %s\n", want);
        printf("  note: frame CONTENT is unknown for an external list, so"
            " content checks will be skipped\n");
        raw_frame_content_known = 0;
        return want;
    }

    printf("  synthesizing %d %dx%d P6 frames in %s\n",
        RAW_FRAME_COUNT, RAW_FRAME_COLS, RAW_FRAME_ROWS, RAW_FRAME_DIR);
    mkdir(RAW_FRAME_DIR, 0755);
    lst = fopen(RAW_FRAME_LIST, "wt");
    if (!lst)
        return 0;
    for (i = 0; i < RAW_FRAME_COUNT; ++i) {
        snprintf(path, sizeof(path), RAW_FRAME_DIR "/frame.%d.ppm", i);
        if (!raw_write_ppm(path, i)) {
            fclose(lst);
            return 0;
        }
        fprintf(lst, "%s\n", path);
    }
    fclose(lst);
    raw_frame_content_known = 1;
    return RAW_FRAME_LIST;
}

// ===== recorded-frame comparison =====
//
// A frame captured from the raw-file source arrives at the display callback
// bit-exact and row-reversed. FileAcquire::need() calls flip_in_place()
// [acqfile.h:199], which for T=pixel resolves to the row-reversing template at
// imgmisc.h:43 (the gray** overload at imgmisc.cc:557 is inside #if 0). Nothing
// else touches the pixels on that path, because the preview and record paths
// both bail out of the coder before any subsample/crop/pad/encode.

// A recorded frame plus the source index it was matched to (-1 if unmatched).
struct raw_captured_frame
{
    int chan;
    int cols;
    int rows;
    int depth;
    int src;                    // matched source frame index, -1 = no match
    std::vector<unsigned char> rgb; // cols*rows*3, row-major
};

static std::vector<raw_captured_frame> raw_rec;

// dwyco_set_video_display_callback() target. Copies every frame it is handed;
// the library frees the buffer immediately after the callback returns
// (ppm_freearray at mmchan.cc:4067), so recording the pointer would be a
// use-after-free.
//
// The api hands over a pixel** (dlli.cpp:1255 casts with (char **)). pixel is a
// struct of three unsigned chars with no padding, so sizeof(pixel) == 3 and it
// can be read as unsigned char** without pulling in ppm.h.
static void DWYCOCALLCONV
raw_video_display(int chan_id, void *img, int cols, int rows, int depth)
{
    raw_captured_frame f;

    if (!img || cols <= 0 || rows <= 0)
        return;

    f.chan = chan_id;
    f.cols = cols;
    f.rows = rows;
    f.depth = depth;
    f.src = -1;
    f.rgb.resize((size_t)cols * (size_t)rows * 3);
    const unsigned char **rows_ptr = (const unsigned char **)img;
    for (int y = 0; y < rows; ++y)
        memcpy(&f.rgb[(size_t)y * (size_t)cols * 3], rows_ptr[y], (size_t)cols * 3);
    raw_rec.push_back(f);
}

// Reset between capture sessions. Turning capture off tears down the
// FileAcquire, so a later session restarts the file list at frame 0 -- without
// this the ordering check would see a decrease and fail spuriously.
static void
raw_rec_reset(void)
{
    raw_rec.clear();
}

// Compare a recorded frame against source frame 'frame'. Returns 1 if identical.
// If 'why' is non-null, fills it with the first differing byte either way.
static int
raw_frame_matches(int frame, const raw_captured_frame &f, int flipped,
    char *why, size_t why_size)
{
    for (int y = 0; y < RAW_FRAME_ROWS; ++y) {
        // flipped: displayed row y corresponds to file row RAW_FRAME_ROWS-1-y.
        int fy = flipped ? (RAW_FRAME_ROWS - 1 - y) : y;
        for (int x = 0; x < RAW_FRAME_COLS; ++x) {
            for (int c = 0; c < 3; ++c) {
                int want = raw_frame_expect(frame, x, fy, c);
                int got = f.rgb[((size_t)y * RAW_FRAME_COLS + x) * 3 + c];
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

// Find which source frame a recorded frame is. Returns the index, or -1.
//
// Row order is the one transform the pipeline applies, so that is the primary
// model. If nothing matches, retry the other orientation so the failure message
// can distinguish "row order differs from what this test models" -- a real
// pipeline finding -- from "the pixels are actually different", which would
// point at a resample or a channel swap.
static int
raw_identify_frame(const raw_captured_frame &f, char *why, size_t why_size)
{
    char flip_detail[256];
    char noflip_detail[256];

    for (int frame = 0; frame < RAW_FRAME_COUNT; ++frame)
        if (raw_frame_matches(frame, f, 1, 0, 0))
            return frame;

    flip_detail[0] = 0;
    noflip_detail[0] = 0;
    // Record where frame 0 diverges under each row order. These must go into
    // separate buffers -- probing the other orientation must not clobber the
    // detail from the first.
    raw_frame_matches(0, f, 1, flip_detail, sizeof(flip_detail));
    if (raw_frame_matches(0, f, 0, 0, 0)) {
        if (why && why_size)
            snprintf(why, why_size,
                "recorded %dx%d frame matches NO source frame under the row-flip"
                " model, but matches source frame 0 WITHOUT the flip -- the"
                " pipeline's row order differs from what this test models",
                f.cols, f.rows);
        return -1;
    }
    raw_frame_matches(0, f, 0, noflip_detail, sizeof(noflip_detail));

    if (why && why_size)
        snprintf(why, why_size,
            "recorded %dx%d frame matches NO source frame under either row"
            " order. vs source 0 flipped: %s | vs source 0 unflipped: %s",
            f.cols, f.rows, flip_detail, noflip_detail);
    return -1;
}

// Identify every recorded frame and check the sequence is strictly increasing
// from 0 -- i.e. frames were consumed from the list in order, with nothing
// skipped or repeated.
//
// Prints per-failure diagnostics and returns the number of failures, so the
// caller can decide how to fold it into its own reporting. Structural checks
// (cols/rows/depth/chan) are done separately by the caller.
static int
raw_check_frame_sequence(int expect_cols, int expect_rows)
{
    char why[512];
    int prev = -1;
    int fails = 0;

    for (size_t i = 0; i < raw_rec.size(); ++i) {
        raw_captured_frame &f = raw_rec[i];
        if (f.cols != expect_cols || f.rows != expect_rows) {
            printf("[FAIL] recorded frame #%zu is %dx%d, expected %dx%d\n",
                i, f.cols, f.rows, expect_cols, expect_rows);
            fails++;
            continue;
        }
        why[0] = 0;
        f.src = raw_identify_frame(f, why, sizeof(why));
        if (f.src < 0) {
            printf("[FAIL] recorded frame #%zu: %s\n", i, why);
            fails++;
            continue;
        }
        if (f.src <= prev) {
            printf("[FAIL] recorded frame #%zu is source frame %d, but the"
                " previous one was source frame %d -- frames must arrive in"
                " list order\n", i, f.src, prev);
            fails++;
        }
        prev = f.src;
    }
    if (raw_rec.size() && fails == 0)
        printf("\n      source frames 0..%d matched bit-exactly, in order"
            " (%zu frames)\n", prev, raw_rec.size());
    return fails;
}

#endif