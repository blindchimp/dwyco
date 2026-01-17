
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// This is a simplified calling interface for cdc-x.
// The idea is to simplify the most common type of calling and
// replace the more general (but more difficult to use) old style of calling
// where the user would decide what media to use before calling.
// This interface assumes there is exactly one "call" going between two
// different uid's. One is designated as the master, and the other is the slave,
// and the call setup is controlled by the master.
// The master object is created when a user requests a call.
// The slave object is created when the call is accepted (which means that the
// the target of the call must screen the call before creating the slave object.)
//
// This object more or less assumes it has control of everything on the
// client, so it is like a "mode". In other words, all media is directed towards
// the target of the call, and is not reflected to any other recipient (eg, no
// audio to chat room, to a conference, etc.etc.)
//
//

// call setup states:
//
// initial control channel setup states:
// start (no buttons visible, no video panels visible)
//  if it fails, no buttons visible, maybe show an indicator that real-time media not available.
//		q-up an automatic retry after some amount of time provided no control channel exists.
//		any event which indicates the user may be online (from pal server, for example)
//		causes an automatic retry. these retries are only performed if the chatbox is
//		active and visible to the user. if the user sends a mesg to the recipient would
//		cause a retry as well.
//
//  if it succeeds, start the state machine below. buttons still not visible.
//  if at any time the control channel is terminated, the state machine below should
//  transition thru any end state then restart this state machine at "start".
//	emit some indication of our camera status to remote end.
//
// after getting a channel setup,
// both caller and callee are in start-state and if the camera is on, the button
// shown on the call box is "send video". otherwise there is no button shown and we remain
// in the start state until we receive notification the camera is on. if the camera is on, but
// it is paused, we show the send-video button. clicking automatically unpauses the video.
//
// if either clicks "send video", they enter state "sending" immediately.
// the send-video button starts blinking, set a timer.
// this sends a msg to the remote end requesting to accept the video.
// if the remote end is in "sending" state, the req is automatically granted -> end state
// if the remote end is in "start", then change buttons to "accept video" and add a new
// button that says "accept video and send your video". change state to "waiting for user".
// start a timer. if the timer expires, the req fails.
// if the user clicks "accept video", reply ok and set our state to receive video.
// replace buttons with just "send video" button in case they want to try again.
// if users clicks "accept and send", reply ok and set our state to sending. the caller
// has no option but to receive the video (ie, the protocol is asymmetric.)
//
// once you enter the end-state, you can pause the video, but you cannot "stop" it
// asymmetrically. you can only hangup the entire channel.
//
#include <QFile>
#include <QScrollBar>
#include <QPainter>
#include <QState>
#include <QFinalState>
#include <QStateMachine>
#include <QSignalTransition>

#include "simple_call.h"
#include "ui_simple_call.h"
#include "mainwin.h"
#include "vidsel.h"
#include "ssmap.h"
#include "tfhex.h"
#include "msgbrowsedock.h"
#include "evret.h"
#include "dlli.h"
#include "ct.h"
#include "composer.h"
#include "simple_public.h"
#include "dwyco_new_msg.h"
#include "snd.h"
#include "ui_vidbox.h"
#include "vidbox.h"
#include "msgtohtml.h"
#include "pfx.h"
#include "dwycolistscoped.h"

extern VidSel *DevSelectBox;
extern int Public_chat_video_pause;
extern DwOString My_uid;
extern int HasCamera;
extern int HasCamHardware;
extern int HasAudioInput;
extern int HasAudioOutput;

extern QRect MainScreenRect;

DwQueryByMember<simple_call> simple_call::Simple_calls;

DwOString dwyco_get_attr(DWYCO_LIST l, int row, const char *col);

static int
send_user_command(int chan_id, DwOString cmd)
{
    if(chan_id == -1)
        return 0;
    dwyco_command_from_keyboard(chan_id, 'u', 0, 0, cmd.c_str(), cmd.length());
    return 1;
}

