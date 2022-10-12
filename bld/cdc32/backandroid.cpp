#include "backandroid.h"
#include "simplesql.h"
#include "qmsgsql.h"
#include "fnmod.h"
#include "qmsg.h"
#include "qauth.h"
#include "ser.h"
#include "xinfo.h"
#include "sepstr.h"


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

#ifdef DWYCO_USE_STATIC_SQLITE
#include "sqlite/sqlite3.h"
#else
#include <sqlite3.h>
#endif

// this autobackup is really not a good archive solution. it is just
// good enough to get you going again with a basic set of messages
// after you have uninstalled/reinstalled or loaded fresh on a new
// phone or something.
// there are lots of problems with the size limit of 25MB, especially
// with attachments that are large.

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
                   "state integer not null, date_sent integer default 0, date_ack integer default 0, date_updated integer default 0, version integer default 0, primary key (lock), check (lock = 0));");
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
sql_insert_record(const vc& from_uid, const vc& mid, const vc& msg, const vc& attfn, const vc& att)
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
    const DwString dir = (const char *)uid_to_dir(uid);
    const vc ret = load_body_by_id(uid, mid);
    if(ret.is_nil())
        return;

    if(ret[QM_BODY_ATTACHMENT].is_nil())
    {
        sql_insert_record(uid, mid, ret, "", "");
    }
    else
    {
        const vc attfn = ret[QM_BODY_ATTACHMENT];
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
attempt_backup(const vc& v)
{
    try
    {
        sql_start_transaction();
        const vc& msgs = sql(v);
        sql_commit_transaction();
        // note: commit after every message, to try and sneak right up
        // to the limit. if there is a large query result
        // we want to get as much as possible in, instead of just failing
        // and dropping the whole thing.
        for(int i = 0; i < msgs.num_elems(); ++i)
        {
            sql_start_transaction();
            const vc& uid = from_hex(msgs[i][0]);
            const vc& mid = msgs[i][1];
            backup_msg(uid, mid);
            sql_commit_transaction();
        }

        return true;

    } catch (...) {
        sql_rollback_transaction();

    }
    return false;
}

int
android_days_since_last_backup()
{
    auto db = new backup_sql;
    if(!db->init())
        oopanic("can't init backup");

    vc res = db->sql_simple("select date_updated from main.bu");
    db->exit();
    delete db;

    long long du = res[0][0];
    long long now = time(0);
    return (now - du) / (24L * 3600);
}

int
android_get_backup_state()
{
    auto db = new backup_sql;
    // note: if the backup file doesn't exist, same
    // as empty backup file, but don't create one
    // if it isn't there.
    if(!db->init(SQLITE_OPEN_READWRITE))
        return 0;

    vc res = db->sql_simple("select state from main.bu");
    db->exit();
    delete db;
    return res[0][0];
}

int
android_set_backup_state(int i)
{
    auto db = new backup_sql;
    if(!db->init())
        oopanic("can't init backup");
    int ret = 1;
    try
    {
        db->start_transaction();
        db->sql_simple("update main.bu set state = ?1", i);
        db->commit_transaction();
    }
    catch(...)
    {
        db->rollback_transaction();
        ret = 0;
    }

    db->exit();
    delete db;
    return ret;
}


void
android_backup()
{
    if(Db)
        return;
    Db = new backup_sql;
    if(!Db->init())
        oopanic("can't init backup");
    Db->sync_off();
    Db->optimize();
    Db->attach(newfn(MSG_IDX_DB), "mi");
    Db->attach(newfn(TAG_DB), "mt");

    try
    {

        sql_start_transaction();
        sql("delete from main.msgs where main.msgs.mid in (select mid from mi.msg_tomb)");
        sql("delete from main.msgs where main.msgs.mid not in (select mid from mi.msg_idx)");
        sql("delete from main.msgs where not exists (select 1 from main.tags,main.msgs using(mid) where main.tags.tag = '_fav')");
        sql("delete from main.tags");
        sql("update main.bu set date_updated = strftime('%s', 'now'), state = 1");
        sql_commit_transaction();
        Db->vacuum();
        // android autobackup limit
        Db->set_max_size(24);

        // these tags are user-generated and usually pretty small
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

        // plain texts that are favorited from pals
        if(!attempt_backup(
                    "select assoc_uid, mid as favmid from mi.msg_idx,mt.gmt using(mid) where tag = '_fav' and has_attachment isnull "
                    "and assoc_uid in (select mid from mt.gmt where tag = '_pal') "
                   "and not exists (select 1 from main.msgs where favmid = mid) "
                   " order by logical_clock desc"))
        {
            goto done;
        }

        // plain texts that are favorited
        if(!attempt_backup(
                    "select assoc_uid, mid as favmid from mi.msg_idx,mt.gmt using(mid) where tag = '_fav' and has_attachment isnull "
                   "and not exists (select 1 from main.msgs where favmid = mid)"
                   " order by logical_clock desc"))
        {
            goto done;
        }

        // favorites with attachments from pals
        if(!attempt_backup(
                    "select assoc_uid, mid as favmid from mi.msg_idx,mt.gmt using(mid) where tag = '_fav' and has_attachment notnull "
                    "and assoc_uid in (select mid from mt.gmt where tag = '_pal') "
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

        // plain texts from pals
        if(!attempt_backup(
                    "select assoc_uid, mid as favmid from mi.msg_idx where has_attachment isnull "
                    "and assoc_uid in (select mid from mt.gmt where tag = '_pal') "
                   "and not exists (select 1 from main.msgs where favmid = mid)"
                   " order by logical_clock desc"))
        {
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

// note: uid is assumed be hex already here
static
int
make_msg_folder(const vc& uid, DwString* fn_out)
{
    if(uid.len() == 0)
        return 0;
    DwString s((const char *)uid);
    s += ".usr";
    s = newfn(s);
    if(fn_out)
        *fn_out = s;
    GRTLOG("restoring user %s", s.c_str(), 0);
    int ret = mkdir(s.c_str());
    GRTLOG("mkdir %d", ret, 0);
    return 1;
}

// note: uid is assumed to hex
static
int
restore_msg(const vc& uid, const vc& mid)
{
    GRTLOG("restore msg %s %s", (const char *)uid, (const char *)mid);
    try
    {
    const vc res = sql("select msg, attfn, att from msgs where mid = ?1", mid);

    vc msg;
    if(!deserialize(res[0][0], msg))
        return 0;
    const vc& attfn = res[0][1];
    const vc& att = res[0][2];

    DwString dir = (const char *)uid;
    dir += ".usr";
    dir = newfn(dir);
    DwString fn = (const char *)mid;
    fn += msg[QM_BODY_SENT].is_nil() ? ".bod" : ".snt";
    dir += DIRSEPSTR;
    DwString ffn = dir;
    ffn += fn;
    if(!save_info(msg, ffn.c_str(), 1))
        return 0;
    if(attfn.len() > 0)
    {
        DwString actual_attfn = dir;
        actual_attfn += (const char *)attfn;
        //don't overwrite attachments if it already exists
#ifdef _Windows
        if(_access(actual_attfn.c_str(), 0) == 0)
            return 1;
        int fd = _creat(actual_attfn.c_str(), _S_IWRITE);
#else
        if(access(actual_attfn.c_str(), F_OK) == 0)
            return 1;
        int fd = creat(actual_attfn.c_str(), 0666);
#endif
        if(fd == -1)
            return 0;
        if(write(fd, (const char *)att, att.len()) != att.len())
        {
            close(fd);
            DeleteFile(actual_attfn.c_str());
            return 0;
        }
        close(fd);
    }
    }
    catch(...)
    {
        return 0;
    }

    return 1;

}

int
android_restore_msgs()
{
    int ret = 1;
    if(Db)
        return 0;
    Db = new backup_sql;
    if(!Db->init())
        oopanic("can't init backup");
    //Db->attach(newfn(MSG_IDX_DB), "mi");
    Db->attach(newfn(TAG_DB), "mt");
    Db->sync_off();
    GRTLOG("android restore", 0, 0);

    try
    {
        vc res;

        res = sql("select distinct(from_uid) from main.msgs");

        // we are likely to be loading something at this point,
        // so kill the index so it is rebuilt
        DeleteFile(newfn(MSG_IDX_DB).c_str());

        for(int i = 0; i < res.num_elems(); ++i)
        {
            const vc& uid = res[i][0];
            make_msg_folder(uid, 0);
        }

        res = sql("select from_uid, mid from main.msgs");

        for(int i = 0; i < res.num_elems(); ++i)
        {
            const vc& uid = res[i][0];
            const vc& mid = res[i][1];
            restore_msg(uid, mid);
        }

        // merge in the tags from the backup
        sql_start_transaction();
        sql("insert into mt.gmt select * from main.tags");
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
        ret = 0;
    }
    Db->exit();
    delete Db;
    Db = 0;
    return ret;
}


}
