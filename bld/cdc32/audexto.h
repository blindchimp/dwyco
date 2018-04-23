
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef AUDEXTO_H
#define AUDEXTO_H
#include "audout.h"

// note: this class *must* be used as a singleton, since it
// vectors thru static function pointers. this is a because
// of a design setup that requires a c-style interface to the
// DLL, and usually it represents and audio device, which is
// assumed to have only one input.
class AudioOutputExternal : public AudioOutput
{
private:
    static int already;
public:
    AudioOutputExternal();
    ~AudioOutputExternal();

    virtual int device_init();
    virtual int device_output(DWBYTE *buf, int len, int user_data);
    virtual int device_done(DWBYTE*& buf, int& len, int& do_free);
    virtual int device_stop();
    virtual int device_reset();
    virtual int device_close();
    virtual int device_status();
    virtual int device_buffer_time(int sz);
    virtual int device_one_buffer_time();
    virtual int device_play_silence();
    virtual int device_bufs_playing();
//        virtual int on() {return device_init();}
//        virtual int off() {return device_close();}
};

#endif
