
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
#include "sha.h"
#include "ezset.h"
using namespace CryptoPP;
using namespace dwyco;

#define HASH_MUMBLE "\x00\x94\x3d\x23\x98\x78\x01"
static
vc
method_5(int& method)
{
    method = 5;

    vc v = get_settings_value("auth/uniq");
    if(v != vc("00000000000000000001"))
    {
        return from_hex(v);
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
    set_settings_value("auth/uniq", to_hex(mid));
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
    ret = system("/sbin/ifconfig | grep ether | sort -u | tr -d '\\012 ' >/tmp/.k");
#else
    ret = system("/sbin/ip link| grep ether | sort -u | tr -d '\\012 ' >/tmp/.k");
#endif
    if(ret != 0)
    {
        return method_5(method);
    }
    SHA sha;
    byte *secret = (byte *)HASH_MUMBLE;
    sha.Update(secret, sizeof(HASH_MUMBLE) - 1);
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
