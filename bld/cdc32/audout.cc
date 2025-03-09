
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/audout.cc 1.28 1999/01/10 16:09:27 dwight Checkpoint $
 */
#include "audout.h"
#include "audconv.h"


#define AUDDBG
#ifdef AUDDBG
#include "dwrtlog.h"
int AudioOutput::ln;
#define AUDRTLOG(l, s, i, j) GRTLOGA("id %d " s, logid, (i), (j), 0, 0);
#define AUDRTLOGA(l, s, i, j, k, z) GRTLOGA("id %d " s, logid, (i), (j), (k), (z));

#else
#define AUDRTLOG(l, s, i, j)
#define AUDRTLOGA(l, s, i, j, k, z)
#endif

AudioOutput::AudioOutput(int decom, int nbufs)
    : bufs(nbufs, 0, 0), lens(nbufs, 0, 0), do_free(nbufs, 0, 0), at_time(nbufs, 0, 0),
      bufs2(nbufs, 0, 0), lens2(nbufs, 0, 0), do_free2(nbufs, 0, 0),
      decoded(nbufs, 0, 0)
{
#ifdef AUDDBG
    logid = ln++;
#endif

    for(int i = 0; i < bufs.num_elems(); ++i)
    {
        bufs[i] = 0;
        lens[i] = 0;
        do_free[i] = 0;
        bufs2[i] = 0;
        lens2[i] = 0;
        do_free2[i] = 0;
        decoded[i] = 0;
    }
    //output_timer.set_autoreload(0);
    output_timer.set_oneshot(1);
    next = 0;
    next_out = 0;
    time_last_buf_played = output_timer.time_now();
    decompress = decom;
    // seq_bufs[0] = &bufs;
    // seq_lens[0] = &lens;
    // seq_bufs[1] = &bufs2;
    // seq_lens[1] = &lens2;
    curseq = 0;
    seq0 = -1;
    reset_timer = 1;
    overlap = 0;


    bufs_playing = 0;
    bufs_to_buffer = nbufs;
    bufs_buffered = 0;
    // this forces the first packet to start
    // the timer and other buffering item
    // since there are no bufs currently playing.
    buffering = 0;

    max_off = -1;
    ec_used = 0;

    decoder = 0;
    fancy_recover = 0;
    mixer = 0;
}

AudioOutput::~AudioOutput()
{
    for(int i = 0; i < bufs.num_elems(); ++i)
    {
        if(bufs[i] && do_free[i])
            delete [] bufs[i];
#if 0
        if(bufs2[i] && do_free2[i])
            delete [] bufs2[i];
#endif
    }
}

void
AudioOutput::set_decoder(AudioConverter *ac)
{
    decoder = ac;
}

AudioConverter *
AudioOutput::get_decoder()
{
    return decoder;
}

int
AudioOutput::status()
{
    return device_status();
}

int
AudioOutput::init()
{
    int res;
    res = device_init();
    flush_bufs();
    bufs_playing = 0;
    return res;
}

int
AudioOutput::reset()
{
    int res;
    output_timer.reset();
    res = device_reset();
    bufs_playing = 0;
    flush_bufs();
    return res;
}

int
AudioOutput::stop()
{
    int res;
    output_timer.stop();
    res = device_stop();
    return res;
}

int
AudioOutput::off()
{
    if(!device_status())
        return 1;
    device_stop();
    device_reset();
    bufs_playing = 0;
    device_close();
    flush_bufs();
    return 1;
}

int
AudioOutput::on()
{
    if(device_status())
        return 1;
    return device_init();
}

int
AudioOutput::prev(int i)
{
    return (i - 1) % bufs.num_elems();
}

void
AudioOutput::advance_output()
{
    // don't want to free here since device
    // may still be using the buffer.
    bufs[next_out] = 0;
    lens[next_out] = 0;
    next_out = (next_out + 1) % bufs.num_elems();
}

