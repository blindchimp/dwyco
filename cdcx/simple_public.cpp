
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <stdio.h>
#include <QScrollArea>
#include <QScrollBar>
#include <QDateTime>
#include <QBuffer>
#include <QTextDocumentFragment>
#include <QMessageBox>
#include <QFile>
#include <time.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include "simple_public.h"
#include "ui_simple_public.h"
#include "adminw.h"
#include "mainwin.h"
#include "tfhex.h"
#include "composer.h"
#include "simple_call.h"
#include "popup_profile.h"
#include "ui_popup_profile.h"
#include "evret.h"
#include "ssmap.h"
#include "config.h"
#include "pfx.h"


QList<DVP> simple_public::Simple_publics;
extern int Allow_multiple_windows;
extern int HasAudioInput;
extern int HasAudioOutput;
extern DwOString My_uid;
extern QImage Last_preview_image;
extern DwOString ZeroUID;

// NOTE: removing the profile_pic filename could be a problem
// if the profile_pic is a regular file from a list of defaults
// (it works now because the defaults are resources that can't
// be removed.)

static void
clear_label(QLabel *l)
{
    l->clear();
    l->setPixmap(QPixmap());
    l->setText("");
    l->setProperty("used", 0);
    l->setProperty("uid", QVariant());
    l->setProperty("profile", QVariant());
    QString fn = l->property("profile_pic").toString();
    if(fn.length() > 0)
    {
        QFile::remove(fn);
    }
    l->setProperty("profile_pic", QVariant());

}

static void
clear_label_no_release(QLabel *l)
{
    l->clear();
    l->setPixmap(QPixmap());
    l->setText("");
    l->setProperty("uid", QVariant());
    l->setProperty("profile", QVariant());
    QString fn = l->property("profile_pic").toString();
    if(fn.length() > 0)
    {
        QFile::remove(fn);
    }
    l->setProperty("profile_pic", QVariant());

}

static
int
is_my_uid(const QByteArray& uid)
{
    if(uid == QByteArray(My_uid.c_str(), My_uid.length()))
        return 1;
    return 0;
}


#if 0
static void
label_border(QLabel *l, int state)
{
    switch(state)
    {
        QPalette p(ui.them->palette());
        p.setColor(QPalette::Light, QColor(Qt::red));
        p.setColor(QPalette::Dark, QColor(Qt::darkRed));
        ui.them->setPalette(p);
    }
    else
    {
        ui.them->setPalette(QApplication::palette());
    }

}
#endif


bool
hover_filter::eventFilter(QObject *o, QEvent *e)
{
#if 0
    if(/*e->type() == QEvent::HoverEnter*/ e->type() == QEvent::Enter)
    {
        printf("hover enter\n");
        QHoverEvent *he = static_cast<QHoverEvent *>(e);

        QByteArray uid = o->property("uid").toByteArray();
        int profile_up(QByteArray);
        if(profile_up(uid))
            return false;
        popup_profile *p = new popup_profile(0, Qt::FramelessWindowHint);
        p->uid = uid;
        p->where = QCursor::pos() + QPoint(5,5);
        QString fn = o->property("profile_pic").toString();
        if(fn.length() > 0)
        {

            QPixmap pm(fn);
            p->ui->pic->setPixmap(pm);
        }
        p->ui->name->setText(o->property("profile").toString());

    }
    else if(/*e->type() == QEvent::HoverLeave || */e->type() == QEvent::Leave)
    {
        printf("hover leave\n");
        QByteArray uid = o->property("uid").toByteArray();
        void close_profile(QByteArray);
        close_profile(uid);

    }
    else
#endif
        if(e->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent *me = static_cast<QMouseEvent *>(e);
            if(me->button() == Qt::LeftButton)
            {
                QByteArray uid = o->property("uid").toByteArray();
                if(uid.length() == 0)
                    return false;
                DwOString duid(uid.constData(), 0, uid.length());
                Mainwinform->emit uid_selected(duid, 5);
                int regular;
                int reviewed;
                get_review_status(duid, reviewed, regular);
                if(!show_profile(duid, reviewed, regular))
                {
                    if(!confirm_show_profile())
                        return false;
                }
                if(!Allow_multiple_windows)
                    viewer_profile::delete_all();
                viewer_profile *c = viewer_profile::get_viewer_profile(duid);
                if(!c)
                {
                    c = new viewer_profile(Mainwinform);
                    c->set_uid(duid);
                    c->move(QCursor::pos());
                    c->show();
                    c->start_fetch();
                }
                else
                {
                    c->move(QCursor::pos());
                    c->show();
                    c->raise();
                    c->showNormal();
                }

            }
        }
    return false;
}

