
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/dwrtlog.h 1.16 1999/01/10 16:10:56 dwight Checkpoint $
#ifndef DWRTLOG_H
#define DWRTLOG_H

#ifdef DW_RTLOG
#include <windows.h>
#include <stdio.h>
#include "dwtimer.h"
#include "vc.h"
#include "vcio.h"
class VcIOHackStr;

#define RTLOG_DEFAULT_FILENAME "rtlog.out"
#define RTLOG_DEFAULT_SIZE (100 * 1024)
#define RTLOG_DEFAULT_TIME (30 * 60)

class DwRTLog
{
public:
    DwRTLog(const char *filename = RTLOG_DEFAULT_FILENAME,
            int size = RTLOG_DEFAULT_SIZE, int time = RTLOG_DEFAULT_TIME);
    ~DwRTLog();

    void log(const char *fmt, const char *file = "", int line = 0,
             int a1 = 0, int a2 = 0, int a3 = 0, int a4 = 0, int a5 = 0);
    void log(const char *fmt, const char *file = "", int line = 0,
             double a1 = 0, double a2 = 0, double a3 = 0, double a4 = 0, double a5 = 0, double a6 = 0);
    void vlog(const char *fmt, const char *file = "", int line = 0, ...);
    void log(const char *file = "", int line = 0, vc v = vcnil);
    void tick();
    void flush_to_file();

private:
    VcIOHackStr *os;
    FILE *outfile;
    int flush_time;
    DwTimer flush_timer;
    int bsize;
    CRITICAL_SECTION cs;
};


#include "dwstr.h"
DwString logbasename(const char *);
#define RTLOGSTART {static int init; static int logit; \
	if(!init) {\
		init = 1; \
		if(RTLogOn.contains("*") || RTLogOn.contains(logbasename(__FILE__).c_str())) {\
			logit = 1; \
		} \
	}\
	if(logit) {

#define RTLOGEND }}



#define RTLOG(l, fmt, v1, v2) do { RTLOGSTART (l).log((fmt), __FILE__, __LINE__, (v1), (v2)); RTLOGEND } while(0)
#define RTLOGA(l, fmt, v1, v2, v3, v4, v5) do { RTLOGSTART (l).log((fmt), __FILE__, __LINE__, (v1), (v2), (v3), (v4), (v5)); RTLOGEND } while(0)

#define GRTLOG(fmt, v1, v2) do { {if(RTLog) RTLOGSTART RTLog->vlog((fmt), __FILE__, __LINE__, (v1), (v2)); RTLOGEND } } while(0)
#define GRTLOGA(fmt, v1, v2, v3, v4, v5) do { {if(RTLog) RTLOGSTART RTLog->vlog((fmt), __FILE__, __LINE__, (v1), (v2), (v3), (v4), (v5)); RTLOGEND } } while(0)
#define GRTLOGF(fmt, v1, v2, v3, v4, v5, v6) do { {if(RTLog) RTLOGSTART RTLog->vlog((fmt), __FILE__, __LINE__, (v1), (v2), (v3), (v4), (v5), (v6)); RTLOGEND } } while(0)
#define GRTLOGVC(v) do { {if(RTLog) RTLOGSTART RTLog->log(__FILE__, __LINE__, (v)); RTLOGEND } } while(0)
extern DwRTLog *RTLog;
extern vc RTLogOn;
void init_rtlog();
#else

#define RTLOG(l, fmt, v1, v2) do {} while(0)
#define RTLOGA(l, fmt, v1, v2, v3, v4, v5) do {} while(0)

#define GRTLOG(fmt, v1, v2) do {} while(0)
#define GRTLOGA(fmt, v1, v2, v3, v4, v5) do {} while(0)
#define GRTLOGF(fmt, v1, v2, v3, v4, v5, v6) do {} while(0)
#define GRTLOGVC(v) do {} while(0)
#endif

#endif
