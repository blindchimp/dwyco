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
QByteArray My_uid;
int Nostalgia;

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
dwyco_chat_server_status_callback(int id, const char *msg, int /*percent_done*/, void * /*user_arg*/)
{
    if(strcmp(msg, "offline") == 0)
    {
        exit(1);
    }
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


        if(Sent.contains(buid))
            return;
        if(!Nostalgia)
        {
            send_reply_to(buid, "I'm a bot... This is just a test message to get you going. Have fun.");
        }
        else
        {
            send_reply_to(buid,
                          "Hi, this is a Dwyco bot with just a fun something to try:\r\n\r\n"
                          "Do you remember ICUII/Icu2? Do you still have an old copy of "
                          "Icu2 v9? If you do, and you want a quick trip down memory lane, "
                          "send me a message with the word \"yes\" in it, and I'll send you "
                          "a small installer you can run to make your old copy of Icu2 work "
                          "with recently resurrected servers.\r\n"
                          "As always from Dwyco, this little installer is safe, and just updates "
                          "one file to point to the new servers. "
                          "If you prefer to download the installer, you can find it here:\r\n"
                          "http://www.dwyco.com/downloads/ArcheologyServers.exe\r\n"
			  "ps. if you need a copy of Icu2, you can still get it here:\r\n"
			  "http://www.softpedia.com/get/Internet/WebCam/ICUII-Video-Chat.shtml"

                          );

        }
        Sent.insert(buid);
        save_it(Sent, "sent.qds");

        break;
    }
}


int
main(int argc, char *argv[])
{
    if(access("stop", F_OK) == 0)
        exit(0);
    if(argc < 3)
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
    char *cmd = strdup(argv[0]);
    cmd = basename(cmd);
    Nostalgia = 0;
    if(strcmp(cmd, "nostalgia") == 0)
        Nostalgia = 1;

    dwyco_set_login_result_callback(dwyco_db_login_result);
    dwyco_set_chat_server_status_callback(dwyco_chat_server_status_callback);
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

    load_it(Sent, "sent.qds");
    const char *my_uid;
    int len_uid;
    dwyco_get_my_uid(&my_uid, &len_uid);
    My_uid = QByteArray(my_uid, len_uid);
    Sent.insert(My_uid);

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
            exit(0);
        }


        QByteArray uid;
        QByteArray txt;
        int dummy;
        QByteArray mid;
        int has_att;

        if(dwyco_new_msg(uid, txt, dummy, mid, has_att))
        {
            txt = txt.toLower();
            if(!Nostalgia)
            {
                if(txt.contains("please respond"))
                    send_reply_to(uid, "Thanks, I'm a bot, and this is just a test response.");
            }
            else
            {
                if(txt.contains("yes"))
                    send_file_to(uid, "Just save the EXE to your Desktop or Downloads folder and run it.\r\n"
			"ps. if you need a copy of Icu2, you can still get it here:\r\n"
                          "http://www.softpedia.com/get/Internet/WebCam/ICUII-Video-Chat.shtml"
			, "ArcheologyServers.exe");
            }

            HANDLE_MSG(mid);

        }

    }



}
