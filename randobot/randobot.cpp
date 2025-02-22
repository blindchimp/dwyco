/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "dlli.h"
#include "dwyco_new_msg.h"
#include "simplesql.h"
#include <QCryptographicHash>
#include <QList>
#include <QByteArray>
#include <QMap>
#include <QSet>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QVariant>
#define HANDLE_MSG(m) processed_msg(m)

using namespace dwyco;

static const char *Botfiles;
static SimpleSql *Iplog;
static int Throttle = 3;
static int Freebie_interval = 300;

// note: instead of using qt database stuff, we'll just the
// internal database stuff in cdc32 since we are statically linking
// main reason for this is that it is a bit simpler. it also adds
// dependency on vc, but that is ok since this is a one-off hack
// NOTE: need to keep in mind that some uid's maybe joined-group wise and
// take that into account so messages don't get bounced back to user.

struct rando_sql : public SimpleSql
{
    rando_sql() : SimpleSql("rando.sql") {}

    void init_schema(const DwString& schema_name) {
        sql_simple("create table if not exists randos(from_uid text collate nocase, "
                   "mid text collate nocase,"
                   "filename text, time integer, hash text collate nocase unique, loc_lat, loc_long)");
        sql_simple("create index if not exists uid_idx on randos(from_uid)");
        sql_simple("create table if not exists sent_to(to_uid text collate nocase, "
                   "mid text collate nocase,"
                   "filename text, time integer, hash text collate nocase, loc_lat, loc_long)");
        sql_simple("create index if not exists uid_idx2 on sent_to(to_uid)");
        sql_simple("create index if not exists sent_to_hash_idx on sent_to(hash)");
        sql_simple("create table if not exists seeder(uid text collate nocase unique on conflict ignore)");
        sql_simple("create table if not exists reviewers(uid text collate nocase unique on conflict ignore)");
        sql_simple("insert into reviewers (uid) values ('5a098f3df49015331d74')");
        sql_simple("create table if not exists grace(uid text collate nocase unique on conflict ignore, time integer, sent integer default 0)");
        sql_simple("create table if not exists recv_loc(from_uid text collate nocase, mid text collate nocase, hash text collate nocase unique on conflict ignore, time integer, geo text)");
        sql_simple("create table if not exists recv_loc2(from_uid text collate nocase, mid text collate nocase, hash text collate nocase unique on conflict ignore, time integer, lat text, lon text)");
        sql_simple("create table if not exists sent_geo(to_uid text collate nocase, mid text collate nocase, hash text collate nocase, time integer, geo text)");
        sql_simple("create table if not exists sent_geo2(to_uid text collate nocase, mid text collate nocase, hash text collate nocase, time integer, lat text, lon text)");

        sql_simple("create table if not exists logins(uid text collate nocase unique on conflict replace, wants_freebies integer, time integer)");
        sql_simple("create table if not exists sent_freebie(to_uid text collate nocase, mid text collate nocase, filename text, time integer, hash text collate nocase)");
        sql_simple("create index if not exists sf_idx_hash on sent_freebie(hash)");
        sql_simple("create index if not exists sf_idx_to_uid on sent_freebie(to_uid)");

        sql_simple("create table if not exists freebie_interval(lock integer not null default 0, secs integer not null, primary key(lock), check(lock = 0))");
        sql_simple("insert or replace into freebie_interval(lock, secs) values(0, ?1)", Freebie_interval);

        // these just allow us to avoid a lot of unneeded scanning
        sql_simple("create table if not exists state_change(flag integer not null default 0, lock integer not null default 0, primary key(lock), check(lock = 0))");
        sql_simple("insert or ignore into state_change(flag, lock) values (0, 0)");

#define table_trigger(table) \
        sql_simple("create trigger if not exists " #table "_trigger1 after insert on " #table \
                   " begin update state_change set flag = 1; end"); \
        sql_simple("create trigger if not exists " #table "_trigger2 after update on " #table \
                   " begin update state_change set flag = 1; end");

        table_trigger(logins)
                table_trigger(sent_freebie)
                table_trigger(recv_loc)
                table_trigger(recv_loc2)
                table_trigger(sent_geo)
                table_trigger(sent_geo2)
                table_trigger(randos)
                table_trigger(sent_to)
                table_trigger(grace)

#undef table_trigger
    }

};

static rando_sql *D;

static
void
DWYCOCALLCONV
dwyco_db_login_result(const char *str, int what)
{

    if(what != 0)
    {}//dwyco_switch_to_chat_server(0);
    else
        exit(1);
}

quint32 Sent_age;
QByteArray My_uid;
QStringList Ann_names;

