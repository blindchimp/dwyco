
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

// note ca 2018: this rough sketch isn't exactly how it is implemented
// but this gives you a rough idea of how the state machine is set up.
// ---
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

// inputs to this object are:
// cam on/off
// uid ignore event
// keyboard input active
//
// "buttons"
// send video
// unblock audio
// send audio
// accept video
// accept+send video
//
// output signals:
// remote cam off/on/none
// remote cam pause/unpause
// remote cts(audio) on/off
// remote keyboard active/inactive
// button enable/disable/vis/invis signals
//
//

#include <QState>
#include <QFinalState>
#include <QStateMachine>
#include <QSignalTransition>

#include "callsm.h"
//#include "ssmap.h"
//#include "tfhex.h"
//#include "evret.h"
#include "dlli.h"
#include "ct.h"
#include "callsm_objs.h"

extern int Public_chat_video_pause;
//extern DwOString My_uid;
int HasCamera;
int HasCamHardware;
int HasAudioInput;
int HasAudioOutput;

DwQueryByMember<simple_call> simple_call::Simple_calls;

DwOString dwyco_get_attr(DWYCO_LIST l, int row, const char *col);
QObject *simple_call::Mainwinform;


static int
send_user_command(int chan_id, DwOString cmd)
{
    if(chan_id == -1)
        return 0;
    dwyco_command_from_keyboard(chan_id, 'u', 0, 0, cmd.c_str(), cmd.length());
    return 1;
}

