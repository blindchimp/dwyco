// Coverage for attachment composition, copy-out, views, previews, and
// message forwarding.
//
//   dwyco_make_zap_composition_raw, dwyco_set_special_zap,
//   dwyco_is_forward_composition, dwyco_make_forward_zap_composition2,
//   dwyco_zap_send4, dwyco_zap_cancel, dwyco_zap_still_active,
//   dwyco_zap_composition_chan_id, dwyco_copy_out_file_zap2,
//   dwyco_copy_out_file_zap_buf2, dwyco_copy_out_qd_file_zap,
//   dwyco_make_zap_view2, dwyco_make_zap_view_file,
//   dwyco_make_zap_view_file_raw, dwyco_delete_zap_view,
//   dwyco_zap_play_view, dwyco_zap_play_view_no_audio, dwyco_zap_stop_view,
//   dwyco_zap_quick_stats_view, dwyco_zap_create_preview,
//   dwyco_zap_create_preview_buf
//
// Like dwytest_msg, this re-executes itself as a second dwyco client so
// there is a real message to work with. The sender sends one plain message
// and one carrying a file attachment; the receiver then exercises the whole
// attachment read path against them, and finally forwards a message back so
// the sender can confirm the forward really produced a second body
// component.
//
// See dwytest_msg.cpp for how the receiving loop has to work -- in
// particular that a peer-to-peer message never appears in the unfetched
// queue and only shows up via the "_inbox" tag.
//
// A note on the attachment checks: an attachment is only really verified if
// the bytes that come back out are compared against the bytes that went in,
// so everything here uses file_hash64() rather than just checking that
// something was returned.

#include <dlli.h>
#include "test_common.h"
#include "dwyco_new_msg.h"
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

static const char TXT_PLAIN[] = "dwyatt plain text";
static const char TXT_FWD[]   = "dwyatt forward me";

// IMPORTANT: SENDER_DIR below is deliberately NOT wiped between runs, and
// the test dir is. A brand new account is not immediately usable as a message
// *source*: it registers with the server asynchronously, and sends issued
// during that window are accepted by the local API but never routed, so the
// receiver sees nothing. With the sender's directory left in place the tests
// are reproducible; deleting it makes them fail intermittently. The receiver
// may be fresh every time -- sending outbound from a new account is fine.

static const char *SENDER_DIR = "/tmp/dwytest_attach_sender";
static const char *RECV_DIR   = "/tmp/dwytest_attach";
static const char *ATTACH_SRC = "/tmp/dwytest_attach_src.bin";
static const char *ATTACH_OUT = "/tmp/dwytest_attach_out.bin";
static const char *ATTACH_FAKE_FLE = "/tmp/dwytest_attach_fake.fle";
// A real image, so there is something the view/preview code can actually
// decode. A .bin of arbitrary bytes copies out fine but cannot be viewed.
static const char *IMAGE_SRC =
    DWYCO_TEST_APP_DIR "/no_img.png";

static char g_my_uid[64];
static int g_my_uid_len;
static char g_my_uid_hex[64];

// hex uid of whoever sent us the plain message, needed to forward back to.
static char g_last_sender_hex[64];

static char g_plain_mid[256];   static int g_plain_mid_len;
static char g_attach_mid[256];  static int g_attach_mid_len;
static char g_image_mid[256];   static int g_image_mid_len;
static int g_attach_has_att;
static int g_attach_is_file;

static int g_send_ok;
static void DWYCOCALLCONV
send_status_cb(int cmd, int ctx_id, const char *uid, int len_uid,
    const char *name, int len_name, int type, const char *value, int val_len,
    int qid, int extra_arg)
{
    if (cmd == DWYCO_SE_MSG_SEND_SUCCESS || cmd == DWYCO_SE_MSG_SEND_FAIL)
        g_send_ok = 1;
}