QByteArray
random_fn()
{
    char *s;
    dwyco_random_string2(&s, 10);
    QByteArray ret(s, 10);
    ret = ret.toHex();
    dwyco_free_array(s);
    return ret;

}

void
forward_msg(const QByteArray& mid, const QByteArray& uid)
{
    int compid = dwyco_make_forward_zap_composition2(mid.constData(), 1);
    if(compid == 0)
        return;

    dwyco_zap_send4(compid, uid.constData(), uid.length(), "Here is a rando", 15, 0, 0, 0);
    //dwyco_delete_zap_composition(compid);

}

QByteArray
time_till()
{
    int time_till_next = (6 * 3600) - (time(0) - Sent_age);
    if(time_till_next < 0)
        time_till_next = 0;
    int hours_till = time_till_next / 3600;
    int min_till = time_till_next - (hours_till * 3600);
    min_till /= 60;
    QString timestr = QString("Next pic in %1 hours %2 minutes").arg(hours_till).arg(min_till);
    QByteArray ts = timestr.toLatin1();
    return ts;
}

int
send_reply_to(const QByteArray& uid, QString msg)
{
    int compid = dwyco_make_zap_composition(0);
    if(compid == 0)
        return 0;
    QByteArray lm = msg.toLatin1();
    if(!dwyco_zap_send5(compid, uid.constData(), uid.length(),
                    lm.constData(), lm.length(), 0, 0, 0, 0))
        return 0;
    return 1;

}

void
send_file_to(const QByteArray& uid, const char *msg, QByteArray fn)
{
    int compid = dwyco_make_file_zap_composition(fn.constData(), fn.length());
    if(compid == 0)
        return;
    dwyco_zap_send5(compid, uid.constData(), uid.length(),
                    msg, strlen(msg), 0, 0, 0, 0);

}


template<class T>
void
save_it(const T& d, const char *filename)
{
    QFile f(filename);
    f.open(QIODevice::WriteOnly);
    QDataStream out(&f);
    out << d;
    f.close();
}

template<class T>
int
load_it(T& out, const char *filename)
{
    QFile f(filename);
    if(!f.exists())
        return 0;
    f.open(QIODevice::ReadOnly);
    QDataStream in(&f);
    in >> out;
    f.close();
    if(in.status() == QDataStream::Ok)
    {
        return 1;
    }
    f.remove();
    return 0;
}

static
int
send_pic(QByteArray buid)
{
    // select random file
    int n = Ann_names.count();
    int i = rand() % n;
    QString fn = Ann_names[i];
    QByteArray ts = time_till();

    send_file_to(buid, QByteArray("pic of the day, ") + ts, fn.toLatin1());
    return 1;
}

void
DWYCOCALLCONV
dwyco_chat_ctx_callback(int cmd, int id,
    const char *uid, int len_uid,
    const char *name, int len_name,
    int type, const char *val, int len_val,
    int qid,
    int extra_arg)
{
    QByteArray dname;
    if(name)
        dname = QByteArray(name, len_name);

    QByteArray buid;
    if(uid)
    {
        buid = QByteArray(uid, len_uid);
    }
    if(buid == My_uid)
        return;

    switch(cmd)
    {
    case DWYCO_CHAT_CTX_ADD_USER:


        break;
    }
}

vc
uid_due_randos()
{
    // TODO: put some prioritization in here so that users that haven't received anything
    // lately are put on the top of the q
    vc res;
    try
    {
    D->start_transaction();
    // c1 is count of received randos from each non-seeder uid
    D->sql_simple("create temp table c1 as select count(*) as cr, from_uid from randos where from_uid not in (select uid from seeder) group by from_uid");

    // delete from c1 if a uid has been sent 3 or more randos in the last hour
    // NOTE: comment out following line for testing, otherwise you'll get throttled
    D->sql_simple("delete from c1 where from_uid in (select to_uid from sent_to where strftime('%s', 'now') - time < 3600 group by to_uid having(count(*) >= ?1));", Throttle);

    // c2 is the count of messages sent to uid
    D->sql_simple("create temp table c2 as select count(*) as cs, to_uid from sent_to group by to_uid");

    D->sql_simple("create temp table res(uid text collate nocase unique on conflict ignore, count integer)");
    // list of all uids that have sent in more than they have received
    D->sql_simple("insert into res select from_uid,c1.cr - c2.cs from c1,c2 where c1.from_uid = c2.to_uid and c1.cr > c2.cs");

    // list of all uids that have sent something in, but never received anything
    D->sql_simple("insert into res select from_uid,c1.cr from c1 where from_uid not in (select to_uid from c2);");
    //D->sql_simple("drop table c1");
    //D->sql_simple("drop table c2");

    // users getting a grace-period
    // we aren't too worried if something happens later that causes the message
    // to fail... so just terminate the grace-period here.
    D->sql_simple("insert into res select uid, 0 from grace where sent = 0");
    D->sql_simple("update grace set sent = 1 where sent = 0");
    res = D->sql_simple("select * from res");
    //D->sql_simple("drop table res");
    D->rollback_transaction();
    }
    catch(...)
    {
        D->rollback_transaction();
        res = vc(VC_VECTOR);

    }
    return res;
}

