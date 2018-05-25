
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
#include "msglistmodel.h"
#include "msgpv.h"
#include "pfx.h"
#include "dwycolistscoped.h"
#include "dwyco_new_msg.h"
msglist_model *mlm;

static QSet<QByteArray> Selected;
static QList<QByteArray> Fetching;
static QSet<QByteArray> Dont_refetch;
static QList<QByteArray> Delete_msgs;
static QMap<int, QByteArray> Fid_to_mid;
static QMap<QByteArray, int> Mid_to_percent;
static QSet<QByteArray> Manual_fetch;

enum {
    MID = Qt::UserRole,
    SENT,
    MSG_TEXT,
    PREVIEW_FILENAME,
    HAS_VIDEO,
    HAS_SHORT_VIDEO,
    HAS_AUDIO,
    IS_FILE,
    IS_FORWARDED,
    IS_NO_FORWARD,
    DATE_CREATED,
    DATE_RECEIVED,
    LOCAL_TIME_CREATED, // this take into account time zone and all that
    IS_QD,
    IS_ACTIVE,
    IS_FAVORITE,
    SELECTED,
    DIRECT,
    FETCH_STATE,
    ATTACHMENT_PERCENT
};

static int
dwyco_get_attr(DWYCO_LIST l, int row, const char *col, QByteArray& str_out)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_STRING)
        return 0;
    str_out = QByteArray(val, len);
    return 1;
}

static int
dwyco_get_attr_bool(DWYCO_LIST l, int row, const char *col)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_STRING)
        return 0;
    QByteArray str_out(val, len);
    if(str_out == "t")
        return 1;
    return 0;
}

static
QString
gen_time(DWYCO_SAVED_MSG_LIST l, int row)
{
    int hour;
    int minute;
    int second;

    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, DWYCO_QM_BODY_DATE_HOUR, &val, &len, &type))
        return "";
    if(type != DWYCO_TYPE_INT)
        return "";
    hour = atoi(val);

    if(!dwyco_list_get(l, row, DWYCO_QM_BODY_DATE_MINUTE, &val, &len, &type))
        return "";
    if(type != DWYCO_TYPE_INT)
        return "";
    minute = atoi(val);

    if(!dwyco_list_get(l, row, DWYCO_QM_BODY_DATE_SECOND, &val, &len, &type))
        return "";
    if(type != DWYCO_TYPE_INT)
        return "";
    second = atoi(val);

    QTime qt(hour, minute, second);
    QString t = qt.toString("hh:mm ap");
    return t;
}

static
QString
gen_time_unsaved(DWYCO_UNSAVED_MSG_LIST l, int row)
{
    int hour;
    int minute;
    int second;

    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, DWYCO_QMS_DS_HOUR, &val, &len, &type))
        return "";
    if(type != DWYCO_TYPE_INT)
        return "";
    hour = atoi(val);

    if(!dwyco_list_get(l, row, DWYCO_QMS_DS_MINUTE, &val, &len, &type))
        return "";
    if(type != DWYCO_TYPE_INT)
        return "";
    minute = atoi(val);

    if(!dwyco_list_get(l, row, DWYCO_QMS_DS_SECOND, &val, &len, &type))
        return "";
    if(type != DWYCO_TYPE_INT)
        return "";
    second = atoi(val);

    QTime qt(hour, minute, second);
    QString t = qt.toString("hh:mm ap");
    return t;
}

static
void
DWYCOCALLCONV
msg_status_callback(int id, const char *text, int percent_done, void *)
{
    //if(!Fid_to_mid.contains(id))
    //    return;
    QByteArray mid = Fid_to_mid.value(id);
    Mid_to_percent.insert(mid, percent_done);
    int midi = mlm->mid_to_index(mid);
    QModelIndex mi = mlm->index(midi, 0);
    mlm->dataChanged(mi, mi, QVector<int>(1, ATTACHMENT_PERCENT));
    //mlm->dataChanged(mi, mi, QVector<int>(1, FETCH_STATE));
}

