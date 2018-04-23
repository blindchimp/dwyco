
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SQLBQ_H
#define SQLBQ_H
#include "vc.h"

namespace dwyco {

vc sqlite3_bulk_query(sqlite3 *dbs, VCArglist *a);

}

#endif
