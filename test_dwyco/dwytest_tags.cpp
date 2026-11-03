// Coverage for the message-tag API.
//
//   dwyco_set_msg_tag, dwyco_unset_msg_tag, dwyco_unset_all_msg_tag,
//   dwyco_mid_has_tag, dwyco_count_tag, dwyco_get_tagged_mids,
//   dwyco_get_tagged_mids_older_than, dwyco_get_tagged_idx,
//   dwyco_valid_tag_exists, dwyco_get_mid_tag_payload,
//   dwyco_uid_has_tag, dwyco_uid_count_tag, dwyco_all_messages_tagged,
//   dwyco_set_fav_msg, dwyco_get_fav_msg, dwyco_mid_disposition,
//   dwyco_start_bulk_update, dwyco_end_bulk_update
// plus dwyco_get_tagged_mids2, which cannot be called in-process.
//
// WHY THIS FILE BUILDS ITS OWN FIXTURES
//
// The existing tag tests in test_main.cpp tag mid strings like
// "test_mid_001" that exist in no index, and the comment at test_main.cpp:523
// admits that get_tagged_mids therefore returns nothing for them. So every
// readback API in this group has been crash-checked at best and never
// actually verified.
//
// The reason fake mids do not show up is a real and testable distinction in
// the SQL layer:
//
//   * dwyco_count_tag counts mt.gmt alone.
//   * dwyco_valid_tag_exists, dwyco_get_tagged_mids, dwyco_uid_has_tag and
//     dwyco_uid_count_tag all inner-join the global message index gi.
//
// So a mid with a tag but no gi row is counted yet invisible to every other
// API. Tests 1-5 pin exactly that split.
//
// To cover the positive path without depending on a second live process, the
// tests below insert a row straight into gi with dwyco_run_sql -- which is
// documented as the debugging escape hatch for exactly this ("assuming the app
// knows the schema"). That makes the gi-join behaviour deterministic instead
// of dependent on a message actually arriving over the wire.
//
// The cost is that these tests know about two table layouts (gi and mt.gmt).
// If either schema changes the fixture setup stops working and will say so.

#include <dlli.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <ctime>
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

static char g_my_uid[64];
static int g_my_uid_len;
static char g_my_uid_hex[64];

// Make 'mid' visible to the gi-join APIs by inserting a global index row for
// it. is_sent is set so the record looks like one of our own outgoing
// messages.
//
// dwyco_run_sql binds at most three positional args (?1 ?2 ?3), and this
// insert needs four values, so the two integers are interpolated into the
// statement text instead. They are values this file generates, not external
// input, so the interpolation is safe -- but it is also why the mid is bound
// rather than concatenated.
static int
add_gi_row(const char *mid, long date, long logical_clock)
{
    char sql[1024];
    snprintf(sql, sizeof(sql),
        "insert or ignore into gi"
        " (mid, date, is_sent, is_forwarded, is_no_forward, is_file,"
        "  special_type, has_attachment, att_has_video, att_has_audio,"
        "  att_is_short_video, logical_clock, assoc_uid, from_group, is_local)"
        " values (?1, %ld, 1, 0, 0, 0, 0, 0, 0, 0, 0, %ld, ?2, 0, 1)",
        date, logical_clock);
    return dwyco_run_sql(sql, mid, g_my_uid_hex, 0);
}

static void
drop_gi_row(const char *mid)
{
    dwyco_run_sql("delete from gi where mid = ?1", mid, 0, 0);
}

// ===== synthetic mids: the gmt-only side =====

// count_tag reads mt.gmt with no gi join, so it counts tagged mids that
// nothing else in the API can see.
static void
count_tag_counts_synthetic_mids(void)
{
    static const char *mids[] = { "dwytest_synth_a", "dwytest_synth_b" };
    const char *tag = "dwytest_synth_tag";

    CHECK(dwyco_count_tag(tag) == 0);

    dwyco_set_msg_tag(mids[0], tag);
    CHECK(dwyco_count_tag(tag) == 1);
    dwyco_set_msg_tag(mids[1], tag);
    CHECK(dwyco_count_tag(tag) == 2);

    // Setting the same (mid, tag) twice is idempotent, because gmt is keyed
    // on (mid, tag, uid, guid).
    dwyco_set_msg_tag(mids[1], tag);
    CHECK(dwyco_count_tag(tag) == 2);

    dwyco_unset_msg_tag(mids[0], tag);
    CHECK(dwyco_count_tag(tag) == 1);
    dwyco_unset_msg_tag(mids[1], tag);
    CHECK(dwyco_count_tag(tag) == 0);
}

