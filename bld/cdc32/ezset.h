
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef EZSET_H
#define EZSET_H

#include "vc.h"

namespace dwyco {
void init_sql_settings();
namespace ezset {
void sql_start_transaction();
void sql_commit_transaction();
void sql_rollback_transaction();
}

vc get_settings_value(const char *name);
int set_settings_value(const char *name, const char *value);
int set_settings_value(const char *name, int);
int set_settings_value(const char *name, vc);
void bind_sql_setting(vc name, void (*fn)(vc, vc));
void bind_sql_section(vc pfx, void (*fn)(vc, vc));
int settings_get_eager_mode();
}
//template<class T> int set_settings_value(const char *name, T val);
#endif
