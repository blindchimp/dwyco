#ifndef PULLS_H
#define PULLS_H

#include "dwvecp.h"
#include "dwqbm2.h"
#include "vc.h"
#include "simple_property.h"
#include "dwtree2.h"
#include "dlli.h"

namespace dwyco {

struct pulls
{
    pulls(const vc& mid, const vc& uid) :
        mid(mid), uid(uid), in_progress(0) {
        Qbm.add(this);
    }
    ~pulls() {
        Qbm.del(this);
        if(in_progress)
            inp_set.del(this);
    }
    static DwTreeKaz<int, pulls *> inp_set;
    void set_in_progress(int n) {
        if(n)
        {
            inp_set.add(this, 0);
        }
        else
        {
            inp_set.del(this);
        }
        in_progress = n;
    }
    int get_in_progress() const {
        return in_progress;
    }

    vc mid;
    vc uid;
private:
    friend void ::dwyco_debug_dump();
    int in_progress;

public:
    static DWQBM_W_IDX(Qbm, pulls, mid);

    static void assert_pull(vc mid, vc uid);
    static void deassert_pull(vc mid);
    static int is_asserted(vc mid);
    static int pull_in_progress(vc mid, vc uid);
    static DwVecP<pulls> get_stalled_pulls(vc uid);
    static void deassert_by_uid(vc uid);
    static void pull_failed(vc mid, vc uid);
    static void set_pull_in_progress(vc mid, vc uid);
    static int count() {
        return Qbm.count();
    }
    int uid_in_prog(const vc& uid, const int& inprog) {
        if(this->uid == uid && in_progress == inprog)
            return 1;
        return 0;
    }

};

}

#endif // PULLS_H
