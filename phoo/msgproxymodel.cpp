


/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QDateTime>
#include <QImage>
#include <QColor>
#include <QList>
#include <QSet>
#include <QMap>
#ifdef DWYCO_MODEL_TEST
#include <QAbstractItemModelTester>
#endif
#include <stdlib.h>
#include "msglistmodel.h"
#include "msgproxymodel.h"
#include "msgpv.h"
#include "pfx.h"
#include "dwycolist2.h"
#include "dwyco_new_msg.h"
#include "dwyco_top.h"
#include "dlli.h"

#if defined(LINUX) && !(defined(ANDROID) || defined(MACOSX))
#define LINUX_EMOJI_CRASH_HACK
#endif

#define SM (dynamic_cast<msglist_raw *>(sourceModel()))

msgproxy_model::msgproxy_model(QObject *p) :
    QSortFilterProxyModel(p)
{
    filter_show_recv = 1;
    filter_show_sent = 1;
    filter_last_n = -1;
    filter_only_favs = 0;
    filter_show_hidden = 1;
    filter_show_trash = false;
    msglist_raw *m = new msglist_raw(p);
    setSourceModel(m);
    mlm = this;
#ifdef DWYCO_MODEL_TEST
    new QAbstractItemModelTester(this);
#endif
}

msgproxy_model::~msgproxy_model()
{
    // not needed, as msglist is a singleton, but
    // useful to uncomment this for leak checking
    //delete sourceModel();
}

void
msglist_model::toggle_selected(QByteArray bmid)
{
    if(selected.contains(bmid))
        selected.remove(bmid);
    else
        selected.insert(bmid);
    int i = mid_to_index(bmid);
    QModelIndex mi = index(i, 0);
    if(i != -1)
        emit dataChanged(mi, mi, QVector<int>(1, SELECTED));

}

void
msglist_model::set_all_selected()
{
    int n = rowCount();
    for(int i = 0; i < n; ++i)
    {
        QModelIndex mi = index(i, 0);
        QByteArray mid = data(mi, MID).toByteArray();
        selected.insert(mid);
        emit dataChanged(mi, mi, QVector<int>(1, SELECTED));
    }
}

void
msglist_model::set_all_unselected()
{
    QSet<QByteArray> oselected = selected;
    selected.clear();
    foreach (const QByteArray &value, oselected)
    {
        int i = mid_to_index(value);
        if(i != -1)
        {
            QModelIndex mi = index(i, 0);
            emit dataChanged(mi, mi, QVector<int>(1, SELECTED));
        }
    }
}

bool 
msglist_model::at_least_one_selected()
{
	return selected.count() > 0;
}

// forwarded data operations

void
msglist_model::trash_all_selected()
{
	SM->trash_all_selected();
    selected.clear();
}

void
msglist_model::obliterate_all_selected()
{
	SM->obliterate_all_selected();
    selected.clear();
}

void
msglist_model::fav_all_selected(int f)
{
#if 0
    //QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
    dwyco_start_bulk_update();
    foreach (const QString &value, selected)
    {
        QByteArray b = value.toLatin1();
        DWYCO_LIST l;
        if(dwyco_qd_message_to_body(&l, b.constData(), b.length()))
        {
            dwyco_list_release(l);
            continue;
        }
        dwyco_set_fav_msg(b.constData(), f);
    }
    dwyco_end_bulk_update();
    force_reload_model();
#endif
}

void
msglist_model::tag_all_selected(QByteArray tag)
{
#if 0
    //QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
    dwyco_start_bulk_update();
    foreach (const QString &value, selected)
    {
        QByteArray b = value.toLatin1();
        DWYCO_LIST l;
        if(dwyco_qd_message_to_body(&l, b.constData(), b.length()))
        {
            dwyco_list_release(l);
            continue;
        }
        // the core allows multiple tags, but that can result in redundant tags.
        // it doesn't cause things to fail, but i can't think of a good reason to
        // for it right now. so for now, just
        // don't allow it. maybe at some point we'll change the core to not allow
        // it.
        if(!dwyco_mid_has_tag(b.constData(), tag.constData()))
            dwyco_set_msg_tag(b.constData(), tag.constData());
    }
    dwyco_end_bulk_update();
    force_reload_model();
#endif
}

void
msglist_model::untag_all_selected(QByteArray tag)
{
#if 0
    //QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
    dwyco_start_bulk_update();
    foreach (const QString &value, selected)
    {
        QByteArray b = value.toLatin1();
        DWYCO_LIST l;
        if(dwyco_qd_message_to_body(&l, b.constData(), b.length()))
        {
            dwyco_list_release(l);
            continue;
        }
        dwyco_unset_msg_tag(b.constData(), tag.constData());
    }
    dwyco_end_bulk_update();
    force_reload_model();
#endif
}
