
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "dlli.h"
#include "ssmap.h"
#include "ct.h"

#if defined(WIN32)
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

#if 0
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
#endif
