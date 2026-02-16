
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef XINFO_H
#define XINFO_H
#include "vc.h"
class DwString;
namespace dwyco {
int save_info(vc info, const DwString& filename, int nomodify = 0);
int load_info(vc& info, const DwString& filename, int nomodify = 0);
int save_info_e(vc info, const DwString& filename, int nomodify = 0);
int load_info_e(vc& info, const DwString& filename, int nomodify = 0);
}

#endif
