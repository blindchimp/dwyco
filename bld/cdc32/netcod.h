
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/netcod.h 1.18 1999/01/10 16:10:51 dwight Checkpoint $
 */
#ifndef NETCOD_H
#define NETCOD_H
#ifdef DWYCO_CDC_LIBUV
#include "netcod2.h"
#else
#include "vc.h"
#include "matcom.h"
#include "vcxstrm.h"
#include "dwstr.h"

class SimpleSocket
{
    friend class Listener;
    friend class MMChannel; // for debugging
public:
    SimpleSocket();
    SimpleSocket(vc sock);
    virtual ~SimpleSocket();

    static int any_waiting_for_write();
    static int load_write_set();

    vc last_error;

    virtual int init(const char *remote_addr, const char *local_addr = 0, int retry = 0);
    void exit();
    int can_write();
    int has_data(int sec = 0, int usec = 0);
    int active();
    int non_blocking(int);
    int wouldblock();
    int waiting_for_write();

    DwString local_addr();
    DwString peer_addr();

    // vc funs return length in bytes of items
    // read (always > 0), or 0 for fail
    virtual int recvvc(vc& v);
    virtual int sendvc(vc v);

    // these functions return 1 for ok, 0 otherwise
    virtual int sendlc(DWBYTE *, int, int wbok = 0);
    virtual int recvlc(DWBYTE*&, int&);

    virtual int reconnect(const char *remote_addr);


protected:
    // keeps state info around if ops would block
    vcxstream *istream;
    vcxstream *ostream;

    vc sock;
    vc peer;
    vc laddr;

    int noblock;

    void bad_op();

    virtual void initsock();
    int polling_for_connect;
private:
    int working_len;
};

class Listener : public SimpleSocket
{
public:
    int init(const char *, const char *local_addr = 0, int retry = 0);
    int init2(const char *local_addr = 0);
    int accept_ready();
    SimpleSocket *accept();
};

#define MAX_PACKET_SIZE 32767
#define NC_TRYAGAIN (-1)

// this class is for UDP sockets, it can keep a simple
// sequence number to tell if packets have been dropped or
// reordered. there is no attempt to fix droppage/reordering.
// it just can report it to higher levels.
class FrameSocket : public SimpleSocket
{
private:
    DWBYTE *packet_buf;

    unsigned char seq_send;
    unsigned char seq_recv;
    int packets_lost;
    int sent;
    int recvd;

    int recv_seq(vcxstream&, int& seq_out);
    int send_seq(vcxstream&);

    static int dummy;
    static int always_zero;

public:

    int plen;


    FrameSocket(unsigned int len = MAX_PACKET_SIZE);
    FrameSocket(vc sock, unsigned int len = MAX_PACKET_SIZE);
    virtual ~FrameSocket();

    int can_write();
    int send(DWBYTE *buf, int len, int chan = 0,
             int user_byte = 0, int user_len = 1, int user_seq = 0);
    int recv(DWBYTE *&buf, int& len, int& chan = FrameSocket::dummy,
             int& user_out = FrameSocket::dummy, int len_out = 1,
             int& user_seq_out = FrameSocket::always_zero);

    virtual int recvvc(vc& v);
    virtual int sendvc(vc v);
    void initsock();
    int get_packets_lost() {
        return packets_lost;
    }
    void set_packets_lost(int s) {
        packets_lost = s;
    }
    void get_packet_counts(int& sent, int& recv);
    void set_packet_counts(int sent, int recv);
    int reconnect(const char *remote_addr);

    int reason_seq_bad;

};
#endif
#endif
