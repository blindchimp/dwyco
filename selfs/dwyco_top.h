
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWYCO_TOP_H
#define DWYCO_TOP_H
// This class implements an encapsulation of the Dwyco API using
// Qt/QML.

#include <QObject>
#include <QVariant>
#include <QUrl>
#include <QNetworkReply>
#include <QThread>
#include "dlli.h"
#include "QQmlVarPropertyHelpers.h"
#include <QAbstractListModel>
#ifndef NO_BUILDTIME
#include "buildtime.h"
#else
#ifndef BUILDTIME
#define BUILDTIME "debug"
#endif
#endif
class DwycoCore : public QObject
{
    Q_OBJECT

    QML_WRITABLE_VAR_PROPERTY(QString, client_name)
    QML_WRITABLE_VAR_PROPERTY(bool, use_archived)
    QML_READONLY_VAR_PROPERTY(int, total_users)
    QML_READONLY_VAR_PROPERTY(bool, any_unviewed)
    QML_READONLY_VAR_PROPERTY(QString, buildtime)
    QML_READONLY_VAR_PROPERTY(QString, user_dir)
    QML_READONLY_VAR_PROPERTY(QString, tmp_dir)
    QML_READONLY_VAR_PROPERTY(int, external_storage_permission)
    QML_READONLY_VAR_PROPERTY(int, has_audio_input)
    QML_READONLY_VAR_PROPERTY(int, has_audio_output)
    QML_READONLY_VAR_PROPERTY(int, audio_full_duplex)
    QML_READONLY_VAR_PROPERTY(int, vid_dev_idx)
    QML_READONLY_VAR_PROPERTY(QString, vid_dev_name)
    QML_READONLY_VAR_PROPERTY(QString, this_uid)
    QML_READONLY_VAR_PROPERTY(QString, this_handle)
    QML_READONLY_VAR_PROPERTY(bool, directory_fetching)

    QML_READONLY_VAR_PROPERTY(QString, active_group_name)
    QML_READONLY_VAR_PROPERTY(QString, join_key)
    QML_READONLY_VAR_PROPERTY(int, group_status)
    QML_READONLY_VAR_PROPERTY(int, percent_synced)
    QML_READONLY_VAR_PROPERTY(int, eager_pull)
    QML_READONLY_VAR_PROPERTY(int, group_private_key_valid)


public:
    DwycoCore(QObject *parent = 0) : QObject(parent) {
        m_client_name = "";
        m_buildtime = BUILDTIME;
        m_user_dir = ".";
        m_tmp_dir = ".";
        m_external_storage_permission = 0;
        m_has_audio_input = 0;
        m_has_audio_output = 0;
        m_audio_full_duplex = 0;
        m_vid_dev_idx = 0;
        m_vid_dev_name = "";
        m_use_archived = true;
        m_this_uid = "";
        m_this_handle = "";
        m_directory_fetching = false;
        m_active_group_name = "";
        m_join_key = "";
        m_percent_synced = 0;
        m_group_status = 0;
        m_eager_pull = 0;
        m_any_unviewed = false;
    }
    static QByteArray My_uid;


