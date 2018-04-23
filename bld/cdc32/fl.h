
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef FL_H
#define FL_H

#include "dwstr.h"

void load_lhcore();
void faultlog_on(const DwString& systemroot);
void faultlog_off();
DwString faultlog_get_name();
void faultlog_powerup();
void load_stkfault();

#endif
