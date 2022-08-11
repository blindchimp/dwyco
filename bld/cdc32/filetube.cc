
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// NOTE: THERE ARE LEAKS HERE IF  A STREAM IS
// CLOSED... the stuff that is queued up doesn't
// get released (normally it is sent out to the
// caller for eventual release)
/*
 * $Header: g:/dwight/repo/cdc32/rcs/filetube.cc 1.11 1999/01/10 16:09:49 dwight Checkpoint $
 */
#include "mmchan.h"
#ifdef _Windows
#include <windows.h>
#include <io.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "netvid.h"
#include "vc.h"
#include "dwvec.h"
#include "filetube.h"
#include "dwrtlog.h"
#include "netlog.h"

using namespace dwyco;

// switch little-endian stuff in files
#ifdef __ppc__
#define dwui(a) ((unsigned int)(a))
#define leswap4(b) (((dwui(b)&0xff) << 24)|((dwui(b)&0xff00) << 8)|((dwui(b)&0xff0000) >> 8)|((dwui(b)>>24)&0xff))
#else
#define leswap4(a) (a)
#endif


FileTube::FileTube(FILE *f, enum dir w, int wc)
{
    file = f;
    we_close = wc;
    filelen = filelength(fileno(f));
    t0 = -1;
    qtimer.set_oneshot(1);
    last_time_pos = 0;
    which = w;
    acq_time = -1;
    last_t = -1;
    first_time = 1;
    packet_count = -1;
    bytes_left = 100 * 1024 * 1024;
    eotl = 0;
    kludge_done = 0;
    auto_stop_delay = 0;
}

FileTube::FileTube(const DwString& filename, const DwString& filemode, enum dir w)
{
    file = fopen(filename.c_str(), filemode.c_str());
    if(!file)
        return;
    name = filename;
    mode = filemode;
    if(filemode.c_str()[0] == 'a')
        fseek(file, 0, SEEK_END);
    we_close = 1;
    filelen = filelength(fileno(file));
    t0 = -1;
    qtimer.set_oneshot(1);
    last_time_pos = 0;
    which = w;
    acq_time = -1;
    last_t = -1;
    first_time = 1;
    packet_count = -1;
    bytes_left = 100 * 1024 * 1024;
    eotl = 0;
    kludge_done = 0;
    auto_stop_delay = 0;
}


FileTube::~FileTube()
{
    if(we_close && file)
        fclose(file);
    int n = pickupq.num_elems();
    for(int i = 0; i < n; ++i)
        delete [] pickupq[i].buf;
}

void
FileTube::flush()
{
    if(file)
        fflush(file);
}

int
FileTube::full()
{
    if((packet_count != -1 && packet_count == 0) ||
            bytes_left < 100)
        return 1;
    return 0;
}

int
FileTube::has_mmdata()
{
    if(connected) oopanic("has_mm connected");
    if(pickupq.num_elems() > 0)
    {
        struct mmblock& m = pickupq[0];
        GRTLOG("vid pickup", 0, 0);
        if(m.chan == 0)
            return 1;
        return 0;
    }
    return 0;
    //return ftell(file) < filelen;
}

int
FileTube::can_write_mmdata()
{
    if(!ferror(file))
    {
        if(packet_count == 0)
            return 0;
        if(bytes_left < 100)
            return 0;
        return 1;
    }
    return 0;
}

int
FileTube::has_ctrl()
{
    if(which == SINK || auto_stop_delay == 0)
        return 0;
// WARNING: massive kludge
// we wait an extra 400ms after this condition holds
// before returning true. this is because the audio output
// has about 4*80 = 320ms of buffering after the last packet
// is output to the top level driver.
// the only reason this stuff is here is because we want
// some kind of "auto-stop" event when the stream is done
// playing out (cdc doesn't use this, because it is up to
// the user to stop the stream. icuii wants the auto-stop.
    if(kludge_done)
        return 1;
    //kludge_timer.tick();
    if(kludge_timer.is_expired())
    {
        kludge_timer.ack_expire();
        kludge_done = 1;
        return 1;
    }
    if(kludge_timer.is_running())
        return 0;
    // can't test file anymore because it is usually 0 now.
    if(/*(feof(file) || ferror(file) ||*/ timeline.done() && pickupq.num_elems() == 0)
    {
        kludge_timer.load(auto_stop_delay + 400);
        kludge_timer.set_oneshot(1);
        //kludge_timer.set_autoreload(0);
        kludge_timer.reset();
        kludge_timer.start();
    }
    return 0;
}

