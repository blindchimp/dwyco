
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "msgddll.h"

int
MsgDisplayDLL::show(const char *msg)
{
    if(callback)
        (*callback)(id, msg, -1, user_arg);
    return 1;
}