    enum System_event {
        USER_STATUS_CHANGE = DWYCO_SE_USER_STATUS_CHANGE,
        USER_ADD = DWYCO_SE_USER_ADD,
        USER_DEL = DWYCO_SE_USER_DEL,
        SERVER_CONNECTING = DWYCO_SE_SERVER_CONNECTING,
        SERVER_CONNECTION_SUCCESSFUL = DWYCO_SE_SERVER_CONNECTION_SUCCESSFUL,
        SERVER_DISCONNECT = DWYCO_SE_SERVER_DISCONNECT,
        SERVER_LOGIN = DWYCO_SE_SERVER_LOGIN,
        SERVER_LOGIN_FAILED = DWYCO_SE_SERVER_LOGIN_FAILED,
        USER_MSG_RECEIVED = DWYCO_SE_USER_MSG_RECEIVED,
        USER_UID_RESOLVED = DWYCO_SE_USER_UID_RESOLVED,
        USER_PROFILE_INVALIDATE = DWYCO_SE_USER_PROFILE_INVALIDATE,
        USER_MSG_IDX_UPDATED = DWYCO_SE_USER_MSG_IDX_UPDATED,
        USER_MSG_IDX_UPDATED_PREPEND = DWYCO_SE_USER_MSG_IDX_UPDATED_PREPEND,
        MSG_SEND_START = DWYCO_SE_MSG_SEND_START,
        MSG_SEND_FAIL = DWYCO_SE_MSG_SEND_FAIL,
        MSG_SEND_SUCCESS = DWYCO_SE_MSG_SEND_SUCCESS,
        MSG_SEND_STATUS = DWYCO_SE_MSG_SEND_STATUS,
        MSG_SEND_DELIVERY_SUCCESS = DWYCO_SE_MSG_SEND_DELIVERY_SUCCESS,
        MSG_SEND_CANCELED = DWYCO_SE_MSG_SEND_CANCELED
    };

    enum Profile_info {
        HANDLE,
        EMAIL,
        LOCATION,
        DESCRIPTION,
        REVIEWED,
        REGULAR
    };

    enum Chat_event {
        CHAT_CTX_NEW = DWYCO_CHAT_CTX_NEW,
        CHAT_CTX_DEL = DWYCO_CHAT_CTX_DEL,
        CHAT_CTX_ADD_USER = DWYCO_CHAT_CTX_ADD_USER,
        CHAT_CTX_DEL_USER = DWYCO_CHAT_CTX_DEL_USER,
        CHAT_CTX_UPDATE_AH = DWYCO_CHAT_CTX_UPDATE_AH,
        CHAT_CTX_START_UPDATE = DWYCO_CHAT_CTX_START_UPDATE,
        CHAT_CTX_END_UPDATE = DWYCO_CHAT_CTX_END_UPDATE,
        CHAT_CTX_Q_ADD = DWYCO_CHAT_CTX_Q_ADD,
        CHAT_CTX_Q_DEL = DWYCO_CHAT_CTX_Q_DEL,
        CHAT_CTX_Q_GRANT = DWYCO_CHAT_CTX_Q_GRANT,
        CHAT_CTX_Q_DATA = DWYCO_CHAT_CTX_Q_DATA,
        CHAT_CTX_SYS_ATTR = DWYCO_CHAT_CTX_SYS_ATTR,
        CHAT_CTX_UPDATE_ATTR = DWYCO_CHAT_CTX_UPDATE_ATTR,
        CHAT_CTX_ADD_LOBBY = DWYCO_CHAT_CTX_ADD_LOBBY,
        CHAT_CTX_DEL_LOBBY = DWYCO_CHAT_CTX_DEL_LOBBY,
        CHAT_CTX_ADD_GOD = DWYCO_CHAT_CTX_ADD_GOD,
        CHAT_CTX_DEL_GOD = DWYCO_CHAT_CTX_DEL_GOD,
        CHAT_CTX_RECV_DATA = DWYCO_CHAT_CTX_RECV_DATA
    };

    Q_ENUM(System_event)
    Q_ENUM(Profile_info)
    Q_ENUM(Chat_event)

    Q_INVOKABLE void init();
    Q_INVOKABLE int service_channels();
    Q_INVOKABLE void exit() {
        // note: this will clean some parts of the
        // system that might be in use by the client
        // as well, and ignore some parts. like files
        // ending in .jpeg will not be removed from
        // the tmp folder. it is probably a mistake
        // for the client to be using the same tmp
        // folder as the dwyco* api, since it might
        // clean some things the client isn't expecting.
        dwyco_exit();
    }

