
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
void remove_msg_idx_uid(vc uid);
void remove_msg_idx(vc uid, vc mid);
void update_msg_idx(vc recip, vc body);
int msg_index_count(vc uid);

void sql_fav_remove_uid(vc uid);
void sql_fav_remove_mid(vc mid);
void sql_fav_set_fav(vc mid, int fav);
int sql_fav_is_fav(vc mid);
int sql_fav_has_fav(vc from_uid);

void sql_add_tag(vc mid, vc tag);
void sql_remove_tag(vc tag);
void sql_remove_mid_tag(vc mid, vc tag);
vc sql_get_tagged_mids(vc tag);
vc sql_get_tagged_mids2(vc tag);
vc sql_get_tagged_idx(vc tag);
int sql_mid_has_tag(vc mid, vc tag);
int sql_uid_has_tag(vc uid, vc tag);
int sql_uid_count_tag(vc uid, vc tag);
vc sql_get_all_idx();
void sql_set_rescan(int r);
int sql_get_rescan();
int sql_count_tag(vc tag);
void sql_start_transaction();
void sql_commit_transaction();
void sql_rollback_transaction();
int sql_is_mid_local(vc mid);
void import_remote_mi(vc remote_uid);
vc sql_get_uid_from_mid2(vc mid);

int sql_run_sql(vc s, vc a1, vc a2, vc a3);
vc package_downstream_sends(vc remote_uid);
void import_remote_iupdate(vc remote_uid, vc vals);
void import_remote_tupdate(vc remote_uid, vc vals);
vc sql_get_non_local_messages();


}

#endif
