
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QtGui>
#include <QLabel>
#include <QMessageBox>
#include "ui_composer.h"
#include "composer.h"
#include "dlli.h"
#include "player.h"
#include "dwstr.h"
#include "vidsel.h"
#include "mainwin.h"
#include "simple_call.h"
#include "pfx.h"
#include "resizeimage.h"

//note: the ifdef ALLOW_MULTIPLE_CLIPS is intended to make the controls
// work in a way where a user could hit record-stop-record etc. multiple times
// for one message. this was implemented years ago in cdc32 using a simpler codec
// but noone really used the functionality (or at least noone complained when it
// went away.) getting it to work right while allowing users to change punchins
// on each clip made things complicated. the newer theora codec setup crashes when
// trying to play something with multiple clips, so i just disabled it here since
// it isn't worth the time to debug it.

QList<DVP> composer::Composers;
DwOString dwyco_get_attr(DWYCO_LIST l, int row, const char *col);
extern int Block_DLL;
extern DwOString TheManUID;
extern int HasCamera;
extern int HasAudioInput;
extern int HasAudioOutput;
extern VidSel *DevSelectBox;
extern DwOString ZeroUID;
extern DwOString My_uid;

static int
dwyco_get_attr(DWYCO_LIST l, int row, const char *col, DwOString& str_out)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_STRING)
        return 0;
    str_out = DwOString(val, 0, len);
    return 1;
}
// composer style is:
// 0 - regular
// 1 - file
// 2 - forward
// 3 - special
// 4 - autoreply
composer::composer(int style, int special_type, QWidget *parent, Qt::WindowFlags f)
    : QMainWindow(parent, f), vp(this)
{
    ui.setupUi(this);
    ui.progressBar->hide();

    ui.actionAccept_pal_request->setVisible(0);
    ui.actionReject_pal_request->setVisible(0);
    // "record picture" is only used in the profile composer
    // to attach a pic to a profile. might make sense to put it on
    // the normal composer and make it possible to send pics
    // that way, but for now, keep it simple.
    ui.actionRecord_Picture->setVisible(0);
    ui.groupBox->setVisible(0);
    switch(style)
    {
    case COMP_STYLE_FILE:
        cdcxpanic("oops");
        break;
    case COMP_STYLE_REGULAR:
    case COMP_STYLE_FORWARD: // note: for forwards, the subclass gets the right kind of composer set up
    case COMP_STYLE_AUTOREPLY:
        compid = dwyco_make_zap_composition(0);
        break;
    case COMP_STYLE_SPECIAL:
        cdcxpanic("bad style");
#if 0
        // for now, the only special types are for pal authorization
        // and the interface is a little messed up if we really wanted to
        // allow a full msg to be sent with the special type (we really need
        // to allow the type to be specified at send time instead of
        // create time.)
        // dunno if this might not be better in a subclass...
        compid = dwyco_make_special_zap_composition(special_type, 0, 0, 0);
        if(special_type == DWYCO_PAL_AUTH_REJECT ||
                special_type == DWYCO_PAL_AUTH_ACCEPT)
        {
            ui.actionAccept_pal_request->setVisible(1);
            ui.actionReject_pal_request->setVisible(1);
            ui.actionSend_Message->setVisible(0);
            ui.menubar->clear();
            ui.menubar->addMenu(ui.menuSend);
            ui.toolBar_2->hide();
        }
#endif
        break;
    default:
        cdcxpanic("bad composer style");
    }

    this->special_type = special_type;
    ui_id = -1;
    Composers.append(vp);
    has_attachment = 0;
    ui.recipients->hide();
    ui.select_recipients_text->hide();
    ui.textBrowser->hide();
    const char *val;
    int len;
    int type;
    if(dwyco_get_setting("zap/no_forward_default", &val, &len, &type))
    {
        if(type == DWYCO_TYPE_STRING || type == DWYCO_TYPE_INT)
        {
            ui.actionDon_t_allow_forwarding->setChecked(atoi(val));
        }
    }
    multi_recipient = 0;
    failed = 0;
    send_in_progress = 0;
    modified = 0;
    recording = 0;
    recording_audio = 0;
    chan_id = -1;
    connect(ui.textEdit, SIGNAL(textChanged()), this, SLOT(text_changed()));
    connect(ui.p_handle, SIGNAL(textEdited(QString)), this, SLOT(line_text_changed(QString)));
    connect(ui.p_email, SIGNAL(textEdited(QString)), this, SLOT(line_text_changed(QString)));
    connect(ui.p_loc, SIGNAL(textEdited(QString)), this, SLOT(line_text_changed(QString)));
    setAttribute(Qt::WA_QuitOnClose, 0);
    setAttribute(Qt::WA_DeleteOnClose);


    if(!HasCamera)
    {
        ui.actionRecord_Video->setChecked(0);
        ui.actionRecord_Video->setEnabled(0);
        ui.actionHi_Quality_Video->setChecked(0);
        ui.actionHi_Quality_Video->setEnabled(0);
        //ui.actionRecord_Picture->setEnabled(0);
        ui.label->hide();
    }
    if(!HasAudioInput)
    {
        ui.actionRecord_Audio->setChecked(0);
        ui.actionRecord_Audio->setEnabled(0);
    }
    else
    {
#if 1
        connect(Mainwinform, SIGNAL(audio_recording(int, int)), this, SLOT(audio_recording_event(int, int)));
        connect(this, SIGNAL(audio_recording(int, int)), Mainwinform, SIGNAL(audio_recording(int, int)));

#endif
    }
    if(!HasCamera && !HasAudioInput)
    {
        ui.actionRecord->setEnabled(0);
        ui.actionPlay->setEnabled(0);
        ui.actionStop->setEnabled(0);
        ui.actionStart_over->setEnabled(0);
    }
    if(DevSelectBox)
        connect(DevSelectBox, SIGNAL(camera_change(int)), this, SLOT(camera_event(int)));

    QLabel *bl = new QLabel;
    bl->setPixmap(QPixmap(":/new/green24/icons/PrimaryCons_Green_KenSaunders/PNGs/24x24/Eye-24x24.png"));
    ui.statusbar->addPermanentWidget(bl);
    bl->hide();
    blinker = bl;
    blink = 0;
    connect(&animation_timer, SIGNAL(timeout()), this, SLOT(animate()));
    animation_timer.start(500);

    connect(this, SIGNAL(ignore_event(DwOString)), Mainwinform, SIGNAL(ignore_event(DwOString)));
    connect(this, SIGNAL(pal_event(DwOString)), Mainwinform, SIGNAL(pal_event(DwOString)));
    connect(Mainwinform, SIGNAL(uid_info_event(DwOString)), this, SLOT(update_recipient_decorations(DwOString)));
    connect(this, SIGNAL(send_msg_event(DwOString)), Mainwinform, SIGNAL(send_msg_event(DwOString)));
    connect(Mainwinform, SIGNAL(msg_send_status(int,DwOString,DwOString)), this, SLOT(pers_msg_send_status(int,DwOString,DwOString)));
    connect(Mainwinform, SIGNAL(msg_progress(DwOString,DwOString,DwOString,int)), this, SLOT(msg_progress(DwOString,DwOString,DwOString,int)));
    // only show these in some profile situations, since starting a new composer
    // or ignoring from a message you are sending are kinda confusing.
    ui.actionCompose_Message->setVisible(0);
    ui.actionIgnore_user->setVisible(0);
    if(style == COMP_STYLE_REGULAR)
        dwyco_field_debug("composer-regular", 1);

}

void
composer::animate()
{
    if(!blink)
        blinker->hide();
    else
    {
        static int state;
        blinker->show();
        if(state)
        {
            blinker->setPixmap(QPixmap(":/new/green24/icons/PrimaryCons_Green_KenSaunders/PNGs/24x24/blank-24x24.png"));
        }
        else
        {
            blinker->setPixmap(QPixmap(":/new/green24/icons/PrimaryCons_Green_KenSaunders/PNGs/24x24/arrow right-24x24.png"));
        }
        state = !state;
    }
}

