
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
#include <QCryptographicHash>
#include <QFileDevice>
#include "msglistmodel.h"
#include "msgpv.h"
#include "pfx.h"
#include "dwycolist2.h"
#include "dwyco_new_msg.h"
#include "dwyco_top.h"
#include "qloc.h"
#ifdef DWYCO_MODEL_TEST
#include <QAbstractItemModelTester>
#endif

class DwycoCore;
extern DwycoCore *TheDwycoCore;
void update_unseen_from_db();

// note: this model integrates 3 lists when a particular uid is
// selected: the saved message list, the inbox (just msgs from that uid) and
// the set of messages that are queued to send that that uid.
//
// alternately, if you set a tag, the model is constructed of just messages
// having that tag (from all uids), with two special values: "_fav" for favorites, and
// "_hid" for hidden messages.
//
// this needs to be fixed, either allow multiple
// models, or promote this to bona-fide singleton
msglist_model *mlm;

static QSet<QByteArray> Selected;
static QList<QByteArray> Fetching;
static QSet<QByteArray> Dont_refetch;
static QList<QByteArray> Delete_msgs;
static QMap<QByteArray, int> Mid_to_percent;
// messages are automatically fetched, unless it fails.
// after that, the fetch can be initiated explicitly
static QSet<QByteArray> Manual_fetch;

extern QMultiMap<QByteArray,QLoc> Hash_to_loc;
extern QMap<QByteArray,QByteArray> Hash_to_review;
extern QMap<QByteArray,long> Hash_to_max_lc;

static QMap<QByteArray,QByteArray> Mid_to_hash;

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
    LOGICAL_CLOCK,
    IS_QD,
    IS_ACTIVE,
    IS_FAVORITE,
    IS_HIDDEN,
    SELECTED,
    DIRECT,
    FETCH_STATE,
    ATTACHMENT_PERCENT,
    ASSOC_UID, // who the message is from (or to, if sent msg)
    SENT_TO_LOCATION,
    REVIEW_RESULTS,
    IS_UNSEEN,
    ASSOC_HASH,
    SENT_TO_LAT,
    SENT_TO_LON,
    IS_UNFETCHED
};


static
QString
gen_time(DWYCO_SAVED_MSG_LIST l, int row)
{
    int hour;
    int minute;
    int second;

    dwyco_list_throw m(l);
    try {
        hour = m.get_long(row, DWYCO_QM_BODY_DATE_HOUR);
        minute = m.get_long(row, DWYCO_QM_BODY_DATE_MINUTE);
        second = m.get_long(row, DWYCO_QM_BODY_DATE_SECOND);
    }
    catch (...) {
        return "";
    }

    QTime qt(hour, minute, second);
    QString t = qt.toString("hh:mm ap");
    return t;
}

static
QString
gen_time_unsaved(DWYCO_UNFETCHED_MSG_LIST l, int row)
{
    int hour;
    int minute;
    int second;

    dwyco_list_throw m(l);
    try {
        hour = m.get_long(row, DWYCO_QMS_DS_HOUR);
        minute = m.get_long(row, DWYCO_QMS_DS_MINUTE);
        second = m.get_long(row, DWYCO_QMS_DS_SECOND);
    }  catch (...) {
        return "";
    }

    QTime qt(hour, minute, second);
    QString t = qt.toString("hh:mm ap");
    return t;
}

static
int
att_file_hash(const QByteArray& mid, QByteArray& hash_out)
{
    DWYCO_SAVED_MSG_LIST qsm;
    QByteArray uid;
    if(!dwyco_get_saved_message(&qsm, uid.constData(), uid.length(), mid.constData()))
    {
        return 0;
    }
    simple_scoped sm(qsm);
    QByteArray aname = sm.get<QByteArray>(DWYCO_QM_BODY_ATTACHMENT);
    if(aname.length() == 0)
    {
        return 0;
    }

    if(Mid_to_hash.contains(mid))
    {
        hash_out = Mid_to_hash.value(mid);
        return 1;
    }

    QByteArray user_filename = sm.get<QByteArray>(DWYCO_QM_BODY_FILE_ATTACHMENT);
    int is_file = user_filename.length() > 0;
    if(!is_file)
        return 0;

    const char *buf = 0;
    int len = 0;
    if(!dwyco_copy_out_file_zap_buf2(mid.constData(), &buf, &len, 4096))
        return 0;
    QCryptographicHash ch(QCryptographicHash::Sha1);
    QByteArrayView b(buf, len);
    ch.addData(b);

    QByteArray res = ch.result();
    Mid_to_hash.insert(mid, res);
    hash_out = res;
    dwyco_free_array((char *)buf);
    return 1;
}


