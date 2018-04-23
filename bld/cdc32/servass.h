
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SERVASS_H
#define SERVASS_H
#include "pval.h"
#include "vc.h"

class MMChannel;
class MessageDisplay;

void start_serv_recv_thread(vc ip, vc port, ValidPtr);
void stop_serv_recv_thread();
void got_serv_r(vc m, void *, vc, ValidPtr);
void start_server_assisted_call(vc to_uid, int use_stun, ValidPtr, MessageDisplay * = 0);
void aux_channel_setup(MMChannel *mc, vc v);
#endif
