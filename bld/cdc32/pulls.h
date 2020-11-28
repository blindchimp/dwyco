#ifndef PULLS_H
#define PULLS_H

#include "dwvecp.h"
#include "dwqbm.h"
#include "vc.h"

namespace dwyco {


struct pulls
{
    pulls(vc mid, vc uid) {
        this->mid = mid;
        this->uid = uid;
        in_progress = 0;
        Qbm.add(this);
    }
    ~pulls() {
        Qbm.del(this);
    }

    static DwQueryByMember<pulls> Qbm;

    int uid_in_prog(const vc& uid, const int& inprog) {
        if(this->uid == uid && in_progress == inprog)
            return 1;
        return 0;
    }
    vc mid;
    vc uid;
    int in_progress;

    static void assert_pull(vc mid, vc uid);
    static void deassert_pull(vc mid);
    static int is_asserted(vc mid);
    static int pull_in_progress(vc mid, vc uid);
    static void deassert_by_uid(vc uid);
    static void pull_failed(vc mid, vc uid);
    static void set_pull_in_progress(vc mid, vc uid);

};

}

#endif // PULLS_H
