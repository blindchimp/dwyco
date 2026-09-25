/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef MSGPROXYMODEL_H
#define MSGPROXYMODEL_H
#include <QQmlEngine>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QSet>
#include <QByteArray>
#include <QStringList>
#include "QQmlVarPropertyHelpers.h"


class msgproxy_model : public QSortFilterProxyModel
{
    Q_OBJECT
	    QML_NAMED_ELEMENT(DwycoMsgList)
    QML_WRITABLE_VAR_PROPERTY(QString, uid)
    QML_WRITABLE_VAR_PROPERTY(QString, tag)

    QSet<QByteArray> selected;

public:
    msgproxy_model(QObject * = 0);
    virtual ~msgproxy_model();


    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

    Q_INVOKABLE void invalidate_model_filter();
    Q_INVOKABLE void set_filter(int show_sent, int show_recv, int last_n, int only_fav);
    Q_INVOKABLE void set_show_hidden(int);
    Q_INVOKABLE void set_show_trash(bool);
    Q_INVOKABLE void set_show_video_only(int);
    Q_INVOKABLE void reload_model();

    Q_INVOKABLE void toggle_selected(QByteArray mid);
    Q_INVOKABLE void set_all_selected();
    Q_INVOKABLE void set_all_unselected();
    // note: the bulk ops return the mids they actually changed, so the client
    // can push an undo for them. the *_mids getters let the client snapshot
    // the selection before an op that clears it.
    Q_INVOKABLE QStringList selected_mids();
    Q_INVOKABLE QStringList trash_all_selected();
    Q_INVOKABLE void obliterate_all_selected();
    Q_INVOKABLE QStringList fav_all_selected(int);
    Q_INVOKABLE QStringList tag_all_selected(QByteArray tag);
    Q_INVOKABLE QStringList untag_all_selected(QByteArray tag);
    Q_INVOKABLE bool at_least_one_selected();

    int mid_to_index(QByteArray mid);
public slots:
    void refilter(const QByteArray& mid);

private:

    int filter_show_sent;
    int filter_show_recv;
    int filter_last_n;
    int filter_only_favs;
    int filter_show_hidden;
    bool filter_show_trash;
    int filter_only_video;
};


#endif
