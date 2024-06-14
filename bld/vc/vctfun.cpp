
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vctfun.h"
#include "vcmap.h"
#include "vctrt.h"


vc_trans_fundef::vc_trans_fundef(vc nm, VCArglist *a, vc fdef, int sty)
    : vc_fundef(nm, sty)
{
    int nargs = a->num_elems();
    bindargs = new DwSVec<vc>;
    // if a vector is passed as the first argument,
    // we assume its is a list of arguments
    // to be broken-out
    if(nargs >= 1 && (*a)[0].type() == VC_VECTOR)
    {
        if(nargs != 2)
        {
            USER_BOMB2("must have only 2 arguments when defining function with vector of args");
        }
        int nvargs = (*a)[0].num_elems();
        for(int i = 0; i < nvargs; ++i)
        {
            (*bindargs).append((*a)[0][i]);
        }
    }
    else
    {
        for(int i = 0; i < nargs; ++i)
            (*bindargs).append((*a)[i]);
    }

    tfundef = fdef;

    // note: at run time, quoted vars are represented by
    // integers (really addresses), so this test doesn't work
    // right. possible it might make sense to differentiate them
    // so other parts of the runtime intended for the interpreter
    // aren't confused.
    if(tfundef.is_atomic())
    {
        if(tfundef.type() == VC_STRING)
        {
            oopanic("can't dynamic compile (yet)");
            //fundef = vc(VC_CVAR, fundef, decrypt ? -fundef.len() : fundef.len()); // try to compile it
        }
#if 0
        else
            fundef = vc(VC_CVAR, fundef); // try to compile it
#endif
    }
    // otherwise, assume it was already compiled
}

vc
vc_trans_fundef::do_function_call(VCArglist *, int suppress_break) const
{
    vc (*p)();
    p = (vc (*)())(long)tfundef;
    try
    {
        return (*p)();
    }
    catch(const VcRet& vcr)
    {
        return vcr.retval;
    }
    catch(const VcExc& vce)
    {
        // note: here is where we would match against our handlers
        // and call backouts (even if there is no match).
        // if there is a match, eval the handler that matches.
        // if there is no match re-throw
    }
    oopanic("unimplemented translation");
    return vcnil;
}