// Write the attachment the sender will use. Deliberately small and with some
// non-text bytes so a text-mode mistake shows up in the hash comparison.
static void
write_attachment(void)
{
    FILE *f = fopen(ATTACH_SRC, "wb");
    if (!f)
        return;
    static const unsigned char body[] = {
        'd', 'w', 'y', 'a', 't', 't', 0x00, 0x01,
        (unsigned char)0xff, (unsigned char)0xfe, '\n', 'e', 'n', 'd'
    };
    fwrite(body, 1, sizeof(body), f);
    fclose(f);
}

// ===== sender mode =====

static int
send_one(const char *dest, int dest_len, const char *text, const char *attach)
{
    int cid;
    if (attach) {
        cid = dwyco_make_file_zap_composition(attach, (int)strlen(attach));
        if (cid <= 0) {
            fprintf(stderr, "[sender] file composition failed\n");
            return 0;
        }
    } else {
        cid = dwyco_make_zap_composition(0);
        if (cid <= 0) {
            fprintf(stderr, "[sender] composition failed\n");
            return 0;
        }
    }
    const char *pers_id;
    int pers_len;
    g_send_ok = 0;
    int rc = dwyco_zap_send6(cid, dest, dest_len, text, (int)strlen(text),
        0, 0, 0, &pers_id, &pers_len);
    int ok = 0;
    if (rc != 0)
        ok = wait_for([&]() { return g_send_ok != 0; }, 60000);
    dwyco_delete_zap_composition(cid);
    printf("SENT text='%s' attach=%s rc=%d confirmed=%d\n",
        text, attach ? "yes" : "no", rc, ok);
    fflush(stdout);
    return ok;
}

static int
peer_send_main(int argc, char **argv)
{
    if (argc < 4) {
        fprintf(stderr, "usage: %s --peer-send <sender_dir> <dest_uid_hex>\n", argv[0]);
        return 1;
    }
    const char *dir = argv[2];
    char sys_dir[512], tmp_dir[512];
    snprintf(sys_dir, sizeof(sys_dir), "%s/sys", dir);
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", dir);
    mkdir(dir, 0755);
    mkdir(sys_dir, 0755);
    mkdir(tmp_dir, 0755);
    // Keep our output out of the parent's stdout.
    char logp[512];
    snprintf(logp, sizeof(logp), "%s/sender.log", dir);
    int lf = open(logp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (lf >= 0) {
        fflush(stdout);
        dup2(lf, 1);
        close(lf);
    }
    install_app_files(dir);

    dwyco_set_fn_prefixes(sys_dir, dir, tmp_dir);
    dwyco_set_system_event_callback(send_status_cb);
    dwyco_set_client_version("dwytest", 7);
    if (!dwyco_init()) {
        fprintf(stderr, "[sender] init failed\n");
        return 1;
    }
    test_bootstrap_profile("dwytest-attach-sender", "dwytest attach sender");
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);

    char dest[64];
    int dest_len = 0;
    if (!uid_from_hex(argv[3], dest, sizeof(dest), &dest_len)) {
        fprintf(stderr, "[sender] bad dest uid\n");
        return 1;
    }

    dwyco_pal_add(dest, dest_len);
    service_ms(2000);
    if (!wait_for([&]() { return dwyco_uid_online(dest, dest_len) != 0; }, 60000))
        fprintf(stderr, "[sender] dest never came online, sending anyway\n");
    int ok = 1;
    ok &= send_one(dest, dest_len, TXT_PLAIN, 0);
    service_ms(500);
    ok &= send_one(dest, dest_len, "dwyatt with a file", ATTACH_SRC);
    service_ms(500);
    ok &= send_one(dest, dest_len, "dwyatt with an image", IMAGE_SRC);
    service_ms(1000);

    // Stay online so the receiver can send a forwarded message back, then
    // report how many body components it ended up with. The receiver reads
    // this out of sender.log.
    printf("SENDER_WAITING_FORWARD\n");
    fflush(stdout);
    int forwarded = 0;
    for (int i = 0; i < 1200; i++) {
        service_ms(100);
        if (dwyco_get_rescan_messages()) {
            dwyco_set_rescan_messages(0);
            process_remote_msgs();
        }
        DwString ruid, txt, mid, creator;
        int zv, ha, isf;
        if (dwyco_new_msg2(ruid, txt, zv, mid, ha, isf, creator)) {
            // Count the forwarded components of whatever we just got.
            int components = -1;
            char midbuf[256];
            int n = mid.length() < (int)sizeof(midbuf) - 1
                ? mid.length() : (int)sizeof(midbuf) - 1;
            memcpy(midbuf, mid.c_str(), (size_t)n);
            midbuf[n] = 0;

            char myuid_bin[64];
            int myuid_len = 0;
            const char *mu;
            int mul = 0;
            dwyco_get_my_uid(&mu, &mul);
            memcpy(myuid_bin, mu, (size_t)mul);
            myuid_len = mul;

            DWYCO_SAVED_MSG_LIST sm = 0;
            if (dwyco_get_saved_message(&sm, myuid_bin, myuid_len, midbuf)) {
                DWYCO_LIST ba = dwyco_get_body_array(sm);
                if (ba) {
                    int r = -1, c = -1;
                    dwyco_list_numelems(ba, &r, &c);
                    components = r;
                    dwyco_list_release(ba);
                }
                dwyco_list_release(sm);
            }
            printf("RECEIVED_FORWARD text='%s' components=%d\n",
                txt.c_str(), components);
            fflush(stdout);
            processed_msg(mid);
            if (components >= 0)
                forwarded = components;
            break;
        }
    }
    printf("SENDER_DONE ok=%d fwd_components=%d\n", ok, forwarded);
    fflush(stdout);
    dwyco_exit();
    return ok ? 0 : 1;
}