composer::~composer()
{
    dwyco_delete_zap_composition(compid);
    int i;
    if((i = Composers.indexOf(vp)) == -1)
        cdcxpanic("bad composer");
    Composers.removeAt(i);
    ui.actionHi_Quality_Video->setChecked(0);
    vp.invalidate();
    delete blinker;
}

void
composer::audio_recording_event(int chan_id, int)
{
    if(chan_id != this->chan_id && recording)
    {
        on_actionStop_triggered(1);
    }
}

void
composer::update_recipient_decorations(DwOString uid)
{
    if(uid != this->uid)
        return;
    QString t("To: ");
    QString nm = dwyco_info_to_display(uid);
    t += nm;
    setWindowTitle(t);
}

void
composer::set_uid(const DwOString& u)
{
    uid = u;
    update_recipient_decorations(u);
    if(uid == My_uid)
    {
        ui.p_email->setVisible(1);
        ui.p_email_label->setVisible(1);
    }
    else
    {
        ui.p_email->setVisible(0);
        ui.p_email_label->setVisible(0);
    }

#if 0
    if(special_type == DWYCO_PAL_AUTH_ACCEPT ||
            special_type == DWYCO_PAL_AUTH_REJECT)
    {
        ui.label->show();
        QString m = QString("User \"%1\" wants to add you to their pal list. Click \"Accept\" to allow them to do this. Click \"Reject\" to reject the request. Click \"Cancel\" to silently ignore the request (the request is silently rejected.)").arg(nm);
        ui.label->setText(m);
    }
#endif
}

void
composer::camera_event(int on)
{
    // hmmm, this is a bit dicey. need to decide what to do if the user
    // already has something recorded, should we clear it? maybe not.
    if(on)
    {
        ui.actionRecord_Video->setEnabled(1);
        ui.actionRecord_Video->setChecked(1);
        ui.actionHi_Quality_Video->setEnabled(1);
        //ui.actionRecord_Picture->setEnabled(1);
        ui.label->show();
        ui.actionRecord->setEnabled(1);
        ui.actionPlay->setEnabled(1);
        ui.actionStop->setEnabled(0);
        ui.actionStart_over->setEnabled(0);
    }
    else
    {
        ui.actionRecord_Video->setChecked(0);
        ui.actionRecord_Video->setEnabled(0);
        ui.actionHi_Quality_Video->setChecked(0);
        ui.actionHi_Quality_Video->setEnabled(0);
        //ui.actionRecord_Picture->setEnabled(1);
        ui.label->hide();
        ui.actionRecord->setEnabled(HasAudioInput);
        ui.actionPlay->setEnabled(0);
        ui.actionStop->setEnabled(0);
        ui.actionStart_over->setEnabled(0);
        if(recording)
        {
            dwyco_zap_stop(compid);
        }
        if(has_attachment)
        {
            ui.label->show();
            ui.actionRecord->setEnabled(0);
            ui.actionPlay->setEnabled(1);
            ui.actionStop->setEnabled(1);
            ui.actionStart_over->setEnabled(1);
        }
    }
}

void
composer::text_changed()
{
    modified = 1;
}

void
composer::line_text_changed(const QString&)
{
    modified = 1;
}

void
composer::on_actionDon_t_allow_forwarding_triggered(bool newstate)
{
}

void
composer::on_actionHi_Quality_Video_triggered(bool newstate)
{
}

static void
DWYCOCALLCONV
dwyco_record_status_callback(int /*id*/, const char *msg, int percent_done, void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    composer_profile *c = (composer_profile *)(void *)vp;
    c->ui.statusbar->showMessage(msg);
    if(c->has_attachment)
    {
        c->ui.progressBar->show();
        c->ui.progressBar->setValue(percent_done);
    }
}


/*static*/ void
DWYCOCALLCONV
composer::dwyco_record_done(int /*id*/, void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    composer *c = (composer *)(void *)vp;
    // note: old codecs allowed us to record multiple clips
    // into one messages. since we introducted theora for the
    // codec, that functionality is broken, so once you
    // record, you can only start over or play
#ifdef ALLOW_MULTIPLE_CLIPS
    if(HasCamera || HasAudioInput)
        c->ui.actionRecord->setEnabled(1);
    else
        c->ui.actionRecord->setEnabled(0);
#else
    c->ui.actionRecord->setEnabled(0);
#endif
    c->ui.actionStop->setEnabled(0);
    c->ui.actionPlay->setEnabled(1);
    c->ui.actionStart_over->setEnabled(1);
    c->recording = 0;

    c->ui.actionSend_Message->setEnabled(1);
    c->ui.statusbar->showMessage("");
}

void
composer::on_actionRecord_triggered(bool)
{
    if(!ui.actionRecord_Video->isChecked() &&
            !ui.actionRecord_Audio->isChecked() &&
            !ui.actionRecord_Picture->isChecked())
        return;
    //int hiq = ui.actionHi_Quality_Video->isChecked();
    if(ui.actionRecord_Picture->isChecked())
    {
        if(!dwyco_zap_record(compid,
                             0, 0, 1,
                             1,
                             dwyco_record_done, (void *)vp.cookie, &ui_id))
        {
            return;
        }
        ui.label->ui_id = ui_id;
    }
    else
    {
        if(!dwyco_zap_record2(compid,
                              ui.actionRecord_Video->isChecked(),
                              ui.actionRecord_Audio->isChecked(),
                              -1, (97 * 1024 * 1024),
                              1,
                              dwyco_record_status_callback, (void *)vp.cookie,
                              dwyco_record_done, (void *)vp.cookie, &ui_id))
        {
            return;
        }
        ui.label->ui_id = ui_id;
        if(ui.actionRecord_Audio->isChecked())
        {
            chan_id = dwyco_zap_composition_chan_id(compid);
            emit audio_recording(chan_id, 1);
        }
    }
    ui.actionRecord->setEnabled(0);
    ui.actionStop->setEnabled(1);
    ui.actionPlay->setEnabled(0);
    ui.actionStart_over->setEnabled(0);
    has_attachment = 1;
    recording = 1;
    ui.label->show();

    ui.actionSend_Message->setEnabled(0);
}

static void
DWYCOCALLCONV
dwyco_play_done(int /*id*/, void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    composer *c = (composer *)(void *)vp;
#ifdef ALLOW_MULTIPLE_CLIPS
    if(HasCamera || HasAudioInput)
        c->ui.actionRecord->setEnabled(1);
    else
        c->ui.actionRecord->setEnabled(0);
#else
    c->ui.actionRecord->setEnabled(0);
#endif
    c->ui.actionStop->setEnabled(0);
    c->ui.actionPlay->setEnabled(1);
    c->ui.actionStart_over->setEnabled(1);
}


void
composer::on_actionPlay_triggered(bool)
{
    if(!dwyco_zap_play(compid, dwyco_play_done, (void *)vp.cookie, &ui_id))
        return;
    ui.label->ui_id = ui_id;
    ui.actionRecord->setEnabled(0);
    ui.actionStop->setEnabled(1);
    ui.actionPlay->setEnabled(0);
    ui.actionStart_over->setEnabled(0);
}

void
composer::on_actionStop_triggered(bool)
{
    if(!dwyco_zap_stop(compid))
        return;
#ifdef ALLOW_MULTIPLE_CLIPS
    ui.actionRecord->setEnabled(1);
#else
    ui.actionRecord->setEnabled(0);
#endif
    ui.actionStop->setEnabled(0);
    ui.actionPlay->setEnabled(1);
    ui.actionStart_over->setEnabled(1);
}

