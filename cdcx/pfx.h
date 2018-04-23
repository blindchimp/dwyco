
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef PFX_H
#define PFX_H
#include "dwstr.h"

extern DwOString User_pfx;
extern DwOString Sys_pfx;
extern DwOString Tmp_pfx;

DwOString add_pfx(const DwOString& pfx, const DwOString& fn);
#endif
