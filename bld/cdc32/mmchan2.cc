
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/mmchan2.cc 1.17 1999/01/10 16:09:36 dwight Checkpoint $
#include <sys/stat.h>
#include "mmchan.h"
#include "netvid.h"

#include "gvchild.h"
#include "senc.h"
#include "qauth.h"
#include "zapadv.h"
#include "qmsg.h"
#include "snds.h"
#include "dwrtlog.h"
#include "codec.h"
#include "fnmod.h"
#include "se.h"
#include "dwscoped.h"
using namespace dwyco;

#ifdef LINUX
#include "miscemu.h"
#endif
#include "sepstr.h"

DwString
MMChannel::name_id()
{
    vc u;
    vc name;
    if(!remote_cfg.is_nil() && !remote_cfg.find("my uid", u) && u.type() != VC_STRING &&
            u.len() != 10)
        u = "0000000000";
    u = to_hex(u);
    DwString chop((const char *)u);
    chop.erase(8);
    if(!remote_cfg.is_nil() && !remote_cfg.find("username", name))
        name = "?";
    DwString ret((const char *)name);
    ret += "(#";
    ret += chop;
    ret += ")";
    return ret;
}

void
MMChannel::drop_subchannel(int i)
{
    if(!tube)
        return;
    if(i == audio_chan)
        audio_state = MEDIA_ERR;
    if(i == video_chan)
        video_state = MEDIA_ERR;
    if(i == msync_chan)
        msync_state = MEDIA_ERR;
    tube->drop_channel(i);
}

void
MMChannel::set_progress_abort(const DwString& why)
{
    DwString a("Failed: ");
    a += why;
    set_progress_status(a);
}

void
MMChannel::set_progress_got(int len)
{
    char a[4096];
    if(expected_size == 0)
        expected_size = 1;

    if(total_got != expected_size)
    {
        int old_pct = (int)(((double)total_got * 100) / expected_size);
        int new_pct = (int)(((double)(total_got + len) * 100) / expected_size);
        total_got += len;
        if(old_pct / 10 == new_pct / 10)
            return;
    }

    if(total_got < 1024)
    {
        sprintf(a, "Xfer: %d bytes", total_got);
    }
    else if(total_got < 1024 * 1024)
    {
        sprintf(a, "Xfer: %dK", total_got / 1024);
    }
    else
    {
        sprintf(a, "Xfer: %4.2fM", (double)total_got / (1024 * 1024));
    }

    int p = (int)(((double)total_got * 100) / expected_size);
    set_progress_status(a, p);
}

void
MMChannel::set_progress_done(const DwString& how)
{
    DwString a("End: ");
    a += how;
    set_progress_status(a);
}

void
MMChannel::set_progress_status(const DwString& s, int barval)
{
    if(status_callback)
    {
        (*status_callback)(this, s.c_str(), scb_arg1, scb_arg2);
    }
    //progress_signal.emit(s, barval);
}


int
MMChannel::handle_channels()
{
    // scan thru all reliable channels
    // and crank protocols as necessary

    if(!tube)
        return 1;
    int i;
    for(i = 0; i < simple_protos.num_elems(); ++i)
    {
        if(simple_protos[i])
        {
            simple_protos[i]->crank();

        }
    }

    return 1;
}

int
MMChannel::send_reliable_audio(DWBYTE *buf, int len)
{

    vc sv(VC_VECTOR);
    sv[0] = vc(VC_BSTRING, (const char *)buf, len);
    sv[1] = (int)audio_timecode;
    sv[2] = (int)current_payload_type;
    int err;
    if((err = tube->send_data(sv, audio_chan, MMTube::WB_OK)) == SSERR)
    {
        drop_subchannel(audio_chan);
        audio_chan = -1;
        force_unreliable_audio = 1;
        return 0;
    }
    else if(err == SSTRYAGAIN)
    {
        // q it for another try
        audioq.add(buf, len, audio_timecode, last_audio_index, current_payload_type);
        return 0;
    }
    else // worked
    {

    }
    return 1;

}

