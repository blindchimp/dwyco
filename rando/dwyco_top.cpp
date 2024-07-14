
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QObject>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QtQml>
#include <QUrl>
#include <QUrlQuery>
#include <QSslSocket>
#include <QGuiApplication>
#include <QImage>
#ifdef ANDROID
//#include <QtAndroid>
#endif
#include "androidperms.h"
#include "dlli.h"
#include <stdlib.h>
#include "dwyco_new_msg.h"
#include "getinfo.h"
#include "msglistmodel.h"
#include "pfx.h"
#include "ssmap.h"
//#include "dwycoimageprovider.h"
//#include "dwycoprofilepreviewprovider.h"
//#include "dwycovideopreviewprovider.h"
//#include "ignoremodel.h"
//#include "chatlistmodel.h"
#include "convmodel.h"
//#include "simple_user_list.h"
//#include "ctlist.h"
#include "QQmlVarPropertyHelpers.h"
#include "QQmlVariantListModel.h"
#ifdef ANDROID
#include "notificationclient.h"
//#include "audi_qt.h"

#endif
//#include "profpv.h"
////#if defined(LINUX) && !defined(MAC_CLIENT) && !defined(ANDROID)
////#include "v4lcapexp.h"
//////#include "esdaudin.h"
////#include "audi_qt.h"
////#include "aextsdl.h"
////#include "audo_qt.h"
////#endif
//#ifdef DWYCO_IOS
//#include "audi_qt.h"
//#endif
//#include "audo_qt.h"
//#include "dvp.h"
//#include "callsm.h"
#include "resizeimage.h"
#ifdef _Windows
#include <time.h>
#endif
#include "dwycolist2.h"
#include "dwyco_top.h"
//#include "callsm_objs.h"
//#if defined(DWYCO_FORCE_DESKTOP_VGQT) || defined(ANDROID) || defined(DWYCO_IOS)
//#include "vgqt.h"
//#endif
#include "qloc.h"
#include "geospray.h"


#if defined(MACOSX) && !defined(DWYCO_IOS) && defined(DWYCO_QT5)
#include <QtMacExtras>
#endif

void init_mac_drivers();

namespace jhead {
int do_jhead(const char *);
}

class DwycoCore;
DwycoCore *TheDwycoCore;
static QQmlContext *TheRootCtx;
QByteArray DwycoCore::My_uid;
static QByteArray TheMan;
static int AvoidSSL = 0;
typedef QHash<QByteArray, QByteArray> UID_ATTR_MAP;
typedef QHash<QByteArray, QByteArray>::iterator UID_ATTR_MAP_ITER;
static UID_ATTR_MAP Uid_attrs;
static int Init_ok;

//static ChatSortFilterModel *Chat_sort_proxy;
static ConvSortFilterModel *Conv_sort_proxy;
//static IgnoreSortFilterModel *Ignore_sort_proxy;
static QTcpServer *BGLockSock;
//static DwycoImageProvider *Dwyco_video_provider;
//static DwycoVideoPreviewProvider *Dwyco_video_preview_provider;
static QQmlVariantListModel *CamListModel;
static int BGLockPort;
int auto_fetch(QByteArray mid);
int retry_auto_fetch(QByteArray mid);
void clear_manual_gate();
extern int HasAudioInput;
extern int HasAudioOutput;
extern int HasCamera;
extern int HasCamHardware;

int DwycoCore::Android_migrate;

QMultiMap<QByteArray, QLoc> Hash_to_loc;
QMap<QByteArray,QByteArray> Hash_to_review;
QMap<QByteArray, long> Hash_to_max_lc;

// this just downloads the current servers2 file from
// a website using whatever dns is setup for dwyco.com.
// normally we avoid using dns. this is useful if we
// really need to move someplace in case there is a large
// outage or something.
static
void
install_emergency_servers2(QNetworkReply *reply)
{
    //volatile auto d = reply->error();
    // NOTENOTE! if you don't have SSL installed properly (happens on
    // windows sometimes) you will get an "unknown error" IF THE WEBSERVER
    // USES automatic https redirection. the solution is to make the
    // webserver use exactly what we asked for: plain HTTP. for caddy,
    // this involved updating the Caddyfile to explicitly match
    // the URL in the request and not to the normal https redirection.
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray tfile = add_pfx(Tmp_pfx, "servers2.tmp");
        QFile file(tfile);
        QByteArray em = reply->readAll();
        // note: we can have an "error" if file not found or
        // redirect isn't handled properly, and it will still count
        // as "NoError", it just gives you no bytes in the download.
        if (em.length() > 0 && file.open(QIODevice::WriteOnly))
        {
            file.write(em);
            file.close();
        }
        // NOTE: this might be something we should gate with some user
        // input since the program may just shut down.
        dwyco_update_server_list(em.constData(), em.length());
    }
    reply->manager()->deleteLater();
    reply->deleteLater();
}

static
void
setup_emergency_servers()
{
    auto manager = new QNetworkAccessManager;
    QObject::connect(manager, &QNetworkAccessManager::finished, install_emergency_servers2);
    auto r = QNetworkRequest(QUrl("http://www.dwyco.com/downloads/servers2.eme"));
    // not sure, maybe i don't need this in qt6 any more?
    //r.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkReply *reply = manager->get(r);
}

void
hack_unread_count()
{
    if(TheDwycoCore)
        TheDwycoCore->update_unread_count(total_unviewed_msgs_count());
}

void
update_unseen_from_db()
{
    if(!TheDwycoCore)
        return;
    DWYCO_LIST tl;
    if(!dwyco_get_tagged_mids(&tl, "unviewed"))
        return;
    simple_scoped stl(tl);

    bool has_urando = false;
    bool has_ugeo = false;
    for(int i = 0; i < stl.rows(); ++i)
    {
        QByteArray mid = stl.get<QByteArray>(i, DWYCO_TAGGED_MIDS_MID);
        if(dwyco_mid_has_tag(mid.constData(), "_json"))
            has_ugeo = true;
        else
            has_urando = true;
        if(has_ugeo && has_urando)
            break;
    }
    // if there are messages on the server from the reviewer, it is
    // almost certainly bad news
    if(uid_has_unviewed_msgs(TheMan))
        has_ugeo = true;

    // if there are server messages from the bot (NOT the reviewer)
    // it could be either a rando or a geo location msg
    // the msg summary doesn't tell us directly, we have to wait
    // until the message is downloaded, so just tell the user it
    // could be either. note, we could kluge and check the length, or
    // actually put the attachment info in the summary, but that is
    // too much for now.
    if(total_unviewed_msgs_count() > uid_unviewed_msgs_count(TheMan))
    {
        // the geo things from the bot are small and will be downloaded
        // quickly, no need to indicate this here, it will happen above
        // when the download is complete.
        //has_ugeo = true;
        has_urando = true;
    }

    TheDwycoCore->update_has_unseen_rando(has_urando);
    TheDwycoCore->update_has_unseen_geo(has_ugeo);
}

void
DwycoCore::clear_unseen_rando()
{
    clear_unviewed_except_for_uid(TheMan);
    TheDwycoCore->update_has_unseen_rando(false);
}

void
reload_conv_list()
{
    Conv_sort_proxy->setDynamicSortFilter(false);
    int total = 0;
    dwyco_load_users2(0, /*TheDwycoCore->get_use_archived() ? 0 : 1,*/ &total);
    TheDwycoCore->update_total_users(total);
    TheConvListModel->load_users_to_model();
    Conv_sort_proxy->setDynamicSortFilter(true);
}

static void
reload_ignore_list()
{
#if 0
    Conv_sort_proxy->setDynamicSortFilter(false);
    Ignore_sort_proxy->setDynamicSortFilter(false);
    TheIgnoreListModel->load_users_to_model();
    Ignore_sort_proxy->setDynamicSortFilter(true);
    Conv_sort_proxy->setDynamicSortFilter(true);
#endif
}

static
void
takeover_from_background(int port)
{
#ifdef ANDROID
    int c = dwyco_request_singleton_lock("dwyco", port);
    if(c < 0)
        exit(0);
#endif

    if(!BGLockSock)
        BGLockSock = new QTcpServer;

#ifdef ANDROID
    BGLockSock->setSocketDescriptor(c);
    return;
#endif

    while(!BGLockSock->isListening() &&
            !BGLockSock->listen(QHostAddress("127.0.0.1"), port))
    {
        QTcpSocket conn;
        conn.connectToHost(QHostAddress("127.0.0.1"), port);
        conn.waitForConnected(1000);
        conn.close();

//        if(conn.waitForConnected(1000))
//        {
//            if(conn.state() != QAbstractSocket::ConnectedState)
//                continue;
//            //conn.waitForReadyRead();
//        }
    }
}

static
QImage
transpose(const QImage& src)
{
    QImage dst(src.height(), src.width(), QImage::Format_RGB32);
    for(int y = 0; y < src.height(); ++y)
    {
        for(int x = 0; x < src.width(); ++x)
        {
            dst.setPixel(y, x, src.pixel(x, y));
        }
    }
    return dst;
}

static
int
jhead_rotate(const QString& filename, int rot, bool mirror_y = false)
{
    if(rot == 0)
        return 1;

    QImage s(filename);
    if(s.isNull())
        return 0;
    switch(rot)
    {
    case 3: //180
        s = s.mirrored(true, true);
        break;
    case 6: //90
        s = transpose(s);
        s = s.mirrored(true, false);
        if(mirror_y)
            s = s.mirrored(true, false);
        break;
    case 8: //270
        // i think this is broken, have to check it
        s = s.mirrored(true, false);
        s = transpose(s);

        if(mirror_y)
            s = s.mirrored(true, false);
        break;
    }

    s.save(filename);

    return 1;
}

// note: tbe main reason we need this is because
// on iOS images come back without rotation meta
// data, so we kinda guess how to rotate them
// so this strips out whatever exif stuff is in there,
// and then rotates it according to what the caller wants
int
DwycoCore::rotate_in_place(QString fn, int rot, int mirror_y)
{
    {
        // note: iOS makes a copy and it must inherit the read-only perms
        // of the original, so we try to update the permissions on the copy
        // so we can update it in place
        QFile cpy(fn);
        cpy.setPermissions(cpy.permissions()|QFileDevice::WriteOwner|QFileDevice::WriteUser|
                           QFileDevice::ReadOwner|QFileDevice::ReadUser);
    }
    int ret = jhead::do_jhead(fn.toLatin1().constData());
    if(ret & 1)
        return 0;

    ret = jhead_rotate(fn, rot, mirror_y ? true : false);
    return ret;

}

static
int
copy_and_tweak_jpg(const QString& fn, QByteArray& dest_out)
{
    // stopgap... if it is a jpg file, we'll make a copy and
    // strip out exif.
    dest_out = "";
    QImage tst(fn, "JPG");
    if(!tst.isNull())
    {
        // copy, then nuke the exif
        QFile f(fn);

        char *rs;
        dwyco_random_string2(&rs, 4);
        QByteArray rsb(rs, 4);
        dwyco_free_array(rs);
        rsb = rsb.toHex();

        dest_out += rsb;
        dest_out += ".jpg";
        dest_out = add_pfx(Tmp_pfx, dest_out);
        if(!f.copy(dest_out))
        {
            return 0;
        }
        {
            // note: iOS makes a copy and it must inherit the read-only perms
            // of the original, so we try to update the permissions on the copy
            // so we can update it in place
            QFile cpy(dest_out);
            cpy.setPermissions(cpy.permissions()|QFileDevice::WriteOwner|QFileDevice::WriteUser|
                               QFileDevice::ReadOwner|QFileDevice::ReadUser);
        }
        int rot = jhead::do_jhead(dest_out.constData());

        if((rot & 1) == 0)
        {
            jhead_rotate(dest_out, rot >> 1);
        }
        else
        {
            QFile::remove(dest_out);
            return 0;
        }
        return 1;
    }
    return 0;
}

[[noreturn]]
void
cdcxpanic(const char *)
{
    ::abort();
}

#if 0
void
uid_attrs_clear()
{
    Uid_attrs.clear();
}

void
uid_attrs_add(const QByteArray& uid, const QByteArray& attr)
{
    Uid_attrs.insertMulti(uid, attr);
}

void
uid_attrs_del(const QByteArray& uid, const QByteArray& attr)
{
    UID_ATTR_MAP_ITER i = Uid_attrs.find(uid);
    while (i != Uid_attrs.end() && i.key() == uid)
    {
        if(i.value() == attr)
        {
            Uid_attrs.erase(i);
            break;
        }
        ++i;
    }
}

int
uid_attrs_has_attr(const QByteArray& uid, const QByteArray& attr)
{
    UID_ATTR_MAP_ITER i = Uid_attrs.find(uid);
    while (i != Uid_attrs.end() && i.key() == uid)
    {
        if(i.value() == attr)
        {
            return 1;
        }
        ++i;
    }
    return 0;
}

