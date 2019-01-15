#include "upnp.h"
#include "miniupnpc.h"
#include "upnpcommands.h"
#include "upnperrors.h"
#include "dwrtlog.h"

using namespace dwyco;

Upnp::Upnp()
{

}

namespace dwyco {
void
do_upnp()
{
    struct UPNPDev * devlist = 0;
    int error = 0;

    devlist = upnpDiscover(2000, 0, 0,
                           UPNP_LOCAL_PORT_ANY, 0, 2, &error);
    if(devlist == 0)
        return;
    int i;
    struct UPNPDev * device;
    struct UPNPUrls urls;
    struct IGDdatas data;
    char lanaddr[64] = "unset";

    i = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr));

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

//    if(SetRedirectAndTest(&urls, &data,
//                               lanaddr, "6780",
//                               "46780", "tcp",
//                               "0",
//                               "cdc-upnp", 0) < 0)
//        return;
    int r;

    r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,
                            "46780", "6780", lanaddr, "cdc-upnp",
                            "TCP", 0, "0");
    if(r!=UPNPCOMMAND_SUCCESS)
    {
        GRTLOG("AddPortMapping failed with code %d (%s)",
               r, strupnperror(r));
    }

}
}

