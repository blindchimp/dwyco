
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "ignoremodel.h"
#include "dlli.h"
#include "dwyco_new_msg.h"
#include "getinfo.h"
#include "dwycolist2.h"

IgnoreListModel *TheIgnoreListModel;

IgnoreListModel::IgnoreListModel(QObject *parent) :
    QQmlObjectListModel<IgnoredUser>(parent, "display", "uid")
{
    if(TheIgnoreListModel)
        ::abort();
    TheIgnoreListModel = this;

}

IgnoreListModel::~IgnoreListModel()
{
    TheIgnoreListModel = 0;
}

IgnoredUser *
IgnoreListModel::add_uid_to_model(const QByteArray& uid)
{
    QString huid = uid.toHex();
    IgnoredUser *c = getByUid(huid);
    if(!c)
    {
        c = new IgnoredUser(this);
        c->update_uid(huid);
        append(c);
    }
    c->update_display(dwyco_info_to_display(uid));
    return c;
}

void
IgnoreListModel::remove_uid_from_model(const QByteArray& uid)
{
    QString huid = uid.toHex();
    IgnoredUser *c = getByUid(huid);
    if(!c)
        return;
    remove(c);
}

void
IgnoreListModel::load_users_to_model()
{
    DWYCO_LIST l;
    int n;

    clear();

    l = dwyco_ignore_list_get();
    simple_scoped ql(l);
    n = ql.rows();
    for(int i = 0; i < n; ++i)
    {
        QByteArray uid = ql.get<QByteArray>(i);
        add_uid_to_model(uid);
    }
}



void
IgnoreListModel::uid_resolved(const QString &huid)
{
    IgnoredUser *c = getByUid(huid);
    if(!c)
        return;

    QByteArray buid = QByteArray::fromHex(huid.toLatin1());
    c->update_display(dwyco_info_to_display(buid));
    c->update_invalid(0);
    c->update_resolved_counter(c->get_resolved_counter() + 1);
}

void
IgnoreListModel::uid_invalidate_profile(const QString &huid)
{
    IgnoredUser *c = getByUid(huid);
    if(!c)
        return;
    c->update_invalid(1);

}

IgnoreSortFilterModel::IgnoreSortFilterModel(QObject *p)
    : QSortFilterProxyModel(p)
{
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    sort(0);
    m_count = 0;
}

