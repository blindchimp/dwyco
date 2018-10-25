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
#include "libgen.h"
#include "dlli.h"
#include "dwyco_new_msg.h"
#include "dwycolistscoped.h"
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
#define HANDLE_MSG(m) dwyco_save_message(m)

using namespace dwyco;

// note: instead of using qt database stuff, we'll just the
// internal database stuff in cdc32 since we are statically linking
// main reason for this is that it is a bit simpler. it also adds
// dependency on vc, but that is ok since this is a one-off hack
// NOTE: need to keep in mind that some uid's maybe joined-group wise and
// take that into account so messages don't get bounced back to user.

struct rando_sql : public SimpleSql
{
    rando_sql() : SimpleSql("rando.sql") {}

    void init_schema() {
        sql_simple("create table if not exists randos(from_uid text collate nocase, "
                   "mid text collate nocase,"
                   "filename text, time integer, hash text collate nocase, loc_lat, loc_long)");
        sql_simple("create index if not exists uid_idx on randos(from_uid)");
        sql_simple("create table if not exists sent_to(to_uid text collate nocase, "
                   "mid text collate nocase,"
                   "filename text, time integer, hash text collate nocase, loc_lat, loc_long)");
        sql_simple("create index if not exists uid_idx2 on sent_to(to_uid)");
    }

};

rando_sql *D;
const char *Botfiles;

static
void
DWYCOCALLCONV
dwyco_db_login_result(const char *str, int what)
{

    if(what != 0)
        dwyco_switch_to_chat_server(0);
    else
        exit(1);
}

quint32 Sent_age;
QByteArray My_uid;
QMap<QByteArray, QString> Who_got_what;

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
    int compid = dwyco_make_forward_zap_composition(0, 0, mid.constData(), 1);
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
    D->start_transaction();
    D->sql_simple("create temp table c1 as select count(*), from_uid from randos group by from_uid");
    D->sql_simple("create temp table c2 as select count(*), to_uid from sent_to group by to_uid");
    vc res = D->sql_simple("select from_uid from c1,c2 where c1.from_uid = c2.to_uid and c1.'count(*)' > c2.'count(*)'");
    vc res2 = D->sql_simple("select from_uid from c1 where from_uid not in (select to_uid from c2);");
    D->sql_simple("drop table c1");
    D->sql_simple("drop table c2");
    D->commit_transaction();
    for(int i = 0; i < res2.num_elems(); ++i)
        res.append(res2[i]);
    return res;
}

