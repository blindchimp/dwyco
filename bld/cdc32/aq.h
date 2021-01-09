
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/aq.h 1.4 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef AQ_H
#define AQ_H

#include "vidaq.h"

class KeyboardAcquire;

int initaq(int mbox, DwString &fail_reason);
void exitaq();

int init_msgaq(int mbox, DwString &fail_reason);
void exit_msgaq();
KeyboardAcquire *init_msgaq_private(int mbox, int& chatbox_id, const char *caption, int chan_id, DwString &fail_reason);


extern VidAcquire *TheAq;
extern KeyboardAcquire *TheMsgAq;
extern int Chatbox_id;
extern int ExternalVideoAcquisition;

#endif