void
msglist_model::msg_recv_status(int cmd, const QString &smid)
{
    QByteArray mid = smid.toLatin1();

    int i = Fetching.indexOf(mid);
    switch(cmd)
    {
    case DWYCO_SE_MSG_DOWNLOAD_START:
    // here is a case where it makes sense to provide the text of the message
    // immediately while the attachment is being downloaded in the background.
    case DWYCO_SE_MSG_DOWNLOAD_FETCHING_ATTACHMENT:
        break;

    case DWYCO_SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED:
    case DWYCO_SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED_DECRYPT_FAILED:
        // if this msg is ratholed, it can never be fetched, so just delete it.
        Dont_refetch.insert(mid);
        if(i >= 0)
            Fetching.removeAt(i);
        Delete_msgs.append(mid);
        //del_unviewed_mid(mid);
        //Fid_to_mid.remove(id);
        Mid_to_percent.remove(mid);
        break;

    case DWYCO_SE_MSG_DOWNLOAD_ATTACHMENT_FETCH_FAILED:
    case DWYCO_SE_MSG_DOWNLOAD_FAILED:
        // this is potentially just a transient failure, so maybe try refetching
        // a certain number of times. really need something from the server that says
        // whether there is any hope of getting the message or not.
        // i think i will handle this case on the server and just delete messages that
        // have not been fetched in a month or something. this will cause a bit of thrashing for
        // users that have a lot of messages that can't be fetched, but gives the best chance to get
        // a message in transient failure situations.
        //Dont_refetch.insert(mid);
        //del_unviewed_mid(mid);

        if(i >= 0)
            Fetching.removeAt(i);
        //Fid_to_mid.remove(id);
        Mid_to_percent.remove(mid);
        Manual_fetch.insert(mid);
        break;
    case DWYCO_SE_MSG_DOWNLOAD_OK:
        // reload the inbox since a server messages
        // just got transformed into a direct message
    {
        msglist_raw *mr = dynamic_cast<msglist_raw *>(sourceModel());
        mr->reload_inbox_model();
    }
    // FALLTHRU
    default:
        if(i >= 0)
            Fetching.removeAt(i);
        //Fid_to_mid.remove(id);
        Mid_to_percent.remove(mid);
        Manual_fetch.remove(mid);
    }
    int midi = mlm->mid_to_index(mid);
    if(midi < 0)
        return;
    QModelIndex mi = mlm->index(midi, 0);
    QVector<int> roles;
    roles.append(IS_ACTIVE);
    roles.append(FETCH_STATE);
    roles.append(ATTACHMENT_PERCENT);
    roles.append(DIRECT);
    mlm->dataChanged(mi, mi, roles);
    //mlm->dataChanged(mi, mi, QVector<int>(1, FETCH_STATE));
    //mlm->dataChanged(mi, mi, QVector<int>(1, ATTACHMENT_PERCENT));

}


msglist_model::msglist_model(QObject *p) :
    QSortFilterProxyModel(p)
{
    filter_show_recv = 1;
    filter_show_sent = 1;
    filter_last_n = -1;
    filter_only_favs = 0;
    msglist_raw *m = new msglist_raw(p);
    setSourceModel(m);
    mlm = this;
}

msglist_model::~msglist_model()
{

}

void
msglist_model::toggle_selected(QByteArray bmid)
{
    if(Selected.contains(bmid))
        Selected.remove(bmid);
    else
        Selected.insert(bmid);
    int i = mid_to_index(bmid);
    QModelIndex mi = index(i, 0);
    if(i != -1)
        emit dataChanged(mi, mi, QVector<int>(1, SELECTED));

}

void
msglist_model::set_all_unselected()
{
    QSet<QByteArray> oselected = Selected;
    Selected.clear();
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
    return Selected.count() > 0;
}

int
msglist_model::mid_to_index(QByteArray bmid)
{
    int n = rowCount();
    for(int i = 0; i < n; ++i)
    {
        QModelIndex mi = index(i, 0);
        if(data(mi, MID).toByteArray() == bmid)
        {
            return i;
        }
    }
    return -1;
}

void
msglist_model::delete_all_selected()
{
    QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
    foreach (const QString &value, Selected)
    {
        QByteArray mid = value.toLatin1();
        DWYCO_LIST l;
        if(dwyco_get_unsaved_message(&l, mid.constData()))
        {
            dwyco_list_release(l);
            dwyco_delete_unsaved_message(mid.constData());
        }
        else if(dwyco_qd_message_to_body(&l, mid.constData(), mid.length()))
        {
            dwyco_list_release(l);
            dwyco_kill_message(mid.constData(), mid.length());
        }
        else
        {
            if(!dwyco_get_fav_msg(0, 0, mid.constData()))
                dwyco_delete_saved_message(buid.constData(), buid.length(), mid.constData());
        }

    }
    Selected.clear();
    reload_model();
}

