
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/aqpgm.cc 1.7 1997/11/25 20:41:03 dwight Stable095 $
 */
#include "aqext.h"
#include "dlli.h"
#include "vc.h"

//using namespace dwyco;

extern DwycoVVCallback dwyco_vidacq_new;
extern DwycoVVCallback dwyco_vidacq_delete;
extern DwycoIVICallback dwyco_vidacq_init;
extern DwycoIVCallback dwyco_vidacq_has_data;
extern DwycoVVCallback dwyco_vidacq_need;
extern DwycoVVCallback dwyco_vidacq_pass;
extern DwycoVVCallback dwyco_vidacq_stop;
extern DwycoVidGetDataCallback dwyco_vidacq_get_data;
extern DwycoVVCallback dwyco_vidacq_free_data;

ExtAcquire::ExtAcquire()
{
#if defined(MACOSX) || defined(LINUX)
    vidconv.set_format(AQ_RGB24);
    vidconv.add_style(AQ_COLOR);
    vidconv.swap_rb = 1;
    vidconv.set_upside_down(0);
#else
    vidconv.set_format(AQ_RGB24);
    vidconv.add_style(AQ_COLOR);
    vc srb = get_settings_value("video_format/swap_rb");
    if(srb.type() != VC_INT)
        vidconv.swap_rb = 0;
    else
        vidconv.swap_rb = (int)srb;
    vidconv.set_upside_down(1);
#endif
    if(dwyco_vidacq_new)
        (*dwyco_vidacq_new)(this);
}

ExtAcquire::~ExtAcquire()
{
    if(dwyco_vidacq_delete)
        (*dwyco_vidacq_delete)(this);
}

void
ExtAcquire::set_swap_rb(int s)
{
#if defined(MACOSX) || defined(LINUX)
// should really standardize a bit better here. this is in here
// mainly because of the macosx driver being a little goofy i think.
    vidconv.swap_rb = !s;
#else
    vidconv.swap_rb = s;
#endif
}

int
ExtAcquire::generates_events()
{
    // NOTE: you HAVE to arrange for a message to come
    // in on every capture in order to kick the idle
    // loop out, otherwise, you will get whacky capture.
    // if you set this to 0, you will get all the frames, but
    // the main service loop will spin because it can't
    // figure out when the frames might come in.
    return 1;
}

int
ExtAcquire::init(int frame_rate)
{
    if(dwyco_vidacq_init)
        return (*dwyco_vidacq_init)(this, frame_rate);
    return 0;
}

int
ExtAcquire::has_data()
{
    if(dwyco_vidacq_has_data)
        return (*dwyco_vidacq_has_data)(this);
    return 0;
}

void
ExtAcquire::need()
{
    if(dwyco_vidacq_need)
        (*dwyco_vidacq_need)(this);
}

void
ExtAcquire::pass()
{
    if(dwyco_vidacq_pass)
        (*dwyco_vidacq_pass)(this);
}

void
ExtAcquire::stop()
{
    if(dwyco_vidacq_stop)
        (*dwyco_vidacq_stop)(this);
}

void *
ExtAcquire::get_data(int& c, int& r, void*& y, void*& cb, void *& cr,
                     int& fmt, unsigned long& captime, int no_convert)
{
    if(!dwyco_vidacq_get_data)
        return 0;
    // the external get_data call should return just raw bits
    // which we will copy at this point and convert into a
    // form the codecs can use. doing it this way avoids problems
    // with mixing allocations from an external source, with
    // the codec eventually freeing the data when it is done
    // with it (which causes problems when stuff is linked
    // with other copies of the allocator and so on in dll's.)
    int bytes;
    void *raw_data =
        (*dwyco_vidacq_get_data)(this, &c, &r, &bytes, &fmt, &captime);
    vidconv.set_format(fmt);

    void *ret = vidconv.convert_data((unsigned char *)raw_data, bytes, c, r, y, cb, cr, fmt, no_convert);

    if(dwyco_vidacq_free_data)
        (*dwyco_vidacq_free_data)(raw_data);
    return ret;

}