// this is used in situations where we are multiplexing streams
// from multiple sources (timecode streams) into one audio output
// device. normally, each stream gets its own AudioOutput object
// in order to do buffering, and this object assumes there is one
// increasing timecode stream in order to do the jitter buffering.
// now, we have a situation where we do not know how many streams
// we have, and they switch arbitrarily on and off. so we use one
// AudioOutput object, and reset the counters and stuff when the
// audio source switches.
void
AudioOutput::switch_streams()
{
    play_seq_to_device();
    off();
    on();
}

int
AudioOutput::full()
{
    return bufs[next] != 0;
}

int
AudioOutput::more_bufs()
{
    return bufs[next_out] != 0;
}

int
AudioOutput::play(DWBYTE *buf, int len, int do_free)
{
    int res;
    time_last_buf_played = output_timer.time_now();
    AUDRTLOGA(L, "play buf %p len %d fr %d", buf, len, do_free, 0)
    res = device_output(buf, len, do_free);
    ++bufs_playing;
    return res;
}

int
AudioOutput::play_silence()
{
    device_play_silence();
    ++bufs_playing;
    return 1;
}

int
AudioOutput::play_seq_to_device()
{
    //BUFVEC *s = seq_bufs[curseq];
    //LENVEC *l = seq_lens[curseq];
    int nbufs = bufs_to_buffer;
    int i;
    // don't play trailing silence, we set
    // seq0 to a value that might allow
    // us to recover those parts of the stream
    // if it coming in late.
    if(!fancy_recover)
    {
        for(i = nbufs - 1; i >= 0; --i)
        {
            if(bufs[i] != 0)
                break;
        }
        nbufs = i + 1;
    }
    for(i = 0; i < nbufs; ++i)
    {
        if(bufs[i] == 0)
        {
            play_silence();
#ifdef AUDDBG
            AUDRTLOG(L, "silence", 0, 0);
#endif
            decoded[i] = 0;
            // XXX experiment
            if(!fancy_recover)
                max_off = seq0 + i;
        }
        else
        {
            if(!decoded[i])
            {
                if(decoder)
                {
#ifdef AUDDBG
                    AUDRTLOG(L, "decoding buf %d", i, 0);
#endif
                    decoder->play_timed(bufs[i], lens[i], 0, 1);
                    while(!decoder->device_done2(bufs[i], lens[i]))
                        ; /* spin */
                }
            }
#ifdef AUDDBG
            AUDRTLOG(L, "play buf %d", i, 0);
#endif
            play(bufs[i], lens[i], 1);
            bufs[i] = 0;
            lens[i] = 0;
            decoded[i] = 0;
            max_off = seq0 + i;
        }
    }
    return 1;
}

int
AudioOutput::tick()
{
    //output_timer.tick();
    if(output_timer.is_expired())
    {
#ifdef AUDDBG
        AUDRTLOG(L, "timer exp", 0, 0);
#endif
        output_timer.reset();
        play_now();
    }
    DWBYTE *free_buf;
    int len;
    int dfree;
    while(device_done(free_buf, len, dfree))
    {
        if(dfree)
            delete [] free_buf;
        --bufs_playing;
#ifdef AUDDBG
        AUDRTLOG(L, "tick bufs %d", bufs_playing, 0);
#endif
        if(bufs_playing < 0)
        {
            oopanic("audout neg bufs");
        }
    }

    return 1;
}

int
AudioOutput::play_timed(DWBYTE *buf, int len, int in_at_time, int dfree)
{
    if(full())
        return 0;
    bufs[next] = buf;
    lens[next] = len;
    do_free[next] = dfree;

    // decide when we want to play the buffer.
    // the at_time param is an absolute time that
    // may have been derived from the time the data
    // was sampled on another system.
    // the time that we started playing the
    // last buffer is used.

    long time_until_play;
    if(in_at_time == 0)
        time_until_play = 0;
    else
        time_until_play = (output_timer.time_now() - time_last_buf_played) + in_at_time;
    if(time_until_play <= 0)
    {
        // play it now
        bufs[next] = 0;
        lens[next] = 0;
        play(buf, len, dfree);
    }
    else
    {
        // play it later
        at_time[next] = in_at_time;
        next = (next + 1) % bufs.num_elems();
        if(!output_timer.is_running())
        {
            output_timer.load(time_until_play);
            output_timer.start();
        }
    }
    return 1;
}

