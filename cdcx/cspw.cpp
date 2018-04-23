
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QtGui>
#include "ui_cspw.h"
#include "cspw.h"
#include "dlli.h"
#include "dwstr.h"

cspwform::cspwform(QDialog *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
}

void
cspwform::accept()
{
    QDialog::accept();
}

