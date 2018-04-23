
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/audout.h 1.24 1999/01/10 16:10:42 dwight Checkpoint $
 */
#ifndef AUDOUT_H
#define AUDOUT_H

// this class is used to encasulate output
// of audio to a device in the system.
// The class can invoke a decompressor if
// necessary to decode the samples presented
// to it.
// This class handles most of the timing issues
// related to outputing the audio as well. When
// a client gives a buffer of information to
// an object of this class, it also specifies
// a time at which the audio should be played
// relative to earlier buffers. This allows
// for some buffers to be dropped during transmission
// and played back with gaps.
// When you give a buffer to this class, the class owns
// the buffer, and will free it unless told otherwise.
//
// This class assumes that a subclass implements the
// actual output of samples to a device.

#define AUDDBG
#include "matcom.h"
#include "dwvec.h"
#include "dwvecp.h"
#include "dwtimer.h"
#ifdef AUDDBG
#include "dwrtlog.h"
#endif
#define DEFAULT_AUDIO_BUFS 3
class AudioConverter;
class AudioMixer;

class AudioOutput
{
    friend class AudioMixer;
public:
    AudioOutput(int decompress = 0, int num_bufs = DEFAULT_AUDIO_BUFS);
    virtual ~AudioOutput();

    virtual int init();
    virtual int reset();
    virtual int stop();
    virtual int on();
    virtual int off();
    virtual int status();
    virtual void switch_streams();
    void set_decoder(AudioConverter *);
    AudioConverter *get_decoder();


    virtual int play_timed(DWBYTE *buf, int len, int at_time = 0, int do_free = 1);
    virtual int play_seq(DWBYTE *buf, int len, int seq, int dfree);
    virtual int play_seq_ec(DWBYTE *buf, int len, int seq, int packet_seq, int dfree);
    virtual int change_buffering(int n);
    virtual int tick();

    virtual int device_init() = 0;
    virtual int device_output(DWBYTE *buf, int len, int user_data) = 0;
    virtual int device_done(DWBYTE*& buf, int& len, int& do_free) = 0;
    virtual int device_stop() = 0;
    virtual int device_reset() = 0;
    virtual int device_close() = 0;
    virtual int device_status() = 0;
    virtual int device_buffer_time(int sz) = 0;
    virtual int device_one_buffer_time() = 0;
    virtual int device_play_silence() = 0;
    virtual int device_bufs_playing() {
        return -1;
    }

    // set to 0 if you know the source is reliable
    // (like a file or something) otherwise
    // tries to play with silence blocks to
    // get late coming packets played out right.
    int fancy_recover;

protected:
    int decompress;

private:
    typedef DwVecP<DWBYTE> BUFVEC;
    typedef DwVec<int> LENVEC;
    DwTimer output_timer;
    DwVecP<DWBYTE> bufs;
    DwVec<int> lens;
    DwVec<int> do_free;
    DwVec<long> at_time;
    DwVec<int> decoded;
    int next;
    int next_out;
    unsigned long time_last_buf_played;
    int reset_timer;
    int overlap;
    int max_off;
    int ec_used;
    AudioConverter *decoder;

    int curseq;
    int seq0;
    BUFVEC *seq_bufs[2];
    LENVEC *seq_lens[2];
    DwVecP<DWBYTE> bufs2;
    DwVec<int> lens2;
    DwVec<int> do_free2;

    int bufs_buffered;
    int bufs_to_buffer;
    int buffering;
    int bufs_playing;

    int full();
    int more_bufs();
    void advance_output();
    int play(DWBYTE *buf, int len, int do_free);
    int play_silence();
    int play_seq_to_device();
    int prev(int);
    int play_now();
    int flush_bufs();
    void manage_decode(int);

#ifdef AUDDBG
    int logid;
    static int ln;
#endif
};

#endif