simple_call::simple_call(const DwOString& auid, QObject *parent) :
    QObject(parent),
    vp(this),
	keyboard_active_timer(this),
    uid(auid)
{
    ui = &smo;
    ui->setupUi(this);
    Simple_calls.add(this);
#if 0
    ReturnFilter *return_filter = new ReturnFilter;
    connect(return_filter, SIGNAL(chat_typing()), this, SLOT(keyboard_input()));
#endif
    //connect(return_filter, SIGNAL(return_hit()), this, SLOT(on_send_button_clicked()));
    //connect(return_filter, SIGNAL(esc_hit()), this, SLOT(clear_chatwin()));
	keyboard_active_timer.setSingleShot(1);
	connect(&keyboard_active_timer, SIGNAL(timeout()), this, SLOT(keyboard_inactive()));
	kb_active = 0;
    rem_kb_active = 0;

    //ui->textEdit->installEventFilter(return_filter);

    connect(ui->actionCam_off, SIGNAL(triggered(bool)), this, SLOT(cam_control_off(bool)));
    connect(ui->actionCam_on, SIGNAL(triggered(bool)), this, SLOT(cam_control_on(bool)));

    //connect(this, SIGNAL(pal_event(DwOString)), Mainwinform, SIGNAL(pal_event(DwOString)));
    //connect(Mainwinform, SIGNAL(pal_event(DwOString)), this, SLOT(update_pal(DwOString)));
    //connect(this, SIGNAL(ignore_event(DwOString)), Mainwinform, SIGNAL(ignore_event(DwOString)));
    connect(Mainwinform, SIGNAL(ignore_event(DwOString)), this, SLOT(process_ignore_event(DwOString)));
    connect(Mainwinform, SIGNAL(camera_change(int)), this, SLOT(camera_event(int)));
    //connect(Mainwinform, SIGNAL(video_capture_preview(QImage)), this, SLOT(set_preview_image(QImage)));
    //connect(Mainwinform, SIGNAL(video_display(int,QImage)), this, SLOT(set_remote_image(int, QImage)));
    //connect(Mainwinform, SIGNAL(control_msg(int,DwOString,int,int,DwOString)), this, SLOT(recv_control_msg(int,DwOString,int,int,DwOString)));
    connect(this, SIGNAL(rem_cam_off()), this, SLOT(react_to_rem_cam_off()));
    connect(this, SIGNAL(rem_cam_on()), this, SLOT(react_to_rem_cam_on()));
    connect(this, SIGNAL(rem_cam_none()), this, SLOT(react_to_rem_cam_none()));
    //connect(Mainwinform, SIGNAL(global_video_pause_toggled(bool)), ui->actionPause, SLOT(setChecked(bool)));
    //connect(Mainwinform, SIGNAL(global_video_pause_toggled(bool)), ui->actionPause, SLOT(setDisabled(bool)));
    //connect(Mainwinform, SIGNAL(uid_info_event(DwOString)), this, SLOT(uid_resolved(DwOString)));
    //connect(Mainwinform, SIGNAL(invalidate_profile(DwOString)), this, SLOT(uid_resolved(DwOString)));
    //connect(Mainwinform, SIGNAL(content_filter_event(int)), this, SLOT(process_content_filter_change(int)));

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
    
    connect(this, SIGNAL(rem_keyboard_active(int)), this, SLOT(rem_keyboard_active_with_uid(int)));
    connect(this, SIGNAL(rem_keyboard_active_uid(const QString&,int)), Mainwinform, SIGNAL(rem_keyboard_active(const QString&, int)));

    dwyco_get_audio_hw(&HasAudioInput, &HasAudioOutput, 0);

    chan_id = -1;
    call_id = -1;

    connect_state_machine = new QStateMachine(this);
    call_setup_state_machine = new QStateMachine(this);
    send_state_machine = new QStateMachine(this);
    recv_state_machine = new QStateMachine(this);
    audio_stream_state_machine = new QStateMachine(this);
    full_duplex_audio_stream_state_machine = new QStateMachine(this);
    mute_state_machine = new QStateMachine(this);

    {
        // state machine for keeping a control connection between caller and callee
        QState *start = new QState;
        QState *connecting = new QState;
        QState *established = new QState;
        QState *retrying = new QState;
        QState *no_auto_retry = new QState;
        QFinalState *final = new QFinalState;

        start->addTransition(this, SIGNAL(try_connect()), connecting);
        connecting->addTransition(this, SIGNAL(connect_established()), established);
        connecting->addTransition(this, SIGNAL(connect_failed()), no_auto_retry);
        connecting->addTransition(this, SIGNAL(connect_already_exists()), established);
        established->addTransition(this, SIGNAL(connect_terminated()), retrying);

        retrying->addTransition(&reconnect_timer, SIGNAL(timeout()), connecting);
        retrying->addTransition(this, SIGNAL(try_connect()), connecting);
        no_auto_retry->addTransition(this, SIGNAL(try_connect()), connecting);
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
        assign_button_states(start, 0, 0, 0, 0, 0, 0);

        wait_for_call->addTransition(this, SIGNAL(cam_is_on()), send_and_wait_for_call);
        wait_for_call->addTransition(this, SIGNAL(recv_query()), accept_video_query);
        assign_button_states(wait_for_call, 0, 0, 0, 0, 0, 0);

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

        assign_button_states(accept_video_query, 1, 0, 0, 1, 0, 0);

        send_and_wait_for_call->addTransition(this, SIGNAL(recv_query()), accept_and_send_video_query);
        send_and_wait_for_call->addTransition(ui->send_video, SIGNAL(clicked()), wait_rem_response);
        send_and_wait_for_call->addTransition(this, SIGNAL(cam_is_off()), wait_for_call);
        assign_button_states(send_and_wait_for_call, 0, 0, 1, 0, 0, 0);

        accept_and_send_video_query->addTransition(&accept_timer, SIGNAL(timeout()), send_and_wait_for_call);
        accept_and_send_video_query->addTransition(ui->accept_and_send, SIGNAL(clicked()), start_recv_and_send_machine);
        accept_and_send_video_query->addTransition(ui->reject, SIGNAL(clicked()), start);
        accept_and_send_video_query->addTransition(this, SIGNAL(rem_cam_off()), start);
        accept_and_send_video_query->addTransition(this, SIGNAL(recv_cancel()), start);
        accept_and_send_video_query->addTransition(this, SIGNAL(cam_is_off()), accept_video_query);
        QObject::connect(accept_and_send_video_query, SIGNAL(entered()), this, SLOT(start_accept_timer()));
        QObject::connect(accept_and_send_video_query, SIGNAL(exited()), this, SLOT(stop_accept_timer()));
        assign_button_states(accept_and_send_video_query, 0, 1, 0, 1, 0, 0);

        wait_rem_response->addTransition(this, SIGNAL(recv_ok()), start_send_machine);
        wait_rem_response->addTransition(this, SIGNAL(recv_query()), start_recv_and_send_machine);
        wait_rem_response->addTransition(&ask_timer, SIGNAL(timeout()), start);
        wait_rem_response->addTransition(this, SIGNAL(recv_reject()), start);
        wait_rem_response->addTransition(this, SIGNAL(cam_is_off()), start);
        wait_rem_response->addTransition(ui->cancel_req, SIGNAL(clicked()), start);
        QObject::connect(wait_rem_response, SIGNAL(entered()), this, SLOT(start_ask_timer()));
        QObject::connect(wait_rem_response, SIGNAL(exited()), this, SLOT(stop_ask_timer()));
        QObject::connect(&ask_timer, SIGNAL(timeout()), ui->cancel_req, SIGNAL(clicked()));
        assign_button_states(wait_rem_response, 0, 0, 0, 0, 0, 1);

        QObject::connect(start_send_machine, SIGNAL(entered()), send_state_machine, SLOT(start()));
        QObject::connect(start_send_machine, SIGNAL(entered()), recv_state_machine, SLOT(start()));

        QObject::connect(start_recv_machine, SIGNAL(entered()), recv_state_machine, SLOT(start()));
        QObject::connect(start_recv_machine, SIGNAL(entered()), send_state_machine, SLOT(start()));
        assign_button_states(start_recv_machine, 0, 0, 0, 0, 1, 0);


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
        //start->assignProperty(Mainwinform->vid_preview->ui->actionPause, "checked", 0);
        assign_button_states(start, 0, 0, 0, 0, 1, 0);
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
        //start->assignProperty(ui->label, "visible", 1);
        //start->assignProperty(ui->actionZoom_in, "checked", 0);
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
        //cts_wait->assignProperty(ui->send_audio_button, "styleSheet", " background-color: red; border: none; color: rgb(255, 255, 255);");
        cts_wait->assignProperty(ui->send_audio_button, "backgroundColor", QColor("red"));

        QObject::connect(play_incoming_record_off, SIGNAL(entered()), this, SLOT(set_half_duplex_play()));
        play_incoming_record_off->addTransition(ui->send_audio_button, SIGNAL(pressed()), play_off_record_on);
        play_incoming_record_off->addTransition(this, SIGNAL(rem_cts_off()), cts_wait);
        play_incoming_record_off->assignProperty(ui->send_audio_button, "enabled", 1);
        //play_incoming_record_off->assignProperty(ui->send_audio_button, "styleSheet", " background-color: green; border: none; color: rgb(255, 255, 255);");
        play_incoming_record_off->assignProperty(ui->send_audio_button, "backgroundColor", QColor("green"));
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
        //play_incoming_record_off->assignProperty(ui->send_audio_button, "styleSheet", " background-color: green; border: none; color: rgb(255, 255, 255);");
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
        //go->assignProperty(ui->mute_button, "styleSheet", " background-color: red; border: none; color: rgb(255, 255, 255);");
        go->assignProperty(ui->mute_button, "backgroundColor", QColor("red"));
        go->assignProperty(ui->send_audio_button, "enabled", 1);
        go->assignProperty(ui->send_audio_button, "visible", 1);

        lmute->addTransition(this, SIGNAL(rem_mute_on()), lrmute);
        lmute->addTransition(this, SIGNAL(mute_off()), go);
        lmute->assignProperty(ui->mute_button, "text", "Accept live audio");
        lmute->assignProperty(ui->send_audio_button, "visible", 0);
        //lmute->assignProperty(ui->mute_button, "styleSheet", " background-color: green; border: none; color: rgb(255, 255, 255);");
        lmute->assignProperty(ui->mute_button, "backgroundColor", QColor("green"));

        rmute->addTransition(this, SIGNAL(mute_on()), lrmute);
        rmute->addTransition(this, SIGNAL(rem_mute_off()), go);
        rmute->assignProperty(ui->mute_button, "text", "Remote audio blocked");
        //rmute->assignProperty(ui->mute_button, "styleSheet", " background-color: yellow; border: none; color: rgb(0, 0, 0);");
        rmute->assignProperty(ui->send_audio_button, "visible", 0);
        rmute->assignProperty(ui->mute_button, "backgroundColor", QColor("yellow"));

        lrmute->addTransition(this, SIGNAL(rem_mute_off()), lmute);
        lrmute->addTransition(this, SIGNAL(mute_off()), rmute);
        lrmute->assignProperty(ui->mute_button, "text", "Unblock audio");
        //lrmute->assignProperty(ui->mute_button, "styleSheet", " background-color: green; border: none; color: rgb(255, 255, 255);");
        lrmute->assignProperty(ui->send_audio_button, "visible", 0);
        lrmute->assignProperty(ui->mute_button, "backgroundColor", QColor("green"));
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

    //allstop = 0;
    rpause = 0;
    rcam_on = 1;
    rcam_none = 0;

    // initial audio state
    rcts = -1;
    rmute = -1;

    cts = 0;
    mute = 0;
    m_connected = 0;
    connect(this, SIGNAL(connectedChanged(int)), this, SLOT(connected_changed_uid(int)));
    connect(this, SIGNAL(sig_on_connected_change_uid(QString,int)), Mainwinform, SIGNAL(established_active(QString, int)));

}

void
simple_call::connected_changed_uid(int a)
{
    emit sig_on_connected_change_uid(QString(uid.toHex()), a);
}

void
simple_call::init(QObject *mainwin)
{
    dwyco_set_private_chat_init_callback(dwyco_private_chat_init);
    dwyco_set_private_chat_display_callback(dwyco_private_chat_display);
    dwyco_set_call_acceptance_callback(dwyco_call_accepted);
    dwyco_set_call_screening_callback(dwyco_call_screening_callback);
    Mainwinform = mainwin;
}

void
simple_call::assign_button_states(QState *s, int accept, int accept_and_send, int send_video, int reject, int hangup, int cancel_req)
{
    s->assignProperty(ui->accept, "visible", accept);
    s->assignProperty(ui->accept_and_send, "visible", accept_and_send);
    s->assignProperty(ui->send_video, "visible", send_video);
    s->assignProperty(ui->reject, "visible", reject);
    s->assignProperty(ui->hangup, "visible", hangup);
    s->assignProperty(ui->cancel_req, "visible", cancel_req);

}

void
simple_call::reset_call_buttons()
{
    //ui->label->setVisible(0);

    ui->accept->setVisible(0);
    ui->accept_and_send->setVisible(0);
    ui->send_video->setVisible(0);
    ui->reject->setVisible(0);
    ui->hangup->setVisible(0);
    ui->cancel_req->setVisible(0);
    ui->mute_button->setVisible(0);
    ui->send_audio_button->setVisible(0);
    ui->actionPause->setVisible(0);
    dwyco_set_all_mute(0);
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
    dwyco_set_refresh_users(1);
}

int
simple_call::can_stream()
{
    return send_state_machine->isRunning() || recv_state_machine->isRunning();
}

void
simple_call::start_control(bool a)
{
    if(a)
    {
#if 0
        if(dwyco_get_invisible_state() && !Visible_to.contains(uid))
            return;
#endif

        emit try_connect();
    }
}

simple_call::~simple_call()
{
    //delete ui;

    //dwyco_delete_zap_composition(zap_id);
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
    //start_animation(uid, ":/new/prefix1/connect_creating.png");
}
void
simple_call::stop_accept_timer()
{
    accept_timer.stop();
    //stop_animation(uid);
}

void
simple_call::start_ask_timer()
{

    ask_timer.setSingleShot(1);
    ask_timer.start(30000);
    //start_animation(uid, ":/new/prefix1/connect_creating.png");
}
void
simple_call::stop_ask_timer()
{
    ask_timer.stop();
    //stop_animation(uid);
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

// there may be a better way of doing this that preserves more
// state between suspend/resume cycles, but for now, we are
// draconian since all the network connections are killed, and
// we want to avoid QTimers going off and calling into uninitialized
// areas of the core on resume.

void
simple_call::suspend()
{
    QList<simple_call *> tmp(Simple_calls.objs);
    int n = tmp.count();
    for(int i = 0; i < n; ++i)
    {
        tmp[i]->update_connected(0);
        //delete tmp[i];
    }
}

simple_call *
simple_call::get_simple_call(DwOString uid)
{
    QList<simple_call *> c = Simple_calls.query_by_member(uid, &simple_call::uid);
    simple_call *sc = 0;
    if(c.count() == 0)
    {
        sc = new simple_call(uid);
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
simple_call::rem_keyboard_active_with_uid(int i)
{
    emit rem_keyboard_active_uid(QString(uid.toHex()), i);
}

void 
simple_call::keyboard_input()
{
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
            rem_kb_active = 1;
            emit rem_keyboard_active(1);
        }
        else if(str.eq("ki"))
        {
            rem_kb_active = 0;
            emit rem_keyboard_active(0);
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
simple_call::react_to_rem_cam_off()
{
    //remote_image = QImage(":/new/green32/gooduser.png");
    rcam_on = 0;
    //render_image();

}

void
simple_call::react_to_rem_cam_none()
{
    //remote_image = QImage(":/new/green32/gooduser.png");
    rcam_none = 1;
    rcam_on = 0;
    //render_image();

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


static void
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
simple_call::dwyco_simple_calldisp(int call_id, int chan_id, int what, void *arg, const char *cuid, int len_uid, const char *call_type, int len_call_type)
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
        // cant do this until private_chat_init is called
        //dwyco_command_from_keyboard(chan_id, 'u', 0, 0, "ready", 5);
        c->emit connect_established();
        c->call_setup_state_machine->start();
        c->mute_state_machine->start();
        c->update_connected(1);
    }
        break;
    case DWYCO_CALLDISP_FAILED:
    case DWYCO_CALLDISP_CANCELED:
    case DWYCO_CALLDISP_REJECTED:
    case DWYCO_CALLDISP_REJECTED_PASSWORD_INCORRECT:
    case DWYCO_CALLDISP_REJECTED_BUSY:
        // in this case, the other side has gotten out of "simple call" mode, so
        // we should just bail out too.

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
        c->update_connected(0);

        break;


    case DWYCO_CALLDISP_TERMINATED:
    {
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
        c->update_connected(0);
        break;
    }
    default:
        cdcxpanic("bad call disp");
    }
}

int
DWYCOCALLCONV
simple_call::dwyco_private_chat_init(int chan_id, const char *)
{
    QList<simple_call *> scl = simple_call::Simple_calls.query_by_member(chan_id, &simple_call::chan_id);
    if(scl.count() != 1)
        return 0;
    return 1;
}

int
DWYCOCALLCONV
simple_call::dwyco_private_chat_display(int chan_id, const char *com, int arg1, int arg2, const char *str, int len)
{
    QList<simple_call *> scl = simple_call::Simple_calls.query_by_member(chan_id, &simple_call::chan_id);
    if(scl.count() != 1)
        return 0;
#if 0
    scl[0]->display_new_msg(scl[0]->uid,
    QString("priv chat com chan %1 com %2 a1 %3 a2 %4 str %5 len %6").arg(chan_id).arg(com).arg(arg1).arg(arg2).arg(str).arg(len).toAscii().constData(), "");
#endif
    scl[0]->recv_control_msg(chan_id, DwOString(com, 0, strlen(com)), arg1, arg2, DwOString(str, 0, len));
    return 1;
}

// warning: relying on the simple_call context being valid until
// the call is terminated is a bit of a problem. it is ok for now,
// but maybe need to avoid deleting the sc until the call has
// died. problems in: remove user processing, and ignore processing
void
DWYCOCALLCONV
simple_call::call_died(int chan_id, void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    simple_call *sc = (simple_call *)(void *)vp;

    call_del(chan_id);
    sc->chan_id = -1;
    sc->call_id = -1;
    // we know we stopped it before because we accepted a call from it.
    sc->on_hangup_clicked();
    sc->connect_state_machine->start();
}

void
DWYCOCALLCONV
simple_call::dwyco_call_accepted(int chan_id, const char *name, const char *location, const char *uid, int len_uid, const char *call_type, int len_call_type)
{
    DwOString suid(uid, 0, len_uid);

    DwOString dct(call_type, 0, len_call_type);

    simple_call *sc = simple_call::get_simple_call(suid);

    dwyco_set_channel_destroy_callback(chan_id, call_died, (void *)sc->vp.cookie);

    dwyco_call_type ct(chan_id, call_type, len_call_type, uid, len_uid);

    if(!call_find(chan_id, ct))
    {
        call_add(ct);
    }
    sc->chan_id = chan_id;
    // note: once we receive a call successfully, we turn ourselves
    // into a slave, and avoid trying to set up control call from this
    // end.
    sc->connect_state_machine->stop();
    sc->call_setup_state_machine->start();
    sc->mute_state_machine->start();
    sc->update_connected(1);
}

int
DWYCOCALLCONV
simple_call::dwyco_call_screening_callback(int chan_id,
                              int remote_wants_to_recv_your_video, int remote_wants_to_send_you_video,
                              int remote_wants_to_recv_your_audio, int remote_wants_to_send_you_audio,
                              int remote_wants_to_exchange_pubchat, int remote_wants_to_exchange_privchat,
                              const char *call_type, int len_call_type,
                              const char *uid, int len_uid,
                              int *accept_call_style,
                              char **error_msg)
{
    DwOString suid(uid, 0, len_uid);
    DwOString ct(call_type, 0, len_call_type);

    // need to check if ignore processing is handled in the dll anymore
    if(dwyco_is_ignored(uid, len_uid))
    {
        *accept_call_style = DWYCO_CSC_REJECT_CALL;
        return 0;
    }
    // new style: the "call" is just a set up of the channels without
    // any streaming going, so we just test for duplicates (not allowed.)
    if(call_exists_by_uid(suid))
    {
        *accept_call_style = DWYCO_CSC_REJECT_CALL;
        return 0;
    }
#if 0
    // if we are invisible, and we are not "visible to user", then
    // reject the call. note this is an info leak if it got this far,
    // but better than allowing the call.
    if(dwyco_get_invisible_state())
    {
        if(!Visible_to.contains(suid))
        {
            *accept_call_style = DWYCO_CSC_REJECT_CALL;
            return 0;
        }
    }
#endif

    if(dwyco_get_pals_only() && !dwyco_is_pal(uid, len_uid))
    {
        *accept_call_style = DWYCO_CSC_REJECT_CALL;
        return 0;
    }
    dwyco_call_type cto(chan_id, call_type, len_call_type, uid, len_uid, DWYCO_CT_RECV);
    call_add(cto);
    *accept_call_style = DWYCO_CSC_ACCEPT_CALL;
    return 0;

}



void
simple_call::camera_event(int on)
{

    if(on)
    {
        send_user_command(chan_id, "cam on");
        emit cam_is_on();
        start_control(1);
        ui->actionCam_on->setChecked(1);
        ui->actionPause->setEnabled(1);
        ui->actionPause->setChecked(0);
        ui->actionPause->setVisible(1);
        //ui->actionRefresh->setVisible(1);

    }
    else
    {
        // camera is turning off, just terminate our outgoing stream.
        // send a control message to other side saying camera is off now.
		dwyco_channel_stop_send_video(chan_id);
        show_preview = 0;
        render_image();
        send_user_command(chan_id, "cam off");
        if(!HasCamHardware)
            send_user_command(chan_id, "cam none");
        emit cam_is_off();
        ui->actionCam_off->setChecked(1);
        ui->actionPause->setEnabled(0);
        ui->actionPause->setVisible(0);
        //ui->actionRefresh->setVisible(0);
    }
}

void
simple_call::cam_control_off(bool a)
{
    if(a)
    {
        ui->actionPause->setEnabled(0);
    }

}

void
simple_call::cam_control_on(bool a)
{
    if(a)
    {
        ui->actionPause->setEnabled(1);

    }

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
#if 0
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
#endif
}

void
simple_call::set_preview_image(const QImage &qi)
{
#if 0
    if(ui->actionPause->isChecked())
        return;
    preview_image = qi;
    if(isVisible())
        render_image();
#endif
}

void
simple_call::set_remote_image(int chan, const QImage &qi)
{
#if 0
    if(chan == chan_id)
        remote_image = qi;
    else if(chan == record_preview_id)
        record_preview_image = qi;
    if(chan == chan_id || chan == record_preview_id)
        render_image();
#endif

}


void simple_call::on_actionPause_toggled(bool arg1)
{
    if(arg1)
    {
        send_user_command(chan_id, "pause video");
    }
    else
    {
        send_user_command(chan_id, "unpause video");
    }
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

}

void simple_call::on_hangup_clicked()
{
    dwyco_destroy_channel(chan_id);
    dwyco_cancel_call(call_id);
    call_del_by_uid(uid);
    call_del(chan_id);
    chan_id = -1;
    call_id = -1;
    //allstop = 0;
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
    update_connected(0);
}

void simple_call::on_reject_clicked()
{
    send_user_command(chan_id, "recv reject");

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
    //allstop = 0;
    reset_call_buttons();
    stop_accept_timer();
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

void simple_call::on_cancel_req_clicked()
{
    send_user_command(chan_id, "recv cancel");

}

