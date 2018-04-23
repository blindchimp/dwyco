
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCTRT_H
#define VCTRT_H
// definitions of stuff that is needed by a
// translated LH program at run time.


struct VcBreak
{
    VcBreak() {lev = -1;}
    VcBreak(int i) {lev = i;}
    int lev;

};

struct VcRet
{
    vc retval;
    VcRet(const vc& val) {retval = val;}
};

// program error
struct VcErr
{
    vc specific;
    vc err;
    VcErr(const vc& e, const vc& s) {err = e; specific = s;}
};

// user generated exception
struct VcExc
{
    vc excstr;
    VcExc(const vc& e) {excstr = e;}
};

#endif
