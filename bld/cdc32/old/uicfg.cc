
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "uicfg.h"
#include "vc.h"
#include "syncvar.h"
#include "qauth.h"
#include "xinfo.h"
#include <string.h>

DwTProfile::DwTProfile(const char *sec, const char *, SyncMap *s)
{
    sm = s;
    strcpy(section, sec);
    strcat(section, ".dif");
    if(!load_info(map, section) || map.type() != VC_MAP)
    {
        map = vc(VC_MAP, "", 31);
        save_info(map, section);
    }
}

int
DwTProfile::GetInt(const char *name, int df)
{
    vc v;
    if(!map.find(name, v) || v.type() != VC_INT)
    {
        if(sm)
            sm->replace(name, df);
        return df;
    }
    if(sm)
        sm->replace(name, v);
    return (int)v;
}

void
DwTProfile::GetString(const char *name, char *buf, int buflen, const char *df)
{
    vc v;
    if(!map.find(name, v) || v.type() != VC_STRING ||
            v.len() >= buflen)
    {
        int dflen = strlen(df);
        int cpy = dflen < buflen ? dflen : buflen - 1;
        strncpy(buf, df, cpy);
        buf[cpy] = 0;
        buf[buflen - 1] = 0;
        if(sm)
            sm->replace(name, buf);
        return;
    }
    memcpy(buf, (const char *)v, v.len());
    buf[v.len()] = 0;
    if(sm)
        sm->replace(name, buf);
}

void
DwTProfile::WriteInt(const char *name, int val)
{
    map.add_kv(name, val);
    if(sm)
        sm->replace(name, val);
    save_info(map, section);
}

void
DwTProfile::WriteString(const char *name, const char *val)
{
    map.add_kv(name, val);
    if(sm)
        sm->replace(name, val);
    save_info(map, section);
}

void
save_syncmap(SyncMap *sm, const char *file)
{
    vc om(VC_VECTOR);

    SyncMapIter i(sm);
    for(; !i.eol(); i.forward())
    {
        SyncMapAssoc sa = i.get();
        vc v(VC_VECTOR);
        v[0] = sa.get_key();
        v[1] = sa.get_value();
        om.append(v);
    }
    save_info(om, file);
}

void
load_syncmap(SyncMap *sm, const char *file)
{
    vc im;

    if(!load_info(im, file) || im.type() != VC_VECTOR)
        return;

    int n = im.num_elems();
    for(int i = 0; i < n; ++i)
    {
        sm->replace(im[i][0], im[i][1]);
    }
}
