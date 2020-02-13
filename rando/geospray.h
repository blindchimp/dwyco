
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef GEOSPRAY_H
#define GEOSPRAY_H

#include <QObject>
#include "QSortFilterProxyModel"
#include "QQmlObjectListModel.h"
#include "QQmlVarPropertyHelpers.h"
#include "qloc.h"

class GeoSent : public QObject
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(QString, hash)
    QML_READONLY_VAR_PROPERTY(QString, mid)
    QML_READONLY_VAR_PROPERTY(QString, display)
    QML_READONLY_VAR_PROPERTY(QString, lat)
    QML_READONLY_VAR_PROPERTY(QString, lon)
    QML_READONLY_VAR_PROPERTY(double, nlat)
    QML_READONLY_VAR_PROPERTY(double, nlon)
    QML_WRITABLE_VAR_PROPERTY(bool, selected)

public:
    GeoSent(QObject *parent = 0) : QObject(parent) {
        m_nlat = 0.0;
        m_nlon = 0.0;
        m_selected = false;
    }
    int load_external_state(const QLoc&);
    int update_counter;
};

class GeoSprayListModel : public QQmlObjectListModel<GeoSent>
{
    Q_OBJECT
public:
    explicit GeoSprayListModel(QObject *parent = 0);
    ~GeoSprayListModel();

    Q_INVOKABLE void load_hash_to_model(const QByteArray& hash);
    GeoSent * add_mid_to_model(const QByteArray& mid);

    Q_INVOKABLE void set_all_selected(bool);
    Q_INVOKABLE void delete_all_selected();

};


extern GeoSprayListModel *TheGeoSprayListModel;

#endif
