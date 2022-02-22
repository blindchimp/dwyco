
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "syncmodel.h"
#include "dlli.h"
#include "dwyco_new_msg.h"
#include "getinfo.h"
#include "dwycolist2.h"
#include "dwyco_top.h"
extern DwycoCore *TheDwycoCore;

SyncDescModel *TheSyncDescModel;

void
Sync_desc::load_external_state(const QByteArray& uid)
{

}

SyncDescModel::SyncDescModel(QObject *parent) :
    QQmlObjectListModel<Sync_desc>(parent, "uid", "uid")
{
    if(TheSyncDescModel)
        ::abort();
    m_connection_count = 0;
    TheSyncDescModel = this;

}

SyncDescModel::~SyncDescModel()
{
    TheSyncDescModel = 0;
}



void
init_syncdesc_model()
{

}

void
SyncDescModel::uid_resolved(QString huid)
{
    Sync_desc *c = getByUid(huid);
    if(!c)
        return;
    c->update_handle(dwyco_info_to_display(QByteArray::fromHex(huid.toLatin1())));
}

void
SyncDescModel::load_model()
{
    DWYCO_SYNC_MODEL l;
    int n;
    static int cnt;
    ++cnt;

    dwyco_get_sync_model(&l);
    simple_scoped sl(l);
    n = sl.rows();
    for(int i = 0; i < n; ++i)
    {
        QByteArray uid = sl.get<QByteArray>(i, DWYCO_SM_UID);
        QByteArray huid = uid.toHex();
        Sync_desc *c = getByUid(huid);
        if(!c)
        {
            c = new Sync_desc;
            c->update_uid(huid);
            c->update_handle(dwyco_info_to_display(uid));
            connect(c, SIGNAL(statusChanged(QString)), this, SLOT(update_connections(QString)));
            append(c);
        }
        c->update_status(sl.get<QByteArray>(i, DWYCO_SM_STATUS));
        c->update_ip(sl.get<QByteArray>(i, DWYCO_SM_IP));
        c->update_proxy(!sl.is_nil(i, DWYCO_SM_PROXY));
        c->update_asserts(sl.get_long(i, DWYCO_SM_PULLS_ASSERT));
        c->update_sendq_count(sl.get_long(i, DWYCO_SM_SENDQ_COUNT));
        c->update_percent_synced(sl.get_long(i, DWYCO_SM_PERCENT_SYNCED));
        int can_do_direct = 0;
        char *ip_out;
        int o = dwyco_uid_to_ip2(uid.constData(), uid.length(), &can_do_direct, &ip_out);
        if(!o)
            c->update_adv_ip("");
        else
        {
            c->update_adv_ip(ip_out);
            dwyco_free_array(ip_out);
        }
        c->update_counter = cnt;
    }
    QList<Sync_desc *> dead;
    for(int i = 0; i < count(); ++i)
    {
        Sync_desc *c = at(i);
        if(c->update_counter < cnt)
            dead.append(c);

    }
    for(int i = 0; i < dead.count(); ++i)
    {
        Sync_desc *c = dead[i];
        remove(c);
    }
#if 1
    {
    DWYCO_LIST l;
    if(!dwyco_get_group_status(&l))
        return;
    simple_scoped gl(l);
    TheDwycoCore->update_active_group_name(gl.get<QByteArray>(DWYCO_GS_GNAME));
    TheDwycoCore->update_join_key(gl.get<QByteArray>(DWYCO_GS_JOIN_KEY));
    TheDwycoCore->update_percent_synced(gl.get_long(DWYCO_GS_PERCENT_SYNCED));
    TheDwycoCore->update_group_status(gl.get_long(DWYCO_GS_IN_PROGRESS));
    TheDwycoCore->update_eager_pull(gl.get_long(DWYCO_GS_EAGER));
    TheDwycoCore->update_group_private_key_valid(gl.get_long(DWYCO_GS_VALID));
    //TheJoinLogModel->load_model();
    }
#endif

}

void
SyncDescModel::update_connections(QString )
{
    int cnt = 0;
    for(int i = 0; i < count(); ++i)
    {
        auto c = at(i);
        auto state = c->get_status();
        if(state == "oa" || state == "ra")
            ++cnt;
    }
    update_connection_count(cnt);

}

