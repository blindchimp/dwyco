#include "backandroid.h"
#include "simplesql.h"
#include "qmsgsql.h"
#include "fnmod.h"
#include "qmsg.h"
#include "qauth.h"
#include "ser.h"


#ifdef _WIN32
#ifdef _MSC_VER
#include <direct.h>
#endif
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace dwyco {

struct backup_sql : public SimpleSql
{
    backup_sql() : SimpleSql("aback.sql") {}

    void init_schema(const DwString&) {
        sql_simple("create table if not exists msgs ("
                   "from_uid text collate nocase,"
                   "mid text unique on conflict ignore,"
                   "msg blob,"
                   "attfn text,"
                   "att blob);"
                  );
        sql_simple("create table if not exists misc_blobs (name text unique on conflict replace, data blob);");
        sql_simple("create table if not exists bu (lock integer not null default 0, "
                   "state integer not null, date_sent integer default 0, date_ack integer default 0, date_updated default 0, version default 0, primary key (lock), check (lock = 0));");
        sql_simple("insert or ignore into bu(lock, state) values(0, 0);");

        sql_simple("create index if not exists from_uid_idx on msgs(from_uid);");
        sql_simple("create index if not exists mid_idx on msgs(mid);");
        sql_simple("create table if not exists tags("
                   "mid text not null, "
                   "tag text not null, "
                   "time integer not null, "
                   "uid text not null, "
                   "guid text not null collate nocase, "
                   "unique(mid, tag, uid, guid) on conflict ignore)");
    }


};

static backup_sql *Db;

#define sql Db->sql_simple

namespace backandroid {

void
sql_start_transaction()
{
    Db->start_transaction();
}
void
sql_commit_transaction()
{
    Db->commit_transaction();
}
void
sql_rollback_transaction()
{
    Db->rollback_transaction();
}

}
using namespace dwyco::backandroid;

static
vc
blob(vc m)
{
    vc ret(VC_VECTOR);
    ret.append("blob");
    ret.append(m);
    return ret;
}

static
void
sql_insert_record(vc from_uid, vc mid, vc msg, vc attfn, vc att)
{

    sql("replace into main.msgs values(?1,?2,?3,?4,?5)",
        to_hex(from_uid),
        mid,
        blob(serialize(msg)),
        attfn,
        blob(att)
        );
}

static
void
backup_msg(const vc& uid, const vc& mid)
{
    DwString dir = (const char *)uid_to_dir(uid);
    vc ret = load_body_by_id(uid, mid);
    if(ret.is_nil())
        return;

    if(ret[QM_BODY_ATTACHMENT].is_nil())
    {
        sql_insert_record(uid, mid, ret, vcnil, vcnil);
    }
    else
    {
        vc attfn = ret[QM_BODY_ATTACHMENT];
        char *buf = 0;
        int fd = -1;
        try
        {

            DwString actual_fn = newfn(dir);
            actual_fn += DIRSEPSTR;
            actual_fn += (const char *)attfn;
            if(access(actual_fn.c_str(), 0) != 0)
            {
                throw -1;
            }
            struct stat s;
            if(stat(actual_fn.c_str(), &s) == -1)
                throw -1;
            int fd = open(actual_fn.c_str(), O_RDONLY);
            if(fd == -1)
                throw -1;
            buf = new char[s.st_size];
            if(buf == 0)
                throw -1;
            if(read(fd, buf, s.st_size) != s.st_size)
                throw -1;
            sql_insert_record(uid, mid, ret, attfn, vc(VC_BSTRING, buf, s.st_size));
            delete [] buf;
            buf = 0;
            close(fd);
        }
        catch(...)
        {
            if(fd != -1)
                close(fd);
            delete [] buf;
            sql_insert_record(uid, mid, ret, attfn, "");
        }

    }

}

static
bool
attempt_backup(vc v)
{
    try
    {
        const vc msgs = sql(v);
        // note: commit after every message, to try and sneak right up
        // to the limit. if there is a large query result
        // we want to get as much as possible in, instead of just failing
        // and dropping the whole thing.
        for(int i = 0; i < msgs.num_elems(); ++i)
        {
            sql_start_transaction();
            vc uid = from_hex(msgs[i][0]);
            vc mid = msgs[i][1];
            backup_msg(uid, mid);
            sql_commit_transaction();
        }

        return true;

    } catch (...) {

    }
    return false;
}

void
android_backup()
{
    if(Db)
        return;
    Db = new backup_sql;
    if(!Db->init())
        oopanic("can't init backup");
    Db->attach(newfn(MSG_IDX_DB), "mi");
    Db->attach(newfn(TAG_DB), "mt");
    Db->sync_off();

    try
    {
        sql_start_transaction();
        sql("delete from main.msgs where main.msgs.mid in (select mid from mi.msg_tomb)");
        sql("delete from main.msgs where main.msgs.mid not in (select mid from mi.msg_idx)");
        sql("delete from main.tags");
        sql_commit_transaction();
        Db->vacuum();
        // android autobackup limit
        Db->set_max_size(24);

        // plain texts that are favorited
        if(!attempt_backup(
                    "select assoc_uid, mid as favmid from mi.msg_idx,mt.gmt using(mid) where tag = '_fav' and has_attachment isnull "
                   "and not exists (select 1 from main.msgs where favmid = mid)"
                   " order by logical_clock desc"))
        {
            goto done;
        }
        // favorites with attachments
        if(!attempt_backup(
                    "select assoc_uid, mid as favmid from mi.msg_idx,mt.gmt using(mid) where tag = '_fav' and has_attachment notnull "
                   "and not exists (select 1 from main.msgs where favmid = mid)"
                   " order by logical_clock desc"))
        {
            goto done;
        }

        try
        {
            sql_start_transaction();
            sql("insert into main.tags select * from mt.gmt where tag in (select * from mt.static_crdt_tags)");
            sql_commit_transaction();
        }
        catch(...)
        {
            sql_rollback_transaction();
            goto done;
        }

        // plain texts
        if(!attempt_backup(
                    "select assoc_uid, mid as favmid from mi.msg_idx where has_attachment isnull "
                   "and not exists (select 1 from main.msgs where favmid = mid)"
                   " order by logical_clock desc"))
        {
            goto done;
        }
        // with attachments
        if(!attempt_backup(
                    "select assoc_uid, mid as favmid from mi.msg_idx where has_attachment notnull "
                   "and not exists (select 1 from main.msgs where favmid = mid)"
                   " order by logical_clock desc"))
        {

        }
        done:
        ;
    }
    catch(vc err)
    {
        sql_rollback_transaction();
        if(err == vc("full"))
        {

        }
    }
    catch(...)
    {

    }
    Db->exit();
    delete Db;
    Db = 0;
}


}
