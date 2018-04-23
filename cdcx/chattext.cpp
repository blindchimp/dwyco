
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "chattext.h"
#include "mainwin.h"

chattext::chattext(QWidget *parent) :
    QTextBrowser(parent)
{
}

void chattext::resizeEvent(QResizeEvent *e)
{
    QTextBrowser::resizeEvent(e);
    cursor_to_bottom(this);

}
