
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// note: this really needs to be subsumed by the native qt stuff
// eventually
#include <QFile>
#include <QDataStream>
#include <QIODevice>
#include <QMap>
#include <QVariant>
#include <stdio.h>
#include <stdlib.h>
#include "ssmap.h"
#include "pfx.h"

typedef QMap<QString, QString> SSMap;

static SSMap Settings;

void
settings_load()
{
	
	QFile f(add_pfx(User_pfx, "settings.q2"));
	if(!f.open(QIODevice::ReadOnly))
		return;
	QDataStream in(&f);
	in >> Settings;
}

void
settings_save()
{

    QFile f(add_pfx(User_pfx, "settings.q2"));
	f.open(QIODevice::WriteOnly);
	QDataStream out(&f);
	out << Settings;
}


int
setting_get(const QString& key, QString& out)
{
	if(!Settings.contains(key))
		return 0;
    out = Settings.value(key);
	return 1;
}



int
setting_put(const QString& key, const QString& val)
{
	Settings.insert(key, val);
    settings_save();
	return 1;
}