void
uid_attrs_del_uid(const QByteArray& uid)
{
    Uid_attrs.remove(uid);
}

int
uid_in_lobby(const QByteArray& uid)
{
    return Uid_attrs.contains(uid);
}

int
uid_active(const QByteArray& uid)
{
    if(!uid_in_lobby(uid))
        return 0;
    // note: this looks backwards, but ui-activity non-nil
    // means the value is "idle" (the only one we have now.
    // when ui-activity is missing, it means "active".
    // it is correct, but confusing for sure
    if(uid_attrs_has_attr(uid, "ui-activity"))
        return 0;
    return 1;
}

QList<QByteArray>
uid_attrs_get_uids()
{
    return Uid_attrs.uniqueKeys();
}
#endif



static
void
DWYCOCALLCONV
dwyco_db_login_result(const char *str, int what)
{
    if(!TheDwycoCore)
        return;
    TheDwycoCore-> emit server_login(str, what);
    if(what > 0)
    {
        reload_conv_list();
        reload_ignore_list();
        //dwyco_switch_to_chat_server(0);
    }
}

static
QByteArray
dwyco_col(int i)
{
    if(i < 0 || i > 999)
        cdcxpanic("bad col");
    char a[30];
    sprintf(a, "%03d", i);
    return QByteArray(a, 3);
}

static
QVariant
dwyco_type_to_qv(int type, const char *val, int len)
{
    QVariant ret;
    switch(type)
    {
    case DWYCO_TYPE_INT:
    {
        char *es;
        long l = strtol(val, &es, 10);
        if(es - val != len)
            cdcxpanic("bogus dwyco int");
        ret.setValue(l);
    }
    break;
    case DWYCO_TYPE_STRING:
        ret.setValue(QByteArray(val, len));
        break;
    case DWYCO_TYPE_NIL:
        break;
    case DWYCO_TYPE_VECTOR:
    {
        QList<QVariant> rv;
        DWYCO_LIST dl = (DWYCO_LIST)val;
        int n;
        int rows;
        dwyco_list_numelems(dl, &rows, &n);
        if(rows != 1)
            cdcxpanic("multidimensional array unimp");
        for(int i = 0; i < n; ++i)
        {
            const char *tmpo;
            int tmplen;
            int tmptype;
            if(!dwyco_list_get(dl, 0, dwyco_col(i).constData(), &tmpo, &tmplen, &tmptype))
                cdcxpanic("bad vector conversion");
            rv.append(dwyco_type_to_qv(tmptype, tmpo, tmplen));
        }
        ret.setValue(rv);

    }
    break;
    default:
        cdcxpanic("bogus type");
    }
    return ret;
}

void
DWYCOCALLCONV
DwycoCore::dwyco_sys_event_callback(int cmd, int id,
                         const char *uid, int len_uid,
                         const char *name, int len_name,
                         int type, const char *val, int len_val,
                         int qid,
                         int extra_arg)
{
    if(!TheDwycoCore)
        return;
    // note: these events can originate from a background thread
    // in android when the app is suspended. since most of these
    // calls end up doing something to objects that are created in the
    // main thread, or end up tweaking the UI in some way, we can't
    // really do this. instead, what we do is just reestablish the state
    // of the UI when the app is brought back to life.
    // note that some of this *might* work, as qt is reasonable about
    // queuing signals to other objects. but there are cases where
    // new object might get created in models, but qt doesn't like that.
    // so unless we find a clever way of using signals to perform those
    // sorts of operations, the method mentioned above will have to do.
    if(DwycoCore::Suspended)
        return;
    if(uid == 0)
    {
        uid = "";
        len_uid = 0;
    }
    QByteArray suid(uid, len_uid);
    QString huid = suid.toHex();
    QByteArray str_data;
    if(type == DWYCO_TYPE_STRING)
        str_data = QByteArray(val, len_val);
    QByteArray namestr;
    if(name)
    {
        namestr = QByteArray(name, len_name);
    }
    dwyco_add_entropy_timer(suid.constData(), suid.length());

    //printf("SYS EVENT %d\n", cmd);

    if(cmd == DWYCO_SE_USER_UID_RESOLVED)
    {
        TheDwycoCore-> emit sys_uid_resolved(huid);
    }
    else if(cmd == DWYCO_SE_USER_PROFILE_INVALIDATE)
    {
        //Session_uids.remove(suid);
        TheDwycoCore->emit sys_invalidate_profile(huid);
        dwyco_fetch_info(uid, len_uid);

    }
    else if(cmd == DWYCO_SE_USER_MSG_IDX_UPDATED ||
            cmd == DWYCO_SE_USER_MSG_IDX_UPDATED_PREPEND)
    {
        TheDwycoCore->emit sys_msg_idx_updated(huid, cmd == DWYCO_SE_USER_MSG_IDX_UPDATED_PREPEND ? 1 : 0);
    }
    else if(cmd == DWYCO_SE_USER_ADD)
    {
        TheConvListModel->add_uid_to_model(suid);
        dwyco_fetch_info(uid, len_uid);
    }
    else if(cmd == DWYCO_SE_USER_DEL)
    {
        TheConvListModel->remove_uid_from_model(suid);
    }

    switch(cmd)
    {
    case DWYCO_SE_MSG_SEND_START:
    case DWYCO_SE_MSG_SEND_FAIL:
    case DWYCO_SE_MSG_SEND_SUCCESS:
        TheDwycoCore->emit msg_send_status(cmd, huid, str_data);
        break;
    case DWYCO_SE_MSG_SEND_STATUS:
        TheDwycoCore->emit msg_progress(str_data, huid, namestr, extra_arg);
        break;
    case DWYCO_SE_MSG_DOWNLOAD_PROGRESS:
        TheDwycoCore->emit msg_recv_progress(str_data, huid, namestr, extra_arg);
        break;
    case DWYCO_SE_MSG_DOWNLOAD_START:
    case DWYCO_SE_MSG_DOWNLOAD_FAILED:
    case DWYCO_SE_MSG_DOWNLOAD_FETCHING_ATTACHMENT:
    case DWYCO_SE_MSG_DOWNLOAD_ATTACHMENT_FETCH_FAILED:
    case DWYCO_SE_MSG_DOWNLOAD_OK:
    case DWYCO_SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED:
    case DWYCO_SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED_DECRYPT_FAILED:
        TheDwycoCore->emit msg_recv_state(cmd, str_data, huid);
        break;

    default:
        break;
    }
}

void
DwycoCore::emit_chat_event(int cmd, int id, const char *uid, int len_uid, const char *name, int len_name,
                int type, const char *val, int len_val,
                int qid, int extra_arg)
{
#if 0
    QByteArray buid(uid, len_uid);
    QString huid = buid.toHex();
    QString sname(QByteArray(name, len_name));
    QByteArray bname(name, len_name);
    QVariant v = dwyco_type_to_qv(type, val, len_val);
    TheDwycoCore->emit chat_event(cmd, id, QString(buid.toHex()), sname, v, qid, extra_arg);

    switch(cmd)
    {
    case DWYCO_CHAT_CTX_NEW:
        uid_attrs_clear();
        Chat_sort_proxy->setDynamicSortFilter(false);
        TheChatListModel->clear();
        Chat_sort_proxy->setDynamicSortFilter(true);
        //decorate_users();
        break;

    case DWYCO_CHAT_CTX_ADD_USER:
    {
        // clear all previous attrs
        uid_attrs_del_uid(buid);
        uid_attrs_add(buid, "added");
        ChatUser *c = TheChatListModel->add_uid_to_model(buid);
        c->update_active(true);

        //TheDwycoCore->emit decorate_user(huid);
    }

    break;

    case DWYCO_CHAT_CTX_DEL_USER:
    {
        uid_attrs_del_uid(buid);
        TheChatListModel->remove_uid_from_model(buid);
        //TheDwycoCore->emit decorate_user(huid);
    }
    break;

    case DWYCO_CHAT_CTX_START_UPDATE:
        Chat_sort_proxy->setDynamicSortFilter(0);
        break;

    case DWYCO_CHAT_CTX_END_UPDATE:
        Chat_sort_proxy->setDynamicSortFilter(1);
        break;

    case DWYCO_CHAT_CTX_UPDATE_ATTR:
    {
#if 0
        if(dname.eq("ui-dict-err"))
        {
#if 0
            if(Me)
            {
                append_msg_to_textedit("<font color=\"#ff0000\">Please change your handle to remove profanity and advertising (website links, etc.)</font>", c->ui.text_display, "");
                cursor_to_bottom(c->ui.text_display);
            }
#endif
            if(type == DWYCO_TYPE_NIL)
                uid_attrs_del(duid, dname);
            else
                uid_attrs_add(duid, dname);
        }
        else if(dname.eq("ui-muted"))
        {
            if(type == DWYCO_TYPE_NIL)
                uid_attrs_del(duid, dname);
            else
                uid_attrs_add(duid, dname);
        }
        else if(dname.eq("ui-pals-only"))
        {
            if(type == DWYCO_TYPE_NIL)
                uid_attrs_del(duid, dname);
            else
                uid_attrs_add(duid, dname);

        }
        else if(dname.eq("ui-demigod"))
        {
            if(type == DWYCO_TYPE_NIL)
                uid_attrs_del(duid, dname);
            else
            {
                uid_attrs_add(duid, dname);
                if(Me)
                    Mainwinform->emit admin_event(2);
            }
        }
        else if(dname.eq("ui-god"))
        {
            if(type == DWYCO_TYPE_NIL)
                uid_attrs_del(duid, dname);
            else
            {
                uid_attrs_add(duid, dname);
                if(Me)
                    Mainwinform->emit admin_event(1);
            }
        }
        else if(dname.eq("ui-subgod"))
        {
            if(type == DWYCO_TYPE_NIL)
                uid_attrs_del(duid, dname);
            else
            {
                uid_attrs_add(duid, dname);
                if(Me)
                    Mainwinform->emit admin_event(4);
            }
        }
        else if(dname.eq("ui-call-vector"))
        {
            if(type == DWYCO_TYPE_NIL)
                uid_attrs_del(duid, dname);
            else
            {
                DWYCO_LIST l = (DWYCO_LIST)val;
                QVector<int> cv(5);
                cv[0] = dwyco_get_attr_int(l, 0, DWYCO_CV_MAX_AUDIO);
                cv[1] = dwyco_get_attr_int(l, 0, DWYCO_CV_MAX_VIDEO);
                cv[2] = dwyco_get_attr_int(l, 0, DWYCO_CV_CHAT);
                cv[3] = dwyco_get_attr_int(l, 0, DWYCO_CV_MAX_AUDIO_RECV);
                cv[4] = dwyco_get_attr_int(l, 0, DWYCO_CV_MAX_VIDEO_RECV);
            }
        }
        else
#endif
            if(sname == "ui-activity")
            {
                if(type == DWYCO_TYPE_NIL)
                    uid_attrs_del(buid, bname);
                else
                    uid_attrs_add(buid, bname);
                ChatUser *c = TheChatListModel->getByUid(huid);
                if(c)
                    c->update_active(type == DWYCO_TYPE_NIL);
                TheDwycoCore->emit decorate_user(huid);
            }
            else
            {
            }
    }
#if 0
    case DWYCO_CHAT_CTX_SYS_ATTR:
        if(name == "us-server-rules" || name == "icuii-server-rules")
        {
            append_msg_to_textedit(data.toString(), ui->text_display, "");
            cursor_to_bottom(ui->text_display);
        }
        else if(name == "ui-trial-block")
        {
            append_msg_to_textedit(QString("<font color=\"#ff0000\">Trial expired: %1").arg(data.toString()),
                                   ui->text_display, "");
            cursor_to_bottom(ui->text_display);
        }
        else  if(name == "ui-regcode-block")
        {
            append_msg_to_textedit(QString("<font color=\"#ff0000\">Subscription code problem: %1").arg(data.toString()),
                                   ui->text_display, "");
            cursor_to_bottom(ui->text_display);
        }
        else if(name == "ui-global-block")
        {
            append_msg_to_textedit("<font color=\"#ff0000\">You have been blocked from using the service.",
                                   ui->text_display, "");
            cursor_to_bottom(ui->text_display);
        }
        else if(name == "ui-full")
        {
            append_msg_to_textedit("<font color=\"#ff0000\">Sorry, the server is full, try a different one.</font>", ui->text_display, "");
            cursor_to_bottom(ui->text_display);

        }
        else if(name == "ui-pw-bad")
        {
            append_msg_to_textedit("<font color=\"#ff0000\">Password incorrect.</font>", ui->text_display, "");
            cursor_to_bottom(ui->text_display);
        }
        else if(name == "ui-blocked")
        {
            append_msg_to_textedit("<font color=\"#ff0000\">You have been blocked from using this server.",
                                   ui->text_display, "");
            // in this case, we have a list of qvariants
            // corresponding to DWYCO_BLOCK_REC*. prolly ought to change it to a
            // map with strings or something, but the current dlli api doesn't
            // really have that level of easy introspection. so we just do it by
            // hand here.
            QList<QVariant> br = data.toList();
            time_t unblock_time = br[1].toInt();
            QString reason = br[4].toString();
            QDateTime ubt = QDateTime::fromTime_t(unblock_time);
            QString msg = QString("Block is in effect until %1<br>Reason: %2").arg(ubt.toString(Qt::SystemLocaleDate)).arg(reason);

            append_msg_to_textedit(msg, ui->text_display, "");
        }
        break;
#endif
    default:
        ;
    }
#endif
}

