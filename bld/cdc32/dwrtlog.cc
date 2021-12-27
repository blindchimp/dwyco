
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/dwrtlog.cc 1.16 1999/01/10 16:09:42 dwight Checkpoint $
#ifdef DW_RTLOG
#include <windows.h>
#include "dwrtlog.h"
#include "dwstr.h"
#include <stdarg.h>
#include "sepstr.h"
#include "fnmod.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

vc RTLogOn;
DwRTLog *RTLog;
#define TMPBUFSIZE 16384

void
init_rtlog()
{
    RTLogOn = vc(VC_VECTOR);
#ifdef ANDROID
    RTLogOn.add("dlli.cpp");
    RTLogOn.add("netcod.cc");
    RTLogOn.add("netvid.cc");
    RTLogOn.add("qmsg.cc");
#else
    // note: we do not use "newfn" here to map the file
    // location, because that itself may cause some logging.
    // the way the logging is initialized, if RTLogOn
    // is empty, it will never probe again, which ends up
    // disabling all the log statements that were attempted too early.
    FILE *f = fopen("rtlog.dbg", "rt");
    if(!f)
        return;
    char buf[500];
    while(fgets(buf, sizeof(buf), f))
    {
        int l = strlen(buf);
        if(l == 0)
            break;
        if(buf[0] == '#')
            continue;
        if(buf[l - 1] == '\n')
            buf[l - 1] = 0;
        DwString f(buf);
        f.to_lower();
        if(!RTLogOn.contains(f.c_str()))
            RTLogOn.add(f.c_str());
    }
    fclose(f);
#endif
}

DwString
logbasename(const char *name)
{
    DwString f(name);
    int b = f.rfind(DIRSEPSTR);
    if(b != DwString::npos)
        f.remove(0, b + 1);
    f.to_lower();
    return f;
}

int DwRTLog_on = 1;

DwRTLog::DwRTLog(const char *filename, int size, int time) :
    flush_timer("log-flush")
{
    bsize = size;

    flush_time = time;
    flush_timer.set_interval(time * 1000);
    flush_timer.set_autoreload(1);
    //flush_timer.set_oneshot(0);
    flush_timer.reset();
    flush_timer.start();

    os = new VcIOHackStr(size);
    // see comment above regarding initialization of logging.
    // don't use "newfn" here. this just means that the logging
    // stuff is controlled from the working directory instead of
    // elsewhere. may cause a problem, but it is for debugging...
    outfile = fopen(filename, "a");
    if(!outfile)
        outfile = stdout;
    InitializeCriticalSection(&cs);
}

DwRTLog::~DwRTLog()
{
    EnterCriticalSection(&cs);
    flush_to_file();
    fclose(outfile);
    delete os;
    LeaveCriticalSection(&cs);
    DeleteCriticalSection(&cs);
}

void
DwRTLog::tick()
{
//#ifndef NO_RTLOG
    EnterCriticalSection(&cs);
    if(flush_timer.is_expired())
    {
        flush_timer.ack_expire();
        flush_to_file();
    }
    LeaveCriticalSection(&cs);
//#endif
}

