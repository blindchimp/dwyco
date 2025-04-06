
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
#include "msgpv.h"
#include "pfx.h"
#include "dwycolist2.h"
#include "dwyco_new_msg.h"
#include "dwyco_top.h"

#if defined(LINUX) && !(defined(ANDROID) || defined(MACOSX))
#define LINUX_EMOJI_CRASH_HACK
#endif
[[noreturn]] void cdcxpanic(const char *);

class DwycoCore;
extern DwycoCore *TheDwycoCore;

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
msglist_raw *mlm;

static QList<QByteArray> Fetching;
static QSet<QByteArray> Dont_refetch;
static QMap<QByteArray, int> Mid_to_percent;
// messages are automatically fetched, unless it fails.
// after that, the fetch can be initiated explicitly
static QSet<QByteArray> Manual_fetch;
static QSet<QByteArray> Failed_fetch;

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

void
msglist_raw::msg_recv_progress(QString mid, QString huid, QString msg, int percent_done)
{
    QByteArray bmid = mid.toLatin1();
    Mid_to_percent.insert(bmid, percent_done);
    int midi = mid_to_index(bmid);
    if(midi == -1)
        return;
    QModelIndex mi = index(midi, 0);
    emit dataChanged(mi, mi, QVector<int>(1, ATTACHMENT_PERCENT));
}

void
msglist_raw::invalidate_mid(const QByteArray& mid, const QString& huid)
{
    //if(huid != uid())
    //    return;
    int midi = mid_to_index(mid);
    if(midi == -1)
        return;

    QModelIndex mi = index(midi, 0);
    emit dataChanged(mi, mi, QVector<int>());
    emit invalidate_item(mid);
    // probably needs an emit or something invalidateFilter();

}

void
msglist_raw::msg_recv_status(int cmd, const QString &smid, const QString &shuid)
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
        // XXX fix this, if we can't decrypt it now, it might be
        // because it went to a group but the sender was old software
        // and didn't encrypt with the group key. "delete" means
        // remove the message for the entire group, which we don't really
        // want to do.
    case DWYCO_SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED_DECRYPT_FAILED:
        // if this msg is ratholed, it can never be fetched, so just delete it.
    {
        Dont_refetch.insert(mid);
        if(i >= 0)
            Fetching.removeAt(i);
        Mid_to_percent.remove(mid);
        Failed_fetch.insert(mid);
        Manual_fetch.insert(mid);

        reload_inbox_model();
        break;
    }

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

    {
        reload_inbox_model();
        add_unviewed(buid, mid);
        emit TheDwycoCore->new_msg(shuid, "", smid);
        emit TheDwycoCore->decorate_user(shuid);

        dwyco_unset_msg_tag(mid.constData(), "_inbox");
        if(m_uid == shuid)
            mid_tag_changed(smid);

    }
    // FALLTHRU
        [[clang::fallthrough]];
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
    emit dataChanged(mi, mi, roles);
}

int
msglist_raw::mid_to_index(QByteArray bmid)
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
msglist_raw::mid_tag_changed(QString mid)
{
    int midi = mid_to_index(mid.toLatin1());
    if(midi == -1)
        return;
    QModelIndex mi = index(midi, 0);

    QVector<int> roles;
    roles.append(IS_HIDDEN);
    roles.append(IS_FAVORITE);
    emit dataChanged(mi, mi, roles);
    emit invalidate_item(mid.toLatin1());
}

void
msglist_raw::trash_all_selected(const QSet<QByteArray>& selected)
{
    //QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
    dwyco_start_bulk_update();
    foreach (const QString &value, selected)
    {
        QByteArray mid = value.toLatin1();
        DWYCO_LIST l;
        // note: "trashing" something we might end up downloading, we just
        // put the tag in, the download will probably happen at some point
        // from somewhere, which is what we want. deleting it altogether is
        // not what we want.
#if 0
        if(dwyco_get_unfetched_message(&l, mid.constData()))
        {
            dwyco_list_release(l);
            dwyco_delete_unfetched_message(mid.constData());
        }
        //else
#endif
        // trashing a q-d message just means stopping it and throwing it away
        if(dwyco_qd_message_to_body(&l, mid.constData(), mid.length()))
        {
            dwyco_list_release(l);
            dwyco_kill_message(mid.constData(), mid.length());
        }
        else
        {
            // race condition here... if the fav tag isn't here yet,
            // we can "trash" it, which will trash it everywhere
            // despite being fav. we probably need to add something
            // about checking for fav in the trash check and automatically
            // untrash them if they are favs
            if(!dwyco_get_fav_msg(mid.constData()))
            {
                dwyco_set_msg_tag(mid.constData(), "_trash");
            }
        }

    }
    dwyco_end_bulk_update();
    reload_model(1);
}

