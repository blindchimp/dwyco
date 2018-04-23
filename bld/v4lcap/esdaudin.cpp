
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "dlli.h"
#include "aqextaud.h"
#include "esdaudin.h"
#include <windows.h>
#include "esdaqaud.h"


void
DWYCOCALLCONV
esd_new(void *e, int buflen, int nbufs)
{
    ExtAudioAcquire *ea = (ExtAudioAcquire *)e;
    EsdAqAud *esd = new EsdAqAud(buflen, nbufs);
    ea->user_data = esd;
}

void
DWYCOCALLCONV
esd_delete(void *e)
{
    ExtAudioAcquire *ea = (ExtAudioAcquire *)e;
    EsdAqAud *esd = (EsdAqAud *)ea->user_data;
    delete esd;
}

int
DWYCOCALLCONV
esd_init(void *e)
{
    ExtAudioAcquire *ea = (ExtAudioAcquire *)e;
    EsdAqAud *esd = (EsdAqAud *)ea->user_data;
    return esd->init();
}

int
DWYCOCALLCONV
esd_has_data(void *e)
{
    ExtAudioAcquire *ea = (ExtAudioAcquire *)e;
    EsdAqAud *esd = (EsdAqAud *)ea->user_data;
    return esd->has_data();
}

void
DWYCOCALLCONV
esd_need(void *e)
{
    ExtAudioAcquire *ea = (ExtAudioAcquire *)e;
    EsdAqAud *esd = (EsdAqAud *)ea->user_data;
    esd->need();
}

void
DWYCOCALLCONV
esd_pass(void *e)
{
    ExtAudioAcquire *ea = (ExtAudioAcquire *)e;
    EsdAqAud *esd = (EsdAqAud *)ea->user_data;
    esd->pass();
}

void
DWYCOCALLCONV
esd_stop(void *e)
{
    ExtAudioAcquire *ea = (ExtAudioAcquire *)e;
    EsdAqAud *esd = (EsdAqAud *)ea->user_data;
    esd->stop();
}

void
DWYCOCALLCONV
esd_on(void *e)
{
    ExtAudioAcquire *ea = (ExtAudioAcquire *)e;
    EsdAqAud *esd = (EsdAqAud *)ea->user_data;
    esd->on();
}

void
DWYCOCALLCONV
esd_off(void *e)
{
    ExtAudioAcquire *ea = (ExtAudioAcquire *)e;
    EsdAqAud *esd = (EsdAqAud *)ea->user_data;
    esd->off();
}

void
DWYCOCALLCONV
esd_reset(void *e)
{
    ExtAudioAcquire *ea = (ExtAudioAcquire *)e;
    EsdAqAud *esd = (EsdAqAud *)ea->user_data;
    esd->reset();
}

int
DWYCOCALLCONV
esd_status(void *e)
{
    ExtAudioAcquire *ea = (ExtAudioAcquire *)e;
    EsdAqAud *esd = (EsdAqAud *)ea->user_data;
    return esd->status();
}

void *
DWYCOCALLCONV
esd_get_data(void *e, int *r, int *c)
{
    ExtAudioAcquire *ea = (ExtAudioAcquire *)e;
    EsdAqAud *esd = (EsdAqAud *)ea->user_data;
    return esd->get_data(*r, *c);
}

