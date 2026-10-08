// Coverage for the zap composition / record / playback / send api, and for the
// zap view api that reads an attachment back out of a saved message.
//
//   Compose:   dwyco_make_zap_composition, dwyco_delete_zap_composition
//   Capture:   dwyco_zap_record, dwyco_zap_record2, dwyco_zap_stop,
//              dwyco_zap_composition_chan_id, dwyco_zap_still_active
//   Playback:  dwyco_zap_play
//   Send:      dwyco_zap_send4, dwyco_zap_send6
//   Locate:    dwyco_get_message_index, dwyco_get_saved_message3,
//              dwyco_get_body_array
//   View:      dwyco_make_zap_view2, dwyco_zap_quick_stats_view,
//              dwyco_zap_create_preview_buf, dwyco_zap_play_view_no_audio,
//              dwyco_zap_stop_view, dwyco_delete_zap_view
//
// The whole flow runs against a single client with NO peer and NO live server:
//
//   * recording video calls MMChannel::build_outgoing(1, 1, ...) [mcc.cpp:415],
//     which is the same initaq() entry point the capture preview uses. So with
//     video_input/source="raw" it records from the synthetic PPM frames in
//     raw_frames.h instead of needing a camera.
//   * sending a zap to your OWN uid with an attachment short-circuits the whole
//     delivery state machine [directsend.cpp:564-585]: it calls
//     cleanup_after_send() (which does do_local_store) then succeed(),
//     synchronously, locally.
//   * a self-send also forces dont_save_sent=0 [dlli.cpp:5721-5729] so the
//     message is always filed.
//
// Locating the message afterwards must NOT go through the "_inbox" tag: a
// self-send is *stored*, never received, and do_local_store adds no _inbox tag.
// dwyco_new_msg2() would never see it. Use the message index keyed on our own
// uid instead -- for a sent message assoc_uid is the recipient, which here is
// us.
//
// Note there is no plain dwyco_make_zap_view in the api; the entry points are
// dwyco_make_zap_view2 (from a saved message), dwyco_make_zap_view_file and
// dwyco_make_zap_view_file_raw (from a file on disk).
//
// Documented oddities this pins rather than works around:
//
//  * dwyco_zap_record's `frames` argument is COMPLETELY IGNORED. The body only
//    reads video, audio, pic, dcb and dcb_arg1. Only dwyco_zap_record2 sets
//    max_frames/max_bytes. Worse, TMsgCompose's constructor sets
//    max_frames = -1 [mcc.cpp:175] and record_buttonClick copies that to
//    ft->packet_count [mcc.cpp:379], so dwyco_zap_record records INDEFINITELY
//    and dwyco_zap_stop is mandatory.
//
//  * dwyco_zap_record always reports success, even when it recorded nothing.
//    record_buttonClick returns 0 when video, audio and pic are all zero
//    [mcc.cpp:344], but dwyco_zap_record discards that and unconditionally
//    `return 1`. dwyco_zap_record2 checks the same value and returns 0. So the
//    two near-identical entry points disagree, and the common one lies.
//
//  * A composition with no media correctly refuses both dwyco_zap_stop and
//    dwyco_zap_play: FormShow() -> init_av_buttons() sets stop_button_enabled
//    and play_button_enabled to 0 [mcc.cpp:745-746]. Note this overrides the
//    constructor's stop_button_enabled = 1 [mcc.cpp:146], so the "probably not
//    recording" guard in dwyco_zap_stop is unreachable for a fresh composition.
//
// Two traps handled deliberately:
//
//  * dwyco_zap_create_preview_buf returns a PPM (an array of row pointers), NOT
//    a flat buffer, despite the header calling it a "buf". dwyco_free_array is
//    `delete []` [dlli.cpp:1350] and would mismatch-malloc against it. The right
//    free is dwyco_free_image(buf, rows) -> ppm_freearray [dlli.cpp:1357].
//
//  * A composition id is an int cookie. Passing a made-up one must be rejected
//    rather than dereferenced, so every entry point is probed with a bogus id.

#include <dlli.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>

#include "test_common.h"
#include "list_readback.h"
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

static const char *g_dir = "/tmp/dwytest_zap";
static const char *g_file_list;

// File zap fixtures. The source deliberately lives in a subdirectory so that
// comparing the stored FILE_ATTACHMENT against FILE_ZAP_BASENAME is a real test
// of the basename logic rather than a tautology.
static const char *FILE_ZAP_SRC = "/tmp/dwytest_zap_src/submission.bin";
static const char *FILE_ZAP_BASENAME = "submission.bin";
static const char *FILE_ZAP_OUT = "/tmp/dwytest_zap_out.bin";

static char g_my_uid[64];
static int g_my_uid_len;

// The composition recorded in record_then_stop_captures_frames(), kept alive so
// the play and send sections can use the same recorded media.
static int g_last_compid;

static int g_send_done;     // saw DWYCO_SE_MSG_SEND_SUCCESS or _FAIL
static int g_send_was_fail;

// Channel-destroy callbacks. dwyco_zap_record/play store these on the
// composition and reset_buttons() invokes one when the record/play channel is
// torn down [mcc.cpp:315]. The id handed back is the composition cookie, not a
// channel id.
static int g_record_dcb_calls;
static int g_record_dcb_id;
static int g_play_dcb_calls;
static int g_play_dcb_id;

static void DWYCOCALLCONV
record_dcb(int id, void *arg)
{
    g_record_dcb_calls++;
    g_record_dcb_id = id;
    (void)arg;
}

static void DWYCOCALLCONV
play_dcb(int id, void *arg)
{
    g_play_dcb_calls++;
    g_play_dcb_id = id;
    (void)arg;
}

static void DWYCOCALLCONV
system_event(int cmd, int ctx_id, const char *uid, int len_uid,
    const char *name, int len_name, int type, const char *value, int val_len,
    int qid, int extra_arg)
{
    if (cmd == DWYCO_SE_MSG_SEND_SUCCESS || cmd == DWYCO_SE_MSG_SEND_FAIL) {
        g_send_done = 1;
        if (cmd == DWYCO_SE_MSG_SEND_FAIL)
            g_send_was_fail = 1;
    }
    (void)ctx_id; (void)uid; (void)len_uid; (void)name; (void)len_name;
    (void)type; (void)value; (void)val_len; (void)qid; (void)extra_arg;
}

static const char *ZAP_TEXT = "zap coverage test";
static const char *FILE_ZAP_TEXT = "file zap coverage test";

