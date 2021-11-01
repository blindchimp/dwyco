
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "sysattr.h"
#include "vc.h"

static vc Defaults;
static vc Attrs;

void
init_sysattr()
{
    Defaults = vc(VC_TREE);
    // note: this was originally used for audio/video channels. sync channels
    // don't really follow this, but i think tally the data into the
    // video channel, which is probably wrong. the whole thing needs to
    // be revisited and fixed.
    Defaults.add_kv("us-proxy-bw-limit", 384);
#ifdef UWB_SAMPLING
    Defaults.add_kv("us-audio-sample-rate", UWB_SAMPLE_RATE);
#else
    Defaults.add_kv("us-audio-sample-rate", 8000);
#endif
    Defaults.add_kv("us-audio-coder", "ms-gsm610");
    Defaults.add_kv("us-audio-coder-chats", "ms-gsm610");
    Defaults.add_kv("us-audio-coder-calls", "ms-gsm610");
    Defaults.add_kv("us-audio-coder-qms", "ms-gsm610");
    Defaults.add_kv("us-server-rules",
                    "The use of this server is subject to the Terms of Service."
                   );
    Defaults.add_kv("us-sub-expired", vcnil);
    Defaults.add_kv("us-video-coder", "theora");
    Defaults.add_kv("us-video-coder-chats", "theora");
    Defaults.add_kv("us-video-coder-calls", "theora");
    Defaults.add_kv("us-video-coder-qms", "theora");

    // these are queried by the theora encoder stuff when getting
    // setup. they are mostly for testing, and probably shouldn't
    // be tweaked during production, as i'm not sure how the streaming
    // will react if these are tweaked midstream.
    Defaults.add_kv("us-video-coder-theora-quality-upper", 63);
    Defaults.add_kv("us-video-coder-theora-quality-lower", 0);
    Defaults.add_kv("us-video-coder-theora-quality-step", 5);
    Defaults.add_kv("us-video-coder-theora-quality-cbr", 0);

    Attrs = vc(VC_TREE);
}

int
sysattr_get_int(const char *name, int def)
{
    vc ret;
    if(!Attrs.find(name, ret) ||
            ret.type() != VC_INT)
    {
        if(!Defaults.find(name, ret))
        {
            return def;
        }
    }
    return (int) ret;
}

vc
sysattr_get_vc(const char *name)
{
    vc ret;
    if(!Attrs.find(name, ret))
    {
        if(!Defaults.find(name, ret))
        {
            oopanic("asking for bad sysattr");
        }
    }
    return ret;
}

void
sysattr_put(vc name, vc value)
{
    Attrs.add_kv(name, value);
}

