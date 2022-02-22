
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCTRANS_H
#define VCTRANS_H
#include "vc.h"

vc trans_doif(VCArglist *a, VcIO o);
vc trans_dowhile(VCArglist *a, VcIO o);
vc trans_dobreak(VCArglist *a, VcIO o);
vc trans_compile(VCArglist *a, VcIO o);
vc trans_doreturn(VCArglist *a, VcIO o);
vc trans_domul(VCArglist *a, VcIO o);
vc trans_dodiv(VCArglist *a, VcIO o);
vc trans_dosub(VCArglist *a, VcIO o);
vc trans_doadd(VCArglist *a, VcIO o);
vc trans_lbindfun(VCArglist *a, VcIO o);
vc trans_gbindfun(VCArglist *a, VcIO o);
vc trans_doprog(VCArglist *a, VcIO o);
vc trans_doloop(VCArglist *a, VcIO o);
vc trans_doforeach(VCArglist *a, VcIO o);
vc trans_docand(VCArglist *a, VcIO o);
vc trans_docor(VCArglist *a, VcIO o);
vc trans_doswitch(VCArglist *a, VcIO o);
vc trans_docond(VCArglist *a, VcIO o);
vc trans_eval(VCArglist *a, VcIO o);

#endif