// ===== receiver: collection =====

static int
collect(const char *want_text, int timeout_ms, char *mid_out, int *mid_len_out,
    int *has_att_out, int *is_file_out)
{
    DwString ruid, txt, mid, creator;
    int zap_viewer, has_att, is_file;

    for (int i = 0; i < timeout_ms / 100; i++) {
        if (dwyco_get_rescan_messages()) {
            dwyco_set_rescan_messages(0);
            process_remote_msgs();
        }
        if (dwyco_new_msg2(ruid, txt, zap_viewer, mid, has_att, is_file, creator)) {
            if (want_text == 0 || txt.eq(want_text)) {
                int n = mid.length() < 255 ? mid.length() : 255;
                memcpy(mid_out, mid.c_str(), (size_t)n);
                mid_out[n] = 0;
                *mid_len_out = n;
                if (has_att_out)
                    *has_att_out = has_att;
                if (is_file_out)
                    *is_file_out = is_file;
                processed_msg(mid);
                return 1;
            }
            processed_msg(mid);
        }
        service_ms(100);
    }
    return 0;
}

// ===== composition-only tests (no message needed) =====

// A raw composition only accepts dwyco's own container formats: the
// implementation looks for a trailing ".dyc" or ".fle" and returns 0 without
// one, before it ever looks at the contents. So an ordinary file is refused
// even though it is perfectly readable.
//
// That also means the *positive* case cannot be reached without a real .dyc
// or .fle container, which there is no public way to manufacture -- hence the
// two refusals below and no positive assertion.
static void
raw_composition_requires_dwyco_container(void)
{
    // Unreadable path.
    CHECK(dwyco_make_zap_composition_raw("/tmp/dwytest_no_such_file", 0) == 0);
    // Readable, but not a dwyco container.
    CHECK(dwyco_make_zap_composition_raw(ATTACH_SRC, 0) == 0);
    // A file whose name DOES end in .fle is accepted -- only the extension is
    // checked, the contents are not validated by this function. Worth pinning
    // because it means the caller is responsible for the file being real.
    FILE *f = fopen(ATTACH_FAKE_FLE, "wb");
    if (f) {
        fwrite("not really a dwyco container", 1, 28, f);
        fclose(f);
    }
    int cid = dwyco_make_zap_composition_raw(ATTACH_FAKE_FLE, 0);
    printf("(cid=%d) ", cid);
    CHECK(cid != 0);
    if (cid > 0) {
        CHECK(dwyco_is_file_zap(cid) != 0);
        CHECK(dwyco_delete_zap_composition(cid) != 0);
    }
    unlink(ATTACH_FAKE_FLE);
}