void
FileTube::set_auto_stop_delay(int a)
{
    auto_stop_delay = a;
}

void
FileTube::set_timeline_speed(double x)
{
    timeline.set_speed(x);
}

int
FileTube::can_write_ctrl()
{
    return 1;
}


int
FileTube::has_data(int chan)
{
    if(connected) oopanic("has_data connected");
    if(pickupq.num_elems() > 0)
    {
        struct mmblock& m = pickupq[0];
        if(m.chan == chan)
        {
            GRTLOG("pickup chan %d", chan, 0);
            return 1;
        }
        return 0;
    }
    return 0;
}

int
FileTube::can_write_data(int chan)
{
    if(!ferror(file))
        return 1;
    return 0;
}


// note: errors in the mm channel
// are not treated as fatal.
//
// note: in order for these calls to produce a
// stream that is compatible with ver 0.95
// user len must be 1, and seq must be 0.
// likewise, the receiver has to be told it is an
// old style stream by setting the user_len to 1
// and the seq to 0.

// note: found some really wierd stuff:
// if you don't terminate a thread just right,
// it trashes the file descriptor handling
// in windows, causing write failures... odd.
int
FileTube::send_mm_data(DWBYTE *buf, int len, int subchan, int user_byte, int user_len, int seq)
{
    GRTLOGA("send_mm_data %d subchan %d ub %d ul %d seq %d", len, subchan, user_byte, user_len, seq);
    if(bytes_left - (len + 16) <= 0)
    {
        bytes_left = 0;
        return 0;
    }
    int wlen = leswap4(len);
    fwrite(&wlen, sizeof(wlen), 1, file);
    fwrite(buf, len, 1, file);
    fwrite(&subchan, 1, 1, file);
    int chan = 0;
    fwrite(&chan, 1, 1, file);
    if(user_len != 4)
        oopanic("send_mm userlen!=4");

    // override user_byte to include the
    // new timing info. this used at
    // index time to play things back
    // in sync. old software ignores the
    // user byte so this is safe. new software
    // will have to detect the user byte
    // generated by old software, and use
    // the old timing info instead of this info.
    if(acq_time == -1)
    {
        user_byte = 0;
    }
    else
    {
        user_byte = leswap4(acq_time);
    }

    fwrite(&user_byte, user_len, 1, file);
    int wseq = leswap4(seq);
    fwrite(&wseq, 4, 1, file);
    // keep old time info as is for old
    // software
    unsigned long t;

    t = DwTimer::time_now();
    // XXX BAD in 64-bitsville
    int wt = leswap4(t);
    fwrite(&wt, 4, 1, file);

    int ret = SSERR;
    if(!ferror(file))
    {
        ret = 1;
        in_bits[subchan] += 8 *
                            (long)len +
                            8 +
                            8 +
                            user_len * 8 +
                            (seq != 0 ? 32 : 0) +
                            4 * 8 ;
    }
    filelen = ftell(file);
    if(packet_count > 0)
        --packet_count;
    bytes_left -= len + 16;
    return ret;
}

int
FileTube::reset_timer(int user_flag)
{
    const char *buf = "dwyco1";
    //send_mm_data(buf, 6, 0, 0xaa, 1, 0);
    int len = 6;
    int wlen = leswap4(len);
    fwrite(&wlen, sizeof(wlen), 1, file);
    fwrite(buf, len, 1, file);
    int chan = 0;
    fwrite(&chan, 1, 1, file);
    fwrite(&chan, 1, 1, file);
    int user_byte = user_flag;
    int user_len = 4;
    fwrite(&user_byte, user_len, 1, file);
    int seq = 0;
    fwrite(&seq, 4, 1, file);

    long t = -1;
    fwrite(&t, 4, 1, file);
    return 1;
}