void
msglist_model::msg_recv_progress(QString mid, QString huid, QString msg, int percent_done)
{
    QByteArray bmid = mid.toLatin1();
    Mid_to_percent.insert(bmid, percent_done);
    int midi = mid_to_index(bmid);
    QModelIndex mi = index(midi, 0);
    emit dataChanged(mi, mi, QVector<int>(1, ATTACHMENT_PERCENT));
}

// note: despite this being a member function, it is called for any message
// that might getting downloaded independent of whatever uid happens to be loaded
// into the model at a given time.
void
msglist_model::msg_recv_status(int cmd, const QString &smid, const QString& shuid)
{
    QByteArray mid = smid.toLatin1();
    QByteArray huid = shuid.toLatin1();
    QByteArray buid = QByteArray::fromHex(huid);

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

        if(i >= 0)
            Fetching.removeAt(i);
        Mid_to_percent.remove(mid);
        Manual_fetch.insert(mid);
        break;
    case DWYCO_SE_MSG_DOWNLOAD_OK:
        // reload the inbox since a server messages
        // just got transformed into a direct message
    {
        msglist_raw *mr = dynamic_cast<msglist_raw *>(sourceModel());
        mr->reload_inbox_model();
        add_unviewed(buid, mid);
        load_to_hash(buid, mid);
        dwyco_unset_msg_tag(mid.constData(), "_inbox");
        TheDwycoCore->emit new_msg(shuid, "", smid);
        TheDwycoCore->emit decorate_user(shuid);
        update_unseen_from_db();
        if(uid() == shuid)
            invalidate_sent_to();

    }
    // FALLTHRU
    default:
        if(i >= 0)
            Fetching.removeAt(i);
        Mid_to_percent.remove(mid);
        Manual_fetch.remove(mid);
    }
    int midi = mid_to_index(mid);
    if(midi < 0)
        return;
    QModelIndex mi = index(midi, 0);
    QVector<int> roles;
    roles.append(IS_ACTIVE);
    roles.append(FETCH_STATE);
    roles.append(ATTACHMENT_PERCENT);
    roles.append(PREVIEW_FILENAME);
    //roles.append(DIRECT);
    emit dataChanged(mi, mi, roles);
    mlm->invalidateFilter();
}


msglist_model::msglist_model(QObject *p) :
    QSortFilterProxyModel(p)
{
    filter_show_recv = 1;
    filter_show_sent = 1;
    filter_last_n = -1;
    filter_only_favs = 0;
    filter_show_hidden = 1;
    special_sort = false;
    msglist_raw *m = new msglist_raw(p);
    setDynamicSortFilter(true);
    setSourceModel(m);
    mlm = this;
#ifdef DWYCO_MODEL_TEST
    new QAbstractItemModelTester(this);
#endif
}

