
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SYNCCALLS_H
#define SYNCCALLS_H
#include "mmchan.h"
#include "dhgsetup.h"

ChanList get_all_sync_chans();
vc build_sync_status_model();
void sync_call_setup();
void drop_all_sync_calls(dwyco::DH_alternate *);
vc uids_to_call();

// UID, STATUS, IP, PROXY, LOCAL, GLOBAL, PULLS_ASSERT, PULLS_QED, SENDQ_COUNT, INQ_COUNT, TOMB_COUNT
#define M_UID 0
#define M_STATUS 1
#define M_IP 2
#define M_PROXY 3
#define M_LOCAL 4
#define M_GLOBAL 5
#define M_PULLS_ASSERT 6
#define M_PULLS_QED 7
#define M_SENDQ_COUNT 8
#define M_INQ_COUNT 9
#define M_TOMB_COUNT 10
#define M_PERCENT_SYNCED 11

#endif

