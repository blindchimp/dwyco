
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef GRPMSG_H
#define GRPMSG_H
#include "vc.h"
#include "ssns.h"

namespace dwyco {
int init_gj();
int exit_gj();
void clear_gj();
void clean_gj();
vc get_status_gj();
vc get_join_log();
void add_join_log(vc msg, vc uid);

// initiator
int start_gj(vc target_uid, vc gname, vc password);
int recv_gj2(vc from, vc msg, vc password);
int install_group_key(vc from, vc msg, vc password);
//responder
int recv_gj1(vc from, vc msg, vc password);
int recv_gj3(vc from, vc msg, vc password);

extern ssns::signal1<vc> Join_signal;
extern ssns::signal2<vc, vc> Join_attempts;

}
#endif
