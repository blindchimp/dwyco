
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "aqextaud.h"

#include "dlli.h"

extern DwycoVVIICallback dwyco_audacq_new;
extern DwycoVVCallback dwyco_audacq_delete;
extern DwycoIVCallback dwyco_audacq_init;
extern DwycoIVCallback dwyco_audacq_has_data;
extern DwycoVVCallback dwyco_audacq_need;
extern DwycoVVCallback dwyco_audacq_pass;
extern DwycoVVCallback dwyco_audacq_stop;
extern DwycoVVCallback dwyco_audacq_on;
extern DwycoVVCallback dwyco_audacq_off;
extern DwycoVVCallback dwyco_audacq_reset;
extern DwycoIVCallback dwyco_audacq_status;
extern DwycoAudGetDataCallback dwyco_audacq_get_data;

ExtAudioAcquire::ExtAudioAcquire(int bufsize, int nbufs)
{
    user_data = 0;
    if(dwyco_audacq_new)
        (*dwyco_audacq_new)(this, bufsize, nbufs);
}

ExtAudioAcquire::~ExtAudioAcquire()
{
    if(dwyco_audacq_delete)
        (*dwyco_audacq_delete)(this);
    user_data = 0;
}

int
ExtAudioAcquire::init()
{
    if(dwyco_audacq_init)
        return (*dwyco_audacq_init)(this);
    return 0;
}

int
ExtAudioAcquire::has_data()
{
    if(dwyco_audacq_has_data)
        return (*dwyco_audacq_has_data)(this);
    return 0;
}

void
ExtAudioAcquire::need()
{
    if(dwyco_audacq_need)
        (*dwyco_audacq_need)(this);
}

void
ExtAudioAcquire::pass()
{
    if(dwyco_audacq_pass)
        (*dwyco_audacq_pass)(this);
}

void
ExtAudioAcquire::stop()
{
    if(dwyco_audacq_stop)
        (*dwyco_audacq_stop)(this);
}

void
ExtAudioAcquire::on()
{
    if(dwyco_audacq_on)
        (*dwyco_audacq_on)(this);
}

void
ExtAudioAcquire::off()
{
    if(dwyco_audacq_off)
        (*dwyco_audacq_off)(this);
}

void
ExtAudioAcquire::reset()
{
    if(dwyco_audacq_reset)
        (*dwyco_audacq_reset)(this);
}

int
ExtAudioAcquire::status()
{
    if(dwyco_audacq_status)
        return (*dwyco_audacq_status)(this);
    return 0;
}


void *
ExtAudioAcquire::get_data(int& c, int& r)
{
    if(!dwyco_audacq_get_data)
        return 0;
    return (*dwyco_audacq_get_data)(this, &c, &r);
}

