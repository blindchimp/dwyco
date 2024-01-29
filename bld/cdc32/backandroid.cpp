#include "backandroid.h"
#include "simplesql.h"
#include "qmsgsql.h"
#include "fnmod.h"
#include "qmsg.h"
#include "qauth.h"
#include "ser.h"
#include "xinfo.h"
#include "sepstr.h"
#include "ta.h"


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
#define BACKUP_FREQ_DEFAULT (3 * 24 * 3600)
// Note: Enable_backups can be set by the server to enable
// automatic remote backups. Since this may contain sensitive
// info, some thought needs to go into keeping this secure, or
// at least disclosing to the user what the situation is.
// ca 2018, the test implementation of remote backups is
// disabled pending some better key management and some
// technique to allow restores from the remote backup.
// If Enable_backups is 0, local backups are still created,
// they just are not sent out to the "backup server".
int Enable_backups;
int Backup_freq = BACKUP_FREQ_DEFAULT;

struct backup_sql : public SimpleSql
{
    backup_sql(const char *f = "aback.sql") : SimpleSql(f) {}

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
        sql_simple("insert or ignore into bu(lock, state, date_sent) values(0, 0, strftime('%s', 'now'));");

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

// this has suffered cut and paste enough times that
// a bit of remodularization is probably in order.
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
            // note: this is probably not a good idea
            //sql_insert_record(uid, mid, ret, attfn, "");
            throw;
        }

    }

}

static
bool
attempt_backup(const vc& v, int batch_size = 1)
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
        int b = 0;
        for(int i = 0; i < msgs.num_elems(); ++i)
        {
            if(b == 0)
                sql_start_transaction();
            const vc& uid = from_hex(msgs[i][0]);
            const vc& mid = msgs[i][1];
            backup_msg(uid, mid);
            ++b;
            if(b == batch_size)
            {
                sql_commit_transaction();
                b = 0;
            }
        }
        if(b != 0)
            sql_commit_transaction();
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
    {
        delete db;
        return 0;
    }

    vc res = db->sql_simple("select date_updated from main.bu");
    db->exit();
    delete db;
    vc r2 = res[0][0];
    if(r2.type() != VC_INT)
        return 0;
    long long du = r2;
    auto now = time(0);
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
    {
        delete db;
        return 0;
    }

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
    {
        delete db;
        return 0;
    }

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

// note: this should probably be updated to allow newer messages to
// be loaded into the backup, erasing older ones. for now it is ok, but
// this will be more useful if it has the latest information in it i think.

