
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/mmbld.cc 1.16 1999/01/10 16:09:44 dwight Checkpoint $

#include <windows.h>
#include "mmchan.h"
#include "aq.h"
#include "codec.h"

#ifndef LINUX
#include "aqextaud.h"
#else
#include "aqaud.h"
#endif
#include "audout.h"
#include "audo.h"
#include "audi.h"
#include "audchk.h"
#include "audth.h"
#include "audmixs.h"
#include "audmix.h"
//#include "lpcconv.h"
//#include "rawconv.h"
#include "gsmconv.h"
#include "spxconv.h"
#include "vorbconv.h"
#include "gvchild.h"
#include "netvid.h"
#include "doinit.h"
#include "dwrtlog.h"
#include "ezset.h"

namespace dwyco {
extern double Audio_delay;
}
using namespace dwyco;

#define FAILRET(x) do {fail_reason = (x); Log->make_entry(x); return 0;} while(0)

int
MMChannel::audio_decoder_from_config()
{
    vc ac;

    // check to see that we at least have something in common
    if(!common_cfg.find("audio codec", ac) ||
            ac.num_elems() == 0)
    {
        FAILRET("no audio codecs in common.");
    }

    // since the xmitter can send anything our way,
    // be prepared to accept any sort of payload
    // by creating all the possible decoders

    for(int i = 0; i < PAYLOAD_NUM; ++i)
    {
        delete audio_decoders[i];
        audio_decoders[i] = 0;
    }
#ifndef DWYCO_NO_GSM
    audio_decoders[PAYLOAD_MSGSM610] = new GSMConverter(1);
#else
    audio_decoders[PAYLOAD_MSGSM610] = 0;
#endif
    audio_decoders[PAYLOAD_RAW8KHZ] =  0; //new RawConverter(1);
    // default, narrow band
#ifdef DWYCO_USE_SPEEX
    audio_decoders[PAYLOAD_SPEEX] = new SpeexConverter(1, 8000, 8000);
    audio_decoders[PAYLOAD_UWB_SPEEX] = new SpeexConverter(1, 16000, UWB_SAMPLE_RATE);
#else
    audio_decoders[PAYLOAD_SPEEX] = 0;
    audio_decoders[PAYLOAD_UWB_SPEEX] = 0;
#endif
#ifndef DWYCO_NO_VORBIS
#ifdef UWB_SAMPLING
    audio_decoders[PAYLOAD_VORBIS] = new VorbisConverter(1, 32000, UWB_SAMPLE_RATE);
#else
    audio_decoders[PAYLOAD_VORBIS] = 0;
#endif
    audio_decoders[PAYLOAD_VORBIS8KHZ] = new VorbisConverter(1, 32000, 8000);
#else
    audio_decoders[PAYLOAD_VORBIS] = 0;
    audio_decoders[PAYLOAD_VORBIS8KHZ] = 0;
#endif
    return 1;
}

int
MMChannel::str_to_payload_type(vc str)
{
    static vc ms_gsm610("ms-gsm610");
    static vc speex("speex");
    static vc uwb_speex("uwb-speex");
    static vc vorbis("vorbis");
    static vc raw8khz("raw8khz");
    static vc vorbis8khz("vorbis8khz");

    if(str == ms_gsm610)
        return PAYLOAD_MSGSM610;
    if(str == speex)
        return PAYLOAD_SPEEX;
    if(str == uwb_speex)
        return PAYLOAD_UWB_SPEEX;
    if(str == vorbis)
        return PAYLOAD_VORBIS;
    if(str == raw8khz)
        return PAYLOAD_RAW8KHZ;
    if(str == vorbis8khz)
        return PAYLOAD_VORBIS8KHZ;
    // something simple, probably won't work but better than crashing
    return PAYLOAD_RAW8KHZ;
    return 0;
}