msglist_model::~msglist_model()
{
    // not needed, as msglist is a singleton, but
    // useful to uncomment this for leak checking
    //delete sourceModel();
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
msglist_model::set_all_selected()
{
    int n = rowCount();
    for(int i = 0; i < n; ++i)
    {
        QModelIndex mi = index(i, 0);
        QByteArray mid = data(mi, MID).toByteArray();
        Selected.insert(mid);
        emit dataChanged(mi, mi, QVector<int>(1, SELECTED));
    }
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

// this is a bit sloppy, but since there aren't that
// many things in the list, it hopefully won't be a problem
// to just invalidate everything
void
msglist_model::invalidate_sent_to()
{
    int n = rowCount();
    for(int i = 0; i < n; ++i)
    {
        QModelIndex mi = index(i, 0);
        QVector<int> v;
        v.append(SENT_TO_LOCATION);
        v.append(SENT_TO_LAT);
        v.append(SENT_TO_LON);
        v.append(REVIEW_RESULTS);
        v.append(IS_UNSEEN);

        emit dataChanged(mi, mi, v);
    }
    if(special_sort)
        sort(0, Qt::DescendingOrder);
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

// note: this is a kluge, we just invalidate all tag-based
// attributes, causes a little more refreshing than needed, but
// is ok for now. should fix it...
void
msglist_model::mid_tag_changed(QString mid)
{
    int midi = mid_to_index(mid.toLatin1());
    QModelIndex mi = index(midi, 0);
    QVector<int> roles;
    roles.append(IS_HIDDEN);
    roles.append(IS_FAVORITE);
    roles.append(IS_UNSEEN);
    dataChanged(mi, mi, roles);
}

void
msglist_model::delete_all_selected()
{
    QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
    foreach (const QString &value, Selected)
    {
        QByteArray mid = value.toLatin1();
        DWYCO_LIST l;
        if(dwyco_get_unfetched_message(&l, mid.constData()))
        {
            dwyco_list_release(l);
            dwyco_delete_unfetched_message(mid.constData());
        }
        else if(dwyco_qd_message_to_body(&l, mid.constData(), mid.length()))
        {
            dwyco_list_release(l);
            dwyco_kill_message(mid.constData(), mid.length());
        }
        else
        {
            if(!dwyco_get_fav_msg(mid.constData()))
                dwyco_delete_saved_message(buid.constData(), buid.length(), mid.constData());
        }

    }
    Selected.clear();
    force_reload_model();
}

void
msglist_model::fav_all_selected(int f)
{
    foreach (const QString &value, Selected)
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
    force_reload_model();
}

void
msglist_model::tag_all_selected(QByteArray tag)
{
    foreach (const QString &value, Selected)
    {
        QByteArray b = value.toLatin1();
        DWYCO_LIST l;
        if(dwyco_qd_message_to_body(&l, b.constData(), b.length()))
        {
            dwyco_list_release(l);
            continue;
        }
        dwyco_set_msg_tag(b.constData(), tag.constData());
    }
    force_reload_model();
}

void
msglist_model::untag_all_selected(QByteArray tag)
{
    foreach (const QString &value, Selected)
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
    force_reload_model();
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
        if(special_sort)
            sort(0, Qt::DescendingOrder);
    }
}

QString
msglist_model::uid() const
{
    return m_uid;
}

void
msglist_model::setTag(const QString& tag)
{
    if(m_tag != tag)
    {
        Selected.clear();
        msglist_raw *mr = dynamic_cast<msglist_raw *>(sourceModel());
        if(mr)
        {
            mr->setTag(tag);
            m_tag = tag;
            emit tagChanged();
        }
        //invalidateFilter();
    }
}

QString
msglist_model::tag() const
{
    return m_tag;
}

void
msglist_model::reload_model()
{
    msglist_raw *mr = dynamic_cast<msglist_raw *>(sourceModel());
    if(mr)
    {
        mr->reload_model();
    }
    if(special_sort)
        sort(0, Qt::DescendingOrder);
}

void
msglist_model::force_reload_model()
{
    msglist_raw *mr = dynamic_cast<msglist_raw *>(sourceModel());
    if(mr)
    {
        mr->reload_model(1);
    }
    if(special_sort)
        sort(0, Qt::DescendingOrder);
}

void
msglist_model::set_filter(int sent, int recv, int last_n, int only_favs)
{
    filter_show_recv = recv;
    filter_show_sent = sent;
    filter_last_n = last_n;
    filter_only_favs = only_favs;
    invalidateFilter();
    Selected.clear();
}

void
msglist_model::set_show_hidden(int show_hidden)
{
    filter_show_hidden = show_hidden;
    invalidateFilter();
    Selected.clear();
}

void
msglist_model::set_sort(bool s)
{
    special_sort = s;
    if(s)
    {
        sort(0, Qt::DescendingOrder);
    }
}

int
msglist_model::find_first_unseen()
{
    int n = rowCount();
    for(int i = 0; i < n; ++i)
    {
        QModelIndex mi = index(i, 0);
        if(data(mi, IS_UNSEEN).toInt() == 1)
        {
            return i;
        }
    }
    return -1;
}

bool
msglist_model::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    //return true;
    QAbstractItemModel *alm = sourceModel();

#if 0
    // note: this won't work as long as the model contains remote messages,
    // which don't have a hash associated with them yet.
    QByteArray hl = alm->data(alm->index(source_row, 0), ASSOC_HASH).toByteArray();
    if(hl.length() == 0)
        return false;
#endif
    QModelIndex mi = alm->index(source_row, 0);
    QVariant is_unfetched = alm->data(mi, IS_UNFETCHED);
    if(is_unfetched.toBool())
        return true;
    QVariant is_file = alm->data(mi, IS_FILE);
    if(is_file.toInt() == 1)
        return true;
    QVariant is_active = alm->data(mi, IS_ACTIVE);
    if(is_active.toBool())
        return true;
    QVariant is_qd = alm->data(mi, IS_QD);
    if(is_qd.toInt() == 1)
        return true;

    QVariant fetch_state = alm->data(mi, FETCH_STATE);
    if(fetch_state.toString() == "manual")
        return true;
    return false;


    QVariant is_sent = alm->data(alm->index(source_row, 0), SENT);
    if(filter_show_sent == 0 && is_sent.toInt() == 1)
        return false;
    if(filter_show_recv == 0 && is_sent.toInt() == 0)
        return false;
    if(filter_only_favs)
    {
        QVariant is_fav = alm->data(alm->index(source_row, 0), IS_FAVORITE);
        if(is_fav.toInt() == 0)
            return false;
    }
    if(filter_show_hidden == 0)
    {
        QVariant mid = alm->data(alm->index(source_row, 0), MID);
        int hidden = dwyco_mid_has_tag(mid.toByteArray().constData(), "_hid");
        if(hidden)
            return false;
    }

    return true;
}

#if 0
bool
msglist_model::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    msglist_raw *m = dynamic_cast<msglist_raw *>(sourceModel());

    QByteArray hl = m->data(source_left, ASSOC_HASH).toByteArray();
    QByteArray hr = m->data(source_right, ASSOC_HASH).toByteArray();

    if(hl.length() == 0 && hr.length() == 0)
        return false;
    if(hl.length() == 0)
        return true;
    if(hr.length() == 0)
        return false;

    long lcl = m->hash_to_effective_lc(hl);
    long lcr = m->hash_to_effective_lc(hr);
    if(lcl == 0 || lcr == 0)
    {
        // hack, no review yet, just use the logical clock for sorting
        lcl = m->data(source_left, LOGICAL_CLOCK).toLongLong();
        lcr = m->data(source_right, LOGICAL_CLOCK).toLongLong();
    }
    if(lcl < lcr)
        return true;
    return false;
}
#endif

