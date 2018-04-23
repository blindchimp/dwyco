
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// this hack does a basic version of rfc3489

#include "vc.h"
#include "vcwsock.h"
#include "dwstr.h"
#include "dwtimer.h"
#include "senc.h"
#include "dirth.h"
#include "netdiag.h"
#include <fcntl.h>
#ifdef _Windows
#include <io.h>
#include <windows.h>
#endif
#include <math.h>
#include <stdio.h>
#ifndef _Windows
#include <netdb.h>
#endif
#include "dwyco_rand.h"

#define BW_TEST_BUFSIZE (64 * 1024)
#define DEFAULT_MIN_BW (28000) // in bits/sec


vc My_connection;

static vc Last_test1;
static DwString Res;

static void
printres2(DwString& res, const char *msg)
{
    res += msg;
    res += "\r\n";
}
#define printres(msg) printres2(res, (msg))

#ifdef _Windows
static double
ms_to_pc_ticks(int us)
{
    double ret;
    static double shft = pow(2., 32);
    LARGE_INTEGER ticks_per_sec;
    QueryPerformanceFrequency(&ticks_per_sec);
    ret = (double)ticks_per_sec.u.HighPart * shft + (double)ticks_per_sec.u.LowPart;
    ret *= ((double)us / 1000);
    return ret;
}

static double
li_to_double(LARGE_INTEGER li)
{
    static double shft = pow(2., 32);
    return (double)li.u.HighPart * shft + (double)li.u.LowPart;
}

static long
pc_ticks_to_ms()
{
    LARGE_INTEGER ticks_per_sec;
    static double shft = pow(2., 32);
    QueryPerformanceFrequency(&ticks_per_sec);
    if(ticks_per_sec.u.HighPart == 0 && ticks_per_sec.u.LowPart == 0)
    {
        return GetTickCount();
    }
    double ret = (double)ticks_per_sec.u.HighPart * shft + (double)ticks_per_sec.u.LowPart;
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    double tks = (double)li.u.HighPart * shft + (double)li.u.LowPart;
    tks /= ret;
    tks *= 1000;
    return (long) tks;
}
#else
static long
pc_ticks_to_ms()
{
    return GetTickCount();
}
#endif

static int
isPrivateIp(DwString ip) {
    struct in_addr addr;

    addr.s_addr = inet_addr(ip.c_str());

    if (addr.s_addr  != INADDR_NONE) {
        uint32_t haddr = ntohl(addr.s_addr);
        return ((haddr & 0xff000000) == 0x0a000000 || // 10.0.0.0/8
                (haddr & 0xff000000) == 0x7f000000 || // 127.0.0.0/8
                (haddr & 0xfff00000) == 0xac100000 || // 172.16.0.0/12
                (haddr & 0xffff0000) == 0xc0a80000);  // 192.168.0.0/16
    }
    return 0;
}

DwString
getLocalIp() {
    DwString tmp;

    char buf[256];
    gethostname(buf, 255);
    hostent* he = gethostbyname(buf);
    if(he == NULL || he->h_addr_list[0] == 0)
        return "";
    sockaddr_in dest;
    int i = 0;

    // We take the first ip as default, but if we can find a better one, use it instead...
    memcpy(&(dest.sin_addr), he->h_addr_list[i++], he->h_length);
    tmp = inet_ntoa(dest.sin_addr);
    if(isPrivateIp(tmp) || strncmp(tmp.c_str(), "169", 3) == 0) {
        while(he->h_addr_list[i]) {
            memcpy(&(dest.sin_addr), he->h_addr_list[i], he->h_length);
            DwString tmp2 = inet_ntoa(dest.sin_addr);
            if(!isPrivateIp(tmp2) && strncmp(tmp2.c_str(), "169", 3) != 0) {
                tmp = tmp2;
            }
            i++;
        }
    }
    return tmp;
}


static int
backout(vc *)
{
    return VC_SOCKET_BACKOUT;
}
#if 0

