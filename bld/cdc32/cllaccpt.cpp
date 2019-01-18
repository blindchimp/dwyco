
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/cllaccpt.cpp 1.16 1999/01/10 16:09:57 dwight Checkpoint $
 */

#include "cllaccpt.h"
#include "doinit.h"
#include "audchk.h"
#include "aq.h"
#include "qauth.h"
#ifdef LINUX
#include "miscemu.h"
#endif

CallAcceptanceXfer CallAcceptanceData;


#define CALL_ACCEPTANCE_SECTION "call accept"
#define CA_MAX_AUDIO_RECV "max_audio_recv"
#define CA_MAX_VIDEO_RECV "max_video_recv"
#define CA_MAX_AUDIO "max_audio"
#define CA_MAX_VIDEO "max_video"
#define CA_MAX_CHAT "max_chat"
#define CA_MAX_PCHAT "max_pchat"
#define CA_AUTO_ACCEPT "auto_accept"
#define CA_REQUIRE_PW "require_pw"
#define CA_PW "pw"

#define DEFAULT_MAX_AUDIO_RECV "4"
#define DEFAULT_MAX_VIDEO_RECV "4"
#define DEFAULT_MAX_AUDIO "4"
#define DEFAULT_MAX_VIDEO "4"
#define DEFAULT_MAX_CHAT "100"
#define DEFAULT_MAX_PCHAT "4"
#define DEFAULT_AUTO_ACCEPT BF_UNCHECKED
#define DEFAULT_REQUIRE_PW BF_UNCHECKED
#define DEFAULT_PW ""

CallAcceptanceXfer::CallAcceptanceXfer()
DWUIINIT_CTOR_BEGIN,
DWUIINIT_CTOR_VAL(max_audio),
DWUIINIT_CTOR_VAL(max_chat),
DWUIINIT_CTOR_VAL(max_video),
DWUIINIT_CTOR_VAL(max_audio_recv),
DWUIINIT_CTOR_VAL(max_video_recv),
DWUIINIT_CTOR_VAL(max_pchat),
DWUIINIT_CTOR_VAL(pw),
DWUIINIT_CTOR_VAL(auto_accept),
DWUIINIT_CTOR_VAL(require_pw)
DWUIINIT_CTOR_END
{
    set_max_audio_recv(DEFAULT_MAX_AUDIO_RECV);
    set_max_video_recv(DEFAULT_MAX_VIDEO_RECV);
    set_max_audio(DEFAULT_MAX_AUDIO);
    set_max_video(DEFAULT_MAX_VIDEO);
    set_max_chat(DEFAULT_MAX_CHAT);
    set_max_pchat(DEFAULT_MAX_PCHAT);
    set_auto_accept(DEFAULT_AUTO_ACCEPT);
    set_require_pw(DEFAULT_REQUIRE_PW);
    set_pw(DEFAULT_PW);
}

void
CallAcceptanceXfer::load()
{
    load_syncmap(syncmap, CALL_ACCEPTANCE_SECTION ".dif");
}


void
CallAcceptanceXfer::save()
{
    save_syncmap(syncmap, CALL_ACCEPTANCE_SECTION ".dif");
}

