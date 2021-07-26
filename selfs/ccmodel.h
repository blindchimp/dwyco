
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CCMODEL_H
#define CCMODEL_H

#include <QObject>
#include "QQmlObjectListModel.h"
#include "QQmlVarPropertyHelpers.h"
#include "callsm.h"


class CallContextModel : public QQmlObjectListModel<simple_call>
{
    Q_OBJECT
public:
    explicit CallContextModel(QObject *parent = 0);
    ~CallContextModel();

signals:

public slots:
    // note: the invalidate is needed because we need to
    // clear the internal cache of images that may refer to uid's profile.
    void uid_invalidate_profile(const QString& huid);
    void uid_resolved(const QString& huid);
    //void decorate(QString huid, QString txt, QString mid);
    //void decorate(QString huid);
};


extern CallContextModel *TheCallContextModel;

#endif
