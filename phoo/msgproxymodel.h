/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef MSGPROXYMODEL_H
#define MSGPROXYMODEL_H
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QSet>
#include <QByteArray>

class msgproxy_model : public QSortFilterProxyModel
{
    Q_OBJECT

    QSet<QByteArray> selected;

public:
    msgproxy_model(QObject * = 0);
    virtual ~msgproxy_model();


    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

    Q_INVOKABLE void invalidate_model_filter();
    Q_INVOKABLE void set_filter(int show_sent, int show_recv, int last_n);
    Q_INVOKABLE void set_show_hidden(int);
    Q_INVOKABLE void set_show_trash(bool);

    Q_INVOKABLE void toggle_selected(QByteArray mid);
    Q_INVOKABLE void set_all_selected();
    Q_INVOKABLE void set_all_unselected();
    Q_INVOKABLE void trash_all_selected();
    Q_INVOKABLE void obliterate_all_selected();
    Q_INVOKABLE void fav_all_selected(int);
    Q_INVOKABLE void tag_all_selected(QByteArray tag);
    Q_INVOKABLE void untag_all_selected(QByteArray tag);
    Q_INVOKABLE bool at_least_one_selected();

    int mid_to_index(QByteArray mid);

private:

    int filter_show_sent;
    int filter_show_recv;
    int filter_last_n;
    int filter_only_favs;
    int filter_show_hidden;
    bool filter_show_trash;
};


#endif