int
do_rando(vc huid)
{
    vc res;
    try
    {
        D->start_transaction();
        res = D->sql_simple("select filename, hash from randos where from_uid != $1 and "
                            "not exists(select 1 from sent_to where randos.hash = hash) order by time desc limit 1",
                            huid);
        vc fn;
        vc hash;
        if(res.num_elems() == 0)
        {
            // no brand new content, so double-up on some old randos.
            // candidate is the newest rando that has been resent
            // the least number of times.
            res = D->sql_simple("select sent_to.filename, sent_to.hash from sent_to,randos "
                                "where sent_to.hash = randos.hash and from_uid != $1 "
                                "and to_uid != $1 "
                                "group by sent_to.hash having max(randos.time) "
                                "order by count(*) asc, randos.time desc limit 10",
                                huid);
            // pick a random one from the first 10
            if(res.num_elems() > 0)
            {
                int i = rand() % res.num_elems();
                fn = res[i][0];
                hash = res[i][1];
            }
        }
        else
        {
            fn = res[0][0];
            hash = res[0][1];
        }
        if(!fn.is_nil())
        {
            QByteArray bf(Botfiles);
            bf += "/";
            bf += (const char *)fn;
            int compid = dwyco_make_zap_composition_raw(bf.constData());
            if(compid == 0)
            {
                D->sql_simple("delete from randos where filename = $1",
                              fn);
                D->commit_transaction();
                // don't handle the message, we might be able to
                // answer it next time, as the most likely cause
                // for the problem is the file went missing we
                // are trying to send
                return 0;
            }
            QByteArray uid = QByteArray::fromHex((const char *)huid);
            if(!dwyco_zap_send5(compid, uid.constData(), uid.length(), "Here is a rando", 15, 0, 1, 0, 0))
            {
                dwyco_delete_zap_composition(compid);
                throw -1;
            }
            D->sql_simple("insert into sent_to (to_uid, filename, mid, hash) select $1, filename, mid, hash from randos where hash = $2",
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


int
main(int argc, char *argv[])
{
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


    dwyco_set_login_result_callback(dwyco_db_login_result);
    dwyco_set_chat_ctx_callback(dwyco_chat_ctx_callback);
    dwyco_set_initial_invis(0);
    dwyco_init();

    dwyco_set_setting("call_acceptance/no_listen", "1");
    dwyco_inhibit_sac(1);

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

    while(1)
    {
        int spin;
        dwyco_service_channels(&spin);
        usleep(10 * 1000);
        ++i;
        if(time(0) - start >= r || access("stop", F_OK) == 0)
        {
            dwyco_power_clean_safe();
            dwyco_empty_trash();
            dwyco_exit();
            creat("stopped", 0666);
            exit(0);
        }
        if(dwyco_chat_online() == 0)
        {
            if(was_online)
                exit(0);
            else
                continue;
        }

        was_online = 1;

        QByteArray uid;
        QByteArray txt;
        int dummy;
        QByteArray mid;
        int has_att;
        int is_file = 0;

        if(dwyco_new_msg(uid, txt, dummy, mid, has_att, is_file))
        {
            txt = txt.toLower();

            if(!has_att && txt.contains("yes"))
            {
                send_reply_to(uid, "Send me a zap with a pic... I'll send you a random pic in return.");
            }

            DWYCO_UNSAVED_MSG_LIST uml;


            if(!dwyco_get_unsaved_messages(&uml, 0, 0))
            {
                dwyco_delete_unsaved_message(mid.constData());
                continue;
            }
            simple_scoped quml(uml);
            int n = quml.rows();
            if(n == 0)
            {
//                send_reply_to(uid, "Thanks, I've already got a message from you and I'm working on sending it to a random Dwyconian, I'll let you know when you can send me another.");
//                HANDLE_MSG(mid.constData());
                continue;
            }
            if(!has_att)
            {
                send_reply_to(uid, "Your message needs more than just text... send a pic or video!");
                HANDLE_MSG(mid.constData());
                continue;
            }
            int compid = dwyco_make_forward_zap_composition(0, 0, mid.constData(), 1);
            int tmp = dwyco_flim(compid);
            dwyco_delete_zap_composition(compid);
            if(tmp)
            {
                send_reply_to(uid, "You sent me a message that is flagged as \"no forward\". The message was deleted, try again.");
                HANDLE_MSG(mid.constData());

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
            dwyco_copy_out_file_zap(0, 0, mid.constData(), actual_b.constData());
            QFile fn(actual_b);
            fn.open(QFile::ReadOnly);
            QCryptographicHash h(QCryptographicHash::Sha1);
            h.addData(&fn);
            QByteArray hash = h.result().toHex();
            QByteArray huid = uid.toHex();

            vc res = D->sql_simple("select 1 from randos where hash = $1", vc(hash.constData()));
            if(res.num_elems() != 0)
            {
                send_reply_to(uid, "Sorry, already seen that one, please send some new content for me to use.");
                HANDLE_MSG(mid);
                continue;
            }

            D->sql_simple("insert into randos (from_uid, filename, time, hash) values($1, $2, strftime('%s', 'now'), $3)",
                          huid.constData(),
                          vc(b.constData()),
                          vc(hash.constData()));

            send_reply_to(uid, "Thanks, got your message, I'll send you a random message soon!");

           HANDLE_MSG(mid);

        }

        vc due = uid_due_randos();
        for(int i = 0; i < due.num_elems(); ++i)
        {
            do_rando(due[i][0]);
        }

    }

}
