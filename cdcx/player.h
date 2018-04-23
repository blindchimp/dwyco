
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include "ui_player.h"
#include "dwstr.h"
#include "dvp.h"

class player : public QWidget
{
    Q_OBJECT


public:
    player(QWidget * parent = 0, Qt::WindowFlags flags = 0 );
    virtual ~player();
    void setText(QString) {}

    DVP vp;
    static QList<DVP> Players;

    int viewid;
    int ui_id;
    DwOString filename;
    DwOString uid;
    DwOString mid;

    int preview_saved_msg(DwOString uid, DwOString mid);

    void do_file_save();
    //void resizeEvent(QResizeEvent *);

    void reset();

public slots:
    void on_play_button_clicked();
    void on_stop_button_clicked();

public:
    Ui::Player ui;

private slots:
    void on_close_button_clicked();
};

#endif
