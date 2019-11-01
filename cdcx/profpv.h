
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef PROFPV_H
#define PROFPV_H
#include <QObject>
#include <QPixmap>
#include "dwstr.h"

class profpv : public QObject
{
    Q_OBJECT
private:
    static int Chat_online;
public:
    profpv();

    QPixmap get_preview_by_uid(const DwOString& uid);
    void refresh_profile(const DwOString& uid);

public slots:
    void clear_cache(int);
    void remove_entry(DwOString);
    void chat_server(int);

signals:
    void profile_resolved(DwOString);
};

extern profpv *ThePreviewCache;
#endif
