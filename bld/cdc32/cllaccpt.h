
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/cllaccpt.h 1.16 1999/01/10 16:10:19 dwight Checkpoint $
 */
#if !defined(__cllaccpt_h)              // Sentry, use file only if it's not already included.
#define __cllaccpt_h

#include "uicfg.h"
#ifdef LINUX
#include "miscemu.h"
#endif

//{{TDialog = CallAcceptance}}
struct CallAcceptanceXfer {
private:
//{{CallAcceptanceXFER_DATA_END}}
public:
    CallAcceptanceXfer();
    void load();
    void save();
    DWUIDECL_BEGIN
    DWUIDECLVAL_char_int(const char *, max_audio)
    DWUIDECLVAL_char_int(const char *, max_chat)
    DWUIDECLVAL_char_int(const char *, max_video)
    DWUIDECLVAL_char_int(const char *, max_audio_recv)
    DWUIDECLVAL_char_int(const char *, max_video_recv)
    DWUIDECLVAL_char_int(const char *, max_pchat)
    DWUIDECLVAL(const char *, pw)
    DWUIDECLVAL(bool, auto_accept)
    DWUIDECLVAL(bool, require_pw)
    DWUIDECL_END
};

extern CallAcceptanceXfer CallAcceptanceData;
#endif                                      // __cllaccpt_h sentry.