// mid of the self-sent file zap, located by its body text.
static char g_file_mid[256];
static int g_file_mid_len;

// ===== composition =====

// dwyco_make_zap_composition takes a char* that it never reads [dlli.cpp:4871],
// so NULL is safe here despite the project-wide rule about not handing the api
// a null pointer. The cookie must be non-zero.
static void
make_composition_accepts_null_dummy(void)
{
    int compid = dwyco_make_zap_composition(0);
    CHECK(compid != 0);
    if (!compid) {
        printf("[FAIL] could not create a zap composition at all\n");
        return;
    }
    // A fresh composition has nothing recorded, so there is no recording channel.
    CHECK(dwyco_zap_composition_chan_id(compid) == -1);
    CHECK(dwyco_delete_zap_composition(compid) != 0);
}

// Every entry point validates the composition cookie instead of dereferencing
// it. cookie_to_ptr() rejects the invalid sentinel.
static void
bogus_composition_ids_are_rejected(void)
{
    int chan_id = 0;
    const char *pers = "";
    int pers_len = 0;

    CHECK(dwyco_zap_composition_chan_id(-1) == 0);
    CHECK(dwyco_zap_stop(-1) == 0);
    CHECK(dwyco_zap_play(-1, play_dcb, 0, &chan_id) == 0);
    CHECK(dwyco_zap_record(-1, 1, 0, 0, 1, record_dcb, 0, &chan_id) == 0);
    CHECK(dwyco_zap_record2(-1, 1, 0, 10, 1000, 0, 0, 0, record_dcb, 0,
        &chan_id) == 0);
    CHECK(dwyco_zap_send4(-1, g_my_uid, g_my_uid_len, ZAP_TEXT,
        (int)strlen(ZAP_TEXT), 0, &pers, &pers_len) == 0);
    CHECK(dwyco_delete_zap_composition(-1) == 0);
}

// dwyco_zap_record reports success even when it recorded nothing.
//
// record_buttonClick returns 0 immediately when video, audio and pic are all
// zero [mcc.cpp:344], but dwyco_zap_record throws that return value away and
// unconditionally `return 1` [dlli.cpp:5545]. dwyco_zap_record2 does the
// opposite and checks it. So a caller of dwyco_zap_record is told the recording
// started when it did not.
//
// Nothing is actually created, which is what makes this safe to pin: the view
// id is never assigned and no recording channel appears.
static void
record_lies_when_nothing_was_requested(void)
{
    int compid = dwyco_make_zap_composition(0);
    int chan_id = 12345;
    CHECK(compid != 0);
    if (!compid) return;

    // Reports success...
    CHECK(dwyco_zap_record(compid, 0, 0, 0, 1, record_dcb, 0, &chan_id) != 0);
    // ...but nothing happened: view_id is still the constructor's -1 and
    // out chan_id_out that, not a channel id.
    CHECK(chan_id == -1);
    CHECK(dwyco_zap_composition_chan_id(compid) == -1);

    // record2 is the honest one: same request, but it checks and returns 0.
    CHECK(dwyco_zap_record2(compid, 0, 0, 10, 1000, 0, 0, 0, record_dcb, 0,
        &chan_id) == 0);

    CHECK(dwyco_delete_zap_composition(compid) != 0);
}

// FormShow() calls init_av_buttons(), which disables both stop and play on a
// composition with no media [mcc.cpp:745-746]. So the two "am I still recording"
// controls correctly reject rather than claiming success. (The constructor sets
// stop_button_enabled = 1, but FormShow overwrites that before anyone can call
// in.)
static void
stop_and_play_rejected_when_idle(void)
{
    int compid = dwyco_make_zap_composition(0);
    int chan_id = 0;
    CHECK(compid != 0);
    if (!compid) return;

    CHECK(dwyco_zap_stop(compid) == 0);
    CHECK(dwyco_zap_play(compid, play_dcb, 0, &chan_id) == 0);

    // Neither actually started a channel, so no destroy callback fired.
    service_ms(300);
    CHECK(g_play_dcb_calls == 0);

    CHECK(dwyco_delete_zap_composition(compid) != 0);
}

// ===== record / stop =====

// Record video from the synthetic PPM frames, confirm frames flow and match the
// source bit-exactly, then stop.
static int g_rec_chan_id;

static void
record_then_stop_captures_frames(void)
{
    int compid = dwyco_make_zap_composition(0);
    int chan_id = 0;
    CHECK(compid != 0);
    if (!compid) return;

    raw_rec_reset();
    g_record_dcb_calls = 0;
    g_record_dcb_id = -999;

    // audio=0 on purpose: this box has no capture hardware and the build has
    // no gsm/vorbis. `frames` is passed but ignored -- see the file header.
    int rc = dwyco_zap_record(compid, 1, 0, 0, 3, record_dcb, 0, &chan_id);
    CHECK(rc != 0);
    if (!rc) {
        printf("[FAIL] dwyco_zap_record refused to record. With"
            " video_input/source=\"raw\" it goes through"
            " MMChannel::build_outgoing(1,1,...) [mcc.cpp:415], the same"
            " initaq() path as the capture preview. If that fails, check the"
            " DWYCO_TESTING define -- see dwytest_vidcap_raw.cpp.\n");
        dwyco_delete_zap_composition(compid);
        return;
    }

    // chan_id_out is the view id, which is also the channel the record runs on
    // and the channel id the display callback reports.
    CHECK(chan_id > 0);
    CHECK(dwyco_zap_composition_chan_id(compid) == chan_id);
    CHECK(dwyco_zap_still_active(compid) != 0);

    // Recording is unbounded (max_frames defaults to -1), so pump briefly and
    // then stop. 600ms at the configured rate is several frames.
    service_ms(600);

    printf("\n      recorded %zu frames on chan %d\n", raw_rec.size(), chan_id);
    CHECK(raw_rec.size() >= 2);
    for (size_t i = 0; i < raw_rec.size(); ++i) {
        // record_buttonClick appends view_id to the coder's display list
        // [mcc.cpp:432] rather than setting gv_id, so frames come out tagged
        // with the record channel.
        CHECK(raw_rec[i].chan == chan_id);
        CHECK(raw_rec[i].depth == 3);
    }
    if (raw_frame_content_known)
        g_fail += raw_check_frame_sequence(RAW_FRAME_COLS, RAW_FRAME_ROWS);

    // Stop. This destroys the record channel, which is what makes
    // reset_buttons() run and invoke our callback.
    CHECK(dwyco_zap_stop(compid) != 0);
    service_ms(200);

    CHECK(g_record_dcb_calls >= 1);
    // reset_buttons passes the composition cookie, not a channel id.
    CHECK(g_record_dcb_id == compid);
    // Once stopped there is no recording channel left.
    CHECK(dwyco_zap_composition_chan_id(compid) == -1);

    g_rec_chan_id = chan_id;
    // Keep compid alive for the play section.
    g_last_compid = compid;
}