void
msglist_model::fav_all_selected(int f)
{
    QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
    foreach (const QString &value, Selected)
    {
        QByteArray b = value.toLatin1();
        DWYCO_LIST l;
        if(dwyco_qd_message_to_body(&l, b.constData(), b.length()))
        {
            dwyco_list_release(l);
            continue;
        }
        dwyco_set_fav_msg(buid.constData(), buid.length(), b.constData(), f);
    }
    reload_model();
}

void
msglist_model::setUid(const QString &uid)
{
    if(m_uid != uid)
    {
        Selected.clear();
        msglist_raw *mr = dynamic_cast<msglist_raw *>(sourceModel());
        if(mr)
        {
            mr->setUid(uid);
            m_uid = uid;
            emit uidChanged();
        }
    }
}

QString
msglist_model::uid() const
{
    return m_uid;
}

void
msglist_model::reload_model()
{
    msglist_raw *mr = dynamic_cast<msglist_raw *>(sourceModel());
    if(mr)
    {
        mr->reload_model();
    }
}

void
msglist_model::set_filter(int sent, int recv, int last_n, int only_favs)
{
    filter_show_recv = recv;
    filter_show_sent = sent;
    filter_last_n = last_n;
    filter_only_favs = only_favs;
    invalidateFilter();
}

bool
msglist_model::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QAbstractItemModel *alm = sourceModel();

    QVariant is_sent = alm->data(alm->index(source_row, 0), SENT);
    if(filter_show_sent == 0 && is_sent.toInt() == 1)
        return 0;
    if(filter_show_recv == 0 && is_sent.toInt() == 0)
        return 0;
    if(filter_only_favs)
    {
        QVariant is_fav = alm->data(alm->index(source_row, 0), IS_FAVORITE);
        if(is_fav.toInt() == 1)
            return 1;
        else
            return 0;
    }
    return 1;
}

msglist_raw::msglist_raw(QObject *p)
    : QAbstractListModel(p)
{
    msg_idx = 0;
    qd_msgs = 0;
    inbox_msgs = 0;
    count_inbox_msgs = 0;
    count_msg_idx = 0;
    count_qd_msgs = 0;
}

msglist_raw::~msglist_raw()
{
    if(msg_idx)
        dwyco_list_release(msg_idx);
    if(qd_msgs)
        dwyco_list_release(qd_msgs);
    if(inbox_msgs)
        dwyco_list_release(inbox_msgs);
}

void
msglist_raw::reload_inbox_model()
{
    QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
    beginResetModel();
    if(inbox_msgs)
        dwyco_list_release(inbox_msgs);

    inbox_msgs = 0;
    count_inbox_msgs = 0;

    if(buid.length() != 10)

    {
        endResetModel();
        return;
    }



    dwyco_get_unsaved_messages(&inbox_msgs, buid.constData(), buid.length());

    if(inbox_msgs)
        dwyco_list_numelems(inbox_msgs, &count_inbox_msgs, 0);

    endResetModel();

}

void
msglist_raw::reload_model()
{
    QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
    beginResetModel();
    if(msg_idx)
        dwyco_list_release(msg_idx);
    if(qd_msgs)
        dwyco_list_release(qd_msgs);
    if(inbox_msgs)
        dwyco_list_release(inbox_msgs);
    msg_idx = 0;
    qd_msgs = 0;
    inbox_msgs = 0;
    count_inbox_msgs = 0;
    count_msg_idx = 0;
    count_qd_msgs = 0;
    // ugh, need to fix this to validate the uid some way
    if(buid.length() != 10)
    {
        endResetModel();
        return;
    }

    dwyco_get_message_index(&msg_idx, buid.constData(), buid.length());
    dwyco_get_qd_messages(&qd_msgs, buid.constData(), buid.length());
    dwyco_get_unsaved_messages(&inbox_msgs, buid.constData(), buid.length());
    if(msg_idx)
        dwyco_list_numelems(msg_idx, &count_msg_idx, 0);
    if(qd_msgs)
        dwyco_list_numelems(qd_msgs, &count_qd_msgs, 0);
    if(inbox_msgs)
        dwyco_list_numelems(inbox_msgs, &count_inbox_msgs, 0);

    endResetModel();
}

