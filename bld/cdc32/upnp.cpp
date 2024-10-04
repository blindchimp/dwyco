
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWYCO_NO_UPNP
#include "upnp.h"
//#ifndef ANDROID
#include "miniupnpc.h"
#include "upnpcommands.h"
#include "upnperrors.h"
//#endif
#include "dwrtlog.h"
#include "dwstr.h"
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif
#include <stdio.h>

using namespace dwyco;

namespace dwyco {

static int Natport1, Natport2, Local_port1, Local_port2;

static
#ifdef _WIN32
void
#else
void *
#endif
threaded_upnp(void *)
{
    do_upnp(Natport1, Natport2, Local_port1, Local_port2);
#ifdef _WIN32
    _endthread();
#else
    return 0;
#endif
}

int
bg_upnp(int natport1, int natport2, int local_port1, int local_port2)
{
#ifndef DWYCO_NO_UPNP
    Natport1 = natport1;
    Natport2 = natport2;
    Local_port1 = local_port1;
    Local_port2 = local_port2;

#ifdef _WIN32
    HANDLE h = (HANDLE)_beginthread(threaded_upnp, 4096, 0);
    if(h == 0 || h == (void *)-1)
	    return 0;
#endif
#ifdef LINUX
    pthread_t pt;
    int i = pthread_create(&pt, 0, threaded_upnp, 0);
    if(i != 0)
        return 0;
#endif

#endif
    return 1;

}

int
do_upnp(int natport1, int natport2, int local_port1, int local_port2)
{
#if !(0 || defined(DWYCO_NO_UPNP))
    struct UPNPDev *devlist;
    int error = 0;

    devlist = upnpDiscover(5000, 0, 0,
                           UPNP_LOCAL_PORT_ANY, 0, 2, &error);
    if(devlist == 0)
        return 0;
    int i;

    struct UPNPUrls urls;
    struct IGDdatas data;
    char lanaddr[64] = "unset";

    i = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr));
    freeUPNPDevlist(devlist);

    switch(i) {
    case 1:
        GRTLOG("Found valid IGD : %s", urls.controlURL, 0);
        break;
    case 2:
        GRTLOG("Found a (not connected?) IGD : %s", urls.controlURL, 0);
        break;
    case 3:
        GRTLOG("UPnP device found. Is it an IGD ? : %s", urls.controlURL, 0);
        break;
    default:
        GRTLOG("Found device (igd ?) : %s", urls.controlURL, 0);
    }
    GRTLOG("Local LAN ip address : %s", lanaddr, 0);

    if(!(i == 1 || i == 2))
        return 0;

    int r;

    DwString p1 = DwString::fromInt(natport1);
    DwString p2 = DwString::fromInt(local_port1);

#ifdef ANDROID
#define NM1 "android1"
#define NM2 "android2"
#else
#define NM1 "dwyco1"
#define NM2 "dwyco2"
#endif

    r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,
                            p1.c_str(), p2.c_str(), lanaddr, NM1,
                            "TCP", 0, "86400");
    if(r!=UPNPCOMMAND_SUCCESS)
    {
        GRTLOG("AddPortMapping failed with code %d (%s)",
               r, strupnperror(r));
        FreeUPNPUrls(&urls);
        return 0;
    }

    p1 = DwString::fromInt(natport2);
    p2 = DwString::fromInt(local_port2);
    r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,
                            p1.c_str(), p2.c_str(), lanaddr, NM2,
                            "TCP", 0, "86400");
    if(r!=UPNPCOMMAND_SUCCESS)
    {
        GRTLOG("AddPortMapping failed with code %d (%s)",
               r, strupnperror(r));
        FreeUPNPUrls(&urls);
        return 0;
    }

    FreeUPNPUrls(&urls);
#endif
    return 1;

}
}
#endif

