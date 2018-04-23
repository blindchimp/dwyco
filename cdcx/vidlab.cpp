
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vidlab.h"
#include "mainwin.h"
#include <QMouseEvent>

vidlab::vidlab(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{

    //QSizePolicy qp(QSizePolicy::Preferred, QSizePolicy::Preferred, QSizePolicy::Label);
    //qp.setHeightForWidth(1);
    //setSizePolicy(qp);
    ui_id = -1;
    connect(Mainwinform, SIGNAL(video_display(int,QImage)), this, SLOT(display_video(int,QImage)));
}
#if 0
int
vidlab::heightForWidth(int width) const
{
    float f = 240./320.;

    return (int)(width * f);
}
QSize
vidlab::sizeHint() const
{
    return QSize(160, 120);
}
#endif

void
vidlab::display_video(int ui_id, QImage img)
{
    if(ui_id == this->ui_id)
        setPixmap(QPixmap::fromImage(img));
}

void
vidlab::resizeEvent(QResizeEvent *re)
{

    if(local_pixmap.isNull())
        return;

    QPixmap p2 = local_pixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QLabel::setPixmap(p2);
    QLabel::resizeEvent(re);
    emit resized();
}

void
vidlab::showEvent(QShowEvent *e)
{
    QLabel::showEvent(e);
    emit resized();
}

void
vidlab::leaveEvent(QEvent *e)
{
    QLabel::leaveEvent(e);
    emit mouse_leave();
}

void
vidlab::mouseReleaseEvent(QMouseEvent *ev)
{
    QLabel::mouseReleaseEvent(ev);
    if(ev->button() == Qt::LeftButton)
        emit mouse_release();
}

void
vidlab::mousePressEvent(QMouseEvent *ev)
{
    QLabel::mousePressEvent(ev);
    if(ev->button() == Qt::LeftButton)
        emit mouse_press();
}


void
vidlab::setPixmap(const QPixmap &p)
{
    local_pixmap = p;
    QPixmap p2;
    if(p.isNull())
        p2 = p;
    else
        p2 = p.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QLabel::setPixmap(p2);
}

void
vidlab::clear()
{
    local_pixmap = QPixmap();
    QLabel::clear();

}