void
DwRTLog::log(const char *fmt, const char *file, int line,
             double a1, double a2, double a3, double a4, double a5, double a6)
{
    unsigned long time = flush_timer.time_now();
    char tmp[TMPBUFSIZE];

    EnterCriticalSection(&cs);
    if(os->pcount() >= bsize - 1000)
        flush_to_file();
    DwString a(file);
    int i = a.rfind("\\");
    if(i == -1)
        i = a.rfind("/");
    if(i != -1)
        a.remove(0, i + 1);
    DWORD tid = GetCurrentThreadId();
    snprintf(tmp, sizeof(tmp) - 1, "%08lx %8.3f %s:%d ", tid, (float)time/1000, a.c_str(), line);
    tmp[sizeof(tmp) - 1] = 0;
    (*os) << tmp;
    snprintf(tmp, sizeof(tmp) - 1, fmt, a1, a2, a3, a4, a5, a6);
    tmp[sizeof(tmp) - 1] = 0;
    (*os) << tmp << "\r\n";
    LeaveCriticalSection(&cs);
}
void
DwRTLog::log(const char *fmt, const char *file, int line,
             int a1, int a2, int a3, int a4, int a5)
{
    unsigned long time = flush_timer.time_now();
    char tmp[TMPBUFSIZE];

    EnterCriticalSection(&cs);
    if(os->pcount() >= bsize - 1000)
        flush_to_file();
    DwString a(file);
    int i = a.rfind("\\");
    if(i == -1)
        i = a.rfind("/");
    if(i != -1)
        a.remove(0, i + 1);
    DWORD tid = GetCurrentThreadId();
    snprintf(tmp, sizeof(tmp) - 1, "%08lx %8.3f %s:%d ", tid, (double)time/1000, a.c_str(), line);
    tmp[sizeof(tmp) - 1] = 0;
    (*os) << tmp;
    snprintf(tmp, sizeof(tmp) - 1, fmt, a1, a2, a3, a4, a5);
    tmp[sizeof(tmp) - 1] = 0;
    (*os) << tmp << "\n";
    LeaveCriticalSection(&cs);
}

void
DwRTLog::vlog(const char *fmt, const char *file, int line, ...)

{
    va_list ap;
    va_start(ap, line);

    unsigned long time = flush_timer.time_now();
    char tmp[TMPBUFSIZE];

    EnterCriticalSection(&cs);
    if(os->pcount() >= bsize - 1000)
        flush_to_file();
    DwString a(file);
    int i = a.rfind("\\");
    if(i == -1)
        i = a.rfind("/");
    if(i != -1)
        a.remove(0, i + 1);
    DWORD tid = GetCurrentThreadId();
    snprintf(tmp, sizeof(tmp) - 1, "%08lx %8.3f %s:%d ", tid, (double)time/1000, a.c_str(), line);
    tmp[sizeof(tmp) - 1] = 0;
    (*os) << tmp;
    vsnprintf(tmp, sizeof(tmp) - 1, fmt, ap);
    tmp[sizeof(tmp) - 1] = 0;
    (*os) << tmp << "\n";
    LeaveCriticalSection(&cs);
    va_end(ap);
}

void
DwRTLog::log(const char *file, int line, vc v)
{
    unsigned long time = flush_timer.time_now();
    char tmp[TMPBUFSIZE];

    EnterCriticalSection(&cs);
    if(os->pcount() >= bsize - 1000)
        flush_to_file();

    DwString a(file);
    int i = a.rfind("\\");
    if(i == -1)
        i = a.rfind("/");
    if(i != -1)
        a.remove(0, i + 1);
    DWORD tid = GetCurrentThreadId();
    snprintf(tmp, sizeof(tmp) - 1, "%08lx %8.3f %s:%d ", tid, (double)time/1000, a.c_str(), line);
    tmp[sizeof(tmp) - 1] = 0;
    (*os) << tmp;
    v.print_top(*os);
    (*os) << "\n";
    LeaveCriticalSection(&cs);
}

void
DwRTLog::flush_to_file()
{
    EnterCriticalSection(&cs);
    if(os->pcount() == 0)
    {
        LeaveCriticalSection(&cs);
        return;
    }
    //os->flush();
    fwrite(os->ref_str(), 1, os->pcount(), outfile);
    fflush(outfile);
    os->reset();
    LeaveCriticalSection(&cs);
}

void
dwrtlog_vc(const char *file, int line, vc v)
{
    if(RTLog)
        RTLog->log(file, line, v);
}

void
dwrtlog(const char *fmt,  const char *file, int line,
        int a1, int a2, int a3, int a4, int a5)
{
    if(RTLog)
        RTLog->log(fmt, file, line, a1, a2, a3, a4, a5);
}
#endif

#undef TEST

#ifdef TEST
main(int argc, char **argv)
{
    DwRTLog l;

    l.log("test %d", 1,2,3,4);
    unsigned long t = timeGetTime();
    while(timeGetTime() - t < 10 * 1000)
    {
        l.tick();
        if((timeGetTime() - t) % 100 == 0)
            l.log("mark");
    }
    RTLOG(l, "done", "", "");
}

#endif