simple_call::simple_call(const DwOString& auid, QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::simple_call),
    vp(this),
    keyboard_active_timer(this),
    uid(auid)
{
    Simple_calls.add(this);
    ui->setupUi(this);
#ifdef CDCX_RELEASE
    ui->debug_label->setVisible(0);
#endif
    ReturnFilter *return_filter = new ReturnFilter;
    connect(return_filter, SIGNAL(chat_typing()), this, SLOT(keyboard_input()));
    connect(return_filter, SIGNAL(return_hit()), this, SLOT(on_send_button_clicked()));
    connect(return_filter, SIGNAL(esc_hit()), this, SLOT(clear_chatwin()));
    keyboard_active_timer.setSingleShot(1);
    connect(&keyboard_active_timer, SIGNAL(timeout()), this, SLOT(keyboard_inactive()));
    kb_active = 0;

    ui->textEdit->installEventFilter(return_filter);
    // this is a workaround for the situation where the
    // resize of the text browswer with a scrollbar present
    // causes the scrollbar to move away from the bottom of the
    // text browser.
    ResizeFilter *rf = new ResizeFilter;
    ui->textBrowser->installEventFilter(rf);
    connect(rf, SIGNAL(resized()), this, SLOT(force_scrollbar_down()));

    // this seems to reflect whether the pane is visible or not a little better
    // than the "isvisible" attribute on the pane. by tracking visibilityChanged,
    // it takes into account the docking/tabbed docks and whatnot.
    vis = 0;

    ui->msgplayer->setVisible(0);
    ui->label->setVisible(0);

    ui->accept->setVisible(0);
    ui->accept_and_send->setVisible(0);
    if(HasCamHardware)
        ui->send_video->setVisible(1);
    else
        ui->send_video->setVisible(0);

    ui->send_video->setEnabled(0);
    ui->send_video->setStyleSheet(" background-color: gray; border: none; color: rgb(255, 255, 255);");
    ui->reject->setVisible(0);
    ui->hangup->setVisible(0);
    ui->cancel_req->setVisible(0);

    ui->send_audio_button->setVisible(0);
    ui->mute_button->setVisible(0);

    ui->send_snapchat->setDefaultAction(ui->actionSend_snapchat);
    ui->snap_timer->setDefaultAction(ui->actionStart_snapshot_timer);
    snapshot_timer.setInterval(1000);
    connect(&snapshot_timer, SIGNAL(timeout()), this, SLOT(snapshot_timeout()));
    snapshot_countdown = 5;

    if(dwyco_is_pal(uid.c_str(), uid.length()))
    {
        ui->pal_button->setVisible(0);
    }

    //ui_id = -1;
    zap_id = dwyco_make_zap_composition(0);
    //int dummy;
    record_preview_id = -1;
#if 0
    if(HasCamera)
    {
        if(!dwyco_zap_record(zap_id, 0, 0, 1, -1,
                             0, 0, &record_preview_id))
        {
        }
    }
#endif


    toolbar = new QToolBar(ui->label);
    //toolbar->addAction(ui->actionCam_off);
    //toolbar->addAction(ui->actionPause);
    //toolbar->addAction(ui->actionCam_on);
    //toolbar->addAction(ui->actionVideo_options);
    //toolbar->addAction(ui->actionRefresh);
#define POPUP_TOOLBAR
    toolbar->addAction(ui->actionZoom_in);
    //toolbar->addAction(ui->actionZoom_out);
    toolbar->addAction(ui->actionPause);

    if(HasCamera)
    {
        ui->actionPause->setVisible(1);
        ui->actionRefresh->setVisible(1);
        ui->actionSend_snapchat->setVisible(1);
        ui->actionStart_snapshot_timer->setVisible(1);
        ui->countdown_label->setVisible(1);
        ui->countdown_label->clear();
    }
    else
    {
        ui->actionRefresh->setVisible(0);
        ui->actionSend_snapchat->setVisible(0);
        ui->actionStart_snapshot_timer->setVisible(0);
        ui->countdown_label->setVisible(0);
        ui->countdown_label->clear();
        ui->snap_timer->setVisible(0);
        ui->send_snapchat->setVisible(0);
        ui->send_preview->setVisible(0);
        ui->verticalSpacer->changeSize(0, 0);
        ui->verticalSpacer_2->changeSize(0, 0);
        ui->textEdit->setMaximumHeight(27);
        ui->verticalLayout->invalidate();
        ui->verticalLayout_2->invalidate();
    }
#if 0
    QActionGroup *a = new QActionGroup(this);
    a->addAction(ui->actionCam_off);
    a->addAction(ui->actionPause);
    a->addAction(ui->actionCam_on);
    a->setExclusive(1);
    if(HasCamera)
    {
        ui->actionCam_off->setChecked(1);
        ui->actionRefresh->setVisible(1);
    }
    else
    {
        ui->actionCam_on->setChecked(1);
        ui->actionRefresh->setVisible(0);
    }
#endif

    connect(ui->actionCam_off, SIGNAL(triggered(bool)), this, SLOT(cam_control_off(bool)));
    connect(ui->actionCam_on, SIGNAL(triggered(bool)), this, SLOT(cam_control_on(bool)));

    toolbar->setVisible(0);
    toolbar->move(3, ui->label->height() - toolbar->height() - 8);
    connect(ui->label, SIGNAL(resized()), this, SLOT(reposition_toolbar()));
    connect(ui->label, SIGNAL(resized()), this, SLOT(render_image()));
    connect(ui->label, SIGNAL(mouse_leave()), this, SLOT(handle_toolbar()));

    connect(this, SIGNAL(pal_event(DwOString)), Mainwinform, SIGNAL(pal_event(DwOString)));
    connect(Mainwinform, SIGNAL(pal_event(DwOString)), this, SLOT(update_pal(DwOString)));
    connect(Mainwinform, SIGNAL(mouse_stopped(QPoint)), SLOT(mouse_stopped_event(QPoint)));
    connect(Mainwinform, SIGNAL(new_msg(DwOString,DwOString,DwOString)), this, SLOT(display_new_msg(DwOString,DwOString,DwOString)));
    connect(this, SIGNAL(uid_selected(DwOString,int)), Mainwinform, SIGNAL(uid_selected(DwOString,int)));
    connect(this, SIGNAL(ignore_event(DwOString)), Mainwinform, SIGNAL(ignore_event(DwOString)));
    connect(Mainwinform, SIGNAL(ignore_event(DwOString)), this, SLOT(process_ignore_event(DwOString)));
    connect(Mainwinform, SIGNAL(camera_change(int)), this, SLOT(camera_event(int)));
    connect(Mainwinform, SIGNAL(video_capture_preview(QImage)), this, SLOT(set_preview_image(QImage)));
    connect(Mainwinform, SIGNAL(video_display(int,QImage)), this, SLOT(set_remote_image(int, QImage)));
    connect(Mainwinform, SIGNAL(control_msg(int,DwOString,int,int,DwOString)), this, SLOT(recv_control_msg(int,DwOString,int,int,DwOString)));
    connect(this, SIGNAL(rem_cam_off()), this, SLOT(react_to_rem_cam_off()));
    connect(this, SIGNAL(rem_cam_on()), this, SLOT(react_to_rem_cam_on()));
    connect(this, SIGNAL(rem_cam_none()), this, SLOT(react_to_rem_cam_none()));
    connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(start_control(bool)));
    connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(vis_unviewed(bool)));
    connect(this, SIGNAL(visibilityChanged(bool)), Mainwinform, SLOT(simplify_view(bool)));
    connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(handle_vis_change(bool)));
    connect(ui->msgplayer->ui.close_button, SIGNAL(clicked()), this, SLOT(hide_msgplayer()));
    connect(ui->send_preview, SIGNAL(mouse_release()), this, SLOT(on_actionRefresh_triggered()));
    connect(Mainwinform, SIGNAL(global_video_pause_toggled(bool)), ui->actionPause, SLOT(setChecked(bool)));
    connect(Mainwinform, SIGNAL(global_video_pause_toggled(bool)), ui->actionPause, SLOT(setDisabled(bool)));
    connect(Mainwinform, SIGNAL(global_video_pause_toggled(bool)), ui->actionSend_snapchat, SLOT(setDisabled(bool)));
    connect(Mainwinform, SIGNAL(global_video_pause_toggled(bool)), ui->actionStart_snapshot_timer, SLOT(setDisabled(bool)));
    connect(Mainwinform, SIGNAL(global_video_pause_toggled(bool)), this, SLOT(stop_snapshot_timer(bool)));
    connect(ui->actionSend_snapchat, SIGNAL(triggered()), this, SLOT(on_actionRefresh_triggered()));
    connect(ui->actionSend_snapchat, SIGNAL(triggered(bool)), ui->actionStart_snapshot_timer, SLOT(setEnabled(bool)));
    connect(ui->actionSend_snapchat, SIGNAL(triggered(bool)), this, SLOT(stop_snapshot_timer(bool)));

    connect(Mainwinform, SIGNAL(uid_info_event(DwOString)), this, SLOT(uid_resolved(DwOString)));
    connect(Mainwinform, SIGNAL(invalidate_profile(DwOString)), this, SLOT(uid_resolved(DwOString)));
    connect(this, SIGNAL(send_msg_event(DwOString)), Mainwinform, SIGNAL(send_msg_event(DwOString)));
    connect(Mainwinform, SIGNAL(content_filter_event(int)), this, SLOT(process_content_filter_change(int)));

    connect(this, SIGNAL(rem_pause()), this, SLOT(react_to_rem_pause()));
    connect(this, SIGNAL(rem_unpause()), this, SLOT(react_to_rem_unpause()));

    connect(this, SIGNAL(rem_mute_on()), this, SLOT(react_to_rem_mute_on()));
    connect(this, SIGNAL(rem_mute_off()), this, SLOT(react_to_rem_mute_off()));
    connect(this, SIGNAL(rem_cts_on()), this, SLOT(react_to_rem_cts_on()));
    connect(this, SIGNAL(rem_cts_off()), this, SLOT(react_to_rem_cts_off()));
    // need to turn check box event into slightly different form.
    connect(ui->mute_button, SIGNAL(clicked(bool)), this, SLOT(mute_event(bool)));
    connect(this, SIGNAL(mute_on()), this, SLOT(check_mute()));
    connect(this, SIGNAL(mute_off()), this, SLOT(uncheck_mute()));

    connect(this, SIGNAL(audio_recording(int,int)), Mainwinform, SIGNAL(audio_recording(int,int)));
    connect(Mainwinform, SIGNAL(audio_recording(int,int)), this, SLOT(audio_recording_event(int,int)));

    connect(&resize_timer, SIGNAL(timeout()), this, SLOT(check_resize()));

    connect(Mainwinform, SIGNAL(msg_send_status(int,DwOString,DwOString)), this, SLOT(pers_msg_send_status(int,DwOString,DwOString)));
    connect(Mainwinform, SIGNAL(msg_progress(DwOString,DwOString,DwOString,int)), this, SLOT(msg_progress(DwOString,DwOString,DwOString,int)));

    reset_title();

    popup_menu = 0;
    popup_menu = new QMenu(this);
    popup_menu->addAction(ui->actionView_profile_ctx);
    popup_menu->addAction(ui->actionCompose_msg_ctx);
    popup_menu->addAction(ui->actionSend_file_ctx);
    popup_menu->addAction(ui->actionIgnore_user_ctx);

    show_preview = 0;

    chan_id = -1;
    call_id = -1;

    connect_state_machine = new QStateMachine(this);
    call_setup_state_machine = new QStateMachine(this);
    send_state_machine = new QStateMachine(this);
    recv_state_machine = new QStateMachine(this);
    audio_stream_state_machine = new QStateMachine(this);
    full_duplex_audio_stream_state_machine = new QStateMachine(this);
    mute_state_machine = new QStateMachine(this);

    connect(connect_state_machine, SIGNAL(stopped()), this, SLOT(disable_connect_button()));

    {
        // state machine for keeping a control connection between caller and callee
        QState *start = new QState;
        QState *connecting = new QState;
        QState *established = new QState;
        QState *retrying = new QState;
        QState *no_auto_retry = new QState;
        QFinalState *final = new QFinalState;

        start->addTransition(this, SIGNAL(try_connect()), connecting);
        start->assignProperty(ui->connect_button, "enabled", 1);
        start->assignProperty(ui->connect_button, "checked", 0);
        //start->assignProperty(ui->connect_button, "icon", QIcon(":/new/red24/icons/PrimaryCons_Red_KenSaunders/PNGs/24x24/restart-24x24.png"));
        connecting->addTransition(this, SIGNAL(connect_established()), established);
        connecting->addTransition(this, SIGNAL(connect_failed()), no_auto_retry);
        connecting->addTransition(this, SIGNAL(connect_already_exists()), established);
        connecting->assignProperty(ui->connect_button, "enabled", 0);
        connecting->assignProperty(ui->connect_button, "checked", 0);
        //connecting->assignProperty(ui->connect_button, "icon", QIcon(":/new/red24/icons/PrimaryCons_Red_KenSaunders/PNGs/24x24/restart-24x24.png"));
        established->addTransition(this, SIGNAL(connect_terminated()), retrying);
        established->assignProperty(ui->connect_button, "enabled", 0);
        established->assignProperty(ui->connect_button, "checked", 1);
        //established->assignProperty(ui->connect_button, "icon", QIcon(":/new/green24/icons/PrimaryCons_Green_KenSaunders/PNGs/24x24/Locked-24x24.png"));

        retrying->addTransition(&reconnect_timer, SIGNAL(timeout()), connecting);
        retrying->addTransition(this, SIGNAL(try_connect()), connecting);
        retrying->assignProperty(ui->connect_button, "enabled", 1);
        retrying->assignProperty(ui->connect_button, "checked", 0);
        //retrying->assignProperty(ui->connect_button, "icon", QIcon(":/new/red24/icons/PrimaryCons_Red_KenSaunders/PNGs/24x24/restart-24x24.png"));
        no_auto_retry->addTransition(this, SIGNAL(try_connect()), connecting);
        no_auto_retry->assignProperty(ui->connect_button, "enabled", 1);
        no_auto_retry->assignProperty(ui->connect_button, "checked", 0);
        //no_auto_retry->assignProperty(ui->connect_button, "icon", QIcon(":/new/red24/icons/PrimaryCons_Red_KenSaunders/PNGs/24x24/restart-24x24.png"));

        QObject::connect(connecting, SIGNAL(entered()), this, SLOT(start_media_calls()));
        QObject::connect(retrying, SIGNAL(entered()), this, SLOT(start_retry_timer()));
        QObject::connect(retrying, SIGNAL(exited()), this, SLOT(stop_retry_timer()));

        connect_state_machine->addState(start);
        connect_state_machine->addState(connecting);
        connect_state_machine->addState(established);
        connect_state_machine->addState(retrying);
        connect_state_machine->addState(no_auto_retry);
        connect_state_machine->addState(final);
        connect_state_machine->setInitialState(start);
        connect_state_machine->start();

    }

    {
        // start of call/call screening state machine
        QState *start = new QState;
        QState *wait_for_call = new QState;
        QState *send_and_wait_for_call = new QState;
        QState *accept_video_query = new QState;
        QState *accept_and_send_video_query = new QState;
        QState *wait_rem_response = new QState;
        QState *start_recv_machine = new QState;
        QState *start_send_machine = new QState;
        QState *start_recv_and_send_machine = new QState;

        QObject::connect(start, SIGNAL(entered()), this, SLOT(initial_cam_setup()));
        start->addTransition(this, SIGNAL(cam_is_off()), wait_for_call);
        start->addTransition(this, SIGNAL(cam_is_on()), send_and_wait_for_call);
        assign_button_states(start, 0, 0, 0, 0, 0, 0, 0);

        wait_for_call->addTransition(this, SIGNAL(cam_is_on()), send_and_wait_for_call);
        wait_for_call->addTransition(this, SIGNAL(recv_query()), accept_video_query);
        wait_for_call->addTransition(ui->send_video, SIGNAL(clicked()), wait_rem_response);
        assign_button_states(wait_for_call, 0, 0, HasCamHardware, HasCamHardware, 0, 0, 0);

        accept_video_query->addTransition(this, SIGNAL(cam_is_on()), accept_and_send_video_query);
        accept_video_query->addTransition(&accept_timer, SIGNAL(timeout()), wait_for_call);
        accept_video_query->addTransition(ui->accept, SIGNAL(clicked()), start_recv_machine);
        accept_video_query->addTransition(ui->reject, SIGNAL(clicked()), start);
        // if they turn their cam off after req to send video, it is the same as us clicking reject.
        accept_video_query->addTransition(this, SIGNAL(rem_cam_off()), start);
        accept_video_query->addTransition(this, SIGNAL(recv_cancel()), start);
        QObject::connect(accept_video_query, SIGNAL(entered()), this, SLOT(start_accept_timer()));
        QObject::connect(accept_video_query, SIGNAL(exited()), this, SLOT(stop_accept_timer()));
        QObject::connect(&accept_timer, SIGNAL(timeout()), ui->reject, SIGNAL(clicked()));
        //accept_video_query->addTransition(this, SIGNAL(recv_reject()), wait_for_call);

        assign_button_states(accept_video_query, 1, 0, 0, 0, 1, 0, 0);

        send_and_wait_for_call->addTransition(this, SIGNAL(recv_query()), accept_and_send_video_query);
        send_and_wait_for_call->addTransition(ui->send_video, SIGNAL(clicked()), wait_rem_response);
        send_and_wait_for_call->addTransition(this, SIGNAL(cam_is_off()), wait_for_call);
        assign_button_states(send_and_wait_for_call, 0, 0, 1, HasCamHardware, 0, 0, 0);

        accept_and_send_video_query->addTransition(&accept_timer, SIGNAL(timeout()), send_and_wait_for_call);
        accept_and_send_video_query->addTransition(ui->accept_and_send, SIGNAL(clicked()), start_recv_and_send_machine);
        accept_and_send_video_query->addTransition(ui->reject, SIGNAL(clicked()), start);
        accept_and_send_video_query->addTransition(this, SIGNAL(rem_cam_off()), start);
        accept_and_send_video_query->addTransition(this, SIGNAL(recv_cancel()), start);
        accept_and_send_video_query->addTransition(this, SIGNAL(cam_is_off()), accept_video_query);
        QObject::connect(accept_and_send_video_query, SIGNAL(entered()), this, SLOT(start_accept_timer()));
        QObject::connect(accept_and_send_video_query, SIGNAL(exited()), this, SLOT(stop_accept_timer()));
        assign_button_states(accept_and_send_video_query, 0, 1, 0, 0, 1, 0, 0);

        wait_rem_response->addTransition(this, SIGNAL(recv_ok()), start_send_machine);
        wait_rem_response->addTransition(this, SIGNAL(recv_query()), start_recv_and_send_machine);
        wait_rem_response->addTransition(&ask_timer, SIGNAL(timeout()), start);
        wait_rem_response->addTransition(this, SIGNAL(recv_reject()), start);
        wait_rem_response->addTransition(this, SIGNAL(cam_is_off()), start);
        wait_rem_response->addTransition(ui->cancel_req, SIGNAL(clicked()), start);
        QObject::connect(wait_rem_response, SIGNAL(entered()), this, SLOT(start_ask_timer()));
        QObject::connect(wait_rem_response, SIGNAL(exited()), this, SLOT(stop_ask_timer()));
        QObject::connect(&ask_timer, SIGNAL(timeout()), ui->cancel_req, SIGNAL(clicked()));
        assign_button_states(wait_rem_response, 0, 0, 0, 0, 0, 0, 1);

        QObject::connect(start_send_machine, SIGNAL(entered()), send_state_machine, SLOT(start()));
        QObject::connect(start_send_machine, SIGNAL(entered()), recv_state_machine, SLOT(start()));

        QObject::connect(start_recv_machine, SIGNAL(entered()), recv_state_machine, SLOT(start()));
        QObject::connect(start_recv_machine, SIGNAL(entered()), send_state_machine, SLOT(start()));
        assign_button_states(start_recv_machine, 0, 0, 0, 0, 0, 1, 0);


        QObject::connect(start_recv_and_send_machine, SIGNAL(entered()), send_state_machine, SLOT(start()));
        QObject::connect(start_recv_and_send_machine, SIGNAL(entered()), recv_state_machine, SLOT(start()));


        call_setup_state_machine->addState(start);
        call_setup_state_machine->addState(wait_for_call);
        call_setup_state_machine->addState(send_and_wait_for_call);
        call_setup_state_machine->addState(accept_video_query);
        call_setup_state_machine->addState(accept_and_send_video_query);
        call_setup_state_machine->addState(wait_rem_response);
        call_setup_state_machine->addState(start_recv_machine);
        call_setup_state_machine->addState(start_send_machine);
        call_setup_state_machine->addState(start_recv_and_send_machine);
        call_setup_state_machine->setInitialState(start);

        //QObject::connect(call_setup_state_machine, SIGNAL(stopped()), this, SLOT(reset_call_buttons()));

    }
    {
        QState *start = new QState;
        QState *stream_out = new QState;
        QState *pause = new QState;
        QState *rem_pause = new QState;
        QState *rem_pause2 = new QState;
        QFinalState *final = new QFinalState;

        start->assignProperty(ui->actionPause, "checked", 0);
        start->assignProperty(Mainwinform->vid_preview->ui->actionPause, "checked", 0);
        assign_button_states(start, 0, 0, 0, 0, 0, 1, 0);
        //start->addTransition(stream_out);
        QObject::connect(start, SIGNAL(entered()), this, SLOT(initial_cam_setup()));
        QObject::connect(start, SIGNAL(entered()), this, SLOT(set_refresh_users()));
        start->addTransition(this, SIGNAL(cam_is_on()), stream_out);
        stream_out->addTransition(ui->actionPause, SIGNAL(toggled(bool)), pause);
        stream_out->addTransition(this, SIGNAL(cam_is_off()), start);
        QObject::connect(stream_out, SIGNAL(entered()), this, SLOT(start_stream()));

        pause->addTransition(ui->actionPause, SIGNAL(toggled(bool)), stream_out);
        pause->addTransition(this, SIGNAL(cam_is_off()), start);
        QObject::connect(pause, SIGNAL(entered()), this, SLOT(pause_stream()));

        QObject::connect(send_state_machine, SIGNAL(stopped()), this, SLOT(set_refresh_users()));


        send_state_machine->addState(start);
        send_state_machine->addState(stream_out);
        send_state_machine->addState(pause);
        send_state_machine->addState(rem_pause);
        send_state_machine->addState(rem_pause2);
        send_state_machine->addState(final);
        send_state_machine->setInitialState(start);

    }

    {
        QState *start = new QState;
        QState *stream_in = new QState;
        QState *pause = new QState;
        QState *rem_pause = new QState;
        QState *rem_pause2 = new QState;
        QFinalState *final = new QFinalState;

        start->addTransition(stream_in);
        start->assignProperty(ui->label, "visible", 1);
        start->assignProperty(ui->actionZoom_in, "checked", 0);
        QObject::connect(start, SIGNAL(entered()), this, SLOT(set_refresh_users()));

        stream_in->addTransition(this, SIGNAL(rem_pause()), rem_pause);
        //stream_in->addTransition(this, SIGNAL(rem_cam_off()), final);
        rem_pause->addTransition(this, SIGNAL(rem_unpause()), stream_in);
        //rem_pause->addTransition(this, SIGNAL(rem_cam_off()), final);

        QObject::connect(recv_state_machine, SIGNAL(stopped()), this, SLOT(set_refresh_users()));


        recv_state_machine->addState(start);
        recv_state_machine->addState(stream_in);
        recv_state_machine->addState(pause);
        recv_state_machine->addState(rem_pause);
        recv_state_machine->addState(rem_pause2);
        recv_state_machine->addState(final);
        recv_state_machine->setInitialState(start);

    }

    {
        QState *start = new QState;
        QState *play_incoming_record_off = new QState;
        QState *play_off_record_on = new QState;
        QState *cts_wait = new QState;

        QObject::connect(start, SIGNAL(entered()), this, SLOT(initial_audio_stream_setup()));
        start->addTransition(this, SIGNAL(rem_cts_off()), cts_wait);
        start->addTransition(this, SIGNAL(rem_cts_on()), play_incoming_record_off);
        //start->assignProperty(ui->send_audio_button, "checked", 0);

        QObject::connect(cts_wait, SIGNAL(entered()), this, SLOT(set_half_duplex_play()));
        cts_wait->addTransition(this, SIGNAL(rem_cts_on()), play_incoming_record_off);
        //cts_wait->assignProperty(ui->send_audio_button, "enabled", 0);
        cts_wait->assignProperty(ui->send_audio_button, "styleSheet", " background-color: red; border: none; color: rgb(255, 255, 255);");

        QObject::connect(play_incoming_record_off, SIGNAL(entered()), this, SLOT(set_half_duplex_play()));
        play_incoming_record_off->addTransition(ui->send_audio_button, SIGNAL(pressed()), play_off_record_on);
        play_incoming_record_off->addTransition(this, SIGNAL(rem_cts_off()), cts_wait);
        play_incoming_record_off->assignProperty(ui->send_audio_button, "enabled", 1);
        play_incoming_record_off->assignProperty(ui->send_audio_button, "styleSheet", " background-color: green; border: none; color: rgb(255, 255, 255);");
        play_incoming_record_off->assignProperty(ui->send_audio_button, "text", "Send audio");

        QObject::connect(play_off_record_on, SIGNAL(entered()), this, SLOT(set_half_duplex_record()));
        play_off_record_on->addTransition(ui->send_audio_button, SIGNAL(released()), play_incoming_record_off);
        play_off_record_on->addTransition(this, SIGNAL(rem_cts_off()), cts_wait);
        play_off_record_on->assignProperty(ui->send_audio_button, "text", ">>>>");

        audio_stream_state_machine->addState(start);
        audio_stream_state_machine->addState(cts_wait);
        audio_stream_state_machine->addState(play_incoming_record_off);
        audio_stream_state_machine->addState(play_off_record_on);
        audio_stream_state_machine->setInitialState(start);

    }

    // assumes both sides are full duplex and have echo cancellation of some sort
    {
        QState *start = new QState;
        QState *play_incoming_record_off = new QState;
        QState *record_on = new QState;

        QObject::connect(start, SIGNAL(entered()), this, SLOT(initial_full_duplex_audio_stream_setup()));
        start->assignProperty(ui->send_audio_button, "checked", 0);
        start->addTransition(play_incoming_record_off);

        QObject::connect(play_incoming_record_off, SIGNAL(entered()), this, SLOT(set_half_duplex_play()));
        play_incoming_record_off->addTransition(ui->send_audio_button, SIGNAL(clicked()), record_on);
        play_incoming_record_off->assignProperty(ui->send_audio_button, "enabled", 1);
        play_incoming_record_off->assignProperty(ui->send_audio_button, "styleSheet", " background-color: green; border: none; color: rgb(255, 255, 255);");
        play_incoming_record_off->assignProperty(ui->send_audio_button, "text", "Send audio");

        QObject::connect(record_on, SIGNAL(entered()), this, SLOT(set_full_duplex_record()));
        record_on->addTransition(ui->send_audio_button, SIGNAL(clicked()), play_incoming_record_off);
        record_on->assignProperty(ui->send_audio_button, "text", ">>>>");

        full_duplex_audio_stream_state_machine->addState(start);
        full_duplex_audio_stream_state_machine->addState(play_incoming_record_off);
        full_duplex_audio_stream_state_machine->addState(record_on);
        full_duplex_audio_stream_state_machine->setInitialState(start);

    }

    {
        QState *audio_none = new QState;
        QState *audio_stop = new QState;
        QState *start = new QState(audio_none);
        QState *go = new QState(audio_none);
        QState *lmute = new QState(audio_none);
        QState *rmute = new QState(audio_none);
        QState *lrmute = new QState(audio_none);
        QState *start2 = new QState(audio_none);

        audio_none->addTransition(this, SIGNAL(audio_none()), audio_stop);
        audio_none->addTransition(this, SIGNAL(rem_audio_none()), audio_stop);

        audio_stop->assignProperty(ui->mute_button, "checked", 0);
        audio_stop->assignProperty(ui->mute_button, "visible", 1);
        audio_stop->assignProperty(ui->mute_button, "enabled", 0);
        audio_stop->assignProperty(ui->send_audio_button, "visible", 0);
        audio_stop->assignProperty(ui->mute_button, "text", "no audio");


        QObject::connect(start, SIGNAL(entered()), this, SLOT(initial_audio_setup()));

        // don't move out of the start state until we get the initial mute state from remote
        // this is a hack to make connections to old software that doesn't do this protocol
        // more or less stop here so it doesn't confuse the user.
        start->addTransition(this, SIGNAL(rem_mute_off()), start2);
        start->addTransition(this, SIGNAL(rem_mute_on()), start2);
        QSignalTransition *tmp = new QSignalTransition(this, SIGNAL(mute_off()));
        start->addTransition(tmp);
        tmp = new QSignalTransition(this, SIGNAL(mute_on()));
        start->addTransition(tmp);
        start->assignProperty(ui->mute_button, "checked", 1);
        start->assignProperty(ui->mute_button, "visible", 0);
        start->assignProperty(ui->mute_button, "enabled", 1);
        start->assignProperty(ui->send_audio_button, "visible", 0);


        QObject::connect(start2, SIGNAL(entered()), this, SLOT(reemit_mute()));
        start2->addTransition(this, SIGNAL(rem_mute_off()), go);
        start2->addTransition(this, SIGNAL(rem_mute_on()), rmute);
        start2->addTransition(this, SIGNAL(mute_off()), go);
        start2->addTransition(this, SIGNAL(mute_on()), lmute);
        start2->assignProperty(ui->mute_button, "checked", 1);
        start2->assignProperty(ui->mute_button, "visible", 1);
        start2->assignProperty(ui->mute_button, "enabled", 1);
        start2->assignProperty(ui->send_audio_button, "visible", 0);

#ifdef FULL_DUPLEX
        // more experimenting with aec, for now doesn't work
        QObject::connect(go, SIGNAL(entered()), full_duplex_audio_stream_state_machine, SLOT(start()));
#else
        QObject::connect(go, SIGNAL(entered()), audio_stream_state_machine, SLOT(start()));
#endif
        QObject::connect(go, SIGNAL(exited()), this, SLOT(reset_audio_stream()));


        go->addTransition(this, SIGNAL(mute_on()), lmute);
        go->addTransition(this, SIGNAL(rem_mute_on()), rmute);
        go->assignProperty(ui->mute_button, "text", "Block audio");
        go->assignProperty(ui->mute_button, "styleSheet", " background-color: red; border: none; color: rgb(255, 255, 255);");
        go->assignProperty(ui->send_audio_button, "enabled", 1);
        go->assignProperty(ui->send_audio_button, "visible", 1);

        lmute->addTransition(this, SIGNAL(rem_mute_on()), lrmute);
        lmute->addTransition(this, SIGNAL(mute_off()), go);
        lmute->assignProperty(ui->mute_button, "text", "Allow live audio");
        lmute->assignProperty(ui->send_audio_button, "visible", 0);
        lmute->assignProperty(ui->mute_button, "styleSheet", " background-color: green; border: none; color: rgb(255, 255, 255);");

        rmute->addTransition(this, SIGNAL(mute_on()), lrmute);
        rmute->addTransition(this, SIGNAL(rem_mute_off()), go);
        rmute->assignProperty(ui->mute_button, "text", "Waiting for remote audio");
        rmute->assignProperty(ui->mute_button, "styleSheet", " background-color: yellow; border: none; color: rgb(0, 0, 0);");
        rmute->assignProperty(ui->send_audio_button, "visible", 0);

        lrmute->addTransition(this, SIGNAL(rem_mute_off()), lmute);
        lrmute->addTransition(this, SIGNAL(mute_off()), rmute);
        lrmute->assignProperty(ui->mute_button, "text", "Allow live audio");
        lrmute->assignProperty(ui->mute_button, "styleSheet", " background-color: green; border: none; color: rgb(255, 255, 255);");
        lrmute->assignProperty(ui->send_audio_button, "visible", 0);
#if 0
        mute_state_machine->addState(start);
        mute_state_machine->addState(go);
        mute_state_machine->addState(lmute);
        mute_state_machine->addState(rmute);
        mute_state_machine->addState(lrmute);
        mute_state_machine->addState(mute_unknown);
#endif
        mute_state_machine->addState(audio_none);
        mute_state_machine->addState(audio_stop);
        mute_state_machine->setInitialState(audio_none);
        audio_none->setInitialState(start);

    }

    allstop = 0;
    rpause = 0;
    rcam_on = 1;
    rcam_none = 0;
    paused_text.setText("Remote Paused");
    off_text.setText("Remote camera off");
    no_cam_hardware_text.setText("Remote has no camera");

    // initial audio state
    rcts = -1;
    rmute = -1;

    cts = 0;
    mute = 0;

    // we create ourselves invisible, and let the context where we are created
    // decide when to show us
    setVisible(0);
    update_content_filters();
}

