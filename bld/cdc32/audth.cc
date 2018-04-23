
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/audth.cc 1.13 1999/01/10 16:09:47 dwight Checkpoint $

#include "audth.h"
#include "audmix.h"
#include "audo.h"
#include "audi.h"
#ifdef _Windows
#include <process.h>
#include "audwin.h"
#endif

#ifdef _Windows
static HANDLE Mixer_thread;
#endif
#ifdef LINUX
#include <pthread.h>
static pthread_t Mixer_thread;
#endif

AudioMixer *volatile TheAudioMixer;
extern CRITICAL_SECTION Audio_lock;
CRITICAL_SECTION Audio_mixer_shutdown_lock;


#ifdef _Windows
static void
#endif
#ifdef LINUX
static void *
#endif
mixer_thread(void *)
{
    if(TheAudioMixer)
        TheAudioMixer->audio_mixer_thread();
#ifdef _Windows
    _endthread();
#endif
#ifdef LINUX
    pthread_exit(0);
#endif
}

int
init_audio_mixer()
{
    if(TheAudioMixer)
        return 1;

    TheAudioMixer = new AudioMixer(100, Audio_lock);
    if(!TheAudioMixer)
        return 0;

    TheAudioMixer->set_driver(TheAudioOutput);
    TheAudioMixer->set_audinp(TheAudioInput);
    TheAudioMixer->exit = 0;
    if(!TheAudioOutput)
        return 0;
#ifdef _Windows
    ((AudioOutputWin32 *)TheAudioOutput)->set_mixer(TheAudioMixer);
#endif

    // mixer needs to run in a different thread
    // for best response

#ifdef _Windows
    Mixer_thread = (HANDLE) _beginthread(mixer_thread, 4096, 0);
    if(Mixer_thread == 0 || Mixer_thread == (void *)-1)
    {
        Mixer_thread = 0;
        delete TheAudioMixer;
        TheAudioMixer = 0;
        return 0;
    }
    SetThreadPriority(Mixer_thread, THREAD_PRIORITY_ABOVE_NORMAL);
#endif
#ifdef LINUX
    int ret = pthread_create(&Mixer_thread, (pthread_attr_t *)0, mixer_thread, (void *)0);
    if(ret != 0)
    {
        Mixer_thread = 0;
        delete TheAudioMixer;
        TheAudioMixer = 0;
        return 0;
    }
#endif
    return 1;
}

int
end_mixer_thread()
{
    if(TheAudioMixer)
        TheAudioMixer->exit = 1;
    unsigned long ec = 0;
#ifdef _Windows
    switch(WaitForSingleObject(Mixer_thread, 20000))
    {
    case WAIT_FAILED:
        Mixer_thread = 0;
        return 0;

    case WAIT_TIMEOUT:
        TerminateThread(Mixer_thread, 0);
        //VcError << "mixer thread timeout\n";
        Mixer_thread = 0;
        return 0;

    case WAIT_OBJECT_0:
        if(!GetExitCodeThread(Mixer_thread, &ec))
        {
            Mixer_thread = 0;
            return 1;
        }
        if(ec == STILL_ACTIVE)
        {
            CloseHandle(Mixer_thread);
            Mixer_thread = 0;
            return 0;
        }
        CloseHandle(Mixer_thread);
        Mixer_thread = 0;
        break;
    }
#endif
#ifdef LINUX
    int ret = pthread_join(Mixer_thread, (void **)0);
    Mixer_thread = 0;
#endif
    return 1;
}

int
exit_audio_mixer()
{
    if(!TheAudioMixer)
        return 1;
    EnterCriticalSection(&Audio_mixer_shutdown_lock);
    if(!end_mixer_thread())
    {
        LeaveCriticalSection(&Audio_mixer_shutdown_lock);
        return 0;
    }
#ifdef _Windows
    if(TheAudioOutput)
        ((AudioOutputWin32 *)TheAudioOutput)->set_mixer(0);
#endif
    AudioMixer * volatile tmp = TheAudioMixer;
    TheAudioMixer = 0;
    delete tmp;

    LeaveCriticalSection(&Audio_mixer_shutdown_lock);
    return 1;
}

// this is needed because during teardown, we have situation where
// driver threads might call back into our mixer, and we need to
// ignore those calls while the teardown is in progress.
// some drivers are also problems in that they need to run
// continuously, despite our lack of need for them to
// run. so they might continue calling this callback for awhile
// even if the mixer thread is stopped.

void
mixer_buf_done_callback()
{
    //EnterCriticalSection(&Audio_mixer_shutdown_lock);
    if(TheAudioMixer)
        TheAudioMixer->done_callback();
    //LeaveCriticalSection(&Audio_mixer_shutdown_lock);
}

