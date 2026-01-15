
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


// void
// aswitch::paintEvent(QPaintEvent* event)
// {
//     QPainter painter(this);
//     painter.fillRect(rect(), Qt::gray);
//     QRect switchRect;
//     QColor switchColor;
//     QString text;
//     int halfWidth = width() / 2;
//     if (isChecked()) {
//         switchRect = QRect(halfWidth, 0, halfWidth, height());
//         switchColor = QColor(Qt::cyan);
//         text = "Turn off";
//     } else {
//         switchRect = QRect(0, 0, halfWidth, height());
//         switchColor = QColor(Qt::darkGray);
//         text = "Turn On";
//     }
//     painter.fillRect(switchRect, switchColor);
//     painter.drawText(switchRect, Qt::AlignCenter, text);
// }

// WARNING: AI generated, ca 2026
/*
 * the following qt6 widgets function creates a button with a cyan
 * foreground in one of the states, which is hard to read with white text
 * in dark mode. can you make it work better by honoring dark mode colors
 * for text.
 * (insert above function)
 */
static QColor contrastColor(const QColor &bg)
{
    // Perceived luminance, simple formula.
    const int luminance = qRound(0.299 * bg.red() +
                                 0.587 * bg.green() +
                                 0.114 * bg.blue());

    return luminance > 128 ? Qt::black : Qt::white;
}

void aswitch::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    const QPalette pal = this->palette();
    QColor bgColor   = pal.color(QPalette::Button);
    QColor onColor   = pal.color(QPalette::Highlight);   // background for "on"
    QColor offColor  = pal.color(QPalette::Button);
    QColor offText   = pal.color(QPalette::ButtonText);  // default text color

    painter.fillRect(rect(), bgColor);

    QRect switchRect;
    QString text;
    QColor switchColor;
    QColor textColor;

    int halfWidth = width() / 2;

    if (isChecked()) {
        switchRect  = QRect(halfWidth, 0, halfWidth, height());
        switchColor = onColor;
        text        = QStringLiteral("Turn off");
        textColor   = contrastColor(onColor);   // ensure readable on highlight
    } else {
        switchRect  = QRect(0, 0, halfWidth, height());
        switchColor = offColor;
        text        = QStringLiteral("Turn on");
        textColor   = offText;                  // normal button text color
    }

    painter.fillRect(switchRect, switchColor);
    painter.setPen(textColor);
    painter.drawText(switchRect, Qt::AlignCenter, text);
}
