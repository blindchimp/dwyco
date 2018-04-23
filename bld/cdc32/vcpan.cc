
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/vcpan.cc 1.12 1999/01/10 16:09:41 dwight Checkpoint $
 */

#include <fcntl.h>
#ifdef _Windows
#include <io.h>
#else
#include <unistd.h>
#include <sys/types.h>
#define O_BINARY 0
#endif
#include <sys/stat.h>

#ifdef _Windows
#include "vfwmgr.h"
#endif
#include "aq.h"
#include "vc.h"
#include "dwrtlog.h"
#include "dlli.h"
#include "fnmod.h"


void stackdump(void *v, int len);
char stack[1024];
extern DwycoEmergencyCallback dwyco_emergency_callback;


void
oopanic(const char *s)
{
    FILE *f = fopen(newfn("lhcore").c_str(), "wb");
    if(f)
    {
        fputs(s, f);
        stackdump((void *)stack, sizeof(stack));
        fwrite(stack, 1, sizeof(stack), f);
        fclose(f);
    }
#ifndef DWYCO_NO_AUTOBUG
    {
        int fd;
        fd = open(newfn("crashed").c_str(), O_WRONLY|O_BINARY);
        if(fd != -1)
        {
            char ct = 2;
            write(fd, &ct, sizeof(ct));
            close(fd);
        }
    }
#endif

    if(dwyco_emergency_callback)
    {
        (*dwyco_emergency_callback)(DWYCO_EMERGENCY_GENERAL_PANIC, 1, s);
    }
    exitaq();
#if defined(_Windows) && defined(USE_VFW)
    delete TheVFWMgr;
#endif
#ifdef DW_RTLOG
    delete RTLog;
#endif
    vc::shutdown_logs();
    exit(1);
}

#if 0
void
oopanic(const char *s)
{
    char a[10000];
    strcpy(a, s);
    oopanic(a);
}
#endif

extern "C" {
    void coopanic(char *s);
};

// this is called from the allocator at error time, so
// be careful not to do anything to reenter it.
void coopanic(char *s)
{
#ifdef _Windows
    int fd = open("lhcore", O_CREAT|O_BINARY|O_TRUNC|O_WRONLY, _S_IREAD|_S_IWRITE);
#else
    int fd = open("lhcore", O_CREAT|O_BINARY|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
#endif
    write(fd, s, strlen(s));
    stackdump((void *)stack, sizeof(stack));
    write(fd, stack, sizeof(stack));
    close(fd);
#ifndef DWYCO_NO_AUTOBUG
    {
        int fd;
        fd = open("crashed", O_WRONLY);
        char ct = 2;
        write(fd, &ct, sizeof(ct));
        close(fd);
    }
#endif
    _exit(1);
}

