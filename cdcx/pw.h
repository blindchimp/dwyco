
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef pw_h
#define pw_h

#include <QWidget>
#include <QDialog>
#include "ui_pw.h"
#include "dwstr.h"

class pwform : public QDialog
{
    Q_OBJECT

public:
    pwform(QDialog *parent = 0);
    QString npw;
    void reset();

private slots:
    void accept();
    void reject();

    void on_fetch_pw_button_clicked();


public:
    Ui::pw_dialog ui;
};

extern pwform *ThePwForm;
#endif