int
AudioOutput::flush_bufs()
{
#ifdef AUDDBG
    AUDRTLOG(L, "flush", 0, 0);
#endif
    for(int i = 0; i < bufs_to_buffer; ++i)
    {
        delete [] bufs[i];
        bufs[i] = 0;
        lens[i] = 0;
    }
    seq0 = -1;
    reset_timer = 1;
    overlap = 0;
    buffering = 0;
    bufs_buffered = 0;
    max_off = -1;
    return 1;
}

int
AudioOutput::play_now()
{
#ifdef AUDDBG
    AUDRTLOG(L, "play now %d", curseq, 0);
#endif
    if(bufs_buffered == 0)
        return 1;
    play_seq_to_device();
    buffering = 0;
    bufs_buffered = 0;
    reset_timer = 1;
    bufs_to_buffer = bufs.num_elems() / 2;
    output_timer.reset();
    output_timer.load(device_one_buffer_time() * (bufs_to_buffer));
    output_timer.start();

    // on a timer flush, we reset the packet
    // ordering in case a packet it going
    // to be a little late...
    //seq0 = -1;
    seq0 = max_off + 1;
#ifdef AUDDBG
    AUDRTLOG(L, "play now timer %d seq0 %d", output_timer.get_time_left(), seq0);
#endif
    return 1;
}

int
AudioOutput::play_seq(DWBYTE *buf, int len, int seq, int dfree)
{
    if(seq0 == -1)
        seq0 = seq;
    int off = seq - seq0;
    // assume it just wrapped, we're assuming
    // that dropped packets are common and
    // out of sequence packets are rare...
    if(off < 0)
    {
#ifdef AUDDBG
        AUDRTLOG(L, "wrapped %d", off, 0);
#endif
        off += 256;
    }
#ifdef AUDDBG
    AUDRTLOGA(L, "seq %d off %d b %d bp %d", seq, off, buffering, device_bufs_playing());
#endif

    if(!buffering)
    {
        if(device_bufs_playing() <= 1)
        {
            buffering = 1;
            bufs_buffered = 0;
            off = 0;
            seq0 = seq;
            output_timer.reset();
            output_timer.load(device_one_buffer_time() * (bufs_to_buffer));
            output_timer.start();
#ifdef AUDDBG
            AUDRTLOG(L, "timer start %d", output_timer.get_time_left(), 0);
#endif
        }
        else
        {
            // see if we need to insert some silence because
            // of a gap in the sequence numbering
            // but we only do it if this packet
            // isn't already represented in the
            // buffer (ei, if it is a little late, then
            // we just want to go ahead and put
            // in for playing without extra silence
            // padding.
            for(int i = 0; i < off; ++i)
            {
#ifdef AUDDBG
                AUDRTLOG(L, "silence in stream", 0, 0);
#endif
                play_silence();
            }
        }
    }

    if(buffering)
    {
        if(off < bufs_to_buffer)
        {
            bufs[off] = buf;
            lens[off] = len;
            ++bufs_buffered;
            //last_seq = seq;
            return 1;
        }
        else
        {
            play_seq_to_device();
            // if the offset if past the end
            // of the buffer, put in the
            // right number of silence units
            // to make up for it.
            int ns = off - bufs_to_buffer;
            while(ns-- > 0)
            {
#ifdef AUDDBG
                AUDRTLOG(L, "silence end", 0, 0);
#endif
                play_silence();
            }
            off = 0;
            buffering = 0;
            bufs_buffered = 0;
            output_timer.reset();
#ifdef AUDDBG
            AUDRTLOG(L, "timer stopped", 0, 0);
#endif
        }
    }
#ifdef AUDDBG
    AUDRTLOG(L, "played %d", seq, 0);
#endif
    play(buf, len, dfree);
    seq0 = seq + 1;
    return 1;
}

