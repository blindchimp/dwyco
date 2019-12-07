
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/doinit.h 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef DOINIT_H
#define DOINIT_H
#include "vc.h"
#include "dwlog.h"


void init_codec(const char *logname = "dwyco.log");
void exit_codec();

void init_bg_msg_send(const char *logname);
void exit_bg_msg_send();
extern vc Myhostname;
extern vc TheMan;
extern DwLog *Log;
#define INI_FILENAME "dwyco.ini"

#endif