int
do_rando(vc huid)
{
    vc res;
    try
    {
        D->start_transaction();
        // this selects a pic that has never been sent to anyone, namely, the freshest
        // stuff we have on hand.
        res = D->sql_simple("select filename, hash, mid, from_uid from randos where from_uid != ?1 and "
                            "not exists(select 1 from sent_to where randos.hash = hash union select 1 from sent_freebie where randos.hash = hash) order by time desc limit 1",
                            huid);
        vc fn;
        vc hash;
        vc mid;
        vc hcreator_uid;
        if(res.num_elems() == 0)
        {
            // no brand new content, so double-up on some old randos.
            // candidate is the newest rando that has been resent
            // the least number of times.

            // previous way of doing this was clever, but hard to understand
            D->sql_simple("create temp table foo as select sent_to.filename as fn, sent_to.hash as hash, randos.mid as mid, randos.from_uid as from_uid, randos.time as time "
                          "from sent_to,randos where sent_to.hash = randos.hash and from_uid != ?1 ", huid);
            D->sql_simple("insert into foo select sent_freebie.filename, sent_freebie.hash, randos.mid, randos.from_uid,randos.time "
                          "from sent_freebie,randos where sent_freebie.hash = randos.hash and from_uid != ?1", huid);
            D->sql_simple("delete from foo where hash in (select hash from sent_to where to_uid = ?1)", huid);
            D->sql_simple("delete from foo where hash in (select hash from sent_freebie where to_uid = ?1)", huid);


            res = D->sql_simple("select * from foo group by hash order by count(*) asc, time desc limit 10");
            D->sql_simple("drop table foo");
            // pick a random one from the first 10
            if(res.num_elems() > 0)
            {
                int i = rand() % res.num_elems();
                fn = res[i][0];
                hash = res[i][1];
                mid = res[i][2];
                hcreator_uid = res[i][3];
            }
        }
        else
        {
            fn = res[0][0];
            hash = res[0][1];
            mid = res[0][2];
            hcreator_uid = res[0][3];
        }
        if(!fn.is_nil())
        {
            QByteArray bf(Botfiles);
            bf += "/";
            bf += (const char *)fn;
            // see if there is an extension we can add to get a file type
            DwString cmd("grep `file -b --mime-type %1` mime2extension.csv | sed 's/.*,//' >mumble");
            cmd.arg(bf.constData());
            const char *poss_ext = 0;
            char a[500];
            memset(a, 0, sizeof(a));
            if(system(cmd.c_str()) == 0)
            {
                FILE *f = fopen("mumble", "rb");
                if(f)
                {
                    if(fgets(a, sizeof(a), f) != 0)
                    {
                        int len = strlen(a);
                        if(a[len - 1] == '\n')
                            a[len - 1] = 0;
                        if(a[0] != '.')
                        {
                            DwString foo(a);
                            foo.insert(0, ".");
                            strcpy(a, foo.c_str());
                        }
                        poss_ext = a;
                    }
                    fclose(f);
                }
            }

            int compid = dwyco_make_zap_composition_raw(bf.constData(), poss_ext);
            if(compid == 0)
            {
                D->sql_simple("delete from randos where filename = ?1",
                              fn);
                D->commit_transaction();
                // don't handle the message, we might be able to
                // answer it next time, as the most likely cause
                // for the problem is the file went missing we
                // are trying to send
                return 0;
            }
            QByteArray uid = QByteArray::fromHex((const char *)huid);
            QByteArray str_to_send("Here is a rando");
            if(Iplog)
            {
                vc res;
                try {
                    // try to get a location estimate for the recipient and send that back to the
                    // to the creator of the msg. note: this gets an estimated location based on recipients
                    // latest login, not on their location when they generated their content.

                    res = Iplog->sql_simple("select geo, max(time) from iplog where id = ?1", huid);
                    QByteArray tagstr("{\"hash\" : \"");
                    tagstr += (const char *)hash;
                    tagstr += "\"";
                    if(res.num_elems() == 1 && res[0][0].type() == VC_STRING && res[0][0].len() > 0)
                    {
                        tagstr += ", \"loc\" : \"";
                        tagstr += (const char *)res[0][0];
                        tagstr += "\"";
                    }

                    // send lat and lon as well to facilitate creating a map image
                    res = Iplog->sql_simple("select lat, lon, max(time) from iplog2 where id = ?1", huid);
                    if(res.num_elems() == 1 && res[0][0].type() == VC_STRING && res[0][0].len() > 0)
                    {
                        tagstr += ", \"lat\" : \"";
                        tagstr += (const char *)res[0][0];
                        tagstr += "\"";
                        tagstr += ", \"lon\" : \"";
                        tagstr += (const char *)res[0][1];
                        tagstr += "\"";
                    }
                    tagstr += "}";
                    int ccid = dwyco_make_zap_composition(0);
                    if(ccid != 0)
                    {
                        QByteArray creator_uid = QByteArray::fromHex((const char *)hcreator_uid);
                        if(!dwyco_zap_send6(ccid, creator_uid.constData(), creator_uid.length(), tagstr.constData(), tagstr.length(), 1, 1, 0, 0, 0))
                        {
                            dwyco_delete_zap_composition(ccid);
                        }
                    }
                } catch (...) {
                }
            }
            // see if we can get some location estimate for the message we received
            {
                str_to_send = "{";
                vc res = D->sql_simple("select geo from recv_loc where hash = ?1 limit 1", hash);
                if(res.num_elems() == 1 && res[0][0].type() == VC_STRING && res[0][0].len() > 0)
                {
                    // note: mid not included because the mid is being generated in the new message
                    // being sent to recipient
                    str_to_send += "\"loc\":\"";
                    str_to_send += (const char *)res[0][0];
                    str_to_send += "\"";

                    D->sql_simple("insert into sent_geo(to_uid, mid, hash, time, geo) values(?1, ?2, ?3, strftime('%s', 'now'), ?4)",
                                  huid,
                                  mid,
                                  hash,
                                  res[0][0]);
                }
                res = D->sql_simple("select lat, lon from recv_loc2 where hash = ?1 limit 1", hash);
                if(res.num_elems() == 1 && res[0][0].type() == VC_STRING && res[0][0].len() > 0)
                {
                    // note: mid not included because the mid is being generated in the new message
                    // being sent to recipient
                    str_to_send += ",\"lat\":\"";
                    str_to_send += (const char *)res[0][0];
                    str_to_send += "\"";

                    str_to_send += ",\"lon\":\"";
                    str_to_send += (const char *)res[0][1];
                    str_to_send += "\"";

                    D->sql_simple("insert into sent_geo2(to_uid, mid, hash, time, lat, lon) values(?1, ?2, ?3, strftime('%s', 'now'), ?4, ?5)",
                                  huid,
                                  mid,
                                  hash,
                                  res[0][0],
                            res[0][1]);
                }
                str_to_send += "}";

            }

            if(!dwyco_zap_send5(compid, uid.constData(), uid.length(), str_to_send.constData(), str_to_send.length(), 0, 1, 0, 0))
            {
                dwyco_delete_zap_composition(compid);
                throw -1;
            }
            D->sql_simple("insert into sent_to (to_uid, filename, mid, hash, time) select ?1, filename, mid, hash, strftime('%s', 'now') from randos where hash = ?2",
                          huid, hash);
        }
        D->commit_transaction();
    }
    catch(...)
    {
        D->rollback_transaction();
        return 0;
    }
    return 1;
}