void
composer::on_actionStart_over_triggered(bool)
{
    dwyco_delete_zap_composition(compid);
    compid = dwyco_make_zap_composition(0);
    has_attachment = 0;
    ui.label->clear();
    ui.actionStart_over->setEnabled(0);
    if(HasCamera || HasAudioInput)
        ui.actionRecord->setEnabled(1);
    else
        ui.actionRecord->setEnabled(0);
    ui.actionStop->setEnabled(0);
    ui.actionPlay->setEnabled(0);
    ui.actionRecord_Picture->setChecked(0);
}


static void
DWYCOCALLCONV
dwyco_msg_send_callback(int id, int what, const char *recip, int recip_len, const char *server_msg, void *arg)
{

    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    composer *c = (composer *)(void *)vp;
    switch(what)
    {
    case DWYCO_MSG_SEND_OK:
        c->ui.statusbar->showMessage("Send OK.");
        c->modified = 0;
        c->send_in_progress = 0;
        c->blink = 0;
        c->has_attachment = 0;
        c->close();
        break;
    case DWYCO_MSG_SEND_FAILED:
    case DWYCO_MSG_ATTACHMENT_FAILED:
    {
        // note: starting with 2.11 (ca 2015) messages are
        // by default q-ed and resent in the background on failure.
        c->ui.statusbar->showMessage("First try failed... will keep trying to send");
        //c->ui.actionSend_Message->setEnabled(1);

        c->modified = 0;
        c->send_in_progress = 0;
        c->blink = 0;
        c->has_attachment = 0;
        c->close();

        break;
    }
    case DWYCO_MSG_IGNORED:
    case DWYCO_MSG_ATTACHMENT_DECLINED:
        c->ui.statusbar->showMessage("Message declined by recipient.");
        // note: if you want a more aggressive msg, might want to
        // not close the box.
        c->modified = 0;
        c->send_in_progress = 0;
        c->has_attachment = 0;
        c->blink = 0;
        c->close();
        break;
    default:
        break;
    }
}

static void
DWYCOCALLCONV
dwyco_send_status_callback(int /*id*/, const char *msg, int percent_done, void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    composer *c = (composer *)(void *)vp;
    c->ui.statusbar->showMessage(msg);
    if(c->has_attachment)
    {
        c->ui.progressBar->show();
        c->ui.progressBar->setValue(percent_done);
    }
}

static void
DWYCOCALLCONV
dwyco_msg_send_callback_multiple(int /*id*/, int what, const char *recip, int recip_len, const char *server_msg, void *arg)
{
    DVP svp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!svp.is_valid())
        return;
    struct sender *s = (struct sender *)(void *)svp;
    if(!s->cvp.is_valid())
        return;
    composer *c = (composer *)(void *)s->cvp;

    int i;
    if((i = c->compids.indexOf(s->pers_id)) == -1)
    {
        return;
        cdcxpanic("bogus multi-send");
    }
    c->compids[i] = "";
    QListWidgetItem *it = c->ui.recipients->item(i);
    switch(what)
    {
    case DWYCO_MSG_SEND_OK:
    {
        c->ui.statusbar->showMessage("Send OK.");
        it->setCheckState(Qt::Unchecked);
        it->setForeground(QBrush("green"));
        QString txt = it->text();
        txt.prepend("+");
        it->setText(txt);
    }

        //c->close();
    break;
    case DWYCO_MSG_SEND_FAILED:
    case DWYCO_MSG_ATTACHMENT_FAILED:
    {

        //c->ui.statusbar->showMessage();
        it->setForeground(QBrush("red"));
        QString txt = it->text();
        txt.prepend("-");
        it->setText(txt);
        c->failed = 1;
        c->ui.actionSend_Message->setEnabled(1);
    }
    break;
    case DWYCO_MSG_IGNORED:
    case DWYCO_MSG_ATTACHMENT_DECLINED:
        c->ui.statusbar->showMessage("Message declined by recipient.");
        it->setCheckState(Qt::Unchecked);
        break;
    default:
        break;
    }
    // if all of them have completed, with no failures, close the box.
    int n = c->compids.count();
    for(i = 0; i < n; ++i)
    {
        if(c->compids[i].length() != 0)
            break;
    }
    if(i == n && !c->failed)
    {
        c->send_in_progress = 0;
        c->modified = 0;
        c->has_attachment = 0;
        c->close();
    }
    // if all the sends have completed, but some failed,
    // let the user know and allow them to retry
    if(i == n && c->failed)
    {
        c->ui.statusbar->showMessage("At least one msg failed to send, they will be sent in the background");
        c->blink = 0;
        c->send_in_progress = 0;
        c->modified = 0;
        c->has_attachment = 0;
        c->close();
    }
}

static void
DWYCOCALLCONV
dwyco_send_status_callback_multiple(int /*id*/, const char *msg, int percent_done, void *arg)
{
    DVP svp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!svp.is_valid())
        return;
    struct sender *s = (struct sender *)(void *)svp;
    if(!s->cvp.is_valid())
        return;
    composer *c = (composer *)(void *)s->cvp;
    int i;
    if((i = c->compids.indexOf(s->pers_id)) == -1)
    {
        return;
        cdcxpanic("bogus multi-send");
    }
    QListWidgetItem *it = c->ui.recipients->item(i);
    c->ui.statusbar->showMessage(msg);
    if(c->has_attachment)
    {
        //c->ui.progressBar->show();
        //c->ui.progressBar->setValue(percent_done);
        QString pd;
        pd.setNum(percent_done);
        QString res = QString("%1% %2").arg(pd, dwyco_info_to_display(s->uid));
        it->setText(res);
    }
}


int
composer::get_composition()
{
    // this is only called in multi-send for non-(file/forward)
    // case
    return dwyco_dup_zap_composition(compid);
}

void
composer::pers_msg_send_status(int status, DwOString pid, DwOString ruid)
{
    if(pid != pers_id)
        return;
    if(status == DWYCO_SE_MSG_SEND_START)
        return;
    dwyco_msg_send_callback(0, status == DWYCO_SE_MSG_SEND_FAIL ? DWYCO_MSG_SEND_FAILED : DWYCO_MSG_SEND_OK,
                            ruid.c_str(), ruid.length(), "", (void *)vp.cookie);
}

void
composer::msg_progress(DwOString pid, DwOString ruid, DwOString msg, int percent)
{
    if(pid != pers_id)
        return;
    dwyco_send_status_callback(0, msg.c_str(), percent, (void *)vp.cookie);
}

void
sender::pers_msg_send_status(int status, DwOString pid, DwOString ruid)
{
    if(pid != pers_id)
        return;
    if(status == DWYCO_SE_MSG_SEND_START)
        return;
    dwyco_msg_send_callback_multiple(0, status == DWYCO_SE_MSG_SEND_FAIL ? DWYCO_MSG_SEND_FAILED : DWYCO_MSG_SEND_OK,
                                     ruid.c_str(), ruid.length(), "", (void *)vp.cookie);
    deleteLater();

}

void
sender::msg_progress(DwOString pid, DwOString ruid, DwOString msg, int percent)
{
    if(pid != pers_id)
        return;
    dwyco_send_status_callback_multiple(0, msg.c_str(), percent, (void *)vp.cookie);
}

void
sender::stop_and_remove()
{
    dwyco_kill_message(pers_id.c_str(), pers_id.length());
}

