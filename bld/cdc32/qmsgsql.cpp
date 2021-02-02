
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifdef _Windows
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
#include "simplesql.h"
#include "qmsgsql.h"
#include "qdirth.h"

namespace dwyco {

class QMsgSql : public SimpleSql
{
public:
    QMsgSql() : SimpleSql("mi.sql") {}
    void init_schema(const DwString &schema_name);
    void init_schema_fav();
};

static QMsgSql *sDb;

static
vc
sql_simple(const char *sql, const vc& a0 = vcnil, const vc& a1 = vcnil, const vc& a2 = vcnil, const vc& a3 = vcnil, const vc& a4 = vcnil)
{
    vc res = sDb->sql_simple(sql, a0, a1, a2, a3, a4);
    return res;
}

static
vc
sql_bulk_query(const VCArglist *a)
{
    return sDb->query(a);
}

void
sql_start_transaction()
{
    sDb->start_transaction();
}


void
sql_commit_transaction()
{
    sDb->commit_transaction();
}

static
void
sql_sync_off()
{
    sDb->sync_off();
}

static
void
sql_sync_on()
{
    sDb->sync_on();

}


void
sql_rollback_transaction()
{
    sDb->rollback_transaction();
}

void
QMsgSql::init_schema_fav()
{
    try {
        start_transaction();
        sql_simple("drop table if exists mt.taglog");
        sql_simple("create table mt.taglog (mid text not null, tag text not null, guid text not null collate nocase, to_uid text not null, op text not null, unique(mid, tag, guid, to_uid, op) on conflict ignore)");
        //sql_simple("drop table if exists mt.gmt");
        sql_simple("create table if not exists mt.gmt("
                   "mid text not null, "
                   "tag text not null, "
                   "time integer not null, "
                   "uid text not null, "
                   "guid text not null collate nocase, "
                   "unique(mid, tag, uid, guid) on conflict ignore)");
        sql_simple("create index if not exists mt.gmti1 on gmt(guid)");
        sql_simple("create index if not exists mt.gmti2 on gmt(mid)");
        sql_simple("create index if not exists mt.gmti3 on gmt(tag)");
        sql_simple("create index if not exists mt.gmti4 on gmt(uid)");
        sql_simple("create table if not exists mt.gtomb (guid text not null collate nocase, time integer, unique(guid) on conflict ignore)");
        sql_simple("create index if not exists mt.gtombi1 on gtomb(guid)");
        sql_simple("create temp table crdt_tags(tag text not null)");
        sql_simple("insert into crdt_tags values('_fav')");
        sql_simple("insert into crdt_tags values('_hid')");
        //sql_simple("insert into crdt_tags values('_del')");
        //vc v = sql_simple("pragma user_version");
        //if(v[0][0] == vczero)
        commit_transaction();
    } catch(...) {
        rollback_transaction();
    }
}

void
QMsgSql::init_schema(const DwString& schema_name)
{
    if(schema_name.eq("main"))
    {
    sql_simple("pragma recursive_triggers=1");
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
    sql_simple("create table if not exists most_recent_msg (uid text primary key not null on conflict ignore, date integer not null on conflict ignore)");
    sql_simple("create table if not exists indexed_flag (uid text primary key)");
    sql_simple("create index if not exists assoc_uid_idx on msg_idx(assoc_uid)");
    sql_simple("create index if not exists logical_clock_idx on msg_idx(logical_clock desc)");
    sql_simple("create index if not exists date_idx on msg_idx(date desc)");
    sql_simple("create index if not exists sent_idx on msg_idx(is_sent)");
    sql_simple("create index if not exists att_idx on msg_idx(has_attachment)");

    // note: mid's are unique identifiers, so its meer existence here means it was
    // deleted.
    sql_simple("create table if not exists msg_tomb(mid text not null, time integer, unique(mid) on conflict ignore)");


    //sql_simple("drop table if exists gi");
    sql_simple("create table if not exists gi ("
               "date integer,"
               "mid text not null,"
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
               "assoc_uid text not null,"
               "is_local,"
               "from_client_uid not null, unique(mid, from_client_uid) on conflict ignore)"
              );

    sql_simple("create index if not exists giassoc_uid_idx on gi(assoc_uid)");
    sql_simple("create index if not exists gilogical_clock_idx on gi(logical_clock desc)");
    sql_simple("create index if not exists gidate_idx on gi(date desc)");
    sql_simple("create index if not exists gisent_idx on gi(is_sent)");
    sql_simple("create index if not exists giatt_idx on gi(has_attachment)");
    sql_simple("create index if not exists gifrom_client_uid_idx on gi(from_client_uid)");

    sql_simple("drop table if exists midlog");
    sql_simple("create table midlog (mid text not null, to_uid text not null, op text not null, unique(mid,to_uid,op) on conflict ignore)");
    }
    else if(schema_name.eq("mt"))
	{
		init_schema_fav();
	}

}

vc
sql_dump_mi()
{
    DwString fn = gen_random_filename();
    fn += ".tmp";
    fn = newfn(fn);
    sDb->attach(fn, "dump");
    sql_start_transaction();
    sql_simple("create table dump.msg_idx ("
               "date integer,"
               "mid,"
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
               "assoc_uid text not null,"
               "is_local, "
               "from_client_uid "
               ");"
              );
    sql_simple("insert into dump.msg_idx select "
               "*"
               " from main.gi");

    sql_simple("create table dump.msg_tomb (mid, time)");
    sql_simple("insert into dump.msg_tomb select * from main.msg_tomb");
    sql_commit_transaction();
    sDb->detach("dump");
    return fn.c_str();
}

vc
sql_dump_mt()
{
    DwString fn = gen_random_filename();
    fn += ".tmp";
    fn = newfn(fn);
    sDb->attach(fn, "dump");
    sql_start_transaction();
    sql_simple("create table dump.msg_tags2(mid text, tag text, time integer default 0, guid text collate nocase unique on conflict ignore)");
    sql_simple("create table dump.tomb (guid text not null collate nocase, time integer, unique(guid) on conflict replace)");
    // note: we only send "user generated" tags. also some tags are completely local, like "unviewed" and "remote" which we
    // don't really want to send at all.
    sql_simple("insert into dump.msg_tags2 select "
               "mid, "
               "tag, "
               "time, "
               "guid "
               "from mt.gmt where tag in (select * from crdt_tags)");
    sql_simple("insert into dump.tomb select * from mt.gtomb");
    sql_commit_transaction();
    sDb->detach("dump");
    return fn.c_str();

}

static
DwString
make_sql_args(int n)
{
    DwString ret;
    int i;
    for(i = 1; i < n; ++i)
    {
        ret += "?";
        ret += DwString::fromInt(i);
        ret += ",";
    }
    ret += "?";
    ret += DwString::fromInt(i);
    return ret;
}

vc
package_downstream_sends(vc remote_uid)
{
    vc huid = to_hex(remote_uid);
    try
    {
        // NOTE: may want to package this as a single command
        // containing both index and tag updates, then perform all of them
        // under a transaction. may not be necessary, but just a thought
        sql_start_transaction();
        vc idxs = sql_simple("select msg_idx.*, midlog.op from main.msg_idx, main.midlog where midlog.mid = msg_idx.mid and midlog.to_uid = ?1 and op = 'a'", huid);
        vc mtombs = sql_simple("select mid, op from main.midlog where midlog.to_uid = ?1 and op = 'd'", huid);
        vc tags = sql_simple("select mt2.mid, mt2.tag, mt2.time, mt2.guid, tl.op from mt.gmt as mt2, mt.taglog as tl where mt2.mid = tl.mid and mt2.tag = tl.tag and to_uid = ?1 and op = 'a'", huid);
        vc tombs = sql_simple("select tl.guid,tl.mid,tl.tag,tl.op from mt.taglog as tl where to_uid = ?1 and op = 'd'", huid);
        if(idxs.num_elems() > 0 || mtombs.num_elems() > 0 || tags.num_elems() > 0 || tombs.num_elems() > 0)
            GRTLOGA("downstream idx %d mtomb %d tag %d ttomb %d", idxs.num_elems(), mtombs.num_elems(), tags.num_elems(), tombs.num_elems(), 0);
        vc ret(VC_VECTOR);
        for(int i = 0; i < idxs.num_elems(); ++i)
        {
            vc cmd(VC_VECTOR);
            cmd[0] = "iupdate";
            cmd[1] = idxs[i];
            ret.append(cmd);
        }
        for(int i = 0; i < mtombs.num_elems(); ++i)
        {
            vc cmd(VC_VECTOR);
            cmd[0] = "iupdate";
            cmd[1] = mtombs[i];
            ret.append(cmd);
        }
        sql_simple("delete from midlog where to_uid = ?1", huid);
        for(int i = 0; i < tags.num_elems(); ++i)
        {
            vc cmd(VC_VECTOR);
            cmd[0] = "tupdate";
            cmd[1] = tags[i];
            ret.append(cmd);
        }
        for(int i = 0; i < tombs.num_elems(); ++i)
        {
            vc cmd(VC_VECTOR);
            cmd[0] = "tupdate";
            cmd[1] = tombs[i];
            ret.append(cmd);
        }
        sql_simple("delete from taglog where to_uid = ?1", huid);
        sql_commit_transaction();
        return ret;
    } catch (...)
    {
        sql_rollback_transaction();
        return vcnil;
    }

}

// note: this is kind of expensive, but useful during debugging to
// make sure the file system messages exactly mirror the contents of the
// index.

static
void
sync_user(vc v)
{

    vc uid = v[0];

    vc id = uid_to_dir(uid);

    DwString s((const char *)id);
    s = newfn(s);
    DwString ss = s;
    s += "" DIRSEPSTR "*.*";

    FindVec& fv = *find_to_vec(s.c_str());
    auto n = fv.num_elems();
    for(int i = 0; i < n; ++i)
    {
        WIN32_FIND_DATA &d = *fv[i];
        if(strlen(d.cFileName) != 24)
            continue;
        DwString mid(d.cFileName);
        mid.remove(20);
        vc v = sql_simple("select 1 from msg_idx where mid = ?1", mid.c_str());
        if(v.num_elems() == 0)
            delete_body3(uid, mid.c_str(), 1);

    }
    delete_findvec(&fv);


}



static
void
sync_files()
{

    load_users(0, 0);

    try
    {
        sql_start_transaction();
        sql_simple("delete from msg_idx where mid in (select mid from msg_tomb)");
        MsgFolders.foreach(vcnil, sync_user);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }
}


// note: this is for testing, we're just assuming the index
// has been materialized here so we can investigate things
void
import_remote_mi(vc remote_uid)
{
    vc huid = to_hex(remote_uid);
    DwString fn = DwString("mi%1.sql").arg((const char *)huid);
    DwString favfn = DwString("fav%1.sql").arg((const char *)huid);

    sDb->attach(fn, "mi2");
    sDb->attach(favfn, "fav2");

    try
    {
        sql_start_transaction();
        vc newuids = sql_simple("select distinct(assoc_uid) from mi2.msg_idx except select distinct(assoc_uid) from main.gi");
        for(int i = 0; i < newuids.num_elems(); ++i)
        {
            se_emit(SE_USER_ADD, from_hex(newuids[i][0]));
        }
        sql_simple("delete from crdt_tags");
        sql_simple("delete from current_clients where uid = ?1", huid);

        //sql_simple("delete from main.gi where from_client_uid = ?1", huid);
        //sql_simple("delete from mt.gmt where uid = ?1", huid);

        sql_simple("insert or ignore into main.gi select * from mi2.msg_idx");
        sql_simple("insert or ignore into main.msg_tomb select * from mi2.msg_tomb");
        sql_simple("delete from main.gi where mid in (select mid from msg_tomb)");

        sync_files();

        sql_simple("insert or ignore into mt.gtomb select guid, time from fav2.tomb");
        sql_simple("insert or ignore into mt.gmt select mid, tag, time, ?1, guid from fav2.msg_tags2", huid);
        sql_simple("delete from mt.gmt where mid in (select mid from msg_tomb)");
        sql_simple("delete from mt.gmt where guid in (select guid from mt.gtomb)");
        sql_simple("insert into crdt_tags values('_fav')");
        sql_simple("insert into crdt_tags values('_hid')");
        sql_simple("insert into current_clients values(?1)", huid);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }

    sDb->detach("mi2");
    sDb->detach("fav2");

}

void
import_remote_iupdate(vc remote_uid, vc vals)
{
    vc huid = to_hex(remote_uid);
    DwString fn = DwString("mi%1.sql").arg((const char *)huid);
    DwString favfn = DwString("fav%1.sql").arg((const char *)huid);

    sDb->attach(fn, "mi2");
    sDb->attach(favfn, "fav2");

    try
    {
        sql_start_transaction();
        // this keeps us from bouncing it back to the client we just
        // received this from
        sql_simple("delete from current_clients where uid = ?1", huid);
        vc op = vals.remove_last();
        if(op == vc("a"))
        {
            vals.append(0);
            vals.append(huid);
            DwString sargs = make_sql_args(vals.num_elems());
            VCArglist a;
            a.set_size(vals.num_elems() + 1);
            a.append(DwString("insert or ignore into main.gi values(%1)").arg(sargs).c_str());
            for(int i = 0; i < vals.num_elems(); ++i)
                a.append(vals[i]);
            sql_bulk_query(&a);
        }
        else if(op == vc("d"))
        {
            vc mid = vals[0];
            sql_simple("insert into msg_tomb (mid, time) values(?1, strftime('%s', 'now'))", mid);
            sql_simple("delete from mi2.msg_idx where mid = ?1", mid);
            sql_simple("delete from gi where mid = ?1", mid);
            // NOTE: if the transaction completes, need to delete the file too
        }
        sql_simple("insert into current_clients values(?1)", huid);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }

    sDb->detach("mi2");
    sDb->detach("fav2");
}

void
import_remote_tupdate(vc remote_uid, vc vals)
{
    vc huid = to_hex(remote_uid);
    DwString fn = DwString("mi%1.sql").arg((const char *)huid);
    DwString favfn = DwString("fav%1.sql").arg((const char *)huid);

    sDb->attach(fn, "mi2");
    sDb->attach(favfn, "fav2");

    try
    {
        sql_start_transaction();
        // this keeps us from bouncing it back to the client we just
        // received this from
        sql_simple("delete from current_clients where uid = ?1", huid);

        vc op = vals.remove_last();
        if(op == vc("a"))
        {
            vc mid = vals[0];
            vc tag = vals[1];
            vc tm = vals[2];
            vc guid = vals[3];

            sql_simple("insert or ignore into mt.gmt (mid, tag, time, uid, guid) select ?1, ?2, ?3, ?4, ?5 where not exists (select 1 from mt.gtomb where ?5 = guid)", mid, tag, tm, huid, guid);
            if(tag == vc("_hid") || tag == vc("_fav"))
            {
                vc uid = sql_get_uid_from_mid(mid);
                if(!uid.is_nil())
                    se_emit_msg_tag_change(mid, from_hex(uid));
            }
        }
        else if (op == vc("d"))
        {
            vc guid = vals[0];
            vc mid = vals[1];
            vc tag = vals[2];
            sql_simple("insert or ignore into mt.gtomb(guid, time) values(?1, strftime('%s', 'now'))", guid);
            sql_simple("delete from gmt where guid = ?1", guid);
            if(tag == vc("_hid") || tag == vc("_fav"))
            {
                vc uid = sql_get_uid_from_mid(mid);
                if(!uid.is_nil())
                    se_emit_msg_tag_change(mid, from_hex(uid));
            }
        }
        else
            oopanic("bad tupdate");
        sql_simple("insert into current_clients values(?1)", huid);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }

    sDb->detach("mi2");
    sDb->detach("fav2");
}

void
init_qmsg_sql()
{
    if(sDb)
        oopanic("already init");
    sDb = new QMsgSql;
    if(!sDb->init())
        throw -1;
    sql_sync_off();
    vc hmyuid = to_hex(My_UID);
    sDb->attach("fav.sql", "mt");
    sql_start_transaction();
    //sql_simple("insert into mt.gmt (mid, tag, uid, guid, time) values(?2, 'bar', ?1, 'mumble', 0)", hmyuid);
    // by default, our local index "local" is implied
    sql_simple("insert into gi select *, 1, ?1 from msg_idx", hmyuid);

    sql_simple("create temp table rescan(flag integer)");
    sql_simple("insert into rescan (flag) values(0)");
    sql_simple("create temp trigger rescan1 after insert on main.msg_tomb begin update rescan set flag = 1; end");
    sql_simple("create temp trigger rescan2 after delete on main.msg_tomb begin update rescan set flag = 1; end");
    sql_simple("create temp trigger rescan3 after delete on main.msg_idx begin update rescan set flag = 1; end");
    sql_simple("create temp trigger rescan4 after insert on main.msg_idx begin update rescan set flag = 1; end");

    sql_simple("drop table if exists current_clients");
    sql_simple("create table current_clients(uid text collate nocase unique on conflict ignore not null on conflict fail)");

    sql_simple("drop trigger if exists miupdate");
    sql_simple("create trigger if not exists miupdate after insert on main.msg_idx begin insert into midlog (mid,to_uid,op) select new.mid, uid, 'a' from current_clients; end");
    sql_simple("drop trigger if exists mt.tagupdate");
    sql_simple("create temp trigger if not exists tagupdate after insert on gmt begin insert into taglog (mid, tag, guid,to_uid,op) "
               "select new.mid, new.tag, new.guid, uid, 'a' from current_clients where new.tag in (select * from crdt_tags); end");

    sql_simple("drop trigger if exists xgi");
    sql_simple(DwString("create trigger if not exists xgi after insert on main.msg_idx begin insert into gi select *, 1, '%1' from msg_idx where mid = new.mid limit 1; end").arg((const char *)hmyuid).c_str());
    sql_simple("drop trigger if exists dgi");
    sql_simple("create trigger if not exists dgi after delete on main.msg_idx begin "
               "delete from gi where mid = old.mid; "
               "insert into msg_tomb (mid, time) values(old.mid, strftime('%s', 'now')); "
               //"insert into midlog (mid,to_uid,op) select old.mid, uid, 'd' from current_clients; "
               "end");
    sql_simple("create temp trigger mtomb_log after insert on msg_tomb begin "
               "insert into midlog(mid,to_uid,op) select new.mid, uid, 'd' from current_clients; "
               "end"
               );
    sql_simple("drop trigger if exists mt.xgmt");
    sql_simple("drop trigger if exists mt.dgmt");

    sql_simple(DwString("create temp trigger if not exists dgmt after delete on mt.gmt begin "
                        "insert into taglog (mid, tag, guid,to_uid,op) select old.mid, old.tag, old.guid, uid, 'd' from current_clients,crdt_tags where old.tag = tag; "
                        "insert into gtomb(guid, time) select old.guid, strftime('%s', 'now') from crdt_tags where old.tag = tag; "
                        "end").arg((const char *)hmyuid).c_str());

    sql_simple("create temp trigger rescan5 after insert on main.gi begin update rescan set flag = 1; end");
    sql_simple("create temp trigger rescan6 after insert on mt.gmt begin update rescan set flag = 1; end");
    sql_simple("create temp trigger rescan7 after delete on main.gi begin update rescan set flag = 1; end");
    sql_simple("create temp trigger rescan8 after delete on mt.gmt begin update rescan set flag = 1; end");
    sql_commit_transaction();

#if 0
    // this is a hack for debugging, load existing data indexes. we expect we won't
    // obliterate gi and gmt on each restart in the future
    FindVec *fv = find_to_vec(newfn("mi????????????????????.sql").c_str());
    for(int i = 0; i < fv->num_elems(); ++i)
    {
        WIN32_FIND_DATA *w = (*fv)[i];
        DwString a(w->cFileName);
        a.erase(0, 2);
        a.erase(20, 4);
        vc uid = from_hex(a.c_str());
        import_remote_mi(uid);
    }
    delete_findvec(fv);
#endif
}

void
exit_qmsg_sql()
{
    if(!sDb)
        return;
    sDb->exit();
}

static
void
sql_insert_record(vc entry, vc assoc_uid)
{
    VCArglist a;
    a.set_size(NUM_QM_IDX_FIELDS + 2);
    a.append("insert or ignore into msg_idx values(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13)");

    for(int i = 0; i < NUM_QM_IDX_FIELDS; ++i)
        a.append(entry[i]);
    a.append(to_hex(assoc_uid));

    sql_bulk_query(&a);
}

//static
//void
//sql_record_most_recent(vc uid)
//{
//    vc huid = to_hex(uid);
//    vc r = sql_simple("select max(date) from msg_idx where assoc_uid = ?1 limit 1", huid);
//    if(r[0][0].is_nil())
//    {
//        sql_simple("insert or replace into most_recent_msg (uid, date) values(?1, 0)", huid);
//    }
//    else
//    {
//        sql_simple("insert or replace into most_recent_msg (uid, date) values(?1, ?2)", huid, r[0][0]);
//    }
//}

static
void
sql_delete_mid(vc mid)
{
    sql_simple("delete from msg_idx where mid = ?1", mid);
}

long
sql_get_max_logical_clock()
{
    vc res = sql_simple("select max(logical_clock) from msg_idx");
    if(res[0][0].type() != VC_INT)
        return 0;
    return (long)res[0][0];
}

int
sql_is_mid_local(vc mid)
{
    vc res = sql_simple("select 1 from msg_idx where mid = ?1", mid);
    if(res.num_elems() == 0)
        return 0;
    return 1;
    int ret = (long)res[0][0];
    return ret;
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
        sql_simple("create temp table foo as select max(date), assoc_uid from gi group by assoc_uid;");
        //sql_simple("insert into foo select date, uid from most_recent_msg;");
        vc res;
        if(total_out)
        {
            VCArglist a;
            res = sql_simple("select count(distinct assoc_uid) from foo");
            *total_out = (int)res[0][0];
        }

#ifdef ANDROID
        res = sql_simple("select distinct assoc_uid from foo order by \"max(date)\" desc limit 20;");
#else
        res = sql_simple("select distinct assoc_uid from foo order by \"max(date)\" desc limit 100;");
#endif
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
    try
    {
        sql_start_transaction();
        sql_simple("create temp table foo as select max(date), assoc_uid from gi group by assoc_uid");
        //sql_simple("insert into foo select date, uid from most_recent_msg");
        vc res;
        res = sql_simple("select distinct assoc_uid from foo where strftime('%s', 'now') - \"max(date)\" < ?1 order by \"max(date)\" desc limit ?2;",
                         max_age, max_count);
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
    try
    {
        sql_start_transaction();
        sql_simple("create temp table foo as select distinct assoc_uid from msg_idx;");
        sql_simple("delete from foo where exists(select * from msg_idx where foo.assoc_uid = assoc_uid and is_sent = 't');");
        sql_simple("delete from foo where exists(select * from msg_idx where foo.assoc_uid = assoc_uid and is_sent isnull and strftime('%s', 'now') - date < 14 * 24 * 3600);");
        sql_simple("delete from foo where (select count(*) from msg_idx where foo.assoc_uid = assoc_uid and has_attachment notnull and is_sent isnull limit 3) > 2;");
        sql_simple("delete from foo where (select count(*) from msg_idx where foo.assoc_uid = assoc_uid limit 5) > 4;");
        vc res = sql_simple("select * from foo;");
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
    return vc(VC_VECTOR);

//    try
//    {
//        sql_start_transaction();
//        vc res = sql_simple("select uid from indexed_flag where not exists (select 1 from msg_idx where uid = assoc_uid);");
//        sql_commit_transaction();
//        vc ret(VC_VECTOR);
//        for(int i = 0; i < res.num_elems(); ++i)
//            ret.append(res[i][0]);
//        return ret;
//    }
//    catch(...)
//    {
//        sql_rollback_transaction();
//        return vcnil;
//    }

}

vc
sql_get_no_response_users()
{
    return vc(VC_VECTOR);

//    try
//    {
//        sql_start_transaction();

//        vc res = sql_simple("select uid from indexed_flag where not exists(select 1 from msg_idx where uid = assoc_uid and is_sent isnull)");
//        sql_commit_transaction();
//        vc ret(VC_VECTOR);
//        for(int i = 0; i < res.num_elems(); ++i)
//            ret.append(res[i][0]);
//        return ret;
//    }
//    catch(...)
//    {
//        sql_rollback_transaction();
//        return vcnil;
//    }

}

static
void
sql_remove_uid(vc uid)
{
    try
    {
        sql_start_transaction();
        vc huid = to_hex(uid);
        //sql_simple("delete from indexed_flag where uid = ?1", huid);
        sql_simple("delete from msg_idx where assoc_uid = ?1", huid);
        //sql_simple("delete from most_recent_msg where uid = ?1", huid);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }

}

static
vc
sql_clear_uid(vc uid)
{

    try
    {
        sql_start_transaction();
        vc huid = to_hex(uid);
        vc mids = sql_simple("select distinct(mid) from gi where assoc_uid = ?1", huid);
        sql_simple("delete from msg_idx where assoc_uid = ?1", huid);
        // note: there is a trigger to delete msg_idx things
        // this just gets all the stuff we don't have downloaded here
        sql_simple("insert into msg_tomb select mid, strftime('%s', 'now') from gi where assoc_uid = ?1", huid);
        sql_simple("delete from gi where assoc_uid = ?1", huid);
        sql_commit_transaction();
        return mids;
    }
    catch(...)
    {
        sql_rollback_transaction();
        return vc(VC_VECTOR);
    }


}

static
void
sql_insert_indexed_flag(vc uid)
{
    sql_simple("insert or replace into indexed_flag values(?1)", to_hex(uid));
}

static
int
sql_check_indexed_flag(vc uid)
{
    vc res = sql_simple("select count(*) from indexed_flag where uid = ?1", to_hex(uid));
    if(res[0][0] == vc(0))
        return 0;
    return 1;
}

static
void
sql_reset_indexed_flag(vc uid)
{
    sql_simple("delete from indexed_flag where uid = ?1", to_hex(uid));
}



static
vc
sql_load_index(vc uid, int max_count)
{
    vc res = sql_simple("select date, mid, is_sent, is_forwarded, is_no_forward, is_file, special_type, "
           "has_attachment, att_has_video, att_has_audio, att_is_short_video, logical_clock, assoc_uid "
           " from gi where assoc_uid = ?1 and not exists (select 1 from msg_tomb as tmb where gi.mid = tmb.mid) group by mid order by logical_clock desc limit ?2",
                        to_hex(uid), max_count);
    return res;
}

static
int
sql_count_index(vc uid)
{
    vc res = sql_simple("select count(distinct(mid)) from gi where assoc_uid = ?1", to_hex(uid));
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
    nentry[QM_IDX_SPECIAL_TYPE] = body[QM_BODY_SPECIAL_TYPE].type() == VC_VECTOR ? body[QM_BODY_SPECIAL_TYPE][0] : vcnil;
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
        auto n = fv.num_elems();
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
        //sql_record_most_recent(uid);
        sql_simple("delete from midlog");
        sql_simple("delete from taglog");
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

// return all index records for messages
// associated with uid and after logical clock
// used for incrementally updating models
vc
msg_idx_get_new_msgs(vc uid, vc logical_clock)
{
    try
    {
        sql_start_transaction();
        VCArglist a;
        vc res = sql_simple("select date, mid, is_sent, is_forwarded, is_no_forward, is_file, special_type, "
               "has_attachment, att_has_video, att_has_audio, att_is_short_video, logical_clock, assoc_uid "
               " from msg_idx where assoc_uid = ?1 and logical_clock > ?2 order by logical_clock desc",
                   to_hex(uid), logical_clock);
        sql_commit_transaction();
        return res;
    }
    catch(...)
    {
        sql_rollback_transaction();
        return vcnil;
    }

}

vc
sql_get_uid_from_mid(vc mid)
{
    try
    {
        sql_start_transaction();
        vc res = sql_simple("select assoc_uid from gi where mid = ?1 limit 1", mid);
        sql_commit_transaction();
        if(res.num_elems() != 1)
            return vcnil;
        return res[0][0];
    }
    catch(...)
    {
        sql_rollback_transaction();
        return vcnil;
    }
}

static
vc
flatten(vc sql_res)
{
    vc ret(VC_VECTOR);
    for(int i = 0; i < sql_res.num_elems(); ++i)
    {
        ret[i] = sql_res[i][0];
    }
    return ret;
}

// this looks in all the loaded global indexes to see what uid's
// might have the mid in their local cache
vc
sql_get_uid_from_mid2(vc mid)
{
    try
    {
        sql_start_transaction();
        vc res = sql_simple("select distinct(from_client_uid) from gi where mid = ?1", mid);
        sql_commit_transaction();
        if(res.num_elems() == 0)
            return vcnil;
        return flatten(res);
    }
    catch(...)
    {
        sql_rollback_transaction();
        return vcnil;
    }
}

// this returns a list of all uid, mid pairs where mid is not local
// this would be used in cases where we want to pull other messages
// in an "eager" fashion rather than waiting for them to be requested
// via some model lookup

vc
sql_get_non_local_messages()
{

    try
    {
        sql_start_transaction();
        sql_simple("create temp table foo as select mid, from_client_uid from gi where mid not in (select mid from msg_idx)");
        sql_simple("delete from foo where mid in (select mid from msg_tomb)");
        vc res = sql_simple("select * from foo");
        sql_simple("drop table foo");
        sql_commit_transaction();
        return res;
    }
    catch (...)
    {
        sql_rollback_transaction();
        return vcnil;
    }
}

// this gets a list of mids that we don't have, that look like they might
// be available at the given uid. when we connect to this uid with a sync
// channel, we can assert pulls to this uid if we are in "eager" sync mode.
// XXX WARNING, UNTESTED
vc
sql_get_non_local_messages_at_uid(vc uid)
{
    vc huid = to_hex(uid);
    try
    {
        sql_start_transaction();
        sql_simple("create temp table foo as select mid from gi where from_client_uid = ?1 "
                   "and not exists (select 1 from msg_idx where gi.mid = mid)", huid);
        sql_simple("delete from foo where exists (select 1 from msg_tomb where mid = foo.mid)");
        vc res = sql_simple("select * from foo");
        sql_simple("drop table foo");
        sql_commit_transaction();
        return flatten(res);
    }
    catch (...)
    {
        sql_rollback_transaction();
        return vcnil;
    }
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
update_msg_idx(vc recip, vc body, int inhibit_sysmsg)
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
        //sql_record_most_recent(uid);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }
    if(!inhibit_sysmsg)
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
    try
    {
        sql_start_transaction();
        vc res = sql_simple("select mid as foo from gi where assoc_uid = ?1 "
                 "and not exists (select 1 from gmt where mid = foo and tag = '_fav') group by mid",
                            to_hex(uid));
        sql_commit_transaction();
        int n = res.num_elems();
        for(int i = 0; i < n; ++i)
        {
            vc mid = res[i][0];
            ret.append(mid);
        }
    }
    catch(...)
    {
        sql_rollback_transaction();
    }
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
    vc mids = sql_clear_uid(uid);
    for(int i = 0; i < mids.num_elems(); ++i)
    {
        dirth_send_addtag(uid, mids[i][0], "_del", QckDone(0, 0));
    }
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


int Index_progress;
int Index_total;


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
        sql_start_transaction();
        MsgFolders.foreach(vcnil, index_user);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }
    sql_sync_on();
}

// FAVMSG


static
void
sql_insert_record_mt(vc mid, vc tag)
{
    sql_simple("insert into mt.gmt (mid, tag, time, guid, uid) values(?1,?2,strftime('%s','now'), lower(hex(randomblob(10))), ?3)",
               mid, tag, to_hex(My_UID));
}

void
sql_add_tag(vc mid, vc tag)
{
    try
    {
        sql_start_transaction();
        sql_insert_record_mt(mid, tag);
        sql_commit_transaction();
    }
    catch (...)
    {
        sql_rollback_transaction();
    }
}

void
sql_remove_tag(vc tag)
{
    try
    {
        sql_start_transaction();
        sql_simple("delete from gmt where tag = ?1", tag);
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
    try
    {
        sql_start_transaction();
        // find all crdt tags associated with all mids, and perform crdt related things for each mid
        sql_simple("delete from gmt where mid in (select distinct(mid) from gi where assoc_uid = ?1)",
                            to_hex(uid));
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
        // find all crdt tags, and perform crdt related things for each mid
        sql_simple("delete from gmt where mid = ?1", mid);
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
        // if the tag is a crdt tag, perform ops for that tag
        sql_simple("delete from gmt where mid = ?1 and tag = ?2", mid, tag);
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
        sql_insert_record_mt(mid, "_fav");
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
    vc res = sql_simple("select 1 from gmt where mid = ?1 and tag = '_fav' and not exists(select 1 from mt.gtomb where guid = gmt.guid) limit 1;", mid);
    return res.num_elems() > 0;
}

// this one only returns mids that have been downloaded and indexed
vc
sql_get_tagged_mids(vc tag)
{
    vc res;
    try
    {
        sql_start_transaction();
        sql_simple("create temp table foo as select assoc_uid, mid from gmt,gi using(mid) where tag = ?1 "
                   "and not exists(select 1 from mt.gtomb where gmt.guid = guid) "
                         "group by mid "
                         "order by logical_clock asc",
                         tag);
        sql_simple("delete from foo where mid in (select mid from msg_tomb)");
        res = sql_simple("select * from foo");
        sql_simple("drop table foo");
        sql_commit_transaction();
    }
    catch (...)
    {
        sql_rollback_transaction();
        res = vc(VC_VECTOR);
    }
    return res;
}

// this returns all mids with given tag regardless of download state
vc
sql_get_tagged_mids2(vc tag)
{
    vc res;
    try
    {
        res = sql_simple("select distinct(mid) from gmt where tag = ?1 and not exists(select 1 from gtomb where gmt.guid = guid)",
                         tag);
    }
    catch (...)
    {
        res = vc(VC_VECTOR);
    }
    return res;
}

vc
sql_get_tagged_idx(vc tag)
{
    vc res;
    try
    {
        sql_start_transaction();
        sql_simple("create temp table foo as select "
                 "date, mid, is_sent, is_forwarded, is_no_forward, is_file, special_type, "
                 "has_attachment, att_has_video, att_has_audio, att_is_short_video, logical_clock, assoc_uid "
                 " from gmt,gi using(mid) where tag = ?1 and not exists(select 1 from mt.gtomb where guid = gmt.guid)"
                         "group by mid "
                         "order by logical_clock desc",
                            tag);
        sql_simple("delete from foo where mid in (select mid from msg_tomb)");
        res = sql_simple("select * from foo");
        sql_simple("drop table foo");
        sql_commit_transaction();
    }
    catch (...)
    {
        sql_rollback_transaction();
        res = vc(VC_VECTOR);
    }
    return res;
}

vc
sql_get_all_idx()
{
    vc res;
    try
    {
        res = sql_simple("select "
                 "date, mid, is_sent, is_forwarded, is_no_forward, is_file, special_type, "
                 "has_attachment, att_has_video, att_has_audio, att_is_short_video, logical_clock, assoc_uid "
                 " from msg_idx order by logical_clock desc");
    }
    catch (...)
    {
        res = vc(VC_VECTOR);
    }
    return res;
}

int
sql_mid_has_tag(vc mid, vc tag)
{
    VCArglist a;
    vc res = sql_simple("select 1 from gmt where mid = ?1 and tag = ?2 and not exists(select 1 from mt.gtomb where guid = gmt.guid) limit 1",
                        mid, tag);
    return res.num_elems() > 0;

}

int
sql_uid_has_tag(vc uid, vc tag)
{
    int c = 0;
    try
    {
        vc res = sql_simple("select 1 from gmt,gi using(mid) where assoc_uid = ?1 and tag = ?2 and not exists(select 1 from gtomb where guid = gmt.guid) limit 1",
                            to_hex(uid), tag);
        c = (res.num_elems() > 0);
    }
    catch(...)
    {

    }
    return c;

}

// NOTE: if a message hasn't been indexed (ie, it hasn't been downloaded and filed
// in the file system yet), it will not show up in this count.
int
sql_uid_count_tag(vc uid, vc tag)
{
    int c = 0;
    try
    {
        vc res = sql_simple("select count(distinct mid) from gmt,gi using(mid) where assoc_uid = ?1 and tag = ?2 and not exists (select 1 from gtomb where guid = gmt.guid)",
                            to_hex(uid), tag);
        c = res[0][0];
    }
    catch(...)
    {

    }
    return c;

}

// this counts a tag whether or not the tag is associated with a particular uid,
// so it is useful for counting messages that have not been downloaded yet
int
sql_count_tag(vc tag)
{
    int c = 0;
    try
    {
        vc res = sql_simple("select count(distinct mid) from gmt where tag = ?1 and not exists (select 1 from gtomb where guid = gmt.guid)", tag);
        c = res[0][0];
    }
    catch(...)
    {

    }
    return c;
}

void
sql_set_rescan(int r)
{
    sql_simple("update rescan set flag = ?1", vc(r));
}

int
sql_get_rescan()
{
    vc res = sql_simple("select flag from rescan");
    return (int)res[0][0];
}

// WARNING: this is a useful api for debugging, it should not be in the
// api generally. it is useful for running arbitrary sql from an app
// assuming the app knows the schema.

int
sql_run_sql(vc s, vc a1, vc a2, vc a3)
{
    try {
        sql_start_transaction();
        sql_simple(s, a1, a2, a3);
        sql_commit_transaction();
        return 1;
    } catch (...) {
        sql_rollback_transaction();
        return 0;
    }
}
}

