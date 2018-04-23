
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef PVAL_H
#define PVAL_H

// $Header: g:/dwight/repo/cdc32/rcs/pval.h 1.10 1999/01/10 16:10:59 dwight Checkpoint $
#include "dwvp.h"
typedef class DwVP ValidPtr;
#define cookie_to_ptr(x) DwVP::cookie_to_ptr(x)
#define init_pval() DwVP::init_dvp()

#endif
