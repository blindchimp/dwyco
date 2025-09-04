#ifndef DWYCO_NO_ACQ_VIDEO_MEDIA
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "aqext_android.h"
#include "dlli.h"
#include "vc.h"
#include "ezset.h"

using namespace dwyco;

extern DwycoVidGetDataCallback dwyco_vidacq_get_data;
extern DwycoVVCallback dwyco_vidacq_free_data;
extern int VGQT_swap_rb;
extern int VGQT_flip;

int
ExtAcquireAndroid::init(int frame_rate)
{
    int ret = ExtAcquire::init(frame_rate);

    vc srb = get_settings_value("video_format/swap_rb");
    if(srb.type() != VC_INT)
        set_swap_rb(0);
    else
        set_swap_rb((int)srb);

    srb = get_settings_value("video_format/flip");
    if(srb.type() != VC_INT)
        set_flip(0);
    else
        set_flip((int)srb);

    return ret;
}

void
ExtAcquireAndroid::set_swap_rb(int s)
{
    VGQT_swap_rb = s;
}

void
ExtAcquireAndroid::set_flip(int s)
{
    VGQT_flip = s;
}


void *
ExtAcquireAndroid::get_data(int& c, int& r, void*& y, void*& cb, void *& cr,
                            int& fmt, unsigned long& captime, int no_convert)
{
    if(!dwyco_vidacq_get_data)
        return 0;

    int bytes;

    // raw_data will be the address of 3 gray planes, hack but ok for now
    // raw_data is actually a pointer to a "finished" struct, whose
    // first member is pointer to the planes. this is really gross.
    void *raw_data = (*dwyco_vidacq_get_data)(this, &c, &r, &bytes, &fmt, &captime);
    gray ***p = (gray ***)raw_data;
    y = p[0];
    cb = p[1];
    cr = p[2];

    // on android, we are trying to do as much stuff as possible in
    // other threads, so we assume nv21 from the capture
    // device, and the driver does the conversion to yuv12.
    // the memory is allocated by the same runtime, so there is
    // no need for another copy, we just pass it back and the
    // caller is responsible for freeing.

    //vidconv.set_format(fmt);

    //void *ret = vidconv.convert_data((unsigned char *)raw_data, bytes, c, r, y, cb, cr, fmt, no_convert);

    if(dwyco_vidacq_free_data)
        (*dwyco_vidacq_free_data)(raw_data);
    return y;

}

#endif
