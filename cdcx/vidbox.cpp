
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vidbox.h"
#include "ui_vidbox.h"
#include "mainwin.h"
#include "vidsel.h"
extern VidSel *DevSelectBox;
extern int Public_chat_video_pause;
extern int HasCamera;
extern QImage Last_preview_image;


vidbox::vidbox(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::vidbox)
{
    ui->setupUi(this);
    ui_id = -1;

    QRect r = rect();

    toolbar = new QToolBar(this);
    toolbar->addAction(ui->actionPause);
    toolbar->addAction(ui->actionVideo_options);
    toolbar->setVisible(0);
    toolbar->move(3, r.height() - toolbar->height() - 3);
    ui->vid_toggle->setVisible(0);
    set_paused = 0;

    connect(Mainwinform, SIGNAL(mouse_stopped(QPoint)), SLOT(mouse_stopped_event(QPoint)));
    connect(Mainwinform, SIGNAL(camera_change(int)), this, SLOT(camera_event(int)));
    connect(Mainwinform, SIGNAL(video_capture_preview(QImage)), this, SLOT(display_preview(QImage)));
    connect(ui->actionPause, SIGNAL(toggled(bool)), Mainwinform, SIGNAL(global_video_pause_toggled(bool)));
}

vidbox::~vidbox()
{
    delete ui;
    delete toolbar;
}

void
vidbox::display_preview(QImage img)
{
    if(ui->actionPause->isChecked())
    {
        if(set_paused)
            return;
        ui->vid->setPixmap(QPixmap::fromImage(Last_preview_image));
        set_paused = 1;
        return;
    }
    ui->vid->setPixmap(QPixmap::fromImage(img));
    set_paused = 0;
}

void
vidbox::camera_event(int on)
{
    if(!on)
    {
        // unpause the video
        Public_chat_video_pause = 0;
        ui->actionPause->setChecked(0);
        ui_id = -1;
        ui->vid_toggle->setVisible(1);
        ui->vid->setVisible(0);
        ui->vid_toggle->setChecked(0);
        toolbar->setVisible(0);
        set_paused = 0;
    }
    else
    {
        ui->vid_toggle->setChecked(1);
        ui->vid->setVisible(1);
        ui->vid_toggle->setVisible(0);
    }
}

void
vidbox::mouse_stopped_event(QPoint pnt)
{
    if(!underMouse())
        return;
    if(HasCamera)
        toolbar->setVisible(1);
    ui->vid_toggle->setVisible(1);

}

void
vidbox::leaveEvent(QEvent *e)
{
    if(!ui->actionPause->isChecked())
    {
        toolbar->setVisible(0);
    }
    if(HasCamera)
        ui->vid_toggle->setVisible(0);
    else
        ui->vid_toggle->setVisible(1);
    QDockWidget::leaveEvent(e);
}

void
vidbox::resizeEvent(QResizeEvent *)
{
    QRect r = rect();
    toolbar->move(3, r.height() - toolbar->height() - 3);
}

void vidbox::on_actionPause_triggered(bool state)
{
    Public_chat_video_pause = state;
    dwyco_field_debug("vid-pause-trigger", 1);
}

void vidbox::on_actionVideo_options_triggered()
{
    DevSelectBox->show();
    DevSelectBox->raise();
}

void
vidbox::on_vid_toggle_clicked(bool a)
{
    if(DevSelectBox)
        DevSelectBox->toggle_device(a);
    dwyco_field_debug("vid-toggle", 1);
}
void vidbox::on_actionPause_toggled(bool arg1)
{
    Public_chat_video_pause = arg1;
    dwyco_field_debug("vid-pause", 1);
}
