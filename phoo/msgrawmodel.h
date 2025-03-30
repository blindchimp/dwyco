
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef MSGRAWMODEL_H
#define MSGRAWMODEL_H
#include <QAbstractListModel>
#include "QQmlVarPropertyHelpers.h"
#include "dlli.h"

class msglist_raw : public QAbstractListModel
{
    Q_OBJECT
    QML_WRITABLE_VAR_PROPERTY(QString, uid)
    QML_WRITABLE_VAR_PROPERTY(QString, tag)


public:
    msglist_raw(QObject * = 0);
    virtual ~msglist_raw();

    enum {
        MID = Qt::UserRole,
        SENT,
        MSG_TEXT,
        PREVIEW_FILENAME,
        HAS_VIDEO,
        HAS_SHORT_VIDEO,
        HAS_AUDIO,
        IS_FILE,
        HAS_ATTACHMENT,
        IS_FORWARDED,
        IS_NO_FORWARD,
        DATE_CREATED,
        LOCAL_TIME_CREATED, // this take into account time zone and all that
        IS_QD,
        IS_ACTIVE,
        IS_FAVORITE,
        IS_HIDDEN,
        // warning: despite this being here, trying to access it will panic.
        // this is really just a way of providing QML a way to know something
        // is selected based on mid, rather than by index (like the selection
        // model stuff does in qt.) the proxy model handles the "selection model"
        // and returns the selected state of an item, so it should never come
        // down to this model.
        SELECTED,
        FETCH_STATE,
        ATTACHMENT_PERCENT,
        ASSOC_UID, // who the message is from (or to, if sent msg)
    };

    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void trash_all_selected(const QSet<QByteArray>&);
    Q_INVOKABLE void obliterate_all_selected(const QSet<QByteArray>&);
    Q_INVOKABLE void fav_all_selected(const QSet<QByteArray> &selected, int);
    Q_INVOKABLE void tag_all_selected(const QSet<QByteArray> &selected, const QByteArray &tag);
    Q_INVOKABLE void untag_all_selected(const QSet<QByteArray> &selected, const QByteArray &tag);

    int mid_to_index(QByteArray mid);

public slots:
    void msg_recv_status(int cmd, const QString& mid, const QString& huid);
    void mid_tag_changed(QString mid);
    void msg_recv_progress(QString mid, QString huid, QString msg, int percent);
    void invalidate_mid(const QByteArray& mid, const QString& huid);

    void reload_model(int force = 0);
    void reload_model2(const QString&);
    void reload_inbox_model();

signals:
    void invalidate_item(const QByteArray& mid);

private:
    DWYCO_MSG_IDX msg_idx;
    DWYCO_QD_MSG_LIST qd_msgs;
    DWYCO_UNFETCHED_MSG_LIST inbox_msgs;
    int count_msg_idx;
    int count_qd_msgs;
    int count_inbox_msgs;

    QVariant qd_data (int r, int role = Qt::DisplayRole ) const;
    QVariant inbox_data (int r, int role = Qt::DisplayRole ) const;
    int check_inbox_model();
    int check_qd_msgs();

    QString get_msg_text(int row) const;
    QString preview_filename(int row) const;
};

extern msglist_raw *mlm;


#endif