void
msglist_raw::setUid(const QString &uid)
{
    if(m_uid != uid)
    {
        m_uid = uid;
        reload_model();
    }
}



int
msglist_raw::rowCount ( const QModelIndex & parent) const
{
    if(parent.isValid())
        return 0;

    return count_inbox_msgs + count_msg_idx + count_qd_msgs;
}

QHash<int, QByteArray>
msglist_raw::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole] = "mid";
    roles[Qt::DisplayRole] = "display";
    roles[Qt::DecorationRole] = "decoration";

#define rn(N) roles[N] = #N
    rn(SENT);
    rn(MSG_TEXT);
    rn(PREVIEW_FILENAME);
    rn(HAS_VIDEO);
    rn(HAS_SHORT_VIDEO);
    rn(HAS_AUDIO);
    rn(IS_FILE);
    rn(IS_QD);
    rn(IS_ACTIVE);
    rn(IS_FORWARDED);
    rn(IS_NO_FORWARD);
    rn(DATE_CREATED);
    rn(DATE_RECEIVED);
    rn(LOCAL_TIME_CREATED);
    rn(IS_FAVORITE);
    rn(SELECTED);
    rn(DIRECT);
    rn(FETCH_STATE);
    rn(ATTACHMENT_PERCENT);
#undef rn
    return roles;
}

QVariant
msglist_raw::qd_data ( int r, int role ) const
{
    QByteArray pers_id;
    dwyco_get_attr(qd_msgs, r, DWYCO_QD_MSG_PERS_ID, pers_id);

    DWYCO_SAVED_MSG_LIST sm;
    if(!dwyco_qd_message_to_body(&sm, pers_id.constData(), pers_id.length()))
        return QVariant();
    simple_scoped qsm(sm);

    // there is not "mid" per-se for qd messages, as the id needs to be
    // assigned by the server (for associating read-indications as stuff.)
    // but we know the pers-id and mid are unique so we just use pers-id
    // this will allow people to cancel half-sent messages.
    switch(role)
    {
    case MID:
        return pers_id;
    case SELECTED:
    {
        return Selected.contains(pers_id);
    }
    case DIRECT:
        return 0;
    case IS_QD:
        return 1;
    case IS_ACTIVE:
        return false;
    case MSG_TEXT:
    {
        DWYCO_LIST ba = dwyco_get_body_array(qsm);
        simple_scoped qba(ba);
        QByteArray txt;
        if(!dwyco_get_attr(qba, 0, DWYCO_QM_BODY_NEW_TEXT2, txt))
            return "";

        return QString(txt);
    }
    case SENT:
        return 1;
    case PREVIEW_FILENAME:
    {
        int is_file;
        QString local_time;
        QByteArray full_size;
        QByteArray pfn;

        if(!preview_unsaved_msg(qsm, pfn, is_file, full_size, local_time))
            return QVariant("");
        return QString(pfn);

    }
    case DATE_CREATED:
    {
        DWYCO_LIST ba = dwyco_get_body_array(qsm);
        simple_scoped qba(ba);
        QByteArray txt;
        const char *out;
        int len_out;
        int type_out;
        if(!dwyco_list_get(qba, 0, DWYCO_QM_BODY_DATE_SECONDS_SINCE_JAN1_1970,
                           &out, &len_out, &type_out))
            return 0;
        if(type_out != DWYCO_TYPE_INT)
            return 0;
        QVariant v;
        v.setValue(QString(out).toLong());
        return v;

    }

    case LOCAL_TIME_CREATED:
    {
        DWYCO_LIST ba = dwyco_get_body_array(qsm);
        simple_scoped qba(ba);
        QByteArray txt;
        const char *out;
        int len_out;
        int type_out;
        if(!dwyco_list_get(qba, 0, DWYCO_QM_BODY_DATE_SECONDS_SINCE_JAN1_1970,
                           &out, &len_out, &type_out))
            return 0;
        if(type_out != DWYCO_TYPE_INT)
            return 0;
        QDateTime dt;
        dt = QDateTime::fromSecsSinceEpoch(QString(out).toLong());
        return dt.toString("hh:mm ap");


    }
    case HAS_AUDIO:
    case HAS_SHORT_VIDEO:
    case HAS_VIDEO:
    case IS_FAVORITE:
        return 0;

    case IS_FORWARDED:
    {
        DWYCO_LIST ba = dwyco_get_body_array(qsm);
        simple_scoped qba(ba);
        int n;
        if(!dwyco_list_numelems(qba, &n, 0))
            return 0;
        if(n > 1)
            return 1;
        return 0;
    }

    case ATTACHMENT_PERCENT:
        return -1.0;
    case FETCH_STATE:
        return QString("none");

    case Qt::DecorationRole:
        return QVariant("qrc:///new/red32/icons/red-32x32/Upload-32x32.png");

    default:
        return QVariant();
    }

}

