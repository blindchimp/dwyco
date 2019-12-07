#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
#define HANDLE_MSG(m) dwyco_save_message(m)

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

QSet<QByteArray> Eligible_recipients;
QMap<QByteArray, QByteArray> Msg_q;

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

void
send_reply_to(const QByteArray& uid, const char *msg)
{
    int compid = dwyco_make_zap_composition(0);
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

void
load_eligible()
{
    dwyco_load_users();
    DWYCO_LIST ul;
    int n;
    if(!dwyco_get_user_list2(&ul, &n))
        return;
    simple_scoped uls(ul);
    for(int i = 0; i < n; ++i)
    {
        const char *uid;
        int len_uid;
        int type;
        if(!dwyco_list_get(uls, i, DWYCO_NO_COLUMN, &uid, &len_uid, &type))
            continue;
        DWYCO_MSG_IDX mi;
        if(!dwyco_get_message_index(&mi, uid, len_uid))
            continue;

        simple_scoped mis(mi);

        int mi_count;
        dwyco_list_numelems(mis, &mi_count, 0);
        if(mi_count < 2)
            continue;
        const char *val;
        int len_val;
        int type_val;
        for(int ii = 0; ii < 2; ++ii)
        {
            if(!dwyco_list_get(mis, ii, DWYCO_MSG_IDX_IS_SENT, &val, &len_val, &type_val))
                goto fail;
            if(type_val == DWYCO_TYPE_NIL)
                goto fail;
            if(!dwyco_list_get(mis, ii, DWYCO_MSG_IDX_MID, &val, &len_val, &type_val))
                goto fail;
            DWYCO_LIST sm;
            if(!dwyco_get_saved_message(&sm, uid, len_uid, val))
                goto fail;
            simple_scoped sms(sm);
            DWYCO_LIST txt = dwyco_get_body_text(sms);
            simple_scoped txts(txt);
            if(!dwyco_list_get(txts, 0, DWYCO_NO_COLUMN, &val, &len_val, &type_val))
                goto fail;
            QByteArray b(val, len_val);
            if(b.contains("Thanks, got your message"))
                continue;
            if(b.contains("Your msg was sent to a random user"))
                continue;
            goto fail;
        }
        {
        QByteArray ba(uid, len_uid);
        qDebug() << ba.toHex() << "\n";
        Eligible_recipients.insert(ba);
        }
fail:;

    }


}

int
main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
	alarm(3600);
    srand(time(0));
    const char *name = argv[1];
    const char *desc = argv[2];
    char *cmd = strdup(argv[0]);
    cmd = basename(cmd);
    int rando = (strcmp(cmd, "rando") == 0);
    int minimal = (strcmp(cmd, "bot") == 0);

    dwyco_set_login_result_callback(dwyco_db_login_result);

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
    int initial_load = 2;
    if(!load_it(Eligible_recipients, "er.qds"))
    {
        load_eligible();
        save_it(Eligible_recipients, "er.qds");
    }

    while(1)
    {
        int spin;
        dwyco_service_channels(&spin);
        usleep(10 * 1000);
        ++i;
        if(time(0) - start >= r)
        {
            dwyco_power_clean_safe();
            dwyco_exit();
            exit(0);
        }
        if(minimal)
            continue;

        if(rando)
        {
            QByteArray uid;
            QByteArray txt;
            int dummy;
            QByteArray mid;
            int has_att;

            if(dwyco_new_msg(uid, txt, dummy, mid, has_att))
            {
                if(initial_load == 2)
                    initial_load = 1;
                DWYCO_UNSAVED_MSG_LIST uml;

                if(!dwyco_get_unsaved_messages(&uml, uid.constData(), uid.length()))
                {
                    dwyco_delete_unsaved_message(mid.constData());
                    continue;
                }
                int n;
                if(dwyco_list_numelems(uml, &n, 0) == 0 || n > 1)
                {
                    send_reply_to(uid, "Thanks, I've already got a message from you and I'm working on sending it to a random Dwyconian, I'll let you know when you can send me another.");
                    HANDLE_MSG(mid.constData());
                    dwyco_list_release(uml);
                    continue;
                }
                dwyco_list_release(uml);
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
                if(!initial_load)
                {
                    send_reply_to(uid, "Thanks, got your message, I'll send you a random message soon!");
                }
                // one new message found, do rando
                Eligible_recipients.insert(uid);
                save_it(Eligible_recipients, "er.qds");
                Msg_q.insert(mid, uid);
            }
            else
            {
                if(initial_load == 1)
                    initial_load = 0;
            }

            // wait until the msg_q gets charged up so we don't have
            // two people communicating with each other via rando
            if(Msg_q.count() > 2)
            {
                QSet<QByteArray>::iterator eu = Eligible_recipients.begin();
                int erc = Eligible_recipients.count();
                erc = rand() % erc;
                while(eu != Eligible_recipients.end())
                {
                    if(erc > 0)
                    {
                        --erc;
                        continue;
                    }
                    QMap<QByteArray, QByteArray>::iterator i = Msg_q.begin();
                    while(i != Msg_q.end())
                    {
                        if(i.value() != *eu)
                        {
                            forward_msg(i.key(), *eu);
                            HANDLE_MSG(i.key());
                            send_reply_to(i.value(), "Your msg was sent to a random user, send me another.");
                            Msg_q.remove(i.key());
                            Eligible_recipients.remove(*eu);
                            save_it(Eligible_recipients, "er.qds");

                            goto done;
                        }
                        ++i;
                    }
                    ++eu;
                }
done:;
            }
        }
    }



}
