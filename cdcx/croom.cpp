
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QtGui>
#include <QMessageBox>
#include "ui_croom.h"
#include "croom.h"
#include "dlli.h"
#include "dwstr.h"
#include "qval.h"

croomform::croomform(QDialog *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    ui.lineEdit->setValidator(size_validator());
}

void
croomform::accept()
{
    if(!ui.lineEdit->hasAcceptableInput())
    {
        QMessageBox::critical(this, "Name too short", "Please enter a lobby name that is at least 3 characters long.");
        return;
    }
    QDialog::accept();
}