    Q_INVOKABLE void power_clean() {
        // note: this is disabled right now, pending some
        // better definition of "clean". it doesn't jive
        // with the syncing stuff we do now.
        dwyco_power_clean_safe();
    }

    Q_INVOKABLE QString get_my_uid() {
        return My_uid.toHex();
    }

    Q_INVOKABLE QString strip_html(QString);

    Q_INVOKABLE int database_online();
    Q_INVOKABLE int chat_online();

    Q_INVOKABLE QUrl get_simple_directory_url();
    Q_INVOKABLE QUrl get_simple_lh_url();
    Q_INVOKABLE QString get_msg_count_url();
    Q_INVOKABLE QString url_to_filename(QUrl);
    Q_INVOKABLE int simple_send(QString recipient, QString msg);
    Q_INVOKABLE int simple_send_file(QString recipient, QString msg, QString filename);
    Q_INVOKABLE int simple_send_url(QString recipient, QString msg, QUrl filename);
    Q_INVOKABLE int send_simple_cam_pic(QString recipient, QString msg, QString filename);
    Q_INVOKABLE int is_file_zap(int compid) {
        return dwyco_is_file_zap(compid);
    }

    Q_INVOKABLE int send_forward(QString recipient, QString add_text, QString mid_to_forward, int save_sent);
    Q_INVOKABLE int flim(QString mid_to_forward);

    Q_INVOKABLE void send_chat(QString text);

    Q_INVOKABLE QString uid_to_name(QString uid);
    Q_INVOKABLE QString uid_to_profile_info(QString uid, enum Profile_info field);
    Q_INVOKABLE bool uid_profile_regular(QString uid);
    Q_INVOKABLE QUrl uid_to_profile_preview(QString uid);
    Q_INVOKABLE QUrl uid_to_http_profile_preview(QString uid);
    Q_INVOKABLE QUrl uid_to_profile_view(QString uid);
    Q_INVOKABLE QString uid_to_profile_image_filename(QString uid);
    Q_INVOKABLE void name_to_uid(QString handle);

    Q_INVOKABLE int set_setting(QString name, QString value);
    Q_INVOKABLE QVariant get_setting(QString name);

    Q_INVOKABLE void set_local_setting(QString name, QString value);
    Q_INVOKABLE QString get_local_setting(QString name);

    Q_INVOKABLE void bootstrap(QString name, QString email);
    Q_INVOKABLE int create_new_account();
    Q_INVOKABLE QString random_string(int len);

    Q_INVOKABLE int set_simple_profile(QString handle, QString email, QString desc, QString img_fn);

    Q_INVOKABLE void set_pal(QString uid, int is_pal);
    Q_INVOKABLE int get_pal(QString uid);

    Q_INVOKABLE void set_ignore(QString uid, int is_ignored);
    Q_INVOKABLE int get_ignore(QString uid);
    Q_INVOKABLE void clear_ignore_list();

    Q_INVOKABLE void reset_unviewed_msgs(QString uid);

    Q_INVOKABLE int make_zap_view(QString mid);
    Q_INVOKABLE int make_zap_view_file(QString fn);
    Q_INVOKABLE int delete_zap_view(int view_id) {
        return dwyco_delete_zap_view(view_id);
    }
    Q_INVOKABLE int play_zap_preview(int view_id) {
        return 0;
    }
    Q_INVOKABLE int play_zap_view(int view_id);
    Q_INVOKABLE int stop_zap_view(int view_id) {
        return dwyco_zap_stop_view(view_id);
    }

    // dwyco video record
    Q_INVOKABLE int make_zap_composition();
    Q_INVOKABLE int start_zap_record(int zid, int vid, int aud);
    Q_INVOKABLE int start_zap_record_pic(int zid);
    Q_INVOKABLE int stop_zap_record(int zid);
    Q_INVOKABLE int play_zap(int zid);
    Q_INVOKABLE int stop_zap(int zid);
    Q_INVOKABLE int cancel_zap(int zid);
    Q_INVOKABLE int send_zap(int zid, QString recipient, int save_sent);

