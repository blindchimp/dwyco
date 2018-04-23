
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCLHFAC_H
#define VCLHFAC_H
#ifdef LHOBJ
// $Header: g:/dwight/repo/vc/rcs/vclhfac.h 1.43 1996/11/17 05:59:42 dwight Stable $

// function prototypes for factory creation function hooked to
// LH.

#include "vc.h"

vc vclh_make_factory(VCArglist *a);
vc vclh_lmake_factory(VCArglist *a);
vc vclh_gmake_factory(VCArglist *a);

#endif
#endif