// figure out which uid's need a freebie pic
vc
uid_due_freebie()
{
    vc res;
    try {
        D->start_transaction();
        vc now(time(0));
        // someone who never received one is eligible
        D->sql_simple("create temp table foo as select uid from logins where wants_freebies = 1 and not exists(select 1 from sent_freebie where logins.uid = to_uid)");
        // time of last sent freebie for each uid
        D->sql_simple("create temp table bar as select to_uid, max(time) as time from sent_freebie group by to_uid");
        D->sql_simple("insert into foo select uid from logins,bar where wants_freebies = 1 and logins.uid = to_uid and ?1 - logins.time <= 2 * (select secs from freebie_interval)"
                      " and ?1 - bar.time > (select secs from freebie_interval)", now);
        res = D->sql_simple("select distinct(uid) from foo");
        D->rollback_transaction();
    } catch (...) {
        D->rollback_transaction();
        res = vc(VC_VECTOR);
    }
    return res;
}
// send a freebie pic to huid
// note: this is a different "channel" from the rando thing, so they
// don't interact as far as throttling and other counts.
int
do_freebie(vc huid)
{
    vc res;
    try
    {
        D->start_transaction();
        // note: this could limit the number of freebies sent to a particular user
        // if no new content is being put into the system. but since we are probably
        // going to 1 or 2 freebies a day, it should 6-12 months if that happens. we'll
        // figure it out later...
        D->sql_simple("create temp table bar as select filename, hash, mid, from_uid from randos where from_uid != ?1 order by time desc limit 300", huid);
        D->sql_simple("delete from bar where hash in (select hash from sent_to where to_uid = ?1)", huid);
        D->sql_simple("delete from bar where hash in (select hash from sent_freebie where to_uid = ?1)", huid);
        res = D->sql_simple("select * from bar");
        D->sql_simple("drop table bar");
        vc fn;
        vc hash;
        vc mid;
        vc hcreator_uid;

        if(res.num_elems() > 0)
        {
            int i = rand() % res.num_elems();
            fn = res[i][0];
            hash = res[i][1];
            mid = res[i][2];
            hcreator_uid = res[i][3];
        }
        else
        {
            // no results, nothing to send them, maybe we could resend a random one?
            // note: the timing thing in uid_due_freebie is assuming we manage to
            // send something and update the sent_freebie table. if we don't do that,
            // due_freebies will show that uid needing a freebie for a long time
            // before it times out. this will soak some cpu, but is otherwise
            // harmless. sometime, we may want to consider just putting in a dummy
            // record so it doesn't keep coming in here until there is some
            // material to send.
            D->commit_transaction();
            return 0;
        }
        if(!fn.is_nil())
        {
            QByteArray bf(Botfiles);
            bf += "/";
            bf += (const char *)fn;
            // see if there is an extension we can add to get a file type
            DwString cmd("grep `file -b --mime-type %1` mime2extension.csv | sed 's/.*,//' >mumble");
            cmd.arg(bf.constData());
            const char *poss_ext = 0;
            char a[500];
            memset(a, 0, sizeof(a));
            if(system(cmd.c_str()) == 0)
            {
                FILE *f = fopen("mumble", "rb");
                if(f)
                {
                    if(fgets(a, sizeof(a), f) != 0)
                    {
                        int len = strlen(a);
                        if(a[len - 1] == '\n')
                            a[len - 1] = 0;
                        if(a[0] != '.')
                        {
                            DwString foo(a);
                            foo.insert(0, ".");
                            strcpy(a, foo.c_str());
                        }
                        poss_ext = a;
                    }
                    fclose(f);
                }
            }

            int compid = dwyco_make_zap_composition_raw(bf.constData(), poss_ext);
            if(compid == 0)
            {
                D->sql_simple("delete from randos where filename = ?1",
                              fn);
                D->commit_transaction();
                // don't handle the message, we might be able to
                // answer it next time, as the most likely cause
                // for the problem is the file went missing we
                // are trying to send
                return 0;
            }
            QByteArray uid = QByteArray::fromHex((const char *)huid);
            QByteArray str_to_send("Here is a freebie");
            if(Iplog)
            {
                vc res;
                try {
                    // try to get a location estimate for the recipient and send that back to the
                    // to the creator of the msg. note: this gets an estimated location based on recipients
                    // latest login, not on their location when they generated their content.

                    res = Iplog->sql_simple("select geo, max(time) from iplog where id = ?1", huid);
                    QByteArray tagstr("{\"hash\" : \"");
                    tagstr += (const char *)hash;
                    tagstr += "\"";
                    if(res.num_elems() == 1 && res[0][0].type() == VC_STRING && res[0][0].len() > 0)
                    {
                        tagstr += ", \"loc\" : \"";
                        tagstr += (const char *)res[0][0];
                        tagstr += "\"";
                    }

                    // send lat and lon as well to facilitate creating a map image
                    res = Iplog->sql_simple("select lat, lon, max(time) from iplog2 where id = ?1", huid);
                    if(res.num_elems() == 1 && res[0][0].type() == VC_STRING && res[0][0].len() > 0)
                    {
                        tagstr += ", \"lat\" : \"";
                        tagstr += (const char *)res[0][0];
                        tagstr += "\"";
                        tagstr += ", \"lon\" : \"";
                        tagstr += (const char *)res[0][1];
                        tagstr += "\"";
                    }
                    tagstr += "}";
                    int ccid = dwyco_make_zap_composition(0);
                    if(ccid != 0)
                    {
                        QByteArray creator_uid = QByteArray::fromHex((const char *)hcreator_uid);
                        if(!dwyco_zap_send6(ccid, creator_uid.constData(), creator_uid.length(), tagstr.constData(), tagstr.length(), 1, 1, 0, 0, 0))
                        {
                            dwyco_delete_zap_composition(ccid);
                        }
                    }
                } catch (...) {
                }
            }
            // see if we can get some location estimate for the message we received
            {
                str_to_send = "{";
                vc res = D->sql_simple("select geo from recv_loc where hash = ?1 limit 1", hash);
                if(res.num_elems() == 1 && res[0][0].type() == VC_STRING && res[0][0].len() > 0)
                {
                    // note: mid not included because the mid is being generated in the new message
                    // being sent to recipient
                    str_to_send += "\"loc\":\"";
                    str_to_send += (const char *)res[0][0];
                    str_to_send += "\"";

                    D->sql_simple("insert into sent_geo(to_uid, mid, hash, time, geo) values(?1, ?2, ?3, strftime('%s', 'now'), ?4)",
                                  huid,
                                  mid,
                                  hash,
                                  res[0][0]);
                }
                res = D->sql_simple("select lat, lon from recv_loc2 where hash = ?1 limit 1", hash);
                if(res.num_elems() == 1 && res[0][0].type() == VC_STRING && res[0][0].len() > 0)
                {
                    // note: mid not included because the mid is being generated in the new message
                    // being sent to recipient
                    str_to_send += ",\"lat\":\"";
                    str_to_send += (const char *)res[0][0];
                    str_to_send += "\"";

                    str_to_send += ",\"lon\":\"";
                    str_to_send += (const char *)res[0][1];
                    str_to_send += "\"";

                    D->sql_simple("insert into sent_geo2(to_uid, mid, hash, time, lat, lon) values(?1, ?2, ?3, strftime('%s', 'now'), ?4, ?5)",
                                  huid,
                                  mid,
                                  hash,
                                  res[0][0],
                            res[0][1]);
                }
                str_to_send += "}";

            }

            if(!dwyco_zap_send5(compid, uid.constData(), uid.length(), str_to_send.constData(), str_to_send.length(), 0, 1, 0, 0))
            {
                dwyco_delete_zap_composition(compid);
                throw -1;
            }
            D->sql_simple("insert into sent_freebie (to_uid, filename, mid, hash, time) select ?1, filename, mid, hash, strftime('%s', 'now') from randos where hash = ?2",
                          huid, hash);
        }
        D->commit_transaction();
    }
    catch(...)
    {
        D->rollback_transaction();
        return 0;
    }
    return 1;
}