void
clear_manual_gate()
{
    Manual_fetch.clear();
}

int
auto_fetch(QByteArray mid)
{
    if(!(Fetching.contains(mid) || Manual_fetch.contains(mid)))
    {
        int special_type;
        const char *uid;
        int len_uid;
        const char *dlv_mid;
        if(dwyco_is_delivery_report(mid.constData(), &uid, &len_uid, &dlv_mid, &special_type))
        {
            // process pal authorization stuff here
            if(special_type == DWYCO_SUMMARY_DELIVERED)
            {
                // NOTE: uid, dlv_mid must be copied out before next
                // dll call
                // hmmm, need new api to get uid/mid_out of delivered msg
                dwyco_delete_unsaved_message(mid.constData());
                return 0;
            }

        }
        // issue a server fetch, client will have to
        // come back in to get it when the fetch is done

        int fetch_id = dwyco_fetch_server_message(mid.constData(), 0, 0, msg_status_callback, 0);
        if(fetch_id != 0)
        {
            Fetching.append(mid);
            Fid_to_mid.insert(fetch_id, mid);
            return 1;
        }
    }
    return 0;
}

int
retry_auto_fetch(QByteArray mid)
{
    Manual_fetch.remove(mid);
    int tmp = auto_fetch(mid);
    int midi = mlm->mid_to_index(mid);
    if(midi < 0)
        return tmp;
    QModelIndex mi = mlm->index(midi, 0);
    QVector<int> roles;
    roles.append(IS_ACTIVE);
    roles.append(FETCH_STATE);
    roles.append(ATTACHMENT_PERCENT);
    mlm->dataChanged(mi, mi, roles);
    return tmp;
}

QVariant
msglist_raw::inbox_data (int r, int role ) const
{
    QByteArray mid;
    const char *out;
    int len_out;
    int type_out;

    int n = count_inbox_msgs;
    //dwyco_list_numelems(inbox_msgs, &n, 0);
    // inbox msgs are sorted in the opposite order (newest last)
    r = n - r - 1;
    dwyco_get_attr(inbox_msgs, r, DWYCO_QMS_ID, mid);
    if(role == MID)
        return mid;

    int direct = dwyco_get_attr_bool(inbox_msgs, r, DWYCO_QMS_IS_DIRECT);

    DWYCO_SAVED_MSG_LIST sm = 0;
    if(direct)
    {
        if(!dwyco_unsaved_message_to_body(&sm, mid.constData()))
        {
            return QVariant();
        }
    }

    simple_scoped qsm(sm);

    switch(role)
    {
    case MID:
        return mid;
    case SELECTED:
    {
        return Selected.contains(mid);
    }
    case IS_QD:
        return QVariant(0);
    case IS_ACTIVE:
    {
        return Fetching.contains(mid);
    }
    case DIRECT:
        return direct;

    case DATE_CREATED:
    {
        if(!dwyco_list_get(inbox_msgs, r, DWYCO_QMS_DS_SECONDS_SINCE_JAN1_1970,
                           &out, &len_out, &type_out))
            return QVariant();
        if(type_out != DWYCO_TYPE_INT)
            return QVariant();
        QVariant v;
        v.setValue(QString(out).toLong());
        return v;
    }

    case LOCAL_TIME_CREATED:
    {
        return gen_time_unsaved(inbox_msgs, r);
    }

    case MSG_TEXT:
    {
        if(!direct)
        {
            auto_fetch(mid);
            return "(fetching)";
        }
        DWYCO_LIST ba = dwyco_get_body_array(qsm);
        simple_scoped qba(ba);
        QByteArray txt;
        if(!dwyco_get_attr(qba, 0, DWYCO_QM_BODY_NEW_TEXT2, txt))
            return "";

        return QVariant(QString(txt));
    }
    case FETCH_STATE:
    {
        if(Manual_fetch.contains(mid))
            return QString("manual");
        return QString("auto");
    }
    case SENT:
        return QVariant(0);
    case PREVIEW_FILENAME:
    {
        return QVariant("");
    }
    case HAS_AUDIO:
    case HAS_SHORT_VIDEO:
    case HAS_VIDEO:
    case IS_FAVORITE:
    case IS_FORWARDED:
        return 0;

    case ATTACHMENT_PERCENT:
        if(!Mid_to_percent.contains(mid))
            return -1.0;
        return (double)Mid_to_percent.value(mid);

    case Qt::DecorationRole:
        return QVariant("qrc:///new/red32/icons/red-32x32/Upload-32x32.png");

    default:
        return QVariant();
    }

}