void
simple_call::assign_button_states(QState *s, int accept, int accept_and_send, int send_video_vis, int send_video_enabled, int reject, int hangup, int cancel_req)
{
    s->assignProperty(ui->accept, "visible", accept);
    s->assignProperty(ui->accept_and_send, "visible", accept_and_send);
    s->assignProperty(ui->send_video, "visible", send_video_vis);
    s->assignProperty(ui->send_video, "enabled", send_video_enabled);
    if(send_video_enabled)
        s->assignProperty(ui->send_video, "styleSheet", " background-color: green; border: none; color: rgb(255, 255, 255);");
    else
        s->assignProperty(ui->send_video, "styleSheet", " background-color: gray; border: none; color: rgb(255, 255, 255);");
    s->assignProperty(ui->reject, "visible", reject);
    s->assignProperty(ui->hangup, "visible", hangup);
    s->assignProperty(ui->cancel_req, "visible", cancel_req);

}

void
simple_call::reset_call_buttons()
{
    ui->label->setVisible(0);

    ui->accept->setVisible(0);
    ui->accept_and_send->setVisible(0);
    if(HasCamHardware)
        ui->send_video->setVisible(1);
    else
        ui->send_video->setVisible(0);
    ui->send_video->setStyleSheet(" background-color: gray; border: none; color: rgb(255, 255, 255);");
    ui->send_video->setEnabled(1);
    ui->reject->setVisible(0);
    ui->hangup->setVisible(0);
    ui->cancel_req->setVisible(0);
    ui->mute_button->setVisible(0);
    ui->send_audio_button->setVisible(0);
    dwyco_set_all_mute(0);
}