QByteArray
fnhash(const char *fn)
{
    QByteArray afn(Botfiles);
    afn += "/";
    afn += fn;


    int fd = open(afn.constData(), O_RDONLY);
    if(fd == -1)
        return "";
    char buf[4096];
    int n = read(fd, buf, sizeof(buf));
    if(n == -1)
    {
        close(fd);
        return "";
    }
    close(fd);

    QCryptographicHash h(QCryptographicHash::Sha1);
    h.addData(buf, n);
    return h.result().toHex();
}

void
update_hashes()
{
    try {
        D->start_transaction();

        vc res = D->sql_simple("select filename, hash from randos");
        for(int i = 0; i < res.num_elems(); ++i)
        {
            vc fn = res[i][0];
            vc hash = res[i][1];

            QByteArray h = fnhash(fn);
            if(h.length() == 0)
                continue;
            if(h != (const char *)hash)
            {
                D->sql_simple("update randos set hash = ?2 where hash = ?1", hash, h.constData());
                D->sql_simple("update sent_to set hash = ?2 where hash = ?1", hash, h.constData());
                D->sql_simple("update recv_loc set hash = ?2 where hash = ?1", hash, h.constData());
                D->sql_simple("update sent_geo set hash = ?2 where hash = ?1", hash, h.constData());
            }
        }

        D->commit_transaction();
    }
    catch(...)
    {
        D->rollback_transaction();
    }

}

