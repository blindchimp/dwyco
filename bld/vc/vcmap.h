
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCMAP_H
#define VCMAP_H

// Control flow is handled by C++ exceptions (VcRet, VcBreak, VcExc, VcErr)
// instead of the old manual flag-based unwinding.

#define USER_BOMB(str, ret) do { if(Throw_user_panic) throw -1; user_panic(str); throw VcErr(vc(str), vcnil); } while(0)
#define USER_BOMB2(str) do { if(Throw_user_panic) throw -1; user_panic(str); throw VcErr(vc(str), vcnil); } while(0)

#define NONLH_CHECK_ANY_BO(ret) ((void)0)

#include "vcdbg.h"
#include "vcctx.h"
#include "vctrt.h"
extern vcctx *Vcmap;
extern int Throw_user_panic;

#endif