msglist_raw::msglist_raw(QObject *p)
    : QAbstractListModel(p)
{
    msg_idx = 0;
    qd_msgs = 0;
    inbox_msgs = 0;
    count_inbox_msgs = 0;
    count_msg_idx = 0;
    count_qd_msgs = 0;
#ifdef DWYCO_MODEL_TEST
    new QAbstractItemModelTester(this);
#endif
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


int
msglist_raw::check_inbox_model()
{
    QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());

    // optimization, to avoid resetting the model in common cases
    DWYCO_UNFETCHED_MSG_LIST new_im;
    if(dwyco_get_unfetched_messages(&new_im, buid.constData(), buid.length()))
    {
        dwyco_list qnew_im(new_im);

        // see if the common case of a new record being right at the end
        // or if a message fetch has finished and toggled the direct attribute

        if(qnew_im.rows() == count_inbox_msgs)
        {
            dwyco_list q_inbox_msgs(inbox_msgs);
            for(int i = 0; i < count_inbox_msgs; ++i)
            {
                QByteArray mid = qnew_im.get<QByteArray>(i, DWYCO_QMS_ID);
                if(mid != q_inbox_msgs.get<QByteArray>(i, DWYCO_QMS_ID))
                {
                    qnew_im.release();
                    return 0;
                }
            }
            qnew_im.release();
            return 1;
        }
        else if(qnew_im.rows() == 1 && count_inbox_msgs == 0)
        {
            beginInsertRows(QModelIndex(), 0, 0);
            if(inbox_msgs)
                dwyco_list_release(inbox_msgs);

            inbox_msgs = qnew_im;
            count_inbox_msgs = qnew_im.rows();
            endInsertRows();
            return 1;
        }
        else if(qnew_im.rows() == 0 && count_inbox_msgs == 1)
        {
            beginRemoveRows(QModelIndex(), 0, 0);
            if(inbox_msgs)
                dwyco_list_release(inbox_msgs);

            inbox_msgs = qnew_im;
            count_inbox_msgs = qnew_im.rows();
            endRemoveRows();
            return 1;
        }
        qnew_im.release();
    }
    return 0;
}

void
msglist_raw::reload_inbox_model()
{
    QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());

    if(check_inbox_model())
        return;

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

    dwyco_get_unfetched_messages(&inbox_msgs, buid.constData(), buid.length());

    if(inbox_msgs)
        dwyco_list_numelems(inbox_msgs, &count_inbox_msgs, 0);

    endResetModel();

}

// return 0 if the q-d msg list has changed
// return 1 if it is the same
int
msglist_raw::check_qd_msgs()
{
    QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
    DWYCO_QD_MSG_LIST qml;
    dwyco_get_qd_messages(&qml, buid.constData(), buid.length());

    simple_scoped qqml(qml);
    if(qqml.rows() != count_qd_msgs)
        return 0;
    dwyco_list oqml(qd_msgs);
    for(int i = 0; i < count_qd_msgs; ++i)
    {
        if(qqml.get<QByteArray>(i, DWYCO_QD_MSG_PERS_ID) != oqml.get<QByteArray>(i, DWYCO_QD_MSG_PERS_ID))
            return 0;
    }
    return 1;
}

