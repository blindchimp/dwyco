
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef config_h
#define config_h

#include <QWidget>
#include <QTimer>
#include "ui_config.h"

class configform : public QDialog
{
    Q_OBJECT

public:
    configform(QDialog *parent = 0);

    void load();
private:
    void load_untrash_button();
    void refresh_tox_tab();
    void refresh_tox_friend_list();
    void update_tox_status_indicator();
    void set_tox_widgets_enabled(bool enabled);

private slots:
    void on_CDC_call_acceptance__max_audio_textChanged(QString );
    void on_revert_auto_reply_clicked();
    void on_create_auto_reply_clicked();
    void on_CDC_net__primary_port_cursorPositionChanged(int , int );
    virtual void accept();
    virtual void reject();
    virtual void on_CDC_zap__no_forward_default_stateChanged(int);
    virtual void on_pal_auth_stateChanged(int);
    virtual void on_run_bw_test_button_clicked();

    void on_empty_trash_clicked();

    void on_untrash_clicked();

    void on_reset_backup_button_clicked();

    void on_restore_button_clicked();

    void on_pals_only_clicked(bool checked);

    void on_sync_enable_clicked(bool checked);

    void on_show_password_clicked(bool checked);

    void on_sync_refresh_button_clicked();

    void on_tox_enable_toggled(bool checked);
    void on_tox_update_name_clicked();
    void on_tox_copy_id_clicked();
    void on_tox_add_friend_clicked();
    void on_tox_delete_friend_clicked();
    void on_tox_user_status_changed(int index);
    void on_tox_friend_list_doubleClicked(const QModelIndex &index);

signals:
    void content_filter_event(int);
    void pals_only(int);

protected:
    void showEvent(QShowEvent *ev) override;

public:
    Ui::config_dialog ui;
    QTimer *tox_refresh_timer;
};

extern configform *TheConfigForm;
#endif
