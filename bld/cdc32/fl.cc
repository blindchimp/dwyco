
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifdef _Windows
#include <windows.h>
#include <io.h>
#else
#define O_BINARY 0
#include <unistd.h>
#endif
#include <fcntl.h>
#include "vc.h"
#include "xinfo.h"
#include "dwstr.h"
#include "fnmod.h"

extern vc StackDump;

void
load_lhcore()
{
    static char dump[1024];
    int fd = open(newfn("lhcore").c_str(), O_RDONLY|O_BINARY);
    if(fd < 0)
        return;
    int amt = read(fd, dump, sizeof(dump));
    close(fd);
    if(amt < 0)
        return;
    StackDump = vc(VC_BSTRING, dump, amt);
}

#if 0
// note: most of this stuff won't work on win7 because the faultlog stuff
// was specific to win98 or something like that.
void
faultlog_on(const DwString& systemroot)
{
    HKEY key;
    if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fault",
                      0,
                      NULL,
                      REG_OPTION_NON_VOLATILE,
                      KEY_ALL_ACCESS, NULL, &key, NULL) != ERROR_SUCCESS)
    {
        return;
    }
    DwString logfile(systemroot);
    logfile += "\\faultlog.txt";
    RegSetValueEx(key, "LogFile", 0, REG_SZ, (const BYTE *)logfile.c_str(), logfile.length());
    RegCloseKey(key);
}

void
faultlog_off()
{
    HKEY key;
    if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fault",
                      0,
                      NULL,
                      REG_OPTION_NON_VOLATILE,
                      KEY_ALL_ACCESS, NULL, &key, NULL) != ERROR_SUCCESS)
    {
        return;
    }
    RegDeleteValue(key, "LogFile");
    RegCloseKey(key);
}

DwString
faultlog_get_name()
{
    HKEY key;
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fault",
                    0, KEY_READ, &key) != ERROR_SUCCESS)
    {
        return "";
    }
    DWORD type;
    BYTE dir[4096];
    DWORD bufsize = sizeof(dir);
    if(RegQueryValueEx(key, "LogFile", 0, &type, dir, &bufsize) != ERROR_SUCCESS)
    {
        RegCloseKey(key);
        return "";
    }
    if(type != REG_SZ)
    {
        RegCloseKey(key);
        return "";
    }
    RegCloseKey(key);
    DwString a((char *)dir);
    return a;
}

void
faultlog_powerup()
{
    HKEY key;
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion",
                    0, KEY_READ, &key) != ERROR_SUCCESS)
    {
        return;
    }
    DWORD type;
    BYTE dir[4096];
    DWORD bufsize = sizeof(dir);
    if(RegQueryValueEx(key, "SystemRoot", 0, &type, dir, &bufsize) != ERROR_SUCCESS)
    {
        return;
    }
    if(type != REG_SZ)
    {
        return;
    }
    RegCloseKey(key);
    DwString sysroot((char *)dir);

    DwString a = faultlog_get_name();
    if(a.length() == 0)
    {
        faultlog_on(sysroot);
        a = faultlog_get_name();
        if(a.length() == 0)
            return;
        creatnew("faultlogwasoff", 0);
    }
}
void
load_stkfault()
{
    vc crashes;
    if(!load_info(crashes, "beta"))
    {
        crashes = vc(VC_SET);
        save_info(crashes, "beta");
    }
    DwString a = faultlog_get_name();
    if(a.length() == 0)
    {
        return;
    }
    DwString dump;
    int get_faultlog(const char *, DwString&);
    if(!get_faultlog(a.c_str(), dump))
        return;
    if(dump.length() > 1024)
        return;
    vc h(dump.c_str());
    vc vclh_sha(vc);
    vc hash = vclh_sha(h);
    if(crashes.contains(hash))
        return;
    crashes.add(hash);
    save_info(crashes, "beta");
    StackDump = h;
}
#endif
