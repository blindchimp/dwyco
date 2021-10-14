
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "dlli.h"
#include "getinfo.h"
#include "dwycolist2.h"

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

void
get_review_status(const QByteArray& uid, int& reviewed, int& regular)
{
    DWYCO_LIST l;
    int cant_resolve;
    l = dwyco_uid_to_info(uid.constData(), uid.length(), &cant_resolve);
    simple_scoped sl(l);
    reviewed = sl.get_long(DWYCO_INFO_REVIEWED);
    regular = sl.get_long(DWYCO_INFO_REGULAR);
}
