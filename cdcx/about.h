
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef about_h
#define about_h

#include <QWidget>
#include "ui_about.h"

class aboutform : public QDialog
{
    Q_OBJECT

public:
    aboutform(QDialog *parent = 0);

private slots:

private:
    Ui::about_dialog ui;
};
#endif