simple_public::simple_public(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::simple_public),
    vp(this)
{
    ui->setupUi(this);

    //labels.resize(ui->totem1->count() + ui->totem2->count() + ui->totem3->count());
    int n = ui->totem1->count();
    for(int i = 0; i < n; ++i)
    {
        labels.append(dynamic_cast<QLabel *>(ui->totem1->itemAt(i)->widget()));
    }
    int n2 = ui->totem2->count();
    for(int i = n; i < n + n2; ++i)
    {
        labels.append(dynamic_cast<QLabel *>(ui->totem2->itemAt(i - n)->widget()));
    }
    int n3 = ui->totem3->count();
    for(int i = n + n2; i < n + n2 + n3; ++i)
    {
        labels.append(dynamic_cast<QLabel *>(ui->totem3->itemAt(i - (n + n2))->widget()));
    }
    // we know they are fixed nxn boxes
    int rad = labels[0]->width();
    QRegion r(QRect(0, 0, rad, rad), QRegion::Ellipse);

    for(int i = 0; i < labels.count(); ++i)
    {
        labels[i]->setProperty("used", 0);
        labels[i]->setMouseTracking(1);
        //labels[i]->setAttribute(Qt::WA_Hover);
        labels[i]->installEventFilter(new hover_filter);
        //labels[i]->setText(QString("%1").arg(i));
        //labels[i]->hide();
        int rad = labels[i]->width();
        QRegion r(QRect(0, 0, rad, rad), QRegion::Ellipse);
        labels[i]->setMask(r);
    }

    connect(Mainwinform, SIGNAL(public_chat_display(DwOString, DwOString, DwOString)), this,
            SLOT(display_public_chat(DwOString, DwOString, DwOString)));
    connect(Mainwinform, SIGNAL(chat_event(int,int,QByteArray,QString,QVariant,int,int)),
            SLOT(chat_event(int,int,QByteArray,QString, QVariant,int,int)));

    connect(Mainwinform, SIGNAL(ignore_event(DwOString)), SLOT(update_ignore(DwOString)));
    connect(this, SIGNAL(ignore_event(DwOString)), Mainwinform, SIGNAL(ignore_event(DwOString)));

    connect(Mainwinform, SIGNAL(admin_event(int)), this, SLOT(admin_update(int)));
    connect(Mainwinform, SIGNAL(call_vector_event(DwOString,QVector<int>)), this, SLOT(update_call_vector_display(DwOString,QVector<int>)));
    connect(this, SIGNAL(uid_selected(DwOString,int)), Mainwinform, SIGNAL(uid_selected(DwOString,int)));
    connect(Mainwinform, SIGNAL(mouse_stopped(QPoint)), this, SLOT(mouse_stopped_event(QPoint)));
    connect(Mainwinform, SIGNAL(chat_server_status(int)), this, SLOT(chat_server_event(int)));
    connect(Mainwinform, SIGNAL(new_server_name(DwOString)), this, SLOT(set_title(DwOString)));
    connect(Mainwinform, SIGNAL(uid_info_event(DwOString)), this, SLOT(uid_resolved(DwOString)));
    connect(Mainwinform, SIGNAL(invalidate_profile(DwOString)), this, SLOT(update_profile(DwOString)));
    connect(Mainwinform, SIGNAL(pal_event(DwOString)), this, SLOT(update_profile(DwOString)));
    connect(TheConfigForm, SIGNAL(content_filter_event(int)), this, SLOT(redisplay_all(int)));
    connect(Mainwinform, SIGNAL(video_display(int,QImage)), this, SLOT(show_video_in_label(int,QImage)));
    connect(Mainwinform, SIGNAL(audio_recording(int,int)), this, SLOT(audio_recording_event(int, int)));
    connect(this, SIGNAL(audio_recording(int,int)), Mainwinform, SIGNAL(audio_recording(int,int)));

    ui->toolButton_admin->setDefaultAction(ui->actionAdmin);
    ui->actionAdmin->setVisible(0);
    ui->toolButton_admin->setVisible(0);

    ui->dj->setDefaultAction(ui->actionDj);
    ui->mute->setDefaultAction(ui->actionMute);
    ui->send_snapchat->setDefaultAction(ui->actionSend_snapchat);

    if(!HasAudioInput)
    {
        ui->actionDj->setVisible(0);
        ui->dj->setVisible(0);
    }
    else
    {
        AudioActionGroup->append(ui->actionDj);
    }
    if(!HasAudioOutput)
    {
        ui->actionMute->setVisible(0);
        ui->mute->setVisible(0);
    }

    popup_menu = new QMenu(this);
    popup_menu->addAction(ui->actionView_profile_ctx);
    popup_menu->addAction(ui->actionShow_Chatbox_ctx);
    popup_menu->addAction(ui->actionCompose_msg_ctx);
    popup_menu->addAction(ui->actionIgnore_user_ctx);

    hide_video = 0;
    if(!setting_get("chat_dont_display_video", hide_video))
    {
        setting_put("chat_dont_display_video", 0);
        settings_save();
    }
    ui_id = -1;
    Simple_publics.append(vp);
}

simple_public::~simple_public()
{
    Simple_publics.removeOne(vp);
    vp.invalidate();
    AudioActionGroup->removeOne(ui->actionDj);
    delete ui;
}

void
simple_public::set_hide_video(int hv)
{
    if(hide_video != hv)
    {
        hide_video = hv;
        refresh_all_profiles();
    }
}

