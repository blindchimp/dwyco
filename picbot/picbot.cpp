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
#include <QList>
#include <QByteArray>
#include <QMap>
#include <QSet>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QDir>
//#define HANDLE_MSG(m) dwyco_save_message(m)
#define HANDLE_MSG(m) dwyco_delete_unsaved_message(m)

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

QSet<QByteArray> Subscribers;
QSet<QByteArray> Sent;
quint32 Sent_age;


QFileInfoList Pics;
QStringList Pic_names;

struct simple_scoped
{
    DWYCO_LIST value;
    simple_scoped(DWYCO_LIST v) {value = v;}
    ~simple_scoped() {dwyco_list_release(value);}
    operator DWYCO_LIST() {return value;}
};

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

void
send_reply_to(const QByteArray& uid, const char *msg)
{
    int compid = dwyco_make_zap_composition(0);
    if(compid == 0)
        return;
    dwyco_zap_send5(compid, uid.constData(), uid.length(),
                    msg, strlen(msg), 0, 0, 0, 0);

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
load_pic_names(QString dirname)
{
    QDir d(dirname);
    d.setFilter(QDir::Files);

    Pic_names = d.entryList();
    int n = Pic_names.count();
    for(int i = 0; i < n; ++i)
    {
        Pic_names[i] = d.absoluteFilePath(Pic_names[i]);
    }
    return 1;
}

int
send_pic(QByteArray buid)
{
    if(!Subscribers.contains(buid))
        return 0;
    if(Sent.contains(buid))
        return 0;
    Sent.insert(buid);
    save_it(Sent, "sent.qds");

    // select random file
    int n = Pic_names.count();
    int i = rand() % n;
    QString fn = Pic_names[i];
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


    switch(cmd)
    {
    case DWYCO_CHAT_CTX_ADD_USER:

        send_pic(buid);

        break;
    }
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

    dwyco_set_login_result_callback(dwyco_db_login_result);
    dwyco_set_chat_ctx_callback(dwyco_chat_ctx_callback);

    dwyco_init();

    dwyco_set_setting("call_acceptance/no_listen", "1");

    if(dwyco_get_create_new_account())
    {
        dwyco_create_bootstrap_profile(name, strlen(name), desc, strlen(desc), "", 0, "", 0);

    }


    dwyco_set_local_auth(1);
    dwyco_finish_startup();
	int i = 0;
	int r = 20 * 60 + (rand() % 30) * 60;
    time_t start = time(0);

    load_pic_names(argv[3]);
    load_it(Subscribers, "subscribers.qds");
    if(!load_it(Sent_age, "sent_age.qds"))
    {
        Sent_age = time(0);
        save_it(Sent, "sent.qds");
        save_it(Sent_age, "sent_age.qds");
    }
    else
    {
        load_it(Sent, "sent.qds");
    }

    int was_online = 0;

    while(1)
    {
        int spin;
        dwyco_service_channels(&spin);
        if(spin)
            usleep(10 * 1000);
        else
            usleep(50 * 1000);
        ++i;
        if(time(0) - start >= r || access("stop", F_OK) == 0)
        {
            dwyco_power_clean_safe();
            dwyco_empty_trash();
            dwyco_exit();
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

        if(time(0) - Sent_age > 6 * 3600)
        {
            Sent.clear();
            save_it(Sent, "sent.qds");
            Sent_age = time(0);
            save_it(Sent_age, "sent_age.qds");
        }

        QByteArray uid;
        QByteArray txt;
        int dummy;
        QByteArray mid;
        int has_att;

        if(dwyco_new_msg(uid, txt, dummy, mid, has_att))
        {
            QByteArray ts = time_till();

            txt = txt.toLower();
            if(txt.contains("yes"))
            {
                if(Subscribers.contains(uid))
                {
                    send_reply_to(uid, QByteArray("You are already subscribed. ") + ts);
                }
                else
                {
                    Subscribers.insert(uid);
                    save_it(Subscribers, "subscribers.qds");
                    send_reply_to(uid, "I'll send you random pics every day you visit Dwyco. To stop me, send me a zap containing the word \"stop\".");
                    if(!send_pic(uid))
                    {
                        send_reply_to(uid, ts);
                    }
                }

            }
            else if(txt.contains("stop"))
            {
                Subscribers.remove(uid);
                save_it(Subscribers, "subscribers.qds");
                send_reply_to(uid, "Ok, no more pic of the day. Thanks!");
            }
            else
            {
                send_reply_to(uid, "I can send you pics, read my profile.");
            }
            HANDLE_MSG(mid);

        }

    }



}
