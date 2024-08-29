
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <time.h>
#include "netvid.h"
#include "aconn.h"
#include "mmchan.h"
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
#include "ssns.h"
#include "calllive.h"
#include "cdcpal.h"
#include "se.h"
#include "dhgsetup.h"
#include "dirth.h"
#include "dwstr.h"

// NOTENOTENOTE! fixme, look at the dlli.cpp section where some of these
// settings are tweaked and set bindings somewhere so we can do the
// machinations needed when network state changes
// XXX NOTE, may want to turn broadcasting off if user is "invisible"
//
// NOTE: separate out broadcasting and discovery
// discovery should probably go on whether we are accepting new connections
// or not (this is needed for apps that want to connect and stream from a
// "server", for example)
//
// broadcasting our whereabouts on the local net is tied to whether we are
// accepting new connections. if you turn off listening, it turns off the
// broadcasting as well.
//
// ca 2023, broadcasting on the local net i thought would be a good idea
// in the case of using a local sync situation. if you aren't in a sync
// group, it makes less sense, since it is unlikely to improve situations
// for messaging. for streaming a security camera, it might make sense, so
// maybe a better api is needed to decide whether local broadcasting is useful.
// for now, if you are not in a sync group, we turn off the broadcasting, since it
// is unlikely to improve service.

// ca 2023, decided to try something a little different.
// i started with a continuous broadcast at a particular interval, and if
// a receiver did not see a broadcast in awhile, it assumed the information
// was stale and removed the IP from our discovery table. this gives better
// liveness information, but it also means there is a lot more broadcasting
// than we really need. in addition, "status change" messages were emitted
// which the app could use to update its display (not sure this was a good
// idea, essentially coupling the IP discovery with "liveness".)
//
// try this instead:
// broadcast more slowly, but just accumulate all the results without
// timing them out. if we receive updated information for a uid, then
// replace it in the discovery table with new info. stale ip addresses
// aren't really a huge problem, since we usually have a "try it, if it fails
// try the next thing" protocol. possibly add an api to remove an ip from the
// table, so stale info could be removed more promptly.
// NOTE: 5/2023, reinstate freshness, but at the slower refresh rate. if we
// keep the stale info around for a long time (ie, like in a background process
// that runs for months) the stale ip would result in essentially "hiding" the
// more up to date info we get from the server. this means our "try it, then go
// via server" protocol wpuld never direct connect.
//
// the API for getting an ip from a uid should probably be updated to take into
// account the freshness of the info. another thing might be to try multiple "direct"
// attempts simultaneously and use the one that finishes first, but that is a major
// change for some minor gains.

extern vc LocalIP;
extern int Media_select;

namespace dwyco {
#define DESERIALIZE_MAX_STRING_LEN (50)
vc App_ID;

static Listener *Listen_sock;
static Listener *Static_secondary_sock;
static int Inhibit_accept;
static vc Local_discover;
static vc Local_broadcast;
vc Broadcast_discoveries;
static DwTreeKaz<time_t, vc> Freshness(0);
ssns::signal2<vc, int> Local_uid_discovered;
void set_listen_state(int on);

// note: this slot is called whenever anything in the net
// section changes. this can lead to a bit of thrashing
// esp during startup, but otherwise, the network state doesn't
// change all that often.

void
net_section_changed(vc name, vc val)
{
    App_ID = get_settings_value("net/app_id");

    if(is_listening())
    {
        set_listen_state(0);
    }

    int listen = get_settings_value("net/listen");
    set_listen_state(listen);
    pal_reset();

    //note: we depend on the values being sent in here being the same
    // as the ones in aconn.h
    int call_setup_media_select = get_settings_value("net/call_setup_media_select");
    switch(call_setup_media_select)
    {
    default:
    case CSMS_VIA_HANDSHAKE:
        Media_select = MEDIA_VIA_HANDSHAKE;
        break;
    case CSMS_TCP_ONLY:
        Media_select = MEDIA_TCP_VIA_PROXY;
        break;
    case CSMS_UDP_ONLY:
        Media_select = MEDIA_UDP_VIA_STUN;
        break;
    }
}

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
    istrm.max_depth = 2;
    istrm.max_element_len = DESERIALIZE_MAX_STRING_LEN;
    istrm.max_elements = BD_DESERIALIZE_LIMIT;
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
    Broadcast_timer.stop();
}

