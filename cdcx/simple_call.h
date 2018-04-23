
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SIMPLE_CALL_H
#define SIMPLE_CALL_H

#include <QDockWidget>
#include <QToolButton>
#include <QToolBar>
#include <QMenu>
#include <QImage>
#include <QTimer>
#include <QStateMachine>
#include <QStaticText>
#include "dvp.h"
#include "dwstr.h"
#include "dwquerybymember.h"
#include "dlli.h"

namespace Ui {
class simple_call;
}

class simple_call : public QDockWidget
{
    Q_OBJECT

    friend void DWYCOCALLCONV dwyco_simple_calldisp(int call_id, int chan_id, int what, void *arg, const char *uid, int len_uid, const char *call_type, int len_call_type);
    friend void DWYCOCALLCONV dwyco_call_accepted(int chan_id, const char *name, const char *location, const char *uid, int len_uid, const char *call_type, int len_call_type);
    friend void DWYCOCALLCONV call_died(int chan_id, void *arg);

public:
    explicit simple_call(const DwOString& uid, QWidget *parent = 0);
    ~simple_call();
    DVP vp;

    static DwQueryByMember<simple_call> Simple_calls;
    static simple_call *get_simple_call(DwOString uid);
    static void hide_all();

    int show_preview;

    int chan_id;
    int call_id;
    DwOString uid;
    int record_preview_id;
    QTimer accept_timer;
    QTimer ask_timer;
    void reset_state_machines();
    int can_stream();

    void showEvent(QShowEvent *);
    void focusInEvent(QFocusEvent *);
    void closeEvent(QCloseEvent *e);
    void hideEvent(QHideEvent *);

    QTimer resize_timer;
    int vis;
    QTimer msg_countdown;

    void set_connection_status_display(int on);

private:

    int zap_id;
    // persisten id of the last msg we sent using this chatbox.
    DwOString pers_id;
    int allstop;
    int rpause;
    int rcam_on;
    int rcam_none;
    QStaticText paused_text;
    QStaticText off_text;
    QStaticText no_cam_hardware_text;

    // audio related state
    int rcts;
    int rmute;

    int cts;
    int mute;

    void leaveEvent(QEvent *);
    void resizeEvent(QResizeEvent *);
    QToolButton *pause_button;
    QToolBar *toolbar;
    QMenu *popup_menu;
    DwOString context_uid;
    DwOString context_mid;
    QImage preview_image;
    QImage remote_image;
    QImage record_preview_image;
    int kb_active;
    QTimer keyboard_active_timer;
    QTimer reconnect_timer;
    QStateMachine *connect_state_machine;
    QStateMachine *call_setup_state_machine;
    QStateMachine *send_state_machine;
    QStateMachine *recv_state_machine;
    QStateMachine *audio_stream_state_machine;
    QStateMachine *full_duplex_audio_stream_state_machine;
    QStateMachine *mute_state_machine;

    int preview_saved_msg(DwOString mid, DwOString& preview_fn, int& video, int& short_video, int& audio, int& file, QString& local_time);
    void reset_unviewed();

    void assign_button_states(QState *s, int accept, int accept_and_send, int send_video_vis, int send_video_enabled, int reject, int hangup, int cancel_req);
    void new_zap();
    void reset_title();

    // snapshot timer
    QTimer snapshot_timer;
    int snapshot_countdown;

    void do_refresh();

public slots:
    void camera_event(int);
    void mouse_stopped_event(QPoint);
    void on_actionPause_triggered(bool);
    void on_actionVideo_options_triggered();
    void display_new_msg(DwOString, DwOString, DwOString);
    void set_preview_image(const QImage& qi);
    void set_remote_image(int, const QImage& qi);
    void start_media_calls();
    void clear_chatwin();
    void check_resize();
    void update_content_filters();

signals:
    void pal_event(DwOString);
    void ignore_event(DwOString);
    void uid_selected(DwOString, int);
    void send_msg_event(DwOString);

    // signals for control connection
    void try_connect();
    void connect_established();
    void connect_failed();
    void connect_terminated();
    void connect_already_exists();

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

public:
    Ui::simple_call *ui;

private slots:
    void on_send_button_clicked();
    void on_actionPause_toggled(bool arg1);
    void on_textBrowser_anchorClicked(const QUrl &url);
    void on_actionView_profile_ctx_triggered();
    void on_actionCompose_msg_ctx_triggered();
    void on_actionIgnore_user_ctx_triggered();

    void recv_control_msg(int ui_id, DwOString com, int arg1, int arg2, DwOString str);
    void on_actionView_attachment_triggered();
    void hide_msgplayer();
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
    void vis_unviewed(bool);

    void force_scrollbar_down();
    void reposition_toolbar();
    void handle_toolbar();

    void on_actionSend_file_ctx_triggered();

    void cam_control_on(bool);
    void cam_control_off(bool);

    void process_ignore_event(DwOString);

    void on_actionRefresh_triggered();

    void render_image();
    void on_cancel_req_clicked();

    void uid_resolved(DwOString);
    void on_actionZoom_in_toggled(bool arg1);

    void start_ask_timer();
    void stop_ask_timer();

    void handle_vis_change(bool);
    void set_refresh_users();
    void on_compose_button_clicked();
    void on_send_file_button_clicked();
    void on_show_profile_button_clicked();

    void process_content_filter_change(int);
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

    void on_pal_button_clicked();
    void update_pal(DwOString uid);
    void on_send_snapchat_clicked(bool checked);

    virtual void pers_msg_send_status(int status, DwOString pid, DwOString ruid);
    virtual void msg_progress(DwOString pid, DwOString ruid, DwOString msg, int percent);
    void on_actionStart_snapshot_timer_toggled(bool checked);
    void snapshot_timeout();
    void stop_snapshot_timer(bool);
    void on_connect_button_clicked();
    void disable_connect_button();
};


#define SIMPLE_CALL_TYPE "simple"


#endif // SIMPLE_CALL_H
