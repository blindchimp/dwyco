
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCTFUN_H
#define VCTFUN_H
#include "vcfundef.h"

class vc_trans_fundef : public vc_fundef
{
public:
    vc_trans_fundef(vc nm, VCArglist *a, vc fdef, int sty);

protected:

    virtual vc do_function_call(VCArglist *, int suppress_break = 0) const;

private:
    vc tfundef;

    vc eval() const {oopanic("can't eval trans fundef"); return vcnil;}
    virtual vc_default *do_copy() const {oopanic("say what?"); return 0;}

};
#endif
