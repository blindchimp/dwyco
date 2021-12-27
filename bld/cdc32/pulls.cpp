// this is a set of (mid, uid, inprogress-flag) records
// the existence of a record means we have some idea that we can
// obtain the mid from the associated uid.
//
// note: clients can use the "in progress" flag, and we set it
// to 0 when a pull failure is indicated. but otherwise we
// ignore it.
//
// if a pull is successful, all the records for that mid are deleted.
//
//
// note: we may want to put timeouts in here so that once something
// is in progress, it times out if nothing happens for awhile.
#include "pulls.h"
namespace dwyco {

DWQBM_W_IDX(pulls::Qbm, pulls, mid);
DwTreeKaz<int, pulls *> pulls::inp_set(0);

void
pulls::assert_pull(vc mid, vc uid, int pri)
{
    DwVecP<pulls> dm = pulls::Qbm.query_by_member(mid, &pulls::mid);
    int nonew = 0;
    for(int i = 0; i < dm.num_elems(); ++i)
    {
        // if the priority of the pull is increased, update that in all
        // the pulls before returning
        if(pri < dm[i]->pri)
            dm[i]->pri = pri;
        if(dm[i]->uid == uid)
        {
            nonew = 1;
        }
    }
    if(nonew)
        return;
    new pulls(mid, uid, pri);
}

void
pulls::deassert_pull(vc mid)
{
    DwVecP<pulls> dm = pulls::Qbm.query_by_member(mid, &pulls::mid);
    for(int i = 0; i < dm.num_elems(); ++i)
        delete dm[i];
}

int
pulls::is_asserted(vc mid)
{
    return pulls::Qbm.exists_by_member(mid, &pulls::mid);
}

int
pulls::pull_in_progress(vc mid, vc uid)
{
    DwVecP<pulls> dm = pulls::Qbm.query_by_member(mid, &pulls::mid);
    for(int i = 0; i < dm.num_elems(); ++i)
    {
        if(dm[i]->uid == uid && dm[i]->m_in_progress)
            return 1;
    }
    return 0;
}

void
pulls::deassert_by_uid(vc uid)
{
    DwVecP<pulls> dm = pulls::Qbm.query_by_member(uid, &pulls::uid);
    for(int i = 0; i < dm.num_elems(); ++i)
        delete dm[i];
}

int
pulls::count_by_uid(vc uid)
{
    return pulls::Qbm.count_by_member(uid, &pulls::uid);
}

void
pulls::pull_failed(vc mid, vc uid)
{
    DwVecP<pulls> dm = pulls::Qbm.query_by_member(mid, &pulls::mid);
    for(int i = 0; i < dm.num_elems(); ++i)
    {
        if(dm[i]->uid == uid)
            dm[i]->set_in_progress(0);
    }
}

void
pulls::set_pull_in_progress(vc mid, vc uid)
{
    DwVecP<pulls> dm = pulls::Qbm.query_by_member(mid, &pulls::mid);
    for(int i = 0; i < dm.num_elems(); ++i)
    {
        if(dm[i]->uid == uid)
            dm[i]->set_in_progress(1);
    }
}

DwVecP<pulls>
pulls::get_stalled_pulls(vc uid)
{
    DwVecP<pulls> dm = pulls::Qbm.query_by_member(uid, &pulls::uid);
    return dm;
#if 0
    DwTreeKazIter<int, pulls *> i(&inp_set);
    DwVecP<pulls> ret;
    for(;!i.eol(); i.forward())
    {
        auto p = i.get().get_key();
        if(p->uid == uid)
            ret.append(p);
    }
    return ret;
#endif
}
}
