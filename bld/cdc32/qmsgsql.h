
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
#include "dwstr.h"

namespace dwyco {
void init_qmsg_sql();
void exit_qmsg_sql();

namespace qmsgsql {
void sql_start_transaction();
void sql_commit_transaction();
void sql_rollback_transaction();
}

void init_group_map();
void remove_sync_state();

vc load_msg_index(vc uid, int load_count);
long sql_get_max_logical_clock();
vc sql_get_recent_users(int recent, int *total_count);
vc sql_get_old_ignored_users();

//void sql_index_all();
void clear_msg_idx_uid(vc uid);
vc get_unfav_msgids(vc uid);
void clear_indexed_flag(vc uid);
vc sql_get_recent_users2(int max_age, int max_count);
vc msg_idx_get_new_msgs(vc uid, vc logical_clock);
vc sql_get_uid_from_mid(vc mid);
void remove_msg_idx_uid(vc uid);
void remove_msg_idx(vc uid, vc mid);
int update_msg_idx(vc recip, vc body, int inhibit_sysmsg);
int msg_index_count(vc uid);

void sql_remove_all_tags_uid(vc uid);
void sql_remove_all_tags_mid(vc mid);
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
bool sql_exists_valid_tag(vc tag);
int sql_count_valid_tag(vc tag);
int sql_is_mid_local(vc mid);
int sql_is_mid_anywhere(vc mid);
int sql_mid_has_tombstone(vc mid);
int import_remote_mi(vc remote_uid);
vc sql_find_who_has_mid(vc mid);
bool sql_has_msg_recently(vc uid, long num_seconds);
vc sql_last_recved_msg(vc uid);

vc sql_run_sql(vc s, vc a1 = vcnil, vc a2 = vcnil, vc a3 = vcnil);
vc package_downstream_sends(vc remote_uid);
vc import_remote_iupdate(vc remote_uid, vc vals);
void import_remote_tupdate(vc remote_uid, vc vals);
vc sql_get_non_local_messages_at_uid(vc uid, int max_count);
vc sql_uid_updated_since(vc time);

vc sql_dump_mi();
vc sql_dump_mt();

void reindex_possible_changes();

vc map_uid_to_gid(vc uid);
vc map_gid_to_uid(vc gid);
vc map_gid_to_uids(vc gid);
vc map_uid_to_uids(const vc &uid);
vc map_uid_list_from_tag(vc tag);
// returns number of uid's folded onto uid, or 0 if there is no folding
int map_is_mapped(vc uid);
// returns the uid that represents the group uid is in.
// this is the smallest uid in the current group.
vc map_to_representative_uid(vc uid);
void refetch_pk(int online);

void add_pull_failed(const vc &mid, const vc &uid);
void clean_pull_failed_mid(const vc &mid);
void clean_pull_failed_uid(const vc& uid);
bool pull_failed(const vc& mid, const vc& uid);

vc get_delta_id(vc uid);
bool generate_delta(vc uid, vc delta_id);
void create_dump_indexes(const DwString& fn);
void import_new_syncpoint(vc remote_uid, vc delta_id);

extern DwString Schema_version_hack;

// note: the message index is disposable, and can be rebuilt
// from the files stored locally.
// the tag database contains items the user has created
// and that have been gleaned from other tag databases
// in the device group. it *can* be disposed of, but you
// may lose some information if it hasn't been propagated to
// another device. the information isn't crucial for operation
// though...
// note: these files got created 4/15/2022, sacrificing
// previous databases (mostly internal and from debugging.)
// cdc-x users will have to wait for indexing on the first
// run, so might want to include something to account for that.
#define MSG_IDX_DB "msgs.sql"
#define TAG_DB "tags.sql"

}

#endif