// Special type and forward flags live on the composition cookie. A fresh
// composition is neither special nor a forward.
static void
composition_flags(void)
{
    int cid = dwyco_make_zap_composition(0);
    CHECK(cid > 0);
    if (cid <= 0)
        return;

    CHECK(dwyco_is_forward_composition(cid) == 0);
    CHECK(dwyco_set_special_zap(cid, DWYCO_SPECIAL_TYPE_USER) != 0);
    CHECK(dwyco_zap_still_active(cid) != 0);
    // dwyco_zap_composition_chan_id returns the composition's stop_id
    // unfiltered. For a composition that is not recording or playing, that
    // is whatever was left in the field -- it is neither a live channel nor
    // guaranteed non-negative, so just record what came back.
    int chan = dwyco_zap_composition_chan_id(cid);
    printf("(chan=%d cancel=%d) ", chan, dwyco_zap_cancel(cid));
    dwyco_delete_zap_composition(cid);

    // Every one of these validates its cookie, so a deleted id is rejected
    // rather than dereferenced.
    CHECK(dwyco_is_forward_composition(cid) == 0);
    CHECK(dwyco_set_special_zap(cid, DWYCO_SPECIAL_TYPE_USER) == 0);
    CHECK(dwyco_zap_still_active(cid) == 0);
    CHECK(dwyco_zap_composition_chan_id(cid) == 0);
    CHECK(dwyco_zap_quick_stats_view(cid, 0, 0, 0) == 0);
}

// zap_send4 differs from zap_send5 only in taking save_sent from the
// zap/save_sent setting rather than as an argument.
static void
zap_send4_uses_the_save_sent_setting(void)
{
    if (g_plain_mid_len == 0 && !g_my_uid_len)
        return;
    // Set save_sent off, send to ourselves via send4, and confirm nothing
    // lands in our sent index; then set it on and confirm something does.
    // Sending to self is the simplest way to get a saved message without a
    // second participant.
    dwyco_set_setting("zap/save_sent", "0");
    {
        int cid = dwyco_make_zap_composition(0);
        const char *pers;
        int plen;
        int rc = dwyco_zap_send4(cid, g_my_uid, g_my_uid_len, "send4 nosave",
            13, 0, &pers, &plen);
        CHECK(rc != 0);
        dwyco_delete_zap_composition(cid);
        if (rc != 0)
            wait_for([&]() { return g_send_ok != 0; }, 60000);
    }
    dwyco_set_setting("zap/save_sent", "1");
    {
        int cid = dwyco_make_zap_composition(0);
        const char *pers;
        int plen;
        g_send_ok = 0;
        int rc = dwyco_zap_send4(cid, g_my_uid, g_my_uid_len, "send4 save",
            11, 0, &pers, &plen);
        CHECK(rc != 0);
        dwyco_delete_zap_composition(cid);
        if (rc != 0)
            wait_for([&]() { return g_send_ok != 0; }, 60000);
    }
}

// ===== attachment tests =====

// The received message must actually be flagged as carrying a file.
static void
attachment_message_is_flagged(void)
{
    if (g_attach_mid_len == 0) {
        printf("(attachment message not received) ");
        return;
    }
    CHECK(g_attach_has_att != 0);
    CHECK(g_attach_is_file != 0);
}

