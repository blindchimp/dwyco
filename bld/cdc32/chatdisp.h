
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/chatdisp.h 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef CHATDISP_H
#define CHATDISP_H
#include "vc.h"
class ChatDisplay
{
public:
    virtual ~ChatDisplay() {}

    virtual int output(const char *who, int len_who, const char *msg, int len_msg, vc from_uid, int = -1) = 0;
    virtual int output(vc log, int id) {
        return 0;
    }

};
#endif