// dwyco_zap_record2 is the same thing with explicit limits. max_frames is a
// packet count, so it bounds the recording and the channel self-terminates
// without zap_stop.
static void
record2_bounds_the_recording(void)
{
    int compid = dwyco_make_zap_composition(0);
    int chan_id = 0;
    CHECK(compid != 0);
    if (!compid) return;

    raw_rec_reset();
    g_record_dcb_calls = 0;
    g_record_dcb_id = -999;

    int rc = dwyco_zap_record2(compid, 1, 0, /*max_frames*/4, /*max_bytes*/100000,
        /*hi_quality*/0, /*scb*/0, /*scb_arg*/0, record_dcb, 0, &chan_id);
    CHECK(rc != 0);
    if (!rc) {
        dwyco_delete_zap_composition(compid);
        return;
    }
    CHECK(chan_id > 0);

    // Give it time to hit the packet limit and tear itself down.
    service_ms(1500);

    printf("\n      record2 captured %zu frames (max_frames=4)\n",
        raw_rec.size());
    // Self-terminated: the destroy callback fired without any zap_stop. This
    // is the observable contract of max_frames, and it is worth asserting
    // precisely because dwyco_zap_record IGNORES its `frames` argument -- so
    // without a real packet limit a recording would never end.
    CHECK(g_record_dcb_calls >= 1);
    CHECK(g_record_dcb_id == compid);
    CHECK(dwyco_zap_composition_chan_id(compid) == -1);

    // Note: do NOT assert raw_rec.size() <= max_frames. max_frames bounds
    // ft->packet_count, the number of packets written to the tube, while
    // raw_rec counts frames handed to the display callback -- and the two
    // differ by about a frame. An earlier version of this test asserted the
    // bound and passed only by luck with the synthetic 128x96 frames; with a
    // real 128x87 sequence it counted 5 frames against a limit of 4.

    // Play is now enabled because stop_buttonClick turned it on.
    CHECK(dwyco_zap_play(compid, play_dcb, 0, &chan_id) != 0);
    service_ms(400);
    CHECK(dwyco_zap_stop(compid) != 0);
    service_ms(200);

    CHECK(dwyco_delete_zap_composition(compid) != 0);
}

// A composition that is already recording refuses a second record: the first
// record cleared record_button_enabled [mcc.cpp:349].
static void
record_twice_is_rejected(void)
{
    int compid = dwyco_make_zap_composition(0);
    int chan_id = 0, chan_id2 = 0;
    CHECK(compid != 0);
    if (!compid) return;

    CHECK(dwyco_zap_record(compid, 1, 0, 0, 1, record_dcb, 0, &chan_id) != 0);
    CHECK(dwyco_zap_record(compid, 1, 0, 0, 1, record_dcb, 0, &chan_id2) == 0);

    CHECK(dwyco_zap_stop(compid) != 0);
    service_ms(200);

    // After stopping, recording is possible again.
    CHECK(dwyco_zap_record(compid, 1, 0, 0, 1, record_dcb, 0, &chan_id2) != 0);
    CHECK(dwyco_zap_stop(compid) != 0);
    service_ms(200);

    CHECK(dwyco_delete_zap_composition(compid) != 0);
}

// ===== playback =====

// Play back what was recorded in record_then_stop_captures_frames(). The
// composition is still alive from that section.
//
// Playback builds an incoming channel off the recorded file. Audio output may or
// may not come up on a machine with no sound card, so assert only that the call
// reports success and that the channel eventually tears down.
static void
play_recorded_zap(void)
{
    int compid = g_last_compid;
    int chan_id = 0;

    if (!compid) {
        printf("[skip] no recorded composition available\n");
        return;
    }

    g_play_dcb_calls = 0;
    g_play_dcb_id = -999;

    CHECK(dwyco_zap_play(compid, play_dcb, 0, &chan_id) != 0);
    CHECK(chan_id > 0);
    CHECK(dwyco_zap_composition_chan_id(compid) == chan_id);

    service_ms(400);

    // It may or may not have self-terminated (play_buttonClick arms
    // auto_stop_delay). Either way it must be stoppable and must report done.
    if (g_play_dcb_calls == 0) {
        CHECK(dwyco_zap_stop(compid) != 0);
        service_ms(200);
    }
    CHECK(g_play_dcb_calls >= 1);
    CHECK(g_play_dcb_id == compid);
}

// ===== send to self =====

static char g_zap_mid[256];
static int g_zap_mid_len;

static void
send_zap_to_self(void)
{
    int compid = g_last_compid;
    const char *pers = 0;
    int pers_len = 0;

    if (!compid) {
        printf("[skip] no recorded composition available\n");
        return;
    }

    g_send_done = 0;
    g_send_was_fail = 0;

    // save_sent=0 is deliberate: sending to yourself forces dont_save_sent=0
    // regardless [dlli.cpp:5721-5729], so the message is always filed.
    int rc = dwyco_zap_send6(compid, g_my_uid, g_my_uid_len, ZAP_TEXT,
        (int)strlen(ZAP_TEXT), /*no_forward*/0, /*save_sent*/0, /*defer*/0,
        &pers, &pers_len);
    CHECK(rc != 0);
    if (!rc) return;

    if (!wait_for([&]() { return g_send_done != 0; }, 60000)) {
        printf("[FAIL] no DWYCO_SE_MSG_SEND_SUCCESS/FAIL within 60s\n");
        g_fail++;
        return;
    }
    CHECK(!g_send_was_fail);

    // do_local_store is synchronous for a self-send with an attachment
    // [directsend.cpp:564-585], so give the index a moment regardless.
    service_ms(500);
}

