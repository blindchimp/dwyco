
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
#include "ignoremodel.h"
#include "dwycolistscoped.h"


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
            append(c);
        }
        connect(c, SIGNAL(statusChanged(QString)), this, SLOT(update_connections(QString)));
        c->update_status(sl.get<QByteArray>(i, DWYCO_SM_STATUS));
        c->update_ip(sl.get<QByteArray>(i, DWYCO_SM_IP));
        c->update_proxy(!sl.is_nil(i, DWYCO_SM_PROXY));
        c->update_asserts(sl.get_long(i, DWYCO_SM_PULLS_ASSERT));
        c->update_sendq_count(sl.get_long(i, DWYCO_SM_SENDQ_COUNT));
        c->update_percent_synced(sl.get_long(i, DWYCO_SM_PERCENT_SYNCED));
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