void
simple_call::set_connection_status_display(int on)
{
    if(on)
    {
        ui->connect_button->setChecked(1);
        ui->connect_button->setEnabled(0);
    }
}

void
simple_call::reemit_mute()
{
    if(rmute == 1)
        emit rem_mute_on();
    else if(rmute == 0)
        emit rem_mute_off();
    else if(rmute == -1)
        emit rem_mute_unknown();
    if(mute)
        emit mute_on();
    else
        emit mute_off();
}

void
simple_call::check_mute()
{
    ui->mute_button->setChecked(1);
}

void
simple_call::uncheck_mute()
{
    ui->mute_button->setChecked(0);
}

void
simple_call::mute_event(bool a)
{
    if(a)
    {
        send_user_command(chan_id, "mute on");
        emit mute_on();
    }
    else
    {
        send_user_command(chan_id, "mute off");
        emit mute_off();
    }
}

void
simple_call::react_to_rem_cts_on()
{
    ui->send_audio_button->setEnabled(1);
    rcts = 1;
    dwyco_set_all_mute(0);

}
void
simple_call::react_to_rem_cts_off()
{
    ui->send_audio_button->setEnabled(0);
    rcts = 0;
    dwyco_set_all_mute(1);

}

void
simple_call::react_to_rem_mute_on()
{
    rmute = 1;
}

void
simple_call::react_to_rem_mute_off()
{
    rmute = 0;
}

void
simple_call::set_refresh_users()
{
    cdcx_set_refresh_users(1);
}

int
simple_call::can_stream()
{
    return send_state_machine->isRunning() || recv_state_machine->isRunning();
}


void
simple_call::force_scrollbar_down()
{
    cursor_to_bottom(ui->textBrowser);
}

void
simple_call::reset_title()
{
    QString ha = "Private: ";
    ha += dwyco_info_to_display(uid);
    ha.truncate(30);
    setWindowTitle(ha);
}

void
simple_call::showEvent(QShowEvent *e)
{
    QDockWidget::showEvent(e);
    if(uid_has_unviewed_msgs(uid))
    {
        reset_unviewed();
    }
    if(dwyco_get_invisible_state() && !Visible_to.contains(uid))
        return;
    emit try_connect();
    ui->textEdit->setFocus();
}

void
simple_call::focusInEvent(QFocusEvent *e)
{
    QDockWidget::focusInEvent(e);
    if(uid_has_unviewed_msgs(uid))
    {
        reset_unviewed();
    }

}

void
simple_call::closeEvent(QCloseEvent *e)
{
    QDockWidget::closeEvent(e);
    if(uid_has_unviewed_msgs(uid))
    {
        reset_unviewed();
    }
    on_hangup_clicked();
}

void
simple_call::hideEvent(QHideEvent *e)
{
    QDockWidget::hideEvent(e);
    if(uid_has_unviewed_msgs(uid))
    {
        reset_unviewed();
    }
}


void
simple_call::start_control(bool a)
{
    if(a)
    {
        if(dwyco_get_invisible_state() && !Visible_to.contains(uid))
            return;
        emit try_connect();
    }
}

void
simple_call::vis_unviewed(bool a)
{
    if(a)
    {
        if(uid_has_unviewed_msgs(uid))
        {
            reset_unviewed();
        }
    }
}

simple_call::~simple_call()
{
    delete ui;

    dwyco_delete_zap_composition(zap_id);
    dwyco_destroy_channel(chan_id);
    dwyco_cancel_call(call_id);
    // attempt to avoid a race condition that leaves these
    // external call structure in an inconsistent state when
    // ignore/remove user is triggered. really need to get rid
    // of these external structures and just query the set of
    // call structures
    call_del_by_uid(uid); // if we originate call, chan_id is suspect
    call_del(chan_id);
    vp.invalidate();
    Simple_calls.del(this);
}

void
simple_call::start_stream()
{
    dwyco_channel_send_video(chan_id, 0);
    //Mainwinform->decorate_users();
    Mainwinform->set_refresh_user(uid);
}

void
simple_call::pause_stream()
{
    dwyco_channel_stop_send_video(chan_id);
}

void
simple_call::reset_state_machines()
{


}

void
simple_call::reset_audio_stream()
{
    audio_stream_state_machine->stop();
    full_duplex_audio_stream_state_machine->stop();
    dwyco_channel_stop_send_audio(chan_id);
    ui->send_audio_button->setVisible(0);
}

void
simple_call::set_half_duplex_play()
{
    dwyco_channel_stop_send_audio(chan_id);
    send_user_command(chan_id, "cts on");
    cts = 1;
    emit cts_on();

}

void
simple_call::set_half_duplex_record()
{
    dwyco_channel_send_audio(chan_id, 0);
    emit audio_recording(chan_id, 1);
    send_user_command(chan_id, "cts off");

    cts = 0;
    emit cts_off();
}

void
simple_call::set_full_duplex_record()
{
    dwyco_channel_send_audio(chan_id, 0);
    emit audio_recording(chan_id, 1);
    send_user_command(chan_id, "cts on");

    cts = 1;
    emit cts_on();
}

