
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef mainwin_h
#define mainwin_h

#include <QWidget>
#include <QTimer>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QStatusBar>
#include <QLabel>
#include <QSystemTrayIcon>
#include <QTextEdit>
#include <QHash>
#include <QByteArray>
#include "ui_mainwin.h"
#include "dwstr.h"
#include "userlob.h"
#include "dvp.h"
#include "dlli.h"
class lobbybox3;
class simple_public;
class dirbox;
class vidbox;
class BrowseBox;
class MsgBrowseDock;
class simple_call;
class WHATBOX;

class PalSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    PalSortFilterModel(QObject *p = 0) : QSortFilterProxyModel(p) {}
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
};

struct godrec
{
    QByteArray uid;
    QString name;
    QString where;
    QString server_id;
    int god;
    int demigod;
    int subgod;

    godrec() {
        god = 0;
        demigod = 0;
        subgod = 0;
    }
};

class mainwinform : public QMainWindow
{
    Q_OBJECT

    friend void
    DWYCOCALLCONV
    dwyco_chat_ctx_callback(int cmd, int id,
                            const char *uid, int len_uid,
                            const char *name, int len_name,
                            int type, const char *val, int len_val,
                            int qid,
                            int extra_arg);
    friend void
    DWYCOCALLCONV
    dwyco_video_display(int ui_id, void *vimg, int cols, int rows, int depth);


