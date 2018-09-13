
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifdef _Windows
#ifdef __BORLANDC__
#include <dir.h>
#endif
#ifdef _MSC_VER
#include <direct.h>
#endif
#include <dos.h>
#include <io.h>
#endif

#ifdef DWYCO_USE_STATIC_SQLITE
#include "sqlite/sqlite3.h"
#else
#include <sqlite3.h>
#endif

#include "fnmod.h"
#include "dwrtlog.h"
#include "qauth.h"
#include "qmsg.h"
#include "vc.h"
#include "xinfo.h"
#include "se.h"
#include "filetube.h"
#include "sepstr.h"
#include "sqlbq.h"
#include "favmsg.h"


static sqlite3 *Db;

static
void
sql_simple(const char *sql)
{
    VCArglist a;
    a.append(sql);
    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
}

static
void
init_schema()
{
    // WARNING: the order and number of the fields in this table
    // is the same as the #defines for the msg index in
    // qmsg.h
    sql_simple("create table if not exists msg_idx ("
               "date integer,"
               "mid text primary key,"
               "is_sent,"
               "is_forwarded,"
               "is_no_forward,"
               "is_file,"
               "special_type,"
               "has_attachment,"
               "att_has_video,"
               "att_has_audio,"
               "att_is_short_video,"
               "logical_clock,"
               "assoc_uid text not null);"
              );
    sql_simple("create table if not exists most_recent_msg (uid text primary key not null on conflict ignore, date integer not null on conflict ignore);");
    sql_simple("create table if not exists indexed_flag (uid text primary key);");
    sql_simple("create index if not exists assoc_uid_idx on msg_idx(assoc_uid);");
    sql_simple("create index if not exists logical_clock_idx on msg_idx(logical_clock desc);");
    sql_simple("create index if not exists date_idx on msg_idx(date desc);");
    sql_simple("create index if not exists sent_idx on msg_idx(is_sent);");
    sql_simple("create index if not exists att_idx on msg_idx(has_attachment);");

}

void
init_qmsg_sql()
{
    if(Db)
        oopanic("already init");
    if(sqlite3_open(newfn("mi.sql").c_str(), &Db) != SQLITE_OK)
    {
        Db = 0;
        return;
    }
    init_schema();
}

void
exit_qmsg_sql()
{
    if(!Db)
        return;
    sqlite3_close_v2(Db);
    Db = 0;
}



static
void
sql_start_transaction()
{
    sql_simple("savepoint mi;");
}

static
void
sql_commit_transaction()
{
    sql_simple("release mi;");
}

static
void
sql_sync_off()
{
    sql_simple("pragma synchronous=off;");
}

static
void
sql_sync_on()
{
    sql_simple("pragma synchronous=full;");

}

static
void
sql_rollback_transaction()
{
    VCArglist a;
    a.append("rollback to savepoint mi;");
    sqlite3_bulk_query(Db, &a);
}

static
void
sql_insert_record(vc entry, vc assoc_uid)
{
    VCArglist a;
    a.set_size(NUM_QM_IDX_FIELDS + 2);
    a.append("replace into msg_idx values($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13);");

    for(int i = 0; i < NUM_QM_IDX_FIELDS; ++i)
        a.append(entry[i]);
    a.append(to_hex(assoc_uid));

    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
}

static
void
sql_record_most_recent(vc uid)
{
    VCArglist a;
    a.append("insert or ignore into most_recent_msg select assoc_uid, max(date) from msg_idx where assoc_uid = $1 limit 1;");
    a.append(to_hex(uid));
    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
}

static
void
sql_delete_mid(vc mid)
{
    VCArglist a;
    a.append("delete from msg_idx where mid = $1;");
    a.append(mid);
    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;

}

long
sql_get_max_logical_clock()
{
    VCArglist a;
    a.append("select max(logical_clock) from msg_idx;");
    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
    if(res[0][0].type() != VC_INT)
        return 0;
    return (long)res[0][0];
}

