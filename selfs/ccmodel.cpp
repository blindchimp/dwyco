
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "ccmodel.h"
#include "dwyco_top.h"

extern DwycoCore *TheDwycoCore;

CallContextModel *TheCallContextModel;

CallContextModel::CallContextModel(QObject *parent) :
    QQmlObjectListModel<simple_call>(parent, "display", "uid")
{
    if(TheCallContextModel)
        ::abort();
    TheCallContextModel = this;
}

CallContextModel::~CallContextModel()
{
    TheCallContextModel = 0;
}

void
CallContextModel::uid_resolved(const QString &huid)
{
    simple_call *c = getByUid(huid);
    if(!c)
        return;

    QByteArray buid = QByteArray::fromHex(huid.toLatin1());
    //c->update_display(dwyco_info_to_display(buid));
   // c->update_invalid(0);
    //c->update_resolved_counter(c->get_resolved_counter() + 1);
}

void
CallContextModel::uid_invalidate_profile(const QString &huid)
{
    simple_call *c = getByUid(huid);
    if(!c)
        return;
    //c->update_invalid(1);

}

