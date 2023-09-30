
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// this bot receives a list of email addresses from clients, performs
// a contact list lookup, and sends a reply message to the sender
// containing a list of uid's that corresponds to the input emails.
// (subject to some constraints, as explained in the queries below.)
//
// the input and output lists are in Qt serialized format.
//

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
#include "dwycolist2.h"
#include <QList>
#include <QByteArray>
#include <QMap>
#include <QSet>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTemporaryFile>
#include <QtPlugin>
#include <QCoreApplication>
//Q_IMPORT_PLUGIN(qsqlite)


static
void
DWYCOCALLCONV
dwyco_db_login_result(const char *str, int what)
{

    if(what != 0)
    {
        //dwyco_switch_to_chat_server(1);
    }
    else
        exit(1);
}

static int
dwyco_get_attr(DWYCO_LIST l, int row, const char *col, QByteArray& str_out)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_STRING)
        return 0;
    str_out = QByteArray(val, len);
    return 1;
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


void
send_reply_to(const QByteArray& uid, const char *msg)
{
    int compid = dwyco_make_zap_composition(0);
    if(compid == 0)
        return;
    dwyco_zap_send4(compid, uid.constData(), uid.length(),
                    msg, strlen(msg), 0, 0, 0);

}

void
send_file_to(const QByteArray& uid, const char *msg, QByteArray fn)
{
    int compid = dwyco_make_file_zap_composition(fn.constData(), fn.length());
    if(compid == 0)
        return;
    dwyco_zap_send4(compid, uid.constData(), uid.length(),
                    msg, strlen(msg), 0, 0, 0);

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
void
save_it(const T& d, QFile& f)
{
    QDataStream out(&f);
    out << d;
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


#define NAME "clbot2"
#define DESC "internal"

int
main(int argc, char *argv[])
{
    if(access("stop", F_OK) == 0)
        exit(0);
    unlink("stopped");
    QCoreApplication app(argc, argv);

    signal(SIGPIPE, SIG_IGN);
	alarm(3600);
    int fd = open("/dev/urandom", O_RDONLY);
    unsigned int foo;
    read(fd, (void *)&foo, sizeof(foo));
    close(fd);

    srand(foo);

    dwyco_set_login_result_callback(dwyco_db_login_result);
    //dwyco_set_chat_server_status_callback(dwyco_chat_server_status_callback);
    //dwyco_set_chat_ctx_callback(dwyco_chat_ctx_callback);

    dwyco_init();

    dwyco_set_setting("net/listen", "0");

    if(dwyco_get_create_new_account())
    {
        dwyco_create_bootstrap_profile(NAME, strlen(NAME), DESC, strlen(DESC), "", 0, "", 0);

    }

    dwyco_set_local_auth(1);
    dwyco_finish_startup();
	int i = 0;
	int r = 20 * 60 + (rand() % 30) * 60;
    time_t start = time(0);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    char *dbn = "/home/dwight/syncdev/profiles.sqlite3";
    if(argc >= 2)
        dbn = argv[1];
    if(access(dbn, R_OK|W_OK) != 0)
        exit(1);
    db.setDatabaseName(dbn);
    if(!db.open())
        exit(1);

    dbn = "/home/dwight/syncdev/iy.sqlite3";
    if(argc >= 3)
        dbn = argv[2];
    if(access(dbn, R_OK|W_OK) != 0)
        exit(1);

    QSqlQuery q2;

    q2.prepare("attach ?1 as iy;");
    q2.addBindValue(dbn);
    q2.exec();

    while(1)
    {
        int spin;
        dwyco_service_channels(&spin);
        if(spin)
            usleep(10 * 1000);
        else
            usleep(100 * 1000);
        ++i;
        if(time(0) - start >= r || access("stop", F_OK) == 0)
        {
            dwyco_power_clean_safe();
            dwyco_exit();
            creat("stopped", 0666);
            exit(0);
        }

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

        if(dwyco_new_msg(uid, txt, dummy, mid, has_att))
        {
            // if no attachment, then we just ignore it
            if(!has_att)
            {
                processed_msg(mid);
                dwyco_delete_saved_message(uid.constData(), uid.length(), mid.constData());
                continue;
            }
            DWYCO_SAVED_MSG_LIST sm;
            if(!dwyco_get_saved_message(&sm, uid.constData(), uid.length(), mid.constData()))
                continue;
            simple_scoped qsm(sm);
            QByteArray attfn;
            if(!dwyco_get_attr(qsm, 0, DWYCO_QM_BODY_ATTACHMENT, attfn))
                continue;
            if(!dwyco_copy_out_file_zap2(mid.constData(), "mumble.qds"))
                continue;
            QString huid = uid.toHex();
            QList<QString> addrs;
            if(!load_it(addrs, "mumble.qds"))
            {
                processed_msg(mid);
                dwyco_delete_saved_message(uid.constData(), uid.length(), mid.constData());
                continue;
            }
            int n = addrs.count();


            {
                QSqlQuery q;
                q.exec("begin transaction");
                q.exec("drop table if exists temp.bar");
                q.exec("create temp table bar(email text not null collate nocase, unique(email) on conflict ignore, check(length(trim(email)) > 0))");
                //q.exec("create index bardex on bar(email collate NOCASE)");
            }
            for(int i = 0; i < n; ++i)
            {
                QSqlQuery q;
                q.prepare("insert or ignore into bar values(trim(?1))");
                q.addBindValue(addrs[i]);
                q.exec();
            }
            QList<QPair<QString, QString> > res;
            {
#define Q qDebug() << q.lastError().text()
                QSqlQuery q;
                // get rid of obvious bogus values
                q.exec("delete from bar where not email like '%@%'");

                // get all candidate uids
                q.exec("create temp table foo as select uid from profiles,bar where trim(profiles.email) collate nocase = bar.email");

                // find the latest profile for all those uids
                q.exec("create temp table baz as select *,max(time) from profiles, foo using(uid) group by uid");

                // remove from baz any uids that don't have an email anymore (ie, user removed it for whatever reason.)
                q.exec("delete from baz where length(trim(email)) = 0");

                // also remove any emails that weren't in the input list, this can happen if someone
                // has an old email that is matched, but then changed the email to something different
                // in their latest profile entry. we don't want to return the new account
                q.exec("delete from baz where not exists(select 1 from bar where trim(baz.email) collate nocase = bar.email)");

                // ignore filtering
                q.prepare("with ign as (select ignorer from iy.entry_bag where ignoree = ?1 union select ignoree from iy.entry_bag where ignorer = ?1)"
                       "delete from baz where uid in (select * from ign)"
                       );
                q.addBindValue(huid);
                q.exec();
                q.exec("select uid, trim(email) from baz");

                while(q.next())
                {
                QString uid = q.value(0).toString();
                QString email = q.value(1).toString();
                res.append(QPair<QString, QString>(uid, email));
                }
                q.exec("rollback transaction");

            }

            QTemporaryFile tf;
            tf.open();
            save_it(res, tf);
            tf.close();
            send_file_to(uid, "", tf.fileName().toLatin1());

            processed_msg(mid);
        }

    }



}