// some users have 1000's of users in their list, and this can
// cause some problems. most of those users are just old users
// that are no longer active, or the user doesn't know how (or doesn't
// care) to use the list cleaning tools. with this, we just exclude
// those users that are not likely to be sending messages and so on, which
// speeds up the ui significantly.
vc
sql_get_recent_users(int *total_out)
{


    try
    {
        sql_start_transaction();
        sql_simple("create temp table foo as select max(date), assoc_uid from msg_idx group by assoc_uid;");
        sql_simple("insert into foo select date, uid from most_recent_msg;");
        vc res;
        if(total_out)
        {
            VCArglist a;
            a.append("select count(distinct assoc_uid) from foo;");
            res = sqlite3_bulk_query(Db, &a);
            if(res.is_nil())
                throw -1;
            *total_out = (int)res[0][0];
        }
        VCArglist a;
        a.append("select distinct assoc_uid from foo order by \"max(date)\" desc limit 100;");
        res = sqlite3_bulk_query(Db, &a);
        if(res.is_nil())
            throw -1;

        sql_simple("drop table foo;");
        sql_commit_transaction();
        vc ret(VC_VECTOR);
        for(int i = 0; i < res.num_elems(); ++i)
            ret.append(res[i][0]);
        return ret;
    }
    catch(...)
    {
        sql_rollback_transaction();
        return vcnil;
    }
}

vc
sql_get_recent_users2(int max_age, int max_count)
{
    VCArglist a;

    try
    {
        sql_start_transaction();
        sql_simple("create temp table foo as select max(date), assoc_uid from msg_idx group by assoc_uid;");
        sql_simple("insert into foo select date, uid from most_recent_msg;");
        vc res;
        a.append("select distinct assoc_uid from foo where strftime('%s', 'now') - \"max(date)\" < $1 order by \"max(date)\" desc limit $2;");
        a.append(max_age);
        a.append(max_count);
        res = sqlite3_bulk_query(Db, &a);
        if(res.is_nil())
            throw -1;

        sql_simple("drop table foo;");
        sql_commit_transaction();
        vc ret(VC_VECTOR);
        for(int i = 0; i < res.num_elems(); ++i)
            ret.append(res[i][0]);
        return ret;
    }
    catch(...)
    {
        sql_rollback_transaction();
        return vcnil;
    }
}

// this is used by power_clean_safe to find users that you have messages
// from, but you have never responded to

vc
sql_get_old_ignored_users()
{
    VCArglist a;

    try
    {
        sql_start_transaction();
        sql_simple("create temp table foo as select distinct assoc_uid from msg_idx;");
        sql_simple("delete from foo where exists(select * from msg_idx where foo.assoc_uid = assoc_uid and is_sent = 't');");
        sql_simple("delete from foo where exists(select * from msg_idx where foo.assoc_uid = assoc_uid and is_sent isnull and strftime('%s', 'now') - date < 14 * 24 * 3600);");
        sql_simple("delete from foo where (select count(*) from msg_idx where foo.assoc_uid = assoc_uid and has_attachment notnull and is_sent isnull limit 3) > 2;");
        sql_simple("delete from foo where (select count(*) from msg_idx where foo.assoc_uid = assoc_uid limit 5) > 4;");
        a.append("select * from foo;");
        vc res = sqlite3_bulk_query(Db, &a);
        if(res.is_nil())
            throw -1;

        sql_simple("drop table foo;");
        sql_commit_transaction();
        vc ret(VC_VECTOR);
        for(int i = 0; i < res.num_elems(); ++i)
            ret.append(res[i][0]);
        return ret;
    }
    catch(...)
    {
        sql_rollback_transaction();
        return vcnil;
    }
}

vc
sql_get_empty_users()
{
    VCArglist a;

    try
    {
        sql_start_transaction();

        a.append("select uid from indexed_flag where not exists (select * from msg_idx where uid = assoc_uid);");
        vc res = sqlite3_bulk_query(Db, &a);
        if(res.is_nil())
            throw -1;
        sql_commit_transaction();
        vc ret(VC_VECTOR);
        for(int i = 0; i < res.num_elems(); ++i)
            ret.append(res[i][0]);
        return ret;
    }
    catch(...)
    {
        sql_rollback_transaction();
        return vcnil;
    }

}

