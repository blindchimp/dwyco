
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef FAVMSG_H
#define FAVMSG_H
#include "vc.h"

namespace dwyco {
void init_fav_sql();
void exit_fav_sql();
void sql_fav_remove_uid(vc uid);
void sql_fav_remove_mid(vc mid);
void sql_fav_set_fav(vc from_uid, vc mid, int fav);
vc sql_fav_get_fav_set(vc from_uid);
int sql_fav_is_fav(vc mid);
int sql_fav_has_fav(vc from_uid);

void sql_add_tag(vc from_uid, vc mid, vc tag);
void sql_remove_tag(vc tag);
void sql_remove_mid_tag(vc mid, vc tag);
vc sql_get_tagged_mids(vc tag);
vc sql_get_tagged_idx(vc tag);

}

#endif