void
simple_public::chat_server_event(int online)
{
    // the chan_id is used to direct audio exclusively
    // via the "exclusive_audio" calls
    if(!online)
    {

        append_msg_to_textedit("<br><font color=#ff0000>Disconnected, click on a lobby name in the lobby list to reconnect</font><br>", ui->text_display, "");

        cursor_to_bottom(ui->text_display);
    }
}

void
simple_public::set_title(DwOString ttl)
{
    current_title = ttl;
    QString tl(ttl.c_str());
    if(podium_uid.length() > 0)
    {
        QString pod = dwyco_info_to_display(DwOString(podium_uid.constData(), 0, podium_uid.length()));
        pod.truncate(15);
        tl += QString(" (dj: ") + pod + QString(")");
    }
    setWindowTitle(tl);
}

void
simple_public::mouse_stopped_event(QPoint pnt)
{
    QLabel *o = 0;
    for(int i = 0; i < labels.count(); ++i)
    {
        if(labels[i]->underMouse())
        {
            o = labels[i];
            break;
        }
    }

    void close_all_profiles();
    if(!o || o->property("used").toInt() == 0)
    {
        // stopped elsewhere, downpop everything

        close_all_profiles();
    }
    else
    {
        QByteArray uid = o->property("uid").toByteArray();
        if(uid.length() == 0)
            return;
        int profile_up(QByteArray);
        if(profile_up(uid))
            return;
        close_all_profiles();
        popup_profile *p = new popup_profile(0, Qt::FramelessWindowHint);
        p->uid = uid;
        p->where = pnt + QPoint(5, 5);
        QString fn = o->property("profile_pic").toString();
        if(fn.length() > 0)
        {
            QPixmap pm(fn);

            QLabel *l = p->ui->pic;
            int rad = l->width() < l->height() ? l->width() : l->height();
            QRegion r(QRect(20, 0, rad, rad), QRegion::Ellipse);
            p->ui->pic->setMask(r);
            p->ui->pic->setPixmap(pm);
        }
        p->ui->name->setText(o->property("profile").toString());
        QString attrs;
        DwOString duid(uid);
        if(uid_attrs_has_attr(duid, "ui-activity"))
        {
            attrs += "idle ";
        }
        else
        {
            attrs += "active ";
        }
        if(uid_attrs_has_attr(duid, "ui-pals-only"))
        {
            attrs += "pals-only";
        }
        p->ui->attrs->setText(attrs);
    }

}

QPixmap
simple_public::get_preview_by_uid(DwOString uid)
{
    QPixmap ret;

    int i = UList.indexOf(uid);
    if(i == -1)
        return ret;

    int lab_idx = UList[i].lab_idx;
    if(lab_idx == -1)
        return ret;

    QLabel *lab = labels[lab_idx];

    QString fn = lab->property("profile_pic").toString();
    if(fn.length() > 0)
    {
        QPixmap pm(fn);
        return pm;
    }
    return ret;
}

void
simple_public::update_profile(DwOString uid)
{
    QByteArray b(uid.c_str(), uid.length());
    refresh_profile(b);
}

void
simple_public::update_ignore(DwOString uid)
{
    if(dwyco_is_ignored(uid.c_str(), uid.length()))
        remove_user(QByteArray(uid.c_str(), uid.length()));
    // hmmm, in the else case, we might need to prepare
    // for some out-of-order updates (ie, they were filtering
    // out all the msgs regarding this user, now all of a sudden
    // we will start getting them again)
}
void
simple_public::admin_update(int style)
{
    // server is telling us we are in admin mode
    // style:
    // 0: no god
    // 1: god
    // 2: demigod
    // 4: subgod
    ui->actionAdmin->setVisible(style > 0 ? 1 : 0);
    ui->toolButton_admin->setVisible(style > 0 ? 1 : 0);

    if(popup_menu)
        delete popup_menu;
    popup_menu = new QMenu(this);
    popup_menu->addAction(ui->actionView_profile_ctx);
    popup_menu->addAction(ui->actionShow_Chatbox_ctx);
    popup_menu->addAction(ui->actionCompose_msg_ctx);
    popup_menu->addAction(ui->actionIgnore_user_ctx);
    if(style > 0)
    {
        popup_menu->addAction(ui->actionAdmin_ctx);
    }


}

void
simple_public::update_call_vector_display(DwOString uid, QVector<int> cv)
{

}

void
simple_public::display_public_chat(DwOString who, DwOString chat, DwOString uid)
{
    QList<QVariant> fl;
    fl.append(chat.c_str());
    fl.append(0);
    fl.append("");
    display_snapchat(QByteArray(uid.c_str(), uid.length()), who.c_str(), fl);
    return;
}

static
int
is_trivia(QByteArray quid)
{
    if(quid.length() != 8)
        return 0;
    return memcmp(quid.constData(), "\xff\x00\xff\x00\xff\x00\xff\x00", 8) == 0;
}

