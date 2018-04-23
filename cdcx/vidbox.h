
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VIDBOX_H
#define VIDBOX_H

#include <QDockWidget>
#include <QToolButton>
#include <QToolBar>


namespace Ui {
class vidbox;
}

class vidbox : public QDockWidget
{
    Q_OBJECT

public:
    explicit vidbox(QWidget *parent = 0);
    ~vidbox();

    int ui_id;

    void leaveEvent(QEvent *);
    void resizeEvent(QResizeEvent *);
    QToolButton *pause_button;
    QToolBar *toolbar;

private:
    int set_paused;

public slots:
    void camera_event(int);
    void mouse_stopped_event(QPoint);
    void on_actionPause_triggered(bool);
    void on_actionVideo_options_triggered();
    void display_preview(QImage);
    void on_vid_toggle_clicked(bool);
    void on_actionPause_toggled(bool arg1);

public:
    Ui::vidbox *ui;
};

#endif // VIDBOX_H
