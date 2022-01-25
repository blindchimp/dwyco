
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
#include "simplesql.h"
#include "qmsgsql.h"
#include "qdirth.h"
#include "dwbag.h"
#include "dhgsetup.h"
#include "profiledb.h"
#include "dirth.h"
#include "pulls.h"

namespace dwyco {
using namespace dwyco::qmsgsql;
DwString Schema_version_hack;

class QMsgSql : public SimpleSql
{
public:
    QMsgSql() : SimpleSql("mi.sql") {check_txn = 1;}
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

namespace qmsgsql {
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

void
sql_rollback_transaction()
{
    sDb->rollback_transaction();
}
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

        sql_simple("drop table if exists mt.static_crdt_tags");
        sql_simple("create table mt.static_crdt_tags(tag text not null)");
        sql_simple("insert into static_crdt_tags values('_fav')");
        sql_simple("insert into static_crdt_tags values('_hid')");
        sql_simple("insert into static_crdt_tags values('_ignore')");
        sql_simple("insert into static_crdt_tags values('_pal')");
        sql_simple("insert into static_crdt_tags values('_leader')");

        // this is an upgrade, the msg_tags2 stuff should be installed in gmt
        // with proper guids. this should mostly only be done once, but there
        // are cases where people reinstall old software that might trigger
        // this more than once
        sql_simple("create table if not exists mt.msg_tags2(mid text not null, tag text not null, time integer not null)");
        sql_simple("insert into mt.gmt select mid, tag, time, ?1, lower(hex(randomblob(8))) from mt.msg_tags2", to_hex(My_UID));
        sql_simple("delete from mt.msg_tags2");
        sql_simple("pragma mt.user_version = 0");

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
        sql_start_transaction();
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

    sql_simple("create table if not exists dir_meta(dirname text collate nocase primary key not null, time integer default 0)");

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
    //sql_simple("create index if not exists gifrom_client_uid_idx on gi(from_client_uid)");
    //sql_simple("create index if not exists gimid on gi(mid)");

    sql_simple("drop table if exists midlog");
    sql_simple("create table midlog (mid text not null, to_uid text not null, op text not null, unique(mid,to_uid,op) on conflict ignore)");

    vc res = sql_simple("pragma user_version");
    if((int)res[0][0] == 0)
    {
        // schema upgrade to include new field in index
        sql_start_transaction();
        sql_simple("alter table msg_idx add column from_group");
        sql_simple("drop table if exists gi");
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
                   "from_group,"
                   "is_local,"
                   "from_client_uid not null, unique(mid, from_client_uid) on conflict ignore)"
                  );

        sql_simple("create index if not exists giassoc_uid_idx on gi(assoc_uid)");
        sql_simple("create index if not exists gilogical_clock_idx on gi(logical_clock desc)");
        sql_simple("create index if not exists gidate_idx on gi(date desc)");
        sql_simple("create index if not exists gisent_idx on gi(is_sent)");
        sql_simple("create index if not exists giatt_idx on gi(has_attachment)");
        sql_simple("create index if not exists gifrom_client_uid_idx on gi(from_client_uid)");
        sql_simple("create index if not exists gimid on gi(mid)");
        sql_simple("create index if not exists gifrom_group on gi(from_group)");
        // force it to reindex
        sql_simple("delete from dir_meta");
        sql_simple("pragma user_version = 1;");
        sql_commit_transaction();
    }

    // vague info instead of complete info
    res = sql_simple("pragma user_version");
    if((int)res[0][0] == 1)
    {
        sql_start_transaction();
        sql_simple("create table if not exists main.newgi ("
                   "date integer,"
                   "mid text not null primary key on conflict ignore,"
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
                   "from_group,"
                   "is_local,"
                   "barf_dont_use)"
                   );
        sql_simple("insert into newgi select * from gi where rowid in (select max(rowid) from gi group by mid)");
        sql_simple("drop trigger if exists xgi");
        sql_simple("drop trigger if exists dgi");
        sql_simple("drop table main.gi");
        sql_simple("alter table newgi rename to gi");

        sql_simple("create index if not exists giassoc_uid_idx on gi(assoc_uid)");
        sql_simple("create index if not exists gilogical_clock_idx on gi(logical_clock desc)");
        sql_simple("create index if not exists gidate_idx on gi(date desc)");
        sql_simple("create index if not exists gisent_idx on gi(is_sent)");
        sql_simple("create index if not exists giatt_idx on gi(has_attachment)");
        sql_simple("create index if not exists gifrom_group on gi(from_group)");

        sql_simple("pragma user_version = 2");
        sql_commit_transaction();
    }

    sql_simple("create index if not exists gi_uid_date on gi(assoc_uid, date)");
    sql_simple("create index if not exists gi_uid_logical_clock on gi(assoc_uid, logical_clock)");
    sql_simple("drop table if exists pull_failed");
    sql_simple("create table if not exists pull_failed(mid not null, uid not null, unique(mid, uid) on conflict ignore)");
    res = sql_simple("pragma user_version");
    if((int)res[0][0] == 2)
    {
        sql_start_transaction();
        sql_simple("create table if not exists deltas(from_client_uid text not null primary key on conflict replace, delta_id text not null)");
        sql_simple("pragma user_version = 3");
        sql_commit_transaction();
    }
    sql_commit_transaction();
    }
    else if(schema_name.eq("mt"))
	{
		init_schema_fav();
	}

}

void
add_pull_failed(vc mid, vc uid)
{
    vc huid = to_hex(uid);
    sql_simple("insert into pull_failed(mid, uid) values(?1, ?2)", mid, huid);
}

void
clean_pull_failed_mid(vc mid)
{
    sql_simple("delete from pull_failed where mid = ?1", mid);
}

void
clean_pull_failed_uid(vc uid)
{
    vc huid = to_hex(uid);
    sql_simple("delete from pull_failed where uid = ?1", huid);
}

vc
get_delta_id(vc uid)
{
    vc huid = to_hex(uid);
    vc res = sql_simple("select delta_id from deltas where from_client_uid = ?1", huid);
    if(res.num_elems() == 0)
        return vcnil;
    return res[0][0];
}

