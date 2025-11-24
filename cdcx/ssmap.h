
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SSMAP_H
#define SSMAP_H
#include "dwstr.h"
class QString;
void settings_save2();
void settings_load();
int setting_get(const DwOString& key, DwOString& out);
int setting_get(const DwOString& key, int& out);
int setting_put(const DwOString& key, int val, int save = 1);
int setting_put(const DwOString& key, const DwOString& val, int save = 1);
int setting_put(const DwOString& key, const QString& val, int save = 1);

#endif
