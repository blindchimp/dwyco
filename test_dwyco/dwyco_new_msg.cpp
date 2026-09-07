// Qt-free port of bld/miscsrc/dwyco_new_msg.cpp using dwcls classes
// (DwString / DwVec) instead of QByteArray / QList / QSet.
#include "dwyco_new_msg.h"
#include "dlli.h"
#include "dwycolist2.h"
#include "dwvec.h"

static DwVec<DwString> fetching;
static DwVec<DwString> Dont_refetch;
static DwVec<DwString> Delete_msgs;
static DwVec<DwString> Already_returned;

static void
set_insert(DwVec<DwString>& s, const DwString& v)
{
    if(!s.contains(v))
        s.append(v);
}

void
DWYCOCALLCONV
msg_callback(int /*id*/, int what, const char *mid, void *)
{
    DwString m(mid);
    int i = (int)fetching.index(m);
    switch(what)
    {
    case DWYCO_MSG_DOWNLOAD_FETCHING_ATTACHMENT:
        return;
    case DWYCO_MSG_DOWNLOAD_RATHOLED:
        set_insert(Dont_refetch, m);
        if(i >= 0)
            fetching.del(i);
        Delete_msgs.append(m);
        return;
    case DWYCO_MSG_DOWNLOAD_ATTACHMENT_FETCH_FAILED:
    case DWYCO_MSG_DOWNLOAD_SAVE_FAILED:
    case DWYCO_MSG_DOWNLOAD_FAILED:
        set_insert(Dont_refetch, m);
        if(i >= 0)
            fetching.del(i);
        return;
    case DWYCO_MSG_DOWNLOAD_OK:
    default:
        if(i >= 0)
            fetching.del(i);
    }
}

int
process_remote_msgs()
{
    for(int i = 0; i < (int)Delete_msgs.num_elems(); ++i)
    {
        dwyco_delete_unfetched_message(Delete_msgs[i].c_str());
    }
    Delete_msgs.set_size(0);

    if(fetching.num_elems() >= 3)
        return 0;
    DWYCO_UNFETCHED_MSG_LIST ufml;
    if(!dwyco_get_unfetched_messages(&ufml, 0, 0))
        return 0;
    simple_scoped qufml(ufml);
    int n = qufml.rows();
    if(n == 0)
        return 0;
    for(int i = 0; i < n; ++i)
    {
        DwString mid = qufml.get<DwString>(i, DWYCO_QMS_ID);
        if(fetching.contains(mid) || Dont_refetch.contains(mid))
            continue;
        if(dwyco_fetch_server_message(mid.c_str(), msg_callback, 0, 0, 0))
        {
            fetching.append(mid);
            if(fetching.num_elems() >= 3)
                return 1;
        }
    }
    return 0;
}

int
dwyco_new_msg2(DwString& uid_out, DwString& txt, int& zap_viewer,
    DwString& mid, int& has_att, int& is_file, DwString& creator_uid)
{
    DWYCO_LIST inbox_mids;
    if(!dwyco_get_tagged_idx(&inbox_mids, "_inbox", 0))
        return 0;
    simple_scoped qim(inbox_mids);
    int n = qim.rows();
    if(n == 0)
        return 0;
    for(int i = 0; i < n; ++i)
    {
        mid = qim.get<DwString>(i, DWYCO_MSG_IDX_MID);
        if(Already_returned.contains(mid))
            continue;
        set_insert(Already_returned, mid);
        uid_out = DwString::from_hex(qim.get<DwString>(i, DWYCO_MSG_IDX_ASSOC_UID));
        has_att = !qim.is_nil(i, DWYCO_MSG_IDX_HAS_ATTACHMENT);
        is_file = !qim.is_nil(i, DWYCO_MSG_IDX_IS_FILE);
        DWYCO_SAVED_MSG_LIST sm;
        if(!dwyco_get_saved_message(&sm, uid_out.c_str(), uid_out.length(), mid.c_str()))
            continue;
        simple_scoped qsm(sm);
        DWYCO_LIST bt = dwyco_get_body_text(qsm);
        simple_scoped qbt(bt);
        txt = qbt.get<DwString>(0);

        DWYCO_LIST ba = dwyco_get_body_array(qsm);
        simple_scoped qba(ba);
        if(qba.rows() == 1)
            creator_uid = uid_out;
        else
        {
            int crow = qba.rows() - 1;
            creator_uid = qba.get<DwString>(crow, DWYCO_QM_BODY_FROM);
        }

        return 1;
    }
    return 0;
}

int
dwyco_new_msg(DwString& uid_out, DwString& txt, int& zap_viewer,
    DwString& mid, int& has_att)
{
    DwString dum1;
    int dum2;
    return dwyco_new_msg2(uid_out, txt, zap_viewer, mid, has_att, dum2, dum1);
}

void
processed_msg(DwString& mid)
{
    dwyco_unset_msg_tag(mid.c_str(), "_inbox");
    dwyco_delete_unfetched_message(mid.c_str());
}