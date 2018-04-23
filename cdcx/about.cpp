
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QtGui>
#include "ui_about.h"
#include "about.h"
#include "dwstr.h"
extern DwOString Version;

aboutform::aboutform(QDialog *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    ui.version->setText(Version.c_str());
}