void
msglist_raw::reload_model(int force)
{
    QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
    if(!force && msg_idx && m_tag.length() == 0 && check_inbox_model() && check_qd_msgs())
    {
        // inbox might have been update, qd msgs are the same
        // check if there are some new messages in the index for this
        // uid.
        dwyco_list qm(msg_idx);
        if(qm.rows() > 0)
        {
            long curlc = qm.get_long(DWYCO_MSG_IDX_LOGICAL_CLOCK);
            DWYCO_MSG_IDX nmi;
            dwyco_get_new_message_index(&nmi, buid.constData(), buid.length(), curlc);
            simple_scoped qnmi(nmi);
            DWYCO_MSG_IDX cmi;
            dwyco_get_message_index(&cmi, buid.constData(), buid.length());
            dwyco_list qcmi(cmi);
            int new_rows = qnmi.rows();
            if(qcmi.rows() == qm.rows() + qnmi.rows())
            {
                if(new_rows > 0)
                    beginInsertRows(QModelIndex(), count_inbox_msgs + count_qd_msgs,
                                count_inbox_msgs + count_qd_msgs + qnmi.rows() - 1);
                qm.release();
                msg_idx = qcmi;
                count_msg_idx = qcmi.rows();
                if(new_rows > 0)
                    endInsertRows();
                return;
            }
            qcmi.release();
        }


    }

    // note: i discovered that an initial empty model would
    // react to a "resetmodel" by loading the entire model
    // and creating delegates for all the elements in the model.
    // this seems like a qt bug... if you use "insertrows" on the empty
    // model, it does more what you would imagine: creates delegates just
    // for what is needed, rather than the entire model.
    int end_reset = 0;
    if(msg_idx || qd_msgs || inbox_msgs)
    {
        beginResetModel();
        end_reset = 1;
    }

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
    if(buid.length() != 10 && m_tag.length() == 0)
    {
        if(end_reset)
               endResetModel();
        return;
    }

    // note: setting the tag overrides the uid
    if(m_tag.length() > 0)
    {
        dwyco_get_tagged_idx(&msg_idx, m_tag.toLatin1().constData(), 0);
        //dwyco_list_print(msg_idx);
        dwyco_list_numelems(msg_idx, &count_msg_idx, 0);
        if(end_reset)
            endResetModel();
        else
        {
            beginInsertRows(QModelIndex(), 0, count_msg_idx == 0 ? 0 : (count_msg_idx - 1));
            endInsertRows();
        }
        return;
    }
    else if(buid.length() == 10)
    {
        dwyco_get_message_index(&msg_idx, buid.constData(), buid.length());
        //dwyco_list_print(msg_idx);
    }
    dwyco_get_qd_messages(&qd_msgs, buid.constData(), buid.length());
    dwyco_get_unfetched_messages(&inbox_msgs, buid.constData(), buid.length());
    if(msg_idx)
        dwyco_list_numelems(msg_idx, &count_msg_idx, 0);
    if(qd_msgs)
        dwyco_list_numelems(qd_msgs, &count_qd_msgs, 0);
    if(inbox_msgs)
        dwyco_list_numelems(inbox_msgs, &count_inbox_msgs, 0);
    if(end_reset)
        endResetModel();
    else
    {
        int endidx = count_msg_idx + count_qd_msgs + count_inbox_msgs - 1;
        if(endidx >= 0)
        {
        beginInsertRows(QModelIndex(), 0, endidx);
        endInsertRows();
        }
    }
}

void
msglist_raw::setUid(const QString &uid)
{
    if(m_uid != uid)
    {
        m_uid = uid;
        reload_model(1);
    }
}

void
msglist_raw::setTag(const QString& tag)
{
    if(m_tag != tag)
    {
        m_tag = tag;
        reload_model(1);
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
    rn(LOGICAL_CLOCK);
    rn(IS_FAVORITE);
    rn(IS_HIDDEN);
    rn(SELECTED);
    rn(DIRECT);
    rn(FETCH_STATE);
    rn(ATTACHMENT_PERCENT);
    rn(ASSOC_UID);
    rn(SENT_TO_LOCATION);
    rn(REVIEW_RESULTS);
    rn(IS_UNSEEN);
    rn(ASSOC_HASH);
    rn(SENT_TO_LAT);
    rn(SENT_TO_LON);
    rn(IS_UNFETCHED);
#undef rn
    return roles;
}

QVariant
msglist_raw::qd_data ( int r, int role ) const
{
    QByteArray pers_id;

    dwyco_list m(qd_msgs);
    pers_id = m.get<QByteArray>(r, DWYCO_QD_MSG_PERS_ID);

    DWYCO_SAVED_MSG_LIST sm;
    if(!dwyco_qd_message_to_body(&sm, pers_id.constData(), pers_id.length()))
        return QVariant();
    simple_scoped qsm(sm);

    // there is not "mid" per-se for qd messages.
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
        QByteArray txt = qba.get<QByteArray>(0, DWYCO_QM_BODY_NEW_TEXT2);
#ifdef LINUX_EMOJI_CRASH_HACK
        return QString::fromLatin1(txt);
#else
        return QString::fromUtf8(txt);
#endif
    }
    case SENT:
        return 1;
    case PREVIEW_FILENAME:
    {
        int is_file;
        QString local_time;
        QByteArray full_size;
        QByteArray pfn;

        if(!preview_msg_body(qsm, pfn, is_file, full_size, local_time))
            return QVariant("");
        return QString(pfn);

    }
    case DATE_CREATED:
    {
        DWYCO_LIST ba = dwyco_get_body_array(qsm);
        simple_scoped qba(ba);
        QVariant v;
        v.setValue(qba.get_long(DWYCO_QM_BODY_DATE_SECONDS_SINCE_JAN1_1970));
        return v;

    }

    case LOCAL_TIME_CREATED:
    {
        DWYCO_LIST ba = dwyco_get_body_array(qsm);
        simple_scoped qba(ba);
        QDateTime dt;
        dt = QDateTime::fromSecsSinceEpoch(qba.get_long(DWYCO_QM_BODY_DATE_SECONDS_SINCE_JAN1_1970));
        return dt.toString("hh:mm ap");
    }

    case LOGICAL_CLOCK:
        return (qlonglong)qsm.get_long(DWYCO_QM_BODY_LOGICAL_CLOCK);

    case HAS_AUDIO:
    case HAS_SHORT_VIDEO:
    case HAS_VIDEO:
    case IS_FAVORITE:
    case IS_HIDDEN:
    case IS_UNSEEN:
    case IS_FILE:
        return 0;

    case IS_FORWARDED:
    {
        DWYCO_LIST ba = dwyco_get_body_array(qsm);
        simple_scoped qba(ba);
        if(qba.rows() > 1)
            return 1;
        return 0;
    }

    case ATTACHMENT_PERCENT:
        return -1.0;
    case FETCH_STATE:
        return QString("none");
    case IS_UNFETCHED:
        return false;

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
#if 0
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
                dwyco_delete_unfetched_message(mid.constData());
                return 0;
            }

        }