void
composer::on_actionSend_Message_triggered(bool)
{
    QString a;
    QString b;
    b = ui.textEdit->toPlainText();

    int file_comp = !!dynamic_cast<composer_file *>(this);
    if(!multi_recipient)
    {
        switch_composition();

//		if(!dwyco_zap_send3(compid,
//			uid.c_str(), uid.length(),
//			b.toAscii().data(),
//			b.toAscii().length(),
//			file_comp ? 0 : ui.actionDon_t_allow_forwarding->isChecked(),
//			dwyco_msg_send_callback, (void *)vp.cookie,
//			dwyco_send_status_callback, (void *)vp.cookie))
//		{
//			QMessageBox::warning(this, "Can't send", "Message cannot be sent to the recipient.");
//			return;
//		}
        const char *pid;
        int len_pid;
        if(!dwyco_zap_send4(compid,
                            uid.c_str(), uid.length(),
                            b.toAscii().data(),
                            b.toAscii().length(),
                            file_comp ? 0 : ui.actionDon_t_allow_forwarding->isChecked(),
                            &pid, &len_pid))
        {
            QMessageBox::warning(this, "Can't send", "Message cannot be sent to the recipient.");
            return;
        }
        pers_id = DwOString(pid, 0, len_pid);
        emit send_msg_event(uid);
        if(uid == My_uid && has_attachment)
        {
            //QMessageBox::information(0, "self-send", "Message was directly filed locally under your own \"sent\" messages");
            modified = 0;
            has_attachment = 0;
            send_in_progress = 0;
            blink = 0;
            // this crashes things... because it immediately deletes
            //close();
            setVisible(0);
            deleteLater();
            return;
        }
        ui.actionRecord->setEnabled(0);
        ui.actionStop->setEnabled(0);
        ui.actionPlay->setEnabled(0);
        ui.actionStart_over->setEnabled(0);
        ui.actionSend_Message->setEnabled(0);
        send_in_progress = 1;
        blink = 1;
    }
    else
    {
        failed = 0;
        int n = ui.recipients->count();
        // turn off sorting because we are going to
        // put status info directly into the msg list
        // which will result in a new sort order, which
        // screws everything up.
        ui.recipients->setSortingEnabled(0);
        int nstart = 0;
        compids.clear();
        for(int i = 0; i < n; ++i)
            compids.append("");
        for(int i = 0; i < n; ++i)
        {
            compids[i] = "";
            QListWidgetItem *it = ui.recipients->item(i);
            if(it->checkState() != Qt::Checked)
                continue;
            int cid = get_composition();
            if(cid == 0)
                continue;
            QVariant quid = it->data(Qt::UserRole);
            DwOString duid(quid.toByteArray().constData(), 0, quid.toByteArray().count());
            // filter out our own uid in case we are on our own pal list
            if(duid == My_uid)
                continue;
            it->setForeground(QBrush("black"));
            struct sender *s = new struct sender;
            s->uid = duid;
            s->cvp = vp;
            // oops, apparently, zap_send3 can call the status callback before
            // it returns
            //compids[i] = cid;
            ++nstart;
//			if(!dwyco_zap_send3(cid,
//				duid.c_str(), duid.length(),
//				b.toAscii().data(),
//				b.toAscii().length(),
//				file_comp ? 0 : ui.actionDon_t_allow_forwarding->isChecked(),
//				dwyco_msg_send_callback_multiple, (void *)s->vp.cookie,
//				dwyco_send_status_callback_multiple, (void *)s->vp.cookie))
//			{
//				delete s;
//				compids[i] = 0;
//				--nstart;
//				continue;
//			}
            // only people already on your pal list are added to the multi-list
            // except for one
            const char *pid;
            int len_pid;
            if(!dwyco_zap_send4(cid,
                                duid.c_str(), duid.length(),
                                b.toAscii().data(),
                                b.toAscii().length(),
                                file_comp ? 0 : ui.actionDon_t_allow_forwarding->isChecked(),
                                &pid, &len_pid))
            {
                delete s;
                compids[i] = "";
                --nstart;
                continue;
            }
            s->pers_id = DwOString(pid, 0, len_pid);
            compids[i] = s->pers_id;
            connect(Mainwinform, SIGNAL(msg_progress(DwOString,DwOString,DwOString,int)), s, SLOT(msg_progress(DwOString,DwOString,DwOString,int)));
            connect(Mainwinform, SIGNAL(msg_send_status(int,DwOString,DwOString)), s, SLOT(pers_msg_send_status(int,DwOString,DwOString)));
            connect(this, SIGNAL(user_cancel()), s, SLOT(stop_and_remove()));
        }
        if(nstart == 0)
        {
            QMessageBox::warning(this, "Can't send", "Message cannot be sent to any of the recipients you selected.");
        }
        else
        {
            ui.actionRecord->setEnabled(0);
            ui.actionStop->setEnabled(0);
            ui.actionPlay->setEnabled(0);
            ui.actionStart_over->setEnabled(0);
            ui.actionSend_Message->setEnabled(0);
            send_in_progress = 1;
            blink = 1;
        }
    }
}

void
composer::on_actionSelect_triggered(bool)
{
    if(multi_recipient)
        return;
    DWYCO_LIST l = dwyco_pal_get_list();
    int n;
    dwyco_list_numelems(l, &n, 0);
    if(n == 0)
    {
        dwyco_list_release(l);
        return;
    }
    multi_recipient = 1;
    setWindowTitle("(Select recipients below)");

    for(int i = 0; i < n; ++i)
    {
        DwOString uid = dwyco_get_attr(l, i, DWYCO_NO_COLUMN);
        if(uid == My_uid)
            continue;
        QVariant quid(QByteArray(uid.c_str(), uid.length()));
        QListWidgetItem *item = new QListWidgetItem(dwyco_info_to_display(uid), ui.recipients);
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole, quid);
        ui.recipients->addItem(item);

    }
    dwyco_list_release(l);
    ui.recipients->show();
    ui.select_recipients_text->show();

}

void
composer::closeEvent(QCloseEvent *cev)
{
    dwyco_zap_stop(compid);
    if(send_in_progress)
    {
        QMessageBox m(this);
        m.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        m.setText("Stop sending and discard content?");
        m.setWindowTitle("Stop send?");
        Block_DLL = 1;
        int b = m.exec();
        Block_DLL = 0;
        if(b == (int)QMessageBox::No)
        {
            cev->ignore();
            return;
        }
        on_actionCancel_triggered(1);
        cev->accept();
        return;
    }
    if(modified || has_attachment)
    {
        //QMessageBox::StandardButton b = QMessageBox::question(this, "Discard?", "You have unsent text or media. Discard it?", QMessageBox::Yes|QMessageBox::No);
        QMessageBox m(this);
        m.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        m.setText("You have unsent text or media. Discard it?");
        m.setWindowTitle("Discard?");
        Block_DLL = 1;
        int b = m.exec();
        Block_DLL = 0;
        if(b == (int)QMessageBox::No)
        {
            cev->ignore();
            return;
        }
        dwyco_delete_zap_composition(compid);
        cev->accept();
        return;
    }
    cev->accept();
}

void
composer::on_actionCancel_triggered(bool)
{

    if(close())
    {
        if(dwyco_zap_cancel(compid))
        {
            compid = 0;
        }
        dwyco_kill_message(pers_id.c_str(), pers_id.length());

        int n = compids.count();
        for(int i = 0; i < n; ++i)
        {
            if(compids[i].length() > 0)
            {
                if(dwyco_kill_message(compids[i].c_str(), compids[i].length()))
                    compids[i] = "";
            }
        }

        send_in_progress = 0;
        blink = 0;
    }

}

void
composer::on_actionRecord_Video_triggered(bool new_state)
{
    if(new_state == 1 &&
            ui.actionRecord_Picture->isChecked())
    {
        ui.actionRecord_Picture->setChecked(0);
        on_actionStart_over_triggered(0);
    }
#ifndef ALLOW_MULTIPLE_CLIPS
    on_actionStart_over_triggered(0);
#endif
    if(/*ui.actionRecord_Picture->isChecked() ||*/
        ui.actionRecord_Video->isChecked() ||
        ui.actionRecord_Audio->isChecked())
    {
        ui.actionRecord->setEnabled(1);
    }
    else
    {
        ui.actionRecord->setEnabled(0);
    }
}

