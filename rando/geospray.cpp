
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "geospray.h"
#include "dlli.h"
#include "dwyco_new_msg.h"
#include "getinfo.h"
#include "dwycolist2.h"
#include "qloc.h"

void hack_unread_count();

GeoSprayListModel *TheGeoSprayListModel;
extern QMultiMap<QByteArray, QLoc> Hash_to_loc;

int
GeoSent::load_external_state(const QLoc& ql)
{
    // lets assume we have loaded all the messages into the internal
    // hash table for now
    update_mid(ql.mid);
    update_hash(ql.hash.toHex());
    update_display(ql.loc);
    update_lat(ql.lat);
    update_lon(ql.lon);
    /// hmmm, maybe just leave the parsing to qml
    return 1;
}

GeoSprayListModel::GeoSprayListModel(QObject *parent) :
    QQmlObjectListModel<GeoSent>(parent, "display", "mid")
{
    if(TheGeoSprayListModel)
        ::abort();
    TheGeoSprayListModel = this;

}

GeoSprayListModel::~GeoSprayListModel()
{
    TheGeoSprayListModel = 0;
}

void
GeoSprayListModel::load_hash_to_model(const QByteArray& hash)
{
    clear();
    QList<QLoc> locs = Hash_to_loc.values(QByteArray::fromHex(hash));
    for(int i = 0; i < locs.count(); ++i)
    {
        GeoSent *gs = new GeoSent;
        gs->load_external_state(locs[i]);
        prepend(gs);
    }
}

void
GeoSprayListModel::set_all_selected(bool b)
{
    int n = count();
    for(int i = 0; i < n; ++i)
    {
        GeoSent *c = at(i);
        c->set_selected(b);
    }

}

void
GeoSprayListModel::delete_all_selected()
{
    int n = count();
    QList<GeoSent *> to_remove;
    for(int i = 0; i < n; ++i)
    {
        GeoSent *c = at(i);
        if(c->get_selected())
        {
            to_remove.append(c);
        }
    }
}



