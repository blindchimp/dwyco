
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef MSGDDLL_H
#define MSGDDLL_H

#include "msgdisp.h"
#include "dlli.h"

class MsgDisplayDLL : public MessageDisplay
{
public:
    MsgDisplayDLL(DwycoStatusCallback cb, int mid, void *arg)
    {
        callback = cb;
        id = mid;
        user_arg = arg;
    }
    int id;
    void *user_arg;
    DwycoStatusCallback callback;
    int show(const char *);
};

#endif