    friend class simple_call;

public:
    mainwinform(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~mainwinform();
    void load_users();
    void decorate_users();
    void decorate_user(const DwOString& uid);
    void handle_animate_uid(const DwOString& uid, const DwOString& icon, int toggle);
    void handle_animate_uid_font(const DwOString& uid, int what, int toggle);
    void add_call(const DwOString& uid, int chan_id);
    void del_call(const DwOString& uid, int chan_id);
    DVP vp;
    int first_expand;
    QStandardItemModel *umodel;
    PalSortFilterModel sort_proxy_model;
#if 0
    QModelIndex pals_idx;
    QModelIndex others_idx;
    QModelIndex calls_idx;
#endif
    QMenu *popup_menu;
    QLabel *db_status;
    QLabel *server_status;
    QLabel *audio_output_status;
    QLabel *audio_input_status;
    QLabel *camera_status;
    QLabel *lic_status;
    QLabel *invis_status;
    QLabel *pals_only_status;
    QLabel *archive_status;
    QSystemTrayIcon *tray_icon;
    QIcon *blank_tray_icon;
    QIcon *gg_tray_icon;
    QMenu *tray_menu;
    lobbybox3 *lbox;
    simple_public *pubchat;
    WHATBOX *dir_listing;
    vidbox *vid_preview;
    WHATBOX *browse_box;
    MsgBrowseDock *msg_browse_dock;

    void contextMenuEvent(QContextMenuEvent *ev);
    void closeEvent(QCloseEvent *);
    bool event(QEvent *event);

    void organize_chatboxes(simple_call *c);

    void next_tab();
    void prev_tab();

    QList<QByteArray> get_selection(int only_one);


    void emit_public_chat_display(DwOString who, DwOString txt, DwOString uid);
    void emit_chat_event(int cmd, int id, const char *uid, int len_uid, const char *name, int len_name,
                         int type, const char *val, int len_val,
                         int qid, int extra_arg);
    void refetch_user_list();
    void show_archive_status(int ul, int total);

public slots:
    void on_usertree_view_doubleClicked(const QModelIndex& );
    void on_usertree_view_clicked(const QModelIndex& );
    void idle();
    void on_actionUser_profile_triggered(bool);
    void on_actionAbout_triggered(bool);

    void on_actionPromote_to_Pal_triggered(bool);
    void on_actionDemote_to_Non_pal_triggered(bool);
    void on_actionBlock_user_triggered(bool);
    void on_actionUnblock_user_triggered(bool);
    void on_actionCompose_Message_triggered(bool);
    void on_actionSend_message_triggered(bool);
    void on_actionView_Profile_triggered(bool);
    void on_actionRemove_user_triggered(bool);
    void on_actionSend_file_triggered(bool);
    void on_actionAlert_when_online_triggered(bool);

    void on_actionVideo_triggered(bool);
    void on_actionCheck_for_Updates_triggered(bool);
    void on_actionDiagnostics_triggered(bool);
    void on_actionFetch_Lost_Password_triggered(bool);
    void on_actionAdvanced_triggered(bool);
    void on_actionUpdate_info_triggered(bool);
    void on_actionBrowse_saved_msgs_triggered(bool);

    void on_actionHow_to_buy_a_subscription_triggered(bool);

    void on_actionShow_Profile_Grid_triggered(bool);
    void on_actionShow_Directory_triggered(bool);
    void on_actionShow_Public_Chat_triggered(bool);
    void on_actionShow_Video_Preview_triggered(bool);
    void on_actionChange_password_triggered(bool);
    void on_actionBandwidth_test_triggered(bool);
    void on_actionShow_Ignore_List_triggered(bool);
    void on_actionExit_triggered(bool);
    void on_actionOpen_triggered(bool);

    void on_login_button_clicked();
    void camera_event(int );

    void trayiconActivated(QSystemTrayIcon::ActivationReason);
    void update_ignore(DwOString);
    void relogin();
    void pal_relogin(DwOString);
    void show_lobby_list(bool);
    void show_pubchat(bool);
    void show_dir_listing(bool);
    void show_vid_preview(bool);
    void show_msgbrowse(bool);
    void show_profiles(bool);

    void load_browsebox();

    void manage_audio_sends(DwOString, int, bool);
    void audio_recording_event(int, int);
    void record_recent_operation(DwOString);
    void simplify_view(bool);
    void set_refresh_user(DwOString);
    void update_pals_only(int);
    void online_status_change(DwOString);

private slots:

    void on_actionMake_me_invisible_triggered(bool checked);
    void on_actionShow_Lobby_List_triggered(bool );
    void on_actionShow_Lobby_List_toggled(bool );
    void on_actionAllow_multiple_profiles_and_browsers_triggered(bool );
    void on_actionReject_call_triggered();
    void on_actionAccept_call_triggered();
    void on_actionHangup_triggered();
    void on_actionHide_All_Chatboxes_triggered();
    void on_actionHide_All_Msg_Browsers_triggered();
    void on_actionHide_All_triggered();
    void on_actionShow_Offline_Users_triggered(bool );

    void animate();

    void on_actionReload_recent_visitors_triggered();
    void on_actionCreate_new_lobby_triggered();
    void on_actionRemove_lobby_triggered();
    void on_actionShow_Msg_Browser_triggered(bool checked);

    // note: don't use const DwOString& here, since if the
    // signals end up being q'd, you may end up with
    // dangling pointers
    friend void DWYCOCALLCONV dwyco_chat_server_status_callback(int id, const char *msg, int percent_done, void *user_arg);
    friend void DWYCOCALLCONV dwyco_sys_event_callback(int cmd, int id,
            const char *uid, int len_uid,
            const char *name, int len_name,
            int type, const char *val, int len_val,
            int qid,
            int extra_arg);
    friend void DWYCOCALLCONV dwyco_user_control(int chan_id, const char *suid, int len_uid, const char *data, int len_data);
    friend struct hover_filter;
    friend int DWYCOCALLCONV dwyco_private_chat_display(int ui_id, const char *com, int arg1, int arg2, const char *str, int len);
    friend simple_call *display_msg(const DwOString& uid, const DwOString& txt, const DwOString& mid);
    friend void DWYCOCALLCONV call_died(int chan_id, void *arg);

    friend int set_current_server(int i);
    friend int set_current_server(const DwOString& id);

    void on_actionHide_triggered();

    void on_actionExit_2_triggered();



    void on_actionShow_Public_Chat_Users_triggered(bool checked);

    void on_actionShow_pics_in_user_list_triggered(bool checked);

    void on_actionShow_Archived_Users_triggered(bool checked);

signals:
    void mic_mute_changed(int);

    void ignore_event(DwOString);
    void add_user_lobby(user_lobby);
    void del_user_lobby(DwOString);
    void clear_user_lobbies();
    void new_server_name(DwOString);
    void admin_event(int);
    void call_vector_event(DwOString, QVector<int>);
    void public_chat_display(DwOString, DwOString, DwOString);
    void chat_event(int, int, QByteArray, QString, QVariant, int, int);
    void pal_event(DwOString);
    void mouse_stopped(QPoint);
    void chat_server_status(int, int);
    void uid_info_event(DwOString);
    void uid_selected(DwOString, int);
    void invalidate_profile(DwOString);
    void god_add(godrec);
    void god_del(QByteArray);

    void audio_recording(int, int);
    void content_filter_event(int);
    void video_display(int ui_id, QImage img);
    void new_msg(DwOString uid, DwOString txt, DwOString mid);
    void user_control(int chan_id, DwOString uid, DwOString data);
    void camera_change(int);
    void video_capture_preview(QImage img);
    void control_msg(int ui_id, DwOString com, int arg1, int arg2, DwOString str);
    void refresh_users();
    void channel_died(int chan_id, DwOString uid);
    void vid_control(int);
    void global_video_pause_toggled(bool);
    void send_msg_event(DwOString);
    void pals_only(int);
    void msg_send_status(int, DwOString, DwOString);
    void msg_progress(DwOString, DwOString, DwOString, int);
    void uid_status_change(DwOString);

public:
    Ui::MainWindow ui;
private:
    QTimer timer;
    QTimer animation_timer;
};

extern mainwinform *Mainwinform;
simple_call *display_msg(const DwOString& uid, const DwOString& txt, const DwOString& mid);
QString dwyco_info_to_display(const DwOString& uid);
int dwyco_info_to_display(const DwOString& uid, QString& handle, QString& desc, QString& loc, QString& email);
int send_file_common(const QByteArray& uid);
QString get_a_filename(QString filter);
extern int DieDieDie;
int get_expired();
int is_registered();
void append_msg_to_textedit(QString txt, QTextEdit *te, DwOString mid);
void append_table_to_textedit(DWYCO_SAVED_MSG_LIST sm, QTextEdit *te, DwOString mid);
QString get_extended_chat(QString txt);
QString get_extended(QString txt);
void cursor_to_bottom(QTextEdit *te);
int uid_attrs_has_attr(const DwOString& uid, const DwOString& attr);
int uid_in_lobby(const DwOString& uid);
DwOString random_fn();
extern int Allow_multiple_windows;
int confirm_ignore();
QString strip_html(const QString& txt);
int confirm_show_profile();
int show_profile(const DwOString& uid, int reviewed, int regular);
void get_review_status(const DwOString& uid, int& reviewed, int& regular);
void start_animation(const DwOString& uid, const DwOString& icon);
void stop_animation(const DwOString& uid);

extern QHash<QByteArray, struct godrec> Gods;
extern QList<QAction *> *AudioActionGroup;
extern QSet<DwOString> Visible_to;
uint qHash(const DwOString&);
void cdcx_set_refresh_users(int i);

class font_animation_timer : public QObject
{
    Q_OBJECT
public:
    DwOString uid;
    int which;
    QTimer timer;

    font_animation_timer(const DwOString& auid, int which);

public slots:
    void done();
};

#endif
