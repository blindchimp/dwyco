
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DHGSETUP_H
#define DHGSETUP_H

#include "vc.h"

namespace dwyco {

class DH_alternate {
    vc DH_static;
    vc uid;
    vc alternate_name;

    int new_account();

public:

    void init(vc uid, vc alternate_name);
    int load_account(vc uid, vc alternate_name);
    vc alt_name() {return alternate_name;}
    // note: these "combined" things are used for manipulation
    // of static/ephemeral UDH keys, which aren't used in this
    // situation.
//    vc gen_combined_keys();
//    vc my_combined_publics();
//    vc static_material(vc combined_material);
    vc my_static();
    vc my_static_public();
};

extern DH_alternate *Current_alternate;

}

#endif