static
QString
html_from_data(QByteArray quid, QString name, QString chat, DwOString picfn)
{
    QString html(
        "<div ><table width=\"100%\" border=\"1\" cellpadding=\"0\" cellspacing=\"0\" \">"
        "<tr valign=\"top\">"
        "<td  rowspan=2 valign=\"middle\" height=\"90px\" ><span>%1</span>"
        "</td>"
        "<td ><span >%2</span>"
        "</td>"
        "</tr>"
        "<tr valign=\"top\">"
        "<td ><span >%3</span>"
        "</td>"
        "</tr>"
        "</table>"
        "</div>"
    );

    // we don't send fancy html in cdcx, just prepend a little
    // html. REMEMBER TO STRIP OUT HTML IN "WHO"
    DwOString uid(quid.constData(), 0, quid.length());
    QString nm = "<a href=\"";
    QString duid = to_hex(uid).c_str();
    nm += duid;
    nm += "\">";
    QString w = strip_html(name);
    w = w.simplified();
    w.truncate(20);
    if(w.length() == 0)
        w = "empty";
    nm += w;
    nm += "</a>";
    DwOString dduid = uid;
    if(uid_attrs_has_attr(dduid, "ui-god"))
    {
        nm.prepend("<font color=#ff0000><sup>admin</sup></font>");
    }
    else if(uid_attrs_has_attr(dduid, "ui-demigod"))
    {
        nm.prepend("<font color=#ff00ff><sup>mod</sup></font>");
    }
    else if(uid_attrs_has_attr(dduid, "ui-subgod"))
    {
        nm.prepend("<font color=#ff00ff><sup>owner</sup></font>");
    }

    QString imghtml = QString("<img src = \"%1\">").arg(picfn.c_str());

    QString ret;

    int no_img = 0;
    setting_get("chat_pic_size", no_img);
    no_img = (no_img == 0);
    if(!is_trivia(quid))
        chat = strip_html(chat);
    if(no_img)
    {
        ret = nm + QString(" : ") + chat;
    }
    else
    {
        ret = html.arg(imghtml, nm, chat);
    }

    return ret;
}



void
simple_public::display_snapchat(QByteArray uid, QString name, QVariant data)
{
    QList<QVariant> dl(data.toList());
    QString chat = dl[0].toString();
    int pic_type = dl[1].toInt();
    QByteArray pic_data = dl[2].toByteArray();

    DwOString fn("no_img.png");
    fn = add_pfx(Sys_pfx, fn);
    if(pic_type == DWYCO_CHAT_DATA_PIC_TYPE_JPG)
    {
        // jpeg image for now
        QPixmap img;
        img.loadFromData(pic_data);
        if(is_trivia(uid))
        {
            img = img.scaled(32, 32, Qt::KeepAspectRatio);
            fn = "triv_icon";
        }
        else
        {
            img = img.scaled(120, 90, Qt::KeepAspectRatio);
            fn = random_fn();
        }
        fn = add_pfx(Tmp_pfx, fn);
        fn += ".ppm";
        if(!QFile::exists(fn.c_str()))
            img.save(fn.c_str());
    }
    else
    {
        // no image
    }
    update_talk_time(uid);

    QString entry = html_from_data(uid, name, chat, fn);

    QScrollBar *sb = ui->text_display->verticalScrollBar();
    int to_bottom = 1;
    if(sb && sb->value() != sb->maximum())
        to_bottom = 0;
    append_msg_to_textedit(entry, ui->text_display, "");
    if(to_bottom)
        cursor_to_bottom(ui->text_display);
}


