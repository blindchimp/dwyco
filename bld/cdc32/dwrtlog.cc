
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/dwrtlog.cc 1.16 1999/01/10 16:09:42 dwight Checkpoint $
#ifdef DW_RTLOG
#define _GNU_SOURCE
#include <windows.h>
#include "dwrtlog.h"
#include "dwstr.h"
#include <stdarg.h>
#include "sepstr.h"
#include "fnmod.h"
#ifdef __linux__
#include <execinfo.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#endif

#if defined(ANDROID) && defined(DW_ANDROID_LOG)
#include <android/log.h>
#define DWRTLOG_TAG "dwyco-rtlog"
#endif

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
    //RTLogOn.add("*");
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

vc
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
    flush_timer.start(DwTimer::REPEATING, time * 1000, time * 1000);

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
#ifndef ANDROID
    DWORD tid = GetCurrentThreadId();
    snprintf(tmp, sizeof(tmp) - 1, "%08lx %8.3f %s:%d ", tid, (float)time/1000, a.c_str(), line);
    tmp[sizeof(tmp) - 1] = 0;
    (*os) << tmp;
#endif
    snprintf(tmp, sizeof(tmp) - 1, fmt, a1, a2, a3, a4, a5, a6);
    tmp[sizeof(tmp) - 1] = 0;
    (*os) << tmp << "\n";
#if defined(ANDROID) && defined(DW_ANDROID_LOG)
    (*os) << '\0';
    __android_log_write(ANDROID_LOG_DEBUG, DWRTLOG_TAG, os->ref_str());
    os->reset();
#endif
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
#ifndef ANDROID
    DWORD tid = GetCurrentThreadId();
    snprintf(tmp, sizeof(tmp) - 1, "%08lx %8.3f %s:%d ", tid, (double)time/1000, a.c_str(), line);
    tmp[sizeof(tmp) - 1] = 0;
    (*os) << tmp;
#endif
    snprintf(tmp, sizeof(tmp) - 1, fmt, a1, a2, a3, a4, a5);
    tmp[sizeof(tmp) - 1] = 0;
    (*os) << tmp << "\n";
#if defined(ANDROID) && defined(DW_ANDROID_LOG)
    (*os) << '\0';
    __android_log_write(ANDROID_LOG_DEBUG, DWRTLOG_TAG, os->ref_str());
    os->reset();
#endif
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
#ifndef ANDROID
    DWORD tid = GetCurrentThreadId();
    snprintf(tmp, sizeof(tmp) - 1, "%08lx %8.3f %s:%d ", tid, (double)time/1000, a.c_str(), line);
    tmp[sizeof(tmp) - 1] = 0;
    (*os) << tmp;
#endif
    vsnprintf(tmp, sizeof(tmp) - 1, fmt, ap);
    tmp[sizeof(tmp) - 1] = 0;
    (*os) << tmp << "\n";
#if defined(ANDROID) && defined(DW_ANDROID_LOG)
    (*os) << '\0';
    __android_log_write(ANDROID_LOG_DEBUG, DWRTLOG_TAG, os->ref_str());
    os->reset();
#endif
    LeaveCriticalSection(&cs);
    va_end(ap);
}

#ifdef __linux__
static bool
rtlog_is_pie_executable()
{
    FILE *f = fopen("/proc/self/exe", "rb");
    if(!f)
        return false;
    unsigned char e[18];
    size_t n = fread(e, 1, sizeof(e), f);
    fclose(f);
    if(n < 18 || e[0] != 0x7f || e[1] != 'E' || e[2] != 'L' || e[3] != 'F')
        return false;
    int et = e[16] | (e[17] << 8);
    return et == 3;
}

static bool
rtlog_exe_maps(char *exe, size_t exesize, unsigned long *base,
               unsigned long *lo, unsigned long *hi)
{
    ssize_t n = readlink("/proc/self/exe", exe, exesize - 1);
    if(n <= 0)
        return false;
    exe[n] = 0;
    FILE *f = fopen("/proc/self/maps", "r");
    if(!f)
        return false;
    bool any = false;
    bool have_base = false;
    *base = 0;
    *lo = (unsigned long)-1;
    *hi = 0;
    char line[2048];
    while(fgets(line, sizeof(line), f))
    {
        unsigned long s, e;
        char perms[16], off[32], dev[32], ino[64], path[PATH_MAX];
        path[0] = 0;
        int r = sscanf(line, "%lx-%lx %15s %31s %31s %63s %1023s",
                       &s, &e, perms, off, dev, ino, path);
        if(r < 6)
            continue;
        if(r == 7)
        {
            if(strncmp(path, " (deleted)", 10) == 0)
                path[0] = 0;
        }
        char pbuf[PATH_MAX + 32];
        pbuf[0] = 0;
        if(r == 7)
        {
            strcpy(pbuf, path);
            char *del = strstr(pbuf, " (deleted)");
            if(del)
                *del = 0;
        }
        if(strcmp(pbuf, exe) != 0)
            continue;
        if(!any || s < *lo)
            *lo = s;
        if(e > *hi)
            *hi = e;
        any = true;
        if(strcmp(off, "00000000") == 0 && (!have_base || s < *base))
        {
            *base = s;
            have_base = true;
        }
    }
    fclose(f);
    if(!have_base && any)
        *base = *lo;
    return any;
}

#define RTLOG_MAX_BACKTRACE 127
void
DwRTLog::backtrace(const char *file, int line, int skip, int count)
{
    if(skip < 0)
        skip = 0;
    if(count < 0)
        count = 0;
    int max_frames = RTLOG_MAX_BACKTRACE + 1;
    if(skip > max_frames - 1)
        skip = max_frames - 1;
    if(count > max_frames - 1 - skip)
        count = max_frames - 1 - skip;
    void *frames[RTLOG_MAX_BACKTRACE + 1];
    int num_frames = ::backtrace(frames, 1 + skip + count);
    if(num_frames <= 1)
        return;

    bool pie = rtlog_is_pie_executable();
    char exe[PATH_MAX];
    unsigned long exe_base = 0, exe_lo = 0, exe_hi = 0;
    bool have_exe = rtlog_exe_maps(exe, sizeof(exe), &exe_base, &exe_lo, &exe_hi);

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
    int first = 1 + skip;
    int last = first + count;
    if(last > num_frames)
        last = num_frames;
    (*os) << "backtrace:";
    for(int j = first; j < last; j++)
    {
        char addr[32];
        unsigned long v = (unsigned long)frames[j];
        if(pie && have_exe && v >= exe_lo && v < exe_hi)
            v -= exe_base;
        snprintf(addr, sizeof(addr), " 0x%lx", v);
        (*os) << addr;
    }
    (*os) << "\n";
    LeaveCriticalSection(&cs);
}
#endif

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
#ifndef ANDROID
    DWORD tid = GetCurrentThreadId();
    snprintf(tmp, sizeof(tmp) - 1, "%08lx %8.3f %s:%d ", tid, (double)time/1000, a.c_str(), line);
    tmp[sizeof(tmp) - 1] = 0;
    (*os) << tmp;
#endif
    v.print_top(*os);
    (*os) << "\n";
#if defined(ANDROID) && defined(DW_ANDROID_LOG)
    (*os) << '\0';
    __android_log_write(ANDROID_LOG_DEBUG, DWRTLOG_TAG, os->ref_str());
    os->reset();
#endif
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
