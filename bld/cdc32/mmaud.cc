
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/mmaud.cc 1.15 1999/01/10 16:09:45 dwight Checkpoint $
#include <windows.h>
#include "mmchan.h"
#include "dwrtlog.h"
#include "netvid.h"
#include "audconv.h"
#include "genaqaud.h"
#include "audout.h"
#include "audo.h"
#include "audi.h"
#include "audchk.h"
#include "snds.h"
//#include "lpcconv.h"
#include "audth.h"
#include "audmix.h"
#include "chatq.h"

extern int Auto_squelch;
extern CRITICAL_SECTION Audio_lock;

// in half-duplex mode, set this to 1 to stop
// audio output and turn on audio acquisition.
// it is expected that this would be used when the
// user clicked a button to start talking
int Talk_override;

// set to 1 to stop audio acquisition
int All_mute;

extern vc My_UID;

int inet_sim();
void set_audio_status(int);

extern "C" {
    void
    log_lpc(double p, double g, double *k, int len)
    {
        GRTLOGF("p %6.3f g %6.3f vuv %2.0f k0 %6.3f k1 %6.3f k2 %6.3f",
                p, g, (double)len, k[0], k[1], k[2]);
    }
};

void set_squelch(int);


int
MMChannel::process_incoming_audio()
{
    DWBYTE *b;
    int len;
    int timecode;
    int subchan;
    int ret;

    if((ret = tube->get_mm_data(b, len, AUDIO_CHANNEL_V2, subchan, timecode, 4)) < 0)
        return ret;
    if(len == 0)
    {
        // dummy hole punching packet
        GRTLOG("aud hole punch %d", myid, 0);
        delete [] b;
        return 1;
    }
    process_incoming_audio2(b, len, subchan, timecode);
    return 1;
}

void
MMChannel::process_incoming_reliable_audio()
{
    DWBYTE *buf;
    int len;
    int err;

    vc rv;
    if((err = tube->recv_data(rv, audio_chan)) == SSERR)
    {
        drop_subchannel(audio_chan);
        audio_chan = -2;
        force_unreliable_audio = 1;
    }
    else if(err != SSTRYAGAIN && rv.type() == VC_VECTOR)
    {
        if(rv[0].type() == VC_STRING
                && rv[1].type() == VC_INT && rv[2].type() == VC_INT)
        {
            int timecode = (int)rv[1];
            int payload = (int)rv[2];
            len = rv[0].len();
            buf = new DWBYTE[len];
            memcpy(buf, (const char *)rv[0], len);
            process_incoming_audio2(buf, len, subchan(AUDIO_SUBCHANNEL, payload), timecode);
        }
        else if(rv[0] == vc("!"))
        {
            // just eat pings
        }
    }

}

void
MMChannel::process_incoming_audio2(DWBYTE *b, int len, int subchan, int timecode)
{
    bps_audio_recv.add_units(8 * len);
    bps_recv.add_units(len * 8);
    int drop = inet_sim();
    GRTLOGA("recv audio got %d pt %d chan %d (drop %d)", timecode, get_payload_type(subchan),
            get_chan_num(subchan), drop, 0);

    // if the audio output has been turned off,
    // just drop the packet.

    MMChannel *mcao = this;

    if(!drop && mcao && mcao->audio_output && mcao->audio_output->status())
    {
        // decode it, should buffer up some amount
        // before starting playback

        // note: assumes audio decoder returns immediately
        // with decoded samples. breaks if audio decoder is
        // asynch.
        int pt = get_payload_type(subchan);

        AudioConverter *ad = 0;
        if(pt < 0 || pt >= PAYLOAD_NUM)
        {
            delete [] b;
            goto exit;
        }
        ad = audio_decoders[pt];

        if(ad == 0)
        {
            delete [] b;
            goto exit;
        }

        if(mcao->audio_output)
        {
            mcao->audio_output->set_decoder(ad);
            mcao->audio_output->play_seq_ec(b, len, timecode, 0, 1);
        }
    }
    else
        delete [] b;
exit:
    ;
}