int
AudioOutput::change_buffering(int n)
{
    // try to change the number of buffers
    // that are used to smooth out playback.
    //

    if(n < 0)
        oopanic("negative buf change");
    if(n == 0)
        n = 1;
    if(n == bufs_to_buffer)
        return 1;
    // easy case: nothing in the buffer and just
    // playing freely...
    if(!buffering)
    {
        bufs.set_size(n);
        lens.set_size(n);
        do_free.set_size(n);
        bufs2.set_size(n);
        lens2.set_size(n);
        do_free2.set_size(n);
        decoded.set_size(n);
        bufs_to_buffer = n;
        for(int i = 0; i < n; ++i)
        {
            bufs[i] = 0;
            lens[i] = 0;
            decoded[i] = 0;
            do_free[i] = 0;
            bufs2[i] = 0;
            lens2[i] = 0;
            do_free2[i] = 0;
        }
        return 1;
    }
    // ignoring this is safe, as it will take effect next time the
    // audio output is set up. changing this while playback is nice
    // but not easy to support right now.
    GRTLOG("ignoring buffer change req", 0, 0);

    return 1;

}

int
AudioOutput::play_seq_ec(DWBYTE *buf, int len, int seq, int packet_seq, int dfree)
{
    if(seq0 == -1)
        seq0 = seq;
    int off = seq - seq0;
    if(off < 0)
    {
#ifdef AUDDBG
        AUDRTLOG(L, "neg off %d", off, 0);
#endif
        return 1;
    }

#ifdef AUDDBG
    AUDRTLOGA(L, "seq %d off %d b %d bp %d", seq, off, buffering, device_bufs_playing());
#endif
    int dont_load_timer = 0;

    if(!buffering && device_bufs_playing() <= 1)
    {
        // go back to buffering the entire
        // buffers worth
        buffering = 1;
        int morebufs = bufs.num_elems() - bufs_to_buffer;
        bufs_to_buffer = bufs.num_elems();
        if(output_timer.is_running())
        {
            output_timer.reset();
            output_timer.load(output_timer.get_time_left() +
                              device_one_buffer_time() * morebufs);
            output_timer.start();
#ifdef AUDDBG
            AUDRTLOG(L, "timer extend %d", output_timer.get_time_left(), 0);
#endif
        }
        else
        {
            output_timer.reset();
            output_timer.load(device_one_buffer_time() * (bufs_to_buffer));
            output_timer.start();
#ifdef AUDDBG
            AUDRTLOG(L, "timer start %d", output_timer.get_time_left(), 0);
#endif
        }
        if(bufs_buffered == 0)
        {
            seq0 = max_off + 1;
            off = seq - seq0;
#ifdef AUDDBG
            AUDRTLOG(L, "reset seq0 %d off %d", seq0, off);
#endif
            if(off < 0)
                return 1;
            if(off < bufs_to_buffer)
            {
                bufs[off] = buf;
                lens[off] = len;
#ifdef AUDDBG
                AUDRTLOG(L, "buffering at off %d", off, 0);
#endif
            }
            else
            {
                seq0 = seq;
                bufs[0] = buf;
                lens[0] = len;
#ifdef AUDDBG
                AUDRTLOG(L, "buffering at 0, seq0 %d", seq0, 0);
#endif
            }
            bufs_buffered = 1;
            manage_decode(0);
            return 1;
        }
    }

    if(off < bufs_to_buffer)
    {
        int used_it = 0;
        if(!bufs[off])
        {
            bufs[off] = buf;
            lens[off] = len;
            ++bufs_buffered;
            //last_seq = seq;
            used_it = 1;
            manage_decode(off);
#ifdef AUDDBG
            AUDRTLOG(L, "buffered %d", off, 0);
#endif
        }
        else
        {
            // chuck it, already got that packet
            if(dfree)
            {
                delete [] buf;
            }
#ifdef AUDDBG
            AUDRTLOG(L, "chucked %d", off, 0);
#endif
        }
        if(seq > max_off)
        {
            //max_off = seq;
        }
        else
        {
            // packet was out of order, which
            // probably means it was an error
            // correcting packet.
            if(used_it)
            {
                ++ec_used;
#ifdef AUDDBG
                AUDRTLOG(L, "ec_used %d %d", off, ec_used);
#endif
            }
        }
        return 1;
    }
    else
    {
        play_seq_to_device();
        // if the offset is past the end
        // of the buffer,
        // adjust the beginning of the buffer to
        // accomodate possible late coming packets.
        int newbtb = bufs.num_elems() / 2;
        int ns = off - bufs_to_buffer;
        if(ns < newbtb)
        {
            //seq0 += bufs_to_buffer;
            seq0 = max_off + 1;
            ns = seq - seq0;
            if(ns >= newbtb)
            {
                seq0 = seq;
                bufs[0] = buf;
                lens[0] = len;

#ifdef AUDDBG
                AUDRTLOG(L, "boosted3 seq0 %d %d", seq0, ns);
#endif
            }
            else
            {
                bufs[ns] = buf;
                lens[ns] = len;
#ifdef AUDDBG
                AUDRTLOG(L, "boosted2 seq0 %d %d", seq0, ns);
#endif
            }
            //max_off = seq;
            //bufs[ns] = buf;
            //lens[ns] = len;
            //bufs[ns] = buf;
            //lens[ns] = len;
        }
        else
        {
            // just reset seq0 to current seq
            seq0 = seq;
            bufs[0] = buf;
            lens[0] = len;
#ifdef AUDDBG
            AUDRTLOG(L, "set seq0 %d %d", seq0, ns);
#endif
            ns = 0;
        }
        off = 0;
        buffering = 0;
        bufs_buffered = ns + 1;
        manage_decode(0);
        bufs_to_buffer = bufs.num_elems() / 2;
        // XXX reset timer to right value
        output_timer.reset();
        output_timer.load(device_one_buffer_time() * (bufs_to_buffer));
        output_timer.start();
#ifdef AUDDBG
        AUDRTLOG(L, "timer start2 %d", output_timer.get_time_left(), 0);
#endif
    }

#if 0
#ifdef AUDDBG
    AUDRTLOG(L, "played %d", seq, 0);
#endif
    play(buf, len, dfree);
    seq0 = seq + 1;
#endif
    return 1;
}

