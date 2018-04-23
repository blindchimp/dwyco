
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef iglist_h
#define iglist_h

#include <QWidget>
#include <QDialog>
#include "ui_iglist.h"
#include "dwstr.h"

class iglistform : public QDialog
{
    Q_OBJECT

public:
    iglistform(QDialog *parent = 0);

    void refresh();
private slots:
    void accept();
    void on_refresh_button_clicked();
    void on_unblock_button_clicked();

signals:
    void ignore_event(DwOString);


public:
    Ui::iglist_dialog ui;
};

extern iglistform *TheIglistForm;
#endif
