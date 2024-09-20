
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef BACKANDROID_H
#define BACKANDROID_H

namespace dwyco {
void android_backup();
int android_restore_msgs();
int android_days_since_last_backup();
int android_get_backup_state();
int android_set_backup_state(int i);
int desktop_backup();
int desktop_days_since_last_backup();
int desktop_days_since_backup_created();
int restore_msgs(const char *fn, int msgs_only);
extern int Enable_backups;
extern int Backup_freq;
}

#endif // BACKANDROID_H
