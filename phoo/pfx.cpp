
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "pfx.h"
#include "dlli.h"


QByteArray User_pfx;
QByteArray Sys_pfx;
QByteArray Tmp_pfx;

QByteArray 
add_pfx(const QByteArray& pfx, const QByteArray& fn)
{
	QByteArray ret = pfx;
	ret += "/";
	ret += fn;
	return ret;
}

QByteArray
random_fn()
{
    char *r;
    dwyco_random_string2(&r, 10);
    QByteArray f(r, 10);
    dwyco_free_array(r);

    return f.toHex();
}

QString
get_extended(QString txt)
{
    int i = txt.indexOf("<html>", 0, Qt::CaseInsensitive);
    if(i == -1)
        return txt;
    txt.remove(0, i);
    // get rid of head too, old msgs have a lot of this in them
    i = txt.indexOf("</head>", 0, Qt::CaseInsensitive);
    if(i == -1)
        return txt;
    txt.remove(0, i + 7);
    txt.prepend("<html>");
    return txt;
}