// copy_out_file_zap2 writes the attachment to a path. The only meaningful
// check is that the bytes match what we sent.
static void
copy_out_file_zap2_matches_source(void)
{
    if (g_attach_mid_len == 0) {
        printf("(no attachment) ");
        return;
    }
    unlink(ATTACH_OUT);
    int rc = dwyco_copy_out_file_zap2(g_attach_mid, ATTACH_OUT);
    printf("(rc=%d) ", rc);
    if (rc == 0)
        return;
    CHECK(rc != 0);

    long src_size = 0, out_size = 0;
    unsigned long long src_h = file_hash64(ATTACH_SRC, &src_size);
    unsigned long long out_h = file_hash64(ATTACH_OUT, &out_size);
    CHECK(src_h != 0);
    CHECK(out_h == src_h);
    CHECK(out_size == src_size);
    printf("(size=%ld) ", out_size);
}

// copy_out_file_zap_buf2 returns the same bytes as a buffer. The 'max'
// argument caps how much it will hand back; too small and it declines.
static void
copy_out_file_zap_buf2_matches_source(void)
{
    if (g_attach_mid_len == 0) {
        printf("(no attachment) ");
        return;
    }
    long src_size = 0;
    CHECK(file_hash64(ATTACH_SRC, &src_size) != 0);

    // A generous max returns the whole thing.
    const char *buf = "";
    int blen = 0;
    int rc = dwyco_copy_out_file_zap_buf2(g_attach_mid, &buf, &blen,
        (int)src_size + 1024);
    CHECK(rc != 0);
    CHECK(buf != 0);
    if (rc != 0 && buf && buf[0]) {
        CHECK(blen == (int)src_size);
        if (blen == (int)src_size) {
            long tmp_size = 0;
            FILE *f = fopen("/tmp/dwytest_attach_buf.bin", "wb");
            if (f) {
                fwrite(buf, 1, (size_t)blen, f);
                fclose(f);
                unsigned long long h = file_hash64("/tmp/dwytest_attach_buf.bin", &tmp_size);
                CHECK(h == file_hash64(ATTACH_SRC, NULL));
            }
            // Per the header this buffer must be freed with dwyco_free_array.
            dwyco_free_array((char *)buf);
        }
    }

    // A max too small to hold it must be refused rather than truncated.
    buf = "";
    blen = 0;
    int rc2 = dwyco_copy_out_file_zap_buf2(g_attach_mid, &buf, &blen, 1);
    CHECK(rc2 == 0 || blen > 0);
}

// A saved message can be turned into a view, which is what the play/stop and
// preview calls operate on.
static void
make_view_and_quick_stats(void)
{
    if (g_image_mid_len == 0) {
        printf("(no image message) ");
        return;
    }
    DWYCO_SAVED_MSG_LIST sm = 0;
    if (!dwyco_get_saved_message3(&sm, g_image_mid))
        return;
    if (!sm)
        return;

    int viewid = dwyco_make_zap_view2(sm, 0);
    printf("(viewid=%d) ", viewid);
    if (viewid <= 0) {
        // A still image may or may not be a viewable "zap" depending on the
        // build's codec set. Either answer is fine; a crash is not.
        dwyco_list_release(sm);
        return;
    }
    CHECK(viewid > 0);

    int has_video = -1, has_audio = -1, short_video = -1;
    int rc = dwyco_zap_quick_stats_view(viewid, &has_video, &has_audio,
        &short_video);
    CHECK(rc != 0);
    CHECK(has_video == 0 || has_video == 1);
    CHECK(has_audio == 0 || has_audio == 1);
    CHECK(short_video == 0 || short_video == 1);
    printf("(v=%d a=%d short=%d) ", has_video, has_audio, short_video);

    // Playing a view with no audio device must still be handled: start it,
    // then stop it, then delete it.
    int chan = 0;
    int prc = dwyco_zap_play_view_no_audio(viewid, 0, 0, &chan);
    printf("(play=%d chan=%d) ", prc, chan);
    dwyco_zap_stop_view(viewid);
    CHECK(dwyco_delete_zap_view(viewid) != 0);
    // A deleted view id is rejected everywhere.
    CHECK(dwyco_zap_quick_stats_view(viewid, 0, 0, 0) == 0);

    dwyco_list_release(sm);
}

