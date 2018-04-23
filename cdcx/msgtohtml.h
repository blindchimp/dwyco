
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef MSGTOHTML_H
#define MSGTOHTML_H
class QTextTable;

#include "dlli.h"

int msg_to_table(DWYCO_SAVED_MSG_LIST m, QTextTable *table);
QString gen_time(DWYCO_SAVED_MSG_LIST l, int row);
#endif
