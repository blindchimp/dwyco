
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef PULLS_H
#define PULLS_H

#include "dwvecp.h"
#include "dwqbm2.h"
#include "vc.h"
#include "dwtree2.h"
#include "dlli.h"

namespace dwyco {

struct pulls
{
    pulls(const vc& mid, const vc& uid, int pri) :
        mid(mid), uid(uid), m_in_progress(0), pri(pri) {
        Qbm.add(this);
    }
    ~pulls() {
        Qbm.del(this);
        if(m_in_progress)
            inp_set.del(this);
    }

    void set_in_progress(int n) {
        if(n)
        {
            inp_set.add(this, 0);
        }
        else
        {
            inp_set.del(this);
        }
        m_in_progress = n;
    }
    int get_in_progress() const {
        return m_in_progress;
    }

    vc mid;
    vc uid;
    int pri;

private:

    friend void ::dwyco_debug_dump();
    static DwTreeKaz<int, pulls *> inp_set;
    int m_in_progress;

public:
    static DWQBM_W_IDX(Qbm, pulls, mid);

    static void assert_pull(const vc& mid, const vc& uid, int pri);
    static void deassert_pull(const vc& mid);
    static int is_asserted(const vc& mid);
    //static int pull_in_progress(const vc& mid, const vc& uid);
    static DwVecP<pulls> get_stalled_pulls(const vc& uid);
    static void deassert_by_uid(const vc& uid);
    static void pull_failed(const vc& mid, const vc& uid);
    static int set_pull_in_progress(const vc &mid, const vc &uid);
    static void reset_inprogress_for_uid(const vc& uid);
    static int count() {
        return Qbm.count();
    }
    static int count_by_uid(const vc &uid);
    static void clear_all_asserts();
//    int uid_in_prog(const vc& uid, const int& inprog) {
//        if(this->uid == uid && m_in_progress == inprog)
//            return 1;
//        return 0;
//    }

};

}

#endif // PULLS_H