void
DWYCOCALLCONV
DwycoCore::dwyco_chat_ctx_callback(int cmd, int id,
                                   const char *uid, int len_uid,
                                   const char *name, int len_name,
                                   int type, const char *val, int len_val,
                                   int qid,
                                   int extra_arg)
{


    emit_chat_event(cmd, id, uid, len_uid, name, len_name,
                    type, val, len_val,
                    qid, extra_arg);

#if 0
// note: can't access val that way without causing crashes in some cases
    printf("chat ctx cmd %d id %d uid %s name %s type %d val %s qid %d\n",
           cmd, id, to_hex(duid).c_str(), dname.c_str(), type, val, qid);
#endif

}

void
DwycoCore::send_chat(QString text)
{
    QByteArray chat = text.toLatin1();

    dwyco_chat_send_data(chat.constBegin(), chat.length(), DWYCO_CHAT_DATA_PIC_TYPE_NONE, 0, 0);
}

int
DwycoCore::switch_to_chat_server(int i)
{
    return dwyco_switch_to_chat_server(i);
}

void
DwycoCore::disconnect_chat_server()
{
    dwyco_disconnect_chat_server();
}

void
DwycoCore::set_invisible_state(int s)
{
    dwyco_set_invisible_state(s);
}
#if 0
// if they installed externally, copy the files in to the local
// app storage, and rename the data folder.
// this should only need to be done once, and the process should be
// exited immediately after doing the dir name swap
//
// both dirs should be absolute paths (this appears to be ok with current
// version of android)
// src_dir will be something like '/storage/emulated/yada/.../Documents'
// target_pfx should be something for the internal storage for the app, usually
// like '/data/com.dwyco.rando/yada/files'
// the files from src_dir are recursively copied into "target_pfx" + "/upg/"
// and the migration should rename "upg" to dwyco/rando as if doing
// mv "target"+ "dwyco" "target" + "dwyco.old", in case it got created before
// if "dwyco.old" already exists, try dwyco.old2, etc. only try a few before giving up.
// mkdir target/rando
// mv target/upg target/dwyco/rando
//
static
void
one_time_migrate(const QString& src_dir, const QString& target_pfx)
{
    QDirIterator di(src_dir, QDir::Files|QDir::NoDotAndDotDot|QDir::Hidden, QDirIterator::Subdirectories);
    //QString target_pfx = "/home/dwight/Downloads/";
    while(di.hasNext())
    {
        QString sfn = di.next();
        QString dfn = sfn;
        int i = dfn.indexOf("dwyco/rando");
        if(i != -1)
        {
            dfn.remove(0, i + 11);
            dfn.prepend("/");
            dfn.prepend(target_pfx);
            QString path = dfn;
            path.truncate(path.lastIndexOf("/"));
            //printf("%s\n", path.toLatin1().constData());
            QDir d(path);
            d.mkpath(path);
        }
        //printf("%s %s\n", sfn.toLatin1().constData(), dfn.toLatin1().constData());
        QFile::copy(sfn, dfn);
    }
}

void
DwycoCore::one_time_copy_files()
{
    QString src = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    src += "/dwyco/rando";
    QString dst = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    dst += "/dwyco/upg";
    one_time_migrate(src, dst);
    //QThread::sleep(10);
}

void
DwycoCore::background_migrate()
{
    // sigh, this won't compile with android ndk's up to 23.2
    //QThread *q = QThread::create(DwycoCore::one_time_copy_files);
    auto q = new fuck_me_with_a_brick;
    connect(q, SIGNAL(finished()), this, SIGNAL(migration_complete()));
    q->start();
}
#endif

void
DwycoCore::background_reindex()
{
    auto q = new fuck_me_with_a_brick2;
    connect(q, SIGNAL(finished()), this, SIGNAL(reindex_complete()));
    q->start();
}

void
DwycoCore::do_reindex()
{
    dwyco_init();
    dwyco_exit();
    //QThread::sleep(10);
}

QUrl
DwycoCore::from_local_file(const QString& s)
{
    return QUrl::fromLocalFile(s);
}

QString
DwycoCore::to_local_file(const QUrl& u)
{
    return u.toLocalFile();
}

#if 0
void
DwycoCore::directory_swap()
{
    // note: we update the User_pfx so we can update the settings file to
    // flag that it has been migrated.
    // then rename the "upg" to "rando"
    QString src = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    src += "/dwyco/upg";
    QDir dsrc(src);
    if(!dsrc.exists())
        return;
    // if the target already exists, it might be a partial migration, try to get
    // rid of it
    QString dst = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    dst += "/dwyco/rando";
    QDir ddst(dst);
    if(ddst.exists())
    {
        int i;
        for(i = 0; i < 10; ++i)
            if(ddst.rename(dst, dst + "." + QString::number(i)))
                break;
        if(i == 10)
            return;
    }
    User_pfx = src.toUtf8();
    settings_load();
    setting_put("android-migrate", "done");
    if(!dsrc.rename(src, dst))
        cdcxpanic("failed migration");

}
#endif


