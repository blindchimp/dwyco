
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include "enc.h"
#include "dwtimer.h"
#include "vc.h"
#include "mmchan.h"
#include "netvid.h"
#include "vccrypt2.h"
#include "qauth.h"
#include "doinit.h"
#include "sha.h"
using namespace CryptoPP;
#include "dwrtlog.h"
#include "fnmod.h"
using namespace dwyco;

#ifdef DISKID
static vc
getdiskid()
{
    DwString getHardDriveComputerID();
    DwString id = getHardDriveComputerID();
    if(id.length() > 0)
    {
        byte a[20];
        SHA sha;
        sha.Update(id.c_str(), id.length());
        sha.Update("ouchyouch", 9);
        sha.Final(a);
        vc v(VC_BSTRING, (const char *)a, 10);
        GRTLOG("disk id worked", 0, 0);
        GRTLOGVC(to_hex(v));
        return v;
    }
    GRTLOG("disk id failed", 0, 0);
    return vcnil;
}
#endif

// new method (ca 2007)
// try the diskid method, if it works, use it.
// if HKLM\software\microsoft\cryptography\machineguid
// exists, SHA it and use that for the mid
// if it doesn't exist, attempt the current method (create mid
// in super-secret key if it doesn't exist.)
// if that doesn't work, fall back to creating a hidden file
// the local directory (this is the case where we can't write
// in system folders and so on, hopefully we'll be able to fix this
// at some point.)
//



static vc
set_get_dummy()
{
// software\microsoft\BIOSEnabler
    static char key3[] =
        "\xD4\xDF\xD6\xF7\xD5\xD2\xFC\xD3\xF2\xCE\xC4\xC5\xFA\xDC\xC5\xC0"
        "\xC2\xDD\xFB\xF2\xF9\xCC\xF1\xF6\xE0\xD7\xCC\xCF\xC8\xD4"
        ;

    // Source1
    static char val3[] =  "\xF4\xDF\xC5\xF1\xC1\xD6\xBF";

    DWORD type;
    BYTE dir[4096];
    DWORD bufsize = sizeof(dir);
    char tmpk[1024];
    char vald[1024];
    HKEY key;
    vc v;

    strcpy(tmpk, key3);
    Enc e4;
    Enc d4;
    e4.munge(tmpk, strlen(tmpk));
    Enc e5;
    Enc d5;
    strcpy(vald, val3);
    e5.munge(vald, strlen(vald));

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, tmpk,
                    0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS)
    {
        GRTLOG("creating new dummy key", 0, 0);
        // create it and make the reg key too
        DWORD disp;
        RegCreateKeyEx(HKEY_LOCAL_MACHINE, tmpk, 0,
                       (LPSTR)".dll", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0,
                       &key, &disp);

        v = gen_id();

        RegSetValueEx(key, vald, 0, REG_BINARY, (const BYTE *)(const char *)v, v.len());
    }
    bufsize = sizeof(dir);
    if(RegQueryValueEx(key, vald, 0, &type, dir, &bufsize) != ERROR_SUCCESS)
    {
        GRTLOG("failed query dummy key", 0, 0);
        v = gen_id();
        RegSetValueEx(key, vald, 0, REG_BINARY, (const BYTE *)(const char *)v, v.len());
        if(RegQueryValueEx(key, vald, 0, &type, dir, &bufsize) != ERROR_SUCCESS)
        {
            goto next_try;
        }
        GRTLOG("reset dummy key succeeded", 0, 0);
        GRTLOGVC(to_hex(v));
    }
    if(type != REG_BINARY || bufsize != 10)
    {
        GRTLOG("dummy key type or len wrong", 0, 0);
        goto next_try;
    }
    RegCloseKey(key);
    d4.munge(tmpk, strlen(tmpk));
    d5.munge(vald, strlen(vald));

    v = vc(VC_BSTRING, (const char *)dir, 10);
    GRTLOG("dummy key worked", 0, 0);
    GRTLOGVC(to_hex(v));
    return v;

next_try:
    RegCloseKey(key);
    return vcnil;
}

static vc
get_guid()
{
    vc v;
    char tmpk[1024];
    char vald[1024];
    HKEY key;
    DWORD type;
    BYTE dir[4096];
    DWORD bufsize = sizeof(dir);
    // just see if we can get the MS supplied UUID
    // software\microsoft\cryptography
    static char mskey[] =
        "\xD4\xDF\xD6\xF7\xD5\xD2\xFC\xD3\xF2\xCE\xC4\xC5\xFA\xDC\xC5\xC0"
        "\xC2\xDD\xFB\xD3\xC2\xFA\xD2\xC7\xE1\xD1\xDC\xC2\xDD\xCE\xF1"
        ;
    // machineguid
    static char msval[] =
        "\xCA\xD1\xD3\xEB\xCB\xDD\xEB\xD1\xDB\xCA\xC9"
        ;

    strcpy(tmpk, mskey);
    Enc e6;
    Enc d6;
    e6.munge(tmpk, strlen(tmpk));
    Enc e7;
    Enc d7;
    strcpy(vald, msval);
    e7.munge(vald, strlen(vald));

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, tmpk,
                    0, KEY_READ, &key) != ERROR_SUCCESS)
    {
        //method = -1;
        GRTLOG("guid open failed", 0, 0);
        return vcnil;
    }
    memset(dir, 0, sizeof(dir));
    bufsize = sizeof(dir) - 1;
    if(RegQueryValueEx(key, vald, 0, &type, dir, &bufsize) != ERROR_SUCCESS)
    {
        GRTLOG("guid query failed", 0, 0);
        //method = -1;
        return vcnil;
    }
    if(type != REG_SZ)
    {
        //method = -1;
        GRTLOG("guid type wrong", 0, 0);
        return vcnil;
    }
    RegCloseKey(key);
    d6.munge(tmpk, strlen(tmpk));
    d7.munge(vald, strlen(vald));

    byte a[20];
    SHA1 sha;
    sha.Update(dir, strlen((const char *)dir));
    sha.Update((const byte *)"8395821084", 10);
    sha.Final(a);
    v = vc(VC_BSTRING, (const char *)a, 10);
    GRTLOG("guid key succeeded", 0, 0);
    GRTLOGVC(to_hex(v));
    return v;
}

static
vc
method_6(int& method)
{
    method = 6;
    DwString cr = newfn("crumb");
    int f = open(cr.c_str(), O_RDONLY|O_BINARY);
    if(f == -1)
    {
        f = open(cr.c_str(), O_WRONLY|O_CREAT|O_BINARY, S_IREAD);
        if(f == -1)
        {
            method = -1;
            return vcnil;
        }
        vc id = gen_id();
        if(write(f, (const char *)id, id.len()) != id.len())
        {
            close(f);
            unlink(cr.c_str());
            method = -1;
            return vcnil;
        }
        close(f);
        return id;
    }
    char id[10];
    if(read(f, id, sizeof(id)) != sizeof(id))
    {
        close(f);
        unlink(cr.c_str());
        method = -1;
        return vcnil;
    }
    close(f);
    vc v(VC_BSTRING, id, sizeof(id));
    return v;
}

vc
set_get_uniq(int& method)
{
    HKEY key;
    Enc e;
    Enc d;
    vc v;

#ifdef DISKID
    method = 1;
    v = getdiskid();
    if(!v.is_nil())
        return v;
#endif
    method = 2;
    v = get_guid();
    if(!v.is_nil())
        return v;

    method = 3;

    v = set_get_dummy();
    if(!v.is_nil())
    {
        return v;
    }

    v = method_6(method);

    return v;


}


