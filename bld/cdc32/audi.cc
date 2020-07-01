
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/audi.cc 1.19 1999/01/10 16:09:26 dwight Checkpoint $
 */
#include "gvchild.h"
#include "audi.h"
#include "audo.h"
#ifdef _Windows
#include "aqaud.h"
#else
#include "aqextaud.h"
#endif

#include "audchk.h"
#include "audth.h"
#include "audmix.h"
#include "mmchan.h"
#include "doinit.h"

AudioAcquire *TheAudioInput;
int ExternalAudioAcquisition;
extern CRITICAL_SECTION Audio_lock;

#define FAILRET(s) { \
	if(locally_invoked) \
	{ \
		(*MMChannel::popup_message_box_callback)(0, s, vcnil, vcnil); \
	} \
	Log->make_entry(s); \
	return 0; \
}

#include "aqextaud.h"

int
init_external_audio(int mbox)
{

#ifdef UWB_SAMPLING
    int nbufs = 3. / (((double)AUDBUF_LEN / 2) / (UWB_SAMPLE_RATE + 0.));
#else
    int nbufs = 3. / (((double)AUDBUF_LEN / 2) / 8000.);
#endif
    ExtAudioAcquire *a = new ExtAudioAcquire(AUDBUF_LEN, nbufs);

    if(!a->init())
    {
        if(mbox)
        {
            char m[255];
            sprintf(m, "audio capture init failure");
            (*MMChannel::popup_message_box_callback)(0, m, vcnil, vcnil);
        }
        delete a;
        TheAudioInput = 0;
        return 0;
    }

    // unfortunately, some drivers can't synchronously do this
    // so it might end up thinking there is nothing there when
    // it just takes a sec to get it up and running.
    // i'm not sure what platform something other than
    // "init" was needed to determine the probe, so for now,
    // just avoid doing this.
#if 0
    // the init may not actually probe for a device, but we really
    // need to know if there is one out there, so just start it up
    // and check the status real quick
    a->on();
    if(!a->status())
    {
        // device didn't come up
        a->reset();
        delete a;
        TheAudioInput = 0;
        return 0;
    }
    a->reset();
#endif

    TheAudioInput = a;
    return 1;
}

int
init_audio_input(int locally_invoked, int force)
{
    if(TheAudioInput)
        return 1;

    if(ExternalAudioAcquisition)
    {
        return init_external_audio(locally_invoked);
    }


#ifdef UWB_SAMPLING
    int nbufs = 3. / (((double)AUDBUF_LEN / 2) / (UWB_SAMPLE_RATE + 0.));
#else
    int nbufs = 3. / (((double)AUDBUF_LEN / 2) / 8000.);
#endif
#ifdef _Windows
    AudioAquireWin32PCM16 *a = new AudioAquireWin32PCM16(AUDBUF_LEN, nbufs, Audio_lock);
    if((force || (Audio_hw_full_duplex && Audio_full_duplex))
            && !a->init(AUDBUF_LEN, nbufs, (*MMChannel::get_main_window_callback)(0)))
    {
        delete a;
        FAILRET("can't open audio input device");
    }
    TheAudioInput = a;
    return 1;
#else
    return 0;
#endif
}

int
exit_audio_input()
{
    if(!TheAudioInput)
        return 1;
    if(TheAudioMixer)
        TheAudioMixer->set_audinp(0);
    TheAudioInput->reset();
    MMChannel::clean_audio_sampler_refs(TheAudioInput);
    delete TheAudioInput;
    TheAudioInput = 0;
    return 1;
}