void
composer::on_actionRecord_Audio_triggered(bool new_state)
{
    if(new_state == 1 &&
            ui.actionRecord_Picture->isChecked())
    {
        ui.actionRecord_Picture->setChecked(0);
        on_actionStart_over_triggered(0);
    }
#ifndef ALLOW_MULTIPLE_CLIPS
    on_actionStart_over_triggered(0);
#endif
    if(/*ui.actionRecord_Picture->isChecked() ||*/
        ui.actionRecord_Video->isChecked() ||
        ui.actionRecord_Audio->isChecked())
    {
        ui.actionRecord->setEnabled(1);
    }
    else
    {
        ui.actionRecord->setEnabled(0);
    }
}

void
composer::on_actionRecord_Picture_triggered(bool new_state)
{
    if(new_state == 1)
    {
        ui.actionRecord_Video->setChecked(0);
        ui.actionRecord_Audio->setChecked(0);
        on_actionStart_over_triggered(0);
        ui.actionRecord_Picture->setChecked(1);
        QString fn = get_a_filename("Images (*.jpg *.jpeg *.png *.bmp *.gif *.ppm);;All files (*)");
        if(fn.length() > 0)
        {
            QString out_fn;
            if(!load_and_resize(fn, out_fn, 65000))
            {
                ui.statusbar->showMessage("Can't load image, either unknown type or it may be too big.");
                ui.actionRecord_Picture->setChecked(0);
                ui.actionStart_over->setEnabled(1);
            }
            else
            {
                QByteArray fn = out_fn.toLatin1();
                compid = dwyco_make_file_zap_composition(fn, fn.length());
                QPixmap pv(out_fn);
                ui.label->show();
                ui.label->clear();
                ui.label->setPixmap(pv);
                ui.actionStart_over->setEnabled(1);
                ui.statusbar->showMessage("Image loaded");
            }
        }
        else
        {
            ui.actionRecord_Picture->setChecked(0);
        }

    }
    else
    {
        image_fn = "";
        on_actionStart_over_triggered(0);
    }
    if(/*ui.actionRecord_Picture->isChecked() ||*/
        ui.actionRecord_Video->isChecked() ||
        ui.actionRecord_Audio->isChecked())
    {
        ui.actionRecord->setEnabled(1);
    }
    else
    {
        ui.actionRecord->setEnabled(0);
    }
}

void
composer::on_actionShow_triggered(bool new_state)
{
    // show the recipients of the message
}


// Profile composer
static
void
DWYCOCALLCONV
dwyco_set_profile_callback(int succ, const char *reason,
                           const char *s1, int len_s1,
                           const char *s2, int len_s2,
                           const char *s3, int len_s3,
                           const char *filename,
                           const char *uid, int len_uid,
                           int reviewed, int regular,
                           void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    composer_profile *c = (composer_profile *)(void *)vp;
    if(succ)
    {
        c->ui.statusbar->showMessage("Update successful");
        c->modified = 0;
        c->has_attachment = 0;
        c->send_in_progress = 0;
        c->blink = 0;
        c->close();
        return;
    }
    c->ui.statusbar->showMessage("Update failed. Try again later.");
    c->blink = 0;
    c->send_in_progress = 0;
}

//static
void
DWYCOCALLCONV
composer_profile::dwyco_profile_composer_fetch_done(int succ, const char *reason,
                                  const char *s1, int len_s1,
                                  const char *s2, int len_s2,
                                  const char *s3, int len_s3,
                                  const char *filename,
                                  const char *uid, int len_uid,
                                  int reviewed, int regular,
                                  void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    composer_profile *c = (composer_profile *)(void *)vp;
    if(succ == 0)
    {
        c->ui.statusbar->showMessage(QString("No profile available. (%1)").arg(reason));
        c->ui.textEdit->clear();
        c->ui.label->clear();
        c->ui.label->hide();
        c->resize(320, 240);
        return;
    }
    QString handle;
    QString desc;
    QString loc;
    QString email;
    dwyco_info_to_display(c->uid, handle, desc, loc, email);
    c->ui.textEdit->setHtml(desc);
    c->ui.p_handle->setText(handle);
    c->ui.p_loc->setText(loc);
    c->ui.p_email->setText(email);

    if(reviewed)
    {
        if(regular)
            c->ui.groupBox->setTitle("Profile (regular)");
        else
            c->ui.groupBox->setTitle("Profile");

    }
    else
    {
        c->ui.groupBox->setTitle("Profile (unreviewed)");
    }
    if(succ == -1)
    {
        c->ui.label->clear();
        c->ui.label->setVisible(0);
        c->ui.actionPlay->setEnabled(0);
        c->ui.actionStop->setEnabled(0);
        c->ui.statusbar->showMessage("Text only");
    }
    else if(succ == -2)
    {
        // profile is a jpg in the filename
        c->ui.label->setVisible(1);
        QImage img(filename);
        c->ui.label->setPixmap(QPixmap::fromImage(img));
        c->ui.actionPlay->setEnabled(0);
        c->ui.actionStop->setEnabled(0);
        c->ui.actionStart_over->setEnabled(1);
        c->ui.statusbar->showMessage("Image");
        int fcomp = dwyco_make_file_zap_composition(filename, strlen(filename));
        dwyco_delete_zap_composition(c->compid);
        c->compid = fcomp;

    }
    else
    {
        c->ui.label->setVisible(1);
        if(c->viewid != -1)
            dwyco_delete_zap_view(c->viewid);
        c->viewid = succ;
        int video = 0;
        int audio = 0;
        int short_video = 0;
        if(dwyco_zap_quick_stats_view(c->viewid, &video, &audio, &short_video))
        {
            // in cdc-x, pic-only profiles don't happen at this point.
#if 0
            if(short_video)
            {
                c->ui.actionPlay->setEnabled(0);
                c->ui.actionStop->setEnabled(0);
                c->ui.statusbar->showMessage("Pic only");
            }
            else
#endif
                if(video && !audio)
                {
                    c->ui.actionPlay->setEnabled(1);
                    c->ui.actionStop->setEnabled(0);
                    c->ui.actionStart_over->setEnabled(1);
                    c->ui.statusbar->showMessage("Video only (no audio)");
                }
                else if(!video && audio)
                {
                    c->ui.actionPlay->setEnabled(1);
                    c->ui.actionStop->setEnabled(0);
                    c->ui.actionStart_over->setEnabled(1);
                    c->ui.statusbar->showMessage("Audio only (no video)");
                    c->ui.label->clear();
                    c->ui.label->setText("(audio only)");
                }
                else if(!audio && !video)
                {
                    c->ui.actionPlay->setEnabled(0);
                    c->ui.actionStop->setEnabled(0);
                    c->ui.statusbar->showMessage("Text only");
                    c->ui.label->clear();
                }
                else
                {
                    c->ui.actionPlay->setEnabled(1);
                    c->ui.actionStop->setEnabled(0);
                    c->ui.actionStart_over->setEnabled(1);
                    c->ui.statusbar->showMessage("Audio+Video");
                }
        }
        dwyco_zap_play_preview(c->viewid, 0, 0, &c->ui_id);
        c->ui.label->ui_id = c->ui_id;
        dwyco_delete_zap_composition(c->compid);
        c->compid = c->viewid;
    }
    c->modified = 0;
}

// camera events affect a profile composer a little different
// because of the case with existing media (including a pic)
void
composer_profile::camera_event(int on)
{

    if(on)
    {
        ui.actionRecord_Video->setEnabled(1);
        ui.actionRecord_Video->setChecked(1);
        //ui.actionHi_Quality_Video->setEnabled(1);
        //ui.actionRecord_Picture->setEnabled(1);
        ui.label->show();
        ui.actionRecord->setEnabled(1);
        ui.actionPlay->setEnabled(1);
        ui.actionStop->setEnabled(0);
        ui.actionStart_over->setEnabled(0);
        on_actionStart_over_triggered(0);
    }
    else
    {
        ui.actionRecord_Video->setChecked(0);
        ui.actionRecord_Video->setEnabled(0);
        ui.actionHi_Quality_Video->setChecked(0);
        ui.actionHi_Quality_Video->setEnabled(0);
        //ui.actionRecord_Picture->setEnabled(1);
        //ui.label->hide();
        ui.actionRecord->setEnabled(HasAudioInput);
        ui.actionPlay->setEnabled(0);
        ui.actionStop->setEnabled(0);
        ui.actionStart_over->setEnabled(0);
        if(recording)
        {
            dwyco_zap_stop(compid);
        }

        ui.label->show();
        ui.actionRecord->setEnabled(0);
        ui.actionPlay->setEnabled(1);
        ui.actionStop->setEnabled(1);
        ui.actionStart_over->setEnabled(1);

    }
}