bool
generate_delta(vc uid, vc delta_id)
{
    vc huid = to_hex(uid);
    DwString dbfn = DwString("minew%1.sql").arg((const char *)huid);
    dbfn = newfn(dbfn);
    SimpleSql s(dbfn);
    if(!s.init(SQLITE_OPEN_READWRITE))
        return false;

    s.attach("mi.sql", "mi");
    s.attach("fav.sql", "mt");

    // ok, here is where we find the list of mid's that have been updated, and create
    // explicit midlog entries that will get sent down normally after the connection
    // is set up and rolling.
    // note: this technique has a  lot of problem since i am not trying to integrate
    // piecemeal updates from normal operations into these things. so what will happen
    // is that a bunch of updates will get done while two things are connected normally
    // and then those updates will be replayed on the next reconnect. not a disaster, but
    // not what we want either. which makes me wonder if it might not be better to just
    // not try to stay connected all the time. will have to think about this, as it seems
    // like it might be reasonable for a chat app not to have the latest info necessarily
    // while it is in the background all the time.

    try
    {
        s.start_transaction();
        vc did = s.sql_simple("select delta_id from id");
        if(did.num_elems() != 1 || did[0][0] != delta_id)
            throw -1;
        vc r1 = s.sql_simple("with newmids(mid) as (select mid from mi.gi where mid not in (select mid from main.msg_idx)) "
                     "insert into midlog(mid, to_uid, op) select mid, ?1, 'a' from newmids returning mid", huid);
        // just insert some dummy rows, since we know that we will never be resending this
        // index database again. if the delta_ids don't match, this gets recreated from scratch
        for(int i = 0; i < r1.num_elems(); ++i)
        {
            s.sql_simple("insert into main.msg_idx(mid, assoc_uid) values(?1, '')", r1[i][0]);
        }
        vc r2 = s.sql_simple("with newmids(mid) as (select mid from mi.msg_tomb where mid not in (select mid from main.msg_tomb)) "
                     "insert into midlog(mid, to_uid, op) select mid, ?1, 'd' from newmids returning mid", huid);
        for(int i = 0; i < r2.num_elems(); ++i)
        {
            s.sql_simple("insert into main.msg_tomb(mid) values(?1)", r2[i][0]);
        }
        // tags
        vc r3 = s.sql_simple("with newguids(mid, tag, guid) as (select mid, tag, guid from mt.gmt where tag in (select * from static_crdt_tags) and "
                     "guid not in (select guid from main.msg_tags2)) "
                     "insert into taglog(mid, tag, guid, to_uid, op) select mid, tag, guid, ?1, 'a' from newguids returning guid", huid);
        for(int i = 0; i < r3.num_elems(); ++i)
        {
            s.sql_simple("insert into main.msg_tags2(guid) values(?1)", r3[i][0]);
        }
        // ok, here is a minor problem: we don't have the actual tag or mid in the dump, just the guid
        // so for now i'll just set the tag to the empty string, and figure it out later. the only
        // reason we were transmitting the mid and tag in this case was for display purposes ("we deleted
        // such and such a tag, here is what we have to update in the UI")
        vc r4 = s.sql_simple("with newguids(guid) as (select guid from mt.gtomb where guid not in (select guid from main.tomb)) "
                     "insert into taglog(mid, tag, guid, to_uid, op) select '', '', guid, ?1, 'd' from newguids returning guid", huid);
        for(int i = 0; i < r4.num_elems(); ++i)
        {
            s.sql_simple("insert into main.tomb(guid) values(?1)", r4[i][0]);
        }

        if(r1.num_elems() > 0 || r2.num_elems() > 0 || r3.num_elems() > 0 || r4.num_elems() > 0)
        {
            vc new_delta_id = s.sql_simple("update id set delta_id = lower(hex(randomblob(8))) returning delta_id");
            s.sql_simple("insert into taglog(mid, tag, to_uid, guid, op) values('', '', ?1, ?2, 's')", huid, new_delta_id[0][0]);
            GRTLOG("new delta id for %s %d", (const char *)huid, r1.num_elems() + r2.num_elems() + r3.num_elems() + r4.num_elems());
        }
        else
        {
            GRTLOG("no delta for %s", (const char *)huid, 0);
        }
        s.commit_transaction();
    }
    catch (...)
    {
        s.rollback_transaction();
        GRTLOG("delta gen failed to %s", (const char *)huid, 0);
        return false;
    }
    return true;
}

vc
sql_dump_mi()
{
    DwString fn = gen_random_filename();
    fn += ".tmp";
    fn = newfn(fn);
    SimpleSql s("mi.sql");
    if(!s.init())
        oopanic("can't dump index");
#ifdef DWYCO_BACKGROUND_SYNC
    s.set_busy_timeout(10 * 1000);
    s.check_txn = 1;
#endif
    s.attach("fav.sql", "mt");
    s.attach(fn, "dump");
    s.start_transaction();
    s.sql_simple("create table dump.msg_idx ("
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
               "from_group,"
               "is_local, "
               "from_client_uid "
               ");"
              );
    s.sql_simple("insert into dump.msg_idx select "
               "*"
               " from main.gi");

    s.sql_simple("create table dump.msg_tomb (mid, time)");
    s.sql_simple("insert into dump.msg_tomb select * from main.msg_tomb");

    // tags
    s.sql_simple("create table dump.msg_tags2(mid text, tag text, time integer default 0, guid text collate nocase)");
    s.sql_simple("create table dump.tomb (guid text not null collate nocase, time integer)");
    // note: we only send "user generated" tags. also some tags are completely local, like "unviewed" and "remote" which we
    // don't really want to send at all.
    s.sql_simple("insert into dump.msg_tags2 select "
               "mid, "
               "tag, "
               "time, "
               "guid "
               "from mt.gmt where tag in (select * from mt.static_crdt_tags)");
    s.sql_simple("insert into dump.tomb select * from mt.gtomb");
    //
    s.sql_simple("create table dump.id(delta_id text)");
    s.sql_simple("insert into dump.id values(lower(hex(randomblob(8))))");
    s.commit_transaction();
    s.detach("dump");
    s.exit();
    return fn.c_str();
}

void
create_dump_indexes(const DwString& fn)
{
    SimpleSql s(fn);
    if(!s.init())
        oopanic("can't index dump");
    try
    {
        s.start_transaction();
        s.sql_simple("create index if not exists i1 on msg_idx(mid)");
        s.sql_simple("create index if not exists i2 on msg_tomb(mid)");
        s.sql_simple("create index if not exists i3 on msg_tags2(guid)");
        s.sql_simple("create index if not exists i4 on tomb(guid)");
        s.commit_transaction();
    }
    catch(...)
    {
        s.rollback_transaction();
    }
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

static void
msg_idx_updated(vc uid, int prepended)
{
    if(prepended)
        se_emit(SE_USER_MSG_IDX_UPDATED_PREPEND, uid);
    else
        se_emit(SE_USER_MSG_IDX_UPDATED, uid);
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

        // note: put in the "group by" since it appears at some point i allowed duplicate
        // guid's in the tag set (at some point i put the receiving uid in there, so we can get
        // a picture of which client has which tags, and i'm not sure i use that info anywhere).
        // the duplicates didn't cause an error, just lots of extra processing that was ignored.

        vc tags = sql_simple("select mt2.mid, mt2.tag, mt2.time, mt2.guid, tl.op from mt.gmt as mt2, mt.taglog as tl where mt2.mid = tl.mid and mt2.tag = tl.tag and to_uid = ?1 and op = 'a' group by mt2.guid", huid);
        vc tombs = sql_simple("select tl.guid,tl.mid,tl.tag,tl.op from mt.taglog as tl where to_uid = ?1 and op = 'd'", huid);
        // note: this sync thiing is supposed to be processed after all the updates
        // that are received during a delta update, letting us know what we have integrated
        // from the remote side. there really only should be 1, and really, this needs to be
        // redesigned so that the ordering is explicit. for now, we just assume
        // we are creating a block of updates which are applied in order on the remote
        // side (along with maybe some other stuff too, but that is ok) and then
        // this sync item is applied so the next cycle knows. it is easy for this
        // to fail, but that will result in a new index being sent, which is ok.
        // the reason we order by rowid here is we want to apply the one that is
        // last in the log (in the odd case that multiple syncs are in the log
        // which is really something that "can't happen")
        vc sync = sql_simple("select guid from mt.taglog where to_uid = ?1 and op = 's' order by rowid desc limit 1", huid);
        if(idxs.num_elems() > 0 || mtombs.num_elems() > 0 || tags.num_elems() > 0 || tombs.num_elems() > 0 || sync.num_elems() > 0)
            GRTLOGA("downstream idx %d mtomb %d tag %d ttomb %d sync %d", idxs.num_elems(), mtombs.num_elems(), tags.num_elems(), tombs.num_elems(), sync.num_elems());
        sql_simple("delete from midlog where to_uid = ?1", huid);
        sql_simple("delete from taglog where to_uid = ?1", huid);
        sql_commit_transaction();

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
        if(sync.num_elems() > 0)
        {
            vc cmd(VC_VECTOR);
            cmd[0] = "sync";
            cmd[1] = sync[0][0];
            ret.append(cmd);
        }
        return ret;
    }
    catch (...)
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

    vc res = sql_simple("select mid from msg_idx where assoc_uid = ?1", to_hex(uid));
    int nm = res.num_elems();
    DwBag<vc> b(vcnil, nm / 3 + 1);
    for(int i = 0; i < nm; ++i)
    {
        b.add(res[i][0]);
    }

    vc id = uid_to_dir(uid);
    {
    DwString s((const char *)id);
    s = newfn(s);
    DwString ss = s;
    s += "" DIRSEPSTR "*.bod";

    FindVec& fv = *find_to_vec(s.c_str());
    auto n = fv.num_elems();
    for(int i = 0; i < n; ++i)
    {
        WIN32_FIND_DATA &d = *fv[i];
        if(strlen(d.cFileName) != 24)
            continue;
        DwString mid(d.cFileName);
        mid.remove(20);
        if(!b.contains(mid.c_str()))
            trash_body(uid, mid.c_str(), 1);

    }
    delete_findvec(&fv);
    }
    {
    DwString s((const char *)id);
    s = newfn(s);
    DwString ss = s;
    s += "" DIRSEPSTR "*.snt";

    FindVec& fv = *find_to_vec(s.c_str());
    auto n = fv.num_elems();
    for(int i = 0; i < n; ++i)
    {
        WIN32_FIND_DATA &d = *fv[i];
        if(strlen(d.cFileName) != 24)
            continue;
        DwString mid(d.cFileName);
        mid.remove(20);
        if(!b.contains(mid.c_str()))
            trash_body(uid, mid.c_str(), 1);

    }
    delete_findvec(&fv);
    }


}


