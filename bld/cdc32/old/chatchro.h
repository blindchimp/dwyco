
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CHATCHRO_H
#define CHATCHRO_H
// $Header: g:/dwight/repo/cdc32/rcs/chatchro.h 1.2 1997/08/08 05:26:39 dwight Stable095 $
#include "vc.h"
#include "chatdisp.h"
#include "chatbox.h"

class ChatCharOwl : public ChatDisplay
{
public:
    ChatCharOwl();

    int output(const char *who, const char *msg, int gvid = -1) {}
    int output(vc log, int id);

private:
    ChatCharOwl(ChatBox *);
    ChatBox *box;
    uint curs0;
    uint curs1;


};

#endif