void simple_public::chat_event(int cmd,int id, QByteArray uid, QString name, QVariant data, int qid, int extra_arg)
{
    //printf("%d", cmd);

    switch(cmd)
    {
    case DWYCO_CHAT_CTX_ADD_USER:
        add_user_profile(name, uid);
        break;
    case DWYCO_CHAT_CTX_DEL_USER:
        remove_user(uid);
        break;
    case DWYCO_CHAT_CTX_NEW:
        clear();
        break;
    case DWYCO_CHAT_CTX_DEL:
        podium_uid = QByteArray();
        set_title(current_title);
        break;
    case DWYCO_CHAT_CTX_Q_ADD:
        if(is_my_uid(uid))
        {
            ui->actionDj->setIcon(QIcon(":new/red32/icons/PrimaryCons_Red_KenSaunders/PNGs/32x32/question-32x32.png"));
        }
        break;
    case DWYCO_CHAT_CTX_Q_DEL:
        if(is_my_uid(uid))
        {
            ui->actionDj->setIcon(QIcon());
            ui->actionDj->setChecked(0);
        }
        if(uid == podium_uid)
            podium_uid = QByteArray();
        set_title(current_title);
        break;
    case DWYCO_CHAT_CTX_Q_GRANT:
        if(is_my_uid(uid))
        {
            ui->actionDj->setIcon(QIcon(":new/green32/icons/PrimaryCons_Green_KenSaunders/PNGs/32x32/exclamation-32x32.png"));
            // audio will be streaming to server now, so stop everyone else
            emit audio_recording(-1, 1);
        }
        podium_uid = uid;
        set_title(current_title);
        break;

    case DWYCO_CHAT_CTX_SYS_ATTR:
        if(name == "us-server-rules" || name == "icuii-server-rules")
        {
            append_msg_to_textedit(data.toString(), ui->text_display, "");
            cursor_to_bottom(ui->text_display);
        }
        else if(name == "ui-trial-block")
        {
            append_msg_to_textedit(QString("<font color=\"#ff0000\">Trial expired: %1").arg(data.toString()),
                                   ui->text_display, "");
            cursor_to_bottom(ui->text_display);
        }
        else  if(name == "ui-regcode-block")
        {
            append_msg_to_textedit(QString("<font color=\"#ff0000\">Subscription code problem: %1").arg(data.toString()),
                                   ui->text_display, "");
            cursor_to_bottom(ui->text_display);
        }
        else if(name == "ui-global-block")
        {
            append_msg_to_textedit("<font color=\"#ff0000\">You have been blocked from using the service.",
                                   ui->text_display, "");
            cursor_to_bottom(ui->text_display);
        }
        else if(name == "ui-full")
        {
            append_msg_to_textedit("<font color=\"#ff0000\">Sorry, the server is full, try a different one.</font>", ui->text_display, "");
            cursor_to_bottom(ui->text_display);

        }
        else if(name == "ui-pw-bad")
        {
            append_msg_to_textedit("<font color=\"#ff0000\">Password incorrect.</font>", ui->text_display, "");
            cursor_to_bottom(ui->text_display);
        }
        else if(name == "ui-blocked")
        {
            append_msg_to_textedit("<font color=\"#ff0000\">You have been blocked from using this server.",
                                   ui->text_display, "");
            // in this case, we have a list of qvariants
            // corresponding to DWYCO_BLOCK_REC*. prolly ought to change it to a
            // map with strings or something, but the current dlli api doesn't
            // really have that level of easy introspection. so we just do it by
            // hand here.
            QList<QVariant> br = data.toList();
            time_t unblock_time = br[1].toInt();
            QString reason = br[4].toString();
            QDateTime ubt = QDateTime::fromTime_t(unblock_time);
            QString msg = QString("Block is in effect until %1<br>Reason: %2").arg(ubt.toString(Qt::SystemLocaleDate)).arg(reason);

            append_msg_to_textedit(msg, ui->text_display, "");
        }
        break;

    case DWYCO_CHAT_CTX_UPDATE_ATTR:

        if(name == "ui-activity")
        {
            uid_idle_event(uid, data);
        }
        else if(name == "ui-mute")
        {
            if(is_my_uid(uid))
            {
                ui->actionMute->setChecked(data == QVariant() ? 0 : 1);
            }
        }
        else if(name == "ui-pals-only")
        {
            //printf("PALS only %d %s\n", (data == QVariant() ? 0 : 1), to_hex(DwOString(uid.constData(), 0, uid.length())).c_str());
            //fflush(stdout);
        }
        break;

    case DWYCO_CHAT_CTX_RECV_DATA:

        display_snapchat(uid, name, data);
        break;
    }
}

void
simple_public::uid_idle_event(QByteArray uid, QVariant data)
{
    int i = UList.indexOf(uid);
    if(i == -1)
        return;
    if(data == QVariant())
        UList[i].idle_time = 0;
    else if(data.toString() == "idle")
        UList[i].idle_time = time(0);
    //QString msg = QString("uid %1: %2").arg(to_hex(DwOString(uid.constData(), 0, uid.length())).c_str()).arg(data.isNull() ? "active" : "idle");
    //append_msg_to_textedit(msg, ui->text_display, "");
}

QLabel *
simple_public::find_vg_label(int ui_id)
{
    int n = Simple_publics.count();
    QString empty;
    for(int i = 0; i < n; ++i)
    {
        simple_public *vg = (simple_public *)(void *)Simple_publics[i];
        QList<ul_item>& ul = vg->UList;
        for(int j = 0; j < ul.count(); ++j)
        {

            if(ul[j].uid != empty && ul[j].ui_id == ui_id)
            {
                if(ul[j].lab_idx != -1)
                    return vg->labels[ul[j].lab_idx];
                else
                    return 0;
            }
        }
    }
    return 0;
}

void
simple_public::show_video_in_label(int ui_id, QImage img)
{
    QLabel *lab = find_vg_label(ui_id);
    if(!lab)
        return;
    lab->setPixmap(QPixmap::fromImage(img));

}

static void
DWYCOCALLCONV
done_playing(int viewer_id, void *)
{
    dwyco_delete_zap_view(viewer_id);
}