vc
sql_get_no_response_users()
{
    VCArglist a;

    try
    {
        sql_start_transaction();

        a.append("select uid from indexed_flag where not exists(select * from msg_idx where uid = assoc_uid and is_sent isnull);");
        vc res = sqlite3_bulk_query(Db, &a);
        if(res.is_nil())
            throw -1;
        sql_commit_transaction();
        vc ret(VC_VECTOR);
        for(int i = 0; i < res.num_elems(); ++i)
            ret.append(res[i][0]);
        return ret;
    }
    catch(...)
    {
        sql_rollback_transaction();
        return vcnil;
    }

}

static
void
sql_remove_uid(vc uid)
{
    try
    {
        vc res;
        sql_start_transaction();
        {
            VCArglist a;
            a.append("delete from indexed_flag where uid = $1;");
            a.append(to_hex(uid));
            res = sqlite3_bulk_query(Db, &a);
            if(res.is_nil())
                throw -1;
        }
        {
            VCArglist a;
            a.append("delete from msg_idx where assoc_uid = $1;");
            a.append(to_hex(uid));
            res = sqlite3_bulk_query(Db, &a);
            if(res.is_nil())
                throw -1;
        }
        {
            VCArglist a;
            a.append("delete from most_recent_msg where uid = $1;");
            a.append(to_hex(uid));
            res = sqlite3_bulk_query(Db, &a);
            if(res.is_nil())
                throw -1;
        }
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }

}

static
void
sql_clear_uid(vc uid)
{
    try
    {
        sql_start_transaction();
        VCArglist a;

        a.append("delete from msg_idx where assoc_uid = $1;");
        a.append(to_hex(uid));
        vc res = sqlite3_bulk_query(Db, &a);
        if(res.is_nil())
            throw -1;

        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }

}

static
void
sql_insert_indexed_flag(vc uid)
{
    VCArglist a;
    a.append("insert or replace into indexed_flag values($1);");
    a.append(to_hex(uid));
    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;

}

static
int
sql_check_indexed_flag(vc uid)
{
    VCArglist a;
    a.append("select count(*) from indexed_flag where uid = $1;");
    a.append(to_hex(uid));
    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
    if(res[0][0] == vc(0))
        return 0;
    return 1;
}

static
void
sql_reset_indexed_flag(vc uid)
{
    VCArglist a;
    a.append("delete from indexed_flag where uid = $1;");
    a.append(to_hex(uid));
    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
}



static
vc
sql_load_index(vc uid, int max_count)
{
    VCArglist a;
    a.append("select date, mid, is_sent, is_forwarded, is_no_forward, is_file, special_type, "
           "has_attachment, att_has_video, att_has_audio, att_is_short_video, logical_clock, assoc_uid "
           " from msg_idx where assoc_uid = $1 order by logical_clock desc limit $2;");
    a.append(to_hex(uid));
    a.append(max_count);
    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
    return res;
}

static
int
sql_count_index(vc uid)
{
    VCArglist a;
    a.append("select count(*) from msg_idx where assoc_uid = $1;");
    a.append(to_hex(uid));
    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
    return (int)res[0][0];
}

