
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef evret_h
#define evret_h

#include <QObject>

struct ReturnFilter : public QObject
{
    Q_OBJECT
public:
    ReturnFilter(QObject *p = 0);
    bool eventFilter(QObject *obj, QEvent *e);
signals:
    void return_hit();
    void ctrl_return_hit();
    void chat_typing();
    void esc_hit();

};

struct LocationFilter : public QObject
{
    Q_OBJECT
public:
    LocationFilter(QObject *p = 0) : QObject(p) {}
    bool eventFilter(QObject *, QEvent *);
};

struct ResizeFilter : public QObject
{
    Q_OBJECT
public:
    ResizeFilter(QObject *p = 0) : QObject(p) {}
    bool eventFilter(QObject *, QEvent *);
signals:
    void resized();
};

struct hover_filter : public QObject
{
    Q_OBJECT
public:
    hover_filter(QObject *p = 0) {}
    bool eventFilter(QObject *, QEvent *);
};

#endif