void
composer_profile::on_actionSend_Message_triggered(bool)
{
    QString a;
    QString desc;
    desc = ui.textEdit->toPlainText();
    QString handle = ui.p_handle->text().simplified();
    QString loc = ui.p_loc->text().simplified();
    QString email = ui.p_email->text().simplified();

    const char *pack;
    int len_pack;

    dwyco_make_profile_pack(
        handle.toAscii().constData(), handle.toAscii().count(),
        desc.toAscii().constData(), desc.toAscii().count(),
        loc.toAscii().constData(), loc.toAscii().count(),
        email.toAscii().constData(), email.toAscii().count(),
        &pack, &len_pack);

    if(!dwyco_set_profile_from_composer(compid, pack, len_pack,
                                        dwyco_set_profile_callback, (void*)vp.cookie))
    {
        ui.statusbar->showMessage("Can't update profile, maybe it is too big.");
        return;
    }

    ui.statusbar->showMessage("Sending profile...");

    ui.actionRecord->setEnabled(0);
    ui.actionStop->setEnabled(0);
    ui.actionPlay->setEnabled(0);
    ui.actionStart_over->setEnabled(0);
    ui.actionSend_Message->setEnabled(0);
    send_in_progress = 1;
    blink = 1;
}

composer_profile::composer_profile(QWidget *parent, Qt::WindowFlags f) :
    composer(0, 0, parent, f)
{
    ui.menubar->clear();
    ui.menubar->addMenu(ui.menuVideo);
    ui.menubar->addMenu(ui.menuOptions);
    ui.actionRecord_Picture->setVisible(1);
    ui.actionDon_t_allow_forwarding->setVisible(0);
    ui.actionHi_Quality_Video->setVisible(0);
    ui.actionSelect->setVisible(0);
    ui.actionSend_Message->setText("Update profile");
    ui.actionCompose_Message->setVisible(0);
    ui.actionShow_Chatbox->setVisible(0);
    ui.actionIgnore_user->setVisible(0);
    ui.actionCancel->setText("Cancel");
    ui.groupBox->setVisible(1);
    QString handle;
    QString desc;
    QString loc;
    QString email;
    // note: you only ever compose for your own uid
    uid = My_uid;
    dwyco_info_to_display(uid, handle, desc, loc, email);
    ui.textEdit->setHtml(desc);
    ui.p_handle->setText(handle);
    ui.p_loc->setText(loc);
    ui.p_email->setText(email);

    setWindowTitle("Profile Composer");
    modified = 0;
    viewid = -1;

}

composer_profile *
composer_profile::get_composer_profile(const DwOString& uid)
{
    int n = Composers.count();
    for(int i = 0; i < n; ++i)
    {
        if(!Composers[i].is_valid())
            continue;
        composer_profile *p = dynamic_cast<composer_profile *>((composer *)(void *)Composers[i]);
        if(!p)
            continue;
        if(uid == ZeroUID || p->uid == uid)
            return p;
    }
    return 0;
}

void
composer_profile::start()
{
    ui.statusbar->showMessage("Fetching previous profile...");
    ui.label->clear();
    ui.actionPlay->setEnabled(0);
    ui.actionStop->setEnabled(0);
    dwyco_get_profile_to_viewer(uid.c_str(), uid.length(),
                                dwyco_profile_composer_fetch_done, (void *)vp.cookie);

}

void
composer_profile::closeEvent(QCloseEvent *ev)
{
    dwyco_delete_zap_view(viewid);
    dwyco_delete_zap_composition(compid);
    composer::closeEvent(ev);
}

void
DWYCOCALLCONV
composer_profile::dwyco_record_profile_done(int /*id*/, void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    composer *c = (composer *)(void *)vp;
    c->ui.actionRecord->setEnabled(0);
    c->ui.actionStop->setEnabled(0);
    c->ui.actionPlay->setEnabled(1);
    c->ui.actionStart_over->setEnabled(1);
    c->recording = 0;
    c->ui.actionSend_Message->setEnabled(1);
}

void
composer_profile::on_actionRecord_triggered(bool)
{
    if(!dwyco_zap_record2(compid,
                          ui.actionRecord_Video->isChecked(),
                          ui.actionRecord_Audio->isChecked(),
                          -1, 60000, 0,
                          dwyco_record_status_callback, (void *)vp.cookie,
                          dwyco_record_profile_done, (void *)vp.cookie, &ui_id))
    {
        return;
    }
    ui.label->ui_id = ui_id;
    if(ui.actionRecord_Audio->isChecked())
    {
        chan_id = dwyco_zap_composition_chan_id(compid);
        emit audio_recording(chan_id, 1);
    }
    ui.actionRecord->setEnabled(0);
    ui.actionStop->setEnabled(1);
    ui.actionPlay->setEnabled(0);
    ui.actionStart_over->setEnabled(0);
    ui.actionRecord_Picture->setChecked(0);
    recording = 1;
    has_attachment = 1;
    ui.label->show();

    ui.actionSend_Message->setEnabled(0);
}

void
composer_profile::on_actionStart_over_triggered(bool a)
{
    composer::on_actionStart_over_triggered(a);
    modified = 1;
}

//
// a viewer that you can use just to playback
// a profile.
//

viewer_profile::viewer_profile(QWidget *parent, Qt::WindowFlags f) :
    composer(0, 0, parent, f)
{
    ui.menubar->clear();
    ui.menubar->addMenu(ui.menuVideo);
    ui.menubar->addMenu(ui.menuZap);
    ui.actionCompose_Message->setVisible(1);
    ui.actionIgnore_user->setVisible(1);
    ui.actionSend_Message->setVisible(0);
    ui.actionCancel->setVisible(1);
    ui.actionCancel->setEnabled(1);
    ui.actionCancel->setText("&Close");
    ui.label_3->hide();
    ui.actionRecord->setVisible(0);
    ui.actionStart_over->setVisible(0);
    ui.actionRecord_Video->setVisible(0);
    ui.actionRecord_Audio->setVisible(0);
    ui.actionRecord_Picture->setVisible(0);
    ui.actionDon_t_allow_forwarding->setVisible(0);
    ui.actionHi_Quality_Video->setVisible(0);
    ui.label->show();
    ui.groupBox->setVisible(1);
    viewid = -1;
    connect(Mainwinform, SIGNAL(invalidate_profile(DwOString)), this, SLOT(update_profile(DwOString)));
    dwyco_field_debug("viewer-profile", 1);
}

void
viewer_profile::update_profile(DwOString uid)
{
    if(uid == this->uid)
        start_fetch();
}

viewer_profile *
viewer_profile::get_viewer_profile(const DwOString& uid)
{
    int n = Composers.count();
    for(int i = 0; i < n; ++i)
    {
        if(!Composers[i].is_valid())
            continue;
        viewer_profile *p = dynamic_cast<viewer_profile *>((composer *)(void *)Composers[i]);
        if(!p)
            continue;
        if(uid == ZeroUID || p->uid == uid)
            return p;
    }
    return 0;
}

void
viewer_profile::delete_all()
{
    viewer_profile *p;
    while((p = get_viewer_profile(ZeroUID)))
    {
        delete p;
    }
}

