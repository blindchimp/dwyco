
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCMAP_H
#define VCMAP_H
// $Header: g:/dwight/repo/vc/rcs/vcmap.h 1.49 1998/06/11 06:53:42 dwight Exp $

// Use these macros after every call to "eval" that has
// stuff after it that must be skipped during an exception
// unwind (yes, you could do this using C++ exceptions
// or some gross variant of longjmp, but we ain't got
// either of those at the moment.)
//
#define CHECK_EXC_BO
#define CHECK_EXC_BO_LOOP
#define USER_BOMB(str, ret) do {if(Throw_user_panic) throw -1; user_panic(str); Vcmap->set_dbg_backout(); throw VcErr(vc(str), vcnil);} while(0)
// need this because some stupid cpp's don't handle null macro args right
#define USER_BOMB2(str) do {if(Throw_user_panic) throw -1; user_panic(str); Vcmap->set_dbg_backout(); throw VcErr(vc(str), vcnil);} while(0)

#define CHECK_DBG_BO(ret) if(Vcmap->dbg_backout_in_progress()) return ret
#define CHECK_DBG_BO_LOOP if(Vcmap->dbg_backout_in_progress()) break

#define CHECK_ANY_BO(ret)
#define CHECK_ANY_BO_CLEANUP(cleanup, ret)
#define CHECK_ANY_BO_LOOP

#if defined(__WIN32__)
#define NONLH_CHECK_ANY_BO(ret)
#else
#define NONLH_CHECK_ANY_BO(ret)
#endif
#include "vcdbg.h"
#include "vcctx.h"
#include "vctrt.h"
extern vcctx *Vcmap;
extern int Throw_user_panic;

#endif