void
android_backup()
{
    if(Db)
        return;
    // since this is a small backup, just delete and do it from scratch
    DeleteFile(newfn("aback.sql").c_str());
    Db = new backup_sql;
    if(!Db->init())
    {
        delete Db;
        return;
        oopanic("can't init backup");
    }
    Db->sync_off();
    Db->optimize();
    Db->attach(newfn(MSG_IDX_DB), "mi");
    Db->attach(newfn(TAG_DB), "mt");

    try
    {

        sql_start_transaction();
        //sql("delete from main.msgs where main.msgs.mid in (select mid from mi.msg_tomb)");
        //sql("delete from main.msgs where main.msgs.mid not in (select mid from mi.msg_idx)");
        //sql("delete from main.msgs where not exists (select 1 from main.tags,main.msgs using(mid) where main.tags.tag = '_fav')");
        //sql("delete from main.tags");
        sql("update main.bu set date_updated = strftime('%s', 'now'), state = 1");
        sql_commit_transaction();
        Db->vacuum();
        // android autobackup limit
        Db->set_max_size(24);

        // these tags are user-generated and usually pretty small
        // since we don't allow backups to be loaded in group mode,
        // only store data that would be used in non-group mode
        try
        {
            sql_start_transaction();
            sql("create temp table static_uid_tags(tag text not null)");
            sql("insert into static_uid_tags values('_ignore')");
            sql("insert into static_uid_tags values('_pal')");
            sql("insert into static_uid_tags values('_leader')");
            sql("insert into main.tags select * from mt.gmt where tag in (select * from mt.static_crdt_tags) "
                "and rowid in (select max(rowid) from mt.gmt group by mid,tag) "
                "and (mid in (select mid from msg_idx) or tag in (select * from temp.static_uid_tags))"
                );
            sql("update main.tags set uid = ?1", to_hex(My_UID));
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

static
void
sql_insert_misc(vc name, vc data, const char *dbn = "main")
{
    sql(DwString("replace into %1.misc_blobs values(?1,?2);").arg(dbn).c_str(), name, blob(data));
}

static
int
backup_file(const char *name, const char *dbn)
{
    char *buf = 0;
    int fd = -1;
    try
    {
        DwString actual_fn = newfn(name);
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

        sql_insert_misc(name, vc(VC_BSTRING, buf, s.st_size), dbn);

        delete [] buf;
        buf = 0;
        close(fd);
    }
    catch(...)
    {
        if(fd != -1)
            close(fd);
        delete [] buf;
        throw;
    }
    return 1;
}

static
int
backup_account_info(const char *dbn)
{
    // this might be a problem, but in the interest of max
    // robustness wrt to msg delivery, go ahead and put priv key in here. the problem
    // is not just that there might be messages encrypted
    // waiting to be delivered, but that the public key has
    // been cached by other clients, and at this time, the clients
    // don't assume the pubkey needs to be updated from the servers
    // all that often. this isn't a huge problem as long as the backups
    // are local or encrypted.
    // the other option is to leave it out of the backup, and have the client generate a
    // new key, and update it with the server. at this time (ca 2017), the
    // server will refuse to sign it (ie, it thinks it is suspicious), but
    // will disseminate it anyway.
    // note: the tag database probably needs to be trimmed before backup (or maybe
    // as restore time.)
    //
    // also note: we do not store the group names and keys, because presumably, after
    // the restore, they should be able to re-enter the group (the first time they
    // restart cdc-x after the restore, they will be removed from the group by the
    // server.)

    try
    {
        // these are a pair
        sql_start_transaction();
        backup_file("auth", dbn);
        backup_file("dh.dif", dbn);
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }
    // not a deal breaker if these don't make it in together
    //backup_file(TAG_DB, dbn);
    // don't back this up, it is just user settings they may
    // want to adjust when they get their messages, but it isn't
    // necessary to reload it, possibly coming up in a weird state.
    //backup_file("set.sql", dbn);
    // this file is useful if you restore but don't have
    // access to the internet to download profiles from
    // the servers.
    backup_file("sinfo", dbn);
    return 1;
}

static
vc
get_file_contents(const char *name, const char *dbn)
{
    try
    {
        vc res = sql(DwString("select data from %1.misc_blobs where name = ?1").arg(dbn).c_str(), name);
        if(res.is_nil())
            return vcnil;
        if(res.num_elems() == 0)
            return vcnil;
        return res[0][0];
    }
    catch(...)
    {
        return vcnil;
    }
}

static
int
restore_blob(const char *name, const char *dbn)
{
    vc blob = get_file_contents(name, dbn);
    if(blob.is_nil())
        return 0;
    // this works if the file is open too on windows, which is likely
    // with a database file.
    DwString rfn = gen_random_filename();
    DwString tf(name);
    tf = newfn(tf);
    DwString sv(tf);
    sv += ".";
    sv += rfn;
    move_replace(tf, sv);

#ifdef _Windows
    int fd = creat(newfn(name).c_str(), _S_IWRITE);
#else
    int fd = creat(newfn(name).c_str(), 0666);
#endif
    if(fd == -1)
        return 0;
    int ret = 1;
    if(write(fd, (const char *)blob, blob.len()) != blob.len())
    {
        ret = 0;
    }
    close(fd);
    return ret;
}

static
int
restore_account_info(const char *dbn)
{
    vc auth = get_file_contents("auth", dbn);
    if(auth.is_nil())
        return 0;
    vc dh = get_file_contents("dh.dif", dbn);
    if(dh.is_nil())
        return 0;


    if(!restore_blob("auth", dbn))
        return 0;
    if(!restore_blob("dh.dif", dbn))
        return 0;
    // if we can't restore the tags, we can still get going
    // without them, so don't error out.
    //restore_blob(TAG_DB, dbn);
    // likewise with sinfo
    restore_blob("sinfo", dbn);


    // invalidate the profile that might be cached locally
    vc new_id;
    vc sk;

    if(!load_auth_info(new_id, sk, "auth"))
        return 0;
    //prf_invalidate(new_id);

    return 1;
}

int
desktop_days_since_last_backup()
{
    backup_sql db("bun.sql");

    if(!db.init())
    {
        return -1;
    }
    try
    {
        vc res = db.sql_simple("select date_updated from main.bu");

        long long du = res[0][0];
        if(du == 0)
            return -1;
        long long now = time(0);
        return (now - du) / (24L * 3600);
    }
    catch(...)
    {
        return -1;
    }
}

int
desktop_days_since_backup_created()
{
    backup_sql db("bun.sql");
    if(!db.init())
    {
        return -1;
    }

    try
    {
        vc res = db.sql_simple("select date_sent from main.bu");

        long long du = res[0][0];
        long long now = time(0);
        return (now - du) / (24L * 3600);
    }
    catch (...)
    {
        return -1;
    }
}


int
desktop_backup()
{
    if(Db)
        return 0;
    Db = new backup_sql("bun.sql");
    // in the field, it appears backups get trashed occasionally, so
    // if there is any problem at all during backup creation, just remove it
    // and hope the next one will work.
    bool redo_backup = false;

    try
    {
        if(!Db->init())
        {
            throw -1;
            return 0;
            //oopanic("can't init backup");
        }
        Db->sync_off();
        Db->optimize();
        Db->attach(newfn(MSG_IDX_DB), "mi");
        Db->attach(newfn(TAG_DB), "mt");

        sql_start_transaction();
        sql("delete from main.msgs where main.msgs.mid in (select mid from mi.msg_tomb)");
        //sql("delete from main.msgs where main.msgs.mid not in (select mid from mi.msg_idx)");
        //sql("delete from main.msgs where not exists (select 1 from main.tags,main.msgs using(mid) where main.tags.tag = '_fav')");
        sql("delete from main.tags");
        sql("update main.bu set date_updated = strftime('%s', 'now'), state = 1");
        sql_commit_transaction();
        Db->vacuum();


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
            throw -1;
        }

        // everything not already backed up
        if(!attempt_backup(
                "select assoc_uid, mid from mi.msg_idx where mid not in (select mid from main.msgs) "
                " order by logical_clock desc", 30))
        {
            // note: if attempt_backup returns 0, it could be the disk is full, not necessarily that
            // the database is corrupt. so make an attempt to store the account info. if that fails,
            // we probably need to redo the backup.
        }
        backup_account_info("main");
    }
    catch(vc err)
    {
        sql_rollback_transaction();
        if(err == vc("full"))
        {
            // this may be transient, and there isn't really
            // any indication the backup is trashed in this case.
            // so just ignore it for now.
        }
        else
        {
            redo_backup = true;
        }
    }
    catch(...)
    {
        redo_backup = true;
    }

    Db->exit();
    delete Db;
    Db = 0;
    if(redo_backup)
    {
        TRACK_ADD(BU_remove_trashed_backup, 1);
        DeleteFile(newfn("bun.sql").c_str());
        return 0;
    }
    return 1;
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
    {
        delete Db;
        Db = 0;
        return 0;
    }

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

int
restore_msgs(const char *cfn, int msgs_only)
{
    // note: this filename is coming from the user's external
    // filesystem, so we don't want to subject it to the same
    // filename munging we do for internal names
    DwString fn(cfn);

    int ret = 1;
    if(Db)
        return 0;
    Db = new backup_sql(cfn);
    if(!Db->init(SQLITE_OPEN_READWRITE, true))
    {
        delete Db;
        Db = 0;
        return 0;
    }

    Db->attach(newfn(TAG_DB), "mt");
    Db->sync_off();

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
