
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef AQEXTAUD_H
#define AQEXTAUD_H
#include "genaqaud.h"

// acquire audio data from an external source. this is used to
// support the dwyco DLL
class ExtAudioAcquire : public AudioAcquire
{
public:
    ExtAudioAcquire(int bufsize, int buflen);
    ~ExtAudioAcquire();
    virtual int init();
    void need();
    void pass();
    int has_data();
    void stop();
    void on();
    void off();
    void reset();
    int status();
    void *get_data(int& cols, int& rows);
    void *user_data;
private:
};

#endif