//static
void
DWYCOCALLCONV
dwyco_profile_fetch_done(int succ, const char *reason,
                         const char *s1, int len_s1,
                         const char *s2, int len_s2,
                         const char *s3, int len_s3,
                         const char *filename,
                         const char *uid, int len_uid,
                         int reviewed, int regular,
                         void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    viewer_profile *c = (viewer_profile *)(void *)vp;
    c->raise();
    if(succ == 0)
    {
        c->ui.statusbar->showMessage(QString("No profile available. (%1)").arg(reason));
        c->ui.textEdit->clear();
        c->ui.label->hide();
        c->resize(320, 240);
        return;
    }
    QString handle;
    QString desc;
    QString loc;
    QString email;
    dwyco_info_to_display(c->uid, handle, desc, loc, email);
    c->ui.textEdit->setHtml(desc);
    c->ui.p_handle->setText(handle);
    c->ui.p_loc->setText(loc);
    c->ui.p_email->setText(email);
    c->ui.p_handle->setReadOnly(1);
    c->ui.p_loc->setReadOnly(1);
    c->ui.p_email->setReadOnly(1);
    c->ui.textEdit->setReadOnly(1);
    if(reviewed)
    {
        if(regular)
            c->ui.groupBox->setTitle("Profile (regular)");
        else
            c->ui.groupBox->setTitle("Profile");

    }
    else
    {
        c->ui.groupBox->setTitle("Profile (unreviewed)");
    }
    if(succ == -1)
    {
        //QString txt = get_extended(s1);
        c->ui.label->clear();
        c->ui.label->setVisible(0);
        c->ui.actionPlay->setEnabled(0);
        c->ui.actionStop->setEnabled(0);
        c->ui.statusbar->showMessage("Text only");
    }
    else if(succ == -2)
    {
        // profile is a jpg in the filename
        c->ui.label->setVisible(1);
        QImage img(filename);
        c->ui.label->setPixmap(QPixmap::fromImage(img));
        c->ui.statusbar->showMessage("Picture");
    }
    else
    {
        //QString txt = get_extended(s1);
        //c->ui.textEdit->setHtml(txt);
        c->ui.label->setVisible(1);
        if(c->viewid != -1)
            dwyco_delete_zap_view(c->viewid);
        c->viewid = succ;
        int video = 0;
        int audio = 0;
        int short_video = 0;
        if(dwyco_zap_quick_stats_view(c->viewid, &video, &audio, &short_video))
        {
            // in cdc-x, pic-only profiles don't happen at this point.
#if 0
            if(short_video)
            {
                c->ui.actionPlay->setEnabled(0);
                c->ui.actionStop->setEnabled(0);
                c->ui.statusbar->showMessage("Pic only");
            }
            else
#endif
                if(video && !audio)
                {
                    c->ui.actionPlay->setEnabled(1);
                    c->ui.actionStop->setEnabled(0);
                    c->ui.statusbar->showMessage("Video only (no audio)");
                }
                else if(!video && audio)
                {
                    c->ui.actionPlay->setEnabled(1);
                    c->ui.actionStop->setEnabled(0);
                    c->ui.statusbar->showMessage("Audio only (no video)");
                    c->ui.label->clear();
                    c->ui.label->setText("(audio only)");
                }
                else if(!audio && !video)
                {
                    c->ui.actionPlay->setEnabled(0);
                    c->ui.actionStop->setEnabled(0);
                    c->ui.statusbar->showMessage("Text only");
                    c->ui.label->clear();
                }
                else
                {
                    c->ui.actionPlay->setEnabled(1);
                    c->ui.actionStop->setEnabled(0);
                    c->ui.statusbar->showMessage("Audio+Video");
                }
        }
        dwyco_zap_play_preview(c->viewid, 0, 0, &c->ui_id);
        c->ui.label->ui_id = c->ui_id;
    }
}

void
viewer_profile::closeEvent(QCloseEvent *ev)
{
    dwyco_delete_zap_view(viewid);
    ev->accept();
}

int
viewer_profile::start_fetch()
{
    ui.statusbar->showMessage("Fetching profile...");
    ui.label->clear();
    ui.actionPlay->setEnabled(0);
    ui.actionStop->setEnabled(0);
    ui.textEdit->clear();
    QString a("Profile: ");
    a += dwyco_info_to_display(uid);
    setWindowTitle(a);
    return dwyco_get_profile_to_viewer(uid.c_str(), uid.length(), dwyco_profile_fetch_done, (void *)vp.cookie);
}

void
viewer_profile::on_actionPlay_triggered(bool)
{
    if(!dwyco_zap_play_view(viewid, dwyco_play_done, (void *)vp.cookie, &ui_id))
        return;
    ui.label->ui_id = ui_id;
    ui.actionStop->setEnabled(1);
    ui.actionPlay->setEnabled(0);
}

void
viewer_profile::on_actionStop_triggered(bool)
{
    if(!dwyco_zap_stop_view(viewid))
        return;
    ui.actionStop->setEnabled(0);
    ui.actionPlay->setEnabled(1);
}

void
viewer_profile::camera_event(int)
{
    // whether or not you have a cam should have no
    // effect on a viewer.
}


// ----- file composer --------

composer_file::composer_file(QString fn, QWidget *par, Qt::WindowFlags f) :
    composer(0, 0, par, f)
{
    ui.menubar->clear();
    ui.menubar->addMenu(ui.menuRecipients);
    ui.menubar->addMenu(ui.menuSend);
    ui.toolBar_2->hide();
    ui.label->show();
    ui.label->clear();
    // note: if they are sending a file, we can't control the forwarding
    // because the user can save the file outside cdc-x
    ui.actionDon_t_allow_forwarding->setChecked(0);
    ui.actionDon_t_allow_forwarding->setVisible(0);
    QString m("File to be sent: ");
    m += fn;
    ui.label->setText(m);
    filename = fn;
    has_attachment = 1;
    QPixmap q(fn);
    if(!q.isNull())
    {
        ui.label->clear();
        ui.label->setFixedSize(320,240);
        ui.label->setPixmap(q);
    }
    dwyco_field_debug("file-composer", 1);
}

int
composer_file::get_composition()
{
    int cid = dwyco_make_file_zap_composition(filename.toAscii().constData(), filename.toAscii().length());
    return cid;
}

void
composer_file::switch_composition()
{
    dwyco_delete_zap_composition(compid);
    compid = get_composition();
}

void
composer_file::camera_event(int)
{
    // whether or not you have a cam should have no
    // effect on a viewer.
}

// -------- forward a msg ---------

composer_forward::composer_forward(QWidget *par, Qt::WindowFlags f) :
    composer(COMP_STYLE_FORWARD, 0, par, f)
{
    ui.menubar->clear();
    //ui.menubar->addMenu(ui.menuRecipients);
    ui.menubar->addMenu(ui.menuSend);
    ui.menubar->addMenu(ui.menuOptions);
    ui.actionHi_Quality_Video->setVisible(0);
    ui.actionRecord_Video->setVisible(0);
    ui.actionRecord_Audio->setVisible(0);
    ui.actionRecord_Picture->setVisible(0);
    ui.actionStart_over->setVisible(0);
    ui.actionRecord->setVisible(0);
    ui.textBrowser->show();
    ui.actionCompose_Message->setVisible(0);
    ui.actionShow_Chatbox->setVisible(0);
    ui.actionIgnore_user->setVisible(0);
    failed = 0;
    dwyco_field_debug("forward-composer", 1);
}

