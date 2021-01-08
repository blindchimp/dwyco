
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
#include "sproto.h"
#include "vc.h"
#include "vcxstrm.h"
#include "vccomp.h"
#include "vcsock.h"
#include "qauth.h"
#include "ezset.h"

// NOTENOTENOTE! fixme, look at the dlli.cpp section where some of these
// settings are tweaked and set bindings somewhere so we can do the
// machinations needed when network state changes

extern vc LocalIP;
namespace dwyco {
//DwNetConfig DwNetConfigData;
static Listener *Listen_sock;
static Listener *Static_secondary_sock;
static int Inhibit_accept;
static vc Local_discover;
static vc Local_broadcast;



// these just pause the connection accept
// process, in order to serialize connection
// setup. see below to really turn off the socket
void
turn_accept_off()
{
    Inhibit_accept = 1;
}

void
turn_accept_on()
{
    Inhibit_accept = 0;
}

int
is_listening()
{
    return Listen_sock != 0;
}

// note: i thought about using Framesocket, but it is too wound up
// with the old network api
static char packet_buf[32768];
static int plen = sizeof(packet_buf);
static DwTimer Broadcast_timer("broadcast");
#define BROADCAST_INTERVAL (10 * 1000)

static
int
sendvc(vc sock, vc v)
{
    int len;

    vcxstream ostrm(sock, (char *)packet_buf, plen, vcxstream::FIXED);

    if(!ostrm.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
        return 0;
    vc_composite::new_dfs();
    if((len = v.xfer_out(ostrm)) < 0)
        return 0;
    if(!ostrm.close(vcxstream::FLUSH))
        return 0;
    return len;
}

static
int
recvvc(vc sock, vc& v, vc& peer)
{
    int len;
    vcxstream istrm(sock, (char *)packet_buf, plen, vcxstream::FIXED);

    if(!istrm.open(vcxstream::READABLE, vcxstream::ATOMIC))
        return 0;
    if((len = v.xfer_in(istrm)) < 0)
    {
        return 0;
    }
    if(!istrm.close(vcxstream::FLUSH))
        return 0;

    peer = sock.socket_peer_addr();
    return len;
}

static
int
has_data(vc sock, int sec, int usec)
{
    int a;
    if((a = sock.socket_poll(VC_SOCK_READ, sec, usec)) == VC_SOCK_READ)
        return 1;
    if(a == -1)
    {
        return 0;
    }
    return 0;
}

#define BROADCAST_PORT "48901"
static
int
net_socket_error(vc *v)
{
    vc_socket *vs = (vc_socket *)v;
    GRTLOG("broadcast socket error", 0, 0);
    GRTLOGVC(vs->socket_error_desc());
    return VC_SOCKET_BACKOUT;
}

static
void
stop_broadcaster()
{
    Local_broadcast = vcnil;
    Local_discover = vcnil;
    Broadcast_timer.stop();
}

static
int
start_broadcaster()
{
    if(LocalIP.is_nil())
        return 0;
    Local_broadcast = vc(VC_SOCKET_DGRAM);
    Local_broadcast.set_err_callback(net_socket_error);
    Local_discover = vc(VC_SOCKET_DGRAM);
    Local_discover.set_err_callback(net_socket_error);

    DwString a;
    a = "any:";
    a += BROADCAST_PORT;
    // this didn't appear to work with just "ip:port" for broadcasts
    if(Local_discover.socket_init(a.c_str(), 0, 0).is_nil())
    {
        stop_broadcaster();
        return 0;
    }
    a = (const char *)LocalIP;
    a += ":any";
    if(Local_broadcast.socket_init(a.c_str(), 0, 0).is_nil())
    {
        stop_broadcaster();
        return 0;
    }
    if(Local_broadcast.socket_set_option(VC_SET_BROADCAST, 1).is_nil())
    {
        stop_broadcaster();
        return 0;
    }
    if(Local_discover.socket_set_option(VC_SET_BROADCAST, 1).is_nil())
    {
        stop_broadcaster();
        return 0;
    }
    a = "255.255.255.255:";
    a += BROADCAST_PORT;
    Local_broadcast.socket_connect(a.c_str());
    Broadcast_timer.reset();
    Broadcast_timer.set_interval(BROADCAST_INTERVAL);
    Broadcast_timer.set_autoreload(0);
    Broadcast_timer.start();
    return 1;
}

static
void
broadcast_tick()
{
    if((Local_broadcast.is_nil() || Local_discover.is_nil()) && LocalIP.is_nil())
    {
        return;
    }
    if((Local_broadcast.is_nil() || Local_discover.is_nil()) && !LocalIP.is_nil())
    {
        start_broadcaster();
        return;
    }
    if(has_data(Local_discover, 0, 0))
    {
        vc data;
        vc peer;
        if(recvvc(Local_discover, data, peer) > 0)
        {
            GRTLOG("FOUND LOCAL from %s", (const char *)peer, 0);
            GRTLOGVC(data);
        }
    }
    if(!Broadcast_timer.is_expired())
        return;
    Broadcast_timer.ack_expire();
    vc announce(VC_VECTOR);
    announce[0] = My_UID;
    announce[1] = LocalIP;
    vc v(VC_VECTOR);
    v.append(get_settings_value("net/primary_port"));
    v.append(get_settings_value("net/secondary_port"));
    v.append(get_settings_value("net/pal_port"));
    announce[2] = v;
    sendvc(Local_broadcast, announce);
    Broadcast_timer.start();
}



void
poll_listener()
{
    if(Inhibit_accept)
        return;
    if(Listen_sock && Listen_sock->accept_ready())
    {
        SimpleSocket *ctrl_sock;
        if((ctrl_sock = Listen_sock->accept()) == 0)
        {
            // try to recover
            set_listen_state(0);
            set_listen_state(1);
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
            turn_accept_off();
            TRACK_ADD(DC_accept_direct, 1);
        }
    }

    if(Static_secondary_sock && Static_secondary_sock->accept_ready())
    {
        SimpleSocket *ctrl_sock;
        if((ctrl_sock = Static_secondary_sock->accept()) == 0)
        {
            // try to recover
            set_listen_state(0);
            set_listen_state(1);
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

    broadcast_tick();

}

void
set_listen_state(int on)
{
    if(!Listen_sock)
    {
        if(on)
        {
            Listen_sock = new Listener;
            Listen_sock->non_blocking(1);
            DwString lp("any:");
            lp += DwString::fromInt((int)get_settings_value("net/primary_port"));
            if(!Listen_sock->init2(lp.c_str()))
            {
                (*MMChannel::popup_message_box_callback)(0, "can't listen (winsock error)", vcnil, vcnil);
                delete Listen_sock;
                Listen_sock = 0;
                return;
            }
            Static_secondary_sock = new Listener;
            Static_secondary_sock->non_blocking(1);
            lp = "any:";
            lp += DwString::fromInt((int)get_settings_value("net/secondary_port"));
            if(!Static_secondary_sock->init2(lp.c_str()))
            {
                (*MMChannel::popup_message_box_callback)(0, "can't listen (winsock error)", vcnil, vcnil);
                delete Static_secondary_sock;
                Static_secondary_sock = 0;
                delete Listen_sock;
                Listen_sock = 0;
            }
            start_broadcaster();
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
            delete Static_secondary_sock;
            Static_secondary_sock = 0;
            stop_broadcaster();
        }
    }
}

}
