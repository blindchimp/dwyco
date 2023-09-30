
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef BACKSQL_H
#define BACKSQL_H
namespace dwyco {
extern int Enable_backups;
extern int Backup_freq;
int create_msg_backup();
void reset_msg_backup();
int restore_msgs(const char *fn, int msgs_only);
}
#endif
