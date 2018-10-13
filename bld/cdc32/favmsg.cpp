
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
#include <fcntl.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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

vc vclh_serialize(vc);

namespace dwyco {

#define USER_BOMB(a, b) {return (b);}
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
sql_start_transaction()
{
    sql_simple("savepoint fav;");
}

static
void
sql_commit_transaction()
{
    sql_simple("release fav;");
}

static
void
sql_rollback_transaction()
{
    VCArglist a;
    a.append("rollback to fav;");
    sqlite3_bulk_query(Db, &a);
}


static
void
init_schema()
{
    sql_simple("create table if not exists fav_msgs ("
               "from_uid text,"
               "mid text unique on conflict ignore);"
              );
    sql_simple("create table if not exists msg_tags(from_uid text, mid text, tag text, unique(from_uid, mid, tag) on conflict ignore)");


    sql_simple("create index if not exists from_uid_idx on fav_msgs(from_uid);");
    sql_simple("create index if not exists mid_idx on fav_msgs(mid);");

    sql_simple("create index if not exists mt_from_uid_idx on msg_tags(from_uid)");
    sql_simple("create index if not exists mt_mid_idx on msg_tags(mid)");
    sql_simple("create index if not exists mt_tag_idx on msg_tags(tag)");

    sql_simple("insert into msg_tags (from_uid, mid, tag) select from_uid, mid, '_fav' from fav_msgs");
    sql_simple("delete from fav_msgs");

    try {
        sql_start_transaction();
        sql_simple("create table if not exists msg_tags2(mid text, tag text, unique(mid, tag) on conflict ignore)");
        sql_simple("insert into msg_tags2 (mid, tag) select mid, tag from msg_tags");
        sql_simple("delete from msg_tags");
        sql_simple("create index if not exists mt2_mid_idx on msg_tags2(mid)");
        sql_simple("create index if not exists mt2_tag_idx on msg_tags2(tag)");
        sql_commit_transaction();
    } catch(...) {
        sql_rollback_transaction();
    }
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

void
init_fav_sql()
{
    if(Db)
        oopanic("already init");
    if(sqlite3_open(newfn("fav.sql").c_str(), &Db) != SQLITE_OK)
    {
        Db = 0;
        return;
    }
    init_schema();
    sql_sync_off();
}

void
exit_fav_sql()
{
    if(!Db)
        return;
    sqlite3_close_v2(Db);
    Db = 0;
}

static
void
sql_insert_record(vc mid, vc tag)
{
    VCArglist a;
    a.append("replace into msg_tags2 (mid, tag) values($1,$2);");
    a.append(mid);
    a.append(tag);

    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
}

void
sql_add_tag(vc mid, vc tag)
{
    sql_insert_record(mid, tag);
}

void
sql_remove_tag(vc tag)
{
    try
    {
        sql_start_transaction();
        VCArglist a;
        a.append("delete from msg_tags2 where tag = $1;");
        a.append(tag);
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

void
sql_fav_remove_uid(vc uid)
{
    DwString mfn = newfn("mi.sql");
    sql_simple(DwString("attach '%1' as mi;").arg(mfn).c_str());
    try
    {
        sql_start_transaction();
        VCArglist a;
        a.append("delete from msg_tags2 where mid in (select mid from msg_idx where assoc_uid = $1)");
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
    sql_simple("detach mi;");

}

void
sql_fav_remove_mid(vc mid)
{
    try
    {
        sql_start_transaction();
        VCArglist a;
        a.append("delete from msg_tags2 where mid = $1;");
        a.append(mid);
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

void
sql_remove_mid_tag(vc mid, vc tag)
{
    try
    {
        sql_start_transaction();
        VCArglist a;
        a.append("delete from msg_tags2 where mid = $1 and tag = $2;");
        a.append(mid);
        a.append(tag);
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

void
sql_fav_set_fav(vc mid, int fav)
{
    if(!fav)
    {
        sql_remove_mid_tag(mid, "_fav");
    }
    else
    {
        sql_insert_record(mid, "_fav");
    }
}

int
sql_fav_has_fav(vc from_uid)
{
    return sql_uid_has_tag(from_uid, "_fav");
}

int
sql_fav_is_fav(vc mid)
{
    VCArglist a;
    a.append("select count(*) from msg_tags2 where mid = $1 and tag = '_fav' limit 1;");
    a.append(mid);

    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;

    long c = (long)res[0][0];
    return c != 0;

}

vc
sql_get_tagged_mids(vc tag)
{
    DwString mfn = newfn("mi.sql");
    sql_simple(DwString("attach '%1' as mi;").arg(mfn).c_str());
    vc res;
    try {
        VCArglist a;
        a.append("select assoc_uid, mid from msg_tags2,mi.msg_idx using(mid) where tag = $1 order by logical_clock asc;");
        a.append(tag);

        res = sqlite3_bulk_query(Db, &a);
        if(res.is_nil())
            throw -1;
    } catch (...) {
        res = vc(VC_VECTOR);
    }
    sql_simple("detach mi;");

    return res;

}

vc
sql_get_tagged_idx(vc tag)
{
    DwString mfn = newfn("mi.sql");
    sql_simple(DwString("attach '%1' as mi;").arg(mfn).c_str());
    vc res;
    try {
        VCArglist a;
        a.append("select "
                 "date, mid, is_sent, is_forwarded, is_no_forward, is_file, special_type, "
                 "has_attachment, att_has_video, att_has_audio, att_is_short_video, logical_clock, assoc_uid "
                 " from msg_tags2,mi.msg_idx using(mid) where tag = $1 order by logical_clock desc;");
        a.append(tag);

        res = sqlite3_bulk_query(Db, &a);
        if(res.is_nil())
            throw -1;
    } catch (...) {
        res = vc(VC_VECTOR);
    }
    sql_simple("detach mi;");

    return res;

}

int
sql_mid_has_tag(vc mid, vc tag)
{
    VCArglist a;
    a.append("select count(*) from msg_tags2 where mid = $1 and tag = $2 limit 1;");
    a.append(mid);
    a.append(tag);

    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;

    long c = (long)res[0][0];
    return c != 0;

}

int
sql_uid_has_tag(vc uid, vc tag)
{
    DwString mfn = newfn("mi.sql");
    sql_simple(DwString("attach '%1' as mi;").arg(mfn).c_str());
    long c = 0;
    try {
        VCArglist a;
        a.append("select count(*) from msg_tags2,mi.msg_idx using(mid) where assoc_uid = $1 and tag = $2 limit 1;");
        a.append(to_hex(uid));
        a.append(tag);

        vc res = sqlite3_bulk_query(Db, &a);
        if(res.is_nil())
            throw -1;
        c = (long)res[0][0];
    }
    catch(...) {

    }
    sql_simple("detach mi;");

    return c != 0;

}
}
