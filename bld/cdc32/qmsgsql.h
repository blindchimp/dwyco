
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef QMSGSQL
#define QMSGSQL
#include "vc.h"

namespace dwyco {
void init_qmsg_sql();
void exit_qmsg_sql();
vc load_msg_index(vc uid, int load_count);
long sql_get_max_logical_clock();
vc sql_get_recent_users(int *total_count);
vc sql_get_old_ignored_users();
vc sql_get_empty_users();
vc sql_get_no_response_users();
void sql_index_all();
void clear_msg_idx_uid(vc uid);
vc get_unfav_msgids(vc uid);
void clear_indexed_flag(vc uid);
vc sql_get_recent_users2(int max_age, int max_count);
vc msg_idx_get_new_msgs(vc uid, vc logical_clock);
vc sql_get_uid_from_mid(vc mid);
}

#endif
