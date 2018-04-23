
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/audo.cc 1.19 1999/01/10 16:09:26 dwight Checkpoint $
 */
#include "gvchild.h"
#include "audo.h"
#include "audwin.h"
#include "mmchan.h"
#include "audexto.h"
#include "doinit.h"
using namespace dwyco;

AudioOutput *TheAudioOutput;
int ExternalAudioOutput;
extern CRITICAL_SECTION Audio_lock;
namespace dwyco {
extern double Audio_delay;
}

#define FAILRET(s) { \
	if(locally_invoked) \
	{ \
		(*MMChannel::popup_message_box_callback)(0, s, vcnil, vcnil); \
	} \
	Log->make_entry(s); \
	return 0; \
}

int
init_external_audio_output(int locally_invoked)
{
    TheAudioOutput = new AudioOutputExternal;
    int bufs = (1000 * Audio_delay) /
               TheAudioOutput->device_one_buffer_time();
    if(bufs >= 0 && bufs <= 200)
        TheAudioOutput->change_buffering(bufs);
    // note: we always try to fire up the output even
    // if the hardware isn't full duplex: the acquire
    // side is normally off and output is normally on...
    if(!TheAudioOutput->init())
    {
        delete TheAudioOutput;
        TheAudioOutput = 0;
        FAILRET("can't open audio output");
    }
    return 1;
}

int
init_audio_output(int locally_invoked)
{
    if(TheAudioOutput)
        return 1;

    if(ExternalAudioOutput)
    {
        return init_external_audio_output(locally_invoked);
    }
#ifdef _Windows
    TheAudioOutput = new AudioOutputWin32PCM16(Audio_lock, 0, 5, AUDBUF_LEN);
    int bufs = (1000 * Audio_delay) /
               TheAudioOutput->device_one_buffer_time();
    if(bufs >= 0 && bufs <= 200)
        TheAudioOutput->change_buffering(bufs);
    // note: we always try to fire up the output even
    // if the hardware isn't full duplex: the acquire
    // side is normally off and output is normally on...
    if(!TheAudioOutput->init())
    {
        delete TheAudioOutput;
        TheAudioOutput = 0;
        FAILRET("can't open audio output");
    }
    return 1;
#else
    return 0;
#endif
}

int
exit_audio_output()
{
    if(!TheAudioOutput)
        return 1;
    TheAudioOutput->reset();
    // kluge: find channel that references what we are about to
    // delete and zero it out
    MMChannel *m = MMChannel::find_audio_player(1);
    if(m)
        m->audio_output = 0;
    delete TheAudioOutput;
    TheAudioOutput = 0;
    return 1;
}
