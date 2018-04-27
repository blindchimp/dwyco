
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef _WIN32
#ifdef __BORLANDC__
#include <dir.h>
#include <sys\stat.h>
#endif
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
#include "sha3.h"
#include "backsql.h"
#include "sepstr.h"
#include "ser.h"
#include "prfcache.h"
#include "ta.h"

using namespace CryptoPP;
extern vc Pals;

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
int Trim_backups;
int Set_main_backup_state;
int Set_diff_backup_state;


#define BUBOT_UID "\xce\xd8\x69\xc8\xad\xdc\x60\xc8\xe7\x5a"
#define BUBOT_UID_LEN 10

struct close_and_remove
{
    int fd;
    DwString fn;
    close_and_remove(int fd, const DwString& fn) {
        this->fd = fd;
        this->fn = fn;
    }
    ~close_and_remove() {
        if(fd != -1)
            close(fd);
        DeleteFile(fn.c_str());
    }
};

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
    sql_simple("create table if not exists msgs ("
               "from_uid text,"
               "mid text unique on conflict ignore,"
               "msg blob,"
               "attfn text,"
               "att blob);"
              );
    sql_simple("create table if not exists misc_blobs (name text unique on conflict replace, data blob);");
    sql_simple("create table if not exists bu (lock integer not null default 0, "
               "state integer not null, date_sent integer default 0, date_ack integer default 0, date_updated default 0, version default 0, primary key (lock), check (lock = 0));");
    sql_simple("insert or ignore into bu(lock, state) values(0, 0);");
//sql_simple("create table if not exists indexed_flag (uid text primary key);");
    sql_simple("create index if not exists from_uid_idx on msgs(from_uid);");
    sql_simple("create index if not exists mid_idx on msgs(mid);");
//sql_simple("create index if not exists logical_clock_idx on msg_idx(logical_clock desc);");
//sql_simple("create index if not exists date_idx on msg_idx(date desc);");

}

static
void
init_backup_sql()
{
    if(Db)
        oopanic("already init");
    if(sqlite3_open(newfn("bu.sql").c_str(), &Db) != SQLITE_OK)
    {
        Db = 0;
        return;
    }
    try
    {
        init_schema();
        sqlite3 *tmpdb = Db;
        if(sqlite3_open(newfn("dbu.sql").c_str(), &Db) == SQLITE_OK)
        {
            init_schema();
            sqlite3_close_v2(Db);
        }

        Db = tmpdb;
        sql_simple(DwString("attach '%1' as dbu;").arg(newfn("dbu.sql")).c_str());
    }
    catch(...)
    {
        Db = 0;
    }

}

static
void
exit_backup_sql()
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
    sql_simple("begin immediate transaction;");
}

static
void
sql_commit_transaction()
{
    sql_simple("commit transaction;");
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
    a[0] = "rollback transaction;";
    sqlite3_bulk_query(Db, &a);
}

static
void
sql_insert_record(vc from_uid, vc mid, vc msg, vc attfn, vc att, const char *dbn = "main")
{
    VCArglist a;
    a[0] = DwString("replace into %1.msgs values($1,$2,$3,$4,$5);").arg(dbn).c_str();

    a.append(to_hex(from_uid));
    a.append(mid);
    vc vm(VC_VECTOR);
    vm[0] = "blob";
    vm[1] = serialize(msg);
    a.append(vm);
    a.append(attfn);
    vc vm2(VC_VECTOR);
    vm2[0] = "blob";
    vm2[1] = att;
    a.append(vm2);

    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
}

