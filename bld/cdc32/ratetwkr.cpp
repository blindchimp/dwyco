
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/ratetwkr.cpp 1.9 1999/01/10 16:10:02 dwight Checkpoint $
 */

#include "doinit.h"
#include "ratetwkr.h"
#include "uicfg.h"

//RateTweakerXferValid RTDefaults;
RateTweakerXferValid RTUserDefaults;

#define RATETWEAKER_SECTION "rates"
#define EXPECTED_RATE_DEFAULT "128"
#define MAX_FRAME_RATE_DEFAULT "10"
#define MAX_UDP_BYTES_DEFAULT "65535"
#define REF_INTERVAL_DEFAULT "10000"
#define LINK_SPEED_DEFAULT "128"
#define LINK_SPEED_RECV_DEFAULT "128"

#define EXPECTED_RATE "expected rate"
#define MAX_FRAME_RATE "max frame rate"
#define MAX_UDP_BYTES "max udp bytes"
#define REF_INTERVAL "ref_interval"
#define LINK_SPEED "link speed"
#define LINK_SPEED_RECV "link speed recv"

RateTweakerXfer::RateTweakerXfer()
{
    strcpy(max_frame_rate, MAX_FRAME_RATE_DEFAULT);
    strcpy(max_udp_bytes, MAX_UDP_BYTES_DEFAULT);
    strcpy(ref_interval, REF_INTERVAL_DEFAULT);
    strcpy(link_speed, LINK_SPEED_DEFAULT);
    strcpy(link_speed_recv, LINK_SPEED_RECV_DEFAULT);
}

const RateTweakerXfer&
RateTweakerXfer::operator=(const RateTweakerXfer& r)
{
    if(this == &r)
        return *this;
    strcpy(max_frame_rate, r.max_frame_rate);
    strcpy(max_udp_bytes, r.max_udp_bytes);
    strcpy(ref_interval, r.ref_interval);
    strcpy(link_speed, r.link_speed);
    strcpy(link_speed_recv, r.link_speed_recv);
    return *this;
}

void
RateTweakerXfer::save()
{
    TProfile t(RATETWEAKER_SECTION, INI_FILENAME);

    t.WriteString(MAX_FRAME_RATE, max_frame_rate);
    t.WriteString(MAX_UDP_BYTES, max_udp_bytes);
    t.WriteString(REF_INTERVAL, ref_interval);
    t.WriteString(LINK_SPEED, link_speed);
    t.WriteString(LINK_SPEED_RECV, link_speed_recv);
}

void
RateTweakerXfer::load()
{
    TProfile t(RATETWEAKER_SECTION, INI_FILENAME);

    t.GetString(MAX_FRAME_RATE, max_frame_rate, sizeof(max_frame_rate), MAX_FRAME_RATE_DEFAULT);
    t.GetString(MAX_UDP_BYTES, max_udp_bytes, sizeof(max_udp_bytes), MAX_UDP_BYTES_DEFAULT);
    t.GetString(REF_INTERVAL, ref_interval, sizeof(ref_interval), REF_INTERVAL_DEFAULT);
    t.GetString(LINK_SPEED, link_speed, sizeof(link_speed), LINK_SPEED_DEFAULT);
    t.GetString(LINK_SPEED_RECV, link_speed_recv, sizeof(link_speed_recv), LINK_SPEED_RECV_DEFAULT);

}

double
RateTweakerXferValid::get_max_frame_rate()
{
    return atof(max_frame_rate);
}
void
RateTweakerXferValid::set_max_frame_rate(double i)
{
    sprintf(max_frame_rate, "%.3f", i);
}
double
RateTweakerXferValid::get_frame_interval()
{
    double r = atof(max_frame_rate);
    if(r == 0.)
        return 86400000.; // eh, what the hell, 24hours...
    r = 1000. / r;
    if(r < 1.)
        r = 1.;
    return r;
}

void
RateTweakerXferValid::set_frame_interval(double i)
{
    i /= 1000.;
    if(i != 0.)
        i = 1. / i;
    else
        i = 1;
    if(i < 1)
        i = 1;
    else if(i > 60.)
        i = 60.;
    sprintf(max_frame_rate, "%.3f", i);
}

long
RateTweakerXferValid::get_max_udp_bytes()
{
    return atol(max_udp_bytes);
}
void
RateTweakerXferValid::set_max_udp_bytes(long i)
{
    sprintf(max_udp_bytes, "%ld", i);
}
long
RateTweakerXferValid::get_ref_interval()
{
    return atol(ref_interval);
}
void
RateTweakerXferValid::set_ref_interval(long i)
{
    sprintf(ref_interval, "%ld", i);
}
long
RateTweakerXferValid::get_link_speed()
{
    return atol(link_speed);
}
void
RateTweakerXferValid::set_link_speed(long i)
{
    sprintf(link_speed, "%ld", i);
}

long
RateTweakerXferValid::get_link_speed_recv()
{
    return atol(link_speed_recv);
}
void
RateTweakerXferValid::set_link_speed_recv(long i)
{
    sprintf(link_speed_recv, "%ld", i);
}