static vc
index_from_body(vc recipient, vc body)
{
    vc uid;
    if(recipient.is_nil())
        uid = body[QM_BODY_FROM];
    else
        uid = recipient;
    // old msgs, don't bother sorting them properly
    vc date;
    if(body[QM_BODY_DATE].num_elems() <= 5)
        date = 0;
    else
        date = body[QM_BODY_DATE][5];
    vc mid = body[QM_BODY_ID];

    vc nentry(VC_VECTOR);
    nentry[QM_IDX_DATE] = date;
    nentry[QM_IDX_MID] = mid;
    nentry[QM_IDX_IS_SENT] = body[QM_BODY_SENT];
    nentry[QM_IDX_IS_FORWARDED] = body[QM_BODY_FORWARDED_BODY].is_nil() ? vcnil : vctrue;
    nentry[QM_IDX_IS_NO_FORWARD] = body[QM_BODY_NO_FORWARD];
    nentry[QM_IDX_IS_FILE] = body[QM_BODY_FILE_ATTACHMENT].is_nil() ? vcnil : vctrue;
    nentry[QM_IDX_SPECIAL_TYPE] = body[QM_BODY_SPECIAL_TYPE];
    nentry[QM_IDX_HAS_ATTACHMENT] = body[QM_BODY_ATTACHMENT].is_nil() ? vcnil : vctrue;
    vc lc = body[QM_BODY_LOGICAL_CLOCK];
    // older messages won't have logical clock, so just fall back on the
    // date
    if(lc.is_nil())
        lc = date;
    nentry[QM_IDX_LOGICAL_CLOCK] = lc;

    if(!body[QM_BODY_ATTACHMENT].is_nil() && body[QM_BODY_FILE_ATTACHMENT].is_nil())
    {
        vc dir;
        dir = uid_to_dir(uid);
        DwString fn((const char *)dir);
        fn += DIRSEPSTR;
        fn += (const char *)body[QM_BODY_ATTACHMENT];
        fn = newfn(fn);

        if(fn.length() > 0 && access(fn.c_str(), 0) == 0)
        {
            SlippyTube st(fn, "rb", FileTube::SOURCE);

            int video_out;
            int audio_out;
            int dummy;
            if(st.quick_stats(35, 5, 30, video_out, audio_out, dummy, dummy) != 0)
            {
                if(video_out >= 1)
                    nentry[QM_IDX_ATT_HAS_VIDEO] = vctrue;
                if(audio_out >= 1)
                    nentry[QM_IDX_ATT_HAS_AUDIO] = vctrue;
                if(video_out >= 1 && video_out <= 2)
                    nentry[QM_IDX_ATT_IS_SHORT_VIDEO] = vctrue;
            }
        }
    }
    return nentry;
}

static
int
create_date_index(vc uid)
{
    vc id = uid_to_dir(uid);
    int i = 1;
    DwString s((const char *)id);
    s = newfn(s);
    DwString ss = s;
    s += "" DIRSEPSTR "*.bod";

    try
    {
        sql_start_transaction();

        FindVec& fv = *find_to_vec(s.c_str());
        int n = fv.num_elems();
        for(i = 0; i < n; ++i)
        {
            WIN32_FIND_DATA &d = *fv[i];
            DwString s2(ss);
            s2 += "" DIRSEPSTR "";
            s2 += d.cFileName;
            vc info;
            if(load_info(info, s2.c_str(), 1))
            {
                sql_insert_record(index_from_body(uid, info), uid);
            }
        }
        delete_findvec(&fv);

        s = ss;
        s += "" DIRSEPSTR "*.snt";

        FindVec& fv2 = *find_to_vec(s.c_str());
        n = fv2.num_elems();
        for(i = 0; i < n; ++i)
        {
            WIN32_FIND_DATA &d = *fv2[i];
            DwString s2(ss);
            s2 += "" DIRSEPSTR "";
            s2 += d.cFileName;
            vc info;
            if(load_info(info, s2.c_str(), 1))
            {
                info[QM_BODY_SENT] = vctrue;
                sql_insert_record(index_from_body(uid, info), uid);
            }
        }
        delete_findvec(&fv2);
        sql_insert_indexed_flag(uid);
        sql_record_most_recent(uid);
        sql_commit_transaction();
        return 1;
    }
    catch(...)
    {
        sql_rollback_transaction();
        return 0;
    }

}