int
MMChannel::audio_coder_from_config()
{
    vc rq, ac;

    // CHECK FOR OLD RECEIVER AND SET SAMPLING to 200ms
    //
    if(!remote_cfg.find("requested config", rq) ||
            !rq.find("audio codec", ac))
    {
        FAILRET("no requested coder?");
    }
    if(ac.contains("ms-gsm610"))
    {
        current_payload_type = PAYLOAD_MSGSM610;
    }
    else if(ac.contains("raw-8khz"))
    {
        current_payload_type = PAYLOAD_RAW8KHZ;
    }
    else if(ac.contains("lpc4800"))
    {
        current_payload_type = PAYLOAD_LPC4800;
    }
    else if(ac.contains("speex"))
    {
        current_payload_type = PAYLOAD_SPEEX;
    }
    else if(ac.contains("uwb-speex"))
    {
        current_payload_type = PAYLOAD_UWB_SPEEX;
    }
    else if (ac.contains("vorbis"))
    {
        current_payload_type = PAYLOAD_VORBIS;
    }
    else if (ac.contains("vorbis8khz"))
    {
        current_payload_type = PAYLOAD_VORBIS8KHZ;
    }
    else
    {
        FAILRET("unknown audio codec");
    }
    for(int i = 0; i < PAYLOAD_NUM; ++i)
    {
        delete audio_coders[i];
        audio_coders[i] = 0;
    }
#ifndef DWYCO_NO_GSM
    audio_coders[PAYLOAD_MSGSM610] = new GSMConverter(0);
#else
    audio_coders[PAYLOAD_MSGSM610] = 0;
#endif
    audio_coders[PAYLOAD_RAW8KHZ] = 0; //new RawConverter(0);
    // default, narrow band
#ifdef DWYCO_USE_SPEEX
    audio_coders[PAYLOAD_SPEEX] = new SpeexConverter(0, 8000, 8000);
    audio_coders[PAYLOAD_UWB_SPEEX] = new SpeexConverter(0, 32000, UWB_SAMPLE_RATE, 7);
#else
    audio_coders[PAYLOAD_SPEEX] = 0;
    audio_coders[PAYLOAD_UWB_SPEEX] = 0;
#endif
#ifndef DWYCO_NO_VORBIS
#ifdef UWB_SAMPLING
    audio_coders[PAYLOAD_VORBIS] = new VorbisConverter(0, 32000, UWB_SAMPLE_RATE);
#else
    audio_coders[PAYLOAD_VORBIS] = 0;
#endif
    audio_coders[PAYLOAD_VORBIS8KHZ] = new VorbisConverter(0, 32000, 8000);
#else
    audio_coders[PAYLOAD_VORBIS] = 0;
    audio_coders[PAYLOAD_VORBIS8KHZ] = 0;
#endif
    return 1;
}

int
MMChannel::build_incoming_audio(int locally_invoked)
{
    MMChannel *mcx;
    if(!(mcx = find_audio_player()))
    {
        if(!TheAudioOutput)
        {
            if(!init_audio_output(locally_invoked))
                FAILRET("can't setup audio output device");
        }
        // create mixer object too
        if(!init_audio_mixer())
        {
            exit_audio_output();
            FAILRET("can't create mixer");
        }
        // SETUP PROXY for OUTPUT
        mcx = new MMChannel;
        //mcx->remote_cfg = remote_cfg;
        mcx->start_service();
        mcx->audio_output = TheAudioOutput;
        GRTLOG("audoutput %p set to theaudiooutput %p", mcx, TheAudioOutput);

        mcx->set_string_id("<<aud output>>");
    }
    audio_output = new AudioMixSend(TheAudioMixer, 5, AUDBUF_LEN);
    int bufs = (int)((1000 * Audio_delay) /
                     audio_output->device_one_buffer_time());
    if(bufs >= 0 && bufs <= 200)
        audio_output->change_buffering(bufs);

    audio_decoder_from_config();
    int cnt = 0;
    vc v(VC_VECTOR);
    for(int i = 0; i < PAYLOAD_NUM; ++i)
    {
        if(!audio_decoders[i] || !audio_decoders[i]->init())
        {
            audio_decoders[i] = 0;
        }
        else
        {
            ++cnt;
            v.append(i);
        }
    }

    available_audio_decoders = v;

    if(cnt >= 1)
    {
        if(current_payload_type != -1 && audio_decoders[current_payload_type] == 0)
        {
            FAILRET("can't setup requested audio decoder");
        }
        // this level of back and forth negotiation is more complicated
        // than we needed. autoupdate keeps clients close enough that
        // this isn't really necessary
#if 0
        // couldn't init the one we want, so select something
        // else, and make sure the other side gets a control
        // message indicating it should send that.
        int cnt = 0;
        int i;
        for(i = 0; i < PAYLOAD_NUM; ++i)
        {
            if(audio_decoders[i] == 0)
                continue;
            // XXX NEED TO INCLUDE BITRATE IN DECISION ON
            // WHETHER TO USE THE PAYLOAD.
            current_payload_type = i;
            break;
        }
        if(i == PAYLOAD_NUM)
        {
            FAILRET("can't setup any audio decoders");
        }
    }
#endif

}
else
{
    FAILRET("can't create any audio decoders");
}
if(gv_id == 0)
    gv_id = gen_new_grayview(myid);
