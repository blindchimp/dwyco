
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef ASWITCH_H
#define ASWITCH_H

#include <QWidget>
#include <QPushButton>

class aswitch : public QPushButton
{
    Q_OBJECT

public:
    explicit aswitch(QWidget *parent = 0);
    ~aswitch();

    void paintEvent(QPaintEvent* event);

};

#endif // ASWITCH_H