void
MMChannel::process_audio_sampler()
{
    while(1)
    {

        int len;
        int time;
        EnterCriticalSection(&Audio_lock);
        if(!audio_sampler->has_data())
        {
            LeaveCriticalSection(&Audio_lock);
            break;
        }
        DWBYTE *b = (DWBYTE *)audio_sampler->get_data(len, time);
        LeaveCriticalSection(&Audio_lock);
#if 0
        {
            FILE *f = fopen("input.aud", "a");
            fwrite(b, len, 1, f);
            fclose(f);
        }
#endif
        if(All_mute)
        {
            delete [] b;
            ++audio_timecode;
            set_squelch(SQUELCH_ON);
            continue;
        }

        int is_silence;
        double sthr = 0;
        // get threshold value from somewhere if we are going to
        // use this type of squelching
        //
        // note: this is goofy, the "silence detector" no
        // longer just does silence detection, but instead
        // actually does preprocessing on the audio, so we
        //always have to do it.
// note: this was harwired to 0 some time ago when
// speex lib gave up on it...
        //is_silence = preprocess_w_vad.sd(b, len, 0);
        is_silence = 0;

        //is_silence = 0;
        if(!Auto_squelch || sthr == 0)
        {
            is_silence = 0;
        }

        GRTLOG("got audio silence %d", is_silence, 0);

        if(!Talk_override)
        {
            if(Auto_squelch && is_silence)
            {
                //set_audio_status(AUDIO_SQUELCH);
                delete [] b;
                ++audio_timecode;
                set_squelch(SQUELCH_ON);
                continue;
            }
        }

        set_squelch(SQUELCH_OFF);
        //set_audio_status(AUDIO_SEND);

        int n = MMChannels.num_elems();

        for(int i = 0; i < n; ++i)
        {
            MMChannel *m = MMChannels[i];
            if(!m)
                continue;
            if(Exclusive_audio)
            {
                if(Exclusive_audio_id == m->myid &&
                        !m->do_destroy && !(int)m->rem_pause_audio)
                {
                    DWBYTE *b2 = new DWBYTE[len];
                    memmove(b2, b, len);
                    m->raw_audioq.add(b2, len, audio_timecode, 0);
                    break;
                }
            }
            else if(m->grab_audio_id == myid && !m->do_destroy && !(int)m->rem_pause_audio)
            {
                DWBYTE *b2 = new DWBYTE[len];
                memmove(b2, b, len);
                m->raw_audioq.add(b2, len, audio_timecode, 0);
            }
        }
        ++audio_timecode;
        delete [] b;
    }
    audio_sampler->need();
}


