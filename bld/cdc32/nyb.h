
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef NYB_H
#define NYB_H
// simple dpcm stream object
// $Header: g:/dwight/repo/cdc32/rcs/nyb.h 1.2 1995/10/08 05:55:39 dwight Stable095 $
#include "matcom.h"

class DPCMStream
{
public:
    DPCMStream() {
        init();
    }
    void init() {
        first = 1;
    }
    int dpcm(int n) {
        if(first)
        {
            last_est = n;
            first = 0;
            return n;
        }
        int ret = n - last_est;
        last_est = n;
        return ret;
    }
    int dpcm_recon(int n) {
        if(first)
        {
            last_est = n;
            first = 0;
            return n;
        }
        last_est += n;
        return last_est;
    }

protected:
    int first;
    int last_est;
};

class DPCMStreamIn : public DPCMStream
{
public:
    int get(DWBYTE*& in) {
        if(first)
        {
            last_est = *(signed char *)in++;
            first = 0;
            return last_est;
        }
        last_est += *(signed char *)in++;
        return last_est;
    }
    int get(BITBUFT*& in) {
        DWBYTE *t = (DWBYTE *)in;
        int ret = get(t);
        in = (BITBUFT *)t;
        return ret;
    }
};

class DPCMStreamOut : public DPCMStream
{
private:
    void put(int n, DWBYTE*& out) {
        *out++ = (DWBYTE)n;
    }

public:
    int put_dpcm(int n, DWBYTE*& out) {
        if(first)
        {
            put(n, out);
            last_est = n;
            first = 0;
            return n;
        }
        put(n - last_est, out);
        int ret = n - last_est;
        last_est = n;
        return ret;
    }
    int put_dpcm(int n, BITBUFT*& out) {
        DWBYTE *t = (DWBYTE *)out;
        int ret = put_dpcm(n, t);
        out = (BITBUFT *)t;
        return ret;
    }
    void flush(DWBYTE*& out) {}
    void flush(BITBUFT*& out) {}
};


#endif