int
FileTube::recv_mm_data(DWBYTE*& buf, int& len, int& chan, int& user, int user_len, int& seq)
{
    if(connected) oopanic("recv_mm connected");
    if(pickupq.num_elems() == 0)
        return SSERR;
    struct mmblock m = pickupq[0];
    pickupq.del(0);
    buf = m.buf;
    len = m.len;
    chan = m.subchan;
    user = m.user;
    seq = m.seq;
    return 1;
}

int
FileTube::read_mm_data(DWBYTE*& buf, int& len, int& chan, int& subchan,
                       int& user, int user_len, int& seq, long& time, long& filepos, int inhibit_buf_read)
{
    if(feof(file))
        return SSERR;

    len = 0;
    if(!fread(&len, 4, 1, file))
        return SSERR;
    len = leswap4(len);
    // arbitrary limit, just to keep things sane
    if(len < 0 || len > 10 * 1024 * 1024)
        return SSERR;
    filepos = ftell(file);
    if(inhibit_buf_read)
    {
        buf = 0;
        if(fseek(file, len, SEEK_CUR))
            return SSERR;
    }
    else
    {
        buf = new DWBYTE[len];
        if(!buf)
            return SSERR;
        if(!fread(buf, len, 1, file))
            return SSERR;
    }
    //chan = 0;
    subchan = 0;
    unsigned char tmp;
    if(!fread(&tmp, 1, 1, file))
        return SSERR;
    subchan = tmp;
    chan = 0;
    if(!fread(&tmp, 1, 1, file))
        return SSERR;
    chan = tmp;
#if 0
    if(chan < 2)
        user_len = 1;
    else
        user_len = 4;
#endif
    user_len = 4;
    user = 0;
    if(!fread(&user, user_len, 1, file))
        return SSERR;
    user = leswap4(user);
    if(!fread(&seq, 4, 1, file))
        return SSERR;
    seq = leswap4(seq);
    // paid for this going to 64bits
    time = 0;
    int tmptime = 0;
    if(!fread(&tmptime, 4, 1, file))
        return SSERR;
    time = leswap4(tmptime);
    if(t0 == -1)
        t0 = time;
//VcError << "got user " << user << "\n";

    if(ferror(file))
        return SSERR;
    GRTLOGF("read_mm_data %d chan %d subchan %d user %d seq %d time %d", len, chan, subchan, user, seq, time);

    return 1;
}

//
// new style
//
int
FileTube::get_mm_data(DWBYTE*& buf, int& len, int chan, int& subchan,
                      int& user, int user_len, int& seq)
{
    if(connected) oopanic("get_mm connected");
    if(pickupq.num_elems() == 0)
        return SSERR;
    struct mmblock m = pickupq[0];
    pickupq.del(0);
    if(m.chan != chan)
        oopanic("get_mm bad chan");
    buf = m.buf;
    len = m.len;
    subchan = m.subchan;
    user = m.user;
    seq = m.seq;
    return 1;
}

int
FileTube::put_mm_data(DWBYTE *buf, int len, int chan, int subchan, int user,
                      int user_len, int seq)
{
    GRTLOGF("put_mm_data %d chan %d subchan %d user %d ul %d seq %d", len, chan, subchan, user, user_len, seq);
    if(bytes_left - (len + 16) <= 0)
    {
        bytes_left = 0;
        return 0;
    }
    if(chan < 2 || user_len != 4)
        oopanic("put_mm chan<2||userlen!=4");
    int wlen = leswap4(len);
    fwrite(&wlen, sizeof(wlen), 1, file);
    fwrite(buf, len, 1, file);
    fwrite(&subchan, 1, 1, file);
    fwrite(&chan, 1, 1, file);
    int wuser = leswap4(user);
    fwrite(&wuser, user_len, 1, file);
    int wseq = leswap4(seq);
    fwrite(&wseq, 4, 1, file);

    // output the normal time so old
    // players work. at indexing time, the
    // timecode will be converted to more
    // accurate time in newer players.
    unsigned long t;

    t = DwTimer::time_now();
    int wt = leswap4(t);
    fwrite(&wt, 4, 1, file);

    int ret = SSERR;
    if(!ferror(file))
    {
        ret = 1;
        in_bits[chan] += 8 *
                         (long)len +
                         8 +
                         8 +
                         user_len * 8 +
                         (seq != 0 ? 32 : 0) +
                         4 * 8;
    }
    filelen = ftell(file);
    if(packet_count > 0)
        --packet_count;
    bytes_left -= len + 16;
    return ret;
}