// Find a saved message we sent to ourselves by its body text.
//
// Must use the message index rather than dwyco_get_tagged_idx("_inbox"): a
// self-send is *stored*, never received, and do_local_store adds no _inbox tag,
// so dwyco_new_msg2() would never see it. For a sent message assoc_uid is the
// recipient, which here is us.
//
// Matching on the text rather than blindly taking row 0 matters: this test
// sends two different messages (a media zap and a file zap) and the client dir
// may carry messages from earlier runs. The index is ordered newest-first by
// logical clock.
//
// Returns 1 and fills out/len_out on success.
static int
find_mid_by_text(const char *want_text, char *out, int out_size, int *len_out)
{
    DWYCO_MSG_IDX idx = 0;

    if (!want_text || !out || out_size < 2 || !len_out)
        return 0;
    out[0] = 0;
    *len_out = 0;

    if (!dwyco_get_message_index(&idx, g_my_uid, g_my_uid_len)) {
        printf("[FAIL] dwyco_get_message_index failed for our own uid\n");
        return 0;
    }

    int rows = 0, cols = 0;
    if (dwyco_list_numelems(idx, &rows, &cols) != 0 && rows > 0) {
        for (int i = 0; i < rows; ++i) {
            const char *mid = 0;
            int mid_len = 0, type = 0;
            if (!dwyco_list_get(idx, i, DWYCO_MSG_IDX_MID, &mid, &mid_len, &type))
                continue;
            if (type != DWYCO_TYPE_STRING || mid_len <= 0)
                continue;
            char midbuf[256];
            int n = mid_len < (int)sizeof(midbuf) - 1
                ? mid_len : (int)sizeof(midbuf) - 1;
            memcpy(midbuf, mid, (size_t)n);
            midbuf[n] = 0;

            DWYCO_SAVED_MSG_LIST sm = 0;
            if (dwyco_get_saved_message3(&sm, midbuf) != DWYCO_GSM_SUCCESS)
                continue;
            // dwyco_get_body_array returns one row per forwarded component, with
            // the top-level body at row 0. lr_rows() *asserts* a count, so ask
            // for the real one first rather than guessing.
            DWYCO_LIST ba = dwyco_get_body_array(sm);
            int ba_rows = -1, ba_cols = -1;
            int matched = 0;
            if (ba && dwyco_list_numelems(ba, &ba_rows, &ba_cols) != 0
                && ba_rows >= 1)
                matched = lr_col_str(ba, 0, DWYCO_QM_BODY_NEW_TEXT2, want_text,
                    (int)strlen(want_text));
            if (ba)
                dwyco_list_release(ba);
            dwyco_list_release(sm);
            if (matched) {
                memcpy(out, midbuf, (size_t)n + 1);
                *len_out = n;
                dwyco_list_release(idx);
                return 1;
            }
        }
    }
    dwyco_list_release(idx);
    printf("[note] no saved message with text \'%s\' (%d index rows scanned)\n",
        want_text, rows);
    return 0;
}

static void
find_self_sent_message(void)
{
    if (g_zap_mid_len) {           // already found
        CHECK(g_zap_mid_len > 0);
        return;
    }
    if (!find_mid_by_text(ZAP_TEXT, g_zap_mid, (int)sizeof(g_zap_mid),
        &g_zap_mid_len)) {
        printf("[FAIL] could not locate the self-sent media zap via the"
            " message index\n");
        g_fail++;
        return;
    }
    printf("\n      found self-sent zap mid=%s\n", g_zap_mid);
}

// ===== saved message body =====

static void
self_sent_body_is_well_formed(void)
{
    DWYCO_SAVED_MSG_LIST sm = 0;

    if (!g_zap_mid_len) {
        printf("[skip] no message located\n");
        return;
    }

    int rc = dwyco_get_saved_message3(&sm, g_zap_mid);
    CHECK(rc == DWYCO_GSM_SUCCESS);
    CHECK(sm != 0);
    if (!sm) return;

    int rows = 0, cols = 0;
    CHECK(dwyco_list_numelems(sm, &rows, &cols) != 0);
    CHECK(rows == 1);
    CHECK(cols > 0);

    // The body's own id column must agree with the mid we looked it up by.
    const char *v = 0;
    int vl = 0, vt = 0;
    CHECK(lr_col_str(sm, 0, DWYCO_QM_BODY_ID, g_zap_mid, g_zap_mid_len));

    // FROM is the raw binary uid, not hex [dlli.h:1124].
    CHECK(dwyco_list_get(sm, 0, DWYCO_QM_BODY_FROM, &v, &vl, &vt) != 0);
    CHECK(vl == 10);
    if (vl == 10)
        CHECK(memcmp(v, g_my_uid, 10) == 0);

    // There must be a zap attachment, and it must NOT be a file zap -- a file
    // attachment makes dwyco_make_zap_view2 return 0 [dlli.cpp:5910].
    //
    // Checked with dwyco_list_get's type rather than the lr_* helpers: those
    // assert an expected value and print their own [FAIL], so asking
    // lr_col_nil() to assert "not nil" is a contradiction and would emit a
    // spurious diagnostic.
    const char *att = 0;
    int att_len = 0, att_type = 0;
    CHECK(dwyco_list_get(sm, 0, DWYCO_QM_BODY_ATTACHMENT, &att, &att_len,
        &att_type) != 0);
    CHECK(att_type == DWYCO_TYPE_STRING);
    CHECK(att_len > 0);

    const char *fatt = 0;
    int fatt_len = 0, fatt_type = 0;
    CHECK(dwyco_list_get(sm, 0, DWYCO_QM_BODY_FILE_ATTACHMENT, &fatt,
        &fatt_len, &fatt_type) != 0);
    CHECK(fatt_type == DWYCO_TYPE_NIL);

    // Flattened text view: exactly one row.
    DWYCO_LIST bt = dwyco_get_body_text(sm);
    CHECK(bt != 0);
    if (bt)
        CHECK(lr_rows(bt, 1));
    if (bt)
        dwyco_list_release(bt);

    // A message we sent ourselves authenticates against our own uid.
    int auth = dwyco_authenticate_body(sm, g_my_uid, g_my_uid_len, 0);
    CHECK(auth == DWYCO_VERF_AUTH_OK);

    dwyco_list_release(sm);
}

// ===== view api =====

static int g_viewid;

static void
make_view_from_saved_message(void)
{
    DWYCO_SAVED_MSG_LIST sm = 0;

    if (!g_zap_mid_len) {
        printf("[skip] no message located\n");
        return;
    }
    if (dwyco_get_saved_message3(&sm, g_zap_mid) != DWYCO_GSM_SUCCESS || !sm) {
        printf("[FAIL] could not reopen the saved message for viewing\n");
        g_fail++;
        return;
    }

    // qd=0: this body came from dwyco_get_saved_message3, so it has a real mid
    // and its attachment lives under <uidhex>.usr/ [dlli.cpp:5921-5930].
    int viewid = dwyco_make_zap_view2(sm, 0);
    CHECK(viewid != 0);
    // Do NOT test `viewid > 0`: the _file variants return the invalid sentinel
    // 0x55555555 rather than 0.
    CHECK(viewid != 0x55555555);
    g_viewid = viewid;

    dwyco_list_release(sm);
}