int
MMChannel::attempt_audio_q_send(int& out_len)
{
    if(audioq.num_elems() == 0)
    {
        out_len = 0;
        return 1;
    }

    DWBYTE *buf;
    int len;
    int timecode;
    int aindex;
    int payload;
    if(!audioq.fifo_buf(buf, len, timecode, aindex, payload))
        oopanic("bad fifo buf");
    vc sv(VC_VECTOR);
    sv[0] = vc(VC_BSTRING, (const char *)buf, len);
    sv[1] = timecode;
    sv[2] = payload;

    int err;

    if((err = tube->send_data(sv, audio_chan, MMTube::WB_OK)) == SSERR)
    {
        drop_subchannel(audio_chan);
        audio_chan = -1;
        force_unreliable_audio = 1;
        return 0;
    }
    else if(err == SSTRYAGAIN)
    {
        return 0;
    }
    else // worked
    {
        // for testing, assume no other xmitters
        audioq.remove();
        // note: this calculation isn't exactly right, though send_data
        // could give it to us exactly... maybe eventually.
        out_len = len + sizeof(audio_timecode) + sizeof(current_payload_type);
        ++last_audio_index;
    }
    return 1;

}

int
MMChannel::send_reliable_video_proxy()
{
    video_output_device_blocked = 0;
    // note: for the purposes of throttling, we
    // assume we are using the VIDEO_CHANNEL instead
    // of the value of "video_chan". this is because
    // the tube as a whole is doing the software
    // throttling, and not individual channels
    // within the tube. this is definitely whacky
    // and needs to be fixed. part of this is because
    // the automatic b/w allocator doesn't know
    // enough about what channels did what, so it is
    // only adjusting at the tube level.
    if(!tube->buf_drained(VIDEO_CHANNEL))
    {
        video_output_device_blocked = 1;
        return 0;
    }
    // here we use "video_chan" because we actually
    // want to know if the device can accept more
    // data.
    if(!tube->can_write_data(video_chan))
    {
        video_output_device_blocked = 1;
        return 0;
    }

    MMChannel *bcast = channel_by_id(grab_coded_id);
    GRTLOG("blast %d ourlast %d", (int)bcast->last_time_index, (int)last_time_index);
    if(bcast->last_coded_buf && bcast->last_time_index != last_time_index)
    {
        GRTLOG("reliable proxy output", 0, 0);
        if(!(int)pause_video && !(int)rem_pause_video)
        {
            fps_send.add_units(1);
            if(vid_make_first_0)
            {
                vid_internal_0_offset = -bcast->last_acq_time;
                vid_make_first_0 = 0;
            }
            tube->set_acquisition_time(vid_internal_0_offset + bcast->last_acq_time);
            int err;
            // note: we send on device "video_chan", but tally the b/w to VIDEO_CHANNEL
            // for backward compatibility with the b/w allocator and stuff.

            vc vcx(VC_BSTRING, bcast->last_coded_buf, bcast->len_last_coded_buf);
            if((err = tube->send_data(vcx, video_chan, VIDEO_CHANNEL)) == SSERR)
            {
                GRTLOG("reliable proxy output error", 0, 0);
                force_unreliable_video = 1;
                drop_subchannel(video_chan);
                video_chan = -1;
                return 0;
            }
            else if(err == SSTRYAGAIN)
            {
                GRTLOG("reliable proxy output blocked", 0, 0);
                video_output_device_blocked = 1;
                return 0;
            }
        }

        sproto *s = simple_protos[video_chan];
        s->timeout.load(VIDEO_IDLE_TIMEOUT);
        s->timeout.start();
        video_output_device_blocked = 0;
        bps_send.add_units((long)bcast->len_last_coded_buf * 8);
        GRTLOG("reliable proxy output ok", 0, 0);

        last_time_index = bcast->last_time_index;
        return 1;
    }
    return 0;
}