int
FileTube::send_ctrl_data(vc v)
{
    return 1;
}

int
FileTube::recv_ctrl_data(vc& v)
{
    // note: only time control data is
    // simulated is when the stream is
    // completed.
    return SSERR;
}


void
FileTube::load_q()
{
    // read the next block and setup timer
    // to come ready when block is to be
    // presented to client.
    struct mmblock m;
    if(read_mm_data(m.buf, m.len, m.chan, m.subchan, m.user, -1, m.seq, m.time, m.filepos, 0) <= 0)
        return;
    qtimer.reset();
    qtimer.load(m.time - t0);
    t0 = m.time;
    qtimer.start();
    if(m.time == -1)
    {
        delete [] m.buf;
        load_q();
        return;
    }
    else
        q.append(m);
}

int
FileTube::tick()
{
    if(which == SOURCE)
    {
        if(qtimer.is_expired())
        {
            qtimer.ack_expire();
            pickupq.append(q[0]);
            q.del(0);
        }
        if(q.num_elems() == 0 && file /*&& !ferror(file) && !feof(file) &&
			ftell(file) < filelen*/)
        {
            load_q();
        }
    }
    unsigned long t = GetTickCount();
    tick_time = t - last_tick;
    last_tick = t;
    int n = baud.num_elems();
    for(int i = 0; i < n; ++i)
    {
        unsigned long bits_pumped = (tick_time * baud[i]) / 1000;
        in_bits[i] -= bits_pumped;
        if(in_bits[i] < 0)
            in_bits[i] = 0;
    }
    return 1;
}


int
FileTube::generates_events()
{
    if(which == SOURCE)
    {
        if(((file && (feof(file) || ferror(file))) || timeline.done()) &&
                pickupq.num_elems() == 0)
            return 1; // since we are at the end, stop spinning
        return 0; // make it spin
    }
    return 1;
}


DummyTube::DummyTube()
{
}

DummyTube::~DummyTube()
{
}

int
DummyTube::send_ctrl_data(vc v)
{
    return 1;
}

int
DummyTube::recv_ctrl_data(vc& v)
{
    return 1;
}
int
DummyTube::has_ctrl()
{
    return 0;
}

int
DummyTube::can_write_ctrl()
{
    return 1;
}
int
DummyTube::has_mmdata()
{
    return 0;
}

int
DummyTube::can_write_mmdata()
{
    return 1;
}



int
DummyTube::connect(const char *remote_addr, const char *local_addr, int block, HWND hwnd, int setup_unreliable)
{
    if(connected)
        return SSERR;
    connected = 1;
    remote = remote_addr;
    return 1;
}

