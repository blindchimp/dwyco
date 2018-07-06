
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

static QByteArray
dwyco_get_attr(DWYCO_LIST l, int row, const char *col)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        ::abort();
    if(type != DWYCO_TYPE_STRING && type != DWYCO_TYPE_NIL)
        ::abort();
    return QByteArray(val, len);
}

static int
dwyco_get_attr_int(DWYCO_LIST l, int row, const char *col, int& int_out)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_INT)
        return 0;
    QByteArray str_out = QByteArray(val, len);
    int_out = str_out.toInt();
    return 1;
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
    dwyco_list_numelems(l, &n, 0);
    for(int i = 0; i < n; ++i)
    {
        QByteArray uid = dwyco_get_attr(l, i, DWYCO_NO_COLUMN);
        add_uid_to_model(uid);
    }
    dwyco_list_release(l);
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

#if 0
bool
IgnoreSortFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    IgnoreListModel *m = dynamic_cast<IgnoreListModel *>(sourceModel());
    if(!m)
        return false;
    int luc = m->data(left, m->roleForName("unseen_count")).toInt();
    int ruc = m->data(right, m->roleForName("unseen_count")).toInt();
    if(luc < ruc)
        return false;
    else if(ruc < luc)
        return true;

    bool lau = m->data(left, m->roleForName("any_unread")).toBool();
    bool rau = m->data(right, m->roleForName("any_unread")).toBool();
    if(lau && !rau)
        return true;
    else if(!lau && rau)
        return false;

    bool lreg = m->data(left, m->roleForName("REGULAR")).toBool();
    bool rreg = m->data(right, m->roleForName("REGULAR")).toBool();
    if(lreg && !rreg)
        return true;
    else if(!lreg && rreg)
        return false;

    return QSortFilterProxyModel::lessThan(left, right);
}


#endif
