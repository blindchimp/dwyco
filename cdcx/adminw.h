
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef ADMINW_H
#define ADMINW_H

#include <QWidget>
#include <QVariant>
#include "dwstr.h"
#include "dlli.h"
#include "tfhex.h"
#include "userlob.h"

namespace Ui {
class adminw;
}

class adminw : public QWidget
{
    Q_OBJECT

public:
    explicit adminw(QWidget *parent = 0);
    ~adminw();

private slots:
    void on_rules_textChanged();
    void on_update_rules_clicked();
    void on_adminw_activated(int);
    void on_actionRefresh_triggered(bool);
    void on_block_button_clicked(bool);
    void on_unblock_button_clicked(bool);
    void on_mod_button_clicked(bool);
    void on_unmod_button_clicked(bool);
    void on_clear_mods_button_clicked(bool);
    void on_set_sys_attr_clicked(bool);
    void on_create_user_lobby_button_clicked(bool);
    void on_remove_user_lobby_button_clicked(bool);

    void on_pushButton_clicked();

public slots:
    void admin_update(int);
    void uid_selected_event(DwOString, int);
    void chat_event(int,int,QByteArray,QString, QVariant,int,int);
    void add_user_lobby(user_lobby);


public:
    void clear_block();
    void clear_mod();
    void clear_server_list();
    void refresh_from_data();
    void add_mod(QVariant v);
    void add_block(QVariant v);
    void add_server(QVariant v);
    void replace_servers(DWYCO_LIST v);
    void clear_sys_attrs();
    void add_sys_attr(QString name, QVariant v);
    void update_user_lobbies();

private:
    Ui::adminw *ui;
};

extern adminw *Adminform;

#endif // ADMINW_H
