
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VIDLAB_H
#define VIDLAB_H
#include <QLabel>
#include <QPixmap>
#include <QImage>

class vidlab : public QLabel
{
    Q_OBJECT
public:
    int ui_id;


    vidlab(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~vidlab() {}

    QPixmap local_pixmap;

    //virtual int heightForWidth(int width) const ;
    //virtual QSize sizeHint() const;

    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);
    void leaveEvent(QEvent *);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    virtual void setPixmap(const QPixmap &);
    void clear();

public slots:
    void display_video(int ui_id, QImage img);

signals:
    void resized();
    void mouse_leave();
    void mouse_press();
    void mouse_release();


};

#endif
