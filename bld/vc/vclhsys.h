
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCLHSYS_H
#define VCLHSYS_H
// $Header: g:/dwight/repo/vc/rcs/vclhsys.h 1.38 1999/02/04 18:43:02 dwight Exp $

#include "vc.h"

vc vclh_getenv(vc name);
vc vclh_putenv(vc name, vc value);
vc vclh_dir2map(vc dir);
vc vclh_file_exists(vc file);
vc vclh_file_remove(vc file);
vc vclh_file_access(vc file, vc how);
vc vclh_file_rename(vc from, vc to);
vc vclh_file_size(vc file);
vc vclh_file_stat(vc file);
vc vclh_time(void);
vc vclh_time_hp(void);
vc vclh_time_hp2(void);
vc vclh_strftime(vc time, vc formt);
vc vclh_strftime_hp(vc formt);
vc vclh_sleep(vc seconds);
vc vclh_system(vc cmd);
vc vclh_process_create(VCArglist *va);
vc vclh_clean_zombies();
vc vclh_clean_zombies2(VCArglist *a);
vc vclh_alarm(vc);
vc vclh_kill(vc, vc);

#endif
