
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef GENAQAUD_H
#define GENAQAUD_H
#include <string.h>
// base class for audio acquisition objects.

class AudioAcquire
{
public:
    AudioAcquire() {
        inited = 0;
        //strcpy(fail_reason, "unknown audio problem");
    }
    virtual ~AudioAcquire() {}

    int init_ok() {
        return inited;
    }

    // can't unload frame now, buffer it, or toss it
    // if necessary (like not enough memory.)
    virtual void pass() = 0;

    // can accept more data (fires up acquisition if
    // it has been halted.)
    virtual void need() = 0;

    // stop acquisition (has_data returns 0,
    // pending data is not affected.)
    virtual void stop() {}

    // get rid of pending data
    virtual void flush() {}

    // return 1 if device has some data waiting
    virtual int has_data() = 0;

    // return next acquired thing
    virtual void *get_data(int& out1, int& out2) = 0;

    //void set_fail_reason(const char *a) {strcpy(fail_reason, a);}
    //char *get_fail_reason() {return strdup(fail_reason);}

    // stop all acquisition
    virtual void reset() = 0;

    // temporarily stop acquisition (for half duplex stuff)
    // to be resumed by subsequent "on" call
    virtual void off() = 0;

    // restart "stopped" acquisition
    virtual void on() = 0;

    // return 1 if device is ready to acquire or acquiring
    virtual int status() = 0;

protected:
    int inited;
    //char fail_reason[255];
};

#endif
