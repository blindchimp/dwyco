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

// Find the message we just sent. Must use the message index rather than
// dwyco_get_tagged_idx("_inbox"), because a self-send is stored, never
// received, so it has no _inbox tag.
static void
find_self_sent_message(void)
{
    DWYCO_MSG_IDX idx = 0;

    if (g_zap_mid_len) {           // already found
        CHECK(g_zap_mid_len > 0);
        return;
    }

    if (!dwyco_get_message_index(&idx, g_my_uid, g_my_uid_len)) {
        printf("[FAIL] dwyco_get_message_index failed for our own uid\n");
        g_fail++;
        return;
    }

    int rows = 0, cols = 0;
    CHECK(dwyco_list_numelems(idx, &rows, &cols) != 0);
    CHECK(rows > 0);
    if (rows <= 0) {
        dwyco_list_release(idx);
        return;
    }

    // Newest first: the index is ordered by descending logical clock. Find the
    // one carrying our text rather than blindly taking row 0, so a leftover
    // message in a reused client dir cannot fool us.
    for (int i = 0; i < rows; ++i) {
        const char *mid = 0;
        int mid_len = 0, type = 0;
        if (!dwyco_list_get(idx, i, DWYCO_MSG_IDX_MID, &mid, &mid_len, &type))
            continue;
        if (type != DWYCO_TYPE_STRING || mid_len <= 0)
            continue;
        char midbuf[256];
        int n = mid_len < (int)sizeof(midbuf) - 1 ? mid_len : (int)sizeof(midbuf) - 1;
        memcpy(midbuf, mid, (size_t)n);
        midbuf[n] = 0;

        DWYCO_SAVED_MSG_LIST sm = 0;
        if (dwyco_get_saved_message3(&sm, midbuf) != DWYCO_GSM_SUCCESS)
            continue;
        // dwyco_get_body_array returns one row per forwarded component, with the
        // top-level body at row 0. lr_rows() *asserts* a count, so ask for the
        // real one first rather than guessing.
        DWYCO_LIST ba = dwyco_get_body_array(sm);
        int ba_rows = -1, ba_cols = -1;
        int matched = 0;
        if (ba && dwyco_list_numelems(ba, &ba_rows, &ba_cols) != 0
            && ba_rows >= 1)
            matched = lr_col_str(ba, 0, DWYCO_QM_BODY_NEW_TEXT2, ZAP_TEXT,
                (int)strlen(ZAP_TEXT));
        if (ba)
            dwyco_list_release(ba);
        if (matched) {
            memcpy(g_zap_mid, midbuf, (size_t)n + 1);
            g_zap_mid_len = n;
            dwyco_list_release(sm);
            break;
        }
        dwyco_list_release(sm);
    }

    dwyco_list_release(idx);

    if (!g_zap_mid_len) {
        printf("[FAIL] could not find a saved message with our text via the"
            " message index (%d rows scanned)\n", rows);
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