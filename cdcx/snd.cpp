
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <string.h>
#include "dlli.h"
#include "ssmap.h"
#include "ct.h"
#if 0 && defined(LINUX) && !defined(MAC_CLIENT)
#include <SDL/SDL.h>
#include <unistd.h>
#include "esd.h"
#include "dwstr.h"
#include <QMap>

// under linux, we'll just assume we have Esd for now
//
static int Esd = -1;
static QMap<DwOString, int> SoundMap;

void
init_sound()
{
    Esd = esd_open_sound(0);
    SoundMap.clear();
}

static void
load_sound(const char *filename)
{
    Uint32 len;
    Uint8 *buf;
#ifndef MAC_CLIENT
    SDL_AudioSpec spec;
    if(SDL_LoadWAV(filename, &spec, &buf, &len) == 0)
        return;

    int ef = ESD_STREAM | ESD_PLAY;
    if(Esd == -1)
        return;
    switch(spec.format)
    {
    case AUDIO_S8:
    case AUDIO_U8:
        ef |= ESD_BITS8;
        break;
    case AUDIO_S16:
        ef |= ESD_BITS16;
        break;

    default:
        return;
    }
    if(spec.channels == 1)
        ef |= ESD_MONO;
    else
        ef |= ESD_STEREO;

    int id = esd_sample_cache(Esd, ef, spec.freq, len, filename);
    write(Esd, buf, len);
    int id2 = esd_confirm_sample_cache(Esd);
    if(id != id2)
        return;
    SoundMap.insert(filename, id);
#endif
}

void
play_sound_plat(const char *file)
{
    int id;
    if(!SoundMap.contains(file))
    {
        load_sound(file);
        if(!SoundMap.contains(file))
            return;
    }
    id = SoundMap.value(file);
    esd_sample_play(Esd, id);
}

void
exit_sound()
{
    QList<int> v = SoundMap.values();
    int n = v.count();
    for(int i = 0; i < n; ++i)
    {
        esd_sample_free(Esd, v[i]);
    }
    SoundMap.clear();
    close(Esd);
    Esd = -1;
}

#elif defined(WIN32)
#include <windows.h>
void
play_sound_plat(const char *fn)
{

    PlaySoundA(fn, NULL, SND_ASYNC|SND_FILENAME|SND_NOWAIT|SND_NOSTOP);
}

void
init_sound()
{
}

void
exit_sound()
{
}
#else

#include <QSound>
#include "pfx.h"

void
play_sound_plat(const char *fn)
{
    DwOString fpx = add_pfx(User_pfx, fn);
    QSound::play(fpx.c_str());
}

void
init_sound()
{
}

void
exit_sound()
{
}

#endif

void
play_sound(const char *fn)
{
    int val = 0;
    if(!setting_get("mute_alerts", val))
    {
        val = 0;
    }
    if(val)
        return;
    play_sound_plat(fn);
}

void
DWYCOCALLCONV
dwyco_alert_callback(const char *cmd, void *, int, const char *)
{
    if(call_exists_by_type("private", DWYCO_CT_ANY))
        return;

    if(strcmp(cmd, "alert_online") == 0)
    {
        play_sound("relaxed-online.wav");
    }
    else if(strcmp(cmd, "alert_call") == 0)
    {
        play_sound("relaxed-call.wav");
    }
}
