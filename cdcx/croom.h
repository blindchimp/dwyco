
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef croom_h
#define croom_h

#include <QWidget>
#include <QDialog>
#include "ui_croom.h"
#include "dwstr.h"

class croomform : public QDialog
{
    Q_OBJECT

public:
    croomform(QDialog *parent = 0);

private slots:
    void accept();

public:
    Ui::croom_dialog ui;
};
#endif