int
DummyTube::gen_channel(unsigned short remote_port, int& chan)
{
    if(!connected)
        return SSERR;
    int retry = 1;
    if(chan == -1)
    {
        chan = find_chan();
    }
    if(socks[chan] == 0)
    {
        socks[chan] = new SimpleSocket;
        retry = 0;
        socks[chan]->non_blocking(1);
    }
    // XXX block/noblock on connect
    socks[chan]->non_blocking(1);
    DwString peer;

    if(!retry)
    {
        char csps[20];
        sprintf(csps, "%u", remote_port);
        peer = (const char *)remote;
        int c;
        if((c = peer.find(":")) == DwString::npos)
        {
            peer += ":";
            peer += csps;
        }
        else
        {
            peer.remove(c);
            peer += ":";
            peer += csps;
        }
    }
    if(!socks[chan]->init(retry ? 0 : peer.c_str(), 0, retry))
    {
        int ret = SSERR;
        if(socks[chan]->wouldblock())
            return SSTRYAGAIN;
        drop_channel(chan);
        return ret;
    }
    Netlog_signal.emit(mklog("event", "dchan connected", "chan_id", chan,
                          "local_ip", socks[chan]->local_addr().c_str(),
                          "peer_ip", socks[chan]->peer_addr().c_str()
                          ));
    return 1;
}

// note: because of the limitation on the number of
// open files in the borland rtl, we have to screw
// around here and make sure we don't keep files
// open while the tube exists (if the file is a SOURCE.)
// if the file is a SINK, then we keep the file open, because
// we don't record that many multiple videos at once.
// with the advent of the visual dir where we may be
// playing back 100's of videos at once, we ran into the
// open file limitation
#if 0
SlippyTube::SlippyTube(FILE *f, enum dir w, int wc) :
    FileTube(f, w, wc)
{
    for(int i = 0; i < FIRST_RELIABLE_CHAN; ++i)
        chan_delays[i] = 0;
    max_search = 0;
    idx = 0;
    use_old_timing = 0;
    force_timing = 0;
    prefer_old_timing = 0;
    enable_video = enable_audio = 1;
    vid_frames_to_index = -1;
}
#endif

SlippyTube::SlippyTube(const DwString &name, const DwString &mode, enum dir w) :
    FileTube(name, mode, w)
{
    for(int i = 0; i < FIRST_RELIABLE_CHAN; ++i)
        chan_delays[i] = 0;
    max_search = 0;
    idx = 0;
    use_old_timing = 0;
    force_timing = 0;
    prefer_old_timing = 0;
    enable_video = enable_audio = 1;
    if(w == SOURCE)
    {
        if(file)
            fclose(file);
        file = 0;
        cur_seek = 0;
    }
    vid_frames_to_index = -1;
}

SlippyTube::~SlippyTube()
{
}

void
SlippyTube::set_chan_enables(int video, int audio)
{
    enable_video = video;
    enable_audio = audio;
}

void
SlippyTube::set_chan_delay(int chan, int delay)
{
    chan_delays[chan] = delay;
    int n = chan_delays.num_elems();

    int min = 100000;
    int max = -100000;
    for(int i = 0; i < n; ++i)
    {
        if(chan_delays[i] < min)
            min = chan_delays[i];
        if(chan_delays[i] > max)
            max = chan_delays[i];
    }
    max_search = max - min;
    GRTLOG("max search %d", max_search, 0);
}

int
cmp_mmblock(const void *b1, const void *b2)
{
    const FileTube::mmblock *m1 = (const FileTube::mmblock *)b1;
    const FileTube::mmblock *m2 = (const FileTube::mmblock *)b2;
    if(m1->time < m2->time)
        return -1;
    if(m1->time > m2->time)
        return 1;
    return 0;
}

int
SlippyTube::is_dwyco_media(const DwString& filename)
{
    FILE *file;
    file = fopen(filename.c_str(), "rb");
    if(!file)
        return 0;
    int ret = 0;

    int len = 0;
    if(fread(&len, 4, 1, file) == 1)
    {
        len = leswap4(len);

        if(len == 6)
        {

            char buf[6];
            if(fread(buf, 6, 1, file) == 1)
            {
                if(strncmp(buf, "dwyco2", 6) == 0 ||
                        strncmp(buf, "dwyco3", 6) == 0)
                {
                    ret = 1;
                }
            }
        }
    }
    fclose(file);
    return ret;

}

