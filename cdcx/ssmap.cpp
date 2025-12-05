
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

// note: if we can't save for some reason, we just
// do nothing. the reason for this is that most of the
// settings have defaults that are set and used in memory, and we don't want to just
// crash the program if we can't save the settings, since
// it might be getting run from read-only media.
// note: i thought about doing some fancy tmpfile or rename related
// stuff here for handling failures, but it doesn't seem worth it.
// you just lose your settings if there is something going wrong with
// writing a few bytes to your FS.
void
settings_save2()
{
    SSMapIter i(Settings);
    QFile f(add_pfx(User_pfx, "settings.qds"));
    if(!f.open(QIODevice::WriteOnly))
        return;
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
setting_put(const DwOString& key, const DwOString& val, int save)
{
    Settings.insert(key, val);
    if(save)
        settings_save2();
    return 1;
}

int
setting_put(const DwOString& key, int val, int save)
{
    char a[100];
    sprintf(a, "%d", val);
    Settings.insert(key, a);
    if(save)
        settings_save2();
    return 1;
}

int
setting_put(const DwOString& key, const QString& val, int save)
{
    Settings.insert(key, DwOString(val.toAscii().constData(), 0, val.toAscii().length()));
    if(save)
        settings_save2();
    return 1;
}