static
void
sql_insert_misc(vc name, vc data, const char *dbn = "main")
{
    VCArglist a;
    a[0] = DwString("replace into %1.misc_blobs values($1,$2);").arg(dbn).c_str();

    a.append(name);

    vc vm2(VC_VECTOR);
    vm2[0] = "blob";
    vm2[1] = data;
    a.append(vm2);

    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
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
void
backup_user(vc v)
{
    vc uid = v[0];
    DwString dir = (const char *)uid_to_dir(uid);
    vc ret = load_bodies(uid_to_dir(uid), 1);
    int n = ret.num_elems();
    for(int i = 0; i < n; ++i)
    {
        vc mid = ret[i][QM_BODY_ID];
        if(ret[i][QM_BODY_ATTACHMENT].is_nil())
        {
            sql_insert_record(uid, mid, ret[i], vcnil, "");
        }
        else
        {
            vc attfn = ret[i][QM_BODY_ATTACHMENT];
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
                sql_insert_record(uid, mid, ret[i], attfn, vc(VC_BSTRING, buf, s.st_size));
                delete [] buf;
                buf = 0;
                close(fd);
            }
            catch(...)
            {
				if(fd != -1)
					close(fd);
                delete [] buf;
                sql_insert_record(uid, mid, ret[i], attfn, "");
            }

        }

    }

}

static
void
backup_msg(vc uid, vc mid)
{
    DwString dir = (const char *)uid_to_dir(uid);
    vc ret = load_body_by_id(uid, mid);
    if(ret.is_nil())
        return;

    if(ret[QM_BODY_ATTACHMENT].is_nil())
    {
        sql_insert_record(uid, mid, ret, vcnil, "", "dbu");
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
            sql_insert_record(uid, mid, ret, attfn, vc(VC_BSTRING, buf, s.st_size), "dbu");
            delete [] buf;
            buf = 0;
            close(fd);
        }
        catch(...)
        {
            close(fd);
            delete [] buf;
            sql_insert_record(uid, mid, ret, attfn, "", "dbu");
        }

    }

}

static
vc
get_new_msgs()
{
    VCArglist a;
    a[0] = "select assoc_uid, mid from mi.msg_idx where mid not in (select mid from msgs union select mid from dbu.msgs);";

    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
    return res;
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
    backup_file("fav.sql", dbn);
    backup_file("pals", dbn);
    return 1;
}

static
vc
get_file_contents(const char *name, const char *dbn)
{
    VCArglist a;
    a[0] = DwString("select data from %1.misc_blobs where name = $1;").arg(dbn).c_str();
    a[1] = name;
    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        return vcnil;
    if(res.num_elems() == 0)
        return vcnil;
    return res[0][0];
}

static
void
append_to_pal(vc, vc kv)
{
    Pals.add_kv(kv[0], vcnil);
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
    vc pals = get_file_contents("pals", dbn);
    if(pals.is_nil())
        return 0;
    DwString rfn = gen_random_filename();
    DwString tf("auth");
    tf = newfn(tf);
    DwString sv(tf);
    sv += ".";
    sv += rfn;
    move_replace(tf, sv);

    tf = newfn("dh.dif");
    sv = tf;
    sv += ".";
    sv += rfn;
    move_replace(tf, sv);

    int fd = creat(newfn("auth").c_str(), 0666);
    if(fd == -1)
        return 0;
    if(write(fd, (const char *)auth, auth.len()) != auth.len())
        return 0;
    close(fd);
    fd = creat(newfn("dh.dif").c_str(), 0666);
    if(fd == -1)
        return 0;
    if(write(fd, (const char *)dh, dh.len()) != dh.len())
        return 0;
    close(fd);

    vc p;
    if(!deserialize(pals, p))
        return 0;
    p.foreach(vcnil, append_to_pal);
    if(!save_info(Pals, "pals"))
        return 0;
    // invalidate the profile that might be cached locally
    vc new_id;
    vc sk;

    if(!load_auth_info(new_id, sk, "auth"))
        return 0;
    prf_invalidate(new_id);
    return 1;
}

static
int
create_incr_backup()
{
    DwString mfn = newfn("mi.sql");
    DwString ffn = newfn("fav.sql");
    //DwString dfn = newfn("dbu.sql");

    try
    {
        sql_simple(DwString("attach '%1' as mi;").arg(mfn).c_str());
        sql_simple(DwString("attach '%1' as fav;").arg(ffn).c_str());
        //sql_simple(DwString("attach '%1' as dbu;").arg(dfn).c_str());
        sql_start_transaction();

        vc nm = get_new_msgs();
        for(int i = 0; i < nm.num_elems(); ++i)
        {
            backup_msg(from_hex(nm[i][0]), nm[i][1]);
        }
        sql_simple("update dbu.bu set state = 1 where state = 0;");
        if(nm.num_elems() > 0)
        {
            sql_simple("update dbu.bu set date_updated = strftime('%s', 'now');");
        }

        sql_commit_transaction();
        sql_simple("detach mi;");
        sql_simple("detach fav;");
        backup_account_info("dbu");
        //sql_simple("detach dbu;");
        return nm.num_elems();

    }
    catch(...)
    {
        sql_rollback_transaction();
        sql_simple("detach mi;");
        sql_simple("detach fav;");
        //sql_simple("detach dbu;");
        return 0;
    }

}