    Q_INVOKABLE int delete_message(QString uid, QString mid);
    Q_INVOKABLE int clear_messages(QString uid);
    Q_INVOKABLE int clear_messages_unfav(QString uid);
    Q_INVOKABLE int delete_user(QString uid);
    Q_INVOKABLE int get_fav_message(QString mid);
    Q_INVOKABLE void set_fav_message(QString mid, int val);
    Q_INVOKABLE int has_tag_message(QString mid, QString tag);
    Q_INVOKABLE void set_tag_message(QString mid, QString tag);
    Q_INVOKABLE void unset_tag_message(QString mid, QString tag);


    Q_INVOKABLE void uid_keyboard_input(QString uid);
    Q_INVOKABLE int get_rem_keyboard_state(QString uid);
    Q_INVOKABLE void create_call_context(QString uid);
    Q_INVOKABLE void start_control(QString uid);
    Q_INVOKABLE int get_established_state(QString uid);
    Q_INVOKABLE void hangup_all_calls();
    // WARNING: calling these are touchy, and you will crash if you
    // call them within slots/handlers which are invoked via
    // call-related signals. generally, it is better NOT to
    // delete the call context, and just let it take care of
    // itself.
    Q_INVOKABLE void delete_call_context(QString uid);
    Q_INVOKABLE void delete_all_call_contexts();

    Q_INVOKABLE void delete_file(QString fn);

    // chat server related
    Q_INVOKABLE int switch_to_chat_server(int);
    Q_INVOKABLE void disconnect_chat_server();
    Q_INVOKABLE void set_invisible_state(int);

    // contact list related functions
    Q_INVOKABLE int load_contacts();
    Q_INVOKABLE QUrl get_cq_results_url();
    Q_INVOKABLE void delete_cq_results();

    // control for auto/manual fetching
    Q_INVOKABLE int retry_auto_fetch(QString mid);

    Q_INVOKABLE bool get_cc_property(QString uid, QString button_name, QString property_name);
    Q_INVOKABLE void set_cc_property(QString uid, QString button_name, QString property_name, bool val);
    //Q_INVOKABLE void invoke_button(QString uid, QString buton_name, QString);
    Q_INVOKABLE QObject *get_button_model(QString uid);

    // dwyco video camera api
    Q_INVOKABLE void select_vid_dev(int i);
    Q_INVOKABLE void enable_video_capture_preview(int i);

    // test stuff
    //Q_INVOKABLE void start_gj(QString uid, QString password);
    Q_INVOKABLE int start_gj2(QString gname, QString password);

    Q_INVOKABLE void set_badge_number(int i);
    Q_INVOKABLE void refresh_directory();

    // control whether we accept any calls from other clients
    // this physically disconnects us from the net and from
    // accepting server assisted calls. does not affect connection
    // to server, so messages sent via the server will still be delivered.
    Q_INVOKABLE void inhibit_all_incoming_calls(int i);

    Q_INVOKABLE int send_report(QString uid);
    Q_INVOKABLE QString export_attachment(QString mid);
    static void one_time_copy_files();
    Q_INVOKABLE void background_migrate();
    Q_INVOKABLE void directory_swap();

    // this can sometime take awhile, so we handle it in a thread, then
    // cause the user to exit and restart
    Q_INVOKABLE void background_reindex();
    static void do_reindex();

    Q_INVOKABLE QUrl from_local_file(const QString&);
    Q_INVOKABLE QString to_local_file(const QUrl& url);

    Q_INVOKABLE int load_backup();
    Q_INVOKABLE int get_android_backup_state();

