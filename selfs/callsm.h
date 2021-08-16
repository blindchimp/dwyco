
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CALLSM_H
#define CALLSM_H

#include <QTimer>
#include <QStateMachine>
#include "QQmlVarPropertyHelpers.h"
#include "dvp.h"
#include "dwquerybymember.h"
#include "dlli.h"
#include "callsm_objs.h"

class simple_call : public QObject
{
    Q_OBJECT
    QML_CONSTANT_VAR_PROPERTY(QString, uid)
    QML_READONLY_VAR_PROPERTY(int, connected)
    QML_READONLY_VAR_PROPERTY(bool, sending_video)

private:
    friend class DwycoCore;
    static void DWYCOCALLCONV dwyco_simple_calldisp(int call_id, int chan_id, int what, void *arg, const char *uid, int len_uid, const char *call_type, int len_call_type);
    static void DWYCOCALLCONV dwyco_call_accepted(int chan_id, const char *name, const char *location, const char *uid, int len_uid, const char *call_type, int len_call_type);
    static void DWYCOCALLCONV call_died(int chan_id, void *arg);
    static int DWYCOCALLCONV dwyco_private_chat_init(int chan_id, const char *);
    static int DWYCOCALLCONV dwyco_private_chat_display(int chan_id, const char *com, int arg1, int arg2, const char *str, int len);
    static int DWYCOCALLCONV
    dwyco_call_screening_callback(int chan_id,
                                  int remote_wants_to_recv_your_video, int remote_wants_to_send_you_video,
                                  int remote_wants_to_recv_your_audio, int remote_wants_to_send_you_audio,
                                  int remote_wants_to_exchange_pubchat, int remote_wants_to_exchange_privchat,
                                  const char *call_type, int len_call_type,
                                  const char *uid, int len_uid,
                                  int *accept_call_style,
                                  char **error_msg);
    void connect_signals();

public:
    explicit simple_call(const QByteArray& uid, QObject *parent = 0);
    ~simple_call();
    DVP vp;

    static DwQueryByMember<simple_call> Simple_calls;
    static simple_call *get_simple_call(QByteArray uid);
    //static void hide_all();
    static void init(QObject *mainwin);
    static void suspend();
    static QObject *Mainwinform;

    int show_preview;

    int chan_id;
    int call_id;
    QByteArray uid;
    int record_preview_id;
    QTimer accept_timer;
    QTimer ask_timer;

    void reset_state_machines();
    int can_stream();

private:

    int rpause;
    int rcam_on;
    int rcam_none;

    // audio related state
    int rcts;
    int rmute;

    int cts;
    int mute;
    int kb_active;
    int rem_kb_active;

    QTimer keyboard_active_timer;
    QTimer reconnect_timer;
    QStateMachine *connect_state_machine;
    QStateMachine *call_setup_state_machine;
    QStateMachine *send_state_machine;
    QStateMachine *recv_state_machine;
    QStateMachine *audio_stream_state_machine;
    QStateMachine *full_duplex_audio_stream_state_machine;
    QStateMachine *mute_state_machine;

    void assign_button_states(QState *s, int accept, int accept_and_send, int send_video, int reject, int hangup, int cancel_req);


public slots:
    void camera_event(int);
    //void on_actionPause_triggered(bool);
    //void on_actionVideo_options_triggered();
    void set_preview_image(const QImage& qi);
    void set_remote_image(int, const QImage& qi);
    void start_media_calls();

signals:
    void pal_event(QByteArray);
    void ignore_event(QByteArray);
    void uid_selected(QByteArray, int);
    void send_msg_event(QByteArray);

    // signals for control connection
    void try_connect();
    void connect_established();
    void connect_failed();
    void connect_terminated();
    //void connect_terminated(QByteArray uid);
    void connect_already_exists();

    // this is really an internal thing that is emitted
    // on call termination. you can use it to clean up
    // cached video frames or whatever.
    void call_death_cleanup(int ui_id);

    // signals for call setup/screening
    void initial_cam_event();
    void cam_is_on();
    void cam_is_off();
    void recv_query();
    void recv_reject();
    void recv_ok();
    void recv_cancel();
    void accept_query_timeout();

    // signals for send video

    // signals for recv video
    void rem_pause();
    void rem_unpause();
    void rem_cam_off();
    void rem_cam_on();
    void rem_cam_none();

    // signals for remote audio
    void rem_cts_on();
    void rem_cts_off();
    void rem_audio_on();
    void rem_audio_off();
    void rem_audio_none();
    void rem_mute_on();
    void rem_mute_off();
    void rem_mute_unknown();

    void mute_on();
    void mute_off();
    void cts_on();
    void cts_off();
    void audio_none();

    void audio_recording(int, int);

    // signals for "typing..." indication
    void rem_keyboard_active(int);
    void rem_keyboard_active(const QString& uid, int);

    // this is mostly for debugging, the messages
    // are not really interesting for end-users.
    void connect_progress(QString);

private slots:
    void on_actionPause_toggled(bool arg1);

    void signal_dispatcher();
    void signal_dispatcher_int(int i);
    void signal_dispatcher_bool(bool i);
    void signal_dispatcher_string(QString i);

    void recv_control_msg(int ui_id, QByteArray com, int arg1, int arg2, QByteArray str);
    void keyboard_input();
    void keyboard_inactive();

    // slots for control connection
    void start_retry_timer();
    void stop_retry_timer();

    // slots for call setup/screening
    void initial_cam_setup();
    void start_accept_timer();
    void stop_accept_timer();

    // slots for streaming out
    void start_stream();
    void pause_stream();


    // button slots
    void on_accept_clicked();
    void on_accept_and_send_clicked();
    void on_send_video_clicked();
    void on_hangup_clicked();
    void on_reject_clicked();

    // remote state slots
    void react_to_rem_cam_on();
    void react_to_rem_cam_off();
    void react_to_rem_cam_none();

    void react_to_rem_pause();
    void react_to_rem_unpause();

    // local button reset
    void reset_call_buttons();

    void start_control(bool);
    //void vis_unviewed(bool);

    void cam_control_on(bool);
    void cam_control_off(bool);

    void process_ignore_event(QByteArray);

    void render_image();
    void on_cancel_req_clicked();

    //void uid_resolved(QByteArray);

    void start_ask_timer();
    void stop_ask_timer();

    //void handle_vis_change(bool);
    void set_refresh_users();

    //void process_content_filter_change(int);
    // audio related slots
    void initial_audio_setup();
    void initial_audio_stream_setup();
    void initial_full_duplex_audio_stream_setup();
    void react_to_rem_cts_on();
    void react_to_rem_cts_off();
    void react_to_rem_mute_on();
    void react_to_rem_mute_off();
    void mute_event(bool);
    void reset_audio_stream();
    void set_half_duplex_play();
    void set_half_duplex_record();
    void set_full_duplex_record();
    void check_mute();
    void uncheck_mute();
    void reemit_mute();

    void audio_recording_event(int, int);

    //void on_pal_button_clicked();
    //void update_pal(QByteArray uid);

public:
    callsm_objs *ui;
private:
    callsm_objs smo;
};


#define SIMPLE_CALL_TYPE "simple"


#endif // CALLSM_H