static
int
create_full_backup()
{
    load_users(0, 0);
    sql_start_transaction();
    MsgFolders.foreach(vcnil, backup_user);
    sql_simple("update bu set state = 1;");
    sql_commit_transaction();
    backup_account_info("main");
    return 1;
}

static
int
get_state(const char *dbn = "main")
{
    VCArglist a;
    a[0] = DwString("select state from %1.bu;").arg(dbn).c_str();

    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
    return (int)res[0][0];
}

static
int
get_date(const char *field_name, const char *dbn = "main")
{
    VCArglist a;
    a[0] = DwString("select %2 from %1.bu;").arg(dbn, field_name).c_str();

    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        throw -1;
    return (int)res[0][0];
}

static
int
send_in_chunks(const DwString& actual_fn, const char *dbn = "main")
{
    // note: this was never tested in the field. it worked ok in-house
    // but i suspect the "all or nothing" aspect of this sort of backup
    // won't work too well. if the key management and privacy issues
    // are sorted out, it might be better to create backups in small
    // independent chunks so missing parts don't obliterate an entire backup.
    return 0;
}

#if 0
static
int
send_backup_file(const DwString& actual_fn)
{
    char a[16];
    memset(a, 0, sizeof(a));
    vc k(VC_BSTRING, a, sizeof(a));
    DwString rfn = gen_random_filename();
    rfn += ".tmp";
    rfn = newfn(rfn);
    if(!encrypt_attachment(actual_fn.c_str(), k, rfn.c_str()))
    {
        return 0;
    }
    int compid = dwyco_make_file_zap_composition(rfn.c_str(), rfn.length());
    if(compid == 0)
    {
        DeleteFile(rfn.c_str());
        return 0;
    }

    const char *pers;
    int len_pers;

    if(!dwyco_zap_send5(compid, BUBOT_UID, BUBOT_UID_LEN, "backup", 6, 0, 0, &pers, &len_pers))
    {
        DeleteFile(rfn.c_str());
        dwyco_delete_zap_composition(compid);
        return 0;
    }
    DwString p(pers, 0, len_pers);

    try
    {
        sql_start_transaction();
        sql_simple("update bu set state = 2, date_sent = strftime('%s', 'now');");
        sql_commit_transaction();
        DeleteFile(rfn.c_str());
        return 1;
    }
    catch(...)
    {
        sql_rollback_transaction();
        dwyco_kill_message(p.c_str(), p.length());
    }
    DeleteFile(rfn.c_str());
    return 0;
}
#endif

void
reset_msg_backup()
{
    DeleteFile(newfn("dbu.sql").c_str());
    DeleteFile(newfn("bu.sql").c_str());

}

