
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCDBG_H
#define VCDBG_H
#include "vc.h"
#ifdef VCDBG
// $Header: g:/dwight/repo/vc/rcs/vcdbg.h 1.47 1997/11/27 03:06:00 dwight Stable $
#include "dwvecp.h"
#include "vcio.h"
#include "dwstr.h"
#include "vcsrc.h"

//
// This class is used to store and coordinate debugging
// information for a running LH program.
// It contains a list of events that have occurred, such
// as function calls and eval steps (effectively emulating
// a call stack, but with more information.)
// It also contains a variety of info related to other
// external events such as asynchronous exceptions, etc.
//
class VcDebugNode;

typedef DwVecP<VcDebugNode> VcDbgNodeVec;
typedef DwVec<vc> VcVec;

class VcDebugInfo
{
    friend class VcDebugNode;

private:
    VcDbgNodeVec callstack;
    //VcVec map_bps;
    //VcVec func_bps;

public:
    virtual ~VcDebugInfo() {}
    void backtrace(VcIOHack &);
    void backtrace_brief(VcIO, int start = 0, int num = -1);
    virtual VcDebugNode *get(int n = -1);
    virtual int num_elems() {return callstack.num_elems();}

};

class VcDebugNode
{
public:
    vc info;
    DwString filename;
    long linenum;
    // this is a list of coordinates as determined from the
    // lexer. this is used mainly for producing information
    // about what exprs have been evaluated, for example to
    // provide some simple coverage info.
    const Src_coord_list *src_list;
    // this is the index of the current expr being evaluated.
    // it should refer to the src_list so you can emit
    // some info related to what has been evaluated.
    int cur_idx;

    VcDebugNode();
    VcDebugNode(const vc&);
    virtual ~VcDebugNode();

    virtual void printOn(VcIO) ;
    virtual int has_brief() {return 0;}
    virtual void printOnBrief(VcIO);
};

extern VcDebugInfo VcDbgInfo;

int drop_to_dbg(const char *msg, const char *why);
vc vclh_break_on_call(vc fun);
vc vclh_break_on_access(vc thing);
vc vclh_eval_break_on(vc);
vc vclh_eval_break_off(vc);

#endif
#endif