int
SlippyTube::quick_stats(int max_read, int stop_after_n_video, int stop_after_n_audio, int& video_out, int& audio_out, int& num_clips_out, int& codec, DWBYTE ** last_vid_buf, int *last_vid_buf_len)
{
    struct mmblock m;
    int first_read = 1;
    int use_new_times = 1;
    num_clips_out = 0;
    video_out = 0;
    audio_out = 0;
    int ao = 0;
    int vo = 0;
    int co = 0;
    if(last_vid_buf)
        *last_vid_buf = 0;

    int i = 0;
    file = fopen(name.c_str(), mode.c_str());
    if(!file)
        return 0;
    if(fseek(file, cur_seek, SEEK_SET) != 0)
        return 0;
    while((max_read == -1 || i < max_read) && read_mm_data(m.buf, m.len, m.chan, m.subchan, m.user, -1, m.seq, m.time, m.filepos, (first_read || last_vid_buf) ? 0 : 1) > 0)
    {
        if(first_read)
        {
            first_read = 0;
            if(m.time == -1)
            {
                // note: in old files this is always set to 0.
                // so the old dct codec is 0.
                codec = m.user;
                if(m.len == 6 && strncmp((const char *)m.buf, "dwyco2", 6) == 0)
                {
                    delete [] m.buf;
                    m.buf = 0;
                    if(force_timing)
                        use_new_times = !use_old_timing;
                    else
                        use_new_times = 1;
                    continue;
                }
                if(m.len == 6 && strncmp((const char *)m.buf, "dwyco3", 6) == 0)
                {
                    delete [] m.buf;
                    m.buf = 0;
                    if(force_timing)
                        use_new_times = !use_old_timing;
                    else
                        use_new_times = 0;
                    continue;
                }
            }
            // really old messages have no header
            //delete [] m.buf;
            //m.buf = 0;
        }
        if(m.time == -1)
            ++co;
        else if(m.chan == AUDIO_CHANNEL_V2 || m.chan == AUDIO_CHANNEL)
        {
            ++ao;
        }
        else if(m.chan == VIDEO_CHANNEL || m.chan == VIDEO_CHANNEL_V2)
        {
            ++vo;
            if(last_vid_buf)
            {
                if(*last_vid_buf)
                    delete [] *last_vid_buf;
                *last_vid_buf = new DWBYTE[m.len];
                // prevent a crash if something doesn't
                // decode right
                if(m.buf)
                {
                    memcpy(*last_vid_buf, m.buf, m.len);
                }
                *last_vid_buf_len = m.len;
            }
        }
        if(m.buf)
        {
            delete [] m.buf;
            m.buf = 0;
        }
        ++i;
        if(stop_after_n_video != -1 && vo >= stop_after_n_video)
            break;
        if(stop_after_n_audio != -1 && ao >= stop_after_n_audio)
            break;

    }
    audio_out = ao;
    video_out = vo;
    num_clips_out = co;
    rewind(file);
    fclose(file);
    file = 0;
    cur_seek = 0;
    return i;
}

