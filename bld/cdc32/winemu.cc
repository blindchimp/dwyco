
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "windows.h"
#include <sys/time.h>
#include <sys/types.h>
#ifdef ANDROID
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <strings.h>

[[noreturn]] void oopanic(const char *);

// WARNING: don't use these time things to initialize
// entropy sources... on non-windows system, they tend to
// return the same values at each app startup.
unsigned long
GetTickCount()
{
    // note: the result doesn't fit in a 32bit long, so we take a snapshot
    // of the time and call that 0
    // note; this isn't reentrant. really need an "init" thing
    static int been_here;
    static double tv0;
    struct timeval tv;
    if(!been_here)
    {
        gettimeofday(&tv, 0);
        tv0 = ((double) tv.tv_sec + (double)tv.tv_usec / 1000000.) * 1000.;
        been_here = 1;
    }
    gettimeofday(&tv, 0);
    double d = ((double) tv.tv_sec + (double)tv.tv_usec / 1000000.) * 1000.;
    return (unsigned long)(d - tv0);
}

unsigned long
timeGetTime()
{
    return GetTickCount();
}

void InitializeCriticalSection(CRITICAL_SECTION *cs)
{
    cs->LockCount = 0;
    cs->RecursionCount = 0;
    cs->is_owned = 0;
    cs->OwningThread = (pthread_t)0xffffffff;
    pthread_mutexattr_init(&cs->attr);
    pthread_mutexattr_settype(&cs->attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&cs->LockSemaphore, &cs->attr);
}

void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
#if 0
    pthread_t me = pthread_self();
    //if (lpCriticalSection->OwningThread == me) {
    if (lpCriticalSection->is_owned &&
            pthread_equal(lpCriticalSection->OwningThread, me)) {
        // I have the lock.
        // This cannot be a race condition.
        lpCriticalSection->RecursionCount++;
        return;
    }
#endif
    if(pthread_mutex_lock(&lpCriticalSection->LockSemaphore)) {
        oopanic("mutex_lock() failed");
        // NOTREACHED
        return;
    }
    // got it. I must be the first thread:
#if 0
    if (lpCriticalSection->RecursionCount) {
        oopanic("RecursionCount != 0");
        return;
    }
    lpCriticalSection->RecursionCount = 1;
    lpCriticalSection->is_owned = 1;
    lpCriticalSection->OwningThread = me;
#endif
    return;
}

void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
#if 0
    pthread_t me = pthread_self();
    //if (lpCriticalSection->OwningThread == me) {
    if (lpCriticalSection->is_owned &&
            pthread_equal(lpCriticalSection->OwningThread, me)) {
        lpCriticalSection->RecursionCount--;
        if(lpCriticalSection->RecursionCount < 1) {
            lpCriticalSection->is_owned = 0;
            lpCriticalSection->OwningThread = (pthread_t)0xFFFFFFFF;
#endif
            if(pthread_mutex_unlock(&lpCriticalSection->LockSemaphore))
                oopanic("mutex_unlock() failed");
#if 0
        }
    } else
        oopanic("not lock owner");
    return;
#endif
}

void
DeleteCriticalSection(CRITICAL_SECTION *cs)
{
// note: have to check the ms docs to see if deleting a critical
// section that has not been "left" is like doing a leave first or what.
// note: it just says all the thread states become "undefined".
#if 0
    if(cs->is_owned)
        oopanic("delete of owned critical section");
#endif
    if(pthread_mutex_destroy(&cs->LockSemaphore))
        oopanic("mutex_destroy() failed");
    if(pthread_mutexattr_destroy(&cs->attr))
        oopanic("mutex_attr_destroy() failed");
}

int
CopyFile(const char *s, const char *d, int fail_if_exists)
{
    if(fail_if_exists && access(d, F_OK) == 0)
        return 0;
    FILE *fs = fopen(s, "rb");
    if(!fs)
        return 0;
    FILE *fd = fopen(d, "wb");
    if(!fd)
    {
        fclose(fs);
        return 0;
    }
    int ret = 1;
    while(!feof(fs))
    {
        char buf[8192];
        int n = fread(buf, 1, sizeof(buf), fs);
        if(fwrite(buf, 1, n, fd) != n)
        {
            unlink(d);
            ret = 0;
            goto out;
        }
    }
out:
    fclose(fs);
    fclose(fd);
    return ret;
}

