#ifdef DWYCO_ASSHAT
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef ASSHOLE_H
#define ASSHOLE_H

#include <time.h>
#include "dwstr.h"

void update_asshole_factor(vc ah);
void load_asshole_factor();
vc get_asshole_factor();
vc get_asshole_thresh();
void set_asshole_thresh(vc);
void init_assholes();
int is_asshole(vc uid);
void new_asshole(vc uid, vc ah);
void new_asshole2(vc uid, vc ah, vc tm);
vc get_asshole(vc uid, time_t& tm);
vc get_decayed_asshole(vc uid);
void set_asshole_param(vc);

DwString display_asshole(vc uid);

extern int Asshole_filtering;

#define ASSHOLE_FILTERING_NONE 0
#define ASSHOLE_FILTERING_LIGHT 1
#define ASSHOLE_FILTERING_MEDIUM 2
#define ASSHOLE_FILTERING_STRONG 3
#endif
#endif
