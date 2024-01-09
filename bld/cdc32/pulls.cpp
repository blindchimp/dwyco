// this is a set of (mid, uid, inprogress-flag) records.
//
// the existence of a record means we have some idea that we can
// obtain the mid from the associated uid.
//
// note: clients can use the "in progress" flag, and we set it
// to 0 when a pull failure is indicated. but otherwise we
// ignore it.
//
// if a pull is successful, all the records for that mid are deleted.
// a pull record stays alive ("asserted") until we receive some kind
// of resolution to searching for the mid from other clients (it is
// NOT deasserted on failure, as the failure might be transient.)
//
// you can also clean out asserts if a uid will never be queried (like
// if it drops out of the group of devices or becomes ignored, for example.)
// or if the mid is deleted.
//
// note: we may want to put timeouts in here so that once something
// is in progress, it times out if nothing happens for awhile.

#include "pulls.h"
namespace dwyco {

DWQBM_W_IDX(pulls::Qbm, pulls, mid);
DwTreeKaz<int, pulls *> pulls::inp_set(0);

void
pulls::assert_pull(const vc &mid, const vc &uid, int pri)
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
pulls::deassert_pull(const vc& mid)
{
    DwVecP<pulls> dm = pulls::Qbm.query_by_member(mid, &pulls::mid);
    for(int i = 0; i < dm.num_elems(); ++i)
        delete dm[i];
}

void
pulls::clear_all_asserts()
{
    DwVecP<pulls> dm = pulls::Qbm.get_all();
    for(int i = 0; i < dm.num_elems(); ++i)
        delete dm[i];
}

int
pulls::is_asserted(const vc &mid)
{
    return pulls::Qbm.exists_by_member(mid, &pulls::mid);
}

//int
//pulls::pull_in_progress(const vc& mid, const vc& uid)
//{
//    DwVecP<pulls> dm = pulls::Qbm.query_by_member(mid, &pulls::mid);
//    for(int i = 0; i < dm.num_elems(); ++i)
//    {
//        if(dm[i]->uid == uid && dm[i]->m_in_progress)
//            return 1;
//    }
//    return 0;
//}

void
pulls::deassert_by_uid(const vc &uid)
{
    DwVecP<pulls> dm = pulls::Qbm.query_by_member(uid, &pulls::uid);
    for(int i = 0; i < dm.num_elems(); ++i)
        delete dm[i];
}

int
pulls::count_by_uid(const vc& uid)
{
    return pulls::Qbm.count_by_member(uid, &pulls::uid);
}

void
pulls::pull_failed(const vc &mid, const vc &uid)
{
    DwVecP<pulls> dm = pulls::Qbm.query_by_member(mid, &pulls::mid);
    for(int i = 0; i < dm.num_elems(); ++i)
    {
        if(dm[i]->uid == uid)
            dm[i]->set_in_progress(0);
    }
}

int
pulls::set_pull_in_progress(const vc& mid, const vc& uid)
{
    DwVecP<pulls> dm = pulls::Qbm.query_by_member(mid, &pulls::mid);
    for(int i = 0; i < dm.num_elems(); ++i)
    {
        if(dm[i]->uid == uid)
        {
            if(!dm[i]->m_in_progress)
            {
                dm[i]->set_in_progress(1);
                return 1;
            }
            return 0;
        }
    }
    return 0;
}

DwVecP<pulls>
pulls::get_stalled_pulls(const vc &uid)
{
#if 1
    // any pull with a uid of nil means "any uid", so we assert pulls
    // based on that, since this is called at the time we are first
    // connecting to a client
    DwVecP<pulls> wildcards = pulls::Qbm.query_by_member(vcnil, &pulls::mid);
    for(int i = 0; i < wildcards.num_elems(); ++i)
    {
        assert_pull(wildcards[i]->mid, uid, wildcards[i]->pri);
    }
    DwVecP<pulls> dm = pulls::Qbm.query_by_member(uid, &pulls::uid);

    return dm;
#endif
//    DwTreeKazIter<int, pulls *> i(&inp_set);
//    DwVecP<pulls> ret;
//    for(;!i.eol(); i.forward())
//    {
//        auto p = i.get().get_key();
//        if(p->uid == uid)
//            ret.append(p);
//    }
//    return ret;
}
}
