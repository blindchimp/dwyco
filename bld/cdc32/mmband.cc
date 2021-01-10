
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/mmband.cc 1.5 1999/01/10 16:09:51 dwight Checkpoint $
 */
#include "mmchan.h"
#include "ratetwkr.h"
#include "netvid.h"
#include "dwrtlog.h"
#include "dwscoped.h"
#include "ezset.h"

using namespace dwyco;

#define MAXBW (1<<30)
//
// each channel has 2 bandwidth limits, and 2
// bandwidth current use variables. one each for
// incoming and outgoing.
// limits are negotiated once at channel setup, and
// cannot be changed.
// current use are changed dynamically.
//
// to determine our throttle settings (outgoing throttles)
// calculate all the amount of bandwidth needed by
// clients of the system, and divide evenly.
//
// to determine our incoming throttle settings (ie, limits
// for remote systems) take our set upper limit on
// incoming bandwidth, divide by the number of incoming
// connections, and send that limit to the remote end
// so their throttles can be updated.

int
MMChannel::adjust_outgoing_bandwidth()
{
    scoped_ptr<ChanList> cl(MMChannel::get_serviced_channels_net());
    ChanListIter cli(cl.get());
    int cnt = 0;
    long total_needed = 0;
    long min_bw = MAXBW;
    long naudio = 0;

    if(cl->num_elems() == 0)
        return 0;
    // this is the calculation for our OUTGOING
    // throttles
    for(cli.rewind(); !cli.eol(); cli.forward())
    {
        // sum negotiated bandwidths
        MMChannel *mc = cli.getp();
        if(mc->msg_chan || mc->user_control_chan || mc->server_channel || mc->secondary_server_channel)
            continue;
        if(!mc->rem_incoming_throttle.valid())
            continue;
        if(mc->proxy_outgoing_bw_limit != -1 &&
                mc->proxy_outgoing_bw_limit < (int)mc->rem_incoming_throttle)
        {
            total_needed += mc->proxy_outgoing_bw_limit;
        }
        else
        {
            total_needed += (int)mc->rem_incoming_throttle;
        }

        // this only counts if it is a video channel
        if(mc->grab_coded_id != -1)
        {
            if(mc->proxy_outgoing_bw_limit == -1)
            {
                if((int)mc->rem_incoming_throttle < min_bw)
                    min_bw = (int)mc->rem_incoming_throttle;
            }
            else
            {
                if(mc->proxy_outgoing_bw_limit < min_bw)
                    min_bw = mc->proxy_outgoing_bw_limit;
            }
            ++cnt;
        }
        // if the channel is audio, and it has something
        // currently pumping out, then count it as a full
        // fixed rate output channel.
        naudio += (mc->grab_audio_id != -1 ? (!mc->tube->buf_drained(AUDIO_CHANNEL_V2) *
                                              (mc->tube->get_est_baud(AUDIO_CHANNEL_V2) / 1000)) : 0);
    }
    GRTLOG("min %d cnt %d", min_bw, cnt);
    if(min_bw == 0 || min_bw == MAXBW)
        min_bw = 1;
    long avail = get_available_output_bandwidth();
    // reduce the available bandwidth by the amount of
    // fixed reserved usage (ie, audio is always a fixed rate.)
    avail -= naudio;
    if(avail <= 0)
    {
        avail = cnt; // leave at least 1kbps for each video
        if(avail == 0)
            avail = min_bw;
    }
    GRTLOG("naudio %d avail %d", (int)naudio, (int)avail);
    int sr;
    if(Auto_sync)
        sr = compute_sync();
    else
        sr = Sync_receivers;
    if(sr)
    {
        // if we are synced, the slowest client is what matters
        total_needed = cnt * min_bw;
        GRTLOG("needed %d", total_needed, 0);
        if(total_needed <= avail)
        {
            // see if we need to adjust
            if(avail / min_bw >= cnt)
            {
                // we can boost the throttle back up
                if(cnt == 0)
                    min_bw = avail;
                else
                    min_bw = avail / cnt;
                for(cli.rewind(); !cli.eol(); cli.forward())
                {
                    MMChannel *mc = cli.getp();
                    // only adjust video channels
                    if(!mc->rem_incoming_throttle.valid())
                        continue;
                    if(mc->grab_coded_id != -1)
                    {
                        if(min_bw <= (int)mc->rem_incoming_throttle)
                            mc->adjust_outgoing_throttle(min_bw);
                        else
                            mc->adjust_outgoing_throttle((int)mc->rem_incoming_throttle);
                    }
                }
                return 1;
            }
            return 0;
        }
        // more needed than available must knock down
        // everybody to fit in.
        if(cnt == 0)
            min_bw = avail;
        else
            min_bw = avail / cnt;
        for(cli.rewind(); !cli.eol(); cli.forward())
        {
            MMChannel *mc = cli.getp();
            if(mc->msg_chan || mc->user_control_chan)
                continue;
            if(mc->grab_coded_id != -1)
            {
                if(!mc->rem_incoming_throttle.valid())
                    continue;
                if(min_bw <= (int)mc->rem_incoming_throttle)
                    mc->adjust_outgoing_throttle(min_bw);
                else
                    mc->adjust_outgoing_throttle((int)mc->rem_incoming_throttle);
            }
        }
        return 1;
    }

    // non-synchronized case

    if(total_needed <= avail)
    {
        // change all bandwidths back to their requested
        // values.
        for(cli.rewind(); !cli.eol(); cli.forward())
        {
            MMChannel *mc = cli.getp();
            if(mc->msg_chan || mc->user_control_chan)
                continue;
            if(!mc->rem_incoming_throttle.valid())
                continue;
            if(mc->grab_coded_id != -1)
                mc->adjust_outgoing_throttle((int)mc->rem_incoming_throttle);
        }
        return 1;
    }
    // too much requested, have to allocate and adjust
    // each channel to get us under the bandwidth requirements.

    // some allocation ideas:
    // we'll take a small, equal amount off each channel
    // to try to get under the limit. this is essentially
    // "share the deficit" allocation, which means that
    // low speed connections are hit harder than high
    // speed connections.
    // another way to do it would be
    // to just cut the bandwidth into equal sections and
    // allocate low bandwidth connections first, then
    // give the remaining to high bandwidth...
    // also can reduce each connections' bandwidth by
    // equal percent. this gives slight preference to low
    // bandwidth connections.
    //

    // this is equal % method
    double k = (double)avail / total_needed;

    for(cli.rewind(); !cli.eol(); cli.forward())
    {
        MMChannel *mc = cli.getp();
        if(mc->msg_chan || mc->user_control_chan)
            continue;
        if(!mc->rem_incoming_throttle.valid())
            continue;
        if(mc->grab_coded_id != -1)
            mc->adjust_outgoing_throttle((int)mc->rem_incoming_throttle * k);
    }
    return 1;
}