static void
mid_has_tag_round_trip(void)
{
    static const char mid[] = "dwytest_has_tag_mid";
    static const char tag[] = "dwytest_has_tag";

    dwyco_unset_msg_tag(mid, tag);
    CHECK(dwyco_mid_has_tag(mid, tag) == 0);
    dwyco_set_msg_tag(mid, tag);
    CHECK(dwyco_mid_has_tag(mid, tag) != 0);
    dwyco_unset_msg_tag(mid, tag);
    CHECK(dwyco_mid_has_tag(mid, tag) == 0);
    // Unsetting again is harmless.
    dwyco_unset_msg_tag(mid, tag);
    CHECK(dwyco_mid_has_tag(mid, tag) == 0);
}

static void
unset_all_clears_every_mid(void)
{
    static const char *mids[] = {
        "dwytest_all_1", "dwytest_all_2", "dwytest_all_3"
    };
    const char *tag = "dwytest_unset_all";

    for (unsigned i = 0; i < 3; i++)
        dwyco_set_msg_tag(mids[i], tag);
    CHECK(dwyco_count_tag(tag) == 3);

    dwyco_unset_all_msg_tag(tag);
    CHECK(dwyco_count_tag(tag) == 0);
    for (unsigned i = 0; i < 3; i++)
        CHECK(dwyco_mid_has_tag(mids[i], tag) == 0);
}

// ===== synthetic mids: the gi-join side =====

// The whole point: a mid that is counted is still invisible to every API that
// inner-joins the global index.
static void
synthetic_mids_are_invisible_to_the_gi_joins(void)
{
    static const char mid[] = "dwytest_no_gi_mid";
    const char *tag = "dwytest_no_gi_tag";

    dwyco_unset_msg_tag(mid, tag);
    drop_gi_row(mid);

    dwyco_set_msg_tag(mid, tag);

    // Present in gmt...
    CHECK(dwyco_mid_has_tag(mid, tag) != 0);
    CHECK(dwyco_count_tag(tag) == 1);
    // ...and absent everywhere gi is joined.
    CHECK(dwyco_valid_tag_exists(tag) == 0);
    CHECK(dwyco_uid_has_tag(g_my_uid, g_my_uid_len, tag) == 0);
    CHECK(dwyco_uid_count_tag(g_my_uid, g_my_uid_len, tag) == 0);

    DWYCO_LIST mids = 0;
    CHECK(dwyco_get_tagged_mids(&mids, tag) != 0);
    if (mids) {
        CHECK(lr_rows(mids, 0));
        dwyco_list_release(mids);
    }

    DWYCO_MSG_IDX idx = 0;
    CHECK(dwyco_get_tagged_idx(&idx, tag, 0) != 0);
    if (idx) {
        int rows = -1, cols = -1;
        CHECK(dwyco_list_numelems(idx, &rows, &cols) != 0);
        CHECK(rows == 0);
        dwyco_list_release(idx);
    }

    dwyco_unset_msg_tag(mid, tag);
    CHECK(dwyco_count_tag(tag) == 0);
}

// ===== the gi-join side, with a real index row =====