static void
view_quick_stats(void)
{
    int has_video = -1, has_audio = -1, short_video = -1;

    if (!g_viewid) {
        printf("[skip] no view\n");
        return;
    }
    int rc = dwyco_zap_quick_stats_view(g_viewid, &has_video, &has_audio,
        &short_video);
    CHECK(rc != 0);
    CHECK(has_video == 0 || has_video == 1);
    CHECK(has_audio == 0 || has_audio == 1);
    // We recorded video only.
    CHECK(has_video == 1);
    printf("\n      quick_stats: video=%d audio=%d short=%d\n",
        has_video, has_audio, short_video);
}

// This is the one place the *decoder* runs: it decodes a real theora keyframe
// out of the recorded attachment. It is also what DWYCO_NO_VIDEO_MSGS used to
// compile away entirely.
static void
view_create_preview_decodes_a_frame(void)
{
    const char *buf = 0;
    int len = 0, cols = 0, rows = 0;

    if (!g_viewid) {
        printf("[skip] no view\n");
        return;
    }

    int rc = dwyco_zap_create_preview_buf(g_viewid, &buf, &len, &cols, &rows);
    CHECK(rc != 0);
    if (!rc) {
        printf("[note] no preview decoded -- the recorded clip may have been"
            " too short to contain a keyframe\n");
        return;
    }
    CHECK(buf != 0);
    CHECK(cols > 0);
    CHECK(rows > 0);
    CHECK(len == cols * rows * 3);
    printf("\n      preview decoded: %dx%d (%d bytes)\n", cols, rows, len);

    // The buffer is really a PPM -- an array of row pointers -- despite being
    // called a "buf". dwyco_free_array is `delete []` [dlli.cpp:1350] and would
    // mismatch-malloc against it; dwyco_free_image is the ppm_freearray one.
    if (buf)
        dwyco_free_image((char *)buf, rows);
}

static void
view_play_and_stop(void)
{
    int chan_id = 0;
    int calls_before = g_play_dcb_calls;

    if (!g_viewid) {
        printf("[skip] no view\n");
        return;
    }

    // The no-audio variant, because there is no audio output device here.
    CHECK(dwyco_zap_play_view_no_audio(g_viewid, play_dcb, 0, &chan_id) != 0);
    CHECK(chan_id > 0);
    service_ms(400);

    CHECK(dwyco_zap_stop_view(g_viewid) != 0);
    service_ms(200);
    CHECK(g_play_dcb_calls > calls_before);

    CHECK(dwyco_delete_zap_view(g_viewid) != 0);
    g_viewid = 0;
}

// ===== bogus ids and other error paths =====

// Every view accessor must reject a bad cookie rather than dereference it.
static void
view_accessors_reject_bad_ids(void)
{
    int hv = -1, ha = -1, sv = -1;
    int chan_id = 0;
    const char *buf = 0;
    int blen = 0, cols = 0, rows = 0;

    CHECK(dwyco_zap_quick_stats_view(-1, &hv, &ha, &sv) == 0);
    CHECK(dwyco_zap_stop_view(-1) == 0);
    CHECK(dwyco_delete_zap_view(-1) == 0);
    CHECK(dwyco_zap_play_view_no_audio(-1, play_dcb, 0, &chan_id) == 0);
    CHECK(dwyco_zap_play_view(-1, play_dcb, 0, &chan_id) == 0);
    CHECK(dwyco_zap_play_preview(-1, play_dcb, 0, &chan_id) == 0);

    // The preview entry points also touch the filesystem on success, so make
    // sure the failure path never gets that far.
    CHECK(dwyco_zap_create_preview_buf(-1, &buf, &blen, &cols, &rows) == 0);
    CHECK(buf == 0);
    CHECK(blen == 0 && cols == 0 && rows == 0);
    CHECK(dwyco_zap_create_preview(-1, "/tmp/dwytest_zap_should_not_exist.ppm",
        34) == 0);
    unlink("/tmp/dwytest_zap_should_not_exist.ppm");
}

// A view cookie is single-use: once deleted, every accessor must refuse it.
static void
deleted_view_id_is_rejected(void)
{
    int viewid = dwyco_make_zap_view_file_raw(FILE_ZAP_SRC);
    CHECK(viewid != 0);
    CHECK(viewid != 0x55555555);
    if (!viewid) return;

    CHECK(dwyco_delete_zap_view(viewid) != 0);
    // Second delete, and every accessor, must all refuse.
    CHECK(dwyco_delete_zap_view(viewid) == 0);
    CHECK(dwyco_zap_quick_stats_view(viewid, 0, 0, 0) == 0);
    CHECK(dwyco_zap_stop_view(viewid) == 0);
    int chan_id = 0;
    CHECK(dwyco_zap_play_view_no_audio(viewid, play_dcb, 0, &chan_id) == 0);
}

// ===== file zaps =====
//
// A file zap is an ordinary file from the filesystem, copied in and renamed to
// a random <hex>.fle, with the original basename remembered in the message
// body. See import_file() [dlli.cpp:4853] and make_file_zap_composition()
// [dlli.cpp:5220].
//
// There is deliberately nothing here about the internals of that container. The
// test only relies on the public behaviour: a file on disk goes in, the bytes
// come back out, and the original name is preserved in the body.

// Locate the self-sent file zap. Same mechanism as the media zap, matched on
// its own body text so the two coexist in one client dir.
static void
find_self_sent_file_zap(void)
{
    if (!find_mid_by_text(FILE_ZAP_TEXT, g_file_mid, (int)sizeof(g_file_mid),
        &g_file_mid_len)) {
        printf("[FAIL] could not locate the self-sent file zap via the"
            " message index\n");
        g_fail++;
        return;
    }
    printf("\n      found self-sent file zap mid=%s\n", g_file_mid);
}