// this event stuff only partly mimics the windows
// stuff. just enough for this app.
// use local pipes for event setting, etc.
#ifdef MACOSX
#include <libkern/OSAtomic.h>
typedef OSSpinLock pthread_spinlock_t;
static
void
pthread_spin_init(pthread_spinlock_t *a, int dum)
{
    *a = 0;
}
static
void
pthread_spin_destroy(pthread_spinlock_t *)
{
}

static
void
pthread_spin_lock(pthread_spinlock_t *a)
{
    OSSpinLockLock(a);
}
static
void
pthread_spin_unlock(pthread_spinlock_t *a)
{
    OSSpinLockUnlock(a);
}

#endif

// note: ndk api 24 includes these now
#if 1
#ifdef ANDROID
typedef volatile int pthread_spinlock_t;
static
void
pthread_spin_init(pthread_spinlock_t *a, int dum)
{
    *a = 0;
}
static
void
pthread_spin_destroy(pthread_spinlock_t *)
{
}

static
void
pthread_spin_lock(pthread_spinlock_t *a)
{
    while(__sync_lock_test_and_set(a, 1))
        ;
}
static
void
pthread_spin_unlock(pthread_spinlock_t *a)
{
    __sync_synchronize();
    *a = 0;
}
#endif

#endif

struct event_fds
{
    int fds[2];
    pthread_spinlock_t lock;
};

HANDLE
CreateEvent(int, int, int, int)
{
    event_fds *e = new event_fds;
    pipe(e->fds);
    fcntl(e->fds[0], F_SETFL, O_NDELAY);
    fcntl(e->fds[1], F_SETFL, O_NDELAY);
    pthread_spin_init(&e->lock, PTHREAD_PROCESS_PRIVATE);
    return (HANDLE)e;
}

void
SetEvent(HANDLE ev)
{
    // note: we spinlock here because we can't go to sleep, as this
    // may get called in god-knows-what kinda context.
    event_fds *a = (event_fds *)ev;
    pthread_spin_lock(&a->lock);
    // check to see if it is already signaled
    fd_set fs;
    FD_ZERO(&fs);
    FD_SET(a->fds[0], &fs);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    int i = select(a->fds[0] + 1, &fs, 0, 0, &tv);
    if(i == -1)
        oopanic("bogus select");
    if(i == 0)
    {
        if(write(a->fds[1], "e", 1) != 1)
            oopanic("event pipe is clogged");
    }
    pthread_spin_unlock(&a->lock);
}

// bad, assumes it is an event
void
CloseHandle(HANDLE h)
{
    event_fds *hh = (event_fds *)h;
    pthread_spin_lock(&hh->lock);
    close(hh->fds[0]);
    close(hh->fds[1]);
    pthread_spin_destroy(&hh->lock);
    delete hh;
}


int
WaitForMultipleObjects(int num, HANDLE* objs, int wait_all, int ms_to_wait)
{
    fd_set fs;
    FD_ZERO(&fs);
    int max = -1;
    // note: some of the objs may be shared, so we have to
    // make sure we get locks on all of them
    // there is a simplification going on here: since we know
    // there is only one thread that calls this function, we
    // don't try to make the lock acq atomic
    event_fds *a = 0;
    for(int i = 0; i < num; ++i)
    {
        a = (event_fds *)objs[i];
        FD_SET(a->fds[0], &fs);
        if(a->fds[0] > max)
            max = a->fds[0];
    }
    struct timeval tv;
    tv.tv_sec = ms_to_wait / 1000;
    tv.tv_usec = (ms_to_wait % 1000) * 1000;
    ++max;
    int ret = select(max, &fs, 0, 0, &tv);
    if(ret == 0)
        return WAIT_TIMEOUT;
    else if(ret != -1)
    {
        // this is goofy, but ok for this app. lock out the
        // writer (setevent) while we clear out the pipe
        for(int i = 0; i < num; ++i)
        {
            a = (event_fds *)objs[i];
            if(FD_ISSET(a->fds[0], &fs))
            {
                pthread_spin_lock(&a->lock);
                char dum[128];
                if(read(a->fds[0], dum, sizeof(dum)) != 1)
                    oopanic("event pipe hosed");
                pthread_spin_unlock(&a->lock);
            }
        }
        return WAIT_OBJECT_0;
    }
    return WAIT_ERROR;
}

int
filelength(int fd)
{
    off_t old = lseek(fd, 0, SEEK_CUR);
    off_t sz = lseek(fd, 0, SEEK_END);
    lseek(fd, old, SEEK_SET);
    return sz;
}

int
mkdir(const char *p)
{
    return mkdir(p, 0700);
}

DWORD
GetCurrentThreadId()
{
    return (DWORD)pthread_self();
}
