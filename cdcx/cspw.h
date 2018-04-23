
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef cspw_h
#define cspw_h

#include <QWidget>
#include <QDialog>
#include "ui_cspw.h"
#include "dwstr.h"

class cspwform : public QDialog
{
    Q_OBJECT

public:
    cspwform(QDialog *parent = 0);

private slots:
    void accept();

public:
    Ui::cspw_dialog ui;
};
#endif