static void
DWYCOCALLCONV
display_profile(int succ, const char *reason,
                const char *s1, int len_s1,
                const char *s2, int len_s2,
                const char *s3, int len_s3,
                const char *filename,
                const char *uid, int len_uid,
                int reviewed, int regular,
                void *user_arg)
{

// note: might want to stash the other text into a
// hint or something so it shows up

    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)user_arg);
    if(!vp.is_valid())
        return;
    simple_public *vg = (simple_public *)(void *)vp;
    QByteArray u(uid, len_uid);
    DwOString duid(uid, 0, len_uid);
    int idx;
    if((idx = vg->UList.indexOf(u)) == -1)
        return;
    int lidx = vg->UList[idx].lab_idx;
    if(lidx == -1)
        return;
    cdcx_set_refresh_users(1);
    int dont_show = !show_profile(duid, reviewed, regular);
    const char *default_guy;
    if(dont_show)
        default_guy = ":/new/prefix1/redguy.png";
    else
        default_guy = ":/new/prefix1/greenguy.png";

    vg->labels[lidx]->setProperty("profile_pic", default_guy);
    vg->labels[lidx]->setProperty("uid", QByteArray(uid, len_uid));
    if(!succ)
    {
        // put a placeholder in if we can get a label spot for them
        //if(dont_show)
        //vg->labels[lidx]->setProperty("profile", "(filtered)");
        //else
        vg->labels[lidx]->setProperty("profile", dwyco_info_to_display(duid));
        vg->labels[lidx]->clear();
        vg->labels[lidx]->setPixmap(QPixmap(default_guy));
        return;
    }
    if(succ == -1 || dont_show)
    {
        // no attachment, just text
        vg->labels[lidx]->setPixmap(QPixmap(default_guy));
        //if(dont_show)
        //vg->labels[lidx]->setProperty("profile", "(filtered)");
        //else
        vg->labels[lidx]->setProperty("profile", dwyco_info_to_display(duid));
    }
    else if(succ == -2)
    {
        if(vg->hide_video)
        {
            vg->labels[lidx]->setPixmap(QPixmap(":/new/prefix1/greenguy.png"));
            vg->labels[lidx]->setProperty("profile", dwyco_info_to_display(duid));
        }
        else
        {
            QImage img(filename);
            vg->labels[lidx]->setPixmap(QPixmap::fromImage(img));
            vg->labels[lidx]->setProperty("profile_pic", filename);
            vg->labels[lidx]->setProperty("profile", dwyco_info_to_display(duid));
        }
    }
    else
    {
        simple_public::ul_item *ul = &vg->UList[idx];
        ul->zv_id = succ;
        int video = 0;
        int audio = 0;
        int short_video = 0;
        if(dwyco_zap_quick_stats_view(ul->zv_id, &video, &audio, &short_video))
        {
            if(video || short_video)
            {
                if(!vg->hide_video)
                    dwyco_zap_play_view_no_audio(ul->zv_id, done_playing, 0, &ul->ui_id);
                else
                {
                    vg->labels[lidx]->setPixmap(QPixmap(":/new/prefix1/greenguy.png"));
                    vg->labels[lidx]->setProperty("profile", dwyco_info_to_display(duid));
                }
            }
            else
            {
                // audio only
                vg->labels[lidx]->setPixmap(QPixmap(":/new/prefix1/greenguy.png"));
                vg->labels[lidx]->setProperty("profile", dwyco_info_to_display(duid));
                return;
            }
        }
        // this is for the popup profile
        DwOString fn = random_fn();
        fn += ".ppm";
        fn = add_pfx(Tmp_pfx, fn);
        if(dwyco_zap_create_preview(succ, fn.c_str(), fn.length()))
        {
            vg->labels[lidx]->setProperty("profile_pic", fn.c_str());
        }
        vg->labels[lidx]->setProperty("profile", dwyco_info_to_display(duid));
    }
}


// this should be called when the person becomes
// a non-lurker, which could be from saying something
// in the chat room, or becoming available for vod, or
// whatever
int
simple_public::update_talk_time(QByteArray uid)
{
    int j;
    for(j = 0; j < UList.count(); ++j)
    {
        if(UList[j].uid.length() != 0 && UList[j].uid == uid)
        {
            UList[j].last_time = time(0);
            break;
        }
    }

    if(j >= UList.count())
        return 0;

    ul_item &ul = UList[j];
    // if this person is not in the non-lurker rows,
    // then see if we can swap him up there

    if(ul.lab_idx == -1 || ul.lab_idx >= 10)
    {
        int oldest = find_oldest_talker();
        if(oldest == -1)
            return 0;
        if(oldest == j)
            return 0;
        int tlo = UList[oldest].lab_idx;
        UList[oldest].lab_idx = ul.lab_idx;
        ul.lab_idx = tlo;
        refresh_profile(ul.uid);
        refresh_profile(UList[oldest].uid);

        // note: problem here if ul.lab_idx == -1, where
        // they are not allocated a label yet. would swap
        // the oldest talker into a non-displayed area.
        // probably need to find the oldest non-talker and bump them
        // out

    }

    return 1;
}