// The source file. Deliberately small and with non-text bytes including an
// embedded NUL and a stray 0xff/0xfe, so a text-mode mistake shows up as a
// hash mismatch. Named with a directory component so the basename comparison
// below is actually meaningful.
static void
write_file_zap_source(void)
{
    FILE *f = fopen(FILE_ZAP_SRC, "wb");
    if (!f) {
        printf("[FAIL] could not create %s\n", FILE_ZAP_SRC);
        g_fail++;
        return;
    }
    static const unsigned char body[] = {
        'd', 'w', 'y', 't', 'e', 's', 't', 0x00, 0x01,
        (unsigned char)0xff, (unsigned char)0xfe, '\n', 'e', 'n', 'd'
    };
    fwrite(body, 1, sizeof(body), f);
    fclose(f);
}

// A file zap is refused a recording, and refused no-forward sends.
//
// The record refusal is the interesting half: a plain composition accepts
// dwyco_zap_record, a file composition must not, because you cannot record new
// media into a file you are attaching. That guard lives at dlli.cpp:5522 and
// keys off user_filename being set.
static void
file_zap_composition_refuses_record(void)
{
    int compid = dwyco_make_file_zap_composition(FILE_ZAP_SRC,
        (int)strlen(FILE_ZAP_SRC));
    CHECK(compid != 0);
    if (!compid) {
        printf("[FAIL] dwyco_make_file_zap_composition refused a readable file\n");
        return;
    }

    // It IS a file zap.
    CHECK(dwyco_is_file_zap(compid) != 0);
    // Nothing is recording, so there is no recording channel.
    CHECK(dwyco_zap_composition_chan_id(compid) == -1);

    // Recording into it is refused, unlike a plain composition.
    int chan_id = 12345;
    CHECK(dwyco_zap_record(compid, 1, 0, 0, 1, record_dcb, 0, &chan_id) == 0);
    CHECK(dwyco_zap_record2(compid, 1, 0, 10, 1000, 0, 0, 0, record_dcb, 0,
        &chan_id) == 0);
    CHECK(dwyco_zap_composition_chan_id(compid) == -1);

    CHECK(dwyco_delete_zap_composition(compid) != 0);
}

// An unreadable path is refused before anything is copied.
static void
file_zap_composition_rejects_bad_path(void)
{
    CHECK(dwyco_make_file_zap_composition("/tmp/dwytest_zap_no_such_file",
        29) == 0);
    CHECK(dwyco_make_file_zap_composition("", 0) == 0);
}

// Sending a file zap with no_forward is refused: the flag cannot be honoured
// for a file attachment [dlli.cpp:5661]. Sending it normally is fine.
static void
file_zap_no_forward_send_is_refused(void)
{
    int compid = dwyco_make_file_zap_composition(FILE_ZAP_SRC,
        (int)strlen(FILE_ZAP_SRC));
    const char *pers = 0;
    int pers_len = 0;
    CHECK(compid != 0);
    if (!compid) return;

    g_send_done = 0;
    g_send_was_fail = 0;
    int rc = dwyco_zap_send6(compid, g_my_uid, g_my_uid_len, FILE_ZAP_TEXT,
        (int)strlen(FILE_ZAP_TEXT), /*no_forward*/1, /*save_sent*/0,
        /*defer*/0, &pers, &pers_len);
    CHECK(rc == 0);

    // Same composition, no_forward cleared: this is the one that goes out.
    rc = dwyco_zap_send6(compid, g_my_uid, g_my_uid_len, FILE_ZAP_TEXT,
        (int)strlen(FILE_ZAP_TEXT), /*no_forward*/0, /*save_sent*/0,
        /*defer*/0, &pers, &pers_len);
    CHECK(rc != 0);
    if (!rc) {
        dwyco_delete_zap_composition(compid);
        return;
    }

    if (!wait_for([&]() { return g_send_done != 0; }, 60000)) {
        printf("[FAIL] no send completion for the file zap within 60s\n");
        g_fail++;
    } else {
        CHECK(!g_send_was_fail);
    }
    service_ms(500);
    dwyco_delete_zap_composition(compid);
}

// The stored body is the mirror image of a media zap: the file attachment name
// is set where a media zap has nil, and the attachment itself was renamed to
// <random>.fle. The original basename must survive untouched.
static void
file_zap_body_preserves_original_name(void)
{
    DWYCO_SAVED_MSG_LIST sm = 0;

    if (!g_file_mid_len) {
        printf("[skip] no file zap message located\n");
        return;
    }
    if (dwyco_get_saved_message3(&sm, g_file_mid) != DWYCO_GSM_SUCCESS || !sm) {
        printf("[FAIL] could not reopen the file zap message\n");
        g_fail++;
        return;
    }

    const char *v = 0;
    int vl = 0, vt = 0;

    // The attachment got renamed by import_file to a random name ending .fle.
    CHECK(dwyco_list_get(sm, 0, DWYCO_QM_BODY_ATTACHMENT, &v, &vl, &vt) != 0);
    CHECK(vt == DWYCO_TYPE_STRING);
    CHECK(vl > 5);
    if (vt == DWYCO_TYPE_STRING && vl > 5)
        CHECK(memcmp(v + vl - 4, ".fle", 4) == 0);
    printf("\n      attachment renamed to: %.*s\n", vl, v ? v : "");

    // ...but the name the user gave us is preserved exactly, as the basename.
    v = 0;
    CHECK(dwyco_list_get(sm, 0, DWYCO_QM_BODY_FILE_ATTACHMENT, &v, &vl,
        &vt) != 0);
    CHECK(vt == DWYCO_TYPE_STRING);
    CHECK(vl == (int)strlen(FILE_ZAP_BASENAME));
    if (vt == DWYCO_TYPE_STRING)
        CHECK(v && memcmp(v, FILE_ZAP_BASENAME, (size_t)vl) == 0);
    printf("      original name preserved as: %.*s\n", vl, v ? v : "");

    dwyco_list_release(sm);
}

