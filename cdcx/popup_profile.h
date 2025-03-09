
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef POPUP_PROFILE_H
#define POPUP_PROFILE_H

#include <QWidget>
#include <QTimer>
#include "dvp.h"

namespace Ui {
class popup_profile;
}

class popup_profile : public QWidget
{
    Q_OBJECT

public:
    explicit popup_profile(QWidget *parent = 0, Qt::WindowFlags = Qt::WindowFlags());
    ~popup_profile();
    DVP vp;
    QTimer tmr;
    QPoint where;


    QByteArray uid;

public slots:
    void pop_me_up();

public:
    Ui::popup_profile *ui;
};

#endif // POPUP_PROFILE_H
