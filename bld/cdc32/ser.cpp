
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "vc.h"
#include "vccomp.h"
#include "vcxstrm.h"

vc
serialize(vc v)
{
    vcxstream vcx((char *)0, (long)128 * 1024, vcxstream::CONTINUOUS);
    long len;
    vc_composite::new_dfs();
    if(!vcx.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
        return vcnil;
    if((len = v.xfer_out(vcx)) == -1)
        return vcnil;

    vc ret(VC_BSTRING, vcx.buf, vcx.cur - vcx.buf);
    return ret;
}

int
deserialize(vc v, vc& out, long mem_limit)
{
    if(v.type() != VC_STRING)
        return 0;
    vcxstream vcx((const char *)v, v.len(), vcxstream::FIXED);
    if(mem_limit != -1)
        vcx.set_max_memory(mem_limit);

    vc item;
    long len;
    if(!vcx.open(vcxstream::READABLE))
    {
        return 0;
    }
    if((len = item.xfer_in(vcx)) < 0)
    {
        return 0;
    }
    out = item;
    return len;
}
