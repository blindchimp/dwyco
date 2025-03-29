


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
#include "msgrawmodel.h"
#include "msgproxymodel.h"
#include "dlli.h"

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
    QObject::connect(this, &msgproxy_model::uidChanged, m, &msglist_raw::set_uid);
    QObject::connect(this, &msgproxy_model::tagChanged, m, &msglist_raw::set_tag);
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
msgproxy_model::reload_model()
{
    SM->reload_model();
}

int
msgproxy_model::mid_to_index(QByteArray mid)
{
    int sidx = SM->mid_to_index(mid);
    if(sidx == -1)
        return -1;
    return mapFromSource(SM->index(sidx)).row();
}

void
msgproxy_model::invalidate_model_filter()
{
    invalidateFilter();
}

void
msgproxy_model::set_filter(int sent, int recv, int last_n, int only_favs)
{
    filter_show_recv = recv;
    filter_show_sent = sent;
    filter_last_n = last_n;
    filter_only_favs = only_favs;
    invalidateFilter();
    selected.clear();
}

void
msgproxy_model::set_show_hidden(int show_hidden)
{
    filter_show_hidden = show_hidden;
    invalidateFilter();
    selected.clear();
}

void
msgproxy_model::set_show_trash(bool show_trash)
{
    filter_show_trash = show_trash;
    invalidateFilter();
    selected.clear();
}

void
msgproxy_model::toggle_selected(QByteArray bmid)
{
    if(selected.contains(bmid))
        selected.remove(bmid);
    else
        selected.insert(bmid);
    int i = mid_to_index(bmid);
    QModelIndex mi = index(i, 0);
    if(i != -1)
        emit dataChanged(mi, mi, QVector<int>(1, msglist_raw::SELECTED));

}

void
msgproxy_model::set_all_selected()
{
    int n = rowCount();
    for(int i = 0; i < n; ++i)
    {
        QModelIndex mi = index(i, 0);
        QByteArray mid = data(mi, msglist_raw::MID).toByteArray();
        selected.insert(mid);
        emit dataChanged(mi, mi, QVector<int>(1, msglist_raw::SELECTED));
    }
}

void
msgproxy_model::set_all_unselected()
{
    QSet<QByteArray> oselected = selected;
    selected.clear();
    foreach (const QByteArray &value, oselected)
    {
        int i = mid_to_index(value);
        if(i != -1)
        {
            QModelIndex mi = index(i, 0);
            emit dataChanged(mi, mi, QVector<int>(1, msglist_raw::SELECTED));
        }
    }
}

bool 
msgproxy_model::at_least_one_selected()
{
	return selected.count() > 0;
}

// forwarded data operations

void
msgproxy_model::trash_all_selected()
{
    SM->trash_all_selected(selected);
    selected.clear();
}

void
msgproxy_model::obliterate_all_selected()
{
    SM->obliterate_all_selected(selected);
    selected.clear();
}

void
msgproxy_model::fav_all_selected(int f)
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
msgproxy_model::tag_all_selected(QByteArray tag)
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
msgproxy_model::untag_all_selected(QByteArray tag)
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

QVariant
msgproxy_model::data ( const QModelIndex & index, int role ) const
{
    if(role == msglist_raw::SELECTED)
    {
        QAbstractItemModel *alm = sourceModel();
        QModelIndex si = mapToSource(index);
        QVariant mid = alm->data(alm->index(si.row(), 0), msglist_raw::MID);
        return selected.contains(mid.toByteArray());
    }
    return SM->data(mapToSource(index), role);
}

bool
msgproxy_model::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QAbstractItemModel *alm = sourceModel();

    if(filter_show_trash == false)
    {
        QVariant mid = alm->data(alm->index(source_row, 0), msglist_raw::MID);
        int trashed = dwyco_mid_has_tag(mid.toByteArray().constData(), "_trash");
        if(trashed)
            return false;
    }

    QVariant is_sent = alm->data(alm->index(source_row, 0), msglist_raw::SENT);
    if(filter_show_sent == 0 && is_sent.toInt() == 1)
        return false;
    if(filter_show_recv == 0 && is_sent.toInt() == 0)
        return false;
    if(filter_only_favs)
    {
        QVariant is_fav = alm->data(alm->index(source_row, 0), msglist_raw::IS_FAVORITE);
        if(is_fav.toInt() == 0)
            return false;
    }
    if(filter_show_hidden == 0)
    {
        QVariant mid = alm->data(alm->index(source_row, 0), msglist_raw::MID);
        int hidden = dwyco_mid_has_tag(mid.toByteArray().constData(), "_hid");
        if(hidden)
            return false;
    }

    return true;
}