void
simple_call::audio_recording_event(int chan_id, int)
{
    if(chan_id != this->chan_id)
    {
        // this is highly unlikely to happen until we get some
        // kind of hands-free send of audio. this is because we
        // stop audio when the button is released, something they
        // would have to do in order find another record button to
        // press.
        // this does something a little different in this case. lets just
        // say that we have "unblocked" audio from someone, and we want to
        // go off and record a zap msg to someone else. we don't really want to
        // have someone's blah blah being recorded on our zap, or being
        // reflected back via the podium audio, for example. so when someone
        // else goes to record audio somewhere else, we change the mute
        // state.
        send_user_command(this->chan_id, "mute on");
        emit mute_on();

    }
}

void
simple_call::initial_cam_setup()
{
    if(HasCamera)
    {
        send_user_command(chan_id, "cam on");
        emit cam_is_on();
    }
    else
    {
        send_user_command(chan_id, "cam off");
        if(!HasCamHardware)
            send_user_command(chan_id, "cam none");
        emit cam_is_off();
    }
}

void
simple_call::initial_audio_setup()
{
    dwyco_set_all_mute(0);
    if(!HasAudioInput || !HasAudioOutput)
    {
        send_user_command(chan_id, "mute on");
        send_user_command(chan_id, "cts off");
        cts = 0;
        mute = 1;
        //rcts = -1;
        //rmute = -1;
        ui->mute_button->setText("no audio hw");
        ui->send_audio_button->setVisible(0);
        ui->mute_button->setEnabled(0);
        ui->mute_button->setVisible(1);
        emit audio_none();
        send_user_command(chan_id, "audio none");
        emit mute_on();
        emit cts_off();
        if(rcts == 0)
            emit rem_cts_off();
        else if(rcts == 1)
            emit rem_cts_on();

        if(rmute == 0)
            emit rem_mute_off();
        else if(rmute == 1)
            emit rem_mute_on();
        else
            emit rem_mute_unknown();
        return;
    }
    ui->mute_button->setText("Unblock audio");
    ui->mute_button->setVisible(1);
    ui->mute_button->setEnabled(0);

    ui->send_audio_button->setVisible(0);
    ui->send_audio_button->setEnabled(0);

    send_user_command(chan_id, "mute on");
    send_user_command(chan_id, "cts off");

    cts = 0;
    mute = 1;
    //rmute = -1;
    //rcts = -1;
    emit mute_on();
    emit cts_off();
    if(rcts == 0)
        emit rem_cts_off();
    else if(rcts == 1)
        emit rem_cts_on();

    if(rmute == 0)
        emit rem_mute_off();
    else if(rmute == 1)
        emit rem_mute_on();
    else
        emit rem_mute_unknown();
}

void
simple_call::initial_audio_stream_setup()
{
    dwyco_set_all_mute(0);
    cts = 0;
    rcts = -1;
    send_user_command(chan_id, "cts off");
    emit cts_off();
}

void
simple_call::initial_full_duplex_audio_stream_setup()
{
    dwyco_set_all_mute(0);
    cts = 1;
    rcts = -1;
    send_user_command(chan_id, "cts on");
    emit cts_on();
}

void
simple_call::start_accept_timer()
{
    const char *val;
    int len;
    int type;
    if(dwyco_get_setting("call_acceptance/auto_accept", &val, &len, &type))
    {
        if(type == DWYCO_TYPE_INT)
        {
            if(atoi(val) == 1)
            {
                ui->accept->click();
                ui->accept_and_send->click();
                return;
            }
        }
    }
    accept_timer.setSingleShot(1);
    accept_timer.start(30000);
    start_animation(uid, ":/new/prefix1/connect_creating.png");
    play_sound("relaxed-call.wav");
}
void
simple_call::stop_accept_timer()
{
    accept_timer.stop();
    stop_animation(uid);
}

void
simple_call::start_ask_timer()
{

    ask_timer.setSingleShot(1);
    ask_timer.start(30000);
    start_animation(uid, ":/new/prefix1/connect_creating.png");
}
void
simple_call::stop_ask_timer()
{
    ask_timer.stop();
    stop_animation(uid);
}

void
simple_call::start_retry_timer()
{
    reconnect_timer.stop();
    reconnect_timer.setSingleShot(1);
    reconnect_timer.setInterval(30000);
    reconnect_timer.start();
}

void
simple_call::stop_retry_timer()
{
    reconnect_timer.stop();
}

simple_call *
simple_call::get_simple_call(DwOString uid)
{
    DWYCO_LIST ur;
    if(!dwyco_map_uid_to_representative(uid.c_str(), uid.length(), &ur))
        return 0;
    simple_scoped qur(ur);
    auto ruid = qur.get<QByteArray>(DWYCO_NO_COLUMN);
    uid = ruid;

    QList<simple_call *> c = Simple_calls.query_by_member(uid, &simple_call::uid);
    simple_call *sc = 0;
    if(c.count() == 0)
    {
        sc = new simple_call(uid);
        Mainwinform->addDockWidget(Qt::LeftDockWidgetArea, sc);
        Mainwinform->tabifyDockWidget(Mainwinform->pubchat, sc);
        sc->setVisible(0);
#if 0
        tst_msg *tm = new tst_msg;
        Msg_sr *msr = new Msg_sr(uid,
                                 tm->ui->textBrowser,
                                 tm->ui->textEdit,
                                 tm->ui->pushButton_3
                                );

        tm->show();
#endif

    }
    else if(c.count() == 1)
    {
        sc = c[0];
    }
    else
        cdcxpanic("simple_call error");
    return sc;
}

void
simple_call::reset_unviewed()
{
    del_unviewed_uid(uid);
    Mainwinform->set_refresh_user(uid);
    //decorate_user();
}

void
simple_call::keyboard_input()
{
    if(uid_has_unviewed_msgs(uid))
    {
        reset_unviewed();
    }

    keyboard_active_timer.start(2000);
    if(kb_active)
        return;
    send_user_command(chan_id, "ka");
    kb_active = 1;
}

void
simple_call::keyboard_inactive()
{
    send_user_command(chan_id, "ki");
    kb_active = 0;
}

void
simple_call::hide_all()
{
    int n = Simple_calls.objs.count();
    for(int i = 0; i < n; ++i)
    {
        Simple_calls.objs[i]->hide();
    }
}

void
simple_call::uid_resolved(DwOString uid)
{
    if(this->uid != uid)
        return;
    reset_title();
    update_content_filters();
}

void
simple_call::update_content_filters()
{
    // give a hint if the profile hasn't been reviewed
    int reviewed;
    int regular;
    get_review_status(uid, reviewed, regular);
    if(!show_profile(uid, reviewed, regular))
    {
        ui->show_profile_button->setIcon(QIcon(":/new/prefix1/baduser.png"));
    }
    else
    {
        ui->show_profile_button->setIcon(QIcon(":/new/green32/gooduser.png"));
    }
}

void
simple_call::recv_control_msg(int ui_id, DwOString com, int /*arg1*/, int /*arg2*/, DwOString str)
{
    if(ui_id != chan_id)
        return;
    if(com.eq("u"))
    {
        if(str.eq("recv?"))
        {
            emit recv_query();
        }
        else if(str.eq("recv ok"))
        {
            emit recv_ok();
        }
        else if(str.eq("recv reject"))
        {
            emit recv_reject();
        }
        else if(str.eq("recv timeout"))
        {

        }
        else if(str.eq("recv cancel"))
        {
            emit recv_cancel();
        }
        else if(str.eq("ka"))
        {
            QString ha = "(typing...) ";
            ha += dwyco_info_to_display(uid);
            ha.truncate(30);
            setWindowTitle(ha);
        }
        else if(str.eq("ki"))
        {
            reset_title();
        }
        else if(str.eq("pause video"))
        {
            emit rem_pause();
        }
        else if(str.eq("unpause video"))
        {
            emit rem_unpause();
        }
        else if(str.eq("cam on"))
        {
            emit rem_cam_on();
        }
        else if(str.eq("cam off"))
        {
            emit rem_cam_off();
        }
        else if(str.eq("cam none"))
        {
            emit rem_cam_none();
        }
        else if(str.eq("cts on"))
        {
            emit rem_cts_on();
        }
        else if(str.eq("cts off"))
        {
            emit rem_cts_off();
        }
        else if(str.eq("mute on"))
        {
            emit rem_mute_on();
        }
        else if(str.eq("mute off"))
        {
            emit rem_mute_off();
        }
        else if(str.eq("audio none"))
        {
            emit rem_audio_none();
        }

    }

}

void
simple_call::new_zap()
{
    zap_id = dwyco_make_zap_composition(0);
    record_preview_id = -1;
    ui->send_preview->ui_id = -1;
    ui->send_preview->clear();
    record_preview_image = QImage();
}

void
simple_call::react_to_rem_cam_off()
{
    remote_image = QImage(":/new/green32/gooduser.png");
    rcam_on = 0;
    render_image();
}

void
simple_call::react_to_rem_cam_none()
{
    remote_image = QImage(":/new/green32/gooduser.png");
    rcam_none = 1;
    rcam_on = 0;
    render_image();
}

void
simple_call::react_to_rem_cam_on()
{
    //ui->label->setVisible(1);
    rcam_on = 1;
    rcam_none = 0;
    render_image();
}

void
simple_call::react_to_rem_pause()
{
    rpause = 1;
    render_image();
}

void
simple_call::react_to_rem_unpause()
{
    rpause = 0;
    render_image();
}


void
DWYCOCALLCONV
dwyco_simple_call_status2(int call_id, const char *msg, int percent, void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    //simple_call *c = (simple_call *)(void *)vp;
    //c->setWindowTitle(msg);
}

void
DWYCOCALLCONV
dwyco_simple_calldisp(int call_id, int chan_id, int what, void *arg, const char *cuid, int len_uid, const char *call_type, int len_call_type)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    simple_call *c = (simple_call *)(void *)vp;
    DwOString ct(call_type, 0, len_call_type);
    DwOString duid(cuid, 0, len_uid);

    switch(what)
    {
    case DWYCO_CALLDISP_STARTED:
    {
        c->call_id = call_id;
        // put the remote uid's profile pic into the call window
        // based on regular/reviewed status...
        // this sets us up to reject incoming calls from the recipient
        // if they are attempting to contact us.
        dwyco_call_type cto(chan_id, call_type, len_call_type, cuid, len_uid, DWYCO_CT_ORIG);
        call_add(cto);
    }
    break;
    case DWYCO_CALLDISP_ESTABLISHED:
    {
        c->chan_id = chan_id;
        Mainwinform->add_call(c->uid, chan_id);
        // cant do this until private_chat_init is called
        //dwyco_command_from_keyboard(chan_id, 'u', 0, 0, "ready", 5);
        c->emit connect_established();
        c->call_setup_state_machine->start();
        c->mute_state_machine->start();
    }
    break;
    case DWYCO_CALLDISP_FAILED:
    case DWYCO_CALLDISP_CANCELED:
    case DWYCO_CALLDISP_REJECTED:
    case DWYCO_CALLDISP_REJECTED_PASSWORD_INCORRECT:
    case DWYCO_CALLDISP_REJECTED_BUSY:
        // in this case, the other side has gotten out of "simple call" mode, so
        // we should just bail out too.

        Mainwinform->del_call(c->uid, chan_id);
        call_del_by_uid(c->uid);
        c->emit connect_failed();
        c->chan_id = -1;
        c->call_id = -1;
        c->send_state_machine->stop();
        c->recv_state_machine->stop();
        c->call_setup_state_machine->stop();
        c->mute_state_machine->stop();
        c->reset_call_buttons();
        c->stop_accept_timer();

        break;


    case DWYCO_CALLDISP_TERMINATED:
    {
        Mainwinform->del_call(c->uid, chan_id);
        call_del_by_uid(c->uid);
        c->emit connect_terminated();
        c->chan_id = -1;
        c->call_id = -1;
        c->send_state_machine->stop();
        c->recv_state_machine->stop();
        c->call_setup_state_machine->stop();
        c->reset_call_buttons();
        c->stop_accept_timer();
        c->mute_state_machine->stop();
        c->rmute = -1;
        c->rcts = -1;
        //Mainwinform->decorate_users();
        Mainwinform->set_refresh_user(c->uid);
        c->reset_title();
        break;
    }
    default:
        cdcxpanic("bad call disp");
    }
}