static void
initlong(long& l)
{
    l = 0;
}
void
SlippyTube::index_time_data()
{
    // create an index on the first run thru

    struct mmblock m;
    DwVec<long> last_time(10, !DWVEC_FIXED, DWVEC_AUTO_EXPAND, DWVEC_DEFAULTBLOCKSIZE, initlong);
    DwVec<long> chan_offsets(10, !DWVEC_FIXED, DWVEC_AUTO_EXPAND,DWVEC_DEFAULTBLOCKSIZE, initlong);
    long latest_time = 0;
    int do_offset = 0;
    int first_read = 1;
    int use_new_times = 0;
    int last_clip_had_video = 0;
    int last_clip_had_audio = 0;
    int this_clip_had_video = 0;
    int this_clip_had_audio = 0;
    long end_of_last_clip = 0;
    int first_audio = 1;
    long audio_offset = 0;
    int first_in_clip = 1;
    long first_tc_in_clip = 0;
    file = fopen(name.c_str(), mode.c_str());
    if(!file)
        return;
    if(fseek(file, cur_seek, SEEK_SET) != 0)
        return;
    int skip_audio_frames = vid_frames_to_index != -1;
    while(read_mm_data(m.buf, m.len, m.chan, m.subchan, m.user, -1, m.seq, m.time, m.filepos, first_read ? 0 : 1) > 0)
    {
        if(first_read)
        {
            first_read = 0;
            if(m.time == -1)
            {
                if(m.len == 6 && strncmp((char *)m.buf, "dwyco2", 6) == 0)
                {
                    delete [] m.buf;
                    m.buf = 0;
                    if(force_timing)
                        use_new_times = !use_old_timing;
                    else
                        use_new_times = 1;
                    continue;
                }
                if(m.len == 6 && strncmp((char *)m.buf, "dwyco3", 6) == 0)
                {
                    delete [] m.buf;
                    m.buf = 0;
                    if(force_timing)
                        use_new_times = !use_old_timing;
                    else
                        use_new_times = 0;
                    continue;
                }
            }
            delete [] m.buf;
            m.buf = 0;
        }
        if(use_new_times)
        {
            if(m.time != -1)
            {
                long stime;
                if(m.chan == VIDEO_CHANNEL)
                {
                    if(vid_frames_to_index != -1 &&
                            vid_frames_to_index-- == 0)
                        break;
                    stime = m.user;
                    // video always starts at 0 in a new
                    // clip, so we always offset by whatever
                    // the last clip ended at.
                    stime += end_of_last_clip;
                    this_clip_had_video = 1;
                }
                else if(m.chan == AUDIO_CHANNEL_V2 && !skip_audio_frames)
                {
                    stime = m.user * 80;
                    // audio is offset by the client, so
                    // we only need to offset audio when
                    // there was no audio in the last clip.
                    // *but* we need to offset it by the
                    // different between the last clip
                    // that had audio, and the start of this
                    // clip.
                    if(first_audio)
                    {
                        audio_offset = end_of_last_clip - m.user * 80;
                        first_audio = 0;
                        GRTLOG("audio off %d", audio_offset, 0);
                    }
                    stime += audio_offset;
                    this_clip_had_audio = 1;
                }
                else
                {
                    // punt, don't know what it is
                    continue;
                }

                last_time[m.chan] = stime;
                if(stime > latest_time)
                    latest_time = stime;
                GRTLOGA("len %d chan %d sc %d u %d time %d", m.len, m.chan, m.subchan, m.user, stime);
                stime += chan_delays[m.chan];
                m.time = stime;
                // do all time computations taking into account
                // the existence of audio and video channels, but
                // if a channel is disabled, don't process it later.
                // (might want to change this later so that if
                // (for example) two video clips were separated by
                // an audio-only clip, and audio is disabled, the playback
                // shows as if there was no audio. minor detail, probably
                // not worth too much effort
                if((enable_audio && (m.chan == AUDIO_CHANNEL_V2 || m.chan == AUDIO_CHANNEL)) ||
                        (enable_video && (m.chan == VIDEO_CHANNEL || m.chan == VIDEO_CHANNEL_V2)))
                    vec.append(m);
                if(vid_frames_to_index == 0)
                    break;
            }
            else
            {
                int n = chan_offsets.num_elems();
                do_offset = 0;
                first_audio = 1;
                // +1 to avoid getting video frames
                // that are on the same time and might
                // get reversed when sorted.
                end_of_last_clip = latest_time + 1;
                for(int i = 0; i < n; ++i)
                {
                    //chan_offsets[i] = last_time[i];
                    chan_offsets[i] = latest_time;
                    GRTLOG("chan %d off %d", i, chan_offsets[i]);
                }

            }
        }
        else
        {
            // old style timing, monotonically
            // increasing for each packet, regardless
            // of channel
            if(m.time != -1)
            {
                if(first_in_clip)
                {
                    first_tc_in_clip = m.time;
                    first_in_clip = 0;
                }
                if(m.chan == VIDEO_CHANNEL)
                {
                    if(vid_frames_to_index != -1 &&
                            vid_frames_to_index-- == 0)
                        break;
                }
                else if(skip_audio_frames) // audio channel of some sort
                {
                    // don't index audio frames
                    continue;
                }

                long stime = (m.time - first_tc_in_clip) + end_of_last_clip;
                if(stime > latest_time)
                    latest_time = stime;
                GRTLOGA("old len %d chan %d sc %d u %d time %d", m.len, m.chan, m.subchan, m.user, stime);
                stime += chan_delays[m.chan];
                m.time = stime;
                if((enable_audio && (m.chan == AUDIO_CHANNEL_V2 || m.chan == AUDIO_CHANNEL)) ||
                        (enable_video && (m.chan == VIDEO_CHANNEL || m.chan == VIDEO_CHANNEL_V2)))
                    vec.append(m);
                if(vid_frames_to_index == 0)
                    break;
            }
            else
            {
                end_of_last_clip = latest_time;
                first_in_clip = 1;
            }
        }
    }
    if(vec.num_elems() >= 2)
        qsort(&vec[0], vec.num_elems(), sizeof(vec[0]), cmp_mmblock);
    // create a timeline
    // and make sure the elements are
    // normalized so the first one is 0
    // this allows us to play old-style files
    // which have the current system time
    // for timing information.
    int n = vec.num_elems();
    timeline.reset();

    unsigned long vt0 = 0;
    if(vec.num_elems() >= 1)
        vt0 = vec[0].time;
    for(int i = 0; i < n; ++i)
    {
        timeline.append(vec[i].time - vt0, &vec[i], i);
    }
    timeline.start();
    idx = 0;
    rewind(file);
    fclose(file);
    file = 0;
    cur_seek = 0;
    // if there is nothing in the timeline, we
    // set the auto-stop delay so that the
    // streams ends almost immediately. note: for cdc
    // we want to avoid this because we normally keep
    // streams going until the user stops it. so,
    // since cdc doesn't use "vid_frames_index" at the moment,
    // we key off that.
    if(skip_audio_frames)
        auto_stop_delay = -400;
}

