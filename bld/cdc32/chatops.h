
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CHATOPS_H
#define CHATOPS_H

#include "mmchan.h"

int start_chat_thread(vc ip, vc port, const char *pw);
int start_chat_thread2(vc ip, vc port, const char *pw, StatusCallback scb);
void stop_chat_thread();

#endif