#endif
        // issue a server fetch, client will have to
        // come back in to get it when the fetch is done

        int fetch_id = dwyco_fetch_server_message(mid.constData(), 0, 0, 0, 0);
        if(fetch_id != 0)
        {
            Fetching.append(mid);
            mlm->invalidate();
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
    emit mlm->dataChanged(mi, mi, roles);
    return tmp;
}

static
int
hash_has_tag(QByteArray hash, const char *tag)
{
    DWYCO_LIST tl;
    dwyco_get_tagged_mids(&tl, hash.constData());
    simple_scoped stl(tl);
    for(int i = 0; i < stl.rows(); ++i)
    {
        QByteArray b = stl.get<QByteArray>(i, DWYCO_TAGGED_MIDS_MID);
        if(dwyco_mid_has_tag(b.constData(), tag))
            return 1;
    }
    return 0;
}


long
msglist_raw::hash_to_effective_lc(const QByteArray& hash)
{
    return Hash_to_max_lc.value(QByteArray::fromHex(hash), 0);
}

QVariant
msglist_raw::inbox_data (int r, int role ) const
{
    QByteArray mid;
    const char *out;
    int len_out;
    int type_out;

    int n = count_inbox_msgs;
    dwyco_list_throw m(inbox_msgs);

    // inbox msgs are sorted in the opposite order (newest last)
    r = n - r - 1;
    try {
    mid = m.get<QByteArray>(r, DWYCO_QMS_ID);

    if(role == MID)
        return mid;

    int direct = 0; //dwyco_get_attr_bool(inbox_msgs, r, DWYCO_QMS_IS_DIRECT);

    DWYCO_SAVED_MSG_LIST sm = 0;
#if 0
    if(direct)
    {
        if(!dwyco_unsaved_message_to_body(&sm, mid.constData()))
        {
            return QVariant();
        }
    }
#endif

    //simple_scoped qsm(sm);

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
        QVariant v;
        v.setValue(m.get_long(r, DWYCO_QMS_DS_SECONDS_SINCE_JAN1_1970));
        return v;
    }

    case LOCAL_TIME_CREATED:
    {
        return gen_time_unsaved(inbox_msgs, r);
    }

    case MSG_TEXT:
    {
        auto_fetch(mid);
        return "(fetching)";

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
        if(!direct)
        {
            auto_fetch(mid);

        }
        return QVariant("");
    }
    case HAS_AUDIO:
    case HAS_SHORT_VIDEO:
    case HAS_VIDEO:
    case IS_FAVORITE:
    case IS_HIDDEN:
    case IS_FORWARDED:
    case IS_UNSEEN:
    case IS_FILE:
        return 0;

    case LOGICAL_CLOCK:
        return 0;

    case ATTACHMENT_PERCENT:
        if(!Mid_to_percent.contains(mid))
            return -1.0;
        return (double)Mid_to_percent.value(mid);
    case IS_UNFETCHED:
        return true;
    case Qt::DecorationRole:
        return QVariant("qrc:///new/red32/icons/red-32x32/Upload-32x32.png");

    default:
        return QVariant();
    }
    }
    catch(...)
    {

    }
    return QVariant();

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
    int r1 = count_qd_msgs;

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

    dwyco_list_throw m(msg_idx);

    try {

    if(role == MID)
    {
        QByteArray mid = m.get<QByteArray>(r, DWYCO_MSG_IDX_MID);
        return mid;
    }
    if(role == SELECTED)
    {
        return Selected.contains(data(index, MID).toByteArray());
    }
    if(role == Qt::DisplayRole)
    {
        QDateTime q(QDateTime::fromSecsSinceEpoch(m.get_long(r, DWYCO_MSG_IDX_DATE)));
        return QVariant(q);
    }
    else if(role == DATE_CREATED)
    {
        QVariant v;
        v.setValue(m.get_long(r, DWYCO_MSG_IDX_DATE));
        return v;
    }
    else if(role == SENT)
    {
        if(!m.is_nil(r, DWYCO_MSG_IDX_IS_SENT))
        {
            return QVariant(1);
        }
        return QVariant(0);
    }
    else if( role == Qt::DecorationRole)
    {
        if(m.is_nil(r, DWYCO_MSG_IDX_HAS_ATTACHMENT))
        {
            return QVariant("");
        }
        if(!m.is_nil(r, DWYCO_MSG_IDX_IS_FILE))
        {
            return QVariant("qrc:///new/prefix1/icons/downlaod-16x16.png");
        }
        if(!m.is_nil(r, DWYCO_MSG_IDX_ATT_HAS_VIDEO))
        {
            if(!m.is_nil(r, DWYCO_MSG_IDX_ATT_IS_SHORT_VIDEO))
                return QVariant("qrc:///new/prefix1/icons/media-1-16x16.png");
            else
                return QVariant("qrc:///new/prefix1/icons/media 2-16x16.png");
        }
        if(!m.is_nil(r, DWYCO_MSG_IDX_ATT_HAS_AUDIO))
            return QVariant("qrc:///new/prefix1/icons/music 2-16x16.png");
        return QVariant("qrc:///new/prefix1/icons/stop.png");
    }
    else if(role == MSG_TEXT)
    {
        return get_msg_text(r);
    }
    else if(role == HAS_VIDEO)
    {
        if(m.is_nil(r, DWYCO_MSG_IDX_HAS_ATTACHMENT))
        {
            return 0;
        }
        if(!m.is_nil(r, DWYCO_MSG_IDX_ATT_HAS_VIDEO))
        {
            return 1;
        }
        return 0;

    }
    else if(role == HAS_SHORT_VIDEO)
    {
        if(m.is_nil(r, DWYCO_MSG_IDX_HAS_ATTACHMENT))
        {
            return 0;
        }
        if(!m.is_nil(r, DWYCO_MSG_IDX_ATT_IS_SHORT_VIDEO))
            return 1;
        return 0;
    }
    else if(role == HAS_AUDIO)
    {
        if(m.is_nil(r, DWYCO_MSG_IDX_HAS_ATTACHMENT))
        {
            return 0;
        }
        if(!m.is_nil(r, DWYCO_MSG_IDX_ATT_HAS_AUDIO))
            return 1;
        return 0;
    }
    else if(role == IS_FILE)
    {
        if(m.is_nil(r, DWYCO_MSG_IDX_HAS_ATTACHMENT))
        {
            return 0;
        }
        if(!m.is_nil(r, DWYCO_MSG_IDX_IS_FILE))
        {
            return 1;
        }
        return 0;

    }
    else if (role == IS_FORWARDED)
    {
        if(m.is_nil(r, DWYCO_MSG_IDX_IS_FORWARDED))
            return 0;
        return 1;
    }
    else if(role == LOCAL_TIME_CREATED)
    {
        // hmm, consider putting the local time in the index, as the way it is now
        // we would have to read in the message to get it, which may be
        // too expensive. we know the local time is only really displayed for
        // "extra info", so it isn't used all that much

        auto mid = m.get<QByteArray>(r, DWYCO_MSG_IDX_MID);
        //auto buid = m.get<QByteArray>(r, DWYCO_MSG_IDX_ASSOC_UID);

        //buid = QByteArray::fromHex(buid);
        DWYCO_SAVED_MSG_LIST sm;
        if(dwyco_get_saved_message3(&sm, mid.constData()) != DWYCO_GSM_SUCCESS)
            return "unknown";
        simple_scoped qsm(sm);

        DWYCO_LIST ba = dwyco_get_body_array(qsm);
        simple_scoped qba(ba);
        return gen_time(qba, 0);
    }
    else if(role == IS_FAVORITE)
    {
        auto mid = m.get<QByteArray>(r, DWYCO_MSG_IDX_MID);
        if(dwyco_get_fav_msg(mid.constData()))
            return 1;
        return 0;
    }
    else if(role == IS_HIDDEN)
    {
        auto mid = m.get<QByteArray>(r, DWYCO_MSG_IDX_MID);
        if(dwyco_mid_has_tag(mid.constData(), "_hid"))
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
    else if(role == ASSOC_UID)
    {
        auto huid = m.get<QByteArray>(r, DWYCO_MSG_IDX_ASSOC_UID);
        return huid;
    }
    else if(role == SENT_TO_LOCATION)
    {
        auto mid = m.get<QByteArray>(r, DWYCO_MSG_IDX_MID);

        QByteArray h;
        if(!att_file_hash(mid, h))
            return QByteArray("");
        QByteArray l = Hash_to_loc.value(h).loc;
        return l;
    }
    else if(role == SENT_TO_LAT)
    {
        auto mid = m.get<QByteArray>(r, DWYCO_MSG_IDX_MID);

        QByteArray h;
        if(!att_file_hash(mid, h))
            return QByteArray("");
        QByteArray l = Hash_to_loc.value(h).lat;
        return l;
    }
    else if(role == SENT_TO_LON)
    {
        auto mid = m.get<QByteArray>(r, DWYCO_MSG_IDX_MID);

        QByteArray h;
        if(!att_file_hash(mid, h))
            return QByteArray("");
        QByteArray l = Hash_to_loc.value(h).lon;
        return l;
    }
    else if(role == REVIEW_RESULTS)
    {
        auto mid = m.get<QByteArray>(r, DWYCO_MSG_IDX_MID);

        QByteArray h;
        if(!att_file_hash(mid, h))
            return QByteArray("");
        QByteArray l = Hash_to_review.value(h, "Unknown");
        return l;
    }
    else if(role == IS_UNSEEN)
    {
        auto mid = m.get<QByteArray>(r, DWYCO_MSG_IDX_MID);

        QByteArray h;
        if(!att_file_hash(mid, h))
            return 0;
        h = h.toHex();
        if(hash_has_tag(h, "unviewed"))
            return 1;
        return 0;
    }
    else if(role == ASSOC_HASH)
    {
        auto mid = m.get<QByteArray>(r, DWYCO_MSG_IDX_MID);

        QByteArray h;
        if(!att_file_hash(mid, h))
            return QByteArray("");
        return QString(h.toHex());
    }
    else if(role == LOGICAL_CLOCK)
    {
        return (qlonglong)m.get_long(r, DWYCO_MSG_IDX_LOGICAL_CLOCK);
    }
    else if(role == IS_UNFETCHED)
    {
        return false;
    }
    }
    catch(...)
    {

    }

    return QVariant();

}


