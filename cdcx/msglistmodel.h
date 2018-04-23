
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef MSGLISTMODEL_H
#define MSGLISTMODEL_H
#include <QAbstractListModel>
#include "dlli.h"
#include "dwstr.h"

class msglist_model : public QAbstractListModel
{
    Q_OBJECT
public:
    msglist_model(DWYCO_LIST = 0, QObject * = 0);

    virtual ~msglist_model();

    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

    DWYCO_LIST msg_idx;
    DwOString uid;
};

#endif
