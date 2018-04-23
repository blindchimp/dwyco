
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef _Windows
/*
 * $Header: g:/dwight/repo/cdc32/rcs/aqaud.h 1.16 1999/01/10 16:10:39 dwight Checkpoint $
 */
#ifndef AQAUD_H
#define AQAUD_H
#include <windows.h>
#include <mmsystem.h>
#include <amstream.h>
#include "matcom.h"
#include "dwvecp.h"
#include "genaqaud.h"

class AudioAquireWin32 : public AudioAcquire
{
public:

    //AudioAquireWin32();
    AudioAquireWin32(int bufsz, int nbufs, CRITICAL_SECTION&);
    ~AudioAquireWin32();

    int init(int bufsize, int nbufs, HWND win = 0);
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
    int aec;

protected:
    virtual int init_format(WAVEFORMATEX&) = 0;

private:
    CRITICAL_SECTION& cs;
    HWAVEIN wavein;
    int streaming;
    int nb;
    int bufsz;

    struct audbuf
    {
        DWBYTE *data;
        int len;
    };
    DwVecP<audbuf> bufs;
    DwVecP<WAVEHDR> wavehdrs;
    DwVecP<DWBYTE> dribble_bufs;
    DwVec<unsigned int> timestamps;

    void *_get_data(int&, int&);
    int _has_data();
};

class AudioAquireWin32PCM8 : public AudioAquireWin32
{
public:
    //AudioAquireWin32PCM8();
protected:
    int init_format(WAVEFORMATEX&);

};

class AudioAquireWin32PCM16 : public AudioAquireWin32
{
public:
    //AudioAquireWin32PCM16();
    AudioAquireWin32PCM16(int a, int b, CRITICAL_SECTION& c)  : AudioAquireWin32(a, b, c) {}
protected:
    int init_format(WAVEFORMATEX&);

};

class AudioAquireWin32GSM : public AudioAquireWin32
{
public:
    //AudioAquireWin32GSM();
protected:
    int init_format(WAVEFORMATEX&);

};
#endif
#endif