// find someone who is in the non-lurker area
// that hasn't said anything lately. they are
// a candidate to be replaced by someone that is
// not lurking
// note: fix: using first 10 labels as "non-lurker" area.
int
simple_public::find_oldest_talker()
{
    // kluge, really need "MAX_TIMET"
    time_t oldest = time(0) + 5000;
    int ret = -1;
    for(int i = 0; i < UList.count(); ++i)
    {
        if(UList[i].lab_idx != -1 &&
                UList[i].lab_idx < 10 &&
                UList[i].uid.length() != 0)
        {
            // return the first person that has said nothing
            if(UList[i].last_time == 0)
                return i;

            if(UList[i].last_time < oldest)
            {
                oldest = UList[i].last_time;
                ret = i;
            }
        }
    }
    return ret;
}

// find the user that has been idle the longest and is
// displayed in the totems. they are candidate to be
// replaced
int
simple_public::find_idle_displayed()
{
    time_t now = time(0);
    int ret = -1;
    int longest = -1;
    for(int i = 0; i < UList.count(); ++i)
    {
        if(UList[i].lab_idx != -1 &&
                UList[i].uid.length() != 0 &&
                UList[i].idle_time != 0 &&
                (now - UList[i].idle_time) > longest)
        {
            longest = now - UList[i].idle_time;
            ret = i;
        }
    }
    return ret;
}

// find the user that is not displayed and has the lowest
// idle time. they should be promoted to being displayed
// in the totem if there is an empty spot.
int
simple_public::find_undisplayed()
{
    time_t now = time(0);
    int ret = -1;
    int shortest = 1 << 30;
    for(int i = 0; i < UList.count(); ++i)
    {
        if(UList[i].lab_idx == -1 &&
                UList[i].uid.length() != 0)
        {
            if(UList[i].idle_time == 0)
            {
                // pick first non-idle person
                return i;
            }
            else if(now - UList[i].idle_time < shortest)
            {
                shortest = now - UList[i].idle_time;
                ret = i;
            }
        }
    }
    return ret;

}

int
simple_public::find_unused_label()
{
    for(int i = 0; i < labels.count(); ++i)
    {
        if(labels[i]->property("used") == 0)
        {
            labels[i]->setProperty("used", 1);
            return i;
        }
    }
    return -1;
}

void
simple_public::redisplay_all(int)
{
    refresh_all_profiles();
}

void
simple_public::refresh_all_profiles()
{
    int n = UList.count();
    for(int i = 0; i < n; ++i)
    {
        refresh_profile(UList[i].uid);
    }
}

void
simple_public::refresh_profile(QByteArray uid)
{
    int i;
    if((i = UList.indexOf(uid)) == -1)
        return;
    if(UList[i].lab_idx == -1)
        return;
    clear_label_no_release(labels[UList[i].lab_idx]);
    dwyco_get_profile_to_viewer(uid.constData(), uid.length(),
                                display_profile, (void *)vp.cookie);

}

void
simple_public::add_user_profile(QString name, QByteArray uid)
{
    if(!UList.contains(uid))
    {
        int i;

        dwyco_get_profile_to_viewer(uid.constData(), uid.length(),
                                    display_profile, (void *)vp.cookie);

        if((i = UList.indexOf(QByteArray())) == -1)
        {
            // no free items
            ul_item a(uid);

            // fix: map new item to someplace in layout
            int li = find_unused_label();
            if(li == -1)
            {
                // no unused labels, give new people preference, and find an
                // old idle user and knock them out of the display totems
                int idle_idx = find_idle_displayed();
                if(idle_idx != -1)
                {
                    clear_label_no_release(labels[UList[idle_idx].lab_idx]);
                    li = UList[idle_idx].lab_idx;
                    UList[idle_idx].lab_idx = -1;
                }
                // note: could have situation where someone is not idle but is
                // not chatting either, and they take up a spot. we can put more
                // detailed prioritizing later, this is ok for now.
            }
            a.lab_idx = li;
            UList.append(a);
            return;
        }
        UList[i].uid = uid;
        if(UList[i].lab_idx == -1)
        {
            // fix: map new item to someplace in layout
            int li = find_unused_label();
            if(li == -1)
            {
                // no unused labels, give new people preference, and find an
                // old idle user and knock them out of the display totems
                int idle_idx = find_idle_displayed();
                if(idle_idx != -1)
                {
                    clear_label_no_release(labels[UList[idle_idx].lab_idx]);
                    li = UList[idle_idx].lab_idx;
                    UList[idle_idx].lab_idx = -1;
                }
                // note: could have situation where someone is not idle but is
                // not chatting either, and they take up a spot. we can put more
                // detailed prioritizing later, this is ok for now.
            }

            UList[i].lab_idx = li;
        }
    }
}

void
simple_public::uid_resolved(DwOString uid)
{
    QByteArray buid(uid.c_str(), uid.length());
    if(podium_uid.length() > 0 && buid == podium_uid)
        set_title(current_title);

    int i;
    if((i = UList.indexOf(buid)) == -1)
    {
        return;
    }
    if(UList[i].lab_idx == -1)
    {
        return;
    }
    QLabel *la = labels[UList[i].lab_idx];
    //int reviewed;
    //int regular;
    //get_review_status(uid, reviewed, regular);
    //if(show_profile(uid, reviewed, regular))
    la->setProperty("profile", dwyco_info_to_display(uid));
    //else
    //la->setProperty("profile", "(filtered)");

}



