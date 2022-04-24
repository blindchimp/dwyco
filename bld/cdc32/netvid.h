
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/netvid.h 1.24 1999/01/10 16:10:51 dwight Checkpoint $
 */
#ifndef NETVID_H
#define NETVID_H
#include <windows.h>
#include "matcom.h"
#include "netcod.h"
#include "dwtimer.h"
#include "dwvecp.h"
#include "dwstr.h"
#include "vccrypt2.h"
#include "pval.h"
#include "servass.h"
#include "ssns.h"

// error returns
// op not completed: <0
// op can't be completed: -1
// op might be completed if you try again: -2
// op ok, >= 0
// this is screaming for exception handling, but
// since not all compilers have it now, i'm hosed...
//

#define SSERR (-1)
#define SSTRYAGAIN (-2)

#define FIRST_UNRELIABLE_CHAN 2
#define FIRST_RELIABLE_CHAN 4
class vc;
//class ValidPtr;
class MMChannel;

class MMTube
{
    friend class MMChannel;
    friend void dwyco::serv_recv_online(MMChannel *mc, vc prox_info, void *, ValidPtr mcv);

protected:
    SimpleSocket *ctrl_sock;
    FrameSocket *mm_sock;

    DwVecP<SimpleSocket> socks;

    unsigned long tick_time;	// in ms
    unsigned long time;
    DwTimer keepalive_timer;
    unsigned long last_tick;
    int connected;
    static int dummy;
    static int always_zero;
    // indexed by channel, number of bits theoretically
    // left in output buffer
    DwVec<long> in_bits;
    DwVec<long> baud;		// in bits/sec, indexed by channel

public:
    MMTube();
    virtual ~MMTube();

    // last socket errors
    vc last_ctrl_error;
    vc last_mm_error;

    virtual int is_connected();

    // returns 1 if the tube generates events that
    // trigger the apps main loop. otherwise, the
    // app has to poll and get events elsewhere.
    virtual int generates_events() {
        return 1;
    }

    // true if we are writing to a network object
    virtual int is_network() {
        return 1;
    }

    // set the actual acquisition time to be encapsulated
    // in the sent packet. this is used mainly in
    // the filetube subclass for now, so allow proper
    // sync on playback (which isn't done for network
    // connections at this time.)
    virtual void set_acquisition_time(long time) {}

    // link setup and destroy
    virtual int connect(const char *remote_addr, const char *local_addr, int block = 0, HWND = 0, int setup_unreliable = 0);
    int accept(SimpleSocket *);
    int disconnect();
    int disconnect_ctrl();
    vc remote_addr_ctrl();

    void set_channel(SimpleSocket *, int enc, int dec, int chan);
    void drop_channel(int chan);
    virtual int gen_channel(unsigned short remote_port, int& chan);
    SimpleSocket *get_chan_sock(int chan);

    // primitive link status
    virtual int has_mmdata();
    virtual int can_write_mmdata();
    virtual int has_ctrl();
    virtual int can_write_ctrl();
    int is_disconnected();

    virtual int has_data(int chan);
    virtual int can_write_data(int chan);

    // link flow control
    // note: model assumes simple full-duplex
    // connection with both control and data
    // multiplexed onto the same link. also
    // assumes underlying protocol utilizes
    // full bandwidth of link, with no
    // assumptions about compression/errors, etc.
    // there is no receive buffer control. all control is
    // done by sender. there is no attempt to
    // control delay (at this level.)
    //
    // bandwidth allocation can be performed by
    // users of this class on a per-channel basis
    // for the "mm" side of the physical link.
    // the control channel's bandwidth requirements
    // are assumed to be so small that it isn't
    // worth including it in the overall total of bandwidth
    // requirements. it is up to the client of this class
    // to keep the total bandwidth consistent with
    // all the different channels.

    // can never tell with inet whether there is more space left,
    // so we have to implement pseudo timers and rate control
    // assuming we're the only one using the link and that
    // bits get pushed out as fast as the bitrate for the channels
    // indicates.
    virtual int buf_drained(int chan = 0);
    virtual int clear_buf_ctrl(int chan = 0);

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

    int dropped_mm_packets();
    void reset_dropped_mm_packets();

    // special control protocol handling
    virtual int send_ctrl_data(vc v);
    virtual int recv_ctrl_data(vc& v);

    enum wbok {WB_NOK = 0, WB_OK = 1};
    int send_data(vc v, int chan, int bwchan);
    int send_data(vc v, int chan);
    //int send_data(DWBYTE *buf, int len, int chan, enum wbok = WB_NOK);
    // this version allows you to say what device
    // gets the data, and what channel gets the b/w tally for it.
    // what a hack.
    //int send_data(DWBYTE *buf, int len, int chan, int bwchan, enum wbok = WB_NOK);
    int recv_data(vc& v, int chan);
    //int recv_data(DWBYTE *&buf, int& len, int chan);

    int init_mm_listener(int port);

    // timer, rate control
    virtual int tick();
    unsigned long set_est_baud(unsigned long baud, int chan = 0);
    unsigned long get_est_baud(int chan = 0);

    void set_keepalive_time(unsigned long);
    void set_keepalive(int);

    void mm_packet_stats(int& dropped, int& s, int& r);
    void set_mm_packet_stats(int dropped, int s, int r);
    // used because we are still using the old version
    // mm_sock for unreliable video at this point.
    void set_mm_sock(FrameSocket *fs);
    FrameSocket *get_mm_sock();
    int reconnect_mm_sock(const char *remote_addr);
    int reconnect_chan(const char *remote_addr, int chan);

    int find_chan();

    virtual void flush() {} // used to flush pending output from a tube.
    virtual int full() {
        return 0;   // used in file subclasses when size limiting is in place.
    }

private:
    vc enc_ctx;
    vc dec_ctx;
    DwVec<int> enc_chan;
    DwVec<int> dec_chan;
    int enc_ctrl;
    int dec_ctrl;

public:
    int set_key_iv(vc key, vc iv);
    int start_decrypt_chan(int chan);
    int start_encrypt_chan(int chan);
    int start_encrypt_ctrl();
    int start_decrypt_ctrl();
    int get_encrypt_ctrl();
    int get_decrypt_ctrl();
    int get_encrypt_chan(int chan);
    int get_decrypt_chan(int chan);

private:
    static vc Ping;
    int keepalive();
    void toss();

    vc tubeid;
    // in bytes
    long total_sent;
    long total_recv;
public:
    vc mklog(vc = vcnil, vc = vcnil, vc = vcnil, vc = vcnil,
             vc = vcnil, vc = vcnil, vc = vcnil, vc = vcnil);
    ssns::signal1<vc> log_signal;


};


#endif