static
int
start_broadcaster()
{
    Local_broadcast = vc(VC_SOCKET_DGRAM);
    Local_broadcast.set_err_callback(net_socket_error);

    DwString a;
    a = "any:any";
    if(Local_broadcast.socket_init(a.c_str(), 0, 1).is_nil())
    {
        stop_broadcaster();
        return 0;
    }
    if(Local_broadcast.socket_set_option(VC_SET_BROADCAST, 1).is_nil())
    {
        stop_broadcaster();
        return 0;
    }
    a = "255.255.255.255:";
    a += DwString::fromInt((int)get_settings_value("net/broadcast_port"));
    Local_broadcast.socket_connect(a.c_str());

    Broadcast_timer.reset();

    // we want the first broadcast to happen fairly quickly.
    Broadcast_timer.set_interval(1);
    Broadcast_timer.set_autoreload(0);
    Broadcast_timer.start();
    return 1;
}

static
void
stop_discover()
{
    Local_discover = vcnil;
    auto it = DwTreeKazIter<time_t, vc>(&Freshness);
    DwVec<vc> kill;
    for(; !it.eol(); it.forward())
    {
        auto a = it.get();
        kill.append(a.get_key());
    }
    for(int i = 0; i < kill.num_elems(); ++i)
    {
        Local_uid_discovered.emit(kill[i], 0);
        se_emit(SE_STATUS_CHANGE, kill[i]);
    }
    Freshness.clear();
    Broadcast_discoveries = vc(VC_TREE);
}

static
int
start_discover()
{
    if(Broadcast_discoveries.is_nil())
        Broadcast_discoveries = vc(VC_TREE);

    Local_discover = vc(VC_SOCKET_DGRAM);
    Local_discover.set_err_callback(net_socket_error);

    DwString a;
    a = "any:";
    a += DwString::fromInt((int)get_settings_value("net/broadcast_port"));
    // this didn't appear to work with just "ip:port" for broadcasts
    if(Local_discover.socket_init(a.c_str(), 0, 1).is_nil())
    {
        stop_discover();
        return 0;
    }
    if(Local_discover.socket_set_option(VC_SET_BROADCAST, 1).is_nil())
    {
        stop_discover();
        return 0;
    }
    return 1;
}

static
void
broadcast_check(DH_alternate *d)
{
    if(d == nullptr)
    {
        stop_broadcaster();
    }
    else
    {
        if(Listen_sock)
            start_broadcaster();
        else
            stop_broadcaster();
    }
}

static
void
broadcast_check2(int d)
{
    if(d && Listen_sock)
        start_broadcaster();
    else
        stop_broadcaster();
}

void
init_aconn()
{
    Broadcast_discoveries = vc(VC_TREE);
    bind_sql_section("net/", net_section_changed);
    App_ID = get_settings_value("net/app_id");
    start_discover();
    Current_alternate.value_changed.connect_ptrfun(broadcast_check, ssns::UNIQUE);
    Database_online.value_changed.connect_ptrfun(broadcast_check2, ssns::UNIQUE);

}

static
vc
strip_port(vc ip)
{
    DwString a((const char *)ip);
    int b = a.find(":");
    if(b == DwString::npos)
        return ip;
    a.remove(b);
    vc ret(a.c_str());
    return ret;
}

static
void
broadcast_announcement()
{
    if(Local_broadcast.is_nil())
    {
        return;
    }

    vc announce(VC_VECTOR);
    announce[0] = My_UID;
    vc v(VC_VECTOR);
    v[BD_IP] = LocalIP;
    v[BD_PRIMARY_PORT] = get_settings_value("net/primary_port");
    v[BD_SECONDARY_PORT] = get_settings_value("net/secondary_port");
    v[BD_PAL_PORT] = get_settings_value("net/pal_port");

    vc nicename = get_settings_value("user/username");
    if(nicename.len() > DESERIALIZE_MAX_STRING_LEN - 20)
    {
        // arbitrary truncation. note, doesn't work if
        // nicename has utf8 stuff in it or whatever
        DwString d((const char *)nicename, 0, DESERIALIZE_MAX_STRING_LEN - 20);
        nicename = vc(VC_BSTRING, d.c_str(), d.length());
    }
    v[BD_NICE_NAME] = nicename;
    v[BD_APP_ID] = App_ID;
    v[BD_DISPOSITION] = MMChannel::My_disposition;
    announce[1] = v;
    if(!sendvc(Local_broadcast, announce))
    {
        // this can happen if the network goes down, and the socket
        // will be closed because of whatever random error the
        // socket stack decides to return.
        stop_broadcaster();
        start_broadcaster();
        // override the short timer
        Broadcast_timer.reset();
        Broadcast_timer.set_interval(60);
        Broadcast_timer.set_autoreload(0);
        Broadcast_timer.start();
    }
}

