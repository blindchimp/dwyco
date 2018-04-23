
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef USERLOB_H
#define USERLOB_H
#include <QList>
#include "dwstr.h"

struct user_lobby
{
    DwOString id;
    DwOString hostname;
    DwOString ip;
    int port;
    DwOString rating;
    DwOString dispname;
    DwOString category;
    DwOString subgod_uid;
    int max_users;
    int operator==(const user_lobby& a) const {
        return id == a.id;
    }
};

extern QList<user_lobby> User_lobbies;

#endif
