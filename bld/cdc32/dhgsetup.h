
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
#include "ssns.h"
#include "dwvp.h"
#include "simple_property.h"

namespace dwyco {

class DH_alternate : public ssns::trackable {
public:
    DwVP vp;
private:
    vc DH_static;
    vc uid;
    vc alternate_name;

    int new_account();
    static int insert_record(vc uid, vc alt_name, vc dh_static);

public:
    DH_alternate() : vp(this) {}
    ~DH_alternate() {
        vp.invalidate();
    }
    // this needs to be filled in from somewhere before
    // group join messages can be processed.
    //vc password;

    void init(vc uid, vc alternate_name);
    int load_account(vc alternate_name);
    void leave();
    vc alt_name() {return alternate_name;}
    vc my_static();
    vc my_static_public();
    // this is used as "password" to filter out some
    // calls early if they obviously have the wrong key
    vc hash_key_material();

    // inserts a new key into the database, which we
    // presumably got from another group member or the server
    static int insert_public_key(vc alt_name, vc grp_key, vc sig);
    static int insert_private_key(vc alt_name, vc grp_key);
    static int insert_sig(vc alt_name, vc sig);

    // call this to remove a key from the database, as when
    // leaving a group.
    static int remove_key(vc alt_name);

    // get all the key-pairs we have
    static vc get_all_keys();

    static void update_group(vc);

    // note: set this when you are working with a provisional
    // key, trying to find the private key where you haven't
    // successfully set up "current_alternate". note: this feels like
    // a kluge. maybe figure out how to associate a provisional
    // dh_alternate with the current round of joins (tho this might
    // require us to keep more state, or break down and let the
    // group processing stuff just use the value from settings directly
    static vc Group_join_password;

    vc gen_challenge_msg(vc& nonce_out);
    int challenge_recv(vc m, vc& resp);
};

void init_dhg();
extern sigprop<dwyco::DH_alternate *> Current_alternate;
extern sigprop<vc> Group_uids;

}

#endif