return 1;
}

int
MMChannel::build_outgoing_audio(int locally_invoked)
{
    MMChannel *mcx;
    // channel setup for audio is different than
    // video. for audio, we have a sampler that can
    // provide raw samples to a number of different
    // encoders. the encoders then send the audio
    // out to peers. this is differnt than video
    // because with video the coder is associated with
    // the sampler, and everyone gets the same video
    // stream because it is too cpu intensive to
    // have separate coders for each recipient.
    //
    if(!(mcx = find_audio_xmitter()))
    {
        if(!TheAudioInput)
        {
            if(!init_audio_input(locally_invoked, 0))
                FAILRET("can't setup audio input device");
            //dangerous
            if(TheAudioMixer)
                TheAudioMixer->set_audinp(TheAudioInput);
        }
        // PUT IN AUDIO PROXY OBJECT CODE HERE
        mcx = new MMChannel;
        mcx->remote_cfg = remote_cfg;
        mcx->start_service();
        mcx->audio_sampler = TheAudioInput;
#if 0
// when we get a better audio processor, redo this
        mcx->preprocess_w_vad.do_agc = CTUserDefaults.get_agc();
        mcx->preprocess_w_vad.do_noise_reduction = CTUserDefaults.get_denoise();
#endif

        mcx->set_string_id("<<aud source>>");
    }
    audio_coder_from_config();
    int cnt = 0;
    vc v(VC_VECTOR);
    for(int i = 0; i < PAYLOAD_NUM; ++i)
    {
        if(audio_coders[i])
        {
            if(!audio_coders[i]->init())
            {
                delete audio_coders[i];
                audio_coders[i] = 0;
            }
            else
            {
                ++cnt;
                v.append(i);
            }
        }
    }
    available_audio_coders = v;
    // end of story, if the receiver wants us to
    // change coders, we'll do that when we receive
    // a message. NOTE: must CHANGE CAPABILTIES TO
    // INCLUDE A TEST OF WHETHER WE CAN USE MSGSM,
    // SINCE IT IS THE THING THAT IS MISSING A LOT OF
    // THE TIME.

    grab_audio_id = mcx->myid;
    last_audio_index = mcx->last_audio_index;
    audio_timecode = mcx->audio_timecode;
    if(gv_id == 0)
        gv_id = gen_new_grayview(myid);
    tube->set_est_baud(14000, AUDIO_CHANNEL_V2);
    tube->clear_buf_ctrl(AUDIO_CHANNEL_V2);

    return 1;
}

int
MMChannel::build_outgoing_chat(int locally_invoked)
{
    MMChannel *mcx;
    MMChannel *cmaster = find_chat_master();
    if(!(mcx = find_msg_xmitter()))
    {
        if(cmaster)
            mcx = cmaster;
        else
            mcx = this;
        if(!TheMsgAq)
        {
            DwString fail_reason;
            if(!init_msgaq(locally_invoked, fail_reason))
                FAILRET(fail_reason.c_str());
        }
        mcx->kaq = TheMsgAq;
        mcx->chat_master = 1;
        mcx->chatbox_id = Chatbox_id;
        mcx->chat_proxy = 0;
    }
    else
    {
        grab_chat_id = mcx->myid;
        chatbox_id = Chatbox_id;
        chat_proxy = 1;
        // need to do this to avoid spitting out
        // the last thing transmitted by the
        // chat master.
        last_msg_index = mcx->last_msg_index;
    }
    return 1;
}

ChatDisplay *
MMChannel::gen_public_chat_display()
{
    return (*gen_public_chat_display_callback)(this);
}