QString
msglist_raw::get_msg_text(int row) const
{
    dwyco_list_throw m(msg_idx);
    QByteArray mid;
    try
    {
        mid = m.get<QByteArray>(row, DWYCO_MSG_IDX_MID);
    }
    catch(...)
    {
        return "";
    }

    DWYCO_SAVED_MSG_LIST sm;

    int disp = dwyco_get_saved_message3(&sm, mid.constData());
    switch(disp)
    {
    case DWYCO_GSM_TRANSIENT_FAIL:
        return "(unknown)";
    case DWYCO_GSM_PULL_IN_PROGRESS:
        return "(fetching)";
    case DWYCO_GSM_TRANSIENT_FAIL_AVAILABLE:
        return "(fetching when available)";
    case DWYCO_GSM_ERROR:
    default:
        return "(failed)";
    case DWYCO_GSM_SUCCESS:
        break;
    }
    simple_scoped qsm(sm);

    DWYCO_LIST ba = dwyco_get_body_array(qsm);
    simple_scoped qba(ba);

    if(qba.rows() > 1)
    {
        // forwarded message, for now just return the
        // yucky dll formatted message
        DWYCO_LIST bt = dwyco_get_body_text(qba);
        if(!bt)
            return "";
        simple_scoped qbt(bt);
        auto ftxt = qbt.get<QByteArray>(0);
#ifdef LINUX_EMOJI_CRASH_HACK
        return QString::fromLatin1(get_extended(ftxt));
#else
        return QString::fromUtf8(get_extended(ftxt));
#endif
    }


    auto txt = qba.get<QByteArray>(0, DWYCO_QM_BODY_NEW_TEXT2);
#ifdef LINUX_EMOJI_CRASH_HACK
    return QString::fromLatin1(get_extended(txt));
#else
    return QString::fromUtf8(get_extended(txt));
#endif
}

QString
msglist_raw::preview_filename(int row) const
{
    QByteArray pfn;
    //QByteArray buid;
    QByteArray mid;
    int is_file;
    QString local_time;
    dwyco_list_throw m(msg_idx);
    try {
        mid = m.get<QByteArray>(row, DWYCO_MSG_IDX_MID);
    }
    catch(...)
    {
        return "";
    }

    QByteArray full_size;

    if(!preview_saved_msg(mid, pfn, is_file, full_size, local_time))
        return "";
    return pfn;
}