// note: in the lists, the newest msgs are the ones at the
// top (index 0). unfortunately, inbox messages are
// returned with the newest one at the end, so we have to
// translate that a little bit.
QVariant
msglist_raw::data ( const QModelIndex & index, int role ) const
{
    if(!index.isValid())
        return QVariant();
    if(index.column() != 0)
        return QVariant();

    int r = index.row();

    int r2 = count_inbox_msgs;
//    if(inbox_msgs)
//    {
//        dwyco_list_numelems(inbox_msgs, &r2, 0);
//    }

    int r1 = count_qd_msgs;
//    if(qd_msgs)
//    {
//        dwyco_list_numelems(qd_msgs, &r1, 0);
//    }

    if(r < r2)
    {
        return inbox_data(r, role);
    }

    if(r >= r2 && r < r1 + r2)
    {
        return qd_data(r - r2, role);
    }
    r -= r1 + r2;

    const char *out;
    int len_out;
    int type_out;

    if(role == IS_QD)
    {
        return QVariant(0);
    }
    if(role == MID)
    {
        QByteArray mid;
        if(!dwyco_get_attr(msg_idx, r, DWYCO_MSG_IDX_MID, mid))
            return QVariant();

        return mid;
    }
    if(role == SELECTED)
    {
        return Selected.contains(data(index, MID).toByteArray());
    }
    if(role == Qt::DisplayRole)
    {
        if(!dwyco_list_get(msg_idx, r, DWYCO_MSG_IDX_DATE,
                           &out, &len_out, &type_out))
            return QVariant();
        if(type_out != DWYCO_TYPE_INT)
            return QVariant();
        QDateTime q(QDateTime::fromTime_t(atol(out)));
        return QVariant(q);
    }
    else if(role == DATE_CREATED)
    {
        if(!dwyco_list_get(msg_idx, r, DWYCO_MSG_IDX_DATE,
                           &out, &len_out, &type_out))
            return QVariant();
        if(type_out != DWYCO_TYPE_INT)
            return QVariant();
        QVariant v;
        v.setValue(QString(out).toLong());
        return v;

    }
    else if(role == SENT)
    {
        if(dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_IS_SENT))
        {
            return QVariant(1);
        }
        return QVariant(0);
    }
    else if( role == Qt::DecorationRole)
    {
        if(dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_HAS_ATTACHMENT) == 0)
        {
            return QVariant("");
        }
        if(dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_IS_FILE) != 0)
        {
            return QVariant("qrc:///new/prefix1/icons/downlaod-16x16.png");
        }
        if(dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_ATT_HAS_VIDEO) != 0)
        {
            if(dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_ATT_IS_SHORT_VIDEO) != 0)
                return QVariant("qrc:///new/prefix1/icons/media-1-16x16.png");
            else
                return QVariant("qrc:///new/prefix1/icons/media 2-16x16.png");
        }
        if(dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_ATT_HAS_AUDIO) != 0)
            return QVariant("qrc:///new/prefix1/icons/music 2-16x16.png");
        return QVariant("qrc:///new/prefix1/icons/stop.png");
    }
    else if(role == MSG_TEXT)
    {
        return get_msg_text(r);
    }
    else if(role == HAS_VIDEO)
    {
        if(dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_HAS_ATTACHMENT) == 0)
        {
            return 0;
        }
        if(dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_ATT_HAS_VIDEO) != 0)
        {
            return 1;
        }
        return 0;

    }
    else if(role == HAS_SHORT_VIDEO)
    {
        if(dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_HAS_ATTACHMENT) == 0)
        {
            return 0;
        }
        if(dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_ATT_IS_SHORT_VIDEO) == 1)
            return 1;
        return 0;
    }
    else if(role == HAS_AUDIO)
    {
        if(dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_HAS_ATTACHMENT) == 0)
        {
            return 0;
        }
        if(dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_ATT_HAS_AUDIO) == 1)
            return 1;
        return 0;
    }
    else if(role == IS_FILE)
    {
        if(dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_HAS_ATTACHMENT) == 0)
        {
            return 0;
        }
        if(dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_IS_FILE) != 0)
        {
            return 1;
        }
        return 0;

    }
    else if (role == IS_FORWARDED)
    {
        if(!dwyco_get_attr_bool(msg_idx, r, DWYCO_MSG_IDX_IS_FORWARDED))
            return 0;
        return 1;
    }
    else if(role == LOCAL_TIME_CREATED)
    {
        // hmm, consider putting the local time in the index, as the way it is now
        // we would have to read in the message to get it, which may be
        // too expensive. we know the local time is only really displayed for
        // "extra info", so it isn't used all that much
        QByteArray mid;
        if(!dwyco_get_attr(msg_idx, r, DWYCO_MSG_IDX_MID, mid))
            return "unknown";
        QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
        DWYCO_SAVED_MSG_LIST sm;
        if(!dwyco_get_saved_message(&sm, buid.constData(), buid.length(), mid.constData()))
            return "unknown";
        simple_scoped qsm(sm);

        DWYCO_LIST ba = dwyco_get_body_array(qsm);
        simple_scoped qba(ba);
        return gen_time(ba, 0);
    }
    else if(role == IS_FAVORITE)
    {
        QByteArray mid;
        if(!dwyco_get_attr(msg_idx, r, DWYCO_MSG_IDX_MID, mid))
            return QVariant();

        if(dwyco_get_fav_msg(0, 0, mid.constData()))
            return 1;
        return 0;

    }
    else if(role == PREVIEW_FILENAME)
    {
        return preview_filename(r);
    }
    else if(role == IS_ACTIVE)
        return false;
    else if(role == ATTACHMENT_PERCENT)
        return -1.0;
    else if(role == FETCH_STATE)
        return QString("fetched");

    return QVariant();

}


