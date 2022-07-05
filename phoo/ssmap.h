
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SSMAP_H
#define SSMAP_H
class QString;

void settings_save();
int settings_load();
int setting_get(const QString& key, QString& out);
int setting_put(const QString& key, const QString& value);


#endif
