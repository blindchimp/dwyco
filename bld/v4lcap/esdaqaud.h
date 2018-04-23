
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef LINUX
/*
 * $Header: g:/dwight/repo/cdc32/rcs/aqaud.h 1.16 1999/01/10 16:10:39 dwight Checkpoint $
 */
#ifndef ESDAQAUD_H
#define ESDAQAUD_H
#include "matcom.h"
#include "dwvecp.h"
#include "genaqaud.h"

class EsdAqAud : public AudioAcquire
{
public:

    EsdAqAud(int bufsz, int nbufs);
    ~EsdAqAud();

    int init();
    void reset();
    void off();
    void on();
    int status();

    virtual void pass();
    virtual void need();
    virtual int has_data();
    virtual void *get_data(int& len, int& out2);

    unsigned int initial_timestamp;
    unsigned int current_offset;
    DWORD last_off_time;

private:
    int streaming;
    int nb;
    int bufsize;
    int wavein;
    int cibuf;
    int cobuf;
    int is_off;
#ifdef USE_ESDREC
    int pid;
#endif
    DWBYTE *tmpbuf;
    DWBYTE *b2;
    int amtleft;
    int used;

    struct audbuf
    {
        DWBYTE *data;
        int len;
    };
    DwVec<audbuf> bufs;
    DwVecP<DWBYTE> dribble_bufs;
    DwVec<unsigned int> timestamps;

    void *_get_data(int&, int&);
    int _has_data();
    void unload_pipe();
};

#endif
#endif
