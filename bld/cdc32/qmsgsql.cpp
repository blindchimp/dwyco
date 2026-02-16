
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "ezset.h"
#ifdef _Windows
#ifdef _MSC_VER
#include <direct.h>
#endif
#include <dos.h>
#include <io.h>
#define F_OK (0)
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
#include "synccalls.h"

namespace dwyco {
using namespace dwyco::qmsgsql;
DwString Schema_version_hack;
ssns::signal1<vc> Qmsg_update;

// the argument should be the number of the "uid" in the arg list for the query.
// so, if the uid is the second arg (ie, ?2), argnum should be 2.
#define with_create_uidset(argnum) "with uidset(uid) as (select ?" #argnum " union select uid from group_map where gid = (select gid from group_map where uid = ?" #argnum "))"

class QMsgSql : public SimpleSql
{
public:
    QMsgSql() : SimpleSql(MSG_IDX_DB) {check_txn = 1;}
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
void
sql_vacuum()
{
    if(sDb)
        sDb->vacuum();
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
    try
    {
        start_transaction();
        const vc& res = sql_simple("pragma mt.user_version");
        if((int)res[0][0] == 2)
            throw 0;
#ifdef DWYCO_DBG_SCHEMA
        if((int)res[0][0] != 0)
        {
            // note: remove and rebuild old databases during debugging
            oopanic("old tags found");
        }
#endif
        sql_simple("create table if not exists mt.taglog (mid text not null, tag text not null, guid text not null collate nocase, to_uid text not null, op text not null, unique(mid, tag, guid, to_uid, op) on conflict ignore)");
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
        sql_simple("create index if not exists mt.gmti5 on gmt(time)");

        sql_simple("create table if not exists mt.gtomb (guid text not null collate nocase, time integer, unique(guid) on conflict ignore)");
        sql_simple("create index if not exists mt.gtombi1 on gtomb(guid)");
        sql_simple("create index if not exists mt.gtombi2 on gtomb(time)");

        sql_simple("drop table if exists mt.static_crdt_tags");
        sql_simple("create table mt.static_crdt_tags(tag text primary key on conflict ignore not null on conflict fail)");
        sql_simple("insert into static_crdt_tags values('_fav')");
        sql_simple("insert into static_crdt_tags values('_hid')");
        sql_simple("insert into static_crdt_tags values('_ignore')");
        sql_simple("insert into static_crdt_tags values('_pal')");
        sql_simple("insert into static_crdt_tags values('_leader')");
        sql_simple("insert into static_crdt_tags values('_trash')");

        // this is an upgrade, the msg_tags2 stuff should be installed in gmt
        // with proper guids. this should mostly only be done once, but there
        // are cases where people reinstall old software that might trigger
        // this more than once
        sql_simple("create table if not exists mt.msg_tags2(mid text not null, tag text not null, time integer not null)");
        sql_simple("insert into mt.gmt select mid, tag, time, ?1, lower(hex(randomblob(8))) from mt.msg_tags2", to_hex(My_UID));
        sql_simple("drop table mt.msg_tags2");
        sql_simple("pragma mt.user_version = 2");

        commit_transaction();
    }
    catch(...)
    {
        rollback_transaction();
    }
    start_transaction();
    sql_simple("insert into static_crdt_tags values('_trash')");
    commit_transaction();
}

void
init_tag_data()
{
    try {
        sql_start_transaction();
        sql_simple("delete from mt.taglog");
        sql_commit_transaction();

    }  catch (...) {
        sql_rollback_transaction();
    }
}

void
QMsgSql::init_schema(const DwString& schema_name)
{
    if(schema_name.eq("main"))
    {
        try
        {
            sql_start_transaction();
            {
                // note: do something sensible here, this just keeps
                // us from mistakenly running on a database we have
                // upgraded during debugging.remove this once
                // experiments are merged.

                // this is cheap, so do it whether or not the schema version changes.
                // this doesn't warrant a version update (which would make old
                // software crash, and not be able to connect to sync.)
                sql_start_transaction();
                sql_simple("drop index if exists gisent_idx");
                sql_simple("drop index if exists giatt_idx");
                sql_simple("drop index if exists gifrom_group");
                sql_simple("drop index if exists sent_idx");
                sql_simple("drop index if exists att_idx");
                sql_simple("create table if not exists mid_att(mid text not null primary key, att text not null)");
                sql_commit_transaction();

                const vc& res = sql_simple("pragma user_version");
                // note: from 6 to 7 were just index changes, so going to 7 then back to 6
                // should be ok, except i forgot to comment out the debugging stuff below, so
                // it will crash, oops.
                if((int)res[0][0] == 6)
                {
                    throw 0;
                }
#ifdef DWYCO_DBG_SCHEMA
                if((int)res[0][0] != 0)
                {
                    // note: remove and rebuild old databases during debugging

                    oopanic("old database found");
                }
#endif
            }

            sql_simple("create table if not exists schema_sections(name primary key not null, val)");

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
               "assoc_uid text not null,"
               "from_group);"
              );
            sql_simple("create table if not exists most_recent_msg (uid text primary key not null on conflict ignore, date integer not null on conflict ignore)");
            sql_simple("create table if not exists indexed_flag (uid text primary key)");
            sql_simple("create index if not exists assoc_uid_idx on msg_idx(assoc_uid)");
            sql_simple("create index if not exists logical_clock_idx on msg_idx(logical_clock)");
            sql_simple("create index if not exists date_idx on msg_idx(date)");
            // NOTE: these indexes need to be dropped
            // if we have a query that is based on global "has_attachment", a partial
            // index might be more useful.
            // sql_simple("create index if not exists sent_idx on msg_idx(is_sent)");
            // sql_simple("create index if not exists att_idx on msg_idx(has_attachment)");

            sql_simple("create table if not exists dir_meta(dirname text collate nocase primary key not null, time integer default 0)");

            // note: mid's are unique identifiers, so its existence here means it was
            // deleted.
            sql_simple("create table if not exists msg_tomb(mid text not null, time integer, unique(mid) on conflict ignore)");
            sql_simple("create index if not exists mtombi_time on msg_tomb(time)");

            sql_simple("create table if not exists midlog (mid text not null, to_uid text not null, op text not null, unique(mid,to_uid,op) on conflict ignore)");

            // WARNING: the order of these fields correspond to msg_idx
            sql_simple("create table if not exists gi ("
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

            sql_simple("create index if not exists giassoc_uid_idx on gi(assoc_uid)");
            sql_simple("create index if not exists gilogical_clock_idx on gi(logical_clock)");
            sql_simple("create index if not exists gidate_idx on gi(date)");
            // NOTE: these indexes need to be dropped
            // if we have a query that is based on global "has_attachment", a partial
            // index might be more useful.
            // likewise for from_group
            // sql_simple("create index if not exists gisent_idx on gi(is_sent)");
            // sql_simple("create index if not exists giatt_idx on gi(has_attachment)");
            // sql_simple("create index if not exists gifrom_group on gi(from_group)");

            sql_simple("create index if not exists gi_uid_date on gi(assoc_uid, date)");
            sql_simple("create index if not exists gi_uid_logical_clock on gi(assoc_uid, logical_clock)");

            sql_simple("create table if not exists pull_failed(mid not null, uid not null, unique(mid, uid) on conflict ignore)");

            sql_simple("create table if not exists deltas(from_client_uid text not null primary key on conflict replace, delta_id text not null)");

            // this one seems to speed up the "uid_has_tag" query by light years if
            // analyze is done periodically.
            sql_simple("create index if not exists gi_assoc_uid_mid on gi(assoc_uid, mid)");

            // a simple map for presentation purposes that can be derived without talking to a server
            sql_simple("create table if not exists group_map(uid primary key collate nocase not null, gid collate nocase not null)");
            sql_simple("create index if not exists gmidx on group_map(gid)");

            sql_simple("pragma user_version = 6");
            sql_commit_transaction();
        }
        catch(int i)
        {
            if(i == 0)
                sql_commit_transaction();
            else
                sql_rollback_transaction();
        }
    }
    else if(schema_name.eq("mt"))
    {
        init_schema_fav();
    }
}

void
init_index_data()
{
    try {
        sql_start_transaction();
        sql_simple("delete from pull_failed");
        sql_simple("delete from midlog");
        sql_commit_transaction();
    }  catch (...) {
        sql_rollback_transaction();
    }
}

void
add_pull_failed(const vc& mid, const vc& uid)
{
    const vc& huid = to_hex(uid);
    sql_simple("insert into pull_failed(mid, uid) values(?1, ?2)", mid, huid);
}

bool
pull_failed(const vc& mid, const vc& uid)
{
    const vc& huid = to_hex(uid);
    const vc& res = sql_simple("select 1 from pull_failed where mid = ?1 and uid = ?2 limit 1", mid, huid);
    return res.num_elems() > 0;
}

void
clean_pull_failed_mid(const vc& mid)
{
    sql_simple("delete from pull_failed where mid = ?1", mid);
}

void
clean_pull_failed_uid(const vc& uid)
{
    vc huid = to_hex(uid);
    sql_simple("delete from pull_failed where uid = ?1", huid);
}

// cancel all the
// extant pulls in other send_qs
// should we also delete any extand pull-resp as well?
// hmm, no. leave that for another functions, but it makes
// sense if have "deleted" a message, and we q-ed up a response
// regarding the message we probably should nuke it.
// likewise, if we receive a response from a message we know
// has been deleted, we should probably discard the response.
// see the comment near "process_pull_resp"
static
void
stop_existing_pulls(const vc& mid)
{
    pulls::deassert_pull(mid);
    ChanList cl = get_all_sync_chans();
    for(int i = 0; i < cl.num_elems(); ++i)
    {
        cl[i]->sync_sendq.del_pull(mid);
    }
}

static
void
stop_existing_pulls(ChanList& cl, const vc& mid)
{
    pulls::deassert_pull(mid);
    for(int i = 0; i < cl.num_elems(); ++i)
    {
        cl[i]->sync_sendq.del_pull(mid);
    }
}

vc
get_delta_id(const vc& uid)
{
    vc huid = to_hex(uid);
    vc res = sql_simple("select delta_id from deltas where from_client_uid = ?1", huid);
    if(res.num_elems() == 0)
        return vcnil;
    return res[0][0];
}

bool
generate_delta(const vc& uid, const vc& delta_id)
{
    const vc& huid = to_hex(uid);
    DwString dbfn = DwString("minew%1.tdb").arg((const char *)huid);
    dbfn = newfn(dbfn);
    SimpleSql s(dbfn);
    if(!s.init(SQLITE_OPEN_READWRITE))
        return false;

    s.attach(MSG_IDX_DB, "mi");
    s.attach(TAG_DB, "mt");

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
    // **note ca 6/2022, in the wild, i've seen an uncommon issue with tombstone updates
    // being lost, despite the sync state being updated (so you end up with situations
    // where someone keeps trying to refetch messages that have been deleted.)
    // this is almost certainly an ordering issue with the wrong sync marker
    // being committed or something. see the comment in the "package_downstream" functions
    // for more details. we probably need to make the ordering explicit somehow or other on
    // these updates.

    try
    {
        s.start_transaction();
        const vc& did = s.sql_simple("select delta_id from id");
        if(did.num_elems() != 1 || did[0][0] != delta_id)
            throw -1;
        const vc& r1 = s.sql_simple("with newmids(mid) as (select mid from mi.gi where mid not in (select mid from main.msg_idx)) "
                     "insert into midlog(mid, to_uid, op) select mid, ?1, 'a' from newmids returning mid", huid);
        // just insert some dummy rows, since we know that we will never be resending this
        // index database again. if the delta_ids don't match, this gets recreated from scratch
        for(int i = 0; i < r1.num_elems(); ++i)
        {
            s.sql_simple("insert into main.msg_idx(mid, assoc_uid) values(?1, '')", r1[i][0]);
        }
        const vc& r2 = s.sql_simple("with newmids(mid) as (select mid from mi.msg_tomb where mid not in (select mid from main.msg_tomb)) "
                     "insert into midlog(mid, to_uid, op) select mid, ?1, 'd' from newmids returning mid", huid);
        for(int i = 0; i < r2.num_elems(); ++i)
        {
            s.sql_simple("insert into main.msg_tomb(mid) values(?1)", r2[i][0]);
        }
        // tags
        const vc& r3 = s.sql_simple("with newguids(mid, tag, guid) as (select mid, tag, guid from mt.gmt where tag in (select * from static_crdt_tags) and "
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
        const vc& r4 = s.sql_simple("with newguids(guid) as (select guid from mt.gtomb where guid not in (select guid from main.tomb)) "
                     "insert into taglog(mid, tag, guid, to_uid, op) select '', '', guid, ?1, 'd' from newguids returning guid", huid);
        for(int i = 0; i < r4.num_elems(); ++i)
        {
            s.sql_simple("insert into main.tomb(guid) values(?1)", r4[i][0]);
        }

        if(r1.num_elems() > 0 || r2.num_elems() > 0 || r3.num_elems() > 0 || r4.num_elems() > 0)
        {
            vc new_delta_id = s.sql_simple("update id set delta_id = lower(hex(randomblob(8))) returning delta_id");
            s.sql_simple("insert into taglog(mid, tag, to_uid, guid, op) values('', '', ?1, ?2, 's')", huid, new_delta_id[0][0]);
            s.sql_simple("insert into midlog(to_uid, mid, op) values(?1, ?2, 's')", huid, new_delta_id[0][0]);
            GRTLOG("new delta id for %s %d", (const char *)huid, r1.num_elems() + r2.num_elems() + r3.num_elems() + r4.num_elems());
        }
        else
        {
            GRTLOG("empty delta for %s", (const char *)huid, 0);
        }
        // ok, this is a problem. we don't want to commit updates to our notion of the REMOTE end
        // until we have some idea that we actually sent them and they are incorporated there.
        // now, i thought i figured that out with the "syncpoint" idea, but maybe it isn't working.

        // yea, it is fucked up. if we create an midlog, we have committed changes to the delta
        // files that will block the changes we just noticed from future midlogs. BUT we delete the
        // midlog when the connection drops, or if the program crashes. this *should* causes the
        // syncpoint to be out of whack, but for some reason it isn't, and the updates are never
        // synced via this delta method.
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
    SimpleSql s(MSG_IDX_DB);
    if(!s.init())
        oopanic("can't dump index");
#ifdef DWYCO_BACKGROUND_SYNC
    s.set_busy_timeout(10 * 1000);
    s.check_txn = 1;
#endif
    s.attach(TAG_DB, "mt");
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
               "from mt.gmt where tag in (select * from mt.static_crdt_tags) group by guid");
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
msg_idx_updated(const vc& uid, int prepended)
{
    if(prepended)
        se_emit(SE_USER_MSG_IDX_UPDATED_PREPEND, uid);
    else
        se_emit(SE_USER_MSG_IDX_UPDATED, uid);
}


vc
package_downstream_sends(const vc& remote_uid)
{
    try
    {
        // NOTE: may want to package this as a single command
        // containing both index and tag updates, then perform all of them
        // under a transaction. may not be necessary, but just a thought
        sql_start_transaction();
        // most of the time, the logs are empty, so just shortcut that case
        const vc chk = sql_simple("select (select count(*) from midlog) + (select count(*) from taglog)");
        if((int)chk[0][0] == 0)
        {
            sql_commit_transaction();
            return vcnil;
        }

        const vc huid = to_hex(remote_uid);
        // note: this "sync" command is supposed to be processed after all the updates
        // that are received during a delta update, letting us know what we have integrated
        // from the remote side. for now, we just assume
        // we are creating a block of updates which are applied in order on the remote
        // side and then
        // this sync id is stored remotely and will match our delta id. it is easy for this
        // to fail, but mismatches in the delta id just result in a full index send.
        // we are relying on sqlite's pseudo-autoincrement stuff with the rowid to
        // process only updates up to the next sync point.
        // there are two copies of the syncpoint, only because we have two logs: one for
        // messages and one for tags. it might make sense to change this to one table
        // in the future to simplify things here.

        const vc next_tag_sync_res = sql_simple("select guid, rowid from taglog where to_uid = ?1 and op = 's' order by rowid limit 1", huid);
        const vc next_mid_sync_res = sql_simple("select mid, rowid from midlog where to_uid = ?1 and op ='s' order by rowid limit 1", huid);
        vc sync_id;
        vc next_tag_sync;
        vc next_mid_sync;
        if(next_tag_sync_res.num_elems() != next_mid_sync_res.num_elems())
            oopanic("missing sync");
        if(next_tag_sync_res.num_elems() == 0)
        {
            // no sync points found, just process the entire log
            next_mid_sync = vc(INT32_MAX);
            next_tag_sync = vc(INT32_MAX);
        }
        else
        {
            if(next_mid_sync_res[0][0] != next_tag_sync_res[0][0])
                oopanic("missing sync2");
            next_mid_sync = next_mid_sync_res[0][1];
            sync_id = next_tag_sync_res[0][0];
            next_tag_sync = next_tag_sync_res[0][1];
        }

        const vc idxs = sql_simple("select msg_idx.*, midlog.op from main.msg_idx, main.midlog "
                             "where midlog.mid = msg_idx.mid and midlog.to_uid = ?1 and op = 'a' and midlog.rowid < ?2", huid, next_mid_sync);
        const vc mtombs = sql_simple("select mid, op from main.midlog "
                               "where midlog.to_uid = ?1 and op = 'd' and rowid < ?2", huid, next_mid_sync);

        // note: put in the "group by" since it appears at some point i allowed duplicate
        // guid's in the tag set (at some point i put the receiving uid in there, so we can get
        // a picture of which client has which tags, and i'm not sure i use that info anywhere).
        // the duplicates didn't cause an error, just lots of extra processing that was ignored.

        const vc tags = sql_simple("select mt2.mid, mt2.tag, mt2.time, mt2.guid, tl.op from mt.gmt as mt2, mt.taglog as tl "
                             "where mt2.mid = tl.mid and mt2.tag = tl.tag and to_uid = ?1 and op = 'a' and tl.rowid < ?2 group by mt2.guid", huid, next_tag_sync);
        const vc tombs = sql_simple("select tl.guid,tl.mid,tl.tag,tl.op from mt.taglog as tl "
                              "where to_uid = ?1 and op = 'd' and tl.rowid < ?2", huid, next_tag_sync);
        if(idxs.num_elems() > 0 || mtombs.num_elems() > 0 || tags.num_elems() > 0 || tombs.num_elems() > 0 || !sync_id.is_nil())
            GRTLOGA("downstream idx %d mtomb %d tag %d ttomb %d sync %s", idxs.num_elems(), mtombs.num_elems(), tags.num_elems(), tombs.num_elems(), (const char *)sync_id);
        sql_simple("delete from midlog where to_uid = ?1 and rowid <= ?2", huid, next_mid_sync);
        sql_simple("delete from taglog where to_uid = ?1 and rowid <= ?2", huid, next_tag_sync);
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
        if(!sync_id.is_nil())
        {
            vc cmd(VC_VECTOR);
            cmd[0] = "sync";
            cmd[1] = sync_id;
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
    const vc& uid = v[0];

    const vc& res = sql_simple("select mid from msg_idx where assoc_uid = ?1", to_hex(uid));
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

    FindVec fv(s);
    auto n = fv.num_elems();
    for(int i = 0; i < n; ++i)
    {
        const WIN32_FIND_DATA &d = *fv[i];
        if(strlen(d.cFileName) != 24)
            continue;
        DwString mid(d.cFileName);
        mid.remove(20);
        if(!b.contains(mid.c_str()))
            trash_body(uid, mid.c_str(), 1);

    }
    }
    {
    DwString s((const char *)id);
    s = newfn(s);
    DwString ss = s;
    s += "" DIRSEPSTR "*.snt";

    FindVec fv(s);
    auto n = fv.num_elems();
    for(int i = 0; i < n; ++i)
    {
        const WIN32_FIND_DATA &d = *fv[i];
        if(strlen(d.cFileName) != 24)
            continue;
        DwString mid(d.cFileName);
        mid.remove(20);
        if(!b.contains(mid.c_str()))
            trash_body(uid, mid.c_str(), 1);

    }
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

static
void
remove_delta_databases()
{
    {
        FindVec fv(newfn("minew????????????????????.tdb"));
        for(int i = 0; i < fv.num_elems(); ++i)
        {
            DeleteFile(newfn(fv[i]->cFileName).c_str());
        }
    }
    {
        FindVec fv(newfn("mi????????????????????.tdb"));
        for(int i = 0; i < fv.num_elems(); ++i)
        {
            DeleteFile(newfn(fv[i]->cFileName).c_str());
        }
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
        // note: bug, don't drop a schema item that we aren't re-creating.
        // msg_tomb isn't that big, so probably not a huge deal
        //sql_simple("drop trigger if exists mtomb_log");
        sql_simple("delete from msg_tomb");

        // here we might want to remove all the triggers on gi
        // so sqlite can do it's optimization, then recreate the triggers
        // remove_gi_triggers

        sql_simple("delete from gi");
        // reinstall_gi_triggers

        sql_simple("delete from dir_meta");
        sql_simple("delete from midlog");
        sql_simple("delete from mt.taglog");
        sql_simple("delete from deltas");
        sql_commit_transaction();
        // note: we might be getting called while inside a nested
        // transaction, and the vacuum fails. which might be ok, but
        // it makes me queasy. it is probably better to just make a vacuum
        // part of a periodic cleanup where we have control over the
        // transaction situation.
        //sDb->vacuum();
        remove_delta_databases();
    }
    catch(...)
    {
        sql_rollback_transaction();
        throw -1;
    }
}


// note: keeping the mi<uid>.sql file around
// is for testing, we're just assuming the index
// has been materialized here so we can investigate things.
// once it is integrated into the main database, it could be
// deleted.
// oh wait, i just thought of a use for it...
// since we changed the protocol for building gi to not have the
// originating uid for each mid, there is a problem when a group
// member appears to leave the group.
// imagine a case where A and B with mostly disjoint
// sets of mid's connect... the mid's from A union B will be stored
// in gi. but gi doesn't know who exactly has what at this point.
// if B disappears for good, A is left with a large index full of stuff
// it may never be able to get, making it appear like it isn't syncing
// right.
// now, if we have this information, and it appears that B has left,
// and we have queried all the other members to no avail, we could
// exclude that mid from the list.
//
// one other thing we might do, if someone appears to have left
// (maybe use the server's authoritative group membership response
// as a cue), we just drop all sync connections, and delete
// all the other sync info (get brand new indexes from everyone
// currently in the group) and reinitialize gi from our existing msg_idx.
// this is a bit more expensive, but is much more straight forward,
// as it will clean out everything B contributed to the index, but
// never contributed in the way of data.
// note that if we do this, it will happen on all clients, since
// presumably they would notice B was gone too.
// tombstones are another issue, since we don't know who they came from.
// maybe we can just use a heuristic like "if B's tombstones are not
// in any other group member's list, remove them from our list."
// though there are plenty of weird cases where if B is admitted to the
// group, deletes mid's, disappears. we still have their tombstones with no
// real way to decide what to do with them. i think this is a case where
// the operation is stored, and is pretty much reflected in the data
// until a client themselves leaves the group.
//
int
import_remote_mi(const vc& remote_uid)
{
    const vc huid = to_hex(remote_uid);
    DwString fn = DwString("mi%1.tdb").arg((const char *)huid);
    //DwString favfn = DwString("fav%1.sql").arg((const char *)huid);
    SimpleSql s(MSG_IDX_DB);
    if(!s.init())
        return 0;
    // let's hang around for 10 seconds if someone else has the
    // database locked
#ifdef DWYCO_BACKGROUND_SYNC
    s.set_busy_timeout(10 * 1000);
#endif
    s.attach(TAG_DB, "mt");
    s.attach(fn, "mi2");
    s.set_cache_size(100000);

    int ret = 1;
    try
    {
        s.start_transaction();
        // we need to trim or do something so this "import" operation isn't so time consuming.
        // i think there might need to be some kind of explicit event like "new sync client initialized"
        // so clients can perform all the re-loading they need. the "add_user" thing below needs to go away
        // and we just signal the client to re-issue the reload_conv_list so all these extra things don't show up
        // in the conv list when the background gets loaded.
        // i wonder if this is a case where wal_mode might help if we could background this operation, allowing the
        // client to continue without getting blocked (might not, since wal_mode isn't really a table thing, but
        // a database-wide thing, i think.
        s.sql_simple("create index if not exists mi2.assoc_uid_idx on msg_idx(assoc_uid)");
        const vc newuids = s.sql_simple("select distinct(assoc_uid) from mi2.msg_idx where not exists (select 1 from main.gi where assoc_uid = mi2.msg_idx.assoc_uid limit 1)");
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
        const vc res = s.sql_simple("select max(logical_clock) from gi");
        if(res[0][0].type() == VC_INT)
        {
            long lc = (long)res[0][0];
            update_global_logical_clock(lc);
        }
        const vc newtombs = s.sql_simple("insert or ignore into main.msg_tomb select * from mi2.msg_tomb returning mid");
        s.sql_simple("delete from main.gi where mid in (select mid from main.msg_tomb)");

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
        s.sql_simple("delete from mt.gmt where mid in (select mid from main.msg_tomb)");
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
        ChanList cl = get_all_sync_chans();
        for(int i = 0; i < newtombs.num_elems(); ++i)
        {
            const vc mid = newtombs[i][0];
            stop_existing_pulls(cl, mid);
        }
#endif
    }
    catch(...)
    {
        s.rollback_transaction();
        ret = 0;
    }
    // explicitly detach so the optimize doesn't
    // look at it.
    s.detach("mi2");
    s.optimize();
    s.exit();
    return ret;
}


// returns the mid of any added index update so pulls can
// be re-started by the caller
vc
import_remote_iupdate(vc remote_uid, vc vals)
{
    const vc huid = to_hex(remote_uid);

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
            if(!mid.is_nil())
            {
                uid = sql_get_uid_from_mid(mid);
                vc res3 = sql_simple("insert into msg_tomb (mid, time) values(?1, strftime('%s', 'now')) returning 1", mid);
                vc res4 = sql_simple("delete from gi where mid = ?1 returning 1", mid);
                // XX note: doing it this way causes log entries to be created by triggers
                // XX which might lead to storms of propagated deletes in completely
                // XX connected clusters. it might make sense to just remove all but one
                // XX of the current_clients so that the updates propagate around to one
                // XX client at a time.

                // ok, here is the crux of the missing tombstone problem: if the msg_idx does not have
                // the mid we are installing a tombstone for (ie, we haven't downloaded it here yet)
                // the triggers on msg_idx will not be done, and the tombstone will not be created.
                // the mid may remain in gi because we heard about the message from another client, but
                // if we got a tombstone request, it should be toast in gi as well.
                //
                // the other side thinks it has propagated it, since the delta version will match, and
                // voila, the tombstone is never propagated here again.
                // the fix is to just install the tombstones manually.
                // i'm not sure why i had
                // it correct above, then commented it out, other than i was worried about storms.
                vc res1 = sql_simple("delete from msg_idx where mid = ?1 returning 1", mid);
                vc res2 = sql_simple("delete from gmt where mid = ?1 returning 1", mid);
                if(res3.num_elems() > 0 || res4.num_elems() > 0 || res1.num_elems() > 0 || res2.num_elems() > 0)
                    index_changed = true;

                if(!uid.is_nil())
                {
                    // note: we don't have to redo index updates, since we just did it
                    // "by hand" right here. "trashing" the body by moving it to another
                    // folder is more of a "avoid data loss" thing, as the message delete
                    // is being requested by a remote group member. and while we're debugging
                    // this can be useful.
                    trash_body(from_hex(uid), mid, 1);
                    // once we see a tombstone, that means we should not
                    // be fetching it from anywhere.
                    stop_existing_pulls(mid);
                }
            }
        }

        sql_simple("insert into current_clients values(?1)", huid);
        sql_commit_transaction();

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
import_remote_tupdate(const vc& remote_uid, const vc& vals)
{
    if(vals.type() != VC_VECTOR || vals.num_elems() < 1)
        return;
    const vc huid = to_hex(remote_uid);

    try
    {
        sql_start_transaction();
        // this keeps us from bouncing it back to the client we just
        // received this from
        sql_simple("delete from current_clients where uid = ?1", huid);

        const vc& op = vals[vals.num_elems() - 1];
        static vc op_a("a");
        static vc op_d("d");
        // note: this ignore processing is a hack. we only need it
        // because we keep an in-memory data structure as a cache.
        // it isn't clear we really need the cache, as it probably
        // isn't queried that much...
        static vc ign("_ignore");
        bool reload_ignore = false;
        if(op == op_a)
        {
            const vc& mid = vals[0];
            const vc& tag = vals[1];
            const vc& tm = vals[2];
            const vc& guid = vals[3];

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
            if(tag == ign)
                reload_ignore = true;
        }
        else if (op == op_d)
        {
            const vc& guid = vals[0];
            const vc& mid = vals[1];
            const vc& tag = vals[2];
            sql_simple("insert or ignore into mt.gtomb(guid, time) values(?1, strftime('%s', 'now'))", guid);
            const vc res = sql_simple("select 1 from static_crdt_tags where tag = ?1 limit 1", tag);
            if(res.num_elems() == 1)
            {
                const vc uid = sql_get_uid_from_mid(mid);
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
            if(tag == ign)
                reload_ignore = true;
        }
        else
            oopanic("bad tupdate");
        sql_simple("insert into current_clients values(?1)", huid);
        sql_commit_transaction();
        //
        if(reload_ignore)
            Cur_ignore = get_local_ignore();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }
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

static
void
setup_update_triggers()
{
    try {
        sql_start_transaction();
        // WARNING: be careful with this schema versioning stuff. you probably need to
        // create NEW KEY's instead of just updating the "val", since old version of the
        // software do not use a "we're at the max value" technique.
        vc res = sql_simple("select val from schema_sections where name = 'update_triggers'");
        if(res.num_elems() == 0)
        {
            sql_simple("insert into schema_sections (name, val) values('update_triggers', 0)");
        }
        else if((int)res[0][0] == 1)
            throw 0;

        // this rescan stuff is probably too expensive. it might just be better to do
        // it in the update_hook
        sql_simple("create table rescan(flag integer)");
        sql_simple("insert into rescan (flag) values(0)");
        sql_simple("create trigger rescan1 after insert on main.msg_tomb begin update rescan set flag = 1; end");
        sql_simple("create trigger rescan2 after delete on main.msg_tomb begin update rescan set flag = 1; end");
        sql_simple("create trigger rescan3 after delete on main.msg_idx begin update rescan set flag = 1; end");
        sql_simple("create trigger rescan4 after insert on main.msg_idx begin update rescan set flag = 1; end");

        sql_simple("create trigger rescan5 after insert on main.gi begin update rescan set flag = 1; end");
        sql_simple("create trigger rescan7 after delete on main.gi begin update rescan set flag = 1; end");

        sql_simple("create table if not exists uid_updated(uid text not null collate nocase unique on conflict ignore)");
        sql_simple("create trigger uid_update1 after insert on  main.gi begin "
                   "insert into uid_updated(uid) values(new.assoc_uid); "
                   "end"
                   );
        sql_simple("create trigger uid_update2 after delete on  main.gi begin "
                   "insert into uid_updated(uid) values(old.assoc_uid); "
                   "end"
                   );

        sql_simple("update schema_sections set val = 1 where name = 'update_triggers'");
        sql_commit_transaction();
    }  catch (...) {
        sql_rollback_transaction();
    }
    // not sure if this is peculiarity of sqlite or what, since we want to update a table in main
    // based on update to mt...
    sql_simple("create temp trigger rescan6 after insert on mt.gmt begin update rescan set flag = 1; end");
    sql_simple("create temp trigger rescan8 after delete on mt.gmt begin update rescan set flag = 1; end");
#if 0
    sql_simple("create temp trigger uid_update3 after insert on mt.gmt begin "
               "insert into uid_updated(uid) select assoc_uid from main.gi where mid = new.mid; "
               "end"
               );
    sql_simple("create temp trigger uid_update4 after delete on mt.gmt begin "
               "insert into uid_updated(uid) select assoc_uid from main.gi where mid = old.mid; "
               "end"
               );
    sql_simple("create temp trigger uid_update5 after insert on mt.gtomb begin "
               "insert into uid_updated(uid) select assoc_uid from main.gi where mid = (select mid from mt.gmt where guid = new.guid); "
               "end"
               );
#endif
    sql_simple("drop trigger if exists uid_update1");
    sql_simple("drop trigger if exists uid_update2");



}

static
void
setup_crdt_triggers()
{
    try {
        sql_start_transaction();
        // WARNING: be careful with this schema versioning stuff. you probably need to
        // create NEW KEY's instead of just updating the "val", since old version of the
        // software do not use a "we're at the max value" technique.
        vc res = sql_simple("select val from schema_sections where name = 'crdt_triggers'");
        if(res.num_elems() == 0)
        {
            sql_simple("insert into schema_sections (name, val) values('crdt_triggers', 0)");
        }
        else if((int)res[0][0] == 4)
            throw 0;

        // note: some of these might be better put in "update triggers" since they aren't really
        // crdt specific. but for now, i'll just keep it as is.
        sql_simple("create trigger if not exists miupdate after insert on main.msg_idx begin insert into midlog (mid,to_uid,op) select new.mid, uid, 'a' from current_clients; end");
        sql_simple("drop trigger if exists mt.tagupdate");

        // note: this drop trigger is a good idea in case the UID changes externally (like they delete the auth file)
        //sql_simple("drop trigger if exists xgi");
        sql_simple("create trigger if not exists xgi after insert on main.msg_idx begin insert into gi select *, 1, null from msg_idx where mid = new.mid limit 1; end");
        sql_simple("drop trigger if exists dgi");
        sql_simple("create trigger if not exists dgi2 after delete on main.msg_idx begin "
                   "delete from gi where mid = old.mid; "
                   //"insert into msg_tomb (mid, time) values(old.mid, strftime('%s', 'now')); "
                   "end");
        sql_simple("create trigger if not exists mtomb_log after insert on msg_tomb begin "
                   "insert into midlog(mid,to_uid,op) select new.mid, uid, 'd' from current_clients; "
                   "end"
                   );

        sql_simple("drop trigger if exists mt.dgmt");

        sql_simple("update schema_sections set val = 4 where name = 'crdt_triggers'");

        sql_commit_transaction();
    }  catch (...) {
        sql_rollback_transaction();

    }

    vc v = get_settings_value("group/alt_name");
    if(v.len() > 0)
    {


    sql_simple("create temp trigger if not exists tagupdate after insert on mt.gmt begin insert into taglog (mid, tag, guid,to_uid,op) "
               "select new.mid, new.tag, new.guid, uid, 'a' from current_clients where new.tag in (select * from crdt_tags); end");
    sql_simple("create temp trigger if not exists dgmt after delete on mt.gmt begin "
                        "insert into taglog (mid, tag, guid,to_uid,op) select old.mid, old.tag, old.guid, uid, 'd' from current_clients,crdt_tags where old.tag = tag; "
                        "insert into gtomb(guid, time) select old.guid, strftime('%s', 'now') from crdt_tags where old.tag = tag; "
                        "end");
    sql_simple("create temp trigger if not exists gen_msg_tomb after delete on main.msg_idx begin "
               "insert into msg_tomb (mid, time) values(old.mid, strftime('%s', 'now')); "
               "end"
               );
    }
    else
    {
        // not in a group, remove any lingering tombstones
        sql_start_transaction();
        sql_simple("delete from msg_tomb");
        sql_simple("delete from mt.gtomb");
        sql_commit_transaction();
    }
    // XXX i wonder why i left out a trigger on insert of gtomb (like the one above for mtomb).
    // possibly to avoid storms? the only time there is a direct insert to the gtomb is when
    // a receiver processes a 'd' update or we get an index from another group member.
}

static
void
sql_update_hook(void *user_arg, int op, const char *db, const char *table, sqlite3_int64 rowid)
{
    if(op != SQLITE_INSERT)
        return;
    if(!(strcmp(table, "midlog") == 0 || (strcmp(table, "taglog") == 0)))
        return;
    Qmsg_update.emit(table);
}


void
init_qmsg_sql()
{
    if(sDb)
        return;
    int force_reindex = 0;

    if(access(newfn(MSG_IDX_DB).c_str(), F_OK) == -1)
    {
        // major update, remove the old databases, and force a reindex
        DeleteFile(newfn("mi.sql").c_str());
        // note: we try to import old tags below
        //DeleteFile(newfn("fav.sql").c_str());
        remove_delta_databases();
        force_reindex = 1;
    }
    sDb = new QMsgSql;

    if(!sDb->init())
        throw -1;
    vc hmyuid = to_hex(My_UID);
    sDb->attach(TAG_DB, "mt");
    sql_simple("pragma recursive_triggers=1");
    sDb->set_update_hook(sql_update_hook, 0);
    sql_start_transaction();
    init_index_data();
    init_tag_data();
    //sql_simple("pragma main.cache_size= -10000");
    //sql_simple("pragma mt.cache_size= -10000");
    sDb->set_cache_size(5000);
    vc sv = sql_simple("pragma main.user_version");
    Schema_version_hack = DwString();
    if((int)sv[0][0] != 0)
        Schema_version_hack += sv[0][0].peek_str();
    vc sv2 = sql_simple("pragma mt.user_version");
    if((int)sv2[0][0] != 0)
        Schema_version_hack += sv2[0][0].peek_str();

    // by default, our local index "local" is implied
    // note: these next two can be expensive, and can probably be avoided.
    // they are useful for debugging to keep things in sync, but in normal
    // usage, this might be something you could relegate to a "maintenance"
    // mode or something.
    sql_simple("delete from msg_idx where mid in (select mid from msg_tomb)");
    sql_simple("insert into gi select *, 1, ?1 from msg_idx", hmyuid);
    sql_simple("delete from gi where mid in (select mid from msg_tomb)");

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

    // this is just a scratch table, i put it up here to avoid "create temp"
    // during normal operations...
    // (which is a schema operation, and locks tons of stuff)
    sql_simple("create temp table uidset(uid text not null)");

    sql_simple("drop table if exists current_clients");
    sql_simple("create table current_clients(uid text collate nocase unique on conflict ignore not null on conflict fail)");

    setup_crdt_triggers();
    setup_update_triggers();

    sql_commit_transaction();
    // this happens one time, the sql file names are changed and old
    // files are deleted. we try to bring in tags from really old
    // databases, not sure if this will work all the time though.
    // note: cdcx as of 2.3x doesn't have any tags that the user created, so
    // we shouldn't have to do anything.
    if(force_reindex)
    {
        if(access(newfn("fav.sql").c_str(), F_OK) == 0)
        {
            sDb->attach("fav.sql", "old");
            sql_start_transaction();
            // note: really old databases will not have a gmt table, so fake one up
            // so we don't crash
            sql_simple("create table if not exists old.gmt(mid, tag, time, uid, guid)");
            sql_simple("insert into mt.gmt select mid, tag, time, uid, guid from old.gmt where tag in ('_hid', '_fav', '_ignore', '_pal')");
            // really old tags, for upgrading old android installs of phoo and rando
            sql_simple("create table if not exists old.msg_tags2(mid, tag)");
            // note: rando used tags for _json and hashes, so don't filter them
            sql_simple("insert into mt.gmt select mid, tag, strftime('%s', 'now'), ?1, lower(hex(randomblob(8))) from old.msg_tags2", //where tag in ('_hid', '_fav', '_ignore', '_pal')",
                       to_hex(My_UID));
            //sql_simple("drop table msg_tags2");
            sql_commit_transaction();
            sDb->detach("old");
            DeleteFile(newfn("fav.sql").c_str());
        }
        sql_start_transaction();
        reindex_possible_changes();
        sql_simple("delete from midlog");
        sql_simple("delete from taglog");
        sql_commit_transaction();

    }
    //Database_online.value_changed.connect_ptrfun(refetch_pk);
    Keys_updated.connect_ptrfun(update_group_map, ssns::UNIQUE);
}

void
exit_qmsg_sql()
{
    if(!sDb)
        return;
    sDb->optimize();
    sDb->exit();
    delete sDb;
    sDb = 0;
}

static
void
sql_insert_record(const vc& entry, const vc& assoc_uid, const vc& att)
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
    if(!att.is_nil())
    {
        sql_simple("insert or ignore into mid_att (mid, att) values(?1, ?2)", a[2], att);
    }
}

static
void
sql_delete_mid(const vc& mid)
{
    // note: if it isn't in the msg_idx, but it is not downloaded here
    // we still might find it in gi. but deleting it from the msg_idx
    // will trigger the updates needed.
    sql_start_transaction();
    vc res = sql_simple("delete from msg_idx where mid = ?1 returning 1", mid);
    if(res.num_elems() == 0)
    {
        // still might find it in gi, so try that
        res = sql_simple("delete from gi where mid = ?1 returning 1", mid);
        if(res.num_elems() > 0)
        {
            sql_simple("insert into msg_tomb (mid, time) values(?1, strftime('%s', 'now'))", mid);
        }
    }
    sql_simple("delete from mid_att where mid = ?1", mid);
    sql_commit_transaction();
}

int
sql_attachment_already_received(const vc& att_name)
{
    if(att_name.is_nil())
        return 0;
    vc res = sql_simple("select 1 from mid_att where att = ?1 limit 1", att_name);
    if(res.num_elems() == 0)
        return 0;
    return 1;
}

long
sql_get_max_logical_clock()
{
    const vc res = sql_simple("select max(logical_clock) from gi");
    if(res[0][0].type() != VC_INT)
        return 0;
    return (long)res[0][0];
}

// what we need here is a "what was the system time locally of
// the last message we got from this uid"
// we don't have this, so we estimate by just using the date
// in the index, which is the creation date, which might be
// screwed up pretty bad.
// note that this does not do the uid folding, since we use this
// in a situation where we are trying to pick out a good uid
// from a group of uid's.
// note: it might be better just to return a couple of lists
// with all the uid's in it to avoid multiple calls to this,
// but i'm not even sure this will work, so keep it simpler.
bool
sql_has_msg_recently(vc uid, long num_seconds)
{
    vc huid = to_hex(uid);
    const vc res = sql_simple("select max(logical_clock) from gi where assoc_uid = ?1 and is_sent isnull", huid);
    // we try two things: first check to see if the logical clock we are currently
    // on minus what is in the index is in the interval now - num_seconds.
    // if we just return "yes" as it is most likely ok.
    // so, logical clocks don't have units, but it is sorta "seconds"-ish.

    // if not, just check the dates directly in case the logical clocks have
    // drifted apart.
    if(res.num_elems() != 1)
        return false;
    int64_t mlc = (long)res[0][0];
    int64_t diff = diff_logical_clock(mlc);
    // diff should be positive, but if it isn't, it isn't
    // a big deal
    if(diff < num_seconds)
        return true;
    const vc res2 = sql_simple("select max(date) from gi where assoc_uid = ?1 and is_sent isnull", huid);
    if(res2.num_elems() != 1)
        return false;
    if(res2[0][0].is_nil())
        return false;
    auto dm = static_cast<long long>(res[0][0]);
    if(time(0) - dm < num_seconds)
        return true;
    return false;
}

vc
sql_last_recved_msg(vc uid)
{
    vc huid = to_hex(uid);
    vc res = sql_simple(
                with_create_uidset(1)
                "select uid, max(logical_clock), max(date) from gi,uidset "
                "where assoc_uid = uidset.uid and is_sent isnull group by assoc_uid "
                "order by 2 desc, 3 desc", huid);

    return res;
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
// once we do talk to a server or a more authoritative source of group
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
        sql_simple("delete from group_map");

        // use the profile database to find candidates, which we can do without
        // being connected to the server.
        const vc keys = sql_simple("select alt_static_public from prf.pubkeys where length(alt_static_public) > 0 group by alt_static_public");
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

    //sDb->detach("prf");
}

vc
map_uid_to_gid(vc uid)
{
    const vc res = sql_simple("select gid from group_map where uid = ?1", to_hex(uid));
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
map_uid_to_uids(const vc& uid)
{
    vc ret(VC_VECTOR);
    try
    {
        const vc huid = to_hex(uid);
        {
            const vc m = sql_simple("select 1 from group_map where uid = ?1", huid);
            if(m.num_elems() == 0)
            {
                ret[0] = uid;
                return ret;
            }
        }
        //sql_start_transaction();
        const vc res = sql_simple("select uid from group_map where gid = (select gid from group_map where uid = ?1) order by uid asc", huid);
        //sql_commit_transaction();
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
        // remove all uid's from the user list except one that represents the group.
        // arbitrary: the lowest one lexicographically, which might change.
        // another option might be to return the one that is currently active,
        // which means we might be able to direct message/call them
        vc res = sql_simple(
                    "with uids(uid,lc) as (select assoc_uid, max(logical_clock) from gi "
                    "where strftime('%s', 'now') - date < ?2 "
                    //"and not exists(select 1 from gmt where mid = gi.mid and tag = '_trash' "
                    //"and not exists(select 1 from gtomb where guid = gmt.guid))"
                    "group by assoc_uid),"

                    // note: the join here just makes sure the mins table doesn't include
                    // group representatives with no messages. the group_map is
                    // derived from the profile database, which is not trimmed
                    // or anything and may contain profiles we have no messages from.

                    // find the groups that are referenced by at least one message
                    "gids(gid) as (select gid from group_map,uids using(uid) group by gid), "
                    // map the gids to the min uid in the group, even if the uid isn't in the
                    // list so far.
                    "mins(muid) as (select min(uid) from group_map,gids using(gid) group by gid),"
                    "grps(guid) as (select uid from group_map) "

                    //"select uid from uids where uid not in (select * from grps) or (uid in (select * from mins)) order by lc desc limit ?1",
                    "select uid from uids where uid not in (select * from grps) union select * from mins limit ?1",
                    recent ? 100 : -1,
                    recent ? (365LL * 24 * 3600) : (100LL * (365LL * 24 * 3600)));

        if(total_out)
        {
            // note: total out is just an estimate of the number of
            // uid's we are not returning. this is really intended just to
            // give the user some idea of how many users are being hidden,
            // and might just as easily be a boolean.
            //vc res2 = sql_simple("select count(distinct assoc_uid) from gi");
            // for whatever reason, this is about twice as fast as the above
            vc res2 = sql_simple("select count(*) from (select count(*) from gi group by assoc_uid)");
            *total_out = (long)res2[0][0];
        }
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
        vc res;
        // note: this doesn't get the "latest", putting in an "order by" doubles the amount of time
        // it takes, and most of the time, the max_count is big enough to get everything anyway
        // note also, this doesn't fold uid's by group, as this is used in situations where you
        // want to know the uid's status individually.
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
sql_remove_uid_msg_idx(vc uid)
{
    try
    {
        sql_start_transaction();
        vc huid = to_hex(uid);
        sql_simple("delete from msg_idx where assoc_uid = ?1", huid);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }

}

static
void
create_uidset(vc uid)
{
    vc huid = to_hex(uid);
    sql_simple("delete from uidset");
    sql_simple("insert into uidset select ?1 union select uid from group_map where gid = (select gid from group_map where uid = ?1)", huid);
}

static
void
drop_uidset()
{

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
    vc res = sql_simple("select 1 from indexed_flag where uid = ?1", to_hex(uid));
    if(res.num_elems() == 0)
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
        // NOTE: the messages of this type will show up in two places, but
        // if they are deleted in one place, they will be deleted from both places.
        res = sql_simple(
                    with_create_uidset(3)
                    "select date, mid, is_sent, is_forwarded, is_no_forward, is_file, special_type, "
           "has_attachment, att_has_video, att_has_audio, att_is_short_video, logical_clock, assoc_uid "
           " from gi where "
           " (assoc_uid in (select * from uidset)"
            // messages from previous group members
                    " or (is_sent isnull and length(?1) > 0 and from_group = ?1 ))"
                " and not exists (select 1 from msg_tomb as tmb where gi.mid = tmb.mid) order by logical_clock desc limit ?2",
                    gid.is_nil() ? vc("") : to_hex(gid),
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

// note: this query is the same as the one above, and as far as i know,
// it isn't needed anywhere. if i put partial paging of message index
// things, this will probably change.
static
int
sql_count_index(vc uid)
{
    vc huid = to_hex(uid);
    vc res;
    try
    {
        sql_start_transaction();
        vc gid = map_uid_to_gid(uid);

        res = sql_simple(
                    with_create_uidset(2)
                    "select count(*) "
                    " from gi where "
                    " (assoc_uid in (select * from uidset)"
                    // messages from previous group members
                    " or (is_sent isnull and length(?1) > 0 and from_group = ?1 ))"
                    " and not exists (select 1 from msg_tomb as tmb where gi.mid = tmb.mid)",
                    gid.is_nil() ? vc("") : to_hex(gid),
                    huid);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
        return -1;
    }
    return (int)res[0][0];
}

static
vc
index_from_body(const vc& recipient,  vc body, vc& att)
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
    att = body[QM_BODY_ATTACHMENT];
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
        FindVec fv(s);
        auto n = fv.num_elems();
        for(i = 0; i < n; ++i)
        {
            const WIN32_FIND_DATA &d = *fv[i];
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
                vc att;
                vc nentry = index_from_body(uid, info, att);
                sql_insert_record(nentry, uid, att);
            }
        }

        s = ss;
        s += "" DIRSEPSTR "*.snt";

        FindVec fv2(s);
        n = fv2.num_elems();
        for(i = 0; i < n; ++i)
        {
            const WIN32_FIND_DATA &d = *fv2[i];
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
                vc att;
                vc nentry = index_from_body(uid, info, att);
                sql_insert_record(nentry, uid, att);
            }
        }
        sql_simple("delete from msg_idx where assoc_uid = ?1 and mid not in (select * from found_mid)", huid);
        sql_insert_indexed_flag(uid);
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
        vc res = sql_simple(
                    with_create_uidset(2)
                "select date, mid, is_sent, is_forwarded, is_no_forward, is_file, special_type, "
               "has_attachment, att_has_video, att_has_audio, att_is_short_video, logical_clock, assoc_uid "
               " from gi where "
               " assoc_uid in (select * from uidset)"
                    " and not exists (select 1 from msg_tomb as tmb where gi.mid = tmb.mid) and logical_clock > ?1 order by logical_clock desc",
                            logical_clock,
                            huid);
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
                            "and not exists (select 1 from msg_tomb where gi.mid = mid) order by logical_clock desc limit ?2",
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

vc
sql_get_non_local_messages_at_uid_recent(vc uid, int max_count)
{
    vc huid = to_hex(uid);
    try
    {
        sql_start_transaction();
        vc res = sql_simple("select mid from gi where "
                            "date > strftime('%s', 'now') - 30 * 86400 "
                            "and not exists(select 1 from pull_failed where gi.mid = mid and uid = ?1) "
                            "and not exists (select 1 from msg_idx where gi.mid = mid)"
                            "and not exists (select 1 from msg_tomb where gi.mid = mid) order by logical_clock desc limit ?2",
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

    vc att;
    vc nentry = index_from_body(recip, body, att);
    GRTLOGVC(nentry);
    int ret = 0;
    try
    {
        sql_start_transaction();
        // WARNING: we are checking the global index, but actually inserting
        // into the local index (msg_idx), assuming that the update will make
        // its way into the global index. what we probably need to do is
        // use a trigger directly on the global index to create the events
        // somehow. the reason this bug went for so long was probably
        // because it was masked by the "rescan" triggers being updated.
        vc res = sql_simple("select 1 from gi where assoc_uid = ?1 limit 1", to_hex(uid));
        sql_insert_record(nentry, uid, att);
        if(res.num_elems() == 0)
        {
            se_emit(SE_USER_ADD, uid);
        }
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
    sql_remove_uid_msg_idx(uid);
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
                 "and not exists (select 1 from gmt where mid = foo and tag = '_fav' and not exists(select 1 from gtomb where gmt.guid = gtomb.guid)) ", to_hex(uid));
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
    vc mids;
    try
    {
        mids = sql_clear_uid(uid);
    }
    catch(...)
    {
        return;
    }
    msg_idx_updated(uid, 0);
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

static
void
create_dir_meta()
{
    try
    {
        sql_start_transaction();
        sql_simple("delete from dir_meta");
        FindVec fv(newfn("*.usr"));
        auto n = fv.num_elems();
        for(int i = 0; i < n; ++i)
        {
            const WIN32_FIND_DATA& d = *fv[i];
            DwString fdirname = newfn(d.cFileName);
            struct stat s;
            if(stat(fdirname.c_str(), &s) == -1)
                continue;
            sql_simple("insert or replace into dir_meta(dirname, time) values(?1, ?2)", d.cFileName, s.st_mtime);
        }
        sql_commit_transaction();
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
        FindVec fv(newfn("*.usr"));
        auto n = fv.num_elems();
        for(int i = 0; i < n; ++i)
        {
            const WIN32_FIND_DATA& d = *fv[i];
            DwString fdirname = newfn(d.cFileName);
            struct stat s;
            if(stat(fdirname.c_str(), &s) == -1)
                continue;
            sql_simple("insert into foo(dirname, time) values(?1, ?2)", d.cFileName, s.st_mtime);
        }
        vc needs_reindex = sql_simple("select replace(dirname, '.usr', '') from foo,dir_meta using(dirname) where foo.time != dir_meta.time "
                                      "union select replace(dirname, '.usr', '') from foo where not exists(select 1 from dir_meta where foo.dirname = dir_meta.dirname)");
        // not sure about this: if a folder is missing now, if we do this, it effectively
        // deletes the files everywhere. if we just ignore it, the files hang around, but
        // are filtered out by tombstones.
        sql_simple("create temp table bar as select dirname from dir_meta where not exists (select 1 from foo where dir_meta.dirname = foo.dirname)");
        sql_simple("delete from dir_meta where dirname in (select * from bar)");
        sql_simple("update bar set dirname = replace(dirname, '.usr', '')");
        sql_simple("delete from msg_idx where assoc_uid in (select * from bar)");
        sql_simple("delete from indexed_flag where uid in (select * from bar)");

        for(int i = 0; i < needs_reindex.num_elems(); ++i)
        {
            vc huid = needs_reindex[i][0];
            vc uid = from_hex(huid);
            sql_simple("delete from indexed_flag where uid = ?1", huid);
            load_msg_index(uid, 1);
        }
        sql_simple("drop table bar");
        sql_simple("drop table foo");
        create_dir_meta();
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
sql_remove_all_tags_uid(vc uid)
{
    try
    {
        sql_start_transaction();
        sql_simple(
                    with_create_uidset(1)
                    "delete from gmt where mid in (select distinct(mid) from gi where assoc_uid in (select * from uidset))",
                    to_hex(uid));
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
        throw;
    }
}

void
sql_remove_all_tags_mid(vc mid)
{
    try
    {
        sql_start_transaction();
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
        // note: in the past, we allowed multiple favorite tags, but this doesn't really
        // make sense, and can lead to a lot of thrashing, so just ignore multiple attempts
        // to create favorites
        if(!sql_fav_is_fav(mid))
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

        for(int i = 0; i < res.num_elems(); ++i)
        {
            res[i][0] = to_hex(map_to_representative_uid(from_hex(res[i][0])));
        }

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

// this returns all mids with given tag regardless of download state
// whose creation time is older than the given number of days.

vc
sql_get_tagged_mids_older_than(vc tag, int days)
{
    vc res;
    try
    {
        sql_start_transaction();
        res = sql_simple("select distinct(mid) from gmt where tag = ?1 and strftime('%s', 'now') - time > (?2 * 24 * 3600) and not exists(select 1 from gtomb where gmt.guid = guid)",
                         tag, days);
        sql_commit_transaction();
        res = flatten(res);
    }
    catch (...)
    {
        sql_rollback_transaction();
        res = vc(VC_VECTOR);
    }
    return res;
}

// this is used for displaying sets of tagged mids. ordering by tag time
// is used for trash handling, since you really want to see when something
// was trashed, not when the messages was created, in that case.
vc
sql_get_tagged_idx(vc tag, int order_by_tag_time)
{
    vc res;
    try
    {
        sql_start_transaction();

        DwString sql = DwString(
                    "select "
                    "date, mid, is_sent, is_forwarded, is_no_forward, is_file, special_type, "
                    "has_attachment, att_has_video, att_has_audio, att_is_short_video, logical_clock, assoc_uid "
                    " from gmt,gi using(mid) where tag = ?1 and not exists(select 1 from mt.gtomb where guid = gmt.guid)"
                    " and not exists(select 1 from msg_tomb where mid = gi.mid) "
                    "group by mid ");
        if(order_by_tag_time)
            sql += " order by gmt.time desc, logical_clock desc";
        else
            sql += " order by logical_clock desc";
        res = sql_simple(sql.c_str(), tag);

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
        //sql_start_transaction();
        vc res = sql_simple(
                    with_create_uidset(2)
                    "select 1 from gmt,gi using(mid) where assoc_uid in (select * from uidset) and tag = ?1 "
                    "and not exists(select 1 from gtomb where guid = gmt.guid) "
                    "and not exists(select 1 from msg_tomb where mid = gi.mid) "
                    "limit 1",
                            tag,
                    to_hex(uid));
        c = (res.num_elems() > 0);
        //sql_commit_transaction();
    }
    catch(...)
    {
        //sql_rollback_transaction();
    }
    return c;

}

// this is used mainly for if a uid's messages have been
// trashed. if so, it is better to avoid showing the uid
// in a lot of contexts.
int
sql_uid_all_mid_tagged(const vc& uid, const vc& tag)
{
    int c = 0;
    try
    {
        //sql_start_transaction();
#if 0
        vc res = sql_simple(
                    with_create_uidset(2) ","
                    "tm(mid) as (select mid from gmt where tag = ?1 and not exists (select 1 from gtomb where gmt.guid = guid) group by mid),"
                    "msgs(mid) as (select mid from gi where assoc_uid in (select * from uidset) and not exists(select 1 from msg_tomb where mid = gi.mid))"
                    "select mid from msgs except select mid from tm limit 1",
                    tag,
                    to_hex(uid)
                    );
#else
        vc res = sql_simple(
                    with_create_uidset(2)
                    "select 1 from gi where assoc_uid in (select * from uidset) "
                    "and not exists(select 1 from gmt where gi.mid = mid and tag = ?1 "
                    "and not exists(select 1 from gtomb where guid = gmt.guid)) "
                    "and not exists(select 1 from msg_tomb where mid = gi.mid) "
                    "limit 1",
                    tag,
                    to_hex(uid)
                    );
#endif
        c = (res.num_elems() != 1);
        //sql_commit_transaction();
    }
    catch(...)
    {
        //sql_rollback_transaction();
    }
    return c;

}

// return a list of uid's that appear to have updates since
// the given time. takes into account tags and tag tombstones
// and potentially new messages only. if the sync stuff loads
// old messages in the background, this would not be reflected in
// these queries. we would need some kind of other update indicator
// for that (maybe that could be handled with a trigger.)
// note: this is expected to be used with relatively small intervals of
// time, like for a mobile device switching in and out of the app, so the
// result sets will likely be tiny.
vc
sql_uid_updated_since(vc time)
{
    vc ret(VC_VECTOR);
    try
    {
        sql_start_transaction();
        vc res = sql_simple(
                    "WITH "
                    "newtags(mid) as (select mid from gmt where time > ?1 union select mid from gmt,gtomb using(guid) where gtomb.time > ?1) "
                    "select distinct assoc_uid from gi,newtags using(mid) union select assoc_uid from gi where date > ?1",
                    time);
        sql_commit_transaction();
        // map down the results
        for(int i = 0; i < res.num_elems(); ++i)
        {
            vc uid = from_hex(res[i][0]);
            uid = map_to_representative_uid(uid);
            if(!ret.contains(uid))
            {
                ret.append(uid);
            }
        }
    }
    catch(...)
    {
        sql_rollback_transaction();
    }
    return ret;
}

// NOTE: if a message hasn't been seen yet (in other words, another
// client in the group might have more messages we haven't seen), it will not show up in this count.
int
sql_uid_count_tag(vc uid, vc tag)
{
    int c = 0;
    try
    {
        sql_start_transaction();
        vc res = sql_simple(
                    with_create_uidset(2)
                    "select count(distinct mid) from gmt,gi using(mid) where assoc_uid in (select * from uidset) and tag = ?1 and not exists (select 1 from gtomb where guid = gmt.guid)",
                            tag,
                    to_hex(uid));
        c = res[0][0];
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