// With a gi row present, every readback API must find the mid. This is the
// coverage test_main.cpp could not provide.
static void
gi_row_makes_the_mid_visible_everywhere(void)
{
    static const char mid[] = "dwytest_gi_mid_1";
    const char *tag = "dwytest_gi_tag";

    dwyco_unset_msg_tag(mid, tag);
    drop_gi_row(mid);
    CHECK(add_gi_row(mid, 1700000000L, 7) != 0);
    dwyco_set_msg_tag(mid, tag);

    CHECK(dwyco_count_tag(tag) == 1);
    CHECK(dwyco_valid_tag_exists(tag) != 0);
    CHECK(dwyco_uid_has_tag(g_my_uid, g_my_uid_len, tag) != 0);
    CHECK(dwyco_uid_count_tag(g_my_uid, g_my_uid_len, tag) == 1);

    // get_tagged_mids returns mid/uid pairs. The header warns the uid comes
    // back as ASCII hex, not binary -- that warning is correct.
    DWYCO_LIST mids = 0;
    CHECK(dwyco_get_tagged_mids(&mids, tag) != 0);
    if (mids) {
        CHECK(lr_rows(mids, 1));
        CHECK(lr_cols(mids, 2));
        CHECK(lr_col_str(mids, 0, DWYCO_TAGGED_MIDS_MID, mid, (int)strlen(mid)));
        CHECK(lr_col_str(mids, 0, DWYCO_TAGGED_MIDS_HEX_UID,
            g_my_uid_hex, (int)strlen(g_my_uid_hex)));
        dwyco_list_release(mids);
    }

    // get_tagged_idx returns the full DWYCO_MSG_IDX record.
    DWYCO_MSG_IDX idx = 0;
    CHECK(dwyco_get_tagged_idx(&idx, tag, 0) != 0);
    if (idx) {
        int rows = -1, cols = -1;
        CHECK(dwyco_list_numelems(idx, &rows, &cols) != 0);
        CHECK(rows == 1);
        CHECK(cols > 0);
        CHECK(lr_col_str(idx, 0, DWYCO_MSG_IDX_MID, mid, (int)strlen(mid)));
        // The index record carries the assoc uid as hex too.
        CHECK(lr_col_str(idx, 0, DWYCO_MSG_IDX_ASSOC_UID,
            g_my_uid_hex, (int)strlen(g_my_uid_hex)));
        dwyco_list_release(idx);
    }

    dwyco_unset_msg_tag(mid, tag);
    drop_gi_row(mid);
}

// A mid can carry several tags at once, and count_tag keeps them distinct.
static void
multiple_tags_per_mid(void)
{
    static const char mid[] = "dwytest_multi_mid";
    static const char *tags[] = {
        "dwytest_multi_a", "dwytest_multi_b", "dwytest_multi_c"
    };

    dwyco_unset_all_msg_tag(tags[0]);
    dwyco_unset_all_msg_tag(tags[1]);
    dwyco_unset_all_msg_tag(tags[2]);

    drop_gi_row(mid);
    CHECK(add_gi_row(mid, 1700000001L, 8) != 0);
    for (unsigned i = 0; i < 3; i++)
        dwyco_set_msg_tag(mid, tags[i]);

    for (unsigned i = 0; i < 3; i++) {
        CHECK(dwyco_mid_has_tag(mid, tags[i]) != 0);
        CHECK(dwyco_count_tag(tags[i]) == 1);
        CHECK(dwyco_valid_tag_exists(tags[i]) != 0);
    }
    // Removing one tag leaves the others alone.
    dwyco_unset_msg_tag(mid, tags[1]);
    CHECK(dwyco_mid_has_tag(mid, tags[0]) != 0);
    CHECK(dwyco_mid_has_tag(mid, tags[1]) == 0);
    CHECK(dwyco_mid_has_tag(mid, tags[2]) != 0);

    dwyco_unset_all_msg_tag(tags[0]);
    dwyco_unset_all_msg_tag(tags[2]);
    dwyco_unset_msg_tag(mid, tags[1]);
    drop_gi_row(mid);
}