void
simple_public::remove_user(QByteArray uid)
{
    int i;
    if((i = UList.indexOf(uid)) == -1)
    {
        return;
    }
    if(UList[i].lab_idx != -1)
    {
        clear_label(labels[UList[i].lab_idx]);
        UList[i] = ul_item();
    }
    else
    {
        // removed user wasn't displayed, so no way to
        // pop someone else into the vacated area
        UList[i] = ul_item();
        return;
    }

    // if there is someone that is undisplayed, put them
    // back into the visible totem
    i = find_undisplayed();
    if(i == -1)
        return;
    UList[i].lab_idx = find_unused_label();
    if(UList[i].lab_idx == -1)
        cdcxpanic("no cleared labels after clearing one?");
    refresh_profile(UList[i].uid);

}

void
simple_public::clear()
{
    int n = UList.count();
    for(int i = 0; i < n; ++i)
    {
        UList[i] = ul_item();
    }
    n = labels.count();
    for(int i = 0; i < n; ++i)
    {
        clear_label(labels[i]);
    }
    ui->text_display->clear();
    podium_uid = QByteArray();
    ui->actionDj->setChecked(0);
    ui->actionDj->setIcon(QIcon());
    ui->actionMute->setChecked(1);
}


void simple_public::on_text_enter_returnPressed()
{
    QString p = ui->text_enter->text();
    int popup = 0;
    if(p.startsWith("/p"))
    {
        p.remove(0, 2);
        popup = 1;

    }
    else if(p.startsWith("/gp"))
    {
        p.remove(0, 3);
        popup = 2;
    }
    if(popup)
    {
        dwyco_chat_send_popup(p.toAscii().constData(), p.toAscii().length(), popup == 1 ? 0 : 1);
    }
    else
    {
        QByteArray b;
        b = p.toAscii();

        //dwyco_line_from_keyboard(ui_id, b.constData(), b.length());
        QBuffer jb;
        jb.open(QIODevice::WriteOnly);
        if(!Last_preview_image.isNull())
            Last_preview_image.save(&jb, "JPG");
        const QByteArray& qb = jb.data();

        int send_img = ui->send_snapchat->isChecked() && !Last_preview_image.isNull();
        dwyco_chat_send_data(b.constData(), b.length(),
                             send_img ? DWYCO_CHAT_DATA_PIC_TYPE_JPG : DWYCO_CHAT_DATA_PIC_TYPE_NONE,
                             qb.constData(), qb.length());
    }
    ui->text_enter->clear();

    void cursor_to_bottom(QTextEdit *);
    cursor_to_bottom(ui->text_display);

}

void simple_public::on_text_display_anchorClicked(const QUrl& url )
{
    context_uid = from_hex(url.path().toAscii().constData());
    emit uid_selected(from_hex(url.path().toAscii().constData()), 1);
    popup_menu->popup(QCursor::pos());
}

void simple_public::on_actionShow_Chatbox_ctx_triggered()
{
    simple_call *c = simple_call::get_simple_call(context_uid);

    c->show();
    c->raise();
    Mainwinform->organize_chatboxes(c);
}

void simple_public::on_actionCompose_msg_ctx_triggered()
{
    composer *c = new composer(COMP_STYLE_REGULAR, 0, this);
    c->set_uid(context_uid);
    c->show();
    c->raise();
}

void simple_public::on_actionIgnore_user_ctx_triggered()
{
    if(confirm_ignore())
        dwyco_ignore(context_uid.c_str(), context_uid.length());
    emit ignore_event(context_uid);
}


void simple_public::on_actionAdmin_triggered()
{
    Adminform->show();
    Adminform->raise();
}

void simple_public::on_actionAdmin_ctx_triggered()
{
    Adminform->show();
    Adminform->raise();
}


void simple_public::on_actionDj_toggled(bool )
{

}

void simple_public::on_actionMute_toggled(bool )
{

}

void simple_public::on_actionDj_triggered(bool checked)
{

    if(checked == 0)
    {
        dwyco_chat_delq(0, 0, 0);
    }
    else
    {
        dwyco_chat_addq(0);
    }

}

void
simple_public::audio_recording_event(int chan_id, int)
{
    // if someone else is requesting to send audio, we need to drop
    // off the podium
    if(chan_id != -1)
        dwyco_chat_delq(0, 0, 0);

}

void simple_public::on_actionMute_triggered(bool checked)
{
    dwyco_chat_mute(0, checked);
}

void simple_public::on_actionView_profile_ctx_triggered()
{
    viewer_profile *c;

    DwOString duid = context_uid;
    if(!Allow_multiple_windows)
        viewer_profile::delete_all();
    c = viewer_profile::get_viewer_profile(duid);
    if(!c)
    {
        c = new viewer_profile(this);
        c->set_uid(duid);
        c->show();
        c->start_fetch();
    }
    else
    {
        c->show();
        c->raise();
        c->showNormal();
    }

}


