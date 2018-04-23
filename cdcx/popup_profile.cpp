
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "popup_profile.h"
#include "ui_popup_profile.h"
#include <stdio.h>

static QList<DVP> popup_profiles;

popup_profile::popup_profile(QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    ui(new Ui::popup_profile),
    vp(this)
{
    ui->setupUi(this);
    connect(&tmr, SIGNAL(timeout()), this, SLOT(pop_me_up()));
    tmr.setSingleShot(1);
    tmr.start(1000);
    setAttribute(Qt::WA_QuitOnClose, 0);
    popup_profiles.append(vp);
}

popup_profile::~popup_profile()
{
    popup_profiles.removeOne(vp);
    delete ui;
    vp.invalidate();
}

void
popup_profile::pop_me_up()
{
    move(where);
    show();
    raise();
}

int
profile_up(QByteArray uid)
{
    int n = popup_profiles.count();
    for(int i = 0; i < n; ++i)
    {
        if(!popup_profiles[i].is_valid())
            continue;
        popup_profile *p = (popup_profile *)(void *)popup_profiles[i];
        if(p && p->uid == uid)
        {
            //printf("profile up");
            return 1;
        }
    }
    return 0;
}

void
close_profile(QByteArray uid)
{
    int n = popup_profiles.count();
    QList<popup_profile *> dl;
    for(int i = 0; i < n; ++i)
    {
        if(!popup_profiles[i].is_valid())
            continue;
        popup_profile *p = (popup_profile *)(void *)popup_profiles[i];
        if(p && p->uid == uid)
        {
            dl.append(p);

            //p->hide();
            //p->deleteLater();
            //p->vp.invalidate();
        }
    }
    for(int i = 0; i < dl.count(); ++i)
    {
        delete dl[i];
    }
}

void
close_all_profiles()
{
    int n = popup_profiles.count();
    QList<DVP> tmp = popup_profiles;
    for(int i = 0; i < n; ++i)
    {
        if(!tmp[i].is_valid())
            continue;
        popup_profile *p = (popup_profile *)(void *)tmp[i];
        if(p)
        {
            //p->hide();
            //p->deleteLater();
            //p->vp.invalidate();
            delete p;
        }
    }
}