// invoke with
// randobot <name> <desc> <dir-to-save-pics> <ip-log-db> <pics-per-hour-throttle> <free-pic-interval-secs>
int
main(int argc, char *argv[])
{
    char *owd = getenv("OWD");
    if(owd)
    {
        printf("OWD %s\n", owd);
        if(chdir(owd) == -1)
            exit(1);
    }
    if(access("stop", F_OK) == 0)
        exit(0);
    if(argc < 4)
        exit(1);
    signal(SIGPIPE, SIG_IGN);
	alarm(3600);
    int fd = open("/dev/urandom", O_RDONLY);
    unsigned int foo;
    read(fd, (void *)&foo, sizeof(foo));
    close(fd);

    srand(foo);

    const char *name = argv[1];
    const char *desc = argv[2];
    Botfiles = argv[3];
    int Reviewer_only = 0;
    if(argc >= 5)
    {
        Reviewer_only = 1;
        const char *iplog = "/home/dwight/local/db/iplog.sqlite3";
        if(strcmp(argv[4], "-") != 0)
            iplog = argv[4];
        Iplog = new SimpleSql(iplog);
        if(!Iplog->init())
        {
            delete Iplog;
            Iplog = 0;
        }
    }

    if(argc >= 6)
    {
        Throttle = atoi(argv[5]);
    }

    if(argc >= 7)
    {
        Freebie_interval = atoi(argv[6]);
    }


    dwyco_set_login_result_callback(dwyco_db_login_result);
    dwyco_set_chat_ctx_callback(dwyco_chat_ctx_callback);
    dwyco_set_initial_invis(0);
    dwyco_init();

    dwyco_set_setting("net/listen", "0");
    //dwyco_inhibit_sac(1);

    if(dwyco_get_create_new_account())
    {
        dwyco_create_bootstrap_profile(name, strlen(name), desc, strlen(desc), "", 0, "", 0);

    }


    dwyco_set_local_auth(1);
    dwyco_finish_startup();
	int i = 0;
	int r = 20 * 60 + (rand() % 30) * 60;
    time_t start = time(0);

    const char *my_uid;
    int len_uid;
    dwyco_get_my_uid(&my_uid, &len_uid);
    My_uid = QByteArray(my_uid, len_uid);

    int was_online = 0;
    unlink("stopped");

    D = new rando_sql;
    if(!D->init())
        exit(1);
    D->set_cache_size(10000);
    if(Reviewer_only)
    {
        update_hashes();
    }
    time_t last_db_check = time(0);

    while(1)
    {
        int spin = 0;
        dwyco_service_channels(&spin);
        if(spin)
            usleep(10 * 1000);
        else
            usleep(100 * 1000);
        ++i;
        if(time(0) - start >= r || access("stop", F_OK) == 0)
        {
            dwyco_power_clean_safe();
            dwyco_empty_trash();
            dwyco_exit();
            creat("stopped", 0666);
            exit(0);
        }
//        if(dwyco_chat_online() == 0)
//        {
//            if(was_online)
//                exit(0);
//            else
//                continue;
//        }

//        was_online = 1;

        if(dwyco_get_rescan_messages())
        {
            dwyco_set_rescan_messages(0);
            process_remote_msgs();
        }

        QByteArray uid;
        QByteArray txt;
        int dummy;
        QByteArray mid;
        int has_att;
        int is_file = 0;
        QByteArray creator_uid;

        if(dwyco_new_msg2(uid, txt, dummy, mid, has_att, is_file, creator_uid))
        {
            txt = txt.toLower();
            QByteArray huid = uid.toHex();

            if(Reviewer_only)
            {
                 vc res = D->sql_simple("select 1 from reviewers where uid = ?1", huid.constData());
                 if(res.num_elems() == 0)
                 {
                     if(!has_att && txt.contains("first"))
                     {
                         // this is a hack to allow a user to get a "grace period" when initially
                         // running the app. since we manually review stuff coming in, there can be
                         // a delay if there are no reviewers awake to approve someone's initial message.
                         // so it looks like the app is doing nothing at first.
                         //
                         // this makes it so we don't need to review their
                         // initial picture, they can just get one if this is the first pic they have sent.
                         // there are all sorts of problems with this, but for now livable, and easy enough to
                         // turn off if there is a problem
                         D->sql_simple("insert into grace(uid, time) values(?1, strftime('%s', 'now'))", huid.constData());
                         HANDLE_MSG(mid);
                         continue;
                     }
                     processed_msg(mid);
                     dwyco_delete_saved_message(uid.constData(), uid.length(), mid.constData());
                     continue;
                 }
            }
            if(!has_att && txt.contains("yes"))
            {
                send_reply_to(uid, "Send me a zap with a pic... I'll send you a random pic in return. "
                                   "NOTE! I will forward the pic you send me anonymously to another "
                                   "random Dwyconian.");
                HANDLE_MSG(mid);
                continue;
            }

            if(!has_att && txt.contains("noseeder"))
            {
                send_reply_to(uid, "Ok, stop seeding.");
                D->sql_simple("delete from seeder where uid = ?1", huid.constData());
                HANDLE_MSG(mid);
                continue;
            }
            if(!has_att && txt.contains("seeder"))
            {
                send_reply_to(uid, "Ok, I won't send you any messages.");
                D->sql_simple("insert into seeder(uid) values(?1)", huid.constData());
                HANDLE_MSG(mid);
                continue;
            }


            if(!has_att)
            {
                send_reply_to(uid, "Your message needs more than just text... send a pic or video!"
                                   "NOTE! I will forward the pic you send me anonymously to another "
                                   "random Dwyconian.");
                HANDLE_MSG(mid);
                continue;
            }
            int compid = dwyco_make_forward_zap_composition2(mid.constData(), 1);
            int tmp = dwyco_flim(compid);
            dwyco_delete_zap_composition(compid);
            if(tmp)
            {
                send_reply_to(uid, "You sent me a message that is flagged as \"no forward\". The message was deleted, try again.");
                HANDLE_MSG(mid);

                continue;
            }

            // save the incoming rando, copy the attachment out
            QByteArray b = random_fn();
            if(is_file)
                b += ".fle";
            else
                b += ".dyc";
            QByteArray actual_b(Botfiles);
            actual_b += "/";
            actual_b += b;
            dwyco_copy_out_file_zap2(mid.constData(), actual_b.constData());

            // mobile devices have a hard time hashing a lot of multi-MB
            // image data, so just hash the first 4k or so (which is ok since
            // we are limiting people to stuff captured from camera)
            QByteArray hash;
            {
                const char *buf;
                int len_buf = 0;

                dwyco_copy_out_file_zap_buf2(mid.constData(), &buf, &len_buf, 4096);
                QCryptographicHash h(QCryptographicHash::Sha1);
                h.addData(buf, len_buf);
                hash = h.result().toHex();
                dwyco_free_array((char *)buf);
            }

            vc res = D->sql_simple("select 1 from randos where hash = ?1", vc(hash.constData()));
            if(res.num_elems() != 0)
            {
                send_reply_to(uid, "Sorry, already seen that one, please send some new content for me to use.");
                HANDLE_MSG(mid);
                continue;
            }

            QByteArray chuid = creator_uid.toHex();
            QByteArray effective_uid = huid;
            if(Reviewer_only)
            {
                effective_uid = chuid;
            }
            D->start_transaction();
            D->sql_simple("insert into randos (from_uid, filename, time, hash, mid) values(?1, ?2, strftime('%s', 'now'), ?3, ?4)",
                          effective_uid.constData(),
                          b.constData(),
                          hash.constData(),
                          mid.constData());
            if(Iplog)
            {
                // see if we can find an estimated location for the effective uid and record that
                vc res;
                try {
                    res = Iplog->sql_simple("select geo, max(time) from iplog where id = ?1", effective_uid.constData());
                    if(res.num_elems() == 1 && res[0][0].type() == VC_STRING && res[0][0].len() > 0)
                    {
                        D->sql_simple("insert into recv_loc(from_uid, mid, hash, time, geo) values(?1, ?2, ?3, strftime('%s', 'now'), ?4)",
                                      effective_uid.constData(),
                                      mid.constData(),
                                      hash.constData(),
                                      res[0][0]);
                    }
                    res = Iplog->sql_simple("select lat, lon, max(time) from iplog2 where id = ?1", effective_uid.constData());
                    if(res.num_elems() == 1 && res[0][0].type() == VC_STRING && res[0][0].len() > 0)
                    {
                        D->sql_simple("insert into recv_loc2(from_uid, mid, hash, time, lat, lon) values(?1, ?2, ?3, strftime('%s', 'now'), ?4, ?5)",
                                      effective_uid.constData(),
                                      mid.constData(),
                                      hash.constData(),
                                      res[0][0],
                                res[0][1]);
                    }
                } catch (...) {
                    res = vcnil;

                }
            }

            // reviewers are treated as seeders, but not the other way around
            res = D->sql_simple("select 1 from seeder,reviewers where seeder.uid = ?1 or reviewers.uid = ?1", huid.constData());

            if(res.num_elems() > 0)
            {
                send_reply_to(uid, "Seeded msg");
                // when you seed, we just pretend the message was sent right back to you,
                // so it doesn't look like you need more randos sent to you.
                D->sql_simple("insert into sent_to (to_uid, filename, mid, hash) select ?1, filename, mid, hash from randos where hash = ?2",
                              huid.constData(), hash.constData());
            }
            else
                send_reply_to(uid, "Thanks, got your message, I'll send you a random message soon!");
            D->commit_transaction();

           HANDLE_MSG(mid);

        }
        {
        vc res = D->sql_simple("select flag from state_change");
        if((int)res[0][0] == 0 && time(0) - last_db_check < 60)
            continue;
        }
        last_db_check = time(0);
        vc due1 = uid_due_randos();
        for(int i = 0; i < due1.num_elems(); ++i)
        {
            do_rando(due1[i][0]);
        }

        vc due2 = uid_due_freebie();
        for(int i = 0; i < due2.num_elems(); ++i)
        {
            do_freebie(due2[i][0]);
        }

        if(due1.num_elems() == 0 && due2.num_elems() == 0)
        {
            D->sql_simple("update state_change set flag = 0");
        }


    }

}