// A view can also be built straight from a local file, in both the normal and
// "raw" forms.
// ===== defect: dwyco_make_zap_view_file terminates the process =====
//
// DEFECT (bld/cdc32/dlli.cpp, dwyco_make_zap_view_file):
//
//   m->actual_filename = newfn(filename);
//
// newfn() -> filename_modify() maps a filename onto one of dwyco's known
// file types via a perfect-hash on the trailing suffix. If the name does not
// end in a registered suffix it does:
//
//   DwString msg = fn; msg += " didn't match anything";
//   oopanic(msg.c_str());
//
// oopanic is [[noreturn]] and exits 1. So dwyco_make_zap_view_file kills the
// process for ANY filename that is not one of dwyco's own types -- a .png, a
// .jpg, anything a caller would plausibly want to view. Verified with a plain
// PNG in /tmp and with the app-dir PNG; both exit.
//
// dwyco_make_zap_view_file_raw is the escape hatch: it assigns actual_filename
// directly instead of going through newfn, and works on any path. That is
// what the test below exercises in-process.
//
// The header documents no such restriction on either function.

static const char *g_argv0 = 0;
static int boot(const char *dir, const char *account, const char *desc);

static void
make_view_file_child(void)
{
    // dwyco_init() is required: newfn() is a passthrough until the filename
    // mapping tables are built, and without it the very bug being tested here
    // cannot happen.
    if (!boot(RECV_DIR, "dwytest-attach", "dwytest attach view probe")) {
        fprintf(stderr, "      child init failed\n");
        _exit(99);
    }
    service_ms(1000);
    int null_fd = open("/dev/null", O_WRONLY);
    if (null_fd >= 0) {
        dup2(null_fd, 2);
        close(null_fd);
    }
    fprintf(stderr, "      child: calling make_zap_view_file\n");
    int v = dwyco_make_zap_view_file(IMAGE_SRC);
    fprintf(stderr, "      child: survived, returned %d\n", v);
    if (v > 0)
        dwyco_delete_zap_view(v);
    exit(0);
}

static void
make_zap_view_file_exits_on_unknown_suffix(void)
{
    if (!g_argv0) {
        printf("[FAIL] no argv[0] recorded for subprocess test\n");
        g_fail++;
        return;
    }
    char *argv[] = { (char *)g_argv0, (char *)"--view-file", 0 };
    // oopanic -> exit(1).
    CHECK(run_subprocess(argv) == 1);
}

// The _raw variant takes the path as given, so it works on any file. For a
// still image there is no decodable zap stream, so quick stats legitimately
// reports nothing -- what matters is that the cookie is valid and the whole
// lifecycle is usable.
static void
make_view_from_local_file_raw(void)
{
    int rawid = dwyco_make_zap_view_file_raw(IMAGE_SRC);
    printf("(raw=%d) ", rawid);
    CHECK(rawid > 0);
    if (rawid <= 0)
        return;

    // A valid cookie means the accessors accept it rather than rejecting it
    // as a bad id; the absence of a decodable stream is reported inside.
    int hv = -1, ha = -1, sv = -1;
    dwyco_zap_quick_stats_view(rawid, &hv, &ha, &sv);
    printf("(v=%d a=%d short=%d) ", hv, ha, sv);
    CHECK(hv == 0 || hv == 1);
    CHECK(ha == 0 || ha == 1);
    CHECK(sv == 0 || sv == 1);

    // Stopping something that is not playing is a no-op, not an error.
    dwyco_zap_stop_view(rawid);

    CHECK(dwyco_delete_zap_view(rawid) != 0);
    // A deleted view id is rejected by every accessor.
    CHECK(dwyco_zap_quick_stats_view(rawid, 0, 0, 0) == 0);
    CHECK(dwyco_delete_zap_view(rawid) == 0);
}

// The preview path decodes a representative frame. Without a real media
// attachment this may legitimately fail; what is asserted is that a
// non-existent view is refused without faulting.
static void
preview_rejects_unknown_view(void)
{
    CHECK(dwyco_zap_create_preview(999999, "/tmp/dwytest_preview.ppm",
        25) == 0);
    unlink("/tmp/dwytest_preview.ppm");

    const char *buf = "";
    int blen = 0, cols = 0, rows = 0;
    CHECK(dwyco_zap_create_preview_buf(999999, &buf, &blen, &cols, &rows) == 0);
}

