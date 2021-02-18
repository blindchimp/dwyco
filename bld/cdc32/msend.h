
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef MSEND_H
#define MSEND_H
#include "dwstr.h"
#include "vc.h"
namespace dwyco {
int send_best_way(const DwString& qfn, vc ruid);
int send_via_server(const DwString& qfn);
int send_via_server_deferred(const DwString& qfn);
int kill_message(const DwString& qfn);
}

#endif
