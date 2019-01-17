
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "netvid.h"
#include "netcod.h"
#include "aconn.h"
#include "dwlog.h"
#include "mmchan.h"
#include "doinit.h"
#include "filetube.h"
#include "dwrtlog.h"
#include "ta.h"


static Listener *Listen_sock;
static Listener *Listen_sock_save;
static Listener *Static_secondary_sock;
static Listener *Static_secondary_sock_save;

DwNetConfig DwNetConfigData;

// these just pause the connection accept
// process, in order to serialize connection
// setup. see below to really turn off the socket
void
turn_listen_off()
{

    if(Listen_sock)
    {
        Listen_sock_save = Listen_sock;
        Listen_sock = 0;
    }
    if(Static_secondary_sock)
    {
        Static_secondary_sock_save = Static_secondary_sock;
        Static_secondary_sock = 0;
    }
}

void
turn_listen_on()
{

    if(Listen_sock_save)
    {
        Listen_sock = Listen_sock_save;
        Listen_sock_save = 0;
    }
    if(Static_secondary_sock_save)
    {
        Static_secondary_sock = Static_secondary_sock_save;
        Static_secondary_sock_save = 0;
    }
}

int
is_listening()
{
    return Listen_sock_save || Listen_sock;
}

void
poll_listener()
{
    if(Listen_sock && Listen_sock->accept_ready())
    {
        SimpleSocket *ctrl_sock;
        if((ctrl_sock = Listen_sock->accept()) == 0)
        {
            // try to recover
            delete Listen_sock;
            Listen_sock = new Listener;
            return;
        }
        // setup Tube
        MMTube *tube = new MMTube;
        if(tube->accept(ctrl_sock, 0) < 0) // NOTE: FIXME, maybe need unreliable channels
        {
            delete tube;
            tube = 0;
        }
        else
        {
            // build a channel
            // rest of channel is built after the
            // remote configuration is received
            // and verified.
            MMChannel *chan = new MMChannel;
            GRTLOG("new primary control chan %d", chan->myid, 0);
            // don't need this listener anymore since we are
            // nixing non-firewall-friendly stuff
            //tube->init_listener();
            chan->tube = tube;
            chan->wait_for_crypto();
            chan->start_service();
            turn_listen_off();
            TRACK_ADD(DC_accept_direct, 1);
        }
    }

    if(Static_secondary_sock && Static_secondary_sock->accept_ready())
    {
        SimpleSocket *ctrl_sock;
        if((ctrl_sock = Static_secondary_sock->accept()) == 0)
        {
            // try to recover
            delete Static_secondary_sock;
            Static_secondary_sock = new Listener;
            return;
        }
        // setup dummy tube to allow deferred
        // association with whatever connection
        // cluster.
        MMTube *tube = new DummyTube;
        tube->connect(ctrl_sock->peer_addr().c_str(), 0);
        int ch = tube->find_chan();
        tube->set_channel(ctrl_sock, 0, 0, ch);

        MMChannel *chan = new MMChannel;
        chan->tube = tube;
        chan->pstate = MMChannel::ESTABLISHED;
        // subchannel, we set it so it can get an
        // initial command that will tell it to
        // associate with another primary channel
        //chan->protos[ch] = MMChannel::CHAN_GET_WHAT_TO_DO;
        chan->start_service();
        sproto *s = new sproto(ch, recv_command, chan->vp);
        chan->simple_protos[ch] = s;
        s->start();
        TRACK_ADD(DC_accept_direct_secondary, 1);
    }

}

void
set_listen_state(int on)
{
    if(!Listen_sock && !Listen_sock_save)
    {
        if(Listen_sock_save || Static_secondary_sock_save)
            return; // you must have listening "on" before you can do this
        if(on)
        {
            Listen_sock = new Listener;
            Listen_sock->non_blocking(1);
            DwString lp(DwNetConfigData.get_primary_suffix("any"));
            if(!Listen_sock->init2(lp.c_str()))
            {
                (*MMChannel::popup_message_box_callback)(0, "can't listen (winsock error)", vcnil, vcnil);
                delete Listen_sock;
                Listen_sock = 0;
            }
            Static_secondary_sock = new Listener;
            Static_secondary_sock->non_blocking(1);
            DwString lp2(DwNetConfigData.get_secondary_suffix("any"));
            if(!Static_secondary_sock->init2(lp2.c_str()))
            {
                (*MMChannel::popup_message_box_callback)(0, "can't listen (winsock error)", vcnil, vcnil);
                delete Static_secondary_sock;
                Static_secondary_sock = 0;
            }
        }
    }
    else
    {
        //XXX NOTE: may want to check for inhibit on
        // various things and uninhibit them, since we
        // won't be getting any more connections at this point.
        if(!on)
        {
            delete Listen_sock;
            Listen_sock = 0;
            delete Listen_sock_save;
            Listen_sock_save = 0;
            delete Static_secondary_sock;
            Static_secondary_sock = 0;
            delete Static_secondary_sock_save;
            Static_secondary_sock_save = 0;
        }
    }
}