static
void
broadcast_tick()
{
    if(Local_broadcast.is_nil())
    {
        return;
    }

    if(!Broadcast_timer.is_expired())
        return;
    Broadcast_timer.ack_expire();

    broadcast_announcement();

    // this is a compat hack
    int iv = (int)get_settings_value("net/broadcast_interval");
    if(iv <= 10)
    {
        iv = 60;
        set_settings_value("net/broadcast_interval", iv);
    }

    Broadcast_timer.set_interval(iv * 1000);
    Broadcast_timer.start();
}

static
void
discover_tick()
{
    if(Local_discover.is_nil())
    {
        return;
    }
    if(has_data(Local_discover, 0, 0))
    {
        vc data;
        vc peer;
        if(recvvc(Local_discover, data, peer) > 0)
        {
            if(data.type() == VC_VECTOR &&
                    data[0].type() == VC_STRING &&
                    data[1].type() == VC_VECTOR)
            {
                vc uid = data[0];
                if(uid != My_UID && (!App_ID.is_nil() && App_ID == data[1][BD_APP_ID]))
                {
                    GRTLOG("FOUND LOCAL from %s", (const char *)peer, 0);
                    GRTLOGVC(data);
                    data[1][BD_IP] = strip_port(peer);
                    vc new_d = data[1];
                    vc d;
                    if(!Broadcast_discoveries.find(uid, d) ||
                            new_d[BD_IP] != d[BD_IP] ||
                            new_d[BD_PRIMARY_PORT] != d[BD_PRIMARY_PORT] ||
                            new_d[BD_SECONDARY_PORT] != d[BD_SECONDARY_PORT] ||
                            new_d[BD_PAL_PORT] != d[BD_PAL_PORT] ||
                            new_d[BD_DISPOSITION] != d[BD_DISPOSITION]
                            )
                    {
                        Broadcast_discoveries.add_kv(uid, data[1]);
                        Local_uid_discovered.emit(uid, 1);
                        se_emit(SE_STATUS_CHANGE, uid);
                        // it might make sense to just do a unicast send back to
                        // the newly discovered address, instead of a broadcast.
                        // this might cause a little mini-storm of announcements
                        broadcast_announcement();
                    }
                    Freshness.replace(uid, time(0));
                }
            }
        }
    }

#if 1
    auto it = DwTreeKazIter<time_t, vc>(&Freshness);
    DwVec<vc> kill;
    int time_limit = (3 * (int)get_settings_value("net/broadcast_interval"));
    for(; !it.eol(); it.forward())
    {
        auto a = it.get();
        auto tm = time(0) - a.get_value();
        if(tm  > time_limit)
            kill.append(a.get_key());
    }
    for(int i = 0; i < kill.num_elems(); ++i)
    {
        Freshness.del(kill[i]);
        Broadcast_discoveries.del(kill[i]);
        Local_uid_discovered.emit(kill[i], 0);
        se_emit(SE_STATUS_CHANGE, kill[i]);
    }
#endif
}



void
poll_listener()
{
    broadcast_tick();
    discover_tick();
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
        if(tube->accept(ctrl_sock) < 0)
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
        chan->start_service();
        sproto *s = new sproto(ch, recv_command, chan->vp);
        chan->simple_protos[ch] = s;
        s->start();
        GRTLOG("new secondary chan %d", chan->myid, 0);
        TRACK_ADD(DC_accept_direct_secondary, 1);
    }
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
            if(Current_alternate)
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