int
MMChannel::process_outgoing_audio()
{
    if(!tube)
        return 1;

    if(raw_audioq.num_elems() == 0)
        return 1;

    MMTube *t = tube;

#if 0
    /* note: no point in checking for buf_drained, since we have
    * no way of applying backpressure in a reasonable way.
    * may as well just dump the bits out and let the
    * comm channel choke on it
    */
    else if(t && (!t->buf_drained(AUDIO_CHANNEL) || !t->can_write_mmdata()))
#endif
        // if we are connected to the chat server, do something different.
        // what a hack.
        if(pstate != CHAT_OPS)
        {
            if(force_unreliable_audio && t && t->full())
            {
                schedule_destroy();
                return 0;
            }
            if((force_unreliable_audio && t && !t->can_write_data(AUDIO_CHANNEL_V2)) ||
                    (!force_unreliable_audio && t && audio_state == MEDIA_SESSION_UP &&
                     !t->can_write_data(audio_chan)))
            {
                return 1;
            }
        }
        else
        {
            // if it is a chat server with audio pending, and we are not granted
            // the podium, just drop the audio on the ground. this probably
            // isn't going to sound right if there is a lot of audio buffered up, but
            // we'll give it a try, since it is easy.
            if(!granted[0])
            {
                while(raw_audioq.num_elems() > 0)
                {
                    // note: remove deletes buffer
                    raw_audioq.remove();
                }
                return 1;
            }
        }

    do
    {
        DWBYTE *obuf;
        DWBYTE *buf;
        int olen;
        int len;

        int timecode;
        int idx;
        raw_audioq.fifo_buf(obuf, olen, timecode, idx);

        //set_audio_status(AUDIO_SEND);
        // compress it
        AudioConverter *ac;
        ac = audio_coders[current_payload_type];
        ac->play_timed(obuf, olen, 0, 0);
        // note: assumes non-async compression...
        // note2: there is a potential problem with the advent of vbr audio
        // codecs like vorbis. sometimes you will get a 0 length return from
        // the compressor if vorbis decides it wants to buffer up the audio
        // for future analysis. in this case, we don't want to send the zero
        // length packet with the current audio timecode and so on, but
        // wait until the codec decides to vomit forth with the next
        // coded output. note that the spin will never happen, but i leave it
        // in there as a note to self, that some coders *can* be async, but
        // in this case, when we "play" a buffer to the encoding device,
        // we get one back synchronously, even if it is 0 length
        while(!ac->device_done2(buf, len))
        {
            // spin
        }
        raw_audioq.remove();
        if(len == 0)
        {
            // note: this will never happen with CBR codecs like gsm and speex
            delete [] buf;
            continue;
        }

        // adjust the timecode sent to the tube
        // based on whatever the client needs to
        // see (ie, for zaps, we want to string
        // together audio frames from successive
        // clips to make it look like they were
        // contiguous, regardles of what the
        // timecode is coming from the acquisition
        // device.
        if(make_first_0)
        {
            internal_0_offset = -timecode;
            make_first_0 = 0;
            GRTLOG("internal_adjust %d", internal_0_offset, 0);
        }
        GRTLOGA("tc %d off %d client %d = %d",
                timecode, internal_0_offset, client_timecode_adjustment,
                timecode + internal_0_offset + client_timecode_adjustment, 0);

        timecode += internal_0_offset + client_timecode_adjustment;

        if(pstate == CHAT_OPS)
        {
            // format the packet exactly like it would be formatted for other
            // cases, and then just send it to the chat server
            DWBYTE *outbuf = new DWBYTE[len +
                                        sizeof(audio_timecode) + sizeof(current_payload_type)];

            memcpy(outbuf, buf, len);
            memcpy(&outbuf[len], &audio_timecode, sizeof(audio_timecode));
            memcpy(&outbuf[len + sizeof(audio_timecode)], &current_payload_type, sizeof(current_payload_type));
            chatq_send_data(0, My_UID, (const char *)outbuf, len + sizeof(audio_timecode) + sizeof(current_payload_type));
            delete [] outbuf;
            bps_audio_send.add_units(8 * len);
            delete [] buf;
            buf = 0;
        }
        else if(force_unreliable_audio || audio_state != MEDIA_SESSION_UP)
        {
            t->put_mm_data(buf, len, AUDIO_CHANNEL_V2,
                           subchan(AUDIO_SUBCHANNEL, current_payload_type),
                           timecode, 4);
            if(status_callback)
                (*status_callback)(this, "Recording", scb_arg1, scb_arg2);
            delete [] last_audio_buf;
            last_audio_buf = buf;
            last_audio_len = len;
            last_payload_type = current_payload_type;
            bps_audio_send.add_units(8 * len);
        }
        else
        {
            if(audioq.num_elems() > 0)
            {
                // we have to q it behind other
                // data waiting
                audioq.add(buf, len, audio_timecode, last_audio_index,
                           current_payload_type);
            }
            else
            {
                // note: if send_reliable_audio returns 0
                // and the channel is still ok, it q's the data
                // to audioq for us.
                if(send_reliable_audio(buf, len))
                {
                    delete [] buf;
                    buf = 0;
                    bps_audio_send.add_units(8 * len);
                }
            }
        }

        last_audio_timecode = audio_timecode;
        audio_timecode = timecode;

        ++last_audio_index;
    } while(raw_audioq.num_elems() > 0);

    // check to see if we are an audio proxy and our source is
    // is alive: if not, destroy.
    if(grab_audio_id != -1 && !channel_by_id(grab_audio_id))
    {
        schedule_destroy();
        return 0;
    }
    // attempt to send any q'd audio data
    //
    if(!force_unreliable_audio && audio_state == MEDIA_SESSION_UP &&
            audioq.num_elems() > 0)
    {
        int len;
        if(attempt_audio_q_send(len))
            bps_audio_send.add_units(8 * len);
    }
    return 1;
}

// this was for display purposes mostly, but since we don't
// do this anymore, just get rid of it
void
MMChannel::handle_audio_duplex()
{
#if 0
    EnterCriticalSection(&Audio_lock);

    if(All_mute)
    {
        set_audio_status(AUDIO_MUTE);
    }
    else if(!Audio_full_duplex && (audio_output || audio_sampler))
    {
        // half-duplex case
        // leave the audio input going except when there
        // is audio being played out, or the user has
        // explicitly overridden with a mute.

        if(TheAudioOutput)
        {
            if(TheAudioOutput->status())
            {
                set_audio_status(AUDIO_NOTALK);
            }
            else if(TheAudioInput && TheAudioInput->status())
            {
                set_audio_status(AUDIO_TALK);
            }
        }
        else
        {
            // no audio output, if there is an input, leave it
            // on.
            if(TheAudioInput)
            {
                TheAudioInput->on();
                set_audio_status(AUDIO_TALK);
            }

        }
    }
    else if(Audio_full_duplex && (audio_output || audio_sampler))
    {
        if(TheAudioInput && TheAudioInput->status())
        {
            set_audio_status(AUDIO_TALK);
        }
        else if(TheAudioOutput && TheAudioOutput->status())
        {
            set_audio_status(AUDIO_PLAYING);
        }
    }
    LeaveCriticalSection(&Audio_lock);
#endif

}

unsigned int
MMChannel::get_audio_timecode()
{
    return audio_timecode;
}

void
MMChannel::set_audio_timecode(unsigned int t)
{
    audio_timecode = t;
    last_audio_timecode = t;
}
