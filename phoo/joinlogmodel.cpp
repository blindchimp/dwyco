
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "joinlogmodel.h"
#include "dlli.h"
#include "getinfo.h"
#include "dwycolist2.h"

JoinLogModel *TheJoinLogModel;

void
Join_log_entry::load_external_state(const QByteArray& uid)
{

}

JoinLogModel::JoinLogModel(QObject *parent) :
    QQmlObjectListModel<Join_log_entry>(parent, "uid", "uid")
{
    if(TheJoinLogModel)
        ::abort();
    TheJoinLogModel = this;

}

JoinLogModel::~JoinLogModel()
{
    TheJoinLogModel = 0;
}


void
JoinLogModel::uid_resolved(QString huid)
{
    int n = count();
    for(int i = 0; i < n; ++i)
    {
        auto c = at(i);
        c->update_handle(dwyco_info_to_display(QByteArray::fromHex(huid.toLatin1())));
    }
}

void
JoinLogModel::load_model()
{
    DWYCO_JOIN_LOG_MODEL l;
    int n;

    dwyco_get_join_log_model(&l);
    simple_scoped sl(l);
    n = sl.rows();
    clear();
    for(int i = 0; i < n; ++i)
    {
        QByteArray huid = sl.get<QByteArray>(i, DWYCO_JL_HUID);
        QByteArray uid = QByteArray::fromHex(huid);
        Join_log_entry *c = new Join_log_entry(this);
        c->update_uid(huid);
        c->update_handle(dwyco_info_to_display(uid));
        c->update_msg(sl.get<QByteArray>(i, DWYCO_JL_MSG));
        c->update_time(QDateTime::fromSecsSinceEpoch(sl.get_long(i, DWYCO_JL_TIME)));

        append(c);
    }
}



