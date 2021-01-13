
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
void init_sql_settings();

vc get_settings_value(const char *name);
int set_settings_value(const char *name, const char *value);
int set_settings_value(const char *name, int);
int set_settings_value(const char *name, vc);
void bind_sql_setting(vc name, void (*fn)(vc, vc));
void bind_sql_section(vc pfx, void (*fn)(vc, vc));
//template<class T> int set_settings_value(const char *name, T val);
#endif