// The index flags the message as a file and, crucially, does NOT claim it has
// video: the media probe is skipped for file attachments [qmsgsql.cpp:2532].
static void
file_zap_index_flags_it_as_a_file(void)
{
    DWYCO_MSG_IDX idx = 0;

    if (!g_file_mid_len) {
        printf("[skip] no file zap message located\n");
        return;
    }
    if (!dwyco_get_message_index(&idx, g_my_uid, g_my_uid_len)) {
        printf("[FAIL] dwyco_get_message_index failed\n");
        g_fail++;
        return;
    }

    int rows = 0, cols = 0;
    dwyco_list_numelems(idx, &rows, &cols);

    int found = 0;
    for (int i = 0; i < rows && !found; ++i) {
        const char *mid = 0;
        int mid_len = 0, type = 0;
        if (!dwyco_list_get(idx, i, DWYCO_MSG_IDX_MID, &mid, &mid_len, &type))
            continue;
        if (type != DWYCO_TYPE_STRING || mid_len != g_file_mid_len)
            continue;
        if (memcmp(mid, g_file_mid, (size_t)mid_len) != 0)
            continue;

        found = 1;
        // is_file is set for a file attachment and nil for a media zap.
        const char *v = 0;
        int vl = 0, vt = 0;
        CHECK(dwyco_list_get(idx, i, DWYCO_MSG_IDX_IS_FILE, &v, &vl, &vt) != 0);
        CHECK(vt != DWYCO_TYPE_NIL);
        // No video: it is a file, not a recorded .dyc.
        v = 0;
        CHECK(dwyco_list_get(idx, i, DWYCO_MSG_IDX_ATT_HAS_VIDEO, &v, &vl, &vt) != 0);
        CHECK(vt == DWYCO_TYPE_NIL);
    }
    dwyco_list_release(idx);

    if (!found) {
        printf("[FAIL] file zap mid %s not present in the message index\n",
            g_file_mid);
        g_fail++;
    }
}

// A file attachment cannot be viewed as a zap: make_zap_view2 refuses it
// outright [dlli.cpp:5910]. The bytes come out through the copy-out api
// instead, and they must be identical to what we put in.
static void
file_zap_copies_out_byte_identical(void)
{
    DWYCO_SAVED_MSG_LIST sm = 0;

    if (!g_file_mid_len) {
        printf("[skip] no file zap message located\n");
        return;
    }

    // The view entry point is the wrong one and says so.
    if (dwyco_get_saved_message3(&sm, g_file_mid) == DWYCO_GSM_SUCCESS && sm) {
        int viewid = dwyco_make_zap_view2(sm, 0);
        CHECK(viewid == 0);
        if (viewid) {
            printf("[FAIL] make_zap_view2 accepted a file attachment (%d),"
                " expected refusal\n", viewid);
            dwyco_delete_zap_view(viewid);
        }
        dwyco_list_release(sm);
    }

    long src_size = 0;
    CHECK(file_hash64(FILE_ZAP_SRC, &src_size) != 0);
    CHECK(src_size > 0);

    // To a path.
    unlink(FILE_ZAP_OUT);
    int rc = dwyco_copy_out_file_zap2(g_file_mid, FILE_ZAP_OUT);
    CHECK(rc != 0);
    if (rc) {
        long out_size = 0;
        unsigned long long h = file_hash64(FILE_ZAP_OUT, &out_size);
        CHECK(h != 0);
        CHECK(h == file_hash64(FILE_ZAP_SRC, NULL));
        CHECK(out_size == src_size);
        unlink(FILE_ZAP_OUT);
    }

    // To a buffer. 'max' caps the copy, so a too-small max must decline
    // rather than truncate.
    const char *buf = "";
    int blen = 0;
    rc = dwyco_copy_out_file_zap_buf2(g_file_mid, &buf, &blen,
        (int)src_size + 1024);
    CHECK(rc != 0);
    CHECK(buf != 0);
    if (rc && buf) {
        CHECK(blen == (int)src_size);
        // Compare against the file's CONTENTS, not FILE_ZAP_SRC itself, which
        // is a path string.
        std::vector<unsigned char> want((size_t)src_size);
        FILE *sf = fopen(FILE_ZAP_SRC, "rb");
        size_t got = sf ? fread(&want[0], 1, want.size(), sf) : 0;
        if (sf) fclose(sf);
        CHECK(got == want.size());
        CHECK(blen == (int)want.size());
        if (blen == (int)want.size())
            CHECK(memcmp(buf, &want[0], want.size()) == 0);
        // Allocated with new char[] by the library, so this is the right free.
        dwyco_free_array((char *)buf);
    }

    // DEFECT: a max too small to hold the file does not decline, it TRUNCATES.
    // The implementation does `if(sz > max) sz = max` [dlli.cpp:5421-5422] and
    // then reports success with the short length, so a caller checking only the
    // return value would silently store a partial file. dwytest_attach.cpp
    // hedges on exactly this with `rc2 == 0 || blen > 0`.
    //
    // Pinned as the truncation it is. If this ever starts returning 0, the
    // truncation was fixed and this test should be tightened.
    buf = "";
    blen = 0;
    int rc2 = dwyco_copy_out_file_zap_buf2(g_file_mid, &buf, &blen, 1);
    CHECK(rc2 != 0);
    CHECK(blen == 1);
    if (buf)
        dwyco_free_array((char *)buf);
}

// copy_out_qd_file_zap is the outbox counterpart and expects a body obtained
// via dwyco_qd_message_to_body. Handing it a saved message is off-label, and
// its CopyFile(newfn(attachment)) omits the <uid>.usr directory component
// [dlli.cpp:5280], so it cannot find the attachment.
//
// Asserted only as "does not crash and reports failure": the off-label path
// is not something to pin a specific value on.
static void
qd_copy_out_on_saved_message_is_rejected(void)
{
    DWYCO_SAVED_MSG_LIST sm = 0;

    if (!g_file_mid_len) {
        printf("[skip] no file zap message located\n");
        return;
    }
    if (dwyco_get_saved_message3(&sm, g_file_mid) != DWYCO_GSM_SUCCESS || !sm) {
        printf("[skip] could not reopen the message\n");
        return;
    }

    unlink(FILE_ZAP_OUT);
    int rc = dwyco_copy_out_qd_file_zap(sm, FILE_ZAP_OUT);
    CHECK(rc == 0);
    printf("\n      qd copy-out on a saved message: rc=%d\n", rc);
    unlink(FILE_ZAP_OUT);

    dwyco_list_release(sm);
}