void
preview_file_zap(DWYCO_SAVED_MSG_LIST sm, vidlab *lab, DwOString filename, QByteArray uid, DwOString mid, int no_resize)
{
#if 0
    // XXX should send it to a "fnmod"-ed tmp directory so we can safely obliterate it
    // XXX should also check size so we aren't previewing gigantic things that aren't
    // XXX images
    if(!dwyco_copy_out_file_zap(uid.constData(), uid.length(), mid.toAscii().constData(), user_fn.c_str()))
        return;
    QPixmap q(user_fn.c_str());
    if(q.isNull())
        return;
    lab->clear();
    if(!no_resize)
        lab->setFixedSize(320,240);
    lab->setPixmap(q);
#endif

    // chop everything off except the extension, then add our random name on the
    // front of it. avoids problems with goofy filenames being sent in.
#if 0
    int idx = filename.rfind(".");
    if(idx == DwOString::npos)
        return;

    filename.erase(0, idx);
#endif
    DwOString rfn;
    rfn = random_fn();
    rfn += ".itm";
    rfn = add_pfx(Tmp_pfx, rfn);
    // copy file out to random filename, scaling to preview size, then emitting the
    // html.
    if(!dwyco_copy_out_file_zap2(mid.c_str(), rfn.c_str()))
        return;
    QPixmap q(rfn.c_str());
    if(q.isNull())
    {
        // prevent crumbs, if we can't preview it, remove it
        QFile::remove(rfn);
        return;
    }
    QFile::remove(rfn);
    q.scaled(320, 240, Qt::KeepAspectRatio);
    lab->clear();
#if 0
    if(!no_resize)
        lab->setFixedSize(320,240);
#endif
    lab->setPixmap(q);
}

int
composer_forward::init(QByteArray uid_prev, DwOString mid)
{
    dwyco_delete_zap_composition(compid);
    compid = 0;
    uid_of_previous_sender = uid_prev;
    mid_to_forward = mid;
    failed = 0;
    // even tho it might be a single sender, we do multi-send
    // because the user has to select who to send it to.
    multi_recipient = 1;
    // we need this in order to play the forwarded message
    compid = dwyco_make_forward_zap_composition2(mid.c_str(), 1);
    if(compid == 0)
        return 0;
    DWYCO_LIST l = dwyco_pal_get_list();
    int n;
    dwyco_list_numelems(l, &n, 0);
    int flim = dwyco_flim(compid);

    int the_man_added = 0;
    QList<QByteArray> added;
    for(int i = 0; i < n; ++i)
    {
        DwOString uid = dwyco_get_attr(l, i, DWYCO_NO_COLUMN);
        if(flim && !dwyco_uid_g(uid.c_str(), uid.length()))
            continue;
        // don't allow forwarding to ourselves for now
        if(uid == My_uid)
            continue;
        QByteArray buid(uid.c_str(), uid.length());
        added.append(buid);
        QVariant quid(buid);
        if(uid == TheManUID)
            the_man_added = 1;
        QListWidgetItem *item = new QListWidgetItem(dwyco_info_to_display(uid), ui.recipients);
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole, quid);
        ui.recipients->addItem(item);

    }

// always load in dwyco, for admin purposes
    if(!the_man_added)
    {
        QByteArray buid(TheManUID.c_str(), TheManUID.length());
        added.append(buid);
        QVariant quid(buid);
        QListWidgetItem *item = new QListWidgetItem("Dwyco", ui.recipients);
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole, quid);
        ui.recipients->insertItem(0, item);
    }

    {
        // load in any other gods that are online
        QList<godrec> g = Gods.values();
        for(int i = 0; i <g.count(); ++i)
        {
            if(g[i].uid == My_uid)
                continue;

            if(!added.contains(g[i].uid) && g[i].god)
            {
                added.append(g[i].uid);
                QVariant quid(g[i].uid);
                QListWidgetItem *item = new QListWidgetItem(g[i].name, ui.recipients);
                item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
                item->setCheckState(Qt::Unchecked);
                item->setData(Qt::UserRole, quid);
                ui.recipients->addItem(item);
            }
        }
    }

    dwyco_list_release(l);
    ui.recipients->show();
    ui.select_recipients_text->show();
    DWYCO_SAVED_MSG_LIST sm;
    if(!dwyco_get_saved_message(&sm, uid_prev.constData(), uid_prev.length(), mid.c_str()))
        return 0;
    DWYCO_LIST bt = dwyco_get_body_text(sm);
    DwOString txt;
    if(dwyco_get_attr(bt, 0, DWYCO_NO_COLUMN, txt))
    {
        DwOString att;
        DwOString fn;
        int is_file = dwyco_get_attr(sm, 0, DWYCO_QM_BODY_FILE_ATTACHMENT, fn);
        if(!is_file)
        {
            if(dwyco_get_attr(sm, 0, DWYCO_QM_BODY_ATTACHMENT, att))
            {
                ui.label->show();
                //ui.label->setFixedSize(320,240);
                ui.actionPlay->setEnabled(1);
                has_attachment = 1;
                dwyco_zap_play_preview(compid, 0, 0, &ui_id);
                ui.label->ui_id = ui_id;
            }
            else
            {
                ui.label->hide();
            }
        }
        else
        {
            // XXX put file specific stuff up here
            has_attachment = 1;
            ui.label->show();
            ui.label->clear();
            ui.label->setText(fn.c_str());
            preview_file_zap(sm, ui.label, fn, uid_prev, mid);
            // note: if they are sending a file, we can't control the forwarding
            // because the user can save the file outside cdc-x
            ui.actionDon_t_allow_forwarding->setChecked(0);
            ui.actionDon_t_allow_forwarding->setVisible(0);

        }
        ui.textBrowser->setPlainText(txt.c_str());
    }
    dwyco_list_release(bt);
    dwyco_list_release(sm);

    return 1;
}
void
composer_forward::camera_event(int)
{
    // whether or not you have a cam should have no
    // effect on a viewer.
}


int
composer_forward::get_composition()
{
    int cid = dwyco_make_forward_zap_composition2(mid_to_forward.c_str(), 1);
    return cid;
}

void
composer_forward::switch_composition()
{
    dwyco_delete_zap_composition(compid);
    compid = get_composition();

}

#if 0
//----- AUTOREPLY -------
// disable most of the "recipient" stuff
composer_autoreply::composer_autoreply(QWidget *par, Qt::WindowFlags f) :
    composer(0, 0, par, f)
{
    ui.menubar->clear();
    ui.menubar->addMenu(ui.menuVideo);
    ui.menubar->addMenu(ui.menuOptions);
    //ui.actionRecord_Picture->setVisible(0);
    ui.actionDon_t_allow_forwarding->setVisible(0);
    //ui.actionHi_Quality_Video->setVisible(0);
    ui.actionSelect->setVisible(0);
    ui.actionSend_Message->setText("Set AutoReply Msg");
    ui.actionCompose_Message->setVisible(0);
    ui.actionShow_Chatbox->setVisible(0);
    ui.actionIgnore_user->setVisible(0);
    setWindowTitle("AutoReply Composer");
}

void
composer_autoreply::on_actionSend_Message_triggered(bool)
{
#if 0
    QString a;
    QString b;
    b = ui.textEdit->toPlainText();
    if(!dwyco_set_auto_reply_msg(b.toAscii().data(),
                                 b.toAscii().count(), compid))
    {
        ui.statusbar->showMessage("Can't update autoreply, maybe some of the files are read-only?.");
        return;
    }
    ui.statusbar->showMessage("Success");
    QMessageBox::information(this, "Autoreply updated successfully", "Success");
    modified = 0;
    has_attachment = 0;
    close();
#endif
}
#endif

void composer::on_actionShow_Chatbox_triggered()
{
    simple_call *c = simple_call::get_simple_call(uid);
    c->show();
    c->raise();
    Mainwinform->organize_chatboxes(c);
    close();
}

void composer::on_actionCompose_Message_triggered()
{
    composer *c = new composer(COMP_STYLE_REGULAR, 0, 0);
    c->set_uid(uid);
    c->show();
    c->raise();
    close();

}

void composer::on_actionIgnore_user_triggered()
{
    if(confirm_ignore())
        dwyco_ignore(uid.c_str(), uid.length());
    emit ignore_event(uid);
    close();
}