#define FW_SECTION "firewall"


#define DEFAULT_PORT_SUFFIX ":6780"
#define DEFAULT_PORT_SUFFIX_SECONDARY ":6781"
#define DEFAULT_PRIMARY_PORT 6780
#define DEFAULT_SECONDARY_PORT 6781
#define DEFAULT_PAL_PORT 6783

#define DEFAULT_ADVERTISE_NAT_PORTS 0
#define DEFAULT_DISABLE_UPNP 0
#ifdef CDC32
#define DEFAULT_CALL_SETUP_MEDIA_SELECT CSMS_VIA_HANDSHAKE
#else
#define DEFAULT_CALL_SETUP_MEDIA_SELECT CSMS_TCP_ONLY
#endif



DwNetConfig::DwNetConfig()
DWUIINIT_CTOR_BEGIN,
DWUIINIT_CTOR_VAL(primary_port),
DWUIINIT_CTOR_VAL(secondary_port),
DWUIINIT_CTOR_VAL(pal_port),
DWUIINIT_CTOR_VAL(nat_primary_port),
DWUIINIT_CTOR_VAL(nat_secondary_port),
DWUIINIT_CTOR_VAL(nat_pal_port),
DWUIINIT_CTOR_VAL(advertise_nat_ports),
DWUIINIT_CTOR_VAL(disable_upnp),
DWUIINIT_CTOR_VAL(call_setup_media_select)
DWUIINIT_CTOR_END
{
    set_primary_port(DEFAULT_PRIMARY_PORT);
    set_secondary_port(DEFAULT_SECONDARY_PORT);
    set_pal_port(DEFAULT_PAL_PORT);

    // note: these defaults correspond to using the DMZ
    // on a NAT box
    set_nat_primary_port(DEFAULT_PRIMARY_PORT);
    set_nat_secondary_port(DEFAULT_SECONDARY_PORT);
    set_nat_pal_port(DEFAULT_PAL_PORT);
    set_advertise_nat_ports(DEFAULT_ADVERTISE_NAT_PORTS);
    set_disable_upnp(DEFAULT_DISABLE_UPNP);
    set_call_setup_media_select(DEFAULT_CALL_SETUP_MEDIA_SELECT);
}

void
DwNetConfig::save()
{
    save_syncmap(syncmap, FW_SECTION ".dif");
}

void
DwNetConfig::load()
{
    load_syncmap(syncmap, FW_SECTION ".dif");
}

DwString
DwNetConfig::get_primary_suffix(const char *ip)
{
    char a[100];
    sprintf(a, ":%d", get_primary_port());
    DwString ret(ip);
    ret += a;
    return ret;
}

DwString
DwNetConfig::get_secondary_suffix(const char *ip)
{
    char a[100];
    sprintf(a, ":%d", get_secondary_port());
    DwString ret(ip);
    ret += a;
    return ret;
}

DwString
DwNetConfig::get_pal_suffix(const char *ip)
{
    char a[100];
    sprintf(a, ":%d", get_pal_port());
    DwString ret(ip);
    ret += a;
    return ret;
}

DwString
DwNetConfig::get_nat_primary_suffix(const char *ip)
{
    char a[100];
    sprintf(a, ":%d", get_nat_primary_port());
    DwString ret(ip);
    ret += a;
    return ret;
}

DwString
DwNetConfig::get_nat_secondary_suffix(const char *ip)
{
    char a[100];
    sprintf(a, ":%d", get_nat_secondary_port());
    DwString ret(ip);
    ret += a;
    return ret;
}

DwString
DwNetConfig::get_nat_pal_suffix(const char *ip)
{
    char a[100];
    sprintf(a, ":%d", get_nat_pal_port());
    DwString ret(ip);
    ret += a;
    return ret;
}
