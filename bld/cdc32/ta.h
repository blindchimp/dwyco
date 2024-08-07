
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef TA_H
#define TA_H
#ifdef DWYCO_FIELD_DEBUG
#include "dwrtlog.h"
#define TRACK_ADD(var, num) do {\
    GRTLOG(#var " %d", num, 0); \
if(dwyco::Track_stats) \
{ \
        vc __n; \
        if(dwyco::Stats.find(#var, __n)) \
	{ \
                int __i = static_cast<int>(__n); \
                __i += (num); \
                dwyco::Stats.add_kv(#var, __i); \
	} \
	else \
	{ \
        dwyco::Stats.add_kv(#var, static_cast<int>(num)); \
	} \
        dwyco::save_info(dwyco::Stats, "stats"); \
} \
} while(0)

#define TRACK_ADD_str(var, num) do {\
    GRTLOG(#var " %d", num, 0); \
if(dwyco::Track_stats) \
{ \
        vc __n; \
        if(dwyco::Stats.find(var, __n)) \
	{ \
                int __i = static_cast<int>(__n); \
                __i += (num); \
                dwyco::Stats.add_kv(var, __i); \
	} \
	else \
	{ \
        dwyco::Stats.add_kv(var, static_cast<int>(num)); \
	} \
        dwyco::save_info(dwyco::Stats, "stats"); \
} \
} while(0)

#define TRACK_ADD_nosave(var, num) do {\
    GRTLOG(#var " %d", num, 0); \
if(dwyco::Track_stats) \
{ \
        vc __n; \
        if(dwyco::Stats.find(#var, __n)) \
	{ \
                int __i = int(__n); \
                __i += (num); \
                dwyco::Stats.add_kv(#var, __i); \
	} \
	else \
	{ \
        dwyco::Stats.add_kv(#var, int(num)); \
	} \
} \
} while(0)

#define TRACK_MAX(var, num) do {\
    GRTLOG(#var " max %d", num, 0); \
if(dwyco::Track_stats) \
{ \
        vc __n; \
        if(dwyco::Stats.find(#var "_max", __n)) \
        { \
                int __i = int(__n); \
                if((num) > __i) \
                    dwyco::Stats.add_kv(#var "_max", int(num)); \
        } \
        else \
        { \
                dwyco::Stats.add_kv(#var "_max", int(num)); \
        } \
        dwyco::save_info(dwyco::Stats, "stats"); \
} \
} while(0)

#define TRACK_ADD_VC(avar, num) do {\
    GRTLOGVC(avar); \
if(dwyco::Track_stats) \
{ \
        vc __n; \
    vc __var = (avar); \
        if(dwyco::Stats.find(__var, __n)) \
	{ \
                int __i = int(__n); \
                __i += (num); \
                dwyco::Stats.add_kv(__var, __i); \
	} \
	else \
	{ \
                dwyco::Stats.add_kv(__var, int(num)); \
	} \
    dwyco::save_info(dwyco::Stats, "stats"); \
} \
} while(0)

#define TRACK_ADD_VC_nosave(avar, num) do {\
    GRTLOGVC(avar); \
if(dwyco::Track_stats) \
{ \
        vc __n; \
    vc __var = (avar); \
        if(dwyco::Stats.find(__var, __n)) \
	{ \
                int __i = int(__n); \
                __i += (num); \
                dwyco::Stats.add_kv(__var, __i); \
	} \
	else \
	{ \
                dwyco::Stats.add_kv(__var, int(num)); \
	} \
} \
} while(0)
#else
#define TRACK_ADD(var, num) do {} while(0)
#define TRACK_ADD_str(var, num) do {} while(0)
#define TRACK_ADD_nosave(var, num) do {} while(0)
#define TRACK_ADD_VC(var, num) do {} while(0)
#define TRACK_ADD_VC_nosave(var, num) do {} while(0)
#define TRACK_MAX(var, num) do {} while(0)
#endif

namespace dwyco {
extern vc Stats;
extern int Track_stats;
void init_stats();
}
#include "xinfo.h"


#endif