void
msglist_raw::obliterate_all_selected(const QSet<QByteArray>& selected)
{
    QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());
    dwyco_start_bulk_update();
    foreach (const QString &value, selected)
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
            {
                dwyco_delete_saved_message(buid.constData(), buid.length(), mid.constData());
            }
        }

    }
    dwyco_end_bulk_update();
    reload_model(1);
}

void
msglist_raw::fav_all_selected(const QSet<QByteArray>& selected, int f)
{
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
    reload_model(1);
}

void
msglist_raw::tag_all_selected(const QSet<QByteArray>& selected, const QByteArray& tag)
{
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
    reload_model(1);
}

void
msglist_raw::untag_all_selected(const QSet<QByteArray> &selected, const QByteArray& tag)
{
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
    reload_model(1);
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
    QObject::connect(this, &msglist_raw::uidChanged, this, &msglist_raw::reload_model2);
    QObject::connect(this, &msglist_raw::tagChanged, this, &msglist_raw::reload_model2);
    mlm = this;
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
msglist_raw::reload_model2(const QString&)
{
	reload_model(1);
}

void
msglist_raw::reload_model(int force)
{
    QByteArray buid = QByteArray::fromHex(m_uid.toLatin1());

    // note: based on dogfooding, i've seen cases where it looks like
    // duplicate mid's might be getting put in the model. i suspect this is
    // from weird cases where a message is received partially from one path
    // (ie, direct, or via sync) and then also exists at the server.
    // we don't make any hard assumptions about the 3 lists being
    // disjoint in the mid's, but it looks weird to a user when it happens.
    //
    // possibly we could enforce the disjoint property by having some
    // priority for messages based on where they are available. also might not be worth it, since
    // it can usually be resolved automatically when the network is working
    // right.
    //
    // remove for testing, when a message is pulled, we need to
    // "invalidate" it more gracefully. for now, just reload the model
#if 1
    if(!force && msg_idx && m_tag.length() == 0 && check_inbox_model() && check_qd_msgs())
    {
        // inbox might have been update, qd msgs are the same
        // check if there are some new messages in the index for this
        // uid.
        dwyco_list qm(msg_idx);
        if(qm.rows() > 0)
        {
            long curlc = qm.get_long(0, DWYCO_MSG_IDX_LOGICAL_CLOCK);
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
        int order_by_tag_time = 0;
        if(m_tag == "_trash")
        {
            // this is a weird corner case where we might have things
            // waiting on the server that have been trashed, and not
            // fetched yet.
            dwyco_get_unfetched_messages(&inbox_msgs, 0, 0);
            // note: the model will include all uid's, but the filterAcceptsRow will
            // only show the trashed ones
            if(inbox_msgs)
                dwyco_list_numelems(inbox_msgs, &count_inbox_msgs, 0);

            order_by_tag_time = 1;
        }
        dwyco_get_tagged_idx(&msg_idx, m_tag.toLatin1().constData(), order_by_tag_time);
        //dwyco_list_print(msg_idx);
        dwyco_list_numelems(msg_idx, &count_msg_idx, 0);
        if(end_reset)
            endResetModel();
        else
        {
            beginInsertRows(QModelIndex(), 0, count_msg_idx + count_inbox_msgs == 0 ? 0 : (count_msg_idx + count_inbox_msgs - 1));
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
    rn(HAS_ATTACHMENT);
    rn(IS_QD);
    rn(IS_ACTIVE);
    rn(IS_FORWARDED);
    rn(IS_NO_FORWARD);
    rn(DATE_CREATED);
    rn(LOCAL_TIME_CREATED);
    rn(IS_FAVORITE);
    rn(IS_HIDDEN);
    rn(SELECTED);
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
        cdcxpanic("hhuh2");
        //return Selected.contains(pers_id);
    }
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
    case HAS_ATTACHMENT:
    {
        return !qsm.is_nil(DWYCO_QM_BODY_ATTACHMENT);
    }
    case IS_FILE:
    {
        if(!qsm.is_nil(DWYCO_QM_BODY_ATTACHMENT) && !qsm.is_nil(DWYCO_QM_BODY_FILE_ATTACHMENT))
            return 1;
        return 0;
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

    case IS_NO_FORWARD:
    {
        // note: determining if a message has limited forwarding can be
        // expensive, since we check the integrity of the message. for the
        // purposes of this model, we just say "no, it's in the message q, so don't try it."
        // since this is used for display purposes, it's fine since forwarding a
        // q-d message seems a little weird anyway.
        return 1;
    }
    case ASSOC_UID:
    {
        // note: this message is in the outgoing q, so it is "from" you. but we'll get whatever
        // is stored in the message, in case some day we do something weird with it.
        DWYCO_LIST ba = dwyco_get_body_array(qsm);
        simple_scoped qba(ba);
        QVariant v;
        v.setValue(qba.get<QByteArray>(DWYCO_QM_BODY_FROM).toHex());

        return v;
    }
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
    if(!(Fetching.contains(mid) || Manual_fetch.contains(mid) || Failed_fetch.contains(mid)))
    {
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
    roles.append(msglist_raw::IS_ACTIVE);
    roles.append(msglist_raw::FETCH_STATE);
    roles.append(msglist_raw::ATTACHMENT_PERCENT);
    roles.append(msglist_raw::PREVIEW_FILENAME);
    emit mlm->dataChanged(mi, mi, roles);
    return tmp;
}

QVariant
msglist_raw::inbox_data (int r, int role ) const
{
    QByteArray mid;

    int n = count_inbox_msgs;
    dwyco_list_throw m(inbox_msgs);

    // inbox msgs are sorted in the opposite order (newest last)
    r = n - r - 1;
    try {
    mid = m.get<QByteArray>(r, DWYCO_QMS_ID);

    if(role == MID)
        return mid;

    switch(role)
    {
    case MID:
        return mid;
    case SELECTED:
    {
        cdcxpanic("jkj");
        //return Selected.contains(mid);
    }
    case IS_QD:
        return QVariant(0);
    case IS_ACTIVE:
    {
        return Fetching.contains(mid);
    }

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
        if(Failed_fetch.contains(mid))
            return QString("failed");
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
    case IS_HIDDEN:
    case IS_FORWARDED:
    case IS_UNSEEN:
    case IS_FILE:
        return 0;

    case HAS_ATTACHMENT:
        return false;

    case ATTACHMENT_PERCENT:
        if(!Mid_to_percent.contains(mid))
            return -1.0;
        return (double)Mid_to_percent.value(mid);
    case IS_UNFETCHED:
        return true;
    case Qt::DecorationRole:
        return QVariant("qrc:///new/red32/icons/red-32x32/Upload-32x32.png");

    case IS_FILE:
    case IS_NO_FORWARD:
        // note: in this case, we don't know until the msg is downloaded
        // and it would be kinda nice if we received a signal when that
        // happened. in reality, i think the entire model is probably reloaded
        // when a message download has completed... have to check
        return QVariant();

    case ASSOC_UID:
    {
        auto buid = m.get<QByteArray>(r, DWYCO_QMS_FROM);
        return buid.toHex();
    }

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
        cdcxpanic("huh");
        //return Selected.contains(data(index, MID).toByteArray());
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
    else if(role == HAS_ATTACHMENT)
    {
        if(m.is_nil(r, DWYCO_MSG_IDX_HAS_ATTACHMENT))
        {
            return QVariant(false);
        }
        return QVariant(true);
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

    QByteArray txt;
    if(qba.is_nil(0, DWYCO_QM_BODY_NEW_TEXT2))
    {

        txt = qba.get<QByteArray>(0, DWYCO_QM_BODY_TEXT2_obsolete);
    }
    else
        txt = qba.get<QByteArray>(0, DWYCO_QM_BODY_NEW_TEXT2);
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