static vc
test1(vc sock, vc to)
{
// caller must supply the socket, since we
// need to have the same source address for
// some tests, and windows gives you a new port
// on each socket creation.
#if 0
    vc sock(VC_SOCKET_DGRAM);
    sock.socket_init("any:any", 0, 0);
    sock.set_err_callback(backout);
#endif
    vc p(VC_VECTOR);
    p.append(0);
    p.append(0);
    for(int i = 0; i < 3; ++i)
    {
        vc dum;
        vc data;
        vc peer;
        sock.socket_send(p, dum, to);
        int r = sock.socket_poll(VC_SOCK_READ, 0, 500000);
        if(!(r&VC_SOCK_READ))
            continue;

        if(sock.socket_recv(data, dum, peer).is_nil())
            continue;
        Last_test1 = data;
        //printres("test1:");
        //printres(data[0]);
        //printres(sock.socket_local_addr());
        if(data[0] == sock.socket_local_addr())
            return "no-nat";

        return "nat";
    }
    return "no-resp";
}

static vc
test2(vc to)
{
    vc sock(VC_SOCKET_DGRAM);
    sock.socket_init("any:any", 0, 0);
    sock.set_err_callback(backout);
    vc p(VC_VECTOR);
    p.append(1);
    p.append(1);
    for(int i = 0; i < 3; ++i)
    {
        vc dum;
        vc data;
        vc peer;
        sock.socket_send(p, dum, to);
        int r = sock.socket_poll(VC_SOCK_READ, 0, 500000);
        if(!(r&VC_SOCK_READ))
            continue;

        if(sock.socket_recv(data, dum, peer).is_nil())
            continue;
        if(data[0] == sock.socket_local_addr())
            return "no-nat";
        return "nat";
    }
    return "no-resp";
}

static vc
test3(vc to)
{
    vc sock(VC_SOCKET_DGRAM);
    sock.socket_init("any:any", 0, 0);
    sock.set_err_callback(backout);
    vc p(VC_VECTOR);
    p.append(0);
    p.append(1);
    for(int i = 0; i < 3; ++i)
    {
        vc dum;
        vc data;
        vc peer;
        sock.socket_send(p, dum, to);
        int r = sock.socket_poll(VC_SOCK_READ, 0, 500000);
        if(!(r&VC_SOCK_READ))
            continue;

        if(sock.socket_recv(data, dum, peer).is_nil())
            continue;
        if(data[0] == sock.socket_local_addr())
            return "no-nat";
        return "nat";
    }
    return "no-resp";
}
#endif


#ifdef _Windows
static vc
get_addr()
{
    vc ret(VC_VECTOR);
    // KLUGE FROM HELL
    system("ipconfig >foo.out");
    int fd;
    if((fd = open("foo.out", O_RDONLY|O_TEXT)) == -1)
        return vcnil;
    DwString line;
    while(1)
    {
        if(!line.read_line(fd))
            break;
        if(line.contains("IP Address"))
        {
            int c = line.find(":");
            if(c == DwString::npos)
                continue;
            line.erase(0, c + 1);
            // at this point, if it contains anything other
            // than numbers, "." and whitespace, we punt, it might
            // be some ipv6 or other whacky thing we can't deal with
            // right now.
            if(line.find_first_not_of(".0123456789 \t\r\n") != DwString::npos)
                continue;
            // strip off all the white space
            int w;
            while((w = line.find_first_of(" \t\r\n")) != DwString::npos)
            {
                line.erase(w, 1);
            }
            ret.append(line.c_str());

        }
    }
    close(fd);
    return ret;
}
#else
vc
get_addr()
{
    vc ret(VC_VECTOR);
    ret[0] = getLocalIp().c_str();
    return ret;
}
#endif