QString
msglist_raw::get_msg_text(int row) const
{
    QByteArray mid;
    if(!dwyco_get_attr(msg_idx, row, DWYCO_MSG_IDX_MID, mid))
        return "";
    DWYCO_SAVED_MSG_LIST sm;

    QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
    if(!dwyco_get_saved_message(&sm, buid.constData(), buid.length(), mid.constData()))
        return "";
    simple_scoped qsm(sm);

    DWYCO_LIST ba = dwyco_get_body_array(qsm);
    simple_scoped qba(ba);

    int n;
    if(dwyco_list_numelems(qba, &n, 0))
    {
        if(n > 1)
        {
            // forwarded message, for now just return the
            // yucky dll formatted message
            QByteArray ftxt;
            DWYCO_LIST bt = dwyco_get_body_text(qba);
            if(!bt)
                return "";
            simple_scoped qbt(bt);
            if(!dwyco_get_attr(qbt, 0, DWYCO_NO_COLUMN, ftxt))
                return "";
            return get_extended(ftxt);
        }
    }

    QByteArray txt;
    if(!dwyco_get_attr(qba, 0, DWYCO_QM_BODY_NEW_TEXT2, txt))
        return "";

    return get_extended(txt);
}

QString
msglist_raw::preview_filename(int row) const
{
    QByteArray pfn;
    QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
    QByteArray mid;
    int is_file;
    QString local_time;

    if(!dwyco_get_attr(msg_idx, row, DWYCO_MSG_IDX_MID, mid))
        return "";
    QByteArray full_size;

    if(!preview_saved_msg(buid, mid, pfn, is_file, full_size, local_time))
        return "";
    return pfn;
}
