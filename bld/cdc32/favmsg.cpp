
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
    a[0] = sql;
    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
}

static
void
init_schema()
{
    sql_simple("create table if not exists fav_msgs ("
               "from_uid text,"
               "mid text unique on conflict ignore);"

              );
    //sql_simple("create table if not exists indexed_flag (uid text primary key);");
    sql_simple("create index if not exists from_uid_idx on fav_msgs(from_uid);");
    sql_simple("create index if not exists mid_idx on fav_msgs(mid);");
    //sql_simple("create index if not exists logical_clock_idx on msg_idx(logical_clock desc);");
    //sql_simple("create index if not exists date_idx on msg_idx(date desc);");

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
    a[0] = "rollback to fav;";
    sqlite3_bulk_query(Db, &a);
}

static
void
sql_insert_record(vc from_uid, vc mid)
{
    VCArglist a;
    a[0] = "replace into fav_msgs values($1,$2);";


    a.append(to_hex(from_uid));
    a.append(mid);

    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
}

void
sql_fav_remove_uid(vc uid)
{
    try
    {
        sql_start_transaction();
        VCArglist a;
        a[0] = "delete from fav_msgs where from_uid = $1;";
        a[1] = to_hex(uid);
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
sql_fav_remove_mid(vc mid)
{
    try
    {
        sql_start_transaction();
        VCArglist a;
        a[0] = "delete from fav_msgs where mid = $1;";
        a[1] = mid;
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
sql_fav_set_fav(vc from_uid, vc mid, int fav)
{
    if(!fav)
    {
        sql_fav_remove_mid(mid);
    }
    else
    {
        sql_insert_record(from_uid, mid);
    }
}

vc
sql_fav_get_fav_set(vc from_uid)
{
    VCArglist a;
    a[0] = "select mid from fav_msgs where from_uid = $1;";
    a.append(to_hex(from_uid));

    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;

    vc ret(VC_TREE);
    int n = res.num_elems();
    for(int i = 0; i < n; ++i)
    {
        ret.add(res[i][0]);
    }

    return ret;
}

int
sql_fav_has_fav(vc from_uid)
{
    VCArglist a;
    a[0] = "select from_uid from fav_msgs where from_uid = $1 limit 1;";
    a.append(to_hex(from_uid));

    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;

    if(res.num_elems() == 0)
        return 0;
    return 1;
}

int
sql_fav_is_fav(vc mid)
{
    VCArglist a;
    a[0] = "select count(*) from fav_msgs where mid = $1;";
    a.append(mid);

    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;

    long c = (long)res[0][0];
    return c != 0;

}
}