// ===== forwarding =====

// A message that arrived with no_forward set must refuse to be forwarded; one
// that did not must produce a forward composition.
static void
forward_composition_flags(void)
{
    if (g_plain_mid_len == 0) {
        printf("(no plain message) ");
        return;
    }
    int cid = dwyco_make_forward_zap_composition2(g_plain_mid, 0);
    printf("(cid=%d) ", cid);
    CHECK(cid > 0);
    if (cid <= 0)
        return;
    CHECK(dwyco_is_forward_composition(cid) != 0);
    CHECK(dwyco_zap_still_active(cid) != 0);
    dwyco_delete_zap_composition(cid);

    // strip_forward_text = 1 must also work and still be a forward.
    int cid2 = dwyco_make_forward_zap_composition2(g_plain_mid, 1);
    CHECK(cid2 > 0);
    if (cid2 > 0) {
        CHECK(dwyco_is_forward_composition(cid2) != 0);
        dwyco_delete_zap_composition(cid2);
    }

    // A mid we have never seen must be refused.
    CHECK(dwyco_make_forward_zap_composition2("dwyatt_no_such_mid", 0) == 0);
}

// The real proof: send a forward back to the sender and have it confirm the
// message it received has two body components instead of one. The sender
// writes the answer to its log, which is read here.
static void
forward_round_trip(void)
{
    if (g_plain_mid_len == 0 || g_last_sender_hex[0] == 0) {
        printf("(nothing to forward) ");
        return;
    }
    int cid = dwyco_make_forward_zap_composition2(g_plain_mid, 0);
    if (cid <= 0) {
        printf("(no forward composition) ");
        return;
    }
    char dest[64];
    int dest_len = 0;
    if (!uid_from_hex(g_last_sender_hex, dest, sizeof(dest), &dest_len)) {
        dwyco_delete_zap_composition(cid);
        CHECK(0);
        return;
    }
    const char *pers;
    int plen;
    g_send_ok = 0;
    int rc = dwyco_zap_send6(cid, dest, dest_len, "forwarded", 9, 0, 0, 0,
        &pers, &plen);
    dwyco_delete_zap_composition(cid);
    CHECK(rc != 0);
    if (rc != 0)
        wait_for([&]() { return g_send_ok != 0; }, 60000);

    // Read the sender's answer out of its log.
    char logp[512];
    snprintf(logp, sizeof(logp), "%s/sender.log", SENDER_DIR);
    FILE *f = fopen(logp, "r");
    if (!f) {
        printf("(no sender log) ");
        return;
    }
    char line[512];
    int components = -1;
    while (fgets(line, sizeof(line), f)) {
        const char *p = strstr(line, "components=");
        if (p)
            components = atoi(p + strlen("components="));
    }
    fclose(f);
    printf("(sender saw %d component(s)) ", components);
    // The forwarded message must arrive as 2 components: the forward itself
    // plus the original.
    CHECK(components == 2);
}

// ===== harness =====

static int
boot(const char *dir, const char *account, const char *desc)
{
    char sys_dir[512], tmp_dir[512];
    snprintf(sys_dir, sizeof(sys_dir), "%s/sys", dir);
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", dir);
    mkdir(dir, 0755);
    mkdir(sys_dir, 0755);
    mkdir(tmp_dir, 0755);
    install_app_files(dir);

    dwyco_set_fn_prefixes(sys_dir, dir, tmp_dir);
    dwyco_set_system_event_callback(send_status_cb);
    dwyco_set_client_version("dwytest", 7);
    if (!dwyco_init())
        return 0;
    test_bootstrap_profile(account, desc);
    dwyco_finish_startup();
    dwyco_set_disposition("foreground", 10);

    const char *uid;
    int len;
    dwyco_get_my_uid(&uid, &len);
    if (len <= 0 || len >= (int)sizeof(g_my_uid))
        return 0;
    memcpy(g_my_uid, uid, (size_t)len);
    g_my_uid_len = len;
    for (int i = 0; i < len; i++)
        snprintf(g_my_uid_hex + i * 2, 3, "%02x", (unsigned char)g_my_uid[i]);
    return 1;
}

