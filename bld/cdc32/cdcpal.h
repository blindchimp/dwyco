
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CDCPAL_H
#define CDCPAL_H

#include "vc.h"
#include "pval.h"
#include "ssns.h"
namespace dwyco {
int init_pal();
void exit_pal();
int pal_logout();
int pal_set_state(vc state);
int pal_tick();
void pal_reset();
int pal_login();
void pal_relogin();
void async_pal(vc, void *, vc, ValidPtr);

extern int Inhibit_pal;
extern int Pal_logged_in;
extern ssns::signal0 Online_info;
}

#endif
