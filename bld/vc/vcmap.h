
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
#define CHECK_EXC_BO if(Vcmap->backout_in_progress()) return vcnil
#define CHECK_EXC_BO_LOOP if(Vcmap->backout_in_progress()) break
#define USER_BOMB(str, ret) do {if(Throw_user_panic) throw -1; user_panic(str); Vcmap->set_dbg_backout(); return ret;} while(0)
// need this because some stupid cpp's don't handle null macro args right
#define USER_BOMB2(str) do {if(Throw_user_panic) throw -1; user_panic(str); Vcmap->set_dbg_backout(); return;} while(0)

#define CHECK_DBG_BO(ret) if(Vcmap->dbg_backout_in_progress()) return ret
#define CHECK_DBG_BO_LOOP if(Vcmap->dbg_backout_in_progress()) break

#define CHECK_ANY_BO(ret) if(Vcmap->unwind_in_progress()) return ret
#define CHECK_ANY_BO_CLEANUP(cleanup, ret) if(Vcmap->unwind_in_progress()) { cleanup return ret;}
#define CHECK_ANY_BO_LOOP if(Vcmap->unwind_in_progress()) break

// use this macro in situations where LH may or may not be
// active. useful in cases when library is used standalone
// or the processing of an error can wind up coming back
// around to an LH expression (like an exception handler that
// is defined in LH) and you need to terminate processing
// gracefully.
// this get/set last error stuff has to be done because in bc5 the
// *access* to Vcmap (a thread local) clears the current error code.
#if defined(__WIN32__)
#define NONLH_CHECK_ANY_BO(ret) {int a = WSAGetLastError(); if(Vcmap) {CHECK_ANY_BO(ret);} WSASetLastError(a);}
#else
#define NONLH_CHECK_ANY_BO(ret) {if(Vcmap) {CHECK_ANY_BO(ret);}}
#endif
#include "vcdbg.h"
#include "vcctx.h"
extern vcctx *Vcmap;
extern int Throw_user_panic;

#endif
