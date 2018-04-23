
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
/*
 * $Header: g:/dwight/repo/cdc32/rcs/filetube.h 1.11 1999/01/10 16:11:00 dwight Checkpoint $
 */
#ifndef FILETUBE_H
#define FILETUBE_H
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "matcom.h"
#include "dwtimer.h"
#include "dwvecp.h"
#include "netvid.h"
#include "tl.h"
#include "dwstr.h"

// error returns
// op not completed: <0
// op can't be completed: -1
// op might be completed if you try again: -2
// op ok, >= 0
// this is screaming for exception handling, but
// since not all compilers have it now, i'm hosed...
//


class FileTube : public MMTube
{
    friend int cmp_mmblock(const void *, const void *);
public:
    enum dir {SINK, SOURCE};
protected:
    struct mmblock
    {
        mmblock() {
            len = chan = subchan = user = seq = time = -1;
            buf = 0;
            filepos = 0;
        }
        DWBYTE *buf;
        int len;
        int chan;
        int subchan;
        int user;
        int seq;
        long time;
        long filepos;
        int operator==(const mmblock& m) const {
            return 0;
        }
    };
    FILE *file;
    DwString name;
    DwString mode;
    long cur_seek;
    int we_close;
    enum dir which;
    long filelen;
    DwVec<mmblock> q;
    DwVec<mmblock> pickupq;
    long t0;
    DwTimer qtimer;
    long last_time_pos;
    long acq_time;
    long last_t;
    DwTimeLine timeline;
    int eotl;
    // this delay tells how long to wait after the
    // last packet is sent out before returning
    // error on the "control" channel. kludge for
    // autostop. auto_stop_delay = 0 means don't
    // do auto-stop. generally it needs to be set
    // to the amount of buffering in the top level
    // audio device (ie, the amount of time you
    // would expect a packet to show up at the output
    // of the device after it has been put into the
    // top level device).
    int auto_stop_delay;
    DwTimer kludge_timer;
    int kludge_done;

    virtual int read_mm_data(DWBYTE*& buf, int& len, int& chan, int& subchan,
                             int& user, int user_len, int& seq, long& time, long& buf_offset, int inhibit_read);
    virtual void load_q();
    virtual void index_time_data() {}
    int first_time;

private:
// this is just so noone uses the filetube interface. everyone uses
// the slippytube interface at this point.
    friend class SlippyTube;
    FileTube(const DwString &filename, const DwString &mode, enum dir);
    FileTube(FILE *, enum dir, int we_close = 1);
    ~FileTube();

public:

    virtual int reset_timer(int);
    void set_auto_stop_delay(int);
    void set_timeline_speed(double);

    virtual int generates_events();
    virtual int is_network() {
        return 0;
    }
    virtual void set_acquisition_time(long time) {
        acq_time = time;
    }

    // primitive link status
    virtual int has_mmdata();
    virtual int can_write_mmdata();
    virtual int has_ctrl();
    virtual int can_write_ctrl();

    virtual int has_data(int chan);
    virtual int can_write_data(int chan);

    // multimedia data
    virtual int send_mm_data(DWBYTE *buf, int len, int chan = 0,
                             int user_byte = 0, int user_len = 1, int seq = 0);
    virtual int recv_mm_data(DWBYTE *&buf, int& len, int& chan = MMTube::dummy,
                             int& user = MMTube::dummy, int user_len = 1, int& user_seq = MMTube::always_zero);

    // this assumes the chan is one socket, and subchan is app
    // either multiplexing onto that socket or used as a payload type.
    virtual int get_mm_data(DWBYTE *&buf, int& len, int chan,
                            int& subchan = MMTube::dummy, int& user = MMTube::dummy,
                            int user_len = 1, int& seq = MMTube::always_zero);
    virtual int put_mm_data(DWBYTE *buf, int len, int chan, int subchan = 0,
                            int user_byte = 0, int user_len = 1, int seq = 0);

    // special control protocol handling
    virtual int send_ctrl_data(vc v);
    virtual int recv_ctrl_data(vc& v);

    virtual void flush();

    //enum wbok {WB_NOK = 0, WB_OK = 1};
    // set this to limit the number of packets recorded in a file.
    int packet_count;
    // set this to limit the number of bytes recorded to a file.
    int bytes_left;
    virtual int full();

    // timer, rate control
    virtual int tick();
};


class DummyTube : public MMTube
{
    friend class MMChannel;

private:
    vc remote;

public:
    DummyTube();
    ~DummyTube();

    // link setup and destroy
    virtual int connect(const char *remote_addr, const char *local_addr, int block = 0, HWND = 0, int setup_unreliable = 0);
    virtual int gen_channel(unsigned short remote_port, int& chan);

    // primitive link status
    virtual int has_mmdata();
    virtual int can_write_mmdata();
    virtual int has_ctrl();
    virtual int can_write_ctrl();

    // special control protocol handling
    virtual int send_ctrl_data(vc v);
    virtual int recv_ctrl_data(vc& v);
};

// a filetube that allows you to slip the timing of
// channels. that way we can provide lip sync for
// audio and video.

class SlippyTube : public FileTube
{
    friend class MMChannel;
public:

    SlippyTube(const DwString& filename, const DwString& mode, enum dir);
    //SlippyTube(FILE *, enum dir, int we_close = 1);
    ~SlippyTube();
    static int is_dwyco_media(const DwString& filename);

    void set_chan_delay(int chan, int delay);
    //virtual int read_mm_data(DWBYTE*& buf, int& len, int& chan, int& subchan,
    //int& user, int user_len, int& seq, long& time, long& filepos, int inhibit);
    virtual int tick();
    int use_old_timing;
    // set to 1 before record to tell players to use the old timing (if
    // the cap timing is messed up, for example).
    int prefer_old_timing;
    // set to one to override the preferred timing set by the
    // recorder.
    int force_timing;
    int reset_timer(int);
    void set_chan_enables(int video, int audio);
    int quick_stats(int max_read, int stop_after_n_video, int stop_after_n_audio, int& video_out, int& audio_out, int& num_clips_out, int& coder, DWBYTE** last_vid_buf = 0, int *last_vid_buf_len = 0);
    // set to the number of frames you want to index
    // useful for previewing and such where you only want to
    // look at the first few frames of video.
    int vid_frames_to_index;

private:
    void index_time_data();
    DwVec<int> chan_delays;
    DwVec<mmblock> vec;
    int max_search;
    int idx;
    int enable_video;
    int enable_audio;
};


#endif
