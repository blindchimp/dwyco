
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/ratetwkr.h 1.9 1999/01/10 16:10:22 dwight Checkpoint $
 */
#if !defined(__ratetwkr_h)              // Sentry, use file only if it's not already included.
#define __ratetwkr_h


class RateTweakerXfer {
protected:
    char    max_frame_rate[ 255 ];
    char    max_udp_bytes[ 255 ];
    char    ref_interval[ 255 ];
    char    link_speed[ 255 ];
    char    link_speed_recv[ 255 ];
    RateTweakerXfer();
public:
// note: can't make this virtual, since we
// might muck up the layout of the data expected
// by the transfer functions.
    const RateTweakerXfer& operator=(const RateTweakerXfer& r);

    void load();
    void save();

private:
    RateTweakerXfer(const RateTweakerXfer&);
};

struct RateTweakerXferValid : public RateTweakerXfer
{
    double get_max_frame_rate();
    void set_max_frame_rate(double);
    double get_frame_interval();
    void set_frame_interval(double);
    long get_max_udp_bytes();
    void set_max_udp_bytes(long);
    long get_ref_interval();
    void set_ref_interval(long);
    long get_link_speed();
    void set_link_speed(long);
    long get_link_speed_recv();
    void set_link_speed_recv(long);
    const RateTweakerXferValid& operator=(const RateTweakerXferValid& r) {
        return (const RateTweakerXferValid&)RateTweakerXfer::operator=(r);
    }
};


//extern RateTweakerXferValid RTDefaults;
//extern RateTweakerXferValid RTUserDefaults;

#endif                                      // __ratetwkr_h sentry.

