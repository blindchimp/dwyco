
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "discomodel.h"
#include "dlli.h"
#include "dwyco_new_msg.h"
#include "getinfo.h"
#include "dwycolistscoped.h"
#include "dwyco_top.h"
extern DwycoCore *TheDwycoCore;

DiscoverListModel *TheDiscoverListModel;

DiscoverListModel::DiscoverListModel(QObject *parent) :
    QQmlObjectListModel<DiscoveredUser>(parent, "display", "uid")
{
    if(TheDiscoverListModel)
        ::abort();
    TheDiscoverListModel = this;
}

DiscoverListModel::~DiscoverListModel()
{
    TheDiscoverListModel = 0;
}

DiscoveredUser *
DiscoverListModel::add_uid_to_model(const QByteArray& uid)
{
    QString huid = uid.toHex();
    DiscoveredUser *c = getByUid(huid);
    if(!c)
    {
        c = new DiscoveredUser(this);
        c->update_uid(huid);
        append(c);
    }
    c->update_display(dwyco_info_to_display(uid));
    if(dwyco_uid_online(uid.constData(), uid.length()))
    {
        c->update_online(true);
        int dum;
        char *ip;
        if(dwyco_uid_to_ip2(uid.constData(), uid.length(), &dum, &ip))
        {
            c->update_ip(ip);
            dwyco_free_array(ip);
        }
    }
    else
    {
        c->update_online(false);
        c->update_ip("");
    }
    return c;
}

void
DiscoverListModel::remove_uid_from_model(const QByteArray& uid)
{
    QString huid = uid.toHex();
    DiscoveredUser *c = getByUid(huid);
    if(!c)
        return;
    remove(c);
}

void
DiscoverListModel::load_users_to_model()
{
    QObject::connect(TheDwycoCore, SIGNAL(sys_uid_resolved(QString)), this, SLOT(uid_resolved(QString)), Qt::UniqueConnection);

    DWYCO_LIST l;

    clear();

    l = dwyco_pal_get_list();
    simple_scoped ql(l);
    int n = ql.rows();
    for(int i = 0; i < n; ++i)
    {
        QByteArray uid = ql.get<QByteArray>(i);

        add_uid_to_model(uid);
    }
}

void
DiscoverListModel::uid_resolved(const QString &huid)
{
    DiscoveredUser *c = getByUid(huid);
    if(!c)
        return;

    QByteArray buid = QByteArray::fromHex(huid.toLatin1());
    c->update_display(dwyco_info_to_display(buid));
    c->update_invalid(0);
    c->update_resolved_counter(c->get_resolved_counter() + 1);
}

void
DiscoverListModel::uid_invalidate_profile(const QString &huid)
{
    DiscoveredUser *c = getByUid(huid);
    if(!c)
        return;
    c->update_invalid(1);

}

