
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "toxfriendmodel.h"
#include "dlli.h"
#include "dwycolist2.h"

ToxFriendModel::ToxFriendModel(QObject *parent) :
    QQmlObjectListModel<ToxFriend>(parent, QByteArray(), "pubkey")
{
}

ToxFriendModel::~ToxFriendModel()
{
}

void
ToxFriendModel::load_friends()
{
    DWYCO_TOX_FRIENDS_MODEL fl = 0;
    if(!dwyco_tox_get_friends_model(&fl))
        return;
    simple_scoped qfl(fl);
    static int cnt;
    ++cnt;
    int n = qfl.rows();
    for(int i = 0; i < n; ++i)
    {
        int fn = qfl.get_long(i, DWYCO_TF_FRIEND_NUMBER);
        QByteArray pk = qfl.get<QByteArray>(i, DWYCO_TF_PUBKEY);
        QString pkhex = pk.toHex();
        ToxFriend *f = getByUid(pkhex);
        if(!f)
        {
            f = new ToxFriend(this);
            f->update_pubkey(pkhex);
            append(f);
        }
        f->update_friend_number(fn);
        f->update_name(qfl.get<QByteArray>(i, DWYCO_TF_NAME));
        f->update_status(qfl.get<QByteArray>(i, DWYCO_TF_STATUS));
        f->update_user_status(qfl.get<QByteArray>(i, DWYCO_TF_USER_STATUS));
        f->update_counter = cnt;
    }
    QList<ToxFriend *> dead;
    for(int i = 0; i < count(); ++i)
    {
        ToxFriend *f = at(i);
        if(f->update_counter < cnt)
            dead.append(f);
    }
    for(int i = 0; i < dead.count(); ++i)
        remove(dead[i]);
    emit friendsLoaded();
}
