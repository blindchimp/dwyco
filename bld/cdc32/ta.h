
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
if(Track_stats) \
{ \
        vc __n; \
        if(Stats.find(#var, __n)) \
	{ \
                int __i = static_cast<int>(__n); \
                __i += (num); \
                Stats.add_kv(#var, __i); \
	} \
	else \
	{ \
        Stats.add_kv(#var, static_cast<int>(num)); \
	} \
	save_info(Stats, "stats"); \
} \
} while(0)

#define TRACK_ADD_str(var, num) do {\
    GRTLOG(#var " %d", num, 0); \
if(Track_stats) \
{ \
        vc __n; \
        if(Stats.find(var, __n)) \
	{ \
                int __i = static_cast<int>(__n); \
                __i += (num); \
                Stats.add_kv(var, __i); \
	} \
	else \
	{ \
        Stats.add_kv(var, static_cast<int>(num)); \
	} \
	save_info(Stats, "stats"); \
} \
} while(0)

#define TRACK_ADD_nosave(var, num) do {\
    GRTLOG(#var " %d", num, 0); \
if(Track_stats) \
{ \
        vc __n; \
        if(Stats.find(#var, __n)) \
	{ \
                int __i = int(__n); \
                __i += (num); \
                Stats.add_kv(#var, __i); \
	} \
	else \
	{ \
        Stats.add_kv(#var, int(num)); \
	} \
} \
} while(0)

#define TRACK_MAX(var, num) do {\
    GRTLOG(#var " max %d", num, 0); \
if(Track_stats) \
{ \
        vc __n; \
        if(Stats.find(#var "_max", __n)) \
        { \
                int __i = int(__n); \
                if((num) > __i) \
                    Stats.add_kv(#var "_max", int(num)); \
        } \
        else \
        { \
                Stats.add_kv(#var "_max", int(num)); \
        } \
        save_info(Stats, "stats"); \
} \
} while(0)

#define TRACK_ADD_VC(avar, num) do {\
    GRTLOGVC(avar); \
if(Track_stats) \
{ \
        vc __n; \
    vc __var = (avar); \
        if(Stats.find(__var, __n)) \
	{ \
                int __i = int(__n); \
                __i += (num); \
                Stats.add_kv(__var, __i); \
	} \
	else \
	{ \
                Stats.add_kv(__var, int(num)); \
	} \
    save_info(Stats, "stats"); \
} \
} while(0)

#define TRACK_ADD_VC_nosave(avar, num) do {\
    GRTLOGVC(avar); \
if(Track_stats) \
{ \
        vc __n; \
    vc __var = (avar); \
        if(Stats.find(__var, __n)) \
	{ \
                int __i = int(__n); \
                __i += (num); \
                Stats.add_kv(__var, __i); \
	} \
	else \
	{ \
                Stats.add_kv(__var, int(num)); \
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

extern vc Stats;
extern int Track_stats;
#include "xinfo.h"


#endif