ChatDisplay *
MMChannel::gen_private_chat_display()
{
    return (*gen_private_chat_display_callback)(this);
}

int
MMChannel::build_outgoing_chat_private(int locally_invoked)
{
    DwString fail_reason;
    private_kaq = init_msgaq_private(locally_invoked, private_chatbox_id, "", myid, fail_reason);
    if(!private_kaq)
        FAILRET(fail_reason.c_str());
    private_chat_display = gen_private_chat_display();
    return 1;
}

int
MMChannel::build_incoming_video()
{
    decoder = decoder_from_config();
    if(!decoder)
    {
        return 0;
    }
    gv_id = decoder->gv_id = gen_new_grayview(myid);
#if 0
    decoder->decode_color = CTUserDefaults.get_decode_color();

    if(!reg_is_registered() && Color_off)
    {
        decoder->decode_color = 0;
        decoder->force_gray = 1;
    }
#endif
    return 1;
}

int
MMChannel::build_outgoing(int locally_invoked, int inhibit_coder_display, int max_fps)
{
    MMChannel *mcx;
    // find the codec/sampler object, even if
    // it has been scheduled for destroy
    GRTLOG("build outgoing", 0, 0);
    if(!(mcx = find_xmitter(0)))
    {
        GRTLOG("no xmitter", 0, 0);
        if(!TheAq)
        {
            GRTLOG("init aq", 0, 0);
            DwString fail_reason;
            if(!initaq(locally_invoked, fail_reason))
                FAILRET(fail_reason.c_str());
        }
        mcx = new MMChannel;
        // clone just enough config info that we can
        // determine what kind of decoder to build
        // from the coder
        mcx->config = config;
        mcx->remote_cfg = remote_cfg;
        mcx->start_service();
        mcx->sampler = TheAq;
        mcx->coder = coder_from_config();
        if(!mcx->coder)
        {
            delete mcx;
            FAILRET("can't create coder (not enough memory or incompatible)");
        }
        // always generate a new gv for the coder,
        // don't want it merged with anything else.
        if(!inhibit_coder_display)
        {
            mcx->gv_id = MMCHAN_PREVIEW_CHAN_ID;
            mcx->coder->gv_id = MMCHAN_PREVIEW_CHAN_ID;
        }

        mcx->set_string_id("<<vid source>>");

        // set up initial data rates, etc.
        //RateTweakerXferValid& rt = RTUserDefaults;
        // this may be an artifact of the old UDP stuff where
        // the reference frame was too big, and some routers
        // would drop it
//			if(Sync_receivers)
//			{
//				// hack, send a reference frame 1
//				// second after connection starts
//				// up... seems like initial packet
//				// is getting lost or sometime...
//				mcx->ref_timer.load(1000);
//			}
        //long r = (long)rt.get_frame_interval();
        int r = get_settings_value("rate/max_fps");
        double intval = 1. / r;
        intval *= 1000;
        mcx->frame_timer.set_interval(intval);
        mcx->frame_timer.reset();
        mcx->frame_timer.start();
        mcx->ready_for_ref = 1;
    }

    // should partition bandwidth, since
    // this is another transmitter...
    grab_coded_id = mcx->myid;
    // new connection, force transmitter to produce
    //a reference frame
    mcx->ready_for_ref = 1;
    frame_timer.stop();
    //RateTweakerXferValid& rt = RTUserDefaults;

    // this is just to get a reasonable default if for
    // some reason negotiation doesn't come up with one.
    vc bw;
    if(!common_cfg.find("channel bandwidth", bw))
        bw = get_settings_value("rate/kbits_per_sec_out");

    tube->set_est_baud((long)bw * 1000);
    tube->clear_buf_ctrl();
    tube->set_keepalive(0);
    // do this to avoid outputing the frame currently
    // queued for sending (we want the next one...)
    last_time_index = mcx->last_time_index;

    return 1;
}

// this should only be enabled on the receive end of a network
// channel, as that is the only place that would have information
// regarding how many packets had been dropped. and then, only on
// unreliable channels.
void
MMChannel::enable_packet_drop_reporting(int e)
{
    if(e)
        drop_timer.start();
    else
    {
        drop_timer.stop();
        drop_send = 0;
    }
}