int
MMChannel::process_incoming_reliable_video()
{
    // some frame info waiting to be processed
    int ret;

    vc rvc;
    int len;
    if((ret = tube->recv_data(rvc, video_chan)) >= 0)
    {
        if(rvc.type() == VC_STRING)
        {
            len = rvc.len();
            if(!decoder)
                build_incoming_video();
            if(decoder && !decoder->busy())
            {
                decoder->decode_postprocess((DWBYTE *)(const char *)rvc, len);
                fps_recv.add_units(1);
                bps_recv.add_units(len * 8);
                if(!pause && rate_updated(RECEIVER))
                {
                    rate_update &= ~RECEIVER;
                    decoder->display_info(info_format(RECEIVER).c_str());
                }

                return 1;
            }
        }
        else if(rvc.type() == VC_VECTOR)
        {
            // drop pings
            if(rvc[0] == vc("!"))
                return 1;
        }
        else
            ret = SSERR;
    }

    if(ret == SSERR)
    {
        force_unreliable_video = 1;
        drop_subchannel(video_chan);
        video_chan = -1;
    }
    return 0;
}

void
MMChannel::pause_all_channels(int p)
{
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m) continue;
        if(m->do_destroy == KEEP
                && m->tube && !m->msg_chan && !m->user_control_chan && !m->server_channel &&
                m->tube->is_network()
          )
        {
            m->pause_video = p;
            m->pause_audio = p;
        }
    }
}


int
MMChannel::selective_chat_recipient_enable(vc uid, int enable)
{

    MMChannel *m = 0;

    scoped_ptr<ChanList> cl(get_serviced_channels_net());
    ChanListIter cli(cl.get());
    for(; !cli.eol(); cli.forward())
    {
        MMChannel *mc = cli.getp();
        if(mc->remote_uid() == uid &&
                (mc->kaq || (mc->grab_chat_id != -1 && channel_by_id(mc->grab_chat_id))))
        {
            m = mc;
            break;
        }
    }

    if(!m)
        return 0;

    MMChannel *xm = MMChannel::find_msg_xmitter();
    if(!xm)
    {
        return 0;
    }

    if(enable)
    {
        // avoid spitting out last message to
        // new private chatter
        m->last_msg_index = xm->last_msg_index;
        m->selective_chat_output = 1;
    }
    else
        m->selective_chat_output = 0;
    return 1;
}

//---------------------------------------------------------------------------


int
MMChannel::selective_chat_enable(int e)
{
    MMChannel *xm = MMChannel::find_msg_xmitter();
    if(!xm)
    {
        return 0;
    }
    xm->selective_chat_mode = !!e;
    if(!e)
    {
        scoped_ptr<ChanList> cl(get_serviced_channels());
        ChanListIter cli(cl.get());
        for(; !cli.eol(); cli.forward())
        {
            MMChannel *mc = cli.getp();
            // do this to avoid spitting out last private message
            // to everyone
            mc->last_msg_index = xm->last_msg_index;
        }

    }
    return 1;
}


void
MMChannel::reset_selective_chat_recipients()
{
    MMChannel *xm = MMChannel::find_msg_xmitter();
    scoped_ptr<ChanList> cl(get_serviced_channels());
    ChanListIter cli(cl.get());
    for(; !cli.eol(); cli.forward())
    {
        MMChannel *mc = cli.getp();
        mc->selective_chat_output = 0;
        mc->selective_chat_mode = 0;
        // do this to avoid spitting out last private message
        // to everyone
        if(xm)
            mc->last_msg_index = xm->last_msg_index;
    }
}

int
MMChannel::is_selective_chat_recipient(vc uid)
{
    scoped_ptr<ChanList> cl(get_serviced_channels_net());
    ChanListIter cli(cl.get());
    for(; !cli.eol(); cli.forward())
    {
        MMChannel *mc = cli.getp();
        if(mc->remote_uid() == uid &&
                (mc->kaq || (mc->grab_chat_id != -1 && channel_by_id(mc->grab_chat_id))))
        {
            return 1;
        }
    }
    return 0;
}