int
create_msg_backup()
{
    DwString actual_bufn = newfn("bu.sql");
    if(Trim_backups)
    {
        DeleteFile(newfn("dbu.sql").c_str());
    }
    init_backup_sql();
    if(!Db)
        return 0;

    sql_sync_off();
    try
    {
        if(get_state() == 0)
        {
            // create a new full backup

            create_full_backup();
#ifdef DWYCO_FIELD_DEBUG
            struct stat s;
            if(stat(actual_bufn.c_str(), &s) == 0)
            {
                TRACK_MAX(main_backup_size, s.st_size);
            }
            else
            {
                TRACK_ADD(main_backup_failed, 1);
            }
#endif
        }
    }
    catch(...)
    {
        exit_backup_sql();
        DeleteFile(actual_bufn.c_str());
        return 0;
    }
    if(Enable_backups)
    {
        if(get_state() == 1)
        {
            // bu created, but not yet sent
            try
            {
                if(!send_in_chunks(actual_bufn))
                {
                    exit_backup_sql();
                    return 0;
                }
            }
            catch(...)
            {
                exit_backup_sql();
                DeleteFile(actual_bufn.c_str());
                return 0;
            }
        }
    }

    // if the main backup is in state 2 or 3, it has been
    // sent, or acked, in that state, just accumulate new messages
    // in the differential until some point

    // create a differential backup
    try
    {
        int newmsgs = create_incr_backup();
#ifdef DWYCO_FIELD_DEBUG
        struct stat s;
        if(stat(newfn("dbu.sql").c_str(), &s) == 0)
        {
            TRACK_MAX(diff_backup_size, s.st_size);
        }
        else
        {
            TRACK_ADD(diff_backup_failed, 1);
        }
#endif
        if(Enable_backups && (newmsgs > 0 || get_date("date_updated", "dbu") > get_date("date_sent", "dbu")))
        {
            int st = get_state("dbu");
            if(st == 1)
            {
                if(!send_in_chunks(newfn("dbu.sql"), "dbu"))
                {
                    exit_backup_sql();
                    return 0;
                }
            }
            else
            {
                time_t now = time(0);
                if(now - get_date("date_sent", "dbu") > Backup_freq)
                {
                    // if it is acked and 3 days old, resend
                    // note: for now, we'll just say if the output q is
                    // empty, then we can send, this is just to avoid
                    // cases where updates happen and too fast a rate
                    // and it start clogging things up. this doesn't
                    // take into account the backup may never make it
                    // to the server, so this should be fixed eventually.
                    if(msg_outq_empty()/*st == 3*/)
                    {
                        if(!send_in_chunks(newfn("dbu.sql"), "dbu"))
                        {
                            exit_backup_sql();
                            return 0;
                        }
                        // send in chunks reset to state 2
                    }
                }
            }
        }
        exit_backup_sql();
        return 1;
    }
    catch(...)
    {
        exit_backup_sql();

        return 0;
    }


}

static
int
make_msg_folder(vc uid, DwString* fn_out)
{
    if(uid.len() == 0)
        return 0;
    DwString s((const char *)uid);
    s += ".usr";
    s = newfn(s);
    if(fn_out)
        *fn_out = s;
    mkdir(s.c_str());
    return 1;
}

static
int
restore_msg(vc uid, vc mid)
{
    VCArglist a;
    a[0] = "select msg, attfn, att from msgs where mid = $1;";
    a[1] = mid;
    vc res = sqlite3_bulk_query(Db, &a);
    if(res.is_nil())
        return 0;
    vc msg;
    if(!deserialize(res[0][0], msg))
        return 0;
    vc attfn = res[0][1];
    vc att = res[0][2];

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
        if(access(actual_attfn.c_str(), 0) == 0)
            return 1;
        int fd = creat(actual_attfn.c_str(), 0666);
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
    return 1;

}

int
restore_msgs(const char *cfn, int msgs_only)
{
    DwString fn(cfn);

    sqlite3 *s;
    if(sqlite3_open(fn.c_str(), &s) != SQLITE_OK)
        return 0;
    Db = s;

    try
    {
        VCArglist a;
        a[0] = "select distinct(from_uid) from msgs;";
        vc res = sqlite3_bulk_query(Db, &a);
        if(res.is_nil())
            return 0;
        // we are likely to be loading something at this point,
        // so kill the index so it is rebuilt
        DeleteFile(newfn("mi.sql").c_str());
        // kill the previous message backup too since it will
        // need to be rebuilt
        reset_msg_backup();
        vc uid_list = res;
        for(int i = 0; i < uid_list.num_elems(); ++i)
        {
            vc uid = uid_list[i][0];
            make_msg_folder(uid, 0);
        }
        a.set_size(0);
        a[0] = "select from_uid, mid from msgs;";
        res = sqlite3_bulk_query(Db, &a);
        if(res.is_nil())
            return 0;
        for(int i = 0; i < res.num_elems(); ++i)
        {
            vc uid = res[i][0];
            vc mid = res[i][1];
            restore_msg(uid, mid);
        }
        if(!msgs_only)
            restore_account_info("main");
    }
    catch(...)
    {
        return 0;
    }

    return 1;
}
}