static
void
sync_files()
{

    load_users_from_files(0);

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

// call this to remove all the sync state from the database, and return
// to "no group" status.
// ASSUMES: there are no network connections, and that the system will be
// exited and restarted in order to reset most of the state before the next login.
// THERE is a bug in here, i think we need to weed out tags that we got from
// other clients that might still be running around.
void
remove_sync_state()
{
    try
    {
        sql_start_transaction();
        // note: syncing files like this is useful during debugging, but
        // it really expensive and probably should be controlled by the
        // user otherwise. by not doing this, we risk having the index
        // not reflect the contents of the files (mainly because the files
        // are not atomically in sync with the index. some day we might
        // store the files in sqlite too, but that is pretty far off)
#ifdef DWYCO_SYNC_DEBUG
        sync_files();
#endif
        sql_simple("delete from current_clients");
        // get rid of tags that might reference unknown mids
        sql_simple("delete from mt.gmt where guid in (select guid from mt.gtomb)");
        sql_simple("delete from mt.gmt where tag not in (select * from static_uid_tags) and mid not in (select mid from msg_idx)");
        // note: we keep the pal/ignore stuff intact, but the _leader tag doesn't
        // make sense when you are outside a group, or when you re-enter a group.
        sql_simple("delete from mt.gmt where tag = '_leader'");
        // remove info we might have received from other clients regarding their tag state (which
        // can be redundant)
        sql_simple("delete from gmt where rowid not in (select max(rowid) from gmt group by mid,tag)");
        // make it look like we created all the tags
        sql_simple("update gmt set uid = ?1", to_hex(My_UID));
        // interesting question: should we redo all the guids? if we enter the same group again
        // if they are all new, it will appear as if we added all the tags again, possibly
        // looking like some old items were re-tagged. if we leave them the same, existing
        // tombstones in other clients might cover them.
        sql_simple("delete from mt.gtomb");
        // drop triggers that might preclude the delete optimization in sqlite
        sql_simple("drop trigger if exists mtomb_log");
        sql_simple("delete from msg_tomb");
        sql_simple("drop trigger if exists temp.rescan7");
        sql_simple("drop trigger if exists temp.rescan5");
        sql_simple("delete from gi");
        sql_simple("delete from dir_meta");
        sql_simple("delete from midlog");
        sql_simple("delete from mt.taglog");
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
        throw -1;
    }
}


// note: this is for testing, we're just assuming the index
// has been materialized here so we can investigate things
int
import_remote_mi(vc remote_uid)
{
    vc huid = to_hex(remote_uid);
    DwString fn = DwString("mi%1.sql").arg((const char *)huid);
    //DwString favfn = DwString("fav%1.sql").arg((const char *)huid);
    SimpleSql s("mi.sql");
    if(!s.init())
        return 0;
    // let's hang around for 10 seconds if someone else has the
    // database locked
#ifdef DWYCO_BACKGROUND_SYNC
    s.set_busy_timeout(10 * 1000);
#endif
    s.attach("fav.sql", "mt");
    s.attach(fn, "mi2");

    int ret = 1;
    try
    {
        s.start_transaction();
        vc newuids = s.sql_simple("select distinct(assoc_uid) from mi2.msg_idx except select distinct(assoc_uid) from main.gi");
        // note sure what i was up to here... removing the contents
        // of crdt_tags will effectively disable the triggers for
        // creating the tag logs (that would get sent to other clients)
        // however, i did *not* do the similar things with the current_clients,
        // so importing the message index will cause midlog entries to be
        // created for any other clients that are connected. which is
        // sort of what you want (anything new we get here gets sent
        // on down to other clients.) i would have thought this is what i
        // wanted for tags as well, but i'll have to try and remember...
        //
        // since we switched to a different database connection to avoid
        // getting hosed by the primary thread locking the database
        // causing the detaches to fail, this crdt_tags table is a temp
        // table that doesn't exist in this connection. fortunately, none
        // of the triggers that are stored in the database reference it, so
        // we are safe just ignoring it here.
        //s.sql_simple("delete from crdt_tags");
        s.sql_simple("delete from current_clients where uid = ?1", huid);

        s.sql_simple("insert or ignore into main.gi select * from mi2.msg_idx");
        vc res = s.sql_simple("select max(logical_clock) from gi");
        if(res[0][0].type() == VC_INT)
        {
            long lc = (long)res[0][0];
            update_global_logical_clock(lc);
        }
        s.sql_simple("insert or ignore into main.msg_tomb select * from mi2.msg_tomb");
        s.sql_simple("delete from main.gi where mid in (select mid from msg_tomb)");

        //sync_files();
        bool update_tags = false;
        vc c = s.sql_simple("select count(*) from mt.gtomb");
        s.sql_simple("insert or ignore into mt.gtomb select guid, time from mi2.tomb");
        vc c2 = s.sql_simple("select count(*) from mt.gtomb");
        if(c[0][0] != c2[0][0])
            update_tags = true;
        if(!update_tags)
            c = s.sql_simple("select count(*) from mt.gmt");
        s.sql_simple("insert or ignore into mt.gmt select mid, tag, time, ?1, guid from mi2.msg_tags2", huid);
        if(!update_tags)
            c2 = s.sql_simple("select count(*) from mt.gmt");
        if(c[0][0] != c2[0][0])
            update_tags = true;
        s.sql_simple("delete from mt.gmt where mid in (select mid from msg_tomb)");
        s.sql_simple("delete from mt.gmt where guid in (select guid from mt.gtomb)");

        // probably makes a lot of sense to clean out unknown uid's, if we have
        // an "authoritative" idea what the current group membership is. it will keep
        // those uids from propagating around to other clients, just in case the remote
        // clients is having troubles getting cleaned up

        //s.sql_simple("insert into crdt_tags select * from static_crdt_tags");
        s.sql_simple("insert into current_clients values(?1)", huid);

        s.sql_simple("insert into deltas(from_client_uid, delta_id) select ?1, delta_id from id", huid);
        s.commit_transaction();
#ifndef DWYCO_BACKGROUND_SYNC
        // these will deadlock if done in the background in another thread
        if(update_tags)
        {
            // NOTE: this will access the main db connection to refresh the pal list
            se_emit_uid_list_changed();
        }
        // likewise, this maps_uids, so probably need to either give it a connection
        // or q it for later.
        for(int i = 0; i < newuids.num_elems(); ++i)
        {
            se_emit(SE_USER_ADD, from_hex(newuids[i][0]));
        }
#endif
    }
    catch(...)
    {
        s.rollback_transaction();
        ret = 0;
    }

    s.detach("mi2");

    s.exit();
    return ret;
}

// returns the mid of any added index update so pulls can
// be re-started by the caller
vc
import_remote_iupdate(vc remote_uid, vc vals)
{
    vc huid = to_hex(remote_uid);

    try
    {
        sql_start_transaction();
        // this keeps us from bouncing it back to the client we just
        // received this from
        sql_simple("delete from current_clients where uid = ?1", huid);
        vc op = vals.remove_last();
        vc uid;
        vc mid;
        bool index_changed = false;
        if(op == vc("a"))
        {
            // note: we wouldn't be getting this update unless the remote side
            // had the msg stored locally. note that this isn't propagated
            // like the deletes below. we set this local flag, even though it isn't
            // used anywhere. but maybe it can be a hint if we put it in another
            // table at some point.
            vals.append(1);
            vals.append(huid);
            DwString sargs = make_sql_args(vals.num_elems());
            VCArglist a;
            a.set_size(vals.num_elems() + 1);
            a.append(DwString("insert or ignore into main.gi values(%1) returning 1").arg(sargs).c_str());
            for(int i = 0; i < vals.num_elems(); ++i)
                a.append(vals[i]);
            vc res = sql_bulk_query(&a);
            if(res.num_elems() > 0)
                index_changed = true;
            boost_logical_clock();
            // vals[12] is the assoc_uid field in msg_idx, ugh, fix this
            uid = vals[12];
            mid = vals[1];
        }
        else if(op == vc("d"))
        {
            mid = vals[0];
            uid = sql_get_uid_from_mid(mid);
            //sql_simple("insert into msg_tomb (mid, time) values(?1, strftime('%s', 'now'))", mid);
            //sql_simple("delete from gi where mid = ?1", mid);
            // note: doing it this way causes log entries to be created by triggers
            // which might lead to storms of propagated deletes in completely
            // connected clusters. it might make sense to just remove all but one
            // of the current_clients so that the updates propagate around to one
            // client at a time.
            vc res1 = sql_simple("delete from msg_idx where mid = ?1 returning 1", mid);
            vc res2 = sql_simple("delete from gmt where mid = ?1 returning 1", mid);
            if(res1.num_elems() > 0 || res2.num_elems() > 0)
                index_changed = true;
        }
        sql_simple("insert into current_clients values(?1)", huid);
        sql_commit_transaction();
        if(op == vc("d") && !uid.is_nil() && !mid.is_nil())
        {
            trash_body(from_hex(uid), mid, 1);
        }
        if(index_changed && !uid.is_nil())
        {
            msg_idx_updated(from_hex(uid), 0);
        }
        if(op == vc("a") && !mid.is_nil())
        {
            return mid;
        }
        return vcnil;
    }
    catch(...)
    {
        sql_rollback_transaction();
        return vcnil;
    }
}

void
import_new_syncpoint(vc remote_uid, vc delta_id)
{
    vc huid = to_hex(remote_uid);
    try
    {
        sql_start_transaction();
        sql_simple("insert into deltas(from_client_uid, delta_id) values(?1, ?2)", huid, delta_id);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }
}

void
import_remote_tupdate(vc remote_uid, vc vals)
{
    vc huid = to_hex(remote_uid);
#if 0
    DwString fn = DwString("mi%1.sql").arg((const char *)huid);
    DwString favfn = DwString("fav%1.sql").arg((const char *)huid);

    sDb->attach(fn, "mi2");
    sDb->attach(favfn, "fav2");
#endif

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
            vc res = sql_simple("select 1 from static_crdt_tags where tag = ?1 limit 1", tag);
            if(res.num_elems() == 1)
            {
                vc uid = sql_get_uid_from_mid(mid);
                if(!uid.is_nil())
                    se_emit_msg_tag_change(mid, from_hex(uid));
                else
                    se_emit_uid_list_changed();
            }
            else
            {
                // hack, we know if it isn't an mid tag it is a uid tag, this should
                // be fixed eventually
                se_emit_uid_list_changed();
            }
        }
        else if (op == vc("d"))
        {
            vc guid = vals[0];
            vc mid = vals[1];
            vc tag = vals[2];
            sql_simple("insert or ignore into mt.gtomb(guid, time) values(?1, strftime('%s', 'now'))", guid);
            vc res = sql_simple("select 1 from static_crdt_tags where tag = ?1 limit 1", tag);
            if(res.num_elems() == 1)
            {
                vc uid = sql_get_uid_from_mid(mid);
                if(!uid.is_nil())
                    se_emit_msg_tag_change(mid, from_hex(uid));
                else
                    se_emit_uid_list_changed();
            }
            else
            {
                // hack, we know if it isn't an mid tag it is a uid tag, this should
                // be fixed eventually
                se_emit_uid_list_changed();
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
#if 0
    sDb->detach("mi2");
    sDb->detach("fav2");
#endif
}


void
refetch_pk(int online)
{
    if(!online)
        return;
    vc huids;
    try {
    sql_start_transaction();
    huids = sql_simple("select uid from group_map");
    sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
        return;
    }

    for(int i = 0; i < huids.num_elems(); ++i)
    {
        vc buid = from_hex(huids[i][0]);
        pk_force_check(buid);
        fetch_info(buid);
    }
}

static
void
update_group_map(vc uid, int what)
{
    init_group_map();
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
    //sql_simple("pragma main.cache_size= -10000");
    //sql_simple("pragma mt.cache_size= -10000");
    vc sv = sql_simple("pragma main.user_version");
    if((int)sv[0][0] != 0)
        Schema_version_hack += sv[0][0].peek_str();
    vc sv2 = sql_simple("pragma mt.user_version");
    if((int)sv2[0][0] != 0)
        Schema_version_hack += sv2[0][0].peek_str();

    //sql_simple("insert into mt.gmt (mid, tag, uid, guid, time) values(?2, 'bar', ?1, 'mumble', 0)", hmyuid);
    // by default, our local index "local" is implied
    sql_simple("insert into gi select *, 1, ?1 from msg_idx", hmyuid);
    sql_simple("delete from gi where mid in (select mid from msg_tomb)");

    sql_simple("create temp table rescan(flag integer)");
    sql_simple("insert into rescan (flag) values(0)");
    sql_simple("create temp trigger rescan1 after insert on main.msg_tomb begin update rescan set flag = 1; end");
    sql_simple("create temp trigger rescan2 after delete on main.msg_tomb begin update rescan set flag = 1; end");
    sql_simple("create temp trigger rescan3 after delete on main.msg_idx begin update rescan set flag = 1; end");
    sql_simple("create temp trigger rescan4 after insert on main.msg_idx begin update rescan set flag = 1; end");

    // triggers use this table when reflecting changes downstream.
    // to disable the trigger, remove the tags from this table
    sql_simple("create temp table crdt_tags(tag text not null)");
    sql_simple("insert into crdt_tags select * from static_crdt_tags");

    // this is a kluge, we set the mid field to a uid, and tag it according
    // to the attributes we want on that user. the tags are replicated
    // like mid tags, it just so happens that mids and uids are random
    // 80bit numbers and are not likely to conflict. might be a good idea
    // to separate them into another table and replicate them separately, but
    // this should be ok for now.
    sql_simple("create temp table static_uid_tags(tag text not null)");
    sql_simple("insert into static_uid_tags values('_ignore')");
    sql_simple("insert into static_uid_tags values('_pal')");
    sql_simple("insert into static_uid_tags values('_leader')");

    sql_simple("drop table if exists current_clients");
    sql_simple("create table current_clients(uid text collate nocase unique on conflict ignore not null on conflict fail)");

    //sql_simple("drop trigger if exists miupdate");
    sql_simple("create trigger if not exists miupdate after insert on main.msg_idx begin insert into midlog (mid,to_uid,op) select new.mid, uid, 'a' from current_clients; end");
    //sql_simple("drop trigger if exists mt.tagupdate");
    sql_simple("create temp trigger if not exists tagupdate after insert on gmt begin insert into taglog (mid, tag, guid,to_uid,op) "
               "select new.mid, new.tag, new.guid, uid, 'a' from current_clients where new.tag in (select * from crdt_tags); end");

    // note: this drop trigger is a good idea in case the UID changes externally (like they delete the auth file)
    sql_simple("drop trigger if exists xgi");
    sql_simple(DwString("create trigger if not exists xgi after insert on main.msg_idx begin insert into gi select *, 1, '%1' from msg_idx where mid = new.mid limit 1; end").arg((const char *)hmyuid).c_str());
    sql_simple("drop trigger if exists dgi");
    sql_simple("create trigger if not exists dgi after delete on main.msg_idx begin "
               "delete from gi where mid = old.mid; "
               "insert into msg_tomb (mid, time) values(old.mid, strftime('%s', 'now')); "
               "end");
    sql_simple("create temp trigger mtomb_log after insert on msg_tomb begin "
               "insert into midlog(mid,to_uid,op) select new.mid, uid, 'd' from current_clients; "
               "end"
               );
    //sql_simple("drop trigger if exists mt.xgmt");
    //sql_simple("drop trigger if exists mt.dgmt");

    sql_simple("create temp trigger if not exists dgmt after delete on mt.gmt begin "
                        "insert into taglog (mid, tag, guid,to_uid,op) select old.mid, old.tag, old.guid, uid, 'd' from current_clients,crdt_tags where old.tag = tag; "
                        "insert into gtomb(guid, time) select old.guid, strftime('%s', 'now') from crdt_tags where old.tag = tag; "
                        "end");

    sql_simple("create temp trigger rescan5 after insert on main.gi begin update rescan set flag = 1; end");
    sql_simple("create temp trigger rescan6 after insert on mt.gmt begin update rescan set flag = 1; end");
    sql_simple("create temp trigger rescan7 after delete on main.gi begin update rescan set flag = 1; end");
    sql_simple("create temp trigger rescan8 after delete on mt.gmt begin update rescan set flag = 1; end");
    sql_commit_transaction();
    //Database_online.value_changed.connect_ptrfun(refetch_pk);
    Keys_updated.connect_ptrfun(update_group_map, 1);

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
    delete sDb;
    sDb = 0;
}

static
void
sql_insert_record(vc entry, vc assoc_uid)
{
    VCArglist a;
	// +1 because of the sql statement
    a.set_size(NUM_QM_IDX_FIELDS + 1);
    a.append("insert or ignore into msg_idx "
            "("
               "date ,"
               "mid ,"
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
               "assoc_uid,"
               "from_group"
            ")"
            "values(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14)");
/*
#define QM_IDX_DATE 0
#define QM_IDX_MID 1
#define QM_IDX_IS_SENT 2
#define QM_IDX_IS_FORWARDED 3
#define QM_IDX_IS_NO_FORWARD 4
#define QM_IDX_IS_FILE 5
#define QM_IDX_SPECIAL_TYPE 6
#define QM_IDX_HAS_ATTACHMENT 7
#define QM_IDX_ATT_HAS_VIDEO 8
#define QM_IDX_ATT_HAS_AUDIO 9
#define QM_IDX_ATT_IS_SHORT_VIDEO 10
#define QM_IDX_LOGICAL_CLOCK 11
#define QM_IDX_ASSOC_HUID 12
#define QM_IDX_FROM_GROUP 13
#define NUM_QM_IDX_FIELDS 14
*/
    for(int i = 0; i < NUM_QM_IDX_FIELDS; ++i)
        a.append(entry[i]);

    a[QM_IDX_ASSOC_HUID + 1] = to_hex(assoc_uid);
    if(entry[QM_IDX_FROM_GROUP].type() != VC_STRING)
        a[QM_IDX_FROM_GROUP + 1] = vcnil;
    else
        a[QM_IDX_FROM_GROUP + 1] = to_hex(entry[QM_IDX_FROM_GROUP]);

    sql_bulk_query(&a);
}

static
void
sql_delete_mid(vc mid)
{
    sql_simple("delete from msg_idx where mid = ?1", mid);
}

long
sql_get_max_logical_clock()
{
    vc res = sql_simple("select max(logical_clock) from gi");
    if(res[0][0].type() != VC_INT)
        return 0;
    return (long)res[0][0];
}

int
sql_is_mid_local(vc mid)
{
    vc res = sql_simple("select 1 from msg_idx where mid = ?1 limit 1", mid);
    if(res.num_elems() == 0)
        return 0;
    return 1;
}

int
sql_is_mid_anywhere(vc mid)
{
    vc res = sql_simple("select 1 from gi where mid = ?1 limit 1", mid);
    if(res.num_elems() == 0)
        return 0;
    return 1;
}

int
sql_mid_has_tombstone(vc mid)
{
    vc res = sql_simple("select 1 from msg_tomb where mid = ?1", mid);
    if(res.num_elems() == 0)
        return 0;
    return 1;
}

// we're folding uid's together that appear to be in a group.
// if that group is the one we're in now, don't fold. this gives
// us a chance to send messages between our devices (dunno about this.)
//
// there are some heuristics going on when mapping the gid back to
// a uid. ideally we would pick one that was currently active, and
// that at least gives us a chance to get a direct connection to
// that device.
static
vc
blob(vc v)
{
    vc ret(VC_VECTOR);
    ret[0] = "blob";
    ret[1] = v;
    return ret;
}

// this uses the current set of profiles stored in the client
// into a mapping of (uid, gid) pairs, where the gid is just
// the hash of the group public key in the profile. this gives us
// a (possibly incorrect) mapping of which uid's are in device groups
// without having to talk to a server. this mapping could be updated
// once we do talk to a server or more authoritative source of group
// info.
// this mapping is used to automatically coalesce sets of uid's in order
// to make the UI less confusing. for example, it is confusing if one
// person is using several devices and messaging you. without this automatic
// mapping, it would appear that the messages were coming from several
// users.
void
init_group_map()
{
    sDb->attach("prfdb.sql", "prf");
    // note: this could be a temp table, just easier to debug this way

    try
    {
        sql_start_transaction();
        sql_simple("drop table if exists group_map");
        sql_simple("create table group_map(uid primary key collate nocase not null, gid collate nocase not null)");
        sql_simple("create index gmidx on group_map(gid)");
        // use the profile database to find candidates, which we can do without
        // being connected to the server.
        vc keys = sql_simple("select alt_static_public from prf.pubkeys where length(alt_static_public) > 0 group by alt_static_public");
        for(int i = 0; i < keys.num_elems(); ++i)
        {
            vc k = keys[i][0];
            // XXX leave as hex? if gid's are surrogates for
            // for uids, it might be a problem.
            vc gid = to_hex(DH_alternate::get_gid(k));
            sql_simple("insert into group_map select uid, ?1 from prf.pubkeys where alt_static_public = ?2", gid, blob(k));
        }
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }

    sDb->detach("prf");
}

vc
map_uid_to_gid(vc uid)
{
    vc res = sql_simple("select gid from group_map where uid = ?1", to_hex(uid));
    if(res.num_elems() == 0)
        return vcnil;
    return from_hex(res[0][0]);
}

vc
map_gid_to_uid(vc gid)
{
    vc res = sql_simple("select uid from group_map where gid = ?1 limit 1", gid);
    if(res.num_elems() == 0)
        return vcnil;
    return from_hex(res[0][0]);
}

vc
map_gid_to_uids(vc gid)
{
    vc res = sql_simple("select uid from group_map where gid = ?1", gid);
    vc ret(VC_VECTOR);
    for(int i = 0; i < res.num_elems(); ++i)
    {
        ret.append(from_hex(res[i][0]));
    }
    return ret;
}

// given a uid, return a vector that contains
// all the known uids in the same group as uid.
// if uid is not in a group, just returns a vector
// with the uid in it.
vc
map_uid_to_uids(vc uid)
{
    vc res;
    vc ret(VC_VECTOR);
    try
    {
        sql_start_transaction();
        res = sql_simple("select uid from group_map where gid = (select gid from group_map where uid = ?1) order by uid asc", to_hex(uid));
        sql_commit_transaction();
        if(res.num_elems() == 0)
        {
            ret[0] = uid;
        }
        else
        {
            for(int i = 0; i < res.num_elems(); ++i)
            {
                ret.append(from_hex(res[i][0]));
            }
        }
    }
    catch (...)
    {
        ret[0] = uid;
    }

    return ret;
}

int
map_is_mapped(vc uid)
{
    vc res = sql_simple("select count(*) from group_map where gid = (select gid from group_map where uid = ?1)", to_hex(uid));
    return res[0][0];
}

// we arbitrarily pick the lexicographically smallest uid in the set
// as the "group representative". note this can change over time as uid's
// enter and leave the group.
vc
map_to_representative_uid(vc uid)
{
    vc res = map_uid_to_uids(uid);
    return res[0];
}

// there are tags where the "mid" field is actually a uid.
// this is a kluge, but it is useful, so we go with it for now.
// this creates a list of uid's given a tag (eg, "_pal") where
// grouped uid's are mapped to the group representative.
vc
map_uid_list_from_tag(vc tag)
{
    vc res;
    try {
        sql_start_transaction();
        res = sql_simple("with foo(uid) as (select distinct(mid) as uid from gmt where tag = ?1 and not exists(select 1 from gtomb where gmt.guid = guid)),"
                         " bar(uid) as (select uid from group_map except select min(uid) from group_map group by gid) "
                         " select * from foo where uid not in (select * from bar)", tag);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
        return vcnil;
    }

    return res;
}



// some users have 1000's of users in their list, and this can
// cause some problems. most of those users are just old users
// that are no longer active, or the user doesn't know how (or doesn't
// care) to use the list cleaning tools. with this, we just exclude
// those users that are not likely to be sending messages and so on, which
// speeds up the ui significantly.
vc
sql_get_recent_users(int recent, int *total_out)
{
    try
    {
        sql_start_transaction();
#if 0
        sql_simple("create temp table foo as select max(date), assoc_uid from gi group by assoc_uid");
        sql_simple("insert into foo select strftime('%s', 'now'), uid from group_map");
        vc res;
        if(total_out)
        {
            res = sql_simple("select count(distinct assoc_uid) from foo");
            *total_out = (int)res[0][0];
        }

        // remove all uid's from the user list except one that represents the group.
        // arbitrary: the lowest one lexicographically, which might change.
        // another option might be to return the one that us currently active,
        // which means we might be able to direct message/call them
        sql_simple("create temp table bar as select min(uid) from group_map group by gid");
        sql_simple("delete from foo where assoc_uid in (select uid from group_map) and assoc_uid not in (select * from bar)");

#ifdef ANDROID
        res = sql_simple("select distinct assoc_uid from foo order by \"max(date)\" desc limit 20");
#else
        res = sql_simple("select distinct assoc_uid from foo order by \"max(date)\" desc limit ?1", recent ? 100 : -1);
#endif
        sql_simple("drop table foo");
        sql_simple("drop table bar");
#endif
	vc res = sql_simple(
        "with uids(uid,lc) as (select assoc_uid, max(logical_clock) from gi   group by assoc_uid),"
         "mins(muid) as (select min(uid) from group_map group by gid),"
         "grps(guid) as (select uid from group_map)"
        //"select uid from uids where uid not in (select * from grps) or (uid in (select * from mins)) order by lc desc limit ?1",
                "select uid from uids where uid not in (select * from grps) union select * from mins limit ?1",

	recent ? 100 : -1);
        sql_commit_transaction();
        vc ret(VC_VECTOR);
        for(int i = 0; i < res.num_elems(); ++i)
            ret.append(res[i][0]);
        if(total_out)
            *total_out = res.num_elems();
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
        //sql_simple("create temp table foo as select max(date), assoc_uid from gi group by assoc_uid");
        vc res;
        // note: this doesn't get the "latest", putting in an order by doubles the amount of time
        // it takes, and most of the time, the max_count is big enough to get everything anyway
        res = sql_simple(
                    "with latest(date, uid) as (select max(date) as md, assoc_uid from gi group by assoc_uid)"
                    "select uid from latest where strftime('%s', 'now') - date < ?1 limit ?2",
                         max_age, max_count);

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

#define with_create_uidset(argnum) "with uidset(uid) as (select ?" #argnum "union select uid from group_map where gid = (select gid from group_map where uid = ?" #argnum "))"

static
void
create_uidset(vc uid)
{
    vc huid = to_hex(uid);

    sql_simple("create temp table uidset as select ?1 union select uid from group_map where gid = (select gid from group_map where uid = ?1)", huid);
}

static
void
drop_uidset()
{
    sql_simple("drop table uidset");
}

static
vc
sql_clear_uid(vc uid)
{

    try
    {
        sql_start_transaction();
        vc huid = to_hex(uid);
        create_uidset(uid);
        vc mids = sql_simple("select distinct(mid) from gi where assoc_uid in (select * from uidset)");
        sql_simple("delete from msg_idx where assoc_uid in (select * from uidset)");
        // note: there is a trigger to delete msg_idx things
        // this just gets all the stuff we don't have downloaded here
        sql_simple("insert into msg_tomb select mid, strftime('%s', 'now') from gi where assoc_uid in (select * from uidset)");
        sql_simple("delete from gi where assoc_uid in (select * from uidset)");
        drop_uidset();
        sql_commit_transaction();
        return mids;
    }
    catch(...)
    {
        sql_rollback_transaction();
        throw;
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
sql_load_group_index(vc uid, int max_count)
{
    vc huid = to_hex(uid);
    vc res;
    try
    {
        sql_start_transaction();
        vc gid = map_uid_to_gid(uid);

        // the first part of this query gets all the messages for
        // current group members. some messages may have been
        // delivered from previous members of the group that are no
        // longer in the group. since it is confusing to have messages
        // disappear (or just show up someplace different), we include
        // the messages from the group, using the group attribute on the
        // message.
        // NOTE: the messages of this sort will show up in two places, but
        // if they are deleted in one place, they will be deleted from both places.
        res = sql_simple(
                    with_create_uidset(3)
                    "select date, mid, is_sent, is_forwarded, is_no_forward, is_file, special_type, "
           "has_attachment, att_has_video, att_has_audio, att_is_short_video, logical_clock, assoc_uid "
           " from gi where "
           " (assoc_uid in (select * from uidset)"
            // messages from previous group members
                    " or (is_sent isnull and length(?1) > 0 and from_group = ?1 ))"
                " and not exists (select 1 from msg_tomb as tmb where gi.mid = tmb.mid) group by mid order by logical_clock desc limit ?2",
                    gid.is_nil() ? "" : to_hex(gid),
                    max_count,
                    huid);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
        res = vc(VC_VECTOR);
    }

    return res;
}

static
vc
sql_load_index(vc uid, int max_count)
{
    return sql_load_group_index(uid, max_count);

//    vc res = sql_simple("select date, mid, is_sent, is_forwarded, is_no_forward, is_file, special_type, "
//           "has_attachment, att_has_video, att_has_audio, att_is_short_video, logical_clock, assoc_uid "
//           " from gi where assoc_uid = ?1 and not exists (select 1 from msg_tomb as tmb where gi.mid = tmb.mid) group by mid order by logical_clock desc limit ?2",
//                        to_hex(uid), max_count);
//    return res;
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
    nentry[QM_IDX_FROM_GROUP] = body[QM_BODY_FROM_GROUP];
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
    vc huid = to_hex(uid);
    int i = 1;
    DwString s((const char *)id);
    s = newfn(s);
    DwString ss = s;
    s += "" DIRSEPSTR "*.bod";

    try
    {
        sql_start_transaction();
        sql_simple("create temp table found_mid(mid text not null primary key)");
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
                vc mid = info[QM_BODY_ID];
                sql_simple("insert into found_mid values(?1)", mid);
                vc res = sql_simple("select 1 from msg_idx where mid = ?1", mid);
                if(res.num_elems() > 0)
                    continue;
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
                vc mid = info[QM_BODY_ID];
                sql_simple("insert into found_mid values(?1)", mid);
                vc res = sql_simple("select 1 from msg_idx where mid = ?1", mid);
                if(res.num_elems() > 0)
                    continue;
                sql_insert_record(index_from_body(uid, info), uid);
            }
        }
        sql_simple("delete from msg_idx where assoc_uid = ?1 and mid not in (select * from found_mid)", huid);
        delete_findvec(&fv2);
        sql_insert_indexed_flag(uid);
        //sql_record_most_recent(uid);
        sql_simple("delete from midlog");
        sql_simple("delete from taglog");
        sql_simple("drop table found_mid");
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
        vc huid = to_hex(uid);
        //create_uidset(uid);
        vc res = sql_simple(
                    with_create_uidset(2)
                "select date, mid, is_sent, is_forwarded, is_no_forward, is_file, special_type, "
               "has_attachment, att_has_video, att_has_audio, att_is_short_video, logical_clock, assoc_uid "
               " from gi where "
               " assoc_uid in (select * from uidset)"
                    " and not exists (select 1 from msg_tomb as tmb where gi.mid = tmb.mid) and logical_clock > ?1 group by mid order by logical_clock desc",
                            logical_clock,
                            huid);
        //drop_uidset();
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
        ret.append(sql_res[i][0]);
    }
    return ret;
}

// this looks in all the loaded global indexes to see what uid's
// might have the mid in their local cache
vc
sql_find_who_has_mid(vc mid)
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



// this gets a list of mids that we don't have, that look like they might
// be available at the given uid. when we connect to this uid with a sync
// channel, we can assert pulls to this uid if we are in "eager" sync mode.

vc
sql_get_non_local_messages_at_uid(vc uid, int max_count)
{
    vc huid = to_hex(uid);
    try
    {
        sql_start_transaction();
        vc res = sql_simple("select mid from gi where "
"not exists(select 1 from pull_failed where gi.mid = mid and uid = ?1) "
                   "and not exists (select 1 from msg_idx where gi.mid = mid)"
                            "and not exists (select 1 from msg_tomb where gi.mid = mid) limit ?2",
                            huid, max_count);
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

// note: before calling this, the msg body should be saved
// into the right place, since if the index doesn't exists, the
// directory with all the messages is scanned and the index is
// initialized.
// note: i changed this to return an error if the update fails
// (which can happen a lot more frequently if we are using
// multiple threads). it is a lot more important to index
// things now that we are syncing. before, if the indexing failed
// it just meant the message might not be seen until everything
// was reindexed.
int
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
    int ret = 0;
    try
    {
        sql_start_transaction();
        sql_insert_record(nentry, uid);
        //sql_record_most_recent(uid);
        sql_commit_transaction();
        ret = 1;
    }
    catch(...)
    {
        sql_rollback_transaction();
    }
    if(!inhibit_sysmsg)
        msg_idx_updated(uid, 0);
    return ret;
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
    vc ret;
    try
    {
        sql_start_transaction();
        vc res = sql_simple(
                    with_create_uidset(1)
                    "select mid as foo from gi where assoc_uid in (select * from uidset) "
                 "and not exists (select 1 from gmt where mid = foo and tag = '_fav') group by mid", to_hex(uid));
        sql_commit_transaction();
        ret = flatten(res);
    }
    catch(...)
    {
        sql_rollback_transaction();
        throw;
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
    // if we are not in a group, then don't bother with notifying the
    // server, since it was done at fetch time
    if(Current_alternate)
    {
        for(int i = 0; i < mids.num_elems(); ++i)
        {
            //dirth_send_addtag(uid, mids[i][0], "_del", QckDone(0, 0));
            dirth_send_ack_get(My_UID, mids[i][0], QckDone(0, 0));
            pulls::deassert_pull(mids[i][0]);
        }
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

#if 0
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
#endif

void
create_dir_meta(int update_existing)
{
    try
    {
        sql_start_transaction();
        if(!update_existing)
            sql_simple("delete from dir_meta");
        FindVec &fv = *find_to_vec(newfn("*.usr").c_str());
        auto n = fv.num_elems();
        for(int i = 0; i < n; ++i)
        {
            WIN32_FIND_DATA& d = *fv[i];
            DwString fdirname = newfn(d.cFileName);
            struct stat s;
            if(stat(fdirname.c_str(), &s) == -1)
                continue;
            sql_simple("insert or replace into dir_meta(dirname, time) values(?1, ?2)", d.cFileName, s.st_mtime);
        }
        sql_commit_transaction();
        delete_findvec(&fv);
    }
    catch (...)
    {
        sql_rollback_transaction();
    }
}

void
reindex_possible_changes()
{
    try
    {
        sql_start_transaction();
        sql_simple("create temp table foo(dirname text collate nocase primary key not null, time default 0)");
        FindVec &fv = *find_to_vec(newfn("*.usr").c_str());
        auto n = fv.num_elems();
        for(int i = 0; i < n; ++i)
        {
            WIN32_FIND_DATA& d = *fv[i];
            DwString fdirname = newfn(d.cFileName);
            struct stat s;
            if(stat(fdirname.c_str(), &s) == -1)
                continue;
            sql_simple("insert into foo(dirname, time) values(?1, ?2)", d.cFileName, s.st_mtime);
        }
        delete_findvec(&fv);
        //sql_commit_transaction();
        vc needs_reindex = sql_simple("select replace(dirname, '.usr', '') from foo,dir_meta using(dirname) where foo.time != dir_meta.time "
                                      "union select replace(dirname, '.usr', '') from foo where not exists(select 1 from dir_meta where foo.dirname = dir_meta.dirname)");
        // not sure about this: if a folder is missing now, if we do this, it effectively
        // deletes the files everywhere. if we just ignore it, the files hang around, but
        // are filtered out by tombstones.
        sql_simple("create temp table bar as select dirname from dir_meta where not exists (select 1 from foo where dir_meta.dirname = foo.dirname)");
        sql_simple("delete from dir_meta where dirname in (select * from bar)");
        sql_simple("update bar set dirname = replace(dirname, '.usr', '')");
        sql_simple("delete from msg_idx where assoc_uid in (select * from bar)");

        for(int i = 0; i < needs_reindex.num_elems(); ++i)
        {
            vc huid = needs_reindex[i][0];
            vc uid = from_hex(huid);
            sql_simple("delete from indexed_flag where uid = ?1", huid);
            load_msg_index(uid, 1);
        }
        sql_simple("drop table bar");
        sql_simple("drop table foo");
        create_dir_meta(1);
        sql_commit_transaction();
    }
    catch (...)
    {
        sql_rollback_transaction();
    }
}

#if 0
void
sql_index_all()
{
    dwyco::Index_progress = 0;
    dwyco::Index_total = 0;
    load_users_from_files(0);
    dwyco::Index_total = MsgFolders.num_elems();
    try
    {
        sql_sync_off();
        sql_start_transaction();
        sql_simple("delete from indexed_flag");

        MsgFolders.foreach(vcnil, index_user);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }
    //create_dir_meta();
    reindex_possible_changes();
    sql_sync_on();
}
#endif

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

// remove all tags associated with all messages associated with uid
void
sql_fav_remove_uid(vc uid)
{
    try
    {
        sql_start_transaction();
        //create_uidset(uid);
        // find all crdt tags associated with all mids, and perform crdt related things for each mid
        sql_simple(
                    with_create_uidset(1)
                    "delete from gmt where mid in (select distinct(mid) from gi where assoc_uid in (select * from uidset))",
                    to_hex(uid));
        //drop_uidset();
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
        throw;
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
        res = sql_simple("select assoc_uid, mid from gmt,gi using(mid) where tag = ?1 "
                   "and not exists(select 1 from mt.gtomb where gmt.guid = guid) "
                   "and not exists(select 1 from msg_tomb where mid = gi.mid)"
                         "group by mid "
                         "order by logical_clock asc",
                         tag);
        //sql_simple("delete from foo where mid in (select mid from msg_tomb)");
        //res = sql_simple("select * from foo");
        for(int i = 0; i < res.num_elems(); ++i)
        {
            res[i][0] = to_hex(map_to_representative_uid(from_hex(res[i][0])));
        }
        //sql_simple("drop table foo");
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
        sql_start_transaction();
        res = sql_simple("select distinct(mid) from gmt where tag = ?1 and not exists(select 1 from gtomb where gmt.guid = guid)",
                         tag);
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
sql_get_tagged_idx(vc tag)
{
    vc res;
    try
    {
        sql_start_transaction();
        res = sql_simple("select "
                 "date, mid, is_sent, is_forwarded, is_no_forward, is_file, special_type, "
                 "has_attachment, att_has_video, att_has_audio, att_is_short_video, logical_clock, assoc_uid "
                 " from gmt,gi using(mid) where tag = ?1 and not exists(select 1 from mt.gtomb where guid = gmt.guid)"
                         " and not exists(select 1 from msg_tomb where mid = gi.mid) "
                         "group by mid "
                         "order by logical_clock desc",
                            tag);
        //sql_simple("delete from foo where mid in (select mid from msg_tomb)");
        //res = sql_simple("select * from foo");
        //sql_simple("drop table foo");
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
        sql_start_transaction();
        //create_uidset(uid);
        vc res = sql_simple(
                    with_create_uidset(2)
                    "select 1 from gmt,gi using(mid) where assoc_uid in (select * from uidset) and tag = ?1 and not exists(select 1 from gtomb where guid = gmt.guid) limit 1",
                            tag,
                    to_hex(uid));
        c = (res.num_elems() > 0);
        //drop_uidset();
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
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
        sql_start_transaction();
        //create_uidset(uid);
        vc res = sql_simple(
                    with_create_uidset(2)
                    "select count(distinct mid) from gmt,gi using(mid) where assoc_uid in (select * from uidset) and tag = ?1 and not exists (select 1 from gtomb where guid = gmt.guid)",
                            tag,
                    to_hex(uid));
        c = res[0][0];
        //drop_uidset();
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
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

int
sql_count_valid_tag(vc tag)
{
    int c = 0;
    try
    {
        vc res = sql_simple("select count(distinct(mid)) from gmt,gi using(mid) where tag = ?1 and not exists (select 1 from gtomb where guid = gmt.guid)", tag);
        c = res[0][0];
    }
    catch(...)
    {

    }
    return c;
}

bool
sql_exists_valid_tag(vc tag)
{
    bool c = false;
    try
    {
        sql_start_transaction();
        vc res = sql_simple("select 1 from gmt,gi using(mid) where tag = ?1 and not exists (select 1 from gtomb where guid = gmt.guid) limit 1", tag);
        sql_commit_transaction();
        c = res.num_elems() > 0;
    }
    catch(...)
    {
        sql_rollback_transaction();
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
    int tmp = sDb->check_txn;
    sDb->check_txn = 0;
    vc res = sql_simple("select flag from rescan");
    sDb->check_txn = tmp;
    return (int)res[0][0];
}

// WARNING: this is a useful api for debugging, it should not be in the
// api generally. it is useful for running arbitrary sql from an app
// assuming the app knows the schema.

vc
sql_run_sql(vc s, vc a1, vc a2, vc a3)
{
    try {
        sql_start_transaction();
        vc res = sql_simple(s, a1, a2, a3);
        sql_commit_transaction();
        return res;
    } catch (...) {
        sql_rollback_transaction();
        return vcnil;
    }
}
}