vc
load_msg_index(vc uid, int load_count)
{
    vc ret;

    if(sql_check_indexed_flag(uid))
    {
        ret = sql_load_index(uid, load_count);
        boost_clock(ret);
        GRTLOGVC(ret);
        return ret;
    }

    if(create_date_index(uid) == 0)
        return vcnil;
    vc di = sql_load_index(uid, load_count);
    boost_clock(di);
    GRTLOG("new idx", 0, 0);
    GRTLOGVC(di);
    return di;
}

int
msg_index_count(vc uid)
{
    return sql_count_index(uid);
}

static void
msg_idx_updated(vc uid, int prepended)
{
    if(prepended)
        se_emit(SE_USER_MSG_IDX_UPDATED_PREPEND, uid);
    else
        se_emit(SE_USER_MSG_IDX_UPDATED, uid);
}

// note: before calling this, the msg body should be saved
// into the right place, since if the index doesn't exists, the
// directory with all the messages is scanned and the index is
// initialized.
void
update_msg_idx(vc recip, vc body)
{

    GRTLOG("update idx", 0, 0);
    GRTLOGVC(recip);
    GRTLOGVC(body);
    vc uid;
    if(recip.is_nil())
        uid = body[QM_BODY_FROM];
    else
        uid = recip;
    // old msgs, don't bother sorting them properly
    vc date;
    if(body[QM_BODY_DATE].num_elems() <= 5)
        date = 0;
    else
        date = body[QM_BODY_DATE][5];

    vc logical_clock = body[QM_BODY_LOGICAL_CLOCK];
    GRTLOG("date lc", 0, 0);
    GRTLOGVC(date);
    GRTLOGVC(logical_clock);

    vc nentry = index_from_body(recip, body);
    GRTLOGVC(nentry);

    try
    {
        sql_start_transaction();
        sql_insert_record(nentry, uid);
        sql_record_most_recent(uid);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }

    msg_idx_updated(uid, 0);
}

void
remove_msg_idx(vc uid, vc mid)
{
    sql_delete_mid(mid);
    msg_idx_updated(uid, 0);
}

void
remove_msg_idx_uid(vc uid)
{
    msg_idx_updated(uid, 0);
    sql_remove_uid(uid);
}

vc
get_unfav_msgids(vc uid)
{
    vc ret(VC_VECTOR);
    DwString mfn = newfn("fav.sql");
    sql_simple(DwString("attach '%1' as mt;").arg(mfn).c_str());
    try
    {
        sql_start_transaction();
        VCArglist a;
        a.append("select mid as foo from msg_idx where assoc_uid = $1 "
                 "and not exists (select * from msg_tags2 where mid = foo and tag = '_fav')");
        a.append(to_hex(uid));
        vc res = sqlite3_bulk_query(Db, &a);
        if(res.is_nil())
            throw -1;
        int n = res.num_elems();
        for(int i = 0; i < n; ++i)
        {
            vc mid = res[i][0];
            ret.append(mid);
        }
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }
    sql_simple("detach mt;");
    return ret;
}

// the difference between clear and remove is that
// for a clear, we leave some entries that will make it
// look like the user is still in the user list, but with
// just no messages. remove obliterates all record of a uid
// from the index.
void
clear_msg_idx_uid(vc uid)
{
    msg_idx_updated(uid, 0);
    sql_clear_uid(uid);
}

void
clear_indexed_flag(vc uid)
{
    try
    {
        sql_start_transaction();
        sql_reset_indexed_flag(uid);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }

}

namespace dwyco {
int Index_progress;
int Index_total;
}

static
void
index_user(vc v)
{
    if(dwyco::Index_total < 0)
        return;
    vc uid = v[0];
    load_msg_index(uid, 1);
    ++dwyco::Index_progress;
}



void
sql_index_all()
{
    dwyco::Index_progress = 0;
    dwyco::Index_total = 0;
    load_users(0, 0);
    dwyco::Index_total = MsgFolders.num_elems();
    try
    {
        sql_sync_off();
        MsgFolders.foreach(vcnil, index_user);
    }
    catch(...)
    {

    }
    sql_sync_on();
}

