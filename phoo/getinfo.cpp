
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "dlli.h"
#include "getinfo.h"
#include "dwycolistscoped.h"

QString
dwyco_info_to_display2(const QByteArray& uid, const char *field)
{
    DWYCO_LIST ql;
    int cant_resolve = 0;
    if(uid.length() == 0)
        return QString("<<empty uid>>");
    ql = dwyco_uid_to_info(uid.constData(), uid.length(), &cant_resolve);
    simple_scoped l(ql);
    if(cant_resolve)
    {
        dwyco_fetch_info(uid.constData(), uid.length());
    }
#if 0
    else if(!Session_uids.contains(uid))
    {
        //Session_uids.insert(uid);
        dwyco_fetch_info(uid.constData(), uid.length());

    }
#endif

    const char *val;
    int len, type;
    dwyco_list_get(l, 0, field, &val, &len, &type);
    if(type != DWYCO_TYPE_STRING)
    {
        return QString("<<bad>>");
    }
    QString ret = QString::fromUtf8(QByteArray(val, len));
    return ret;
}

QString
dwyco_info_to_display(const QByteArray& uid)
{
    return dwyco_info_to_display2(uid, DWYCO_INFO_HANDLE);
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


void
get_review_status(const QByteArray& uid, int& reviewed, int& regular)
{
    DWYCO_LIST l;
    int cant_resolve;
    l = dwyco_uid_to_info(uid.constData(), uid.length(), &cant_resolve);
    reviewed = 0;
    regular = 0;
    dwyco_get_attr_int(l, 0, DWYCO_INFO_REVIEWED, reviewed);
    dwyco_get_attr_int(l, 0, DWYCO_INFO_REGULAR, regular);
    dwyco_list_release(l);
}
