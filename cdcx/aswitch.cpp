
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "aswitch.h"
#include <QPainter>

aswitch::aswitch(QWidget *parent) :
    QPushButton(parent)
{
    setCheckable(true);
}

aswitch::~aswitch()
{

}


void
aswitch::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::gray);
    QRect switchRect;
    QColor switchColor;
    QString text;
    int halfWidth = width() / 2;
    if (isChecked()) {
        switchRect = QRect(halfWidth, 0, halfWidth, height());
        switchColor = QColor(Qt::cyan);
        text = "Turn off";
    } else {
        switchRect = QRect(0, 0, halfWidth, height());
        switchColor = QColor(Qt::darkGray);
        text = "Turn On";
    }
    painter.fillRect(switchRect, switchColor);
    painter.drawText(switchRect, Qt::AlignCenter, text);
}