int
MMChannel::adjust_incoming_bandwidth()
{
    scoped_ptr<ChanList> cl(MMChannel::get_serviced_channels_net());
    ChanListIter cli(cl.get());
    int cnt = 0;

    if(cl->num_elems() == 0)
        return 0;

    // this is the calculation for REMOTE SYSTEMS
    // throttles. We have N clients, and X bandwidth
    // incoming, just give everyone an equal share.
    // except for channels that are just chat or send only, in which
    // case they don't count.
    for(cli.rewind(); !cli.eol(); cli.forward())
    {
        MMChannel *mc = cli.getp();
        if(mc->msg_chan || mc->user_control_chan)
            continue;
        if(mc->decoder || mc->audio_output)
            ++cnt;
    }
    if(cnt == 0)
        cnt = 1;
    long avail = get_available_input_bandwidth();
    long client_bw = avail / cnt;
    if(client_bw == 0)
        client_bw = 1;
    cnt = 0;
    for(cli.rewind(); !cli.eol(); cli.forward())
    {
        MMChannel *mc = cli.getp();
        if(mc->msg_chan || mc->user_control_chan)
            continue;
        // this looks a little funny, but the idea is that
        // if a channel is sending us proxied media (from
        // one of our servers), we limit the incoming
        // bandwidth, to in effect, tell the other side to
        // limit its outgoing b/w. we could do this at aux channel
        // set up time, but that requires us to know what
        // exactly the channels are going to be used for
        // and make a lot of other assumptions. doing it here,
        // we know the situation (the only assumption is that
        // if we are sending via proxy, then they are sending
        // back via proxy too, which might change in the future.)
        //
        if(mc->proxy_outgoing_bw_limit != -1 &&
                mc->proxy_outgoing_bw_limit < client_bw)
        {
            mc->incoming_throttle = mc->proxy_outgoing_bw_limit;
            GRTLOG("force incoming to %d", mc->myid, mc->proxy_outgoing_bw_limit);
        }
        else
        {
            GRTLOG("incoming on %d to %d", mc->myid, client_bw);
            mc->incoming_throttle = (int)client_bw;
        }
        ++cnt;
    }
    return 1;
}

int
MMChannel::get_available_output_bandwidth()
{
    return get_settings_value("rate/kbits_per_sec_out");
}

int
MMChannel::get_available_input_bandwidth()
{
    return get_settings_value("rate/kbits_per_sec_in");
}

int
MMChannel::adjust_outgoing_throttle(int speed)
{
    if(tube)
    {
        if(proxy_outgoing_bw_limit != -1)
            if(speed > proxy_outgoing_bw_limit)
                speed = proxy_outgoing_bw_limit;
        GRTLOG("outgoing on %d to %d", myid, speed);
        tube->set_est_baud(speed * 1000);
    }
    return 1;

}
