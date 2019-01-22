
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// make it a little easier to update settings in the
// DLL.
// use a notation like NetConfigData/option-name to set/get values
// instead of the cumbersome stuff we have now.
// the whole thing needs to be revamped, but i don't have time to change
// the code now to a more reasonable approach (like just putting everything
// in a big syncmap and using that.)

#include "syncvar.h"
#include "dwstr.h"

#include "aconn.h"
#include "cllaccpt.h"
#include "zapadv.h"
#include "vidinput.h"
#include "vfwinvst.h"
#include "usercnfg.h"
#include "rawfiles.h"
//#include "cnfgdsp1.h"
#include "chatq.h"
#include "aq.h"
#include "aqext.h"

#include "ezset.h"
using namespace dwyco;

static
SyncMap *
name_to_syncmap(DwString name)
{
    if(name.eq("net"))
        return DwNetConfigData.syncmap;
    if(name.eq("call_acceptance"))
        return CallAcceptanceData.syncmap;
    if(name.eq("raw_files"))
        return RawFilesData.syncmap;
    if(name.eq("user"))
        return UserConfigData.syncmap;
    if(name.eq("video_format"))
        return VFWInvestigateData.syncmap;
    if(name.eq("video_input"))
        return VidInputData.syncmap;
    if(name.eq("zap"))
        return ZapAdvData.syncmap;
    return 0;
}

static
const char *
name_to_section(DwString name)
{
    if(name.eq("net"))
        return "firewall.dif";
    if(name.eq("call_acceptance"))
        return "call accept.dif";
    if(name.eq("raw_files"))
        return "raw files.dif";
    if(name.eq("user"))
        return "user.dif";
    if(name.eq("video_format"))
        return "mystery vfw.dif";
    if(name.eq("video_input"))
        return "video input.dif";
    if(name.eq("zap"))
        return "zap.dif";
    return 0;
}


vc
get_settings_value(const char *name)
{
    DwString a(name);
    int i = a.find("/");
    if(i == DwString::npos)
        return vcnil;
    DwString b(a.c_str(), 0, i);
    SyncMap *sm = name_to_syncmap(b);
    if(!sm)
        return vcnil;
    a.remove(0, i + 1);
    vc ret;
    if(!sm->find(a.c_str(), ret))
        return vcnil;
    return ret;
}

int
set_settings_value(const char *name, const char *value)
{
    DwString a(name);
    int i = a.find("/");
    if(i == DwString::npos)
        return 0;
    DwString b(a.c_str(), 0, i);
    SyncMap *sm = name_to_syncmap(b);
    if(!sm)
        return 0;

    a.remove(0, i + 1);
    vc ret;
    if(!sm->find(a.c_str(), ret))
        return 0;
    // assume that all the maps have been set up with the
    // proper types ahead of time, so we can infer what to
    // convert the value to based on the type we found in the
    // map.
    switch(ret.type())
    {
    case VC_INT:
        sm->replace(a.c_str(), atol(value));
        break;
    case VC_STRING:
        sm->replace(a.c_str(), value);
        break;
    default:
    case VC_NIL:
        oopanic("can't have nil values in settings map");
        break;
    }
    save_syncmap(sm, name_to_section(b));
    if(b.eq("call_acceptance") || b.eq("video_input"))
    {
        chatq_send_update_call_accept();
    }
    if(a.eq("swap_rb") && b.eq("video_format"))
    {
        if(TheAq)
        {
            ExtAcquire *ea = dynamic_cast<ExtAcquire *>(TheAq);
            if(ea)
                ea->set_swap_rb(atol(value));
        }
    }
    if(b.eq("net") && a.eq("listen"))
    {
        set_listen_state(0);
        set_listen_state(atol(value));
    }
    if(b.eq("call_acceptance") && a.eq("no_listen"))
    {
        oopanic("deprecated no_listen");
    }
    return 1;
}