// A view can still be built from the local file path directly. The _raw form
// takes the path as given and works on any file; the plain form goes through
// newfn(), which oopanic()s (and so exits) on any suffix that is not one of
// dwyco's own registered types. dwytest_attach.cpp already pins that
// process-killing defect out-of-process -- see its make_zap_view_file section.
//
// make_zap_view_file looks like an internal api: it is called from the profile
// machinery, and as a public entry point its filename mapping means it cannot
// be used for anything but dwyco's own container types. It would be reasonable
// to take it out of the public header and leave _raw as the supported form.
static void
view_from_local_file_raw(void)
{
    int viewid = dwyco_make_zap_view_file_raw(FILE_ZAP_SRC);
    CHECK(viewid != 0);
    CHECK(viewid != 0x55555555);
    if (!viewid) return;

    // A valid cookie, but nothing decodable inside, so the probe declines and
    // zeroes its outputs rather than reporting media that is not there.
    int hv = -1, ha = -1, sv = -1;
    CHECK(dwyco_zap_quick_stats_view(viewid, &hv, &ha, &sv) == 0);
    CHECK(hv == 0 && ha == 0 && sv == 0);

    // No playable media either.
    const char *buf = 0;
    int blen = 0, cols = 0, rows = 0;
    CHECK(dwyco_zap_create_preview_buf(viewid, &buf, &blen, &cols, &rows) == 0);
    CHECK(buf == 0);

    // Stopping something that is not playing is a no-op, not an error.
    CHECK(dwyco_zap_stop_view(viewid) == 0);
    CHECK(dwyco_delete_zap_view(viewid) != 0);
}

// DEFECT: dwyco_zap_play accepts a file zap composition and reports success.
//
// Record is refused correctly (dlli.cpp:5522) but play is not:
//   * FormShow() sees a non-empty actual_filename and calls stop_buttonClick(),
//     which sets play_button_enabled = 1 [mcc.cpp:536, 898]
//   * play_buttonClick only bails on an empty name or ".jpg" [mcc.cpp:545-547],
//     so a .fle sails straight past
//   * quick_stats on non-container bytes finds nothing, leaving codec 0
//   * build_incoming_video() fails and the return value is DISCARDED
//     [mcc.cpp:597]
//
// The result is a dead channel reported as a successful play. Pinned so that
// fixing it has to update this test.
static void
file_zap_play_defect_returns_success(void)
{
    int compid = dwyco_make_file_zap_composition(FILE_ZAP_SRC,
        (int)strlen(FILE_ZAP_SRC));
    int chan_id = 0;
    CHECK(compid != 0);
    if (!compid) return;

    g_play_dcb_calls = 0;
    raw_rec_reset();

    int rc = dwyco_zap_play(compid, play_dcb, 0, &chan_id);
    printf("\n      dwyco_zap_play on a file zap: rc=%d chan=%d\n", rc, chan_id);
    // Reports success. If this ever starts returning 0, the defect is fixed and
    // this test must be updated.
    CHECK(rc != 0);
    CHECK(chan_id > 0);

    service_ms(400);

    // But nothing was ever decoded, so no frames appeared.
    CHECK(raw_rec.size() == 0);

    // The channel is stoppable and the composition deletable, so the test does
    // not wedge.
    CHECK(dwyco_zap_stop(compid) != 0);
    service_ms(200);
    CHECK(dwyco_delete_zap_composition(compid) != 0);
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
    // Before dwyco_init, like the other message tests do.
    dwyco_set_system_event_callback(system_event);
    dwyco_set_client_version("dwytest", 7);
    if (dwyco_init() == 0)
        return 0;
    test_bootstrap_profile("dwytest-zap", "dwytest zap account");
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);

    const char *uid = 0;
    int len = 0;
    dwyco_get_my_uid(&uid, &len);
    if (len <= 0 || len >= (int)sizeof(g_my_uid)) {
        fprintf(stderr, "no local uid\n");
        return 0;
    }
    memcpy(g_my_uid, uid, (size_t)len);
    g_my_uid_len = len;
    return 1;
}

int
main(void)
{
    setvbuf(stdout, 0, _IOLBF, 0);
    printf("Dwyco zap composition / record / play / send / view\n");

    g_file_list = raw_ensure_file_list();
    if (!g_file_list) {
        fprintf(stderr, "no usable raw file list\n");
        return 1;
    }
    printf("  file list: %s\n", g_file_list);

    // The file zap source lives in a subdirectory (see FILE_ZAP_SRC), so make
    // sure it exists before the tests that need it.
    mkdir("/tmp/dwytest_zap_src", 0755);
    write_file_zap_source();

    if (!init_test()) {
        fprintf(stderr, "dwyco_init failed\n");
        return 1;
    }

    // Video capture has to come from the synthetic frames, not a camera.
    dwyco_set_setting("video_input/source", "raw");
    dwyco_set_setting("raw_files/raw_files_list", g_file_list);
    dwyco_set_setting("raw_files/use_pattern", "0");
    dwyco_set_setting("raw_files/preload", "0");
    dwyco_set_setting("video_input/no_video", "0");

    dwyco_set_video_display_callback(raw_video_display);

    printf("\nComposition:\n");
    RUN(make_composition_accepts_null_dummy);
    RUN(bogus_composition_ids_are_rejected);
    RUN(record_lies_when_nothing_was_requested);
    RUN(stop_and_play_rejected_when_idle);

    printf("\nRecord / stop:\n");
    RUN(record_then_stop_captures_frames);
    RUN(record_twice_is_rejected);

    printf("\nPlayback:\n");
    RUN(play_recorded_zap);
    RUN(record2_bounds_the_recording);

    printf("\nSend to self:\n");
    RUN(send_zap_to_self);
    RUN(find_self_sent_message);

    printf("\nSaved message:\n");
    RUN(self_sent_body_is_well_formed);

    printf("\nView:\n");
    RUN(make_view_from_saved_message);
    RUN(view_quick_stats);
    RUN(view_create_preview_decodes_a_frame);
    RUN(view_play_and_stop);
    RUN(view_accessors_reject_bad_ids);
    RUN(deleted_view_id_is_rejected);

    printf("\nFile zaps - composition:\n");
    RUN(file_zap_composition_rejects_bad_path);
    RUN(file_zap_composition_refuses_record);
    RUN(file_zap_no_forward_send_is_refused);

    printf("\nFile zaps - message:\n");
    RUN(find_self_sent_file_zap);
    RUN(file_zap_body_preserves_original_name);
    RUN(file_zap_index_flags_it_as_a_file);
    RUN(file_zap_copies_out_byte_identical);
    RUN(qd_copy_out_on_saved_message_is_rejected);

    printf("\nFile zaps - views and errors:\n");
    RUN(view_from_local_file_raw);
    RUN(file_zap_play_defect_returns_success);

    if (g_last_compid)
        dwyco_delete_zap_composition(g_last_compid);

    dwyco_exit();

    printf("\nSummary: passed=%d failed=%d\n", g_pass, g_fail);
    if (g_fail) {
        printf("FAILED\n");
        return 1;
    }
    printf("All zap api tests passed.\n");
    return 0;
}