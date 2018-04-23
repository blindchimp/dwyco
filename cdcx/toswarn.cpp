
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "toswarn.h"
#include "ui_toswarn.h"

toswarn::toswarn(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::toswarn)
{
    ui->setupUi(this);
}

toswarn::~toswarn()
{
    delete ui;
}
