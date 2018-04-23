
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCFEXT_H
#define VCFEXT_H
// $Header: g:/dwight/repo/vc/rcs/vcfext.h 1.45 1996/11/17 05:58:37 dwight Stable $

// external defs for functors

// function styles - mix and match
#define VC_FUNC_VARADIC 1
#define VC_FUNC_DONT_EVAL_ARGS 2
#define VC_FUNC_CONSTRUCT 4

// common styles
#define VC_FUNC_NORMAL 0
#define VC_FUNC_BUILTIN_LEAF (VC_FUNC_CONSTRUCT)
#define VC_FUNC_VBUILTIN_LEAF (VC_FUNC_CONSTRUCT|VC_FUNC_VARADIC)

#endif
