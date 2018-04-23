
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef userwid_h
#define userwid_h

#include <QWidget>
#include "ui_userwid.h"

class userwid : public QWidget
{
    Q_OBJECT

public:
    userwid(QWidget *parent = 0, Qt::WindowFlags flags = 0);


private slots:

private:
    Ui::userwid ui;
};
#endif