    Q_INVOKABLE QString map_to_representative(const QString& uid);

public:

public slots:
    void app_state_change(Qt::ApplicationState);
    void update_dwyco_client_name(QString);
    void internal_cq_check(QString);
    void dir_download_finished(QNetworkReply *);
    void call_cleanup(QString uid, int ui_id);

signals:
    void server_login(const QString& msg, int what);
    void chat_event(int cmd, int sid, const QString& huid, const QString &sname, QVariant vdata, int qid, int extra_arg);
    void new_msg(const QString& from_uid, const QString& txt, const QString& mid);
    void msg_send_status(int status, const QString& recipient, const QString& pers_id);
    void msg_progress(const QString& pers_id, const QString& recipient, const QString& msg, int percent_done);
    void sys_invalidate_profile(const QString& uid);
    void sys_msg_idx_updated(const QString& uid);
    void sys_uid_resolved(const QString& uid);
    void profile_update(int success);
    void pal_event(const QString& uid);
    void ignore_event(QString uid);
    void video_display(int ui_id, int frame_number, QString img_path);
    void video_capture_preview(QString img_path);
// this is used internally, should not fiddle with it via QML
    void user_control(int, QByteArray, QByteArray);
    void decorate_user(const QString& uid);
    void sys_chat_server_status(int id, int status);
    void qt_app_state_change(int app_state);

    // these are sent when a call context is up
    void sc_rem_keyboard_active(QString uid, int active);
    void sc_connect_terminated(QString uid);
    void sc_connectedChanged(QString uid, int connected);
    void sc_call_death_cleanup(QString uid, int ui_id);
    void sc_associate_uid_with_ui_id(QString uid, int ui_id);
    // this is sent if the connection fails during setup, or
    // the call is rejected by the other side for some call
    // screening issue
    void sc_connect_failed(QString uid);
    void sc_connect_established(QString uid);

    void sc_cam_is_off(QString uid);
    void sc_cam_is_on(QString uid);
    void sc_mute_on(QString uid);
    void sc_mute_off(QString uid);
    void sc_cts_on(QString uid);
    void sc_cts_off(QString uid);
    void sc_audio_none(QString uid);

    // remote video manipulation signals

    void sc_rem_pause(QString uid);
    void sc_rem_unpause(QString uid);
    void sc_rem_cam_off(QString uid);
    void sc_rem_cam_on(QString uid);
    void sc_rem_cam_none(QString uid);

    // remote audio state signals

    void sc_rem_cts_on(QString uid);
    void sc_rem_cts_off(QString uid);
    void sc_rem_audio_on(QString uid);
    void sc_rem_audio_off(QString uid);
    void sc_rem_audio_none(QString uid);
    void sc_rem_mute_on(QString uid);
    void sc_rem_mute_off(QString uid);
    void sc_rem_mute_unknown(QString uid);

    // this is mostly for debugging
    void sc_connect_progress(QString uid, QString msg);


    void image_picked(const QString& fn);
    void cq_results_received(int succ);
    void msg_recv_state(int cmd, const QString& mid, const QString& huid);
    void msg_recv_progress(const QString& mid, const QString& ruid, const QString& msg, int percent);
    // dwyco video camera signals
    void camera_change(int cam_on);
    // zap composition record/play stopped
    void zap_stopped(int zid);

    void mid_tag_changed(QString mid);
    void migration_complete();
	void reindex_complete();

    void name_to_uid_result(QString uid, QString handle);

    void msg_pull_ok(const QByteArray& mid, const QString& huid);
    void msg_tag_change_global(const QByteArray& mid, const QString& huid);

    void join_result(const QByteArray& gname, int result);

private:

    static void DWYCOCALLCONV dwyco_chat_ctx_callback(int cmd, int id, const char *uid, int len_uid, const char *name, int len_name, int type, const char *val, int len_val, int qid, int extra_arg);

};

class fuck_me_with_a_brick : public QThread
{
    Q_OBJECT
    void run() {
        DwycoCore::one_time_copy_files();
    }

};

class fuck_me_with_a_brick2 : public QThread
{
    Q_OBJECT
    void run() {
        DwycoCore::do_reindex();
    }

};

#endif
