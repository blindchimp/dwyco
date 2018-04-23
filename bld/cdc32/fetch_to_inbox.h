
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef FETCH_TO_INBOX_H
#define FETCH_TO_INBOX_H
#include "dwstr.h"

namespace ns_dwyco_background_processing {
int fetch_to_inbox(DwString& uid_out, DwString& mid_out);
}
#endif
