
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef ACTIVEUID_H
#define ACTIVEUID_H

#include "vc.h"

namespace dwyco {
vc find_best_candidate_for_initial_send(vc uid, int &skip_direct);
}

#endif // ACTIVEUID_H