void
simple_call::camera_event(int on)
{
    ui->actionStart_snapshot_timer->setChecked(0);

    if(on)
    {
        int send_pic = ui->actionSend_snapchat->isChecked() && !Public_chat_video_pause;
        new_zap();
        if(send_pic)
        {
            if(!dwyco_zap_record(zap_id, 0, 0, 1, -1,
                                 0, 0, &record_preview_id))
            {
            }
            ui->send_preview->ui_id = record_preview_id;
        }
        send_user_command(chan_id, "cam on");
        emit cam_is_on();
        start_control(1);
        ui->actionCam_on->setChecked(1);
        ui->actionPause->setEnabled(1);
        ui->actionPause->setChecked(0);
        ui->actionPause->setVisible(1);
        ui->actionRefresh->setVisible(1);
        ui->actionSend_snapchat->setVisible(1);
        ui->actionSend_snapchat->setEnabled(1);
        ui->actionStart_snapshot_timer->setVisible(1);
        ui->actionStart_snapshot_timer->setEnabled(1);
        ui->countdown_label->setVisible(1);

        ui->snap_timer->setVisible(1);
        ui->send_snapchat->setVisible(1);
        ui->send_preview->setVisible(1);
        ui->verticalSpacer->changeSize(20, 40);
        ui->verticalSpacer_2->changeSize(20, 40);
        ui->textEdit->setMaximumHeight(16777215);
        ui->verticalLayout->invalidate();
        ui->verticalLayout_2->invalidate();

    }
    else
    {
        // camera is turning off, just terminate our outgoing stream.
        // send a control message to other side saying camera is off now.
        dwyco_channel_stop_send_video(chan_id);
        preview_image = QImage();
        show_preview = 0;
        dwyco_delete_zap_composition(zap_id);
        new_zap();
        render_image();
        send_user_command(chan_id, "cam off");
        if(!HasCamHardware)
            send_user_command(chan_id, "cam none");
        emit cam_is_off();
        ui->actionCam_off->setChecked(1);
        ui->actionPause->setEnabled(0);
        ui->actionPause->setVisible(0);
        ui->actionRefresh->setVisible(0);
        ui->actionSend_snapchat->setVisible(0);
        ui->actionStart_snapshot_timer->setVisible(0);
        ui->countdown_label->setVisible(0);

        ui->snap_timer->setVisible(0);
        ui->send_snapchat->setVisible(0);
        ui->send_preview->setVisible(0);
        ui->verticalSpacer->changeSize(0, 0);
        ui->verticalSpacer_2->changeSize(0, 0);
        ui->textEdit->setMaximumHeight(27);
        ui->verticalLayout->invalidate();
        ui->verticalLayout_2->invalidate();
    }
}

void
simple_call::cam_control_off(bool a)
{
    if(a)
    {
        if(DevSelectBox)
            DevSelectBox->toggle_device(0);
        ui->actionPause->setEnabled(0);
    }

}

void
simple_call::cam_control_on(bool a)
{
    if(a)
    {
        if(DevSelectBox)
            DevSelectBox->toggle_device(1);
        ui->actionPause->setEnabled(1);

    }

}

void
simple_call::mouse_stopped_event(QPoint pnt)
{
    if(!ui->label->underMouse())
        return;
#ifdef POPUP_TOOLBAR
    toolbar->setVisible(1);
#endif
    show_preview = 1;
}

void
simple_call::start_media_calls()
{
    if(call_exists_by_uid(uid))
    {
        emit connect_already_exists();
        return;
    }
    if(dwyco_get_pals_only() && !dwyco_is_pal(uid.c_str(), uid.length()))
    {
        emit connect_failed();
        return;
    }
    dwyco_channel_create(uid.c_str(), uid.length(), dwyco_simple_calldisp,
                         (void *)vp.cookie, dwyco_simple_call_status2, (void *)vp.cookie,
                         "",
                         SIMPLE_CALL_TYPE, strlen(SIMPLE_CALL_TYPE), 0);
}