//
// decode as much as we can given that packets
// may come in out of order, which means
// we have to decode only complete runs in order
// to keep decoder state in sync as much as possible.
// XXX ooops, you really need a decoder for each slot, since
// the decoder might change mid-stream...
// NOTE: this version of manage decode assumes there is a one-to-one
// correspondence between a buffer of coded audio, and a buffer of
// decoded audio. other parts of this class assume that the buffers
// are always the same size after they are decoded (there is no such
// assumption regarding the size of CODED audio buffers.) implicit
// in this assumption is that each buffer represents the same period
// of real-time capture.
//
// these assumptions are a problem for VBR audio codecs like vorbis.
// the audio mixer (audmix.cc) assumes that all buffers it receives
// are of a fixed size. we could fix this assumption is audmixs.cc
// (the class that SENDS the raw audio samples to the mixer) by allowing
// it to re-partition the raw audio data into fixed size chunks.
// this doesn't solve the problem in this class of the assumption that
// each buffer represents a fix period of time. in places where we need
// a time computed some time in the future (for timer expirations and
// so on) it should be computed from the already decoded data.
//
void
AudioOutput::manage_decode(int newbuf)
{
    int i;
    int n = bufs.num_elems();
    for(i = 0; i < n; ++i)
    {
        if(!bufs[i])
            return;
        if(!decoded[i])
            break;
    }
    for(; i < n && bufs[i] && !decoded[i]; ++i)
    {
        if(decoder)
        {
#ifdef AUDDBG
            AUDRTLOG(L, "manage_decode buf %d", i, 0);
#endif
            decoder->play_timed(bufs[i], lens[i], 0, 1);
            while(!decoder->device_done2(bufs[i], lens[i]))
                ; /* spin */
            do_free[i] = 1;
            decoded[i] = 1;
        }
    }
}