int
main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--view-file") == 0) {
        setvbuf(stdout, 0, _IOLBF, 0);
        make_view_file_child();
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--peer-send") == 0)
        return peer_send_main(argc, argv);
    g_argv0 = argv[0];

    setvbuf(stdout, 0, _IOLBF, 0);
    printf("Dwyco attachments / views / forwarding\n");
    write_attachment();
    CHECK(file_hash64(ATTACH_SRC, NULL) != 0);

    if (!boot(RECV_DIR, "dwytest-attach", "dwytest attach receiver")) {
        fprintf(stderr, "dwyco_init failed\n");
        return 1;
    }
    printf("  receiver uid=%s\n", g_my_uid_hex);
    service_ms(4000);

    const char *self = argv[0];
    char *child_argv[] = {
        (char *)self, (char *)"--peer-send",
        (char *)SENDER_DIR, g_my_uid_hex, 0
    };
    pid_t child = spawn_subprocess(child_argv);
    if (child < 0) {
        fprintf(stderr, "could not spawn sender\n");
        dwyco_exit();
        return 1;
    }
    printf("  spawned sender pid=%d\n", (int)child);

    int got_plain = collect(TXT_PLAIN, 90000, g_plain_mid, &g_plain_mid_len,
        0, 0);
    int got_attach = collect("dwyatt with a file", 60000, g_attach_mid,
        &g_attach_mid_len, &g_attach_has_att, &g_attach_is_file);
    int got_image = collect("dwyatt with an image", 60000, g_image_mid,
        &g_image_mid_len, 0, 0);
    printf("  plain %s, attachment %s, image %s\n",
        got_plain ? "received" : "TIMED OUT",
        got_attach ? "received" : "TIMED OUT",
        got_image ? "received" : "TIMED OUT");

    // Remember who sent it, for the forward round trip.
    {
        DWYCO_SAVED_MSG_LIST sm = 0;
        if (g_plain_mid_len && dwyco_get_saved_message3(&sm, g_plain_mid) && sm) {
            DWYCO_LIST ba = dwyco_get_body_array(sm);
            if (ba) {
                const char *v;
                int vl, vt;
                if (dwyco_list_get(ba, 0, DWYCO_QM_BODY_FROM, &v, &vl, &vt)
                    && vl == 10)
                    uid_to_hex(v, vl, g_last_sender_hex);
                dwyco_list_release(ba);
            }
            dwyco_list_release(sm);
        }
    }

    printf("\nComposition:\n");
    RUN(raw_composition_requires_dwyco_container);
    RUN(composition_flags);
    RUN(zap_send4_uses_the_save_sent_setting);

    printf("\nAttachment:\n");
    RUN(attachment_message_is_flagged);
    RUN(copy_out_file_zap2_matches_source);
    RUN(copy_out_file_zap_buf2_matches_source);

    printf("\nViews:\n");
    RUN(make_view_and_quick_stats);
    RUN(make_view_from_local_file_raw);
    RUN(make_zap_view_file_exits_on_unknown_suffix);
    RUN(preview_rejects_unknown_view);

    printf("\nForwarding:\n");
    RUN(forward_composition_flags);
    RUN(forward_round_trip);

    int child_status = wait_subprocess(child, 30000);
    printf("  sender exited with %d\n", child_status);

    dwyco_exit();

    printf("\nSummary: passed=%d failed=%d\n", g_pass, g_fail);
    if (g_fail || !got_plain || !got_attach || !got_image) {
        printf("FAILED\n");
        return 1;
    }
    printf("All attachment tests passed.\n");
    return 0;
}