// older_than filters on the gmt row's own tag time:
//
//   select distinct(mid) from gmt where tag = ?1
//     and strftime('%s','now') - time > (?2 * 24 * 3600)
//
// Note it reads gmt directly -- unlike get_tagged_mids it does NOT join gi,
// so no global-index row is needed here.
//
// The window is "strictly older than N days", so a tag created right now is
// not in any window, including a 0-day one. To make the assertions
// independent of the wall clock, the tag time is set explicitly relative to
// now rather than to a fixed epoch.
static void
tagged_mids_older_than_window(void)
{
    static const char mid[] = "dwytest_older_mid";
    static const char tag[] = "dwytest_older_tag";

    dwyco_unset_msg_tag(mid, tag);
    drop_gi_row(mid);
    CHECK(add_gi_row(mid, 1700000002L, 9) != 0);
    dwyco_set_msg_tag(mid, tag);

    // Freshly tagged: seconds old, so not "older than" any real window.
    DWYCO_LIST l = 0;
    CHECK(dwyco_get_tagged_mids_older_than(&l, tag, 0) != 0);
    if (l) { CHECK(lr_rows(l, 0)); dwyco_list_release(l); }
    l = 0;
    CHECK(dwyco_get_tagged_mids_older_than(&l, tag, 9999) != 0);
    if (l) { CHECK(lr_rows(l, 0)); dwyco_list_release(l); }

    // Age the tag row to 100 days ago. The window boundaries below are kept
    // far from the tag's actual age on purpose: the query compares against
    // SQLite's strftime('%s','now'), which is UTC, while time(NULL) is local.
    // On a machine with a non-zero utc offset the two disagree by hours, so
    // any test sitting exactly on the boundary would be flaky. Sticking to
    // ages and windows that are tens of days apart makes that irrelevant.
    char aged[32];
    snprintf(aged, sizeof(aged), "%ld", (long)time(NULL) - 100 * 86400);
    CHECK(dwyco_run_sql("update mt.gmt set time = ?1 where mid = ?2 and tag = ?3",
        aged, mid, tag) != 0);

    // Inside every window narrower than its age...
    l = 0;
    CHECK(dwyco_get_tagged_mids_older_than(&l, tag, 0) != 0);
    if (l) { CHECK(lr_rows(l, 1)); dwyco_list_release(l); }
    l = 0;
    CHECK(dwyco_get_tagged_mids_older_than(&l, tag, 50) != 0);
    if (l) { CHECK(lr_rows(l, 1)); dwyco_list_release(l); }
    l = 0;
    CHECK(dwyco_get_tagged_mids_older_than(&l, tag, 99) != 0);
    if (l) { CHECK(lr_rows(l, 1)); dwyco_list_release(l); }
    // ...and outside every wider one.
    l = 0;
    CHECK(dwyco_get_tagged_mids_older_than(&l, tag, 150) != 0);
    if (l) { CHECK(lr_rows(l, 0)); dwyco_list_release(l); }
    l = 0;
    CHECK(dwyco_get_tagged_mids_older_than(&l, tag, 9999) != 0);
    if (l) { CHECK(lr_rows(l, 0)); dwyco_list_release(l); }

    dwyco_unset_msg_tag(mid, tag);
    drop_gi_row(mid);
}

// all_messages_tagged asks "are ALL of this uid's messages tagged?", which is
// the complement of uid_has_tag ("is ANY tagged?").
//
// Worth spelling out because the implementation reads as the opposite: it
// selects messages that are NOT tagged and returns "there was at least one",
// i.e. it really returns 0 when everything is tagged. The observable contract
// matches the name; only the query looks inverted.
static void
uid_has_tag_versus_all_messages_tagged(void)
{
    static const char tagged_mid[] = "dwytest_allmsgs_a";
    static const char untagged_mid[] = "dwytest_allmsgs_b";
    const char *tag = "dwytest_allmsgs_tag";

    dwyco_unset_all_msg_tag(tag);
    drop_gi_row(tagged_mid);
    drop_gi_row(untagged_mid);
    CHECK(add_gi_row(tagged_mid, 1700000003L, 10) != 0);

    // One message for this uid, and it carries the tag.
    dwyco_set_msg_tag(tagged_mid, tag);
    CHECK(dwyco_uid_has_tag(g_my_uid, g_my_uid_len, tag) != 0);
    CHECK(dwyco_all_messages_tagged(g_my_uid, g_my_uid_len, tag) != 0);

    // Add a second, untagged message: "any" still holds, "all" now fails.
    CHECK(add_gi_row(untagged_mid, 1700000004L, 11) != 0);
    CHECK(dwyco_uid_has_tag(g_my_uid, g_my_uid_len, tag) != 0);
    CHECK(dwyco_all_messages_tagged(g_my_uid, g_my_uid_len, tag) == 0);

    // And with no messages at all for the uid, "all" is vacuously false.
    drop_gi_row(untagged_mid);
    drop_gi_row(tagged_mid);
    CHECK(dwyco_all_messages_tagged(g_my_uid, g_my_uid_len, tag) == 0);

    dwyco_unset_msg_tag(tagged_mid, tag);
}