#if 0
DwVec<probe_result>
rfc3489(vc to, int& num_adapters, DwString &res, int use_ipconfig)
{
#ifndef _Windows
    use_ipconfig = 0;
#endif
    int ret = 0;
    DwVec<probe_result> pret;
    vc ips;
    if(use_ipconfig)
        ips = get_addr();
    else
    {
        ips = vc(VC_VECTOR);
        ips[0] = getLocalIp().c_str();
    }
    num_adapters = ips.num_elems();
    printres("Found Adapter IPs:");
    for(int i = 0; i < ips.num_elems(); ++i)
    {
        printres(ips[i]);
        pret[i].adapter_name = (const char *)ips[i];
    }
    printres("------------------");
    for(int j = 0; j < ips.num_elems(); ++j)
    {
        ret = 0;
        printres("TESTING");
        printres(ips[j]);
        DwString ipport = (const char *)ips[j];
        ipport += ":any";
        vc sock(VC_SOCKET_DGRAM);
        sock.set_err_callback(backout);
        if(sock.socket_init(ipport.c_str(), 0, 0).is_nil())
        {
            printres("Cant' bind to:");
            printres(ipport.c_str());
            ret |= NETDIAG_NAT_CANT_BIND;
            pret[j].res = NETDIAG_NAT_CANT_BIND;
            continue;
        }

        vc t1 = test1(sock, to);
        static vc nat("nat");
        static vc no_resp("no-resp");
        if(t1 == nat)
        {
            printres("NAT Router detected.");
            vc t2 = test2(to);
            if(t2 == no_resp)
            {
                vc saved_mapped_addr = Last_test1[0];
                vc t1_2 = test1(sock, Last_test1[2]);
                if(t1_2 == no_resp)
                {
                    printres("Bad response from second test1, maybe your firewall or router is blocking UDP?");
                }
                else
                {
                    if(saved_mapped_addr != Last_test1[0])
                    {
                        printres("Symmetric NAT (you will not be able to receive calls.)");
                        printres("You will be able to originate calls to people with ");
                        printres("completely open Internet connections.");
                        printres("You should also be able to make chat-only calls to");
                        printres("other users (audio/video will probably appear blank.)");
                        //printres((const char *)saved_mapped_addr);
                        //printres((const char *)Last_test1[0]);
                        ret |= NETDIAG_NAT_SYMMETRIC;
                    }
                    else
                    {
                        vc t3 = test3(to);
                        if(t3 == no_resp)
                        {
                            printres("Port restricted NAT, server assisted calling should work");
                            printres("to most other users (some will not work because their NAT");
                            printres("routers are more restrictive.)");
                            ret |= NETDIAG_NAT_PORT_RESTRICTED;
                        }
                        else
                        {
                            printres("Address restricted NAT, server assisted calling should work");
                            printres("to most other users (some will not work because their NAT");
                            printres("routers are more restrictive.)");
                            ret |= NETDIAG_NAT_ADDRESS_RESTRICTED;
                        }
                    }
                }
            }
            else
            {
                printres("Full cone NAT, server assisted calling should work");
                printres("to most other users (some will not work because their NAT");
                printres("routers are more restrictive.)");
                ret |= NETDIAG_NAT_FULL_CONE;
            }
        }
        else if(t1 == no_resp)
        {
            printres("No UDP connectivity. Check to make sure your firewall is allowing all access to this application.");
            printres("In addition, reboot your computer, and power-off your routers, wireless access points, and other");
            printres("networking gear in order to reset them.");
            ret |= NETDIAG_NAT_NO_UDP;
        }
        else
        {
            printres("No NAT router detected.");
            vc t2 = test2(to);
            if(t2 == no_resp)
            {
                printres("UDP firewall detected, you may have problems originating and receiving calls.");
                ret |= NETDIAG_NAT_UDP_FIREWALL;
            }
            else
            {
                printres("Open Internet access, you should have no problems receiving direct calls.");
                ret |= NETDIAG_NAT_OPEN;
            }
        }
        printres("======================");
        pret[j].res = ret;
    }
    printres("Test complete.");
    return pret;
}
#endif



// updown = 0 means upload test
// updown = 1 mean download test
// if upload test is at port x, download test is
// at port x + 1
static vc
get_bw_server(int updown)
{
    DwString a;
    vc port;
    vc ip = get_random_server_ip(BW_server_list, port);
    if(ip.is_nil())
    {
        return vcnil;
    }

    a += (const char *)ip;
    a += ":";
    if(updown == 1)
        port = (int)port + 1;
    a += port.peek_str();
    return a.c_str();
}