void
simple_call::render_image()
{
    if(remote_image.isNull())
        return;

    QImage ri_scaled = remote_image.scaled(ui->label->size(),
                                           Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if(show_preview && !preview_image.isNull())
    {
        QPainter p(&ri_scaled);

        p.drawImage(QRectF(ri_scaled.width() - 80, ri_scaled.height() - 60, 80, 60), preview_image, preview_image.rect());

    }
#if 0
    if(zap_id != -1 && record_preview_id != -1 && !record_preview_image.isNull())
    {
        // if we have a zap with a pic loaded and ready to go, show that here
        QPainter p(&ri_scaled);

        p.drawImage(QRectF(ri_scaled.width() - 160, ri_scaled.height() - 60, 80, 60), record_preview_image, record_preview_image.rect());

    }
#endif
    if(rpause)
    {
        QPainter p(&ri_scaled);

        p.drawStaticText(0, 0, paused_text);
    }
    if(rcam_on == 0)
    {
        QPainter p(&ri_scaled);
        if(!rcam_none)
            p.drawStaticText(0, 0, off_text);
        else
            p.drawStaticText(0, 0, no_cam_hardware_text);
    }
    ui->label->QLabel::setPixmap(QPixmap::fromImage(ri_scaled));
    // if resize events come in, we don't want to scale the
    // little preview image too
    ui->label->local_pixmap = QPixmap::fromImage(remote_image);
}

void
simple_call::set_preview_image(const QImage &qi)
{
    if(ui->actionPause->isChecked())
        return;
    preview_image = qi;
    if(isVisible())
        render_image();
}

void
simple_call::set_remote_image(int chan, const QImage &qi)
{
    if(chan == chan_id)
        remote_image = qi;
    else if(chan == record_preview_id)
        record_preview_image = qi;
    if(chan == chan_id || chan == record_preview_id)
        render_image();

}

void
simple_call::leaveEvent(QEvent *e)
{
#if 0
    if(!ui->actionPause->isChecked())
    {
        toolbar->setVisible(0);
        show_preview = 0;
    }
#endif
    QDockWidget::leaveEvent(e);
}

void
simple_call::resizeEvent(QResizeEvent *)
{
    cursor_to_bottom(ui->textBrowser);
}

void
simple_call::reposition_toolbar()
{
    toolbar->move(3, ui->label->height() - toolbar->height() - 8);
}

void
simple_call::handle_toolbar()
{
    if(!ui->actionPause->isChecked())
    {
        toolbar->setVisible(0);
        show_preview = 0;
    }

}

void simple_call::on_actionPause_triggered(bool state)
{
    //Public_chat_video_pause = state;
}

void simple_call::on_actionVideo_options_triggered()
{
    DevSelectBox->show();
    DevSelectBox->raise();
}


// msgs sent during the call are previewed as a web page, kinda like
// in the public chat area
int
simple_call::preview_saved_msg(DwOString mid, DwOString& preview_fn, int& video, int& short_video, int& audio, int& file, QString& local_time)
{
    preview_fn = add_pfx(Sys_pfx, "no_img.png");
    video = 0;
    audio = 0;
    short_video = 0;
    file = 0;

    DWYCO_SAVED_MSG_LIST qsm;
    if(!dwyco_get_saved_message(&qsm, uid.c_str(), uid.length(), mid.c_str()))
    {
        return 0;
    }
    simple_scoped sm(qsm);
    local_time = gen_time(sm, 0);
    DwOString fn;
    int is_file = dwyco_get_attr(sm, 0, DWYCO_QM_BODY_FILE_ATTACHMENT).length() > 0;
    if(is_file)
    {
        file = 1;
        //DwOString filename = dwyco_get_attr(sm, 0, DWYCO_QM_BODY_FILE_ATTACHMENT);
        //dwyco_list_release(sm);
        // chop everything off except the extension, then add our random name on the
        // front of it. avoids problems with goofy filenames being sent in.
        int idx;
#if 0
        int idx = filename.rfind(".");
        if(idx == DwOString::npos)
            return 0;

        filename.erase(0, idx);
#endif
        DwOString rfn;
        rfn = random_fn();
        rfn += ".itm";
        // copy file out to random filename, scaling to preview size, then emitting the
        // html.
        rfn = add_pfx(Tmp_pfx, rfn);
        if(!dwyco_copy_out_file_zap2(mid.c_str(), rfn.c_str()))
            return 0;
        QPixmap q(rfn.c_str());
        if(q.isNull())
        {
            // prevent crumbs, if we can't preview it, remove it
            QFile::remove(rfn);
            return 0;
        }
        QFile::remove(rfn);
        q = q.scaled(240, 180, Qt::KeepAspectRatio);
        // change into a ppm
        idx = rfn.rfind(".");
        if(idx == DwOString::npos)
            return 0;
        rfn.erase(idx);
        rfn += ".ppm";
        q.save(rfn.c_str());
        preview_fn = rfn;
        return 1;
    }
    if(dwyco_get_attr(sm, 0, DWYCO_QM_BODY_ATTACHMENT).length() == 0)
    {
        //dwyco_list_release(sm);
        return 0;
    }
    int viewid = dwyco_make_zap_view2(sm, 0);
    if(viewid == 0)
    {
        //dwyco_list_release(sm);
        return 0;
    }
    // create random filename
    fn = random_fn();
    fn += ".ppm";
    fn = add_pfx(Tmp_pfx, fn);
    int ret = 0;
    if(dwyco_zap_create_preview(viewid, fn.c_str(), fn.length()))
    {
        preview_fn = fn;
        ret = 1;
    }
    dwyco_zap_quick_stats_view(viewid, &video, &audio, &short_video);

    dwyco_delete_zap_view(viewid);
    //dwyco_list_release(sm);
    return ret;
}

static
QString
html_local_preview(DwOString uid, QString chat, DwOString picfn)
{

    int no_img = 2;
    setting_get("chat_pic_size", no_img);
    chat = strip_html(chat);

    QString name = dwyco_info_to_display(uid);
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

    if(no_img == 0)
    {
        QString ret;
        ret = nm + QString(" : ") + chat;
        return ret;
    }


// %1 = name text
// %2 = img
// %3 = chat text
    QString html(

        "<p align=\"right\" <div><table  border=\"0\" cellpadding=\"1\" cellspacing=\"1\" >"
        "    <tr>"
        "      <td valign=\"top\" align=\"right\" height=\"24px\"  ><span>%1</span>"
        "      </td>"
        "      <td  rowspan=2 valign=\"middle\" align=\"right\" height=\"90px\"  ><span>%2</span>"
        "      </td>"
        "    </tr>"
        "    <tr>"
        "      <td style=\"width:100%\" valign=\"top\" align=\"right\"  ><span>%3</span>"
        "      </td>"
        "    </tr>"
        "</table></div></p>"

    );


    QPixmap img;
    img.load(picfn.c_str());
    DwOString fn;

    if(picfn.endsWith("no_img.png"))
    {
        img = img.scaled(32, 32);
        fn = "no_img_32";
    }
    else
    {
        if(no_img == 1)
            img = img.scaled(120, 90, Qt::KeepAspectRatio);
        else
            img = img.scaled(240, 180, Qt::KeepAspectRatio);
        fn = random_fn();
    }

    fn += ".ppm";
    fn = add_pfx(Tmp_pfx, fn);
    if(!QFile::exists(fn.c_str()))
        img.save(fn.c_str());


    QString imghtml = QString("<img src = \"%1\">").arg(fn.c_str());

    QString ret;
    ret = html.arg(nm, imghtml, chat);


    return ret;

}

static
QString
gen_av_indicator(int video, int short_video, int audio, int file)
{
    if(file)
    {
        return "<img src= \":/new/prefix1/downlaod-16x16.png\">";
    }
    QString a;
    if(video && !short_video)
        a = "<img src=\":/new/prefix1/media 2-16x16.png\">";
    if(audio)
        a += "<img src=\":/new/prefix1/music 2-16x16.png\">";
    return a;
}

static
QString
html_from_data(QByteArray quid, QString name, QString chat, DwOString picfn,
               int video, int short_video, int audio, int file, QString local_time, DwOString mid)
{
    QString html(
        "<div ><table  border=\"0\" cellpadding=\"1\" cellspacing=\"1\" >"
        "<tr valign=\"top\">"
        "<td  rowspan=2 valign=\"middle\" height=\"90px\" ><span>%1</span>"
        "</td>"
        "<td ><span >%2 %5 %4</span>"
        "</td>"
        "</tr>"
        "<tr valign=\"top\">"
        "<td style=\"width:100%\" ><span >%3</span>"
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
    nm += ",";
    nm += mid.c_str();
    nm += ",";
    nm += video ? "1" : "0";
    nm += short_video ? "1" : "0";
    nm += audio ? "1" : "0";
    nm += file ? "1" : "0";
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
    // these indicators are not appropriate in private chat maybe, since
    // the public chat context of the two chatters might be different.
    // at least, it might be confusing.
    // since god is a global attribute, we show that above.
#if 0
    else if(uid_attrs_has_attr(dduid, "ui-demigod"))
    {
        nm.prepend("<font color=#ff00ff><sup>minion</sup></font>");
    }
    else if(uid_attrs_has_attr(dduid, "ui-subgod"))
    {
        nm.prepend("<font color=#ff00ff><sup>owner</sup></font>");
    }
#endif


    QString imghtml = QString("<img src = \"%1\">").arg(picfn.c_str());

    QString ret;

    int no_img = 2;
    setting_get("chat_pic_size", no_img);
    //if(!is_trivia(quid))
    chat = strip_html(chat);
    QString ltime = QString("<font size=\"2\">") + local_time + QString("</font>");
    if(no_img == 0)
    {
        ret = nm + QString(" : ") + chat;
    }
    else
    {
        ret = html.arg(imghtml, nm, chat, gen_av_indicator(video, short_video, audio, file), ltime);
    }

    return ret;
}

void
simple_call::display_new_msg(DwOString uid, DwOString txt, DwOString mid)
{
    if(uid != this->uid)
        return;
    DwOString pfn;
    int video;
    int short_video;
    int audio;
    int file;
    QString local_time;
    int worked = preview_saved_msg(mid, pfn, video, short_video, audio, file, local_time);

    int no_img = 2;
    setting_get("chat_pic_size", no_img);

    QPixmap img;
    img.load(pfn.c_str());
    DwOString fn;
    if(!worked)
    {
        img = img.scaled(32, 32, Qt::KeepAspectRatio);
        fn = "common_icon";
    }
    else
    {
        if(no_img == 1)
            img = img.scaled(120, 90, Qt::KeepAspectRatio);
        else
            img = img.scaled(240, 180, Qt::KeepAspectRatio);
        fn = random_fn();
    }
    fn += ".ppm";
    fn = add_pfx(Tmp_pfx, fn);
    if(!QFile::exists(fn.c_str()))
        img.save(fn.c_str());

    QString entry = html_from_data(QByteArray(uid.c_str(), uid.length()), dwyco_info_to_display(uid), txt.c_str(), fn,
                                   video, short_video, audio, file, local_time, mid);

    QScrollBar *sb = ui->textBrowser->verticalScrollBar();
    int to_bottom = 1;
    if(sb && sb->value() != sb->maximum())
        to_bottom = 0;
    append_msg_to_textedit(entry, ui->textBrowser, mid);
    if((video && !short_video) || audio || file)
    {
        // automatically pop up the viewer box for playable messages
        context_uid = uid;
        context_mid = mid;
        on_actionView_attachment_triggered();
    }
    if(to_bottom)
    {
        if(!vis || !msg_countdown.isActive())
        {
            play_sound("space-zap.wav");
        }
        cursor_to_bottom(ui->textBrowser);
    }
    else
    {
        // bottom may not be visible, so alert
        play_sound("space-zap.wav");
    }
    msg_countdown.stop();
    msg_countdown.setSingleShot(1);
    msg_countdown.setInterval(5 * 60 * 1000);
    msg_countdown.start();
}

void
simple_call::clear_chatwin()
{
    ui->textBrowser->clear();
    ui->textEdit->clear();
}

static
void
DWYCOCALLCONV
msg_send_callback(int /*id*/, int what, const char *recip, int recip_len, const char *server_msg, void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    simple_call *c = (simple_call *)(void *)vp;
    c->ui->send_button->setText("Send");
    switch(what)
    {
    case DWYCO_MSG_SEND_OK:
    case DWYCO_MSG_IGNORED: // note: ignore is silent fail
    case DWYCO_MSG_ATTACHMENT_DECLINED:
        //play_sound("space-incoming.wav");
        break;
    case DWYCO_MSG_SEND_FAILED:
    case DWYCO_MSG_ATTACHMENT_FAILED:
        //play_sound("space-call.wav");
        //dwyco_zap_defer_send(id);
        break;
    }
    //c->blink = 0;
}

static
void
DWYCOCALLCONV
msg_status_callback(int /*id*/, const char *msg, int percent, void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    simple_call *c = (simple_call *)(void *)vp;

    //c->ui->send_button->setText(msg);
    c->ui->send_button->setText(QString::number(percent) + QString("%"));
}

void
simple_call::pers_msg_send_status(int status, DwOString pid, DwOString ruid)
{
    if(pid != pers_id)
        return;
    if(status == DWYCO_SE_MSG_SEND_START)
        return;
    msg_send_callback(0, status == DWYCO_SE_MSG_SEND_FAIL ? DWYCO_MSG_SEND_FAILED : DWYCO_MSG_SEND_OK,
                      ruid.c_str(), ruid.length(), "", (void *)vp.cookie);
}

void
simple_call::msg_progress(DwOString pid, DwOString ruid, DwOString msg, int percent)
{
    if(pid != pers_id)
        return;
    msg_status_callback(0, msg.c_str(), percent, (void *)vp.cookie);
}

void simple_call::on_send_button_clicked()
{
    QString a;
    QString b;
    b = ui->textEdit->toPlainText();
    a = b;
    QByteArray txt = b.toAscii();
    int no_forward = 0;
    const char *val;
    int len;
    int type;
    if(dwyco_get_setting("zap/no_forward_default", &val, &len, &type))
    {
        if(type == DWYCO_TYPE_STRING || type == DWYCO_TYPE_INT)
        {
            no_forward = atoi(val);
        }
    }

    DwOString fn = random_fn();
    fn += ".ppm";
    fn = add_pfx(Tmp_pfx, fn);

    DwOString preview_fn = add_pfx(Sys_pfx, "no_img.png");
    int send_pic = ui->actionSend_snapchat->isChecked() && !Public_chat_video_pause;

    if(send_pic)
    {
        if(dwyco_zap_create_preview(zap_id, fn.c_str(), fn.length()))
        {
            preview_fn = fn;

        }
    }

    // if not sending pic, redo the zap composition to obliterate any
    // attachments.
    if(!send_pic)
    {
        dwyco_delete_zap_composition(zap_id);
        new_zap();
    }

//    if(!dwyco_zap_send3(zap_id, uid.c_str(), uid.length(), txt.constData(), txt.length(),
//        no_forward,
//        msg_send_callback, (void *)vp.cookie,
//        msg_status_callback, (void *)vp.cookie))
    const char *pid = "";
    int len_pid = 0;
    if(!dwyco_zap_send4(zap_id, uid.c_str(), uid.length(), txt.constData(), txt.length(), no_forward,
                        &pid, &len_pid))
    {
        // this can happen if the time between the automatic "record" we do below
        // and the time we re-enter to send is shorter than the time it takes to
        // take the snapshot (ie, the cam is taking its time to give us another frame.)
        // it can also happen if the cam drivers are stuck, never giving us a
        // frame.
        b += " PIC NOT SENT (quit and restart cdc-x if problems continue)";
        a = b;
        preview_fn = add_pfx(Sys_pfx, "no_img.png");
        dwyco_delete_zap_composition(zap_id);
        new_zap();
//        if(!dwyco_zap_send3(zap_id, uid.c_str(), uid.length(), txt.constData(), txt.length(),
//            no_forward,
//            msg_send_callback, (void *)vp.cookie,
//            msg_status_callback, (void *)vp.cookie))
        if(!dwyco_zap_send4(zap_id, uid.c_str(), uid.length(), txt.constData(), txt.length(), no_forward,
                            &pid, &len_pid))
        {
            b += " MSG SEND FAILED";
            a = b;
            pid = "";
            len_pid = 0;
        }

    }
    pers_id = DwOString(pid, 0, len_pid);



    ui->textEdit->setHtml("");
    emit send_msg_event(uid);

    QString entry = html_local_preview(My_uid, a, preview_fn);

    append_msg_to_textedit(entry, ui->textBrowser, "");
    // move to bottom on all sends
    cursor_to_bottom(ui->textBrowser);

    // note: dll deletes the composition on successful send
    new_zap();
    if(HasCamera && send_pic)
    {
        if(!dwyco_zap_record(zap_id, 0, 0, 1, -1,
                             0, 0, &record_preview_id))
        {
        }
        ui->send_preview->ui_id = record_preview_id;
    }
    emit try_connect();
    Visible_to.insert(uid);
#ifdef DWYCO_QT5
    QApplication::inputMethod()->hide();
#endif
    //blink = 1;

}

void simple_call::on_actionPause_toggled(bool arg1)
{
    dwyco_field_debug("sc-pause-vid", 1);
    dwyco_delete_zap_composition(zap_id);
    new_zap();

    if(arg1)
    {
        send_user_command(chan_id, "pause video");
    }
    else
    {
        if(HasCamera && ui->actionSend_snapchat->isChecked())
        {
            if(!dwyco_zap_record(zap_id, 0, 0, 1, -1,
                                 0, 0, &record_preview_id))
            {
            }
            ui->send_preview->ui_id = record_preview_id;
        }
        send_user_command(chan_id, "unpause video");
    }
}


void simple_call::on_textBrowser_anchorClicked(const QUrl &url)
{
    QByteArray b = url.path().toAscii().constData();
    QList<QByteArray> elems = b.split(',');
    if(elems.count() != 3)
        return;
    dwyco_field_debug("sc-text-browser", 1);

    context_uid = from_hex(elems[0].constData());
    context_mid = elems[1].constData();
    int video;
    int short_video;
    int audio;
    int file;
    video = (elems[2][0] == '1');
    short_video = (elems[2][1] == '1');
    audio = (elems[2][2] == '1');
    file = (elems[2][3] == '1');
    delete popup_menu;
    popup_menu = new QMenu(this);
    if((video && !short_video) || audio || file)
        popup_menu->addAction(ui->actionView_attachment);
    popup_menu->addAction(ui->actionView_profile_ctx);
    popup_menu->addAction(ui->actionCompose_msg_ctx);
    popup_menu->addAction(ui->actionSend_file_ctx);
    popup_menu->addAction(ui->actionIgnore_user_ctx);

    emit uid_selected(from_hex(url.path().toAscii().constData()), 1);
    popup_menu->popup(QCursor::pos());
}

void simple_call::on_actionView_profile_ctx_triggered()
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

void simple_call::on_actionCompose_msg_ctx_triggered()
{
    composer *c = new composer(COMP_STYLE_REGULAR, 0, Mainwinform);
    c->set_uid(context_uid);
    c->show();
    c->raise();

}

void simple_call::on_actionIgnore_user_ctx_triggered()
{
    if(confirm_ignore())
    {
        dwyco_ignore(context_uid.c_str(), context_uid.length());
        deleteLater();
    }
    emit ignore_event(context_uid);

}

void simple_call::on_actionView_attachment_triggered()
{
    //ui->label->setVisible(0);
    ui->msgplayer->setVisible(1);
    ui->msgplayer->preview_saved_msg(context_uid, context_mid);
    ui->msgplayer->ui.close_button->show();
}

void
simple_call::hide_msgplayer()
{
    ui->msgplayer->setVisible(0);

}

void simple_call::on_accept_clicked()
{
    send_user_command(chan_id, "recv ok");

}

void simple_call::on_accept_and_send_clicked()
{
    send_user_command(chan_id, "recv ok");

}

void simple_call::on_send_video_clicked()
{
    send_user_command(chan_id, "recv?");
    if(HasCamHardware && !HasCamera)
        cam_control_on(1);

}

void simple_call::on_hangup_clicked()
{
    dwyco_destroy_channel(chan_id);
    dwyco_cancel_call(call_id);
    call_del_by_uid(uid);
    call_del(chan_id);
    chan_id = -1;
    call_id = -1;
    allstop = 0;
    send_state_machine->stop();
    recv_state_machine->stop();
    call_setup_state_machine->stop();
    mute_state_machine->stop();
    audio_stream_state_machine->stop();
    full_duplex_audio_stream_state_machine->stop();
    reset_call_buttons();
    stop_accept_timer();
    rmute = -1;
    rcts = -1;
    //Mainwinform->decorate_users();
    Mainwinform->set_refresh_user(uid);
}

void
simple_call::handle_vis_change(bool a)
{
    // this doesn't work too well because qt seems to make the
    // visibility change a lot when it doing things like docking
    // and undocking things.
    vis = a;
    // any visibility change, cancel timer
    ui->actionStart_snapshot_timer->setChecked(0);
}

void simple_call::on_reject_clicked()
{
    send_user_command(chan_id, "recv reject");

}


void simple_call::on_actionSend_file_ctx_triggered()
{
    send_file_common(QByteArray(context_uid.c_str(), context_uid.length()));
}

void
simple_call::process_ignore_event(DwOString uid)
{
    if(this->uid != uid)
        return;
    if(!dwyco_is_ignored(uid.c_str(), uid.length()))
    {
        connect_state_machine->start();
        emit try_connect();
        return;
    }
    dwyco_destroy_channel(chan_id);
    dwyco_cancel_call(call_id);
    chan_id = -1;
    call_id = -1;
    allstop = 0;

    send_state_machine->stop();
    recv_state_machine->stop();
    call_setup_state_machine->stop();
    connect_state_machine->stop();
    mute_state_machine->stop();
    audio_stream_state_machine->stop();
    full_duplex_audio_stream_state_machine->stop();
    reset_call_buttons();
    stop_accept_timer();
    deleteLater();
}

void simple_call::on_actionRefresh_triggered()
{
    dwyco_delete_zap_composition(zap_id);
    new_zap();
    int send_pic = ui->actionSend_snapchat->isChecked();
    if(HasCamera && send_pic)
    {
        if(!dwyco_zap_record(zap_id, 0, 0, 1, -1,
                             0, 0, &record_preview_id))
        {
        }
        ui->send_preview->ui_id = record_preview_id;
    }
}

void simple_call::do_refresh()
{
    dwyco_delete_zap_composition(zap_id);
    new_zap();
    if(HasCamera)
    {
        if(!dwyco_zap_record(zap_id, 0, 0, 1, -1,
                             0, 0, &record_preview_id))
        {
        }
        ui->send_preview->ui_id = record_preview_id;
    }
}

void simple_call::on_cancel_req_clicked()
{
    send_user_command(chan_id, "recv cancel");

}

void simple_call::on_actionZoom_in_toggled(bool arg1)
{
    dwyco_field_debug("sc-vid-zoom", 1);
    if(arg1)
    {
        ui->label->setMinimumSize(640, 480);
        ui->label->resize(640, 480);
        ui->actionZoom_in->setToolTip("Make picture smaller");
        if(isFloating())
            move(geometry().x(), 0);
        else
            Mainwinform->move(Mainwinform->geometry().x(), 0);
        resize_timer.stop();
        resize_timer.setSingleShot(1);
        resize_timer.start(250);
        if(Mainwinform->geometry().y() + Mainwinform->geometry().height() > MainScreenRect.height())
        {
            ui->textBrowser->setMinimumHeight(0);
            ui->send_preview->setMinimumSize(0, 0);
            ui->send_preview->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            Mainwinform->resize(Mainwinform->geometry().width(), MainScreenRect.height() - 20);
        }
    }
    else
    {
        ui->label->setMinimumSize(320, 240);
        ui->label->resize(320, 240);
        ui->actionZoom_in->setToolTip("Make picture bigger");
    }
}

// this is a hack to make sure the bottom of the app doesn't fall off the bottom of the screen
// it doesn't seem to work too well on linux because of window
// manager geometry goofiness.

void
simple_call::check_resize()
{
    if(!isFloating())
    {
        if(Mainwinform->geometry().y() + Mainwinform->geometry().height() > MainScreenRect.height())
        {
            ui->textBrowser->setMinimumHeight(0);
            ui->send_preview->setMinimumSize(0, 0);
            ui->send_preview->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            Mainwinform->resize(Mainwinform->geometry().width(), MainScreenRect.height() - 20);
        }
    }

}

void simple_call::on_compose_button_clicked()
{
    composer *c = new composer(COMP_STYLE_REGULAR, 0, Mainwinform);
    c->set_uid(uid);
    c->show();
    c->raise();

}

void simple_call::on_send_file_button_clicked()
{
    dwyco_field_debug("sc-file-send", 1);
    send_file_common(uid);
}

void simple_call::on_show_profile_button_clicked()
{
    viewer_profile *c;


    if(!Allow_multiple_windows)
        viewer_profile::delete_all();
    c = viewer_profile::get_viewer_profile(uid);
    if(!c)
    {
        c = new viewer_profile(this);
        c->set_uid(uid);
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

void
simple_call::process_content_filter_change(int)
{
    uid_resolved(uid);
}

void simple_call::on_pal_button_clicked()
{
    dwyco_field_debug("sc-pal", 1);
    dwyco_pal_add(uid.c_str(), uid.length());
    //Mainwinform->load_users();
    //Mainwinform->decorate_users();
    emit pal_event(uid);
}

void
simple_call::update_pal(DwOString uid)
{
    if(uid != this->uid)
        return;
    if(dwyco_is_pal(uid.c_str(), uid.length()))
    {
        ui->pal_button->setVisible(0);
    }
    else
    {
        ui->pal_button->setVisible(1);
        // if you unpal someone while in pals-only mode, drop
        // any existing connections.
        if(dwyco_get_pals_only())
        {
            dwyco_destroy_channel(chan_id);
            dwyco_cancel_call(call_id);
            chan_id = -1;
            call_id = -1;
            allstop = 0;

            send_state_machine->stop();
            recv_state_machine->stop();
            call_setup_state_machine->stop();
            connect_state_machine->stop();
            mute_state_machine->stop();
            audio_stream_state_machine->stop();
            full_duplex_audio_stream_state_machine->stop();
            reset_call_buttons();
            stop_accept_timer();
        }
    }

}

void simple_call::on_send_snapchat_clicked(bool checked)
{

}

void simple_call::on_actionStart_snapshot_timer_toggled(bool checked)
{
    if(checked == 1)
    {
        snapshot_countdown = 5;
        ui->countdown_label->setText(QString::number(snapshot_countdown));
        snapshot_timer.start();
    }
    else
    {
        snapshot_countdown = 5;
        ui->countdown_label->setText(QString::number(snapshot_countdown));
        snapshot_timer.stop();
    }
    dwyco_field_debug("sc-snapshot-timer", 1);

}

void
simple_call::snapshot_timeout()
{
    --snapshot_countdown;
    if(snapshot_countdown == 0)
    {
        snapshot_timer.stop();
        do_refresh();
        snapshot_countdown = 5;
        ui->actionStart_snapshot_timer->setChecked(0);
        // play a noise here
        play_sound("camera1.wav");
        return;
    }

    ui->countdown_label->setText(QString::number(snapshot_countdown));

}

void
simple_call::stop_snapshot_timer(bool)
{
    ui->actionStart_snapshot_timer->setChecked(0);
}

void simple_call::on_connect_button_clicked()
{
    emit try_connect();
    dwyco_field_debug("sc-connect", 1);

}

void
simple_call::disable_connect_button()
{

    if(call_exists(chan_id))
    {
        ui->connect_button->setEnabled(0);
        ui->connect_button->setChecked(1);
    }
}