// ===== tag payloads =====

// get_mid_tag_payload returns 0 for malformed input and 1 plus a 1-element
// list otherwise. With no payload stored, that element is NIL.
static void
mid_tag_payload_contract(void)
{
    static const char mid[] = "dwytest_payload_mid";
    static const char tag[] = "dwytest_payload_tag";

    dwyco_unset_msg_tag(mid, tag);
    dwyco_set_msg_tag(mid, tag);

    DWYCO_LIST payload = 0;
    CHECK(dwyco_get_mid_tag_payload(mid, tag, &payload) != 0);
    CHECK(payload != 0);
    if (payload) {
        CHECK(lr_rows(payload, 1));
        // No payload stored, so the single element is nil.
        CHECK(lr_nil(payload, 0));
        dwyco_list_release(payload);
    }

    // Malformed input is rejected without touching the out-pointer.
    payload = (DWYCO_LIST)0x1;
    CHECK(dwyco_get_mid_tag_payload("", tag, &payload) == 0);
    CHECK(payload == (DWYCO_LIST)0x1);
    payload = (DWYCO_LIST)0x1;
    CHECK(dwyco_get_mid_tag_payload(mid, "", &payload) == 0);
    CHECK(payload == (DWYCO_LIST)0x1);

    // An (mid, tag) pair that was never set still reports success, with a
    // nil payload -- the documented behaviour.
    payload = 0;
    CHECK(dwyco_get_mid_tag_payload(mid, "dwytest_never_set", &payload) != 0);
    if (payload) {
        CHECK(lr_rows(payload, 1));
        CHECK(lr_nil(payload, 0));
        dwyco_list_release(payload);
    }

    dwyco_unset_msg_tag(mid, tag);
}

// ===== favorites and disposition =====

static void
fav_msg_round_trip(void)
{
    static const char mid[] = "dwytest_fav_mid";
    dwyco_set_fav_msg(mid, 1);
    CHECK(dwyco_get_fav_msg(mid) != 0);
    dwyco_set_fav_msg(mid, 0);
    CHECK(dwyco_get_fav_msg(mid) == 0);
}

// mid_disposition has no documented range beyond -1 meaning "unknown", so
// only check it stays in the defined region.
static void
mid_disposition_is_defined(void)
{
    int d = dwyco_mid_disposition("dwytest_never_existed_mid");
    CHECK(d >= -1);
}

// ===== bulk update =====

// start/end bulk update wrap one SQL transaction around a batch of tag writes.
// The observable effect is that all the rows land.
static void
bulk_update_commits_every_tag(void)
{
    const char *tag = "dwytest_bulk_tag";
    char mid[64];

    dwyco_unset_all_msg_tag(tag);
    dwyco_start_bulk_update();
    for (int i = 0; i < 250; i++) {
        snprintf(mid, sizeof(mid), "dwytest_bulk_%03d", i);
        dwyco_set_msg_tag(mid, tag);
    }
    dwyco_end_bulk_update();

    CHECK(dwyco_count_tag(tag) == 250);
    CHECK(dwyco_mid_has_tag("dwytest_bulk_000", tag) != 0);
    CHECK(dwyco_mid_has_tag("dwytest_bulk_249", tag) != 0);

    dwyco_unset_all_msg_tag(tag);
    CHECK(dwyco_count_tag(tag) == 0);
}

// ===== get_tagged_mids2 =====

// dwyco_get_tagged_mids2 is declared in the header as a normal function but
// its body is a bare oopanic("broken") with the real code commented out
// behind it. Calling it therefore always terminates the process, whatever the
// tag.
//
// The header documents no such restriction, so this is pinned out-of-process.
// If it is ever implemented, this test fails and should be rewritten to
// assert the returned list.
//
// Note this is NOT a substitute for the pair-returning behaviour the dead
// code describes: callers needing uid/mid pairs should use
// dwyco_get_tagged_mids, whose uid column really is the hex form.

