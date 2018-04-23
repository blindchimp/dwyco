
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//---------------------------------------------------------------------------

#ifndef sysattrH
#define sysattrH
#include "vc.h"
int sysattr_get_int(const char *name, int def = 0);
vc sysattr_get_vc(const char *name);
void init_sysattr();
void sysattr_put(vc name, vc value);
//---------------------------------------------------------------------------
#endif
