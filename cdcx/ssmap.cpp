
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// eventually
#include <QFile>
#include <QDataStream>
#include <QIODevice>
#include <QMap>
#include <stdio.h>
#include <stdlib.h>
#include "ssmap.h"
#include "pfx.h"

typedef QMap<DwOString, DwOString> SSMap;
typedef QMapIterator<DwOString, DwOString> SSMapIter;

static SSMap Settings;

void
settings_load()
{

    QFile f(add_pfx(User_pfx, "settings.qds"));
    if(!f.open(QIODevice::ReadOnly))
        return;
    QDataStream in(&f);
    while(!in.atEnd())
    {
        QString key;
        QByteArray val;
        in >> key;
        in >> val;
        Settings.insert(
            DwOString(key.toAscii().constData(), 0, key.toAscii().length()),
            DwOString(val.constData(), 0, val.length())
        );
    }

}

void
settings_save()
{
    SSMapIter i(Settings);
    QFile f(add_pfx(User_pfx, "settings.qds"));
    f.open(QIODevice::WriteOnly);
    QDataStream out(&f);
    for(; i.hasNext(); )
    {
        i.next();
        out << QString(i.key().c_str());
        out << QByteArray(i.value().c_str(), i.value().length());
    }
}


int
setting_get(const DwOString& key, DwOString& out)
{
    if(!Settings.contains(key))
        return 0;
    out = Settings.value(key);
    return 1;
}

int
setting_get(const DwOString& key, int& out)
{
    DwOString tmp;
    if(!Settings.contains(key))
        return 0;
    out = atoi(Settings.value(key).c_str());
    return 1;
}

int
setting_put(const DwOString& key, const DwOString& val)
{
    Settings.insert(key, val);
    return 1;
}

int
setting_put(const DwOString& key, int val)
{
    char a[100];
    sprintf(a, "%d", val);
    Settings.insert(key, a);
    return 1;
}

int
setting_put(const DwOString& key, const QString& val)
{
    Settings.insert(key, DwOString(val.toAscii().constData(), 0, val.toAscii().length()));
    return 1;
}
