
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SIMPLE_PUBLIC_H
#define SIMPLE_PUBLIC_H

#include <QDockWidget>
#include <QLabel>
#include <QList>
#include <QByteArray>
#include <QString>
#include <QVariant>
#include <QMenu>
#include "dvp.h"
#include "dlli.h"
#include "dwstr.h"


namespace Ui {
class simple_public;
}

class simple_public : public QDockWidget
{
    Q_OBJECT

public:
    explicit simple_public(QWidget *parent = 0);
    ~simple_public();
    DVP vp;
    static QList<DVP> Simple_publics;
    static QLabel *find_vg_label(int ui_id);
    int hide_video;
    DwOString context_uid;
    QMenu *popup_menu;
    int ui_id;
    DwOString current_title;
    QByteArray podium_uid;

    struct ul_item
    {
        ul_item() {
            lab_idx = -1;
            zv_id = -1;
            ui_id = -1;
            call_id = -1;
            chan_id = -1;
            last_time = 0;
            idle_time = 0;
        }
        ul_item(const QByteArray& u) : uid(u) {
            lab_idx = -1;
            zv_id = -1;
            ui_id = -1;
            call_id = -1;
            chan_id = -1;
            last_time = 0;
            idle_time = 0;
        }

        ~ul_item() {
            if(zv_id != -1)
            {
                dwyco_zap_stop_view(zv_id);
                dwyco_delete_zap_view(zv_id);
            }
        }
        QByteArray uid;
        int lab_idx;
        int zv_id; 	// zap viewer
        int ui_id;	// for zap viewer
        QString fn;
        int call_id; // if displaying call info
        int chan_id;
        time_t last_time; // used for sorting non-lurkers
        time_t idle_time; // when they went idle

        int operator==(const ul_item& c) const {
            return uid == c.uid;
        }
    };
    QList<ul_item> UList;
    QList<QLabel *> labels;
    int find_unused_label();
    int find_idle_displayed();
    void uid_idle_event(QByteArray uid, QVariant data);
    void add_user_profile(QString name, QByteArray uid);
    void remove_user(QByteArray uid);
    void clear();
    int find_oldest_talker();
    int update_talk_time(QByteArray uid);
    void refresh_profile(QByteArray uid);
    void set_hide_video(int hv);
    int find_undisplayed();
    void display_snapchat(QByteArray uid, QString name, QVariant data);
    void refresh_all_profiles();
    QPixmap get_preview_by_uid(DwOString uid);


public slots:
    void display_public_chat(DwOString, DwOString, DwOString);
    void chat_event(int,int,QByteArray,QString, QVariant,int,int);
    void update_ignore(DwOString);
    //void set_title(DwOString);
    void admin_update(int);
    void update_call_vector_display(DwOString uid, QVector<int>);
    void mouse_stopped_event(QPoint);
    void chat_server_event(int);
    void set_title(DwOString);
    void uid_resolved(DwOString);
    void update_profile(DwOString);
    void redisplay_all(int);
    void show_video_in_label(int, QImage);
    void audio_recording_event(int, int);


signals:
    void ignore_event(DwOString);
    void uid_selected(DwOString, int);
    void audio_recording(int, int);

private:
    Ui::simple_public *ui;

private slots:

    void on_actionView_profile_ctx_triggered();
    void on_actionMute_triggered(bool checked);
    void on_actionDj_triggered(bool checked);
    void on_actionMute_toggled(bool );
    void on_actionDj_toggled(bool );

    void on_actionAdmin_ctx_triggered();
    void on_actionAdmin_triggered();
    void on_text_display_anchorClicked(const QUrl& );
    void on_text_enter_returnPressed();
    void on_actionIgnore_user_ctx_triggered();
    void on_actionCompose_msg_ctx_triggered();
    void on_actionShow_Chatbox_ctx_triggered();
    //void on_actionKick_user_off_podium_triggered();
    //void on_actionIgnore_triggered();
};

#endif // SIMPLE_PUBLIC_H
