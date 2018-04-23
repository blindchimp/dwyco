
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef MSGDISP_H
#define MSGDISP_H
// $Header: g:/dwight/repo/cdc32/rcs/msgdisp.h 1.1 1998/05/16 04:07:03 dwight Exp $

class MessageDisplay
{
public:
    MessageDisplay() { }
    virtual ~MessageDisplay() {}

    virtual int show(const char *msg) = 0;

};

#endif