static
void
setup_locations()
{
    QStandardPaths::StandardLocation filepath;

    filepath = QStandardPaths::AppDataLocation;

#if 0 && defined(ANDROID)
    QString localdir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    localdir += "/dwyco/rando/";
    User_pfx = localdir.toUtf8();
    QString am;
    if(!settings_load())
    {
        // either new or they have something in documents that needs to be migrated
        QString src = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        src += "/dwyco/rando";
        User_pfx = src.toUtf8();
#ifdef ANDROID
        bool check_for_update = false;

        if(QtAndroid::checkPermission("android.permission.READ_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Denied)
        {
            // if they let us, check if there is anything out there
            QtAndroid::PermissionResultMap m = QtAndroid::requestPermissionsSync(QStringList("android.permission.READ_EXTERNAL_STORAGE"));
            if(m.value("android.permission.READ_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Denied)
            {

            }
            else
            {
                check_for_update = true;
            }
        }
        else
            check_for_update = true;

        if(check_for_update && settings_load())
        {
            // do the migration
            if(QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Denied)
            {
                // we aren't going anywhere without being able to setup our state
                QtAndroid::PermissionResultMap m = QtAndroid::requestPermissionsSync(QStringList("android.permission.WRITE_EXTERNAL_STORAGE"));
                if(m.value("android.permission.WRITE_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Denied)
                {
                    // this needs to be thought out a little more... if you deny this, you can't
                    // access your photos on the device easily. maybe need to just request "read"
                    // in this case.
                    filepath = QStandardPaths::AppDataLocation;
                    //exit(0);
                }
                else
                {
                    filepath = QStandardPaths::DocumentsLocation;
                    DwycoCore::Android_migrate = 1;
                }
                if(QtAndroid::checkPermission("android.permission.READ_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Denied)
                {
                    // we aren't going anywhere without being able to setup our state
                    QtAndroid::PermissionResultMap m = QtAndroid::requestPermissionsSync(QStringList("android.permission.READ_EXTERNAL_STORAGE"));
                    if(m.value("android.permission.READ_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Denied)
                    {
                        // can't do migration, we'll probably crash soon since we won't be able to get
                        // access to documents location.
                    }
                    else
                    {
                        filepath = QStandardPaths::DocumentsLocation;
                        DwycoCore::Android_migrate = 1;
                    }
                }

            }
            else
#endif
            {
                filepath = QStandardPaths::DocumentsLocation;
                DwycoCore::Android_migrate = 1;
            }
        }
    }

#endif
    //
    QStringList args = QGuiApplication::arguments();
    QString userdir;
    if(args.count() == 1)
    {
        userdir = QStandardPaths::writableLocation(filepath);
        userdir += "/dwyco/rando/";
    }
    else
    {
        userdir = args[1];
        userdir += "/";
    }

    {
        QDir d(userdir);
        if(!d.mkpath(userdir))
        {
            // note: failed to make the path, or it doesn't exist.
            // this can happen on android because the permission to access
            // external storage might be ok, when in fact external storage is not
            // available. this seems like a qt bug, as why would it return
            // a "writable location" that wasn't writeable or creatable.
            // see if we can fallback to the appdatalocation
            userdir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            userdir += "/dwyco/rando/";
            QDir d2(userdir);
            if(!d2.mkpath(userdir))
                ::abort();
        }

    }
    {
        QDir shares(userdir + "shares");
        shares.mkpath(userdir + "shares");
    }
#ifdef ANDROID
    QFile::copy("assets:/dwyco.dh", userdir + "dwyco.dh");
    QFile::copy("assets:/license.txt", userdir + "license.txt");
    QFile::copy("assets:/no_img.png", userdir + "no_img.png");
    if(!QFile(userdir + "servers2").exists())
        QFile::copy("assets:/servers2", userdir + "servers2");
    QFile::copy("assets:/v21.ver", userdir + "v21.ver");
    QFile::copy("assets:/qtquickcontrols2.conf", userdir + "qtquickcontrols2.conf");
#else
    QFile::copy(":androidinst3/assets/dwyco.dh", userdir + "dwyco.dh");
    QFile::copy(":androidinst3/assets/license.txt", userdir + "license.txt");
    QFile::copy(":androidinst3/assets/no_img.png", userdir + "no_img.png");
    if(!QFile(userdir + "servers2").exists())
        QFile::copy(":androidinst3/assets/servers2", userdir + "servers2");
    QFile::setPermissions(userdir + "servers2", QFile::ReadOwner|QFile::WriteOwner);
    QFile::copy(":androidinst3/assets/v21.ver", userdir + "v21.ver");
#endif
    dwyco_set_fn_prefixes(userdir.toLatin1().constData(), userdir.toLatin1().constData(), QString(userdir + "tmp/").toLatin1().constData());
    // can't do this call until prefixes are set since it wants to init the log file
    dwyco_trace_init();
    {
        char sys[1024];
        char user[1024];
        char tmp[1024];
        int len_sys = sizeof(sys);
        int len_user = sizeof(user);
        int len_tmp = sizeof(tmp);

        dwyco_get_fn_prefixes(sys, &len_sys, user, &len_user, tmp, &len_tmp);
        Sys_pfx = QByteArray(sys, len_sys - 1);
        User_pfx = QByteArray(user, len_user - 1);
        Tmp_pfx = QByteArray(tmp, len_tmp - 1);
        if(Sys_pfx.length() == 0)
            Sys_pfx = ".";
        if(User_pfx.length() == 0)
            User_pfx = ".";
        if(Tmp_pfx.length() == 0)
            Tmp_pfx = ".";

    }

    // now that we have locations, get the local settings
    settings_load();
    srand(time(0) & 0xffffffff);
    QString sport;
    int port;
    if(!setting_get("syncport", sport))
    {
        port = (rand() % 50000) + 10000;
        sport = QString::number(port);
        setting_put("syncport", sport);
    }
    port = sport.toInt();
    //port = 57899;
#ifdef ANDROID
    notificationClient->set_service_params(port, Sys_pfx, User_pfx, Tmp_pfx);
#endif
    takeover_from_background(port);
    BGLockPort = port;

    {
        QDir d(Tmp_pfx);
        d.mkpath(Tmp_pfx);
        // this is just a stopgap, really need to do something to obfuscate the files
        // a little bit to avoid apps indexing temp images on all platforms
        QString fp = d.filePath(".nomedia");
        QFile f(fp);
        f.open(QIODevice::WriteOnly);
        f.putChar(0);
        f.close();
    }
    {
        QDir d;
        d.mkpath(User_pfx);
    }

    QString first_pin;
    if(!setting_get("first-pin", first_pin))
    {
        setting_put("pw", "");
        setting_put("salt", "");
        setting_put("first-pin", "1");
    }

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, userdir);

    // this is where we would set a flag if a one-time upgrade is needed.
    // the qml would immediately put up a "wait..." dialog and launch the
    // copy operation in a background thread, without initializing anything else.
    //
    // note: we *know* that if we are getting the settings here we are running
    // on some kind of install. if that looks like it is in the "documents" folder
    // we assume it is on android "external storage" and needs to be imported.
#if 0 && defined( ANDROID)
    QString am;
    if(!setting_get("android-migrate", am))
    {
        // haven't completed it yet, so check to see if it looks like we are on
        // external storage. note: do not check "write" permission, because that might
        // have been revoked at some point (maybe?) not sure.
        if(userdir.contains(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)))
        {
            DwycoCore::Android_migrate = 1;
        }
        else
        {
            // we probably already were in the app data location, so just pretend we
            // migrated
            setting_put("android-migrate", "done");
        }

    }
#endif


}

// remember: on ms platforms using the old non-qt
// drivers, frames tend to be bgr and upside down.
// the newer drivers are a little more sensible in
// this area, and return rgb and whatever orientation
// is set by the driver.
static
QImage
dwyco_img_to_qimg(void *vimg, int cols, int rows, int depth)
{
    unsigned char **img = (unsigned char **)vimg;

    QImage qi(cols, rows, QImage::Format_BGR888);

#if 1 && defined(DWYCO_FORCE_DESKTOP_VGQT)
    for(int r = 0; r < rows; ++r)
    {
        unsigned char *sli = img[r];
        uchar *sl = qi.scanLine(r);
        memcpy(sl, sli, 3 * cols);
    }
    qi.mirror(false, true);
#else
    for(int r = 0; r < rows; ++r)
    {

        unsigned char *sli = img[rows - r - 1];
        uchar *sl = qi.scanLine(r);
        //memcpy(qid + (r * qi.bytesPerLine()), sli, 3 * cols);
        for(int c = 0; c < cols * 3; c += 3)
        {
            sl[c] = sli[c + 2];
            sl[c + 1] = sli[c + 1];
            sl[c + 2] = sli[c];
        }
    }
#endif
    return qi;
}

static
void
DWYCOCALLCONV
dwyco_video_make_image(int ui_id, void *vimg, int cols, int rows, int depth)
{
#if 0
    if(!TheDwycoCore)
        return;

    // don't do grayscale for the moment
    if(depth != 3)
        return;
    static int frame_number;
    ++frame_number;
    QString img_path;
    QImage qi = dwyco_img_to_qimg(vimg, cols, rows, depth);
    // the frame number being included in the image path is largely just
    // to defeat the caching QML does for images.
    if(ui_id == DWYCO_VIDEO_PREVIEW_CHAN)
    {

//        Mainwinform-> emit video_capture_preview(qi);
        // video preview isn't used right now, have to figure out a way
        // to avoid accumulating a bunch of frames
        Dwyco_video_preview_provider->replace_img(qi);
        img_path = "image://dwyco_video_preview";
        img_path += "/";
        img_path += QString::number(frame_number);
        TheDwycoCore-> emit video_capture_preview(img_path);
        return;

    }

    img_path += QString::number(ui_id);
    img_path += "/";
    img_path += QString::number(frame_number);
    Dwyco_video_provider->add_image(img_path, qi);
    TheDwycoCore-> emit video_display(ui_id, frame_number, QString("image://dwyco_video_frame/") + img_path);
#endif

}

#if 0
static
void
DWYCOCALLCONV
dwyco_user_control(int chan_id, const char *suid, int len_uid, const char *data, int len_data)
{
    if(!TheDwycoCore)
        return;
    QByteArray uid(suid, len_uid);
    QByteArray com(data, len_data);
    // this is a bit weird... if we ignore someone we would really
    // like to terminate all connections to them, even the ones
    // that were setup "in the background" without our explicit knowledge.
    // this happens with these control channels if the other party is
    // the one setting it up.
    // i could change the core to notify when this connection was set up, but
    // that kinda defeats the purpose of making it transparent.
    // it is also a bit of an info leak if you terminate a connection on
    // ignore (not a big deal.) so by doing this here, the connection may
    // survive even if one side or the other has done an ignore, but
    // as soon as something comes across the channel, we nix it.
    // this probably needs to be done in the core, more i think about it.
    if(dwyco_is_ignored(suid, len_uid))
    {
        dwyco_destroy_channel(chan_id);
        return;
    }
    emit TheDwycoCore->user_control(chan_id, uid, com);
}
#endif

// probably need to do something rational here, esp for database changes
// this is good enough for debugging
static
void
DWYCOCALLCONV
dwyco_emergency(int what, int must_exit, const char *msg)
{
    if(what == DWYCO_EMERGENCY_GENERAL_PANIC)
        ::abort();

}

//static
//void
//DWYCOCALLCONV
//dwyco_chat_server_status(int id, const char *msg, int, void *)
//{
//    if(!TheDwycoCore)
//        return;
//    if(strcmp(msg, "online") == 0)
//    {
//        TheDwycoCore-> emit sys_chat_server_status(id, 1);
//    }
//    else if(strcmp(msg, "offline") == 0)
//    {
//        TheDwycoCore->emit sys_chat_server_status(id, 0);
//    }
//}

#if 0
int Block_DLL;

static int
block_enable_video_capture_preview(int on)
{
    Block_DLL = 1;
    int ret = dwyco_enable_video_capture_preview(on);
    Block_DLL = 0;
    return 1;
    // this is super-hacky hack-around. when we turn off
    // preview, we sometimes need to assume the device will have
    // been shutdown before we start tweaking with other video
    // capture related stuff. unfortunately, the dll doesn't provide
    // a clean interface to "shut the device up NOW". so we'll
    // just spin a little bit and hope for the best.
    int dum;
    for(int i = 0; i < 100; ++i)
        dwyco_service_channels(&dum);
    return ret;
}

static
void
write_vid_setting(int i)
{
    QVariant v = CamListModel->get(i);
    setting_put("video device", v.toString());
    settings_save();
    TheDwycoCore->update_vid_dev_idx(i);
    TheDwycoCore->update_vid_dev_name(v.toString());
}

void
DwycoCore::enable_video_capture_preview(int i)
{
    dwyco_enable_video_capture_preview(i);
}

void
DwycoCore::select_vid_dev(int i)
{
    block_enable_video_capture_preview(0);
    HasCamera = 0;
    dwyco_shutdown_vfw();
    dwyco_set_setting("video_input/no_video", "1");
    dwyco_set_setting("video_input/vfw", "0");
    dwyco_set_setting("video_input/raw", "0");

    if(i == 0)
    {
        TheDwycoCore->emit camera_change(0);
        write_vid_setting(0);
        return;
    }
    if(i == 1)
    {
        dwyco_set_setting("video_input/no_video", "0");
        dwyco_set_setting("video_input/vfw", "0");
        dwyco_set_setting("video_input/raw", "1");
    }
    else if(i > 1)
    {
        dwyco_start_vfw(i - 2, 0, 0);
        dwyco_set_setting("video_input/no_video", "0");
        dwyco_set_setting("video_input/vfw", "1");
        dwyco_set_setting("video_input/raw", "0");
    }
    write_vid_setting(i);
    block_enable_video_capture_preview(1);
    HasCamera = 1;
    TheDwycoCore->emit camera_change(HasCamera);
}


static
void
load_cam_model()
{
    QString vid_dev;
    setting_get("video device", vid_dev);

    CamListModel->append("(Select this to disable video)");
    CamListModel->append("(Files)");
#if defined(DWYCO_FORCE_DESKTOP_VGQT) || defined(ANDROID) || defined(DWYCO_IOS)
    CamListModel->append("Camera");
    HasCamHardware = 1;
#else


    DWYCO_LIST drv = dwyco_get_vfw_drivers();
    if(drv)
    {
        int n;
        dwyco_list_numelems(drv, &n, 0);
        for(int i = 0; i < n; ++i)
        {
            QByteArray b = dwyco_get_attr(drv, i, DWYCO_VFW_DRIVER_NAME);

            CamListModel->append(QString(b));
            HasCamHardware = 1;
        }
    }
#endif

    TheDwycoCore->update_vid_dev_name(vid_dev);

    // note: may not be able to get camera
    // selected until after qml is up and
    // running all the way
#if 1
    // select last cam
    int found_it = 0;
    for(int i = 0; i < CamListModel->count(); ++i)
    {
        QVariant v = CamListModel->get(i);
        if(v.toString() == vid_dev)
        {
            TheDwycoCore->update_vid_dev_idx(i);
            TheDwycoCore->select_vid_dev(i);
            return;
        }
    }

//    if(HasCamHardware)
//    {
//        // just select the first one
//        TheDwycoCore->select_vid_dev(2);
//    }
#endif

}
#endif

static
void
emit_finished(int chan_id, void *)
{
    TheDwycoCore->emit zap_stopped(chan_id);
}


int
DwycoCore::make_zap_composition()
{
    return dwyco_make_zap_composition(0);
}

int
DwycoCore::start_zap_record(int zid, int vid, int aud)
{
    int ui_id = -1;
    if(!dwyco_zap_record2(zid, vid, aud, -1, 10000000, 1, 0, 0, emit_finished, 0, &ui_id))
        return -1;
    return ui_id;
}

int
DwycoCore::start_zap_record_pic(int zid)
{
    int ui_id = -1;
    //if(!dwyco_zap_record(zid, 1, 0, 1, -1, emit_finished, 0, &ui_id))
    if(!dwyco_zap_record2(zid, 1, 0, 1, 1000000, 1, 0, 0, emit_finished, 0, &ui_id))
        return -1;
    return ui_id;
}

int
DwycoCore::stop_zap_record(int zid)
{
    if(!dwyco_zap_stop(zid))
        return 0;
    return 1;
}

int
DwycoCore::play_zap(int zid)
{
    int ui_id = -1;
    if(!dwyco_zap_play(zid, emit_finished, 0, &ui_id))
        return -1;
    return ui_id;
}

int
DwycoCore::stop_zap(int zid)
{
    if(!dwyco_zap_stop(zid))
        return 0;
    return 1;
}

int
DwycoCore::cancel_zap(int zid)
{
    if(!dwyco_delete_zap_composition(zid))
        return 0;
    return 1;
}

int
DwycoCore::send_zap(int zid, QString recipient, int save_sent)
{
    QByteArray ruid = QByteArray::fromHex(recipient.toLatin1());

    if(!dwyco_zap_send5(zid, ruid.constData(), ruid.length(),
                        "", 0, 0, save_sent,
                        0, 0)
      )
    {
        return 0;
    }
    return 1;
}

void
DwycoCore::update_dwyco_client_name(QString name)
{
    QByteArray nm = name.toLatin1();
    dwyco_set_client_version(nm.constData(), nm.length());
}


void
DwycoCore::init()
{
    if(TheDwycoCore)
        return;
    TheDwycoCore = this;
    TheMan = QByteArray::fromHex("5a098f3df49015331d74");
    update_user_dir(User_pfx);
    update_tmp_dir(Tmp_pfx);

    //DVP::init_dvp();
    //simple_call::init(this);
    AvoidSSL = 1; //!QSslSocket::supportsSsl();

    dwyco_set_login_result_callback(dwyco_db_login_result);
    //dwyco_set_chat_ctx_callback(dwyco_chat_ctx_callback);
    dwyco_set_system_event_callback(dwyco_sys_event_callback);
    //dwyco_set_video_display_callback(dwyco_video_make_image);
    //dwyco_set_user_control_callback(dwyco_user_control);
    dwyco_set_emergency_callback(dwyco_emergency);
    //dwyco_set_chat_server_status_callback(dwyco_chat_server_status);

#if 0
#if defined(MAC_CLIENT)
    // we do this early, then if we need to override one of the
    // subsystems (like to use qt instead of the old drivers)
    // that can be done further down.
    init_mac_drivers();
#endif
#if ((defined(LINUX) && !defined(MAC_CLIENT)) || defined(DWYCO_IOS)) && !defined(NO_DWYCO_AUDIO)


#if 0&& defined(LINUX) && !defined(ANDROID) && !defined(MAC_CLIENT)
    dwyco_set_external_audio_output_callbacks(
        audout_sdl_new,
        audout_sdl_delete,
        audout_sdl_init,
        audout_sdl_device_output,
        audout_sdl_device_done,
        audout_sdl_device_stop,
        audout_sdl_device_reset,
        audout_sdl_device_status,
        audout_sdl_device_close,
        audout_sdl_device_buffer_time,
        audout_sdl_device_play_silence,
        audout_sdl_device_bufs_playing
    );

// this may eventually work for android, for now there is some
// timing/buffer problem. seems to work ok on linux tho

#else


    dwyco_set_external_audio_output_callbacks(
        audout_qt_new,
        audout_qt_delete,
        audout_qt_init,
        audout_qt_device_output,
        audout_qt_device_done,
        audout_qt_device_stop,
        audout_qt_device_reset,
        audout_qt_device_status,
        audout_qt_device_close,
        audout_qt_device_buffer_time,
        audout_qt_device_play_silence,
        audout_qt_device_bufs_playing
    );
#endif
#endif
#if ((defined(LINUX) && !defined(MAC_CLIENT)) || defined(DWYCO_IOS)) && /*!defined(ANDROID) &&*/ !defined(NO_DWYCO_AUDIO)

//dwyco_set_external_audio_capture_callbacks(
// esd_new,
// esd_delete,
// esd_init,
// esd_has_data,
// esd_need,
// esd_pass,
// esd_stop,
// esd_on,
// esd_off,
// esd_reset,
// esd_status,
// esd_get_data
//
//);

    dwyco_set_external_audio_capture_callbacks(
        audi_qt_new,
        audi_qt_delete,
        audi_qt_init,
        audi_qt_has_data,
        audi_qt_need,
        audi_qt_pass,
        audi_qt_stop,
        audi_qt_on,
        audi_qt_off,
        audi_qt_reset,
        audi_qt_status,
        audi_qt_get_data

    );

#endif

#if defined(DWYCO_FORCE_DESKTOP_VGQT) || defined(ANDROID) //|| defined(DWYCO_IOS)
    dwyco_set_external_video_capture_callbacks(
        vgqt_new,
        vgqt_del,
        vgqt_init,
        vgqt_has_data,
        vgqt_need,
        vgqt_pass,
        vgqt_stop,
        vgqt_get_data,
        vgqt_free_data,
        0, 0, 0, 0, 0, 0, 0, 0

    );

#elif defined(LINUX) && !defined(EMSCRIPTEN) && !defined(MAC_CLIENT) && defined(DWYCO_VIdEO)
    dwyco_set_external_video_capture_callbacks(
        vgnew,
        vgdel,
        vginit,
        vghas_data,
        vgneed,
        vgpass,
        vgstop,
        vgget_data,
        vgfree_data,
        vgget_video_devices,
        vgfree_video_devices,
        vgset_video_device,
        vgstop_video_device,
        0, 0, 0, 0

    );

#endif
#endif

    //settings_load();
    //dwyco_create_bootstrap_profile("qml", 3, "qml test", 8, "none", 4, "fcktola1@gmail.com", 18);
    int inv = 0;
    QString a = get_local_setting("invis");
    if(a == "" || a == "false")
        inv = 0;
    else
        inv = 1;
    dwyco_set_initial_invis(inv);
    dwyco_inhibit_pal(1);
#ifdef ANDROID
    // this is a kluge for android
    // the FCM token may or not be available at this point, but
    // eventually it will be (we hope). so write it out so it gets
    // sent to the server on login. if it doesn't get out properly, it
    // isn't the end of the world, the user may not get notifications
    // right away while the app is in the background, the phone is snoozing, etc.etc.
    // they will get the notification eventually when they restart the app tho
    // this should eventually be made a little more robust, but for now it is ok.
    QString token = notificationClient->get_token();
    // note: kinda assume the token is 8-bit ascii, but who knows
    QByteArray b = token.toLatin1();
    dwyco_write_token(b.constData());

#endif

    if(!dwyco_init())
        ::abort();
    Init_ok = 1;
    dwyco_set_setting("zap/always_server", "0");
    dwyco_set_setting("call_acceptance/auto_accept", "0");
    dwyco_set_setting("net/listen", "0");

    //new profpv;
    // the order of these is important, you have to clear the cache
    // before you run the model "resolved" handlers so they can
    // poke qml to re-read images.
    //connect(this, SIGNAL(sys_invalidate_profile(QString)), ThePreviewCache, SLOT(remove_entry(QString)));
    //connect(this, SIGNAL(sys_uid_resolved(QString)), ThePreviewCache, SLOT(remove_entry(QString)));

    //connect(this, SIGNAL(pal_event(QString)), sort_proxy_model, SLOT(decorate_user(QString)));
    //connect(this, SIGNAL(ignore_event(QString)), sort_proxy_model, SLOT(decorate_user(QString)));

    connect(QGuiApplication::instance(), SIGNAL(applicationStateChanged(Qt::ApplicationState)), this, SLOT(app_state_change(Qt::ApplicationState)));
//    connect(this, SIGNAL(sys_uid_resolved(QString)), TheChatListModel, SLOT(uid_resolved(QString)));
//    connect(this, SIGNAL(sys_invalidate_profile(QString)), TheChatListModel, SLOT(uid_invalidate_profile(QString)));
//    connect(this, SIGNAL(new_msg(QString,QString,QString)), TheChatListModel, SLOT(decorate(QString,QString,QString)));
//    connect(this, SIGNAL(decorate_user(QString)), TheChatListModel, SLOT(decorate(QString)));
//    connect(this, SIGNAL(ignore_event(QString)), TheChatListModel, SLOT(ignore_state_change(QString)));

    connect(this, SIGNAL(new_msg(QString,QString,QString)), TheConvListModel, SLOT(decorate(QString,QString,QString)));
    connect(this, SIGNAL(decorate_user(QString)), TheConvListModel, SLOT(decorate(QString)));
    connect(this, SIGNAL(sys_uid_resolved(QString)), TheConvListModel, SLOT(uid_resolved(QString)));
    connect(this, SIGNAL(sys_invalidate_profile(QString)), TheConvListModel, SLOT(uid_invalidate_profile(QString)));
    connect(this, SIGNAL(ignore_event(QString)), TheConvListModel, SLOT(decorate(QString)));

    //connect(this, SIGNAL(sys_uid_resolved(QString)), TheIgnoreListModel, SLOT(uid_resolved(QString)));
    //connect(this, SIGNAL(sys_invalidate_profile(QString)), TheIgnoreListModel, SLOT(uid_invalidate_profile(QString)));
    connect(this, SIGNAL(msg_recv_state(int,QString,QString)), mlm, SLOT(msg_recv_status(int,QString,QString)));
    connect(this, SIGNAL(mid_tag_changed(QString)), mlm, SLOT(mid_tag_changed(QString)));
    connect(this, SIGNAL(msg_recv_progress(QString,QString,QString,int)), mlm, SLOT(msg_recv_progress(QString,QString,QString,int)));
    connect(this, SIGNAL(client_nameChanged(QString)), this, SLOT(update_dwyco_client_name(QString)));
    connect(this, &DwycoCore::use_archivedChanged, reload_conv_list);
    //connect(this, SIGNAL(sys_msg_idx_updated(QString)), this, SLOT(internal_cq_check(QString)));

    if(dwyco_get_create_new_account())
        return;
    dwyco_set_local_auth(1);
    dwyco_finish_startup();

    //dwyco_run_sql("delete from msg_tags2 where mid not in (select mid from msg_idx)", 0, 0, 0);
    dwyco_unset_all_msg_tag("_seen");

    load_unviewed();
    update_unread_count(total_unviewed_msgs_count());
    reload_conv_list();
    reload_ignore_list();
    update_unseen_from_db();
    QString first_bugfix1;
    if(!setting_get("bugfix1", first_bugfix1))
    {
        clear_unviewed_msgs();
        setting_put("bugfix1", "");

    }


    setting_put("tag_change1", "");

    {
        // just query for _json tag and processes those directly
        DWYCO_LIST tml;
        if(dwyco_get_tagged_idx(&tml, "_json", 0))
        {
            simple_scoped stml(tml);
            for(int i = 0; i < stml.rows(); ++i)
            {
                DWYCO_SAVED_MSG_LIST sml;
                QByteArray u = QByteArray::fromHex(stml.get<QByteArray>(i, DWYCO_MSG_IDX_ASSOC_UID));
                QByteArray mid = stml.get<QByteArray>(i, DWYCO_MSG_IDX_MID);
                long lc = stml.get_long(i, DWYCO_MSG_IDX_LOGICAL_CLOCK);
                if(dwyco_get_saved_message(&sml, u.constData(), u.length(), mid.constData()))
                {
                    simple_scoped ssml(sml);
                    if(ssml.is_nil(DWYCO_QM_BODY_ATTACHMENT))
                    {
                        QByteArray txt = ssml.get<QByteArray>(DWYCO_QM_BODY_NEW_TEXT2);
                        QJsonDocument qjd = QJsonDocument::fromJson(txt);
                        if(!qjd.isNull())
                        {
                            QJsonObject qjo = qjd.object();
                            if(!qjo.isEmpty())
                            {
                                QJsonValue h = qjo.value("hash");
                                QJsonValue loc = qjo.value("loc");
                                QJsonValue rev = qjo.value("review");
                                QJsonValue lat = qjo.value("lat");
                                QJsonValue lon = qjo.value("lon");
                                if(!h.isUndefined())
                                {
                                    QByteArray hh = QByteArray::fromHex(h.toString().toLatin1());
                                    QLoc loca;
                                    loca.hash = hh;
                                    loca.mid = mid;
                                    if(!loc.isUndefined())
                                        loca.loc = loc.toString().toLatin1();
                                    if(!rev.isUndefined())
                                        Hash_to_review.insert(hh, rev.toString().toLatin1());
                                    if(!lat.isUndefined())
                                        loca.lat = lat.toString().toLatin1();
                                    if(!lon.isUndefined())
                                        loca.lon = lon.toString().toLatin1();
                                    QList<QLoc> ql = Hash_to_loc.values(hh);
                                    if(!ql.contains(loca))
                                    {
                                        Hash_to_loc.insert(hh, loca);
                                    }
                                    long v = Hash_to_max_lc.value(hh, 0);
                                    if(lc > v)
                                        Hash_to_max_lc.insert(hh, lc);

                                }
                            }
                        }
                    }
                }
            }
        }
    }


    const char *uid;
    int len_uid;
    dwyco_get_my_uid(&uid, &len_uid);
    My_uid = QByteArray(uid, len_uid);
    //update_this_uid(My_uid.toHex());



#if 0
    int audio_full_duplex = 0;

    dwyco_get_audio_hw(&HasAudioInput, &HasAudioOutput, &audio_full_duplex);
    update_audio_full_duplex(audio_full_duplex);
    update_has_audio_input(HasAudioInput);
    update_has_audio_output(HasAudioOutput);
    if(audio_full_duplex)
        dwyco_set_full_duplex(1);

    if(HasAudioInput)
    {
        dwyco_set_setting("call_acceptance/max_audio", "4");
    }
    else
    {
        dwyco_set_setting("call_acceptance/max_audio", "0");
    }

    if(HasAudioOutput)
    {

        dwyco_set_setting("call_acceptance/max_audio_recv", "4");
    }
    else
    {
        dwyco_set_setting("call_acceptance/max_audio_recv", "0");
    }
#endif
    update_android_backup_available(dwyco_get_android_backup_state());
    setup_emergency_servers();

}

int
DwycoCore::load_backup()
{
    int ret = dwyco_restore_android_backup();
    return ret;
}

int
DwycoCore::get_android_backup_state()
{
    return dwyco_get_android_backup_state();
}

QString
DwycoCore::map_to_representative(const QString& uid)
{
    QByteArray b = uid.toLatin1();
    b = QByteArray::fromHex(b);
    DWYCO_LIST urep;
    dwyco_map_uid_to_representative(b.constData(), b.length(), &urep);
    simple_scoped qurep(urep);
    b = qurep.get<QByteArray>(0);
    return b.toHex();
}

void
DwycoCore::set_badge_number(int i)
{
#if  defined(MACOSX) && !defined(DWYCO_IOS) && defined(DWYCO_QT5)
    if(i == 0)
        QtMac::setBadgeLabelText("");
    else
        QtMac::setBadgeLabelText(QString::number(i));

#endif
}

#if 0
int
DwycoCore::load_contacts()
{
#if 0 && ANDROID
    if(QtAndroid::checkPermission("android.permission.READ_CONTACTS") == QtAndroid::PermissionResult::Denied)
    {
        QtAndroid::PermissionResultMap m = QtAndroid::requestPermissionsSync(QStringList("android.permission.READ_CONTACTS"));
        if(m.value("android.permission.READ_CONTACTS") == QtAndroid::PermissionResult::Denied)
        {
            return 0;
        }
    }
    notificationClient->load_contacts();
    return 1;
#else
    // just some test data, as desktops don't have contact lists yet
    dwyco_clear_contact_list();
    dwyco_add_contact("foo", "123", "bar@baz.com");

//    dwyco_add_contact("bar", "456-456", "bazbar@foo.com");
    dwyco_add_contact("bar", "456-456", "dwyco1@gmail.com");
//    dwyco_add_contact("bar", "456-456", "fcktola1@gmail.com");
//    dwyco_add_contact("bar", "456-456", "dwight@dwyco.com");
    dwyco_add_contact("bar", "456-456", "jewelscumberland@gmail.com");
    return 1;
#endif
}
#endif

int DwycoCore::Suspended;
void
DwycoCore::app_state_change(Qt::ApplicationState as)
{
    // note: comment out the "inactive" normally, but put it back in
    // when testing "background" stuff on desktop
    if(as == Qt::ApplicationSuspended  /*|| as == Qt::ApplicationInactive*/)
    {
        Suspended = 1;
        dwyco_set_disposition("background", 10);
        //simple_call::suspend();
        dwyco_disconnect_chat_server();
        dwyco_suspend();
        if(BGLockSock)
        {
            BGLockSock->close();
            delete BGLockSock;
            BGLockSock = 0;
        }
#ifdef ANDROID
        notificationClient->start_background();
        notificationClient->set_allow_notification(1);
#endif

        emit qt_app_state_change(1);
    }
    else if(as == Qt::ApplicationActive && Suspended)
    {
        takeover_from_background(BGLockPort);
        dwyco_resume();
        //simple_call::resume();
        // note: background process may have updated messages on disk
        // *and* we may not get to the server, so force a reload here just
        // in case.
        dwyco_set_disposition("foreground", 10);
        update_dwyco_client_name(m_client_name);
        QSet<QByteArray> dum;
        load_inbox_tags_to_unviewed(dum);
        reload_conv_list();
        Suspended = 0;
#ifdef ANDROID
        notificationClient->set_allow_notification(0);
#endif
        emit qt_app_state_change(0);
    }
#ifdef ANDROID
    if(notificationClient)
        notificationClient->cancel();
#endif
}

#if 0
bool
DwycoCore::get_cc_property(QString uid, QString button_name, QString property_name)
{
    simple_call *c = simple_call::get_simple_call(QByteArray::fromHex(uid.toLatin1()));
    if(!c)
        return false;
    FauxButton *f = c->findChild<FauxButton *>(button_name);
    if(!f)
        return false;
    QVariant v = f->property(property_name.toLatin1());
    if(!v.isValid())
        return false;
    return v.toBool();
}

void
DwycoCore::set_cc_property(QString uid, QString button_name, QString property_name, bool val)
{
    simple_call *c = simple_call::get_simple_call(QByteArray::fromHex(uid.toLatin1()));
    if(!c)
        return;
    FauxButton *f = c->findChild<FauxButton *>(button_name);
    if(!f)
        return;
    f->setProperty(property_name.toLatin1(), val);
}

QObject *
DwycoCore::get_button_model(QString uid)
{
    simple_call *c = simple_call::get_simple_call(QByteArray::fromHex(uid.toLatin1()));
    if(!c)
        return 0;
    return c->ui->button_model;
}

int
DwycoCore::get_rem_keyboard_state(QString uid)
{
    simple_call *c = simple_call::get_simple_call(QByteArray::fromHex(uid.toLatin1()));
    return c->rem_kb_active;

}

int
DwycoCore::get_established_state(QString uid)
{
    simple_call *c = simple_call::get_simple_call(QByteArray::fromHex(uid.toLatin1()));
    return c->get_connected();

}

void
DwycoCore::try_connect(QString uid)
{
    simple_call *c = simple_call::get_simple_call(QByteArray::fromHex(uid.toLatin1()));
    c-> emit try_connect();

}

void
DwycoCore::create_call_context(QString uid)
{
    simple_call *c = simple_call::get_simple_call(QByteArray::fromHex(uid.toLatin1()));

}

void
DwycoCore::delete_call_context(QString uid)
{
    QByteArray buid = QByteArray::fromHex(uid.toLatin1());
    QList<simple_call *> c = simple_call::Simple_calls.query_by_member(buid, &simple_call::uid);
    simple_call *sc = 0;
    if(c.count() == 0)
    {
        return;
    }
    delete c[0];
}

void
DwycoCore::uid_keyboard_input(QString uid)
{
    QByteArray buid = QByteArray::fromHex(uid.toLatin1());
    QList<simple_call *> c = simple_call::Simple_calls.query_by_member(buid, &simple_call::uid);
    if(c.count() == 0)
        return;
    c[0]->keyboard_input();

}
#endif

void
DwycoCore::bootstrap(QString name, QString email)
{
    QByteArray bname = name.toLatin1();
    QByteArray bemail = email.toLatin1();

    dwyco_create_bootstrap_profile(bname.constData(), bname.length(), "", 0, "mobile user", 11, bemail.constData(), bemail.length());
    dwyco_set_local_auth(1);
    dwyco_finish_startup();
    load_unviewed();
    update_unread_count(total_unviewed_msgs_count());
    reload_conv_list();

    const char *uid;
    int len_uid;
    dwyco_get_my_uid(&uid, &len_uid);
    My_uid = QByteArray(uid, len_uid);
}

void
DwycoCore::delete_file(QString fn)
{
    QFile::remove(fn);
}

static
void
DWYCOCALLCONV
set_profile_done(int succ, const char *reason,
                 const char *s1, int len_s1,
                 const char *s2, int len_s2,
                 const char *s3, int len_s3,
                 const char *filename,
                 const char *uid, int len_uid,
                 int reviewed, int regular,
                 void *arg)
{
    if(!TheDwycoCore)
        return;
    TheDwycoCore->emit profile_update(succ);

}

// this is used to get the image from your existing profile
// so it can be used in an update operation
QString
DwycoCore::uid_to_profile_image_filename(QString uid)
{
    char *ufn = 0;
    int len_ufn = 0;
    QByteArray buid = QByteArray::fromHex(uid.toLatin1());
    int viewid = dwyco_get_profile_to_viewer_sync(buid.constData(), buid.length(), &ufn, &len_ufn);
    if(viewid == 0)
        return QString();
    if(viewid == -2)
    {
        QByteArray ret(ufn, len_ufn);
        dwyco_free_array(ufn);
        return ret;
    }
    if(viewid != -1)
        dwyco_delete_zap_view(viewid);
    return "";

}


// note: this does not remove the source image, in case it was something
// the user sent in, instead of a temp file from the camera.
int
DwycoCore::set_simple_profile(QString handle, QString email, QString desc, QString img_fn)
{
    const char *profile_pack;
    int len_profile_pack;

    QByteArray bhandle = handle.toLatin1();
    QByteArray bemail = email.toLatin1();
    QByteArray bedesc = desc.toLatin1();
    QByteArray fn = img_fn.toLatin1();
    int compid;

    if(fn.length() > 0)
    {
        QByteArray tmpfile;
        if(copy_and_tweak_jpg(fn, tmpfile))
            fn = tmpfile;
        QByteArray out_fn;
        if(!load_and_resize(fn, out_fn, 65000))
            return 0;

        compid = dwyco_make_file_zap_composition(out_fn.constData(), out_fn.length());
    }
    else
    {
        compid = dwyco_make_zap_composition(0);
    }

    if(compid == 0)
        return 0;

    if(!dwyco_make_profile_pack(bhandle.constData(), bhandle.length(), bedesc.constData(), bedesc.length(),
                                "mobile user", 11, bemail.constData(), bemail.length(),
                                &profile_pack, &len_profile_pack))
    {
        return 0;
    }


    if(!dwyco_set_profile_from_composer(compid, profile_pack, len_profile_pack, set_profile_done, 0))
        return 0;
    return 1;

}

void
DwycoCore::set_pal(QString uid, int is_pal)
{
    QByteArray buid = QByteArray::fromHex(uid.toLatin1());
    if(is_pal)
    {
        dwyco_pal_add(buid.constData(), buid.length());
    }
    else
    {
        dwyco_pal_delete(buid.constData(), buid.length());
    }
    emit pal_event(uid);

}

int
DwycoCore::get_pal(QString uid)
{
    QByteArray buid = QByteArray::fromHex(uid.toLatin1());
    return dwyco_is_pal(buid.constData(), buid.length());
}

void
DwycoCore::set_ignore(QString uid, int is_ignored)
{
#if 0
    QByteArray buid = QByteArray::fromHex(uid.toLatin1());
    if(is_ignored)
    {
        dwyco_ignore(buid.constData(), buid.length());
        TheIgnoreListModel->add_uid_to_model(buid);
    }
    else
    {
        dwyco_unignore(buid.constData(), buid.length());
        TheIgnoreListModel->remove_uid_from_model(buid);
    }
    emit ignore_event(uid);
#endif

}

void
DwycoCore::clear_ignore_list()
{
    DWYCO_LIST l;
    int n;

    l = dwyco_ignore_list_get();
    simple_scoped sl(l);
    n = sl.rows();

    for(int i = 0; i < n; ++i)
    {
        QByteArray buid = sl.get<QByteArray>(i);
        dwyco_unignore(buid.constData(), buid.length());
        emit ignore_event(buid.toHex());

    }

    // may involve resorting other lists too
    reload_ignore_list();

}

int
DwycoCore::get_ignore(QString uid)
{
    QByteArray buid = QByteArray::fromHex(uid.toLatin1());
    return dwyco_is_ignored(buid.constData(), buid.length());
}

void
DwycoCore::reset_unviewed_msgs(QString uid)
{
    QByteArray buid = QByteArray::fromHex(uid.toLatin1());
    del_unviewed_uid(buid);
    update_unread_count(total_unviewed_msgs_count());
    emit decorate_user(uid);
    //sort_proxy_model->decorate_user(uid);
}

int
DwycoCore::create_new_account()
{
    return dwyco_get_create_new_account();
}

int
DwycoCore::database_online()
{
    return dwyco_database_online();
}

int
DwycoCore::chat_online()
{
    return dwyco_chat_online();
}

QString
DwycoCore::random_string(int len)
{
    char *rs;
    dwyco_random_string2(&rs, len);
    QByteArray b(rs, len);
    dwyco_free_array(rs);
    b = b.toHex();
    return b;
}

QString
DwycoCore::uid_to_name(QString uid)
{
    QByteArray buid = QByteArray::fromHex(uid.toLatin1());
    return dwyco_info_to_display(buid);
}

bool
DwycoCore::uid_profile_regular(QString uid)
{
    QByteArray buid = QByteArray::fromHex(uid.toLatin1());
    int regular = 0;
    int reviewed = 0;
    get_review_status(buid, reviewed, regular);
    if(regular && reviewed)
        return true;
    return false;
}

QString
DwycoCore::uid_to_profile_info(QString uid, enum Profile_info field)
{
    QByteArray buid = QByteArray::fromHex(uid.toLatin1());
    const char *dwyco_field = "";
    switch(field)
    {
    case HANDLE:
        dwyco_field = DWYCO_INFO_HANDLE;
        break;
    case LOCATION:
        dwyco_field = DWYCO_INFO_LOCATION;
        break;
    case EMAIL:
        dwyco_field = DWYCO_INFO_EMAIL;
        break;
    case DESCRIPTION:
        dwyco_field = DWYCO_INFO_DESCRIPTION;
        break;
#if 0
    case REVIEWED:
        dwyco_field = DWYCO_INFO_REVIEWED;
        break;
    case REGULAR:
        dwyco_field = DWYCO_INFO_REGULAR;
        break;
#endif
    default:
        cdcxpanic("bad field");
    }

    return dwyco_info_to_display2(buid, dwyco_field);
}

QUrl
DwycoCore::uid_to_profile_preview(QString uid)
{

    QString u;
    u = "image://profile_preview/";
    u += uid;
    u += "/";
    u += random_fn();
    QUrl url;
    url.setUrl(u);
    return url;
}

QUrl
DwycoCore::uid_to_http_profile_preview(QString uid)
{
    QString u;

    if(AvoidSSL)
        u = "http://";
    else
        u = "https://";
    u += "profiles.dwyco.org/xinfo/";
    u += uid;
    u += ".jpg";
    QUrl url;
    url.setUrl(u);
    return url;

}

QUrl
DwycoCore::uid_to_profile_view(QString uid)
{
    QString u;
    if(AvoidSSL)
        u = "http://";
    else
        u = "https://";
    u += "profiles.dwyco.org/xinfo/";
    u += uid;
    u += "-full.jpg";
    QUrl url;
    url.setUrl(u);
    return url;
}

int
DwycoCore::set_setting(QString name, QString value)
{
    QByteArray nm = name.toLatin1();
    QByteArray v = value.toLatin1();
    if(dwyco_set_setting(nm.constData(), v.constData()) == 0)
        return 0;
    return 1;
}

QVariant
DwycoCore::get_setting(QString name)
{
    const char *val;
    int len;
    int tp;
    if(dwyco_get_setting(name.toLatin1().constData(), &val, &len, &tp) == 0)
        return QVariant();

    return dwyco_type_to_qv(tp, val, len);

}

void
DwycoCore::set_local_setting(QString name, QString value)
{
    setting_put(name, value);
}

QString
DwycoCore::get_local_setting(QString name)
{
    QString ret;
    if(!setting_get(name, ret))
        return QString();

    return ret;
}

QUrl
DwycoCore::get_simple_directory_url()
{
    QUrlQuery qurl;
    const char *auth;
    int len;

    if(!dwyco_get_authenticator(&auth, &len))
        return QUrl("http://www.dwyco.com");
    QByteArray au(auth, len);
    QByteArray auth_str = au.toPercentEncoding();
    dwyco_free_array((char *)auth);
    QUrl url;

    if(AvoidSSL)
        url.setUrl("http://profiles.dwyco.org/cgi-bin/mksimphtdir.sh");
    else
        url.setUrl("https://profiles.dwyco.org/cgi-bin/mksimphtdir.sh");

    qurl.addQueryItem("uid", My_uid.toHex());
    qurl.addQueryItem("filt", QString::number(1));//QString::number(!ui->filter->isChecked()));
    qurl.addQueryItem("sid", QString::number(0));
#if 0
    if(Current_server != -1)
        qurl.addQueryItem("sid", QString::number(Current_server));
    else if(Current_server_id.length() > 0)
        qurl.addQueryItem("sid", Current_server_id.c_str());
    else
        return;
#endif
    qurl.addQueryItem("auth", auth_str);

    url.setQuery(qurl);
    return url;
}

// convert to LH, since XML stuff is going away in Qt
QUrl
DwycoCore::get_simple_lh_url()
{
#if 0
    QUrlQuery qurl;
    const char *auth;
    int len;

    if(!dwyco_get_authenticator(&auth, &len))
        return QUrl();
    QByteArray au(auth, len);
    QByteArray auth_str = au.toPercentEncoding();
    dwyco_free_array((char *)auth);
    QUrl url;

    if(AvoidSSL)
        url.setUrl("http://profiles.dwyco.org/cgi-bin/mksimplhdir.sh");
    else
        url.setUrl("https://profiles.dwyco.org/cgi-bin/mksimplhdir.sh");
    QString filt = "1";
    QString a;
    if(setting_get("show_unreviewed", a))
    {
        filt = (a == "0") ? "1" : "0";
    }
    qurl.addQueryItem("uid", My_uid.toHex());
    qurl.addQueryItem("filt", filt);//QString::number(!ui->filter->isChecked()));
    qurl.addQueryItem("sid", QString::number(0));
#if 0
    if(Current_server != -1)
        qurl.addQueryItem("sid", QString::number(Current_server));
    else if(Current_server_id.length() > 0)
        qurl.addQueryItem("sid", Current_server_id.c_str());
    else
        return;
#endif
    qurl.addQueryItem("auth", auth_str);

    url.setQuery(qurl);
    return url;
#else
    return QUrl();
#endif
}

QString
DwycoCore::get_msg_count_url(int wants_freebies)
{
#if 1
    QUrlQuery qurl;
    const char *auth;
    int len;

    if(!dwyco_get_authenticator(&auth, &len))
        return "";
    QByteArray au(auth, len);

    dwyco_free_array((char *)auth);
    QUrl url;
    if(AvoidSSL)
        url.setUrl("http://rando.dwyco.com/cgi-bin/webmsgcnt.sh");
    else
        url.setUrl("https://rando.dwyco.com/cgi-bin/webmsgcnt.sh");

    qurl.addQueryItem("uid", QString::fromUtf8(My_uid.toHex()));
    qurl.addQueryItem("auth", QString::fromUtf8(au.toHex()));
    qurl.addQueryItem("freebies", QString::number(wants_freebies));
    qurl.addQueryItem("defeat", QString::number(time(0)));
    url.setQuery(qurl);
    return url.url();
#else
    return "";
#endif
}

int
DwycoCore::delete_message(QString uid, QString mid)
{
    QByteArray buid = uid.toLatin1();
    QByteArray bmid = mid.toLatin1();
    buid = QByteArray::fromHex(buid);
    if(dwyco_get_fav_msg(bmid.constData()))
        return 0;
    return dwyco_delete_saved_message(buid.constData(), buid.length(), bmid.constData());

}

int
DwycoCore::get_fav_message(QString mid)
{
    QByteArray bmid = mid.toLatin1();
    return dwyco_get_fav_msg(bmid.constData());
}

void
DwycoCore::set_fav_message(QString mid, int val)
{
    QByteArray bmid = mid.toLatin1();
    dwyco_set_fav_msg(bmid.constData(), !!val);
    emit mid_tag_changed(mid);
}

int
DwycoCore::has_tag_message(QString mid, QString tag)
{
    QByteArray bmid = mid.toLatin1();
    QByteArray btag = tag.toLatin1();
    return dwyco_mid_has_tag(bmid.constData(), btag.constData());

}
void
DwycoCore::set_tag_message(QString mid, QString tag)
{
    QByteArray bmid = mid.toLatin1();
    QByteArray btag = tag.toLatin1();
    dwyco_set_msg_tag(bmid.constData(), btag.constData());
    emit mid_tag_changed(mid);
}

void
DwycoCore::unset_tag_message(QString mid, QString tag)
{
    QByteArray bmid = mid.toLatin1();
    QByteArray btag = tag.toLatin1();
    dwyco_unset_msg_tag(bmid.constData(), btag.constData());
    emit mid_tag_changed(mid);
}

int
DwycoCore::hash_has_tag(QString hash, QString tag)
{
    QByteArray bhash = hash.toLatin1();
    QByteArray btag = tag.toLatin1();
    DWYCO_LIST tl;
    dwyco_get_tagged_mids(&tl, bhash.constData());
    simple_scoped stl(tl);
    for(int i = 0; i < stl.rows(); ++i)
    {
        QByteArray b = stl.get<QByteArray>(i, DWYCO_TAGGED_MIDS_MID);
        if(dwyco_mid_has_tag(b.constData(), btag.constData()))
            return 1;
    }
    return 0;
}

void
DwycoCore::hash_clear_tag(QString hash, QString tag)
{
    QByteArray bhash = hash.toLatin1();
    QByteArray btag = tag.toLatin1();

    DWYCO_LIST tl;
    dwyco_get_tagged_mids(&tl, bhash.constData());
    simple_scoped stl(tl);
    for(int i = 0; i < stl.rows(); ++i)
    {
        // ugh, need to fill in the API for *tagged_mids
        QByteArray b = stl.get<QByteArray>(i, DWYCO_TAGGED_MIDS_MID);
        dwyco_unset_msg_tag(b.constData(), btag.constData());
        emit mid_tag_changed(b);
        del_unviewed_mid(b);
    }
    update_unseen_from_db();
    mlm->invalidate_sent_to();
}

int
DwycoCore::clear_messages(QString uid)
{
    QByteArray buid = uid.toLatin1();
    buid = QByteArray::fromHex(buid);
    int ret = dwyco_clear_user(buid.constData(), buid.length());
    update_unseen_from_db();
    mlm->invalidate_sent_to();
    return ret;

}

int
DwycoCore::clear_messages_unfav(QString uid)
{
    QByteArray buid = uid.toLatin1();
    buid = QByteArray::fromHex(buid);
    int ret = dwyco_clear_user_unfav(buid.constData(), buid.length());
    update_unseen_from_db();
    mlm->invalidate_sent_to();
    return ret;
}

int
DwycoCore::delete_user(QString uid)
{
    QByteArray buid = uid.toLatin1();
    buid = QByteArray::fromHex(buid);
    int ret = dwyco_delete_user(buid.constData(), buid.length());
    del_unviewed_uid(buid);
    update_unread_count(total_unviewed_msgs_count());
    // note: msglist_model may have cached info that needs to be cleared

    reload_conv_list();
    update_unseen_from_db();
    mlm->invalidate_sent_to();

    return ret;
}

int
DwycoCore::geo_count_from_hash(QString hash)
{
    QByteArray buid = hash.toLatin1();
    buid = QByteArray::fromHex(buid);
    return Hash_to_loc.count(buid);
}

static QByteArray
get_cq_results_filename()
{
    return add_pfx(User_pfx, "cq.xml");

}

#if 0
static void
process_contact_query_response(const QByteArray& mid)
{
    DWYCO_SAVED_MSG_LIST sm;
    if(!dwyco_get_saved_message(&sm, Clbot.constData(), Clbot.length(), mid.constData()))
    {
        TheDwycoCore->emit cq_results_received(0);
        return;
    }
    simple_scoped qsm(sm);

    if(qsm.is_nil(DWYCO_QM_BODY_ATTACHMENT) ||
            qsm.is_nil(DWYCO_QM_BODY_FILE_ATTACHMENT))
    {
        TheDwycoCore->emit cq_results_received(0);
        return;
    }

    QByteArray qrfn = get_cq_results_filename();
    int succ = dwyco_copy_out_file_zap(0, 0, mid.constData(), qrfn.constData());
    TheDwycoCore->emit cq_results_received(succ);
}


void
DwycoCore::internal_cq_check(QString huid)
{
    QByteArray clh = Clbot.toHex();
    if(huid.toLatin1() != clh)
        return;

    if(!dwyco_uid_has_tag(Clbot.constData(), Clbot.length(), "_inbox"))
        return;
    DWYCO_LIST tl;
    dwyco_get_tagged_mids(&tl, "_inbox");
    simple_scoped qtl(tl);
    for(int i = 0; i < qtl.rows(); ++i)
    {
        if(qtl.get<QByteArray>(i, DWYCO_TAGGED_MIDS_HEX_UID) == clh)
        {
            QByteArray mid = qtl.get<QByteArray>(i, DWYCO_TAGGED_MIDS_MID);
            process_contact_query_response(mid);
            dwyco_delete_unfetched_message(mid.constData());

        }
    }
    dwyco_delete_user(Clbot.constData(), Clbot.length());
}


QUrl
DwycoCore::get_cq_results_url()
{
    return QUrl(add_pfx("file://", get_cq_results_filename()));
}

void
DwycoCore::delete_cq_results()
{
    delete_file(get_cq_results_filename());
}
#endif

int
DwycoCore::retry_auto_fetch(QString mid)
{
    QByteArray bmid = mid.toLatin1();
    return ::retry_auto_fetch(bmid);
}

// this is a bit of a hack, we know that the location and other
// non-picture messages are small, so we fetch them automatically.
// the larger picture messages are not fetched until the user
// views the model they are in...
static
void
fetch_small_msgs()
{
    DWYCO_LIST uml;

    if(dwyco_get_unfetched_messages(&uml, 0, 0))
    {
        simple_scoped quml(uml);
        int n = quml.rows();

        if(n == 0)
        {
            return;
        }

        for(int i = 0; i < n; ++i)
        {
            if(quml.is_nil(i, DWYCO_QMS_IS_DIRECT))
            {
                long len = quml.get_long(i, DWYCO_QMS_LEN);
                if(len < 5000)
                {
                    QByteArray mid = quml.get<QByteArray>(i, DWYCO_QMS_ID);
                    auto_fetch(mid);
                }
            }
        }
    }

}

int
DwycoCore::service_channels()
{
    int spin = 0;
    if(Suspended || !Init_ok)
        return 0;
    dwyco_service_channels(&spin);
    if(dwyco_get_rescan_messages())
    {
        dwyco_set_rescan_messages(0);
        DWYCO_UNFETCHED_MSG_LIST uml;

        //scan_special_msgs();
        fetch_small_msgs();
        dwyco_get_unfetched_messages(&uml, 0, 0);
        // just save all the direct messages, since it is relatively cheap
        QSet<QByteArray> uids_out;
        dwyco_process_unfetched_list(uml, uids_out);
        dwyco_list_release(uml);

        load_inbox_tags_to_unviewed(uids_out);

        foreach(const QByteArray& buid, uids_out)
        {
            QByteArray huid = buid.toHex();
            emit new_msg(QString(huid), "", "");
            emit decorate_user(huid);
        }
        update_unseen_from_db();
        mlm->invalidate_sent_to();
    }
    update_unread_count(total_unviewed_msgs_count());
#ifdef ANDROID
    // NOTE: bug: this doesn't work if the android version is statically
    // linked. discovered why: JNI won't find functions properly when statically linked.
    const char *fn;
    int len_fn;
    if(dwyco_get_aux_string(&fn, &len_fn))
    {
        if(len_fn > 0)
        {
            QString fns = QString::fromUtf8(QByteArray(fn, len_fn));
            emit image_picked(fns);
            dwyco_set_aux_string("");
        }
        dwyco_free_array((char *)fn);
    }
#endif


    return spin;
}


int
DwycoCore::send_forward(QString recipient, QString add_text, QString uid_folder, QString mid_to_forward, int save_sent)
{
    QByteArray uid_f = QByteArray::fromHex(uid_folder.toLatin1());
    QByteArray bmid = mid_to_forward.toLatin1();
    int compid = dwyco_make_forward_zap_composition2(bmid.constData(), 1);
    if(compid == 0)
        return 0;
    QByteArray ruid = QByteArray::fromHex(recipient.toLatin1());
    QByteArray atext = add_text.toUtf8();
    if(!dwyco_zap_send5(compid, ruid.constData(), ruid.length(),
                        atext.constData(), atext.length(), 0, save_sent,
                        0, 0)
      )

    {
        dwyco_delete_zap_composition(compid);
        return 0;
    }
    return compid;

}

int
DwycoCore::flim(QString uid_folder, QString mid_to_forward)
{
    QByteArray uid_f = QByteArray::fromHex(uid_folder.toLatin1());
    QByteArray bmid = mid_to_forward.toLatin1();
    int compid = dwyco_make_forward_zap_composition2(bmid.constData(), 1);
    int ret = dwyco_flim(compid);

    dwyco_delete_zap_composition(compid);
    return ret;

}

int
DwycoCore::simple_send(QString recipient, QString msg)
{
    int compid = dwyco_make_zap_composition(0);
    if(compid == 0)
        return 0;
    QByteArray ruid = QByteArray::fromHex(recipient.toLatin1());
    QByteArray txt = msg.toUtf8();
    if(!dwyco_zap_send5(compid, ruid.constData(), ruid.length(),
                        txt.constData(), txt.length(), 0, 1,
                        0, 0)
      )

    {
        dwyco_delete_zap_composition(compid);
        return 0;
    }
    return compid;
}

int
DwycoCore::simple_send_file(QString recipient, QString msg, QString filename)
{
    QByteArray ruid = QByteArray::fromHex(recipient.toLatin1());
    QByteArray txt = msg.toUtf8();
    QByteArray fn = filename.toLatin1();

    int compid = dwyco_make_file_zap_composition(fn.constData(), fn.length());
    if(compid == 0)
        return 0;
    if(!dwyco_zap_send5(compid, ruid.constData(), ruid.length(),
                        txt.constData(), txt.length(), 0, 1,
                        0, 0)
      )

    {
        dwyco_delete_zap_composition(compid);
        return 0;
    }
    return compid;
}

template<class T>
void
save_it(const T& d, const char *filename)
{
    QFile f(filename);
    f.open(QIODevice::WriteOnly);
    QDataStream out(&f);
    out << d;
    f.close();
}


void
send_contact_query(QList<QString> emails)
{
    QByteArray fn = add_pfx(Tmp_pfx, random_fn());
    save_it(emails, fn.constData());
    int compid = dwyco_make_file_zap_composition(fn.constData(), fn.length());
    if(compid == 0)
        return;
    QByteArray clbot(QByteArray::fromHex("f6006af180260669eafc"));

    if(!dwyco_zap_send5(compid, clbot.constData(), clbot.length(),
                        "", 0,
                        0, 0,
                        0, 0)
      )

    {
        dwyco_delete_zap_composition(compid);
    }

    QFile::remove(fn);
}

int
DwycoCore::simple_send_url(QString recipient, QString msg, QUrl filename)
{
    if(!filename.isLocalFile())
        return 0;
    QString fn = filename.toLocalFile();

    // stopgap... if it is a jpg file, we'll make a copy and
    // strip out exif.
    QByteArray tmpfile;
    if(copy_and_tweak_jpg(fn, tmpfile))
        fn = tmpfile;
    return simple_send_file(recipient, msg, fn);

}

QString
DwycoCore::url_to_filename(QUrl u)
{
    if(!u.isLocalFile())
        return "";
    return u.toLocalFile();
}



// this just creates a uniq name for the file, and removes the original
// by default, since we don't want a copy of the image laying around
// in the camera folder. also, strip out the exif information in the file
int
DwycoCore::send_simple_cam_pic(QString recipient, QString msg, QString filename)
{
    QByteArray ruid = QByteArray::fromHex(recipient.toLatin1());
    QByteArray txt = msg.toUtf8();
    QByteArray fn = filename.toLatin1();

    // if for some reason we can't strip out the exit stuff, then
    // don't send the image. it could be that it isn't a jpg file or something
    // but this is the safest thing to do
    int rot = jhead::do_jhead(fn.constData());
    if(rot & 1)
    {
        // should we delete file too?
        return 0;
    }
    jhead_rotate(filename, rot >> 1);
    QFile f(filename);
    //QFileInfo fi(filename);
    char *rs;
    dwyco_random_string2(&rs, 4);
    QByteArray rsb(rs, 4);
    dwyco_free_array(rs);
    rsb = rsb.toHex();
    QByteArray dest;
    dest += rsb;
    dest += ".jpg";
    dest = add_pfx(Tmp_pfx, dest);
    f.copy(dest);
    f.remove();

    QByteArray revhelp;
    {
    QFile df(dest);
    if(!df.open(QFile::ReadOnly))
        return 0;
    char buf[4096];
    int len = df.read(buf, sizeof(buf));
    QCryptographicHash ch(QCryptographicHash::Sha1);
    QByteArrayView b(buf, len);
    ch.addData(b);

    QByteArray res = ch.result();
    res = res.toHex();
    revhelp = QString("{\"hash\" : \"%1\", \"review\" : \"nope\"}").arg(QString(res)).toLatin1();
    }
    int compid = dwyco_make_file_zap_composition(dest.constData(), dest.length());
    if(compid == 0)
    {
        QFile::remove(dest);
        return 0;
    }
    if(!dwyco_zap_send5(compid, ruid.constData(), ruid.length(),
                        revhelp.constData(), revhelp.length(), 0, 1,
                        0, 0)
      )

    {
        dwyco_delete_zap_composition(compid);
        QFile::remove(dest);
        return 0;
    }
    QFile::remove(dest);
    return compid;
}

int
DwycoCore::make_zap_view(QString uid, QString mid)
{
    QByteArray ruid = QByteArray::fromHex(uid.toLatin1());
    QByteArray rmid = mid.toLatin1();

    DWYCO_SAVED_MSG_LIST sm;
    if(!dwyco_get_saved_message(&sm, ruid.constData(), ruid.length(), rmid.constData()))
    {
        return 0;
    }

    int view_id = dwyco_make_zap_view2(sm, 0);

    dwyco_list_release(sm);
    return view_id;
}

int
DwycoCore::make_zap_view_file(QString fn)
{
    QByteArray bfn = fn.toLatin1();

    int view_id = dwyco_make_zap_view_file_raw(bfn.constData());


    return view_id;
}

#ifdef ANDROID
QString
DwycoCore::export_attachment(QString mid)
{
    QByteArray rmid = mid.toLatin1();
    //QString userdir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QByteArray userdir;
    // note: on android, we export the attachment to a writable sub-directory called
    // "shares", and the java android support stuff has a "fileprovider" that
    // can share items from that directory via mediastore. it is the caller's
    // responsibility to initiate the notification to mediastore to pick up the new image.
    // on desktop, we don't need to do all this garbage, just write it to documents or
    // pictures and we're done.
    userdir = add_pfx(User_pfx, "shares");
    // using the name in the message is dicey. it might be utf8, might be unicode,
    // might be from case-sensitive fs, might not. almost certainly it is a security problem.
    // so punt, and just create a random filename, and try to add a file extension if
    // it looks like there is one.
    DWYCO_SAVED_MSG_LIST sm;
    if(dwyco_get_saved_message3(&sm, rmid.constData()) != DWYCO_GSM_SUCCESS)
    {
        return "";
    }
    simple_scoped qsm(sm);
    if(qsm.is_nil(DWYCO_QM_BODY_FILE_ATTACHMENT))
        return "";
    QByteArray scary_fn = qsm.get<QByteArray>(DWYCO_QM_BODY_FILE_ATTACHMENT);
    quint16 csum = qChecksum(QByteArrayView(scary_fn));
    // look for file extension
    int dot = scary_fn.lastIndexOf('.');
    if(dot != -1)
    {
        scary_fn.remove(0, dot);
    }
    else
        scary_fn = "";
    // maybe try sticking a place name in here? just
    // an idea
    QByteArray dstfn("/rando");
    dstfn += QByteArray::number(csum, 16);
    dstfn += scary_fn;
    userdir += dstfn;
    // note: this is probably broken on windows, have to check it out.
    QByteArray lfn = QFile::encodeName(userdir);
    if(!dwyco_copy_out_file_zap2(rmid.constData(), lfn.constData()))
    {
        return "";
    }
    return QFile::decodeName(lfn);
}
#else
QString
DwycoCore::export_attachment(QString mid)
{
    QByteArray rmid = mid.toLatin1();
    QString userdir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    // using the name in the message is dicey. it might be utf8, might be unicode,
    // might be from case-sensitive fs, might not. almost certainly it is a security problem.
    // so punt, and just create a random filename, and try to add a file extension if
    // it looks like there is one.
    DWYCO_SAVED_MSG_LIST sm;
    if(dwyco_get_saved_message3(&sm, rmid.constData()) != DWYCO_GSM_SUCCESS)
    {
        return "";
    }
    simple_scoped qsm(sm);
    if(qsm.is_nil(DWYCO_QM_BODY_FILE_ATTACHMENT))
        return "";
    QByteArray scary_fn = qsm.get<QByteArray>(DWYCO_QM_BODY_FILE_ATTACHMENT);
    quint16 csum = qChecksum(scary_fn.constData(), scary_fn.length());
    // look for file extension
    int dot = scary_fn.lastIndexOf('.');
    if(dot != -1)
    {
        scary_fn.remove(0, dot);
    }
    else
        scary_fn = "";
    QByteArray dstfn("/rando");
    dstfn += QByteArray::number(csum, 16);
    dstfn += scary_fn;
    userdir += dstfn;
    // note: this is probably broken on windows, have to check it out.
    QByteArray lfn = QFile::encodeName(userdir);
    if(!dwyco_copy_out_file_zap2(rmid.constData(), lfn.constData()))
    {
        return "";
    }
    return QFile::decodeName(lfn);
}
#endif

static
void
DWYCOCALLCONV
dwyco_done_view(int chan_id, void *user_arg)
{
    // emit some done signal here
}

int
DwycoCore::play_zap_view(int view_id)
{
    int ui_id = 0;
    if(!dwyco_zap_play_view(view_id, dwyco_done_view, 0, &ui_id))
        return 0;
    return ui_id;
}


void
dwyco_register_qml(QQmlContext *root)
{
    setup_locations();
    TheRootCtx = root;
    qmlRegisterType<DwycoCore>("dwyco", 1, 0, "DwycoCore");
    qmlRegisterType<msglist_model>("dwyco", 1, 0, "DwycoMsgList");
    //qmlRegisterType<SimpleUserSortFilterModel>("dwyco", 1, 0, "DwycoSimpleUserModel");
   // qmlRegisterType<SimpleContactModel>("dwyco", 1, 0, "DwycoSimpleContactModel");
    //qmlRegisterType<FauxButton>("dwyco", 1, 0, "FauxButton");
    //qmlRegisterType<iglist_model>("dwyco", 1, 0, "DwycoIgnoreList");
    //qmlRegisterType<codel>("dwyco", 1, 0, "ChatListModel");
    //Dwyco_video_provider = new DwycoImageProvider;
    //root->engine()->addImageProvider("dwyco_video_frame", Dwyco_video_provider);
    //root->engine()->addImageProvider("profile_preview", new DwycoProfilePreviewProvider);
    //Dwyco_video_preview_provider = new DwycoVideoPreviewProvider;
    //root->engine()->addImageProvider("dwyco_video_preview", Dwyco_video_preview_provider);

    CamListModel = new QQmlVariantListModel;
    root->setContextProperty("camListModel", CamListModel);

    // current users in the chat server, updated dynamically
    //ChatListModel * clm = new ChatListModel;
    //Chat_sort_proxy = new ChatSortFilterModel;
    //Chat_sort_proxy->setSourceModel(clm);
    //root->setContextProperty("ChatListModel", Chat_sort_proxy);

    ConvListModel *convlist = new ConvListModel;
    Conv_sort_proxy = new ConvSortFilterModel;
    //Conv_sort_proxy->setSourceModel(convlist);
    //QObject::connect(convlist, SIGNAL(countChanged()), Conv_sort_proxy, SIGNAL(countChanged()));
    root->setContextProperty("ConvListModel", convlist);

    GeoSprayListModel *gs = new GeoSprayListModel;
    root->setContextProperty("GeoSprayListModel", gs);

    //IgnoreListModel *ignorelist = new IgnoreListModel;
    //Ignore_sort_proxy = new IgnoreSortFilterModel;
    //Ignore_sort_proxy->setSourceModel(ignorelist);
    //QObject::connect(ignorelist, SIGNAL(countChanged()), Ignore_sort_proxy, SIGNAL(countChanged()));
    //root->setContextProperty("IgnoreListModel", Ignore_sort_proxy);

    AndroidPerms *a = new AndroidPerms;
    root->setContextProperty("AndroidPerms", a);

}
