
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef login_h
#define login_h

#include <QWidget>
#include <QDialog>
#include "ui_login.h"
#include "dwstr.h"

class loginform : public QDialog
{
    Q_OBJECT

public:
    loginform(QWidget *parent = 0, Qt::WindowFlags = Qt::WindowFlags());

    void new_account();
    int is_new_account;
    int save_pw();
    DwOString pw();
    DwOString gen_pw;
    int tries;

private slots:
    void accept();
    void reject();

    void on_fetch_pw_button_clicked();

public:
    Ui::login_dialog ui;
};
#endif