int
SlippyTube::tick()
{
    if(which == SOURCE)
    {
        if(first_time)
        {
            index_time_data();
            first_time = 0;
        }
        file = fopen(name.c_str(), mode.c_str());
        if(!file)
            return 0;
        while(timeline.tick())
        {
            unsigned long time;
            void *v;
            int i;
            if(timeline.get_top(time, v, i))
            {
                struct mmblock m = *(struct mmblock *)v;
                if(m.buf != 0)
                    oopanic("sliptick m.buf!=0");
                if(fseek(file, m.filepos, SEEK_SET))
                {
                    continue;
                }
                m.buf = new DWBYTE[m.len];
                if(!m.buf)
                    continue;
                if(!fread(m.buf, m.len, 1, file))
                    continue;
                pickupq.append(m);
            }
            else
            {
                eotl = 1;
            }
        }
        cur_seek = ftell(file);
        fclose(file);
        file = 0;
    }
    unsigned long t = GetTickCount();
    tick_time = t - last_tick;
    last_tick = t;
    int n = baud.num_elems();
    for(int i = 0; i < n; ++i)
    {
        unsigned long bits_pumped = (tick_time * baud[i]) / 1000;
        in_bits[i] -= bits_pumped;
        if(in_bits[i] < 0)
            in_bits[i] = 0;
    }
    return 1;
}

int
SlippyTube::reset_timer(int user_flag)
{
    const char *buf;
    if(prefer_old_timing)
        buf = "dwyco3";
    else
        buf = "dwyco2";
    //send_mm_data(buf, 6, 0, 0xaa, 1, 0);
    int len = 6;
    int wlen = leswap4(len);
    fwrite(&wlen, sizeof(wlen), 1, file);
    fwrite(buf, len, 1, file);
    int chan = 0;
    fwrite(&chan, 1, 1, file);
    fwrite(&chan, 1, 1, file);
    int user_byte = user_flag;
    int user_len = 4;
    fwrite(&user_byte, user_len, 1, file);
    int seq = 0;
    fwrite(&seq, 4, 1, file);

    int t = -1;
    fwrite(&t, 4, 1, file);
    return 1;
}
