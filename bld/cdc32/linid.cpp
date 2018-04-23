
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "vc.h"
#include "qauth.h"
#include "uicfg.h"
#include "sha.h"
using namespace CryptoPP;

#define HASH_SECRET "\x00\x94\x3d\x23\x98\x78\x01"
static
vc
method_5(int& method)
{
    method = 5;
    TProfile p("admin", "");
    char buf[100];
    p.GetString("0102", buf, sizeof(buf) - 1, "");
    if(strlen(buf) == 20)
    {
        return from_hex(vc(buf));
    }
    int f = open("/dev/urandom", O_RDONLY);
    if(f == -1)
    {
        method = -1;
        return vcnil;
    }
    char id[10];
    if(read(f, id, sizeof(id)) != sizeof(id))
    {
        close(f);
        method = -1;
        return vcnil;
    }
    close(f);
    vc mid(VC_BSTRING, id, sizeof(id));
    p.WriteString("0102", (const char *)to_hex(mid));
    return mid;
}

vc
set_get_uniq(int& method)
{
    int ret;
#if defined(ANDROID) || defined(DWYCO_IOS)
    return method_5(method);
#else

#ifdef MACOSX
    ret = system("/sbin/ifconfig | sed -n \"s/.*ether //p\" | head -1 >/tmp/.k");
#else
    ret = system("/sbin/ifconfig | sed -n \"s/.*HWaddr //p\" | head -1 >/tmp/.k");
#endif
    if(ret != 0)
    {
        // just put the mid into a file in the admin.dif
        return method_5(method);
    }
    SHA sha;
    byte *secret = (byte *)HASH_SECRET;
    sha.Update(secret, sizeof(HASH_SECRET) - 1);
    FILE *f = fopen("/tmp/.k", "r");
    if(!f)
        return method_5(method);
    char line[1024];
    memset(line, 0, sizeof(line));
    if(fgets(line, sizeof(line), f) == 0)
    {
        fclose(f);
        return method_5(method);
    }
    fclose(f);
    unlink("/tmp/.k");
    sha.Update((const byte *)line, strlen(line));
    byte a[20];
    sha.Final(a);
    // 80-bits
    vc auth(VC_BSTRING, (const char *)a, 10);
    method = 4;
    return auth;
#endif
}

#undef TEST
#ifdef TEST
int
main(int, char **)
{
    int dum;
    set_get_uniq(dum);
}
#endif
