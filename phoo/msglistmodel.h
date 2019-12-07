
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
#include <QSortFilterProxyModel>
#include "dlli.h"

class msglist_model : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString uid READ uid WRITE setUid NOTIFY uidChanged)
    Q_PROPERTY(QString tag READ tag WRITE setTag NOTIFY tagChanged)


public:
    msglist_model(QObject * = 0);

    virtual ~msglist_model();

    void setUid(const QString &uid);
    QString uid() const;

    void setTag(const QString& tag);
    QString tag() const;

    Q_INVOKABLE void reload_model();

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

    Q_INVOKABLE void set_filter(int show_sent, int show_recv, int last_n, int only_favs);
    Q_INVOKABLE void set_show_hidden(int);

    Q_INVOKABLE void toggle_selected(QByteArray mid);
    Q_INVOKABLE void set_all_selected();
    Q_INVOKABLE void set_all_unselected();
    Q_INVOKABLE void delete_all_selected();
    Q_INVOKABLE void fav_all_selected(int);
    Q_INVOKABLE void tag_all_selected(QByteArray tag);
    Q_INVOKABLE void untag_all_selected(QByteArray tag);
    Q_INVOKABLE bool at_least_one_selected();

    int mid_to_index(QByteArray mid);

public slots:
    void msg_recv_status(int cmd, const QString& mid, const QString& huid);
    void mid_tag_changed(QString mid);
    void msg_recv_progress(QString mid, QString huid, QString msg, int percent);

private:
    QString m_uid;
    QString m_tag;

    int filter_show_sent;
    int filter_show_recv;
    int filter_last_n;
    int filter_only_favs;
    int filter_show_hidden;

    void force_reload_model();

signals:
    void uidChanged();
    void tagChanged();

};

class msglist_raw : public QAbstractListModel
{
    Q_OBJECT
public:
    msglist_raw(QObject * = 0);
    virtual ~msglist_raw();


    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual QHash<int, QByteArray> roleNames() const;

    void setUid(const QString& uid);
    void setTag(const QString& tag);
    void reload_model(int force = 0);
    void reload_inbox_model();

private:
    DWYCO_MSG_IDX msg_idx;
    DWYCO_QD_MSG_LIST qd_msgs;
    DWYCO_UNFETCHED_MSG_LIST inbox_msgs;
    int count_msg_idx;
    int count_qd_msgs;
    int count_inbox_msgs;

    QString m_uid;
    QString m_tag;

    QVariant qd_data (int r, int role = Qt::DisplayRole ) const;
    QVariant inbox_data (int r, int role = Qt::DisplayRole ) const;
    int check_inbox_model();
    int check_qd_msgs();

    QString get_msg_text(int row) const;
    QString preview_filename(int row) const;
};

extern msglist_model *mlm;

#endif