long
simple_bw_out(DwString& res)
{
    Res = "";
    vc s(VC_SOCKET_STREAM);
    s.socket_init("any:any", 0, 0);
    s.set_err_callback(backout);
    vc dum;
    vc peer = get_bw_server(0);
    if(peer.is_nil())
        return DEFAULT_MIN_BW;
    // braindead, but probably gives a good estimate in most cases

    if(s.socket_connect(peer).is_nil())
    {
        printres("Can't connect to test server. Test failed.");
        return DEFAULT_MIN_BW;
    }

    char buf[BW_TEST_BUFSIZE];
    random_block(buf, sizeof(buf));
    double t1 = pc_ticks_to_ms();
    vc p((long)t1);
    vc pack(VC_VECTOR);
    pack[0] = p;
    pack[1] = vc(VC_BSTRING, buf, sizeof(buf));
    s.socket_send(pack, dum, peer);
    // calc approximate time for slowest case, add 5 seconds for pad
    int pr = s.socket_poll(VC_SOCK_READ, (BW_TEST_BUFSIZE * 8) / 28800 + 5, 0);
    if(!(pr & VC_SOCK_READ))
    {
        printres("No timely response from server. Test failed.");
        return DEFAULT_MIN_BW;
    }
    vc data;
    if(s.socket_recv(data, dum, peer).is_nil())
    {
        printres("Bogus response from server. Test failed.");
        return DEFAULT_MIN_BW;
    }
    double t2 = pc_ticks_to_ms();
    long ret = (sizeof(buf) * 8) / ((t2 - t1) / 1000);
    printres("Test ok");
    return ret;
}

long
simple_bw_in(DwString& res)
{
    Res = "";
    vc s(VC_SOCKET_STREAM);
    s.socket_init("any:any", 0, 0);
    s.set_err_callback(backout);
    vc dum;
    vc peer = get_bw_server(1);
    if(peer.is_nil())
        return DEFAULT_MIN_BW;
    // braindead, but probably gives a good estimate in most cases

    if(s.socket_connect(peer).is_nil())
    {
        printres("Failed to connect to test server. Test failed.");
        return DEFAULT_MIN_BW;
    }
    double t1 = pc_ticks_to_ms();
    // NOTE: BAD: assumes server has the same compiled in bufsize
    int pr = s.socket_poll(VC_SOCK_READ, (BW_TEST_BUFSIZE * 8) / 28800, 0);
    if(!(pr & VC_SOCK_READ))
    {
        printres("No timely response from server. Test failed.");
        return DEFAULT_MIN_BW;
    }
    vc data;
    if(s.socket_recv(data, dum, peer).is_nil())
    {
        printres("Bogus response from server. Test failed.");
        return DEFAULT_MIN_BW;
    }
    double t2 = pc_ticks_to_ms();
    long ret = (data.len() * 8) / ((t2 - t1) / 1000);
    printres("Test ok");
    return ret;
}

// this is intended to give us a quick read on our
// NAT situation, so we can tell clients that call us
// what to expect. useful for emiting error messages for
// symmetric NAT users.
// note: ca 2014 we don't even offer the option of UDP
// to users any more (just too problematic), so we just remove
// all this for now since it slows down app startup
int
init_netdiag()
{
#if 1
    My_connection = "no-udp";
    return 0;
#else
    DwVec<probe_result> p;
    DwString dummy;
    int num_adapters;
    extern vc STUN_server;

    p = rfc3489(STUN_server, num_adapters, dummy, 0);
    if(num_adapters == 0)
    {
        My_connection = "no-udp";
        return 0;
    }
    int res = p[0].res;
    // try to select the default adapter
    DwString dip = getLocalIp();
    for(int i = 0; i < num_adapters; ++i)
    {
        if(p[i].adapter_name == dip)
        {
            res = p[i].res;
            break;
        }
    }
    if(res & (NETDIAG_NAT_OPEN))
    {
        My_connection = "open";
    }
    else if(res & (NETDIAG_NAT_UDP_FIREWALL|NETDIAG_NAT_NO_UDP|NETDIAG_NAT_CANT_BIND))
    {
        My_connection = "no-udp";
    }
    else if(res & (NETDIAG_NAT_SYMMETRIC))
    {
        My_connection = "nat-symmetric";
    }
    else
    {
        My_connection = "nat-restricted";
    }
    return 1;
#endif
}