static const char *g_argv0 = 0;

static void
tagged_mids2_child(void)
{
    DWYCO_LIST l = 0;
    int rc = dwyco_get_tagged_mids2(&l, "dwytest_bulk_tag");
    printf("      survived: rc=%d list=%s\n", rc, l ? "non-null" : "NULL");
    exit(0);
}

static void
get_tagged_mids2_terminates(void)
{
    if (!g_argv0) {
        printf("[FAIL] no argv[0] recorded for subprocess test\n");
        g_fail++;
        return;
    }
    char *argv[] = { (char *)g_argv0, (char *)"--tagged-mids2", 0 };
    // oopanic -> exit(1).
    CHECK(run_subprocess(argv) == 1);
}

// ===== harness =====

static int
init_test(void)
{
    static const char *dir = "/tmp/dwytest_tags";
    char sys_dir[512], tmp_dir[512];
    snprintf(sys_dir, sizeof(sys_dir), "%s/sys", dir);
    snprintf(tmp_dir, sizeof(tmp_dir), "%s/tmp", dir);
    mkdir(dir, 0755);
    mkdir(sys_dir, 0755);
    mkdir(tmp_dir, 0755);
    install_app_files(dir);

    dwyco_set_fn_prefixes(sys_dir, dir, tmp_dir);
    dwyco_set_client_version("dwytest", 7);
    if (dwyco_init() == 0)
        return 0;
    test_bootstrap_profile("dwytest-tags", "dwytest tags account");
    dwyco_finish_startup();

    const char *uid;
    int len;
    dwyco_get_my_uid(&uid, &len);
    if (len <= 0 || len >= (int)sizeof(g_my_uid))
        return 0;
    memcpy(g_my_uid, uid, (size_t)len);
    g_my_uid_len = len;
    // assoc_uid is stored as hex; mirror that for the fixture rows.
    for (int i = 0; i < len; i++)
        snprintf(g_my_uid_hex + i * 2, 3, "%02x", (unsigned char)g_my_uid[i]);
    return 1;
}

int
main(int argc, char **argv)
{
    static const char *dir = "/tmp/dwytest_tags";
    (void)dir;

    if (argc > 1 && strcmp(argv[1], "--tagged-mids2") == 0) {
        setvbuf(stdout, 0, _IOLBF, 0);
        int null_fd = open("/dev/null", O_WRONLY);
        if (null_fd >= 0) {
            dup2(null_fd, 2);
            close(null_fd);
        }
        if (!init_test())
            _exit(99);
        tagged_mids2_child();
        return 0;
    }
    g_argv0 = argv[0];
    setvbuf(stdout, 0, _IOLBF, 0);

    printf("Dwyco message tagging\n");

    if (!init_test()) {
        fprintf(stderr, "dwyco_init failed\n");
        return 1;
    }

    printf("\nTags without a global-index row:\n");
    RUN(count_tag_counts_synthetic_mids);
    RUN(mid_has_tag_round_trip);
    RUN(unset_all_clears_every_mid);
    RUN(synthetic_mids_are_invisible_to_the_gi_joins);

    printf("\nTags with a global-index row:\n");
    RUN(gi_row_makes_the_mid_visible_everywhere);
    RUN(multiple_tags_per_mid);
    RUN(tagged_mids_older_than_window);
    RUN(uid_has_tag_versus_all_messages_tagged);

    printf("\nPayloads:\n");
    RUN(mid_tag_payload_contract);

    printf("\nFavorites / disposition:\n");
    RUN(fav_msg_round_trip);
    RUN(mid_disposition_is_defined);

    printf("\nBulk:\n");
    RUN(bulk_update_commits_every_tag);

    printf("\nBroken api:\n");
    RUN(get_tagged_mids2_terminates);

    dwyco_exit();

    printf("\nSummary: passed=%d failed=%d\n", g_pass, g_fail);
    if (g_fail) {
        printf("FAILED\n");
        return 1;
    }
    printf("All tag tests passed.\n");
    return 0;
}