
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QMessageBox>
#include <QTextCursor>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QDateTime>
#include <QStringList>
#include <QDir>
#include <QTextDocumentFragment>
#include <QFileDialog>
#include <QSettings>
#include <QScrollBar>
#include <QDesktopServices>
//#include <QDesktopWidget>
#include <QVector>
#include <QVariant>
#include <QList>
#include <QHash>
#include <QSet>
#include <QTabBar>
#include <QTextFrame>
#include <QQueue>
#include <QThread>
#include <QProgressDialog>
#include <QWaitCondition>
#include <QMutex>
#include <QMutexLocker>
#include <QPainter>
#include <QPainterPath>

#include <time.h>
#include "ui_mainwin.h"
#include "mainwin.h"
#include "dlli.h"
#include "dwyco_new_msg.h"
#include "about.h"
#include "vidsel.h"
#include "autoupdate.h"
#include "composer.h"
#include "msgtohtml.h"
#include "tfhex.h"
#include "adminw.h"
//#include "fetchpw.h"
#include "config.h"
#include "snd.h"
//#include "entercode.h"
#include "login.h"
#include "ssmap.h"
#include "pw.h"
#include "qwiz.h"
#include "cspw.h"
#include "iglist.h"
#include "ct.h"
#include "userlob.h"
#include "simple_public.h"
#include "lobbybox3.h"
#include "vidbox.h"
#include "ui_vidbox.h"
#include "croom.h"
#include "vidlab.h"
#include "browsebox.h"
#include "msgbrowsedock.h"
#include "profilepreviewbox.h"
#include "simpledirbox.h"
#include "simple_call.h"
#include "ui_simple_call.h"
#include "player.h"
#include "prfview.h"
#include "profpv.h"


DWYCO_LIST Model_dir;
mainwinform *Mainwinform;
VidSel *DevSelectBox;
QList<QAction *> *AudioActionGroup;

DwOString dwyco_get_attr(DWYCO_LIST l, int row, const char *col);
int block_enable_video_capture_preview(int on, int *ui_id);

extern DwOString ZeroUID;
extern DwOString My_uid;
extern int ClientGod;
int DieDieDie;
static int DB_logged_in;
extern DWYCO_SERVER_LIST Dwyco_server_list;
int Block_DLL;
int Initial_chat_mute;
extern DwOString TheManUID;
int HasAudioInput;
int HasAudioOutput;
extern int HasCamera;
extern int HasCamHardware;
static int Initial_login_processing = 1;
int DoWizard;
extern int FirstRun;
extern int Current_server;
extern DwOString Current_server_id;
extern int Last_server;
extern DwOString Last_server_id;
extern DwOString Last_selected_id;
extern int Last_selected_idx;
extern int Askup;
static int Ask_lobby_pw;
static DwOString Ask_lobby_pw_id;
static DwOString Current_lobby_pw;
static QHash<DwOString, DwOString> Animate_uids;
static QList<DwOString> Reset_animations_uids;
static QHash<DwOString, font_animation_timer *> Animate_uids_font;

typedef QMultiHash<DwOString, DwOString> UID_ATTR_MAP;
typedef QMultiHash<DwOString, DwOString>::iterator UID_ATTR_MAP_ITER;

static UID_ATTR_MAP Uid_attrs;
QList<user_lobby> User_lobbies;
int Display_all_uid;
int Display_chat_users;
int Display_pics_in_user_list;
int Display_archived_users;
int User_count;
int Allow_multiple_windows;
extern DwOString Version;
QImage Last_preview_image;
int Public_chat_video_pause;
extern int First25;
extern int First214;
static int Pal_relogin;

QHash<QByteArray, struct godrec> Gods;
static QSet<QByteArray> Recent_uid_operations;
QSet<DwOString> Visible_to;
QSet<DwOString> Session_remove;
QQueue<DwOString> Seen_in_chat;

class InitCleanThread : public QThread
{
public:
    void run() {
        dwyco_power_clean_safe();
    }
};

static int Refresh_users;

void
cdcx_set_refresh_users(int i)
{
    Refresh_users = i;
}
int
cdcx_get_refresh_users()
{
    return Refresh_users;
}

uint qHash(const DwOString& a)
{
    return a.hashValue();
}

void
add_seen_in_chat(const DwOString& uid)
{
    if(Seen_in_chat.contains(uid))
        return;
    Seen_in_chat.append(uid);

    while(Seen_in_chat.count() > 30)
    {
        Seen_in_chat.removeFirst();
    }
}

font_animation_timer::font_animation_timer(const DwOString& auid, int which) :
    uid(auid)
{

    Animate_uids_font.insert(uid, this);
    this->which = which;
    connect(&timer, SIGNAL(timeout()), this, SLOT(done()));
    timer.setInterval(5000);
    timer.setSingleShot(1);
    timer.start();
}

void
font_animation_timer::done()
{
    if(Animate_uids_font.contains(uid))
    {
        Animate_uids_font.remove(uid);
        Reset_animations_uids.append(uid);
    }
    deleteLater();
}

void
start_font_animation(DwOString& uid, int which)
{
    if(Animate_uids_font.contains(uid))
    {
        font_animation_timer *f = Animate_uids_font.value(uid);
        delete f;

    }
    Animate_uids_font.insert(uid, new font_animation_timer(uid, which));
}


static int
recent_uid_operation(const DwOString& uid)
{
    if(Recent_uid_operations.contains(QByteArray(uid.c_str(), uid.length())))
        return 1;
    return 0;
}

#ifdef LEAK_CLEANUP
void uid_attrs_clear();
void
mainwin_leak_cleanup()
{
    uid_attrs_clear();
    User_lobbies.clear();
    Animate_uids.clear();
    Reset_animations_uids.clear();
    Ask_lobby_pw_id = "";
    Current_lobby_pw = "";
}
#endif


int
confirm_ignore()
{
    int val = 1;
    if(setting_get("ignore-confirm", val) == 0)
    {
        setting_put("ignore-confirm", 1);
        settings_save();
    }
    if(!val)
        return 1;
    QMessageBox msgBox(QMessageBox::Question, "Really block user(s)?", "Blocking/ignoring a user means you will not be able to contact them,"
                       " and they will not be able to contact or see you. Are you sure you want to do this? (if you want to unblock them later, "
                       "use Setup|show ignore list...)", QMessageBox::NoButton, Mainwinform, Qt::Dialog);
    QPushButton *yes = msgBox.addButton("Yes", QMessageBox::AcceptRole);
    QPushButton *no = msgBox.addButton("No", QMessageBox::RejectRole);
    QPushButton *yesalways = msgBox.addButton("Yes, and don't ask again.", QMessageBox::YesRole);

    msgBox.exec();

    if (msgBox.clickedButton() == yes) {
        return 1;
    } else if (msgBox.clickedButton() == no) {
        return 0;
    }
    else if(msgBox.clickedButton() == yesalways)
    {
        setting_put("ignore-confirm", 0);
        settings_save();
        return 1;
    }
    return 0;
}

QString
strip_html(const QString& txt)
{
    QTextDocumentFragment f = QTextDocumentFragment::fromHtml(txt);
    QString ret = f.toPlainText();
    return ret;
}

void
uid_attrs_clear()
{
    Uid_attrs.clear();
}

void
uid_attrs_add(const DwOString& uid, const DwOString& attr)
{
    Uid_attrs.insert(uid, attr);
}

void
uid_attrs_del(const DwOString& uid, const DwOString& attr)
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
uid_attrs_has_attr(const DwOString& uid, const DwOString& attr)
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
uid_attrs_del_uid(const DwOString& uid)
{
    Uid_attrs.remove(uid);
}

int
uid_in_lobby(const DwOString& uid)
{
    return Uid_attrs.contains(uid);
}

int
uid_active(const DwOString& uid)
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

QList<DwOString>
uid_attrs_get_uids()
{
    return Uid_attrs.uniqueKeys();
}

void
user_lobbies_clear()
{
    User_lobbies.clear();

}

void
user_lobbies_del(const char *id)
{
    user_lobby d;
    d.id = id;
    User_lobbies.removeOne(d);

}

void
user_lobbies_add(const user_lobby& ul)
{
    user_lobbies_del(ul.id.c_str());
    User_lobbies.append(ul);
}

void
start_animation(const DwOString& uid, const DwOString& icon)
{
    Animate_uids.insert(uid, icon);
}

void
stop_animation(const DwOString& uid)
{
    if(Animate_uids.contains(uid))
    {
        Animate_uids.remove(uid);
        Reset_animations_uids.append(uid);
    }
}

static int
is_streaming_uid(const DwOString& uid)
{
    QList<simple_call *> sc = simple_call::Simple_calls.query_by_member(uid, &simple_call::uid);
    if(sc.count() != 1)
        return 0;
    if(sc[0]->can_stream())
        return 1;
    int sv;
    int rv;
    if(!dwyco_channel_streams(sc[0]->chan_id, &sv, &rv, 0, 0, 0, 0))
        return 0;
    if(sv || rv)
        return 1;
    return 0;
}


void
DWYCOCALLCONV
dwyco_debug_callback(int, const char *msg, int, void *)
{
    printf( "%s\n", msg);
    fflush(stdout);
}

DwOString
random_fn()
{
    char *r;
    dwyco_random_string2(&r, 10);
    DwOString f(r, 0, 10);
    f = to_hex(f);
    dwyco_free_array(r);
    return f;
}

int
is_my_uid(const char *uid, int len_uid)
{
    if(DwOString(uid, 0, len_uid) == My_uid)
        return 1;
    return 0;
}

DwOString
computer_gen_pw()
{
    // create a random pw
    char *rs;
    dwyco_random_string2(&rs, 10);
    DwOString a(rs, 0, 10);
    a = to_hex(a);
    dwyco_free_array(rs);
    return a;
}

void
save_low_sec(const DwOString& a)
{
    char *e;
    int elen;
    dwyco_eze2(a.c_str(), a.length(), &e, &elen);
    DwOString ea = DwOString(e, 0, elen);
    dwyco_free_array(e);
    setting_put("server-pw", ea);
    setting_put("mode", DwOString("0"));
    settings_save();
}

void
save_high_sec(const DwOString& a)
{
    char *salt;
    int salt_len = 0;
    char *hash;
    int len_hash;
    dwyco_gen_pass(a.c_str(), a.length(), &salt, &salt_len,
                   &hash, &len_hash);
    DwOString hs(hash, 0, len_hash);
    setting_put("server-pw", hs);
    setting_put("salt", DwOString(salt, 0, salt_len));
    setting_put("mode", DwOString("1"));
    settings_save();
    dwyco_free_array(salt);
    dwyco_free_array(hash);
}

DwOString
get_low_sec()
{
    char *e;
    int elen;
    DwOString a;
    DwOString mode;
    if(!setting_get("server-pw", a) ||
            !setting_get("mode", mode) ||
            !mode.eq("0"))
        return "";
    dwyco_ezd2(a.c_str(), a.length(), &e, &elen);
    DwOString tmp = DwOString(e, 0, elen);
    dwyco_free_array(e);
    return tmp;
}

void
DWYCOCALLCONV
dwyco_db_login_result(const char *str, int what)
{
    DB_logged_in = !!what;
    if(what == 0)
    {
        DwOString err("Can't log into server, try again later: ");
        err += str;
        QMessageBox::critical(0, "Critical Error", err.c_str());
        Mainwinform->db_status->setText("<font color=\"#00FF00\">Not logged in</font>");
    }
    else
    {
        if(Mainwinform)
        {
            Mainwinform->ui.login_button->hide();
            Mainwinform->db_status->setText("<font color=\"#00FF00\">Logged in</font>");
            // authenticator, uid and so on should be ok so we can
            // call the web server and get a profile page
            int val = 0;
            if(!setting_get("profiles_no_auto_load", val) || val == 0)
                Mainwinform->load_browsebox();
        }
#if 0
        // pal auth state comes in from the server
        int pa = dwyco_get_my_pal_auth_state();
        TheConfigForm->ui.pal_auth->setCheckState(pa ? Qt::Checked : Qt::Unchecked);
#endif


#if 0
        // note: don't goof with the local password based on what the server
        // returns. the server doesn't give us this indication anyways anymore
        // as of 2.9
        if(what == 2)
        {
            if(!TheLoginForm)
                return;
            // new account created, we have to update the settings to
            // reflect the new passwords and stuff we created.
            loginform& lf = *TheLoginForm;

            if(lf.save_pw())
            {
                // low security mode, just save the computer generated
                // password.
                save_low_sec(lf.gen_pw);
            }
            else
            {
                save_high_sec(lf.pw());
            }

            delete TheLoginForm;
            TheLoginForm = 0;
        }
#endif
    }
}


int
set_current_server(int i)
{
    if(i == Current_server)
        return 1;
    static int been_here;
    int auto_login = 1;

    if(been_here || (!been_here && auto_login))
        dwyco_switch_to_chat_server(i);
    if(!been_here)
    {
        been_here = 1;
    }
    Current_server = i;
    Current_server_id = "";
    Last_selected_id = "";
    Last_selected_idx = i;
    setting_put("server", i);
    setting_put("server_id", DwOString(""));
    settings_save();

    const char *out;
    int out_len;
    int out_type;
    int n;
    dwyco_list_numelems(Dwyco_server_list, &n, 0);
    if(i >= n)
    {
        i = 0;
        setting_put("server", i);
        settings_save();
    }
    if(dwyco_list_get(Dwyco_server_list, i, DWYCO_SL_SERVER_NAME,
                      &out, &out_len, &out_type) == 0)
        return 1;
    if(out_type == DWYCO_TYPE_STRING)
    {
        if(Mainwinform)
            Mainwinform->emit new_server_name(DwOString(out, 0, out_len));
    }
#ifdef CDCX_WEBKIT
    if(Mainwinform)
    {
        int val = 0;
        if(!setting_get("profiles_no_auto_load", val) || val == 0)
        {
            Mainwinform->load_browsebox();
            Mainwinform->dir_listing->reload_triggered();
        }
        else
        {
            Mainwinform->browse_box->set_to_default_url();
            Mainwinform->dir_listing->set_to_default_url();
        }
    }
#endif
    return 1;


#if 0
// might want some user intervention at this point, since there
// may be transient problems that could cause this.

    // possibly the index is wrong and the servers2 file
    // has changed so the index is out of bounds
    // *or* we are not authenticated
    // *or* there is a chat thread already going
    setting_put("server", DwOString("0"));
    Current_server = 0;
#endif

}

int
set_current_server(const DwOString& id)
{
    if(id == Current_server_id)
        return 1;
    static int been_here;
    int auto_login = 1;

    if(been_here || (!been_here && auto_login))
    {
        int rc;
        if((rc = dwyco_switch_to_chat_server2(id.c_str(), Current_lobby_pw.c_str())) == -1)
        {
            Ask_lobby_pw = 1;
            Ask_lobby_pw_id = id;
            return 0;
        }
        if(rc == 0)
        {
            return 0;
        }
    }
    if(!been_here)
    {
        been_here = 1;
    }
    Current_server = -1;
    Current_server_id = id;
    Last_selected_id = id;
    Last_selected_idx = -1;
    // by avoiding setting the server to -1, we can make sure
    // we log into a system server
    //setting_put("server", -1);
    setting_put("server_id", Current_server_id);
    settings_save();
    DWYCO_LIST snl;

    if(dwyco_get_lobby_name_by_id2(id.c_str(), &snl))
    {
        const char *sn;
        int len;
        int tp;
        dwyco_list_get(snl, 0, DWYCO_NO_COLUMN, &sn, &len, &tp);
        if(Mainwinform)
        {
            DwOString a(sn, 0, len);
            Mainwinform->emit new_server_name(a);
        }
        dwyco_list_release(snl);
    }
#ifdef CDCX_WEBKIT
    if(Mainwinform)
    {
        int val = 0;
        if(!setting_get("profiles_no_auto_load", val) || val == 0)
        {
            Mainwinform->load_browsebox();
            Mainwinform->dir_listing->reload_triggered();
        }
        else
        {
            Mainwinform->browse_box->set_to_default_url();
            Mainwinform->dir_listing->set_to_default_url();
        }
    }
#endif
    return 1;

#if 0
// might want some user intervention at this point, since there
// may be transient problems that could cause this.

    // possibly the index is wrong and the servers2 file
    // has changed so the index is out of bounds
    // *or* we are not authenticated
    // *or* there is a chat thread already going
    setting_put("server", DwOString("0"));
    Current_server = 0;
#endif

}

static int
less_than(const DwOString& u1, const DwOString& u2)
{
    int o1;
    int o2;
#if 0
    o1 = is_streaming_uid(u1);
    o2 = is_streaming_uid(u2);

    if(o1 && !o2)
        return -1;
    else if(!o1 && o2)
        return 1;
#endif

    o1 = any_unread_msg(u1);
    o2 = any_unread_msg(u2);

    if(o1 && !o2)
        return -1;
    else if(!o1 && o2)
        return 1;

#if 0
    // ignored users are greater than anyone else, so they always
    // end up at the bottom, which is why the test looks backwards
    o1 = dwyco_is_ignored(u1.c_str(), u1.length());
    o2 = dwyco_is_ignored(u2.c_str(), u2.length());

    if(o1 && !o2)
        return 1;
    else if(!o1 && o2)
        return -1;
    // end backwards looking
#endif

    o1 = recent_uid_operation(u1);
    o2 = recent_uid_operation(u2);

    if(o1 && !o2)
        return -1;
    else if(!o1 && o2)
        return 1;

    o1 = dwyco_uid_online(u1.c_str(), u1.length()) || (uid_in_lobby(u1) || Animate_uids_font.contains(u1));
    o2 = dwyco_uid_online(u2.c_str(), u2.length()) || (uid_in_lobby(u2) || Animate_uids_font.contains(u2));

    if(o1 && !o2)
        return -1;
    else if(!o1 && o2)
        return 1;

    o1 = dwyco_is_pal(u1.c_str(), u1.length());
    o2 = dwyco_is_pal(u2.c_str(), u2.length());

    if(o1 && !o2)
        return -1;
    else if(!o1 && o2)
        return 1;

    o1 = session_msg(u1);
    o2 = session_msg(u2);

    if(o1 && !o2)
        return -1;
    else if(!o1 && o2)
        return 1;
#if 0
    o1 = dwyco_is_always_visible(u1.c_str(), u1.length());
    o2 = dwyco_is_always_visible(u2.c_str(), u2.length());

    if(o1 && !o2)
        return -1;
    else if(!o1 && o2)
        return 1;
#endif


#if 0
    // never visibles are put near the bottom (always greater)
    o1 = dwyco_is_never_visible(u1.c_str(), u1.length());
    o2 = dwyco_is_never_visible(u2.c_str(), u2.length());

    if(o1 && !o2)
        return 1;
    else if(!o1 && o2)
        return -1;
#endif
    return 0;
}


bool
PalSortFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    QAbstractItemModel *m = sourceModel();
#if 0
    printf("lt %s (%d, %d) %s (%d, %d)\n",
           m->data(left).toString().toAscii().constData(), left.row(), left.column(),
           m->data(right).toString().toAscii().constData(), right.row(), right.column());
#endif


    QByteArray quid1 = m->data(left, Qt::UserRole).toByteArray();
    QByteArray quid2 = m->data(right, Qt::UserRole).toByteArray();

    if(quid1.length() == 0 || quid2.length() == 0)
        return 0;

    int lt = less_than(DwOString(quid1.constData(), 0, quid1.count()), DwOString(quid2.constData(), 0, quid2.count()));

    if(lt == 0)
    {
        int ret1;
        int ret2;
        ret1 =  QSortFilterProxyModel::lessThan(left, right);
        ret2 =  QSortFilterProxyModel::lessThan(right, left);

        if(ret1 == 0 && ret2 == 0)
        {
            // this should stabilize the sort so users that
            // names look the same at least always sort to the
            // same relative place (before, they would just
            // randomly rearrange and be a pain in the butt to figure out.)
            if(quid1 < quid2)
                return 1;
            return 0;
        }
        return ret1;
    }
    return lt < 0;
}


mainwinform::mainwinform(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags), sort_proxy_model(this), vp(this)
{
    ui.setupUi(this);
    Mainwinform = this;
    AudioActionGroup = new QList<QAction *>;
    dwyco_get_audio_hw(&HasAudioInput, &HasAudioOutput, 0);

    ui.toolBar->hide();

    popup_menu = new QMenu(this);
    popup_menu->insertAction(0, ui.actionAccept_call);
    popup_menu->insertAction(0, ui.actionReject_call);
    popup_menu->insertAction(0, ui.actionHangup);
    popup_menu->insertAction(0, ui.actionView_Profile);
    popup_menu->insertAction(0, ui.actionSend_message);
    popup_menu->insertAction(0, ui.actionCompose_Message);
    popup_menu->insertAction(0, ui.actionSend_file);
    popup_menu->insertAction(0, ui.actionPromote_to_Pal);
    popup_menu->insertAction(0, ui.actionDemote_to_Non_pal);
    popup_menu->insertAction(0, ui.actionBrowse_saved_msgs);
    popup_menu->insertAction(0, ui.actionBlock_user);
    popup_menu->insertAction(0, ui.actionUnblock_user);
    popup_menu->insertAction(0, ui.actionRemove_user);
    popup_menu->insertAction(0, ui.actionUpdate_info);
    //popup_menu->insertAction(0, ui.actionAlert_when_online);

    tray_menu = new QMenu(this);
    tray_menu->insertAction(0, ui.actionOpen);
    tray_menu->insertAction(0, ui.actionExit);


    lobbybox3 *lb = new lobbybox3(this);
    addDockWidget(Qt::BottomDockWidgetArea, lb);
    lbox = lb;
    connect(lbox, SIGNAL(visibilityChanged(bool)), this, SLOT(show_lobby_list(bool)));

    vidbox *vb = new vidbox(this);
    addDockWidget(Qt::TopDockWidgetArea, vb);
    vid_preview = vb;
    connect(vb, SIGNAL(visibilityChanged(bool)), this, SLOT(show_vid_preview(bool)));

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    simple_public *s = new simple_public(this);
    addDockWidget(Qt::LeftDockWidgetArea, s);
    pubchat = s;
    connect(pubchat, SIGNAL(visibilityChanged(bool)), this, SLOT(show_pubchat(bool)));

#ifdef CDCX_WEBKIT
    dir_listing = new SimpleDirBox(this);
    addDockWidget(Qt::LeftDockWidgetArea, dir_listing);
    connect(dir_listing, SIGNAL(visibilityChanged(bool)), this, SLOT(show_dir_listing(bool)));
    tabifyDockWidget(s, dir_listing);

    // browser tab
    WHATBOX *bb = new ProfilePreviewBox(this);
    addDockWidget(Qt::LeftDockWidgetArea, bb);
    connect(bb, SIGNAL(visibilityChanged(bool)), this, SLOT(show_profiles(bool)));
    tabifyDockWidget(s, bb);
    browse_box = bb;
#else
    dir_listing = 0;
    browse_box = 0;
#endif

    // saved msgs tab
    MsgBrowseDock *mbd = new MsgBrowseDock(this);
    addDockWidget(Qt::LeftDockWidgetArea, mbd);
    connect(mbd, SIGNAL(visibilityChanged(bool)), this, SLOT(show_msgbrowse(bool)));
    tabifyDockWidget(s, mbd);
    msg_browse_dock = mbd;
    mbd->setVisible(1);

    pubchat->setVisible(1);
    pubchat->raise();

    umodel = new QStandardItemModel(0, 1);

    ui.usertree_view->header()->hide();
    sort_proxy_model.setSourceModel(umodel);
    sort_proxy_model.setFilterCaseSensitivity(Qt::CaseInsensitive);
    sort_proxy_model.setSortCaseSensitivity(Qt::CaseInsensitive);
    sort_proxy_model.setDynamicSortFilter(1);
    ui.usertree_view->setModel(&sort_proxy_model);
    // this doesn't work probably because there is nothing in
    // the model yet.
    //ui.usertree_view->setExpanded(pals_idx, 1);
    first_expand = 1;

    db_status = new QLabel;
    db_status->setText("<font color=\"#FF0000\">Not Logged in</font>");
    ui.statusBar->addWidget(db_status);

    server_status = new QLabel;
    server_status->setText("<font color=\"#FF0000\">Offline</font>");
    ui.statusBar->addWidget(server_status);

    audio_input_status = new QLabel;
    audio_input_status->setFrameShape(QFrame::StyledPanel);
    if(HasAudioInput)
    {
        audio_input_status->setText("Mic");
        DwOString max_audio("4");
        setting_get("call_acceptance/max_audio", max_audio);
        // hack to fix bug
        if(QString(max_audio.c_str()).toInt() < 4)
        {
            max_audio = "4";
            setting_put("call_acceptance/max_audio", max_audio);
        }
        dwyco_set_setting("call_acceptance/max_audio", max_audio.c_str());
    }
    else
    {
        audio_input_status->setText("<font color=\"#FF0000\">No Mic</font>");
        // since not having a mic may be transient, save the
        // current value, just in case.
        dwyco_set_setting("call_acceptance/max_audio", "0");
    }
    //ui.statusBar->addWidget(audio_input_status);

    audio_output_status = new QLabel;
    audio_output_status->setFrameShape(QFrame::StyledPanel);
    if(HasAudioOutput)
    {
        audio_output_status->setText("Speakers");
        DwOString max_audio_recv("4");
        setting_get("call_acceptance/max_audio_recv", max_audio_recv);
        // hack to fix bug
        if(QString(max_audio_recv.c_str()).toInt() < 4)
        {
            max_audio_recv = "4";
            setting_put("call_acceptance/max_audio_recv", max_audio_recv);
        }
        dwyco_set_setting("call_acceptance/max_audio_recv", max_audio_recv.c_str());
    }
    else
    {
        audio_output_status->setText("<font color=\"#FF0000\">No Speakers</font>");
        // since not having speakers may be transient
        dwyco_set_setting("call_acceptance/max_audio_recv", "0");
    }
    //ui.statusBar->addWidget(audio_output_status);
    camera_status = new QLabel;
    camera_status->setFrameShape(QFrame::StyledPanel);
    ui.statusBar->addWidget(camera_status);

    invis_status = new QLabel;
    invis_status->setFrameShape(QFrame::StyledPanel);
    ui.statusBar->addWidget(invis_status);
    if(dwyco_get_invisible_state())
    {
        invis_status->setText("<font color=\"#FF0000\">Invis</font>");
    }
    else
    {
        invis_status->hide();
    }

    pals_only_status = new QLabel;
    pals_only_status->setFrameShape(QFrame::StyledPanel);
    ui.statusBar->addWidget(pals_only_status);
    if(dwyco_get_pals_only())
    {
        pals_only_status->setText("<font color=\"#FF0000\">Pals only</font>");
        pals_only_status->show();
    }
    else
    {
        pals_only_status->hide();
    }

    archive_status = new QLabel;
    archive_status->setFrameShape(QFrame::StyledPanel);
    ui.statusBar->addWidget(archive_status);
    archive_status->hide();

    lic_status = new QLabel;
    lic_status->setFrameShape(QFrame::StyledPanel);

    ui.login_button->hide();
    ThePwForm = new pwform;

    gg_tray_icon = new QIcon(":/new/prefix1/ggtiny.png");
    blank_tray_icon = new QIcon(":/new/prefix1/black.png");
    tray_icon = new QSystemTrayIcon(*gg_tray_icon);
    tray_icon->setToolTip("Oh wow! It's Dwyco!");
    tray_icon->setContextMenu(tray_menu);
    tray_icon->show();
    connect(tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayiconActivated(QSystemTrayIcon::ActivationReason)));
    connect(&timer, SIGNAL(timeout()), this, SLOT(idle()));
    timer.start(20);

    connect(&animation_timer, SIGNAL(timeout()), this, SLOT(animate()));
    animation_timer.start(500);

    DwOString mstate("menu-state ");

    connect(this, SIGNAL(ignore_event(DwOString)), SLOT(update_ignore(DwOString)));
    int doff;
    if(!setting_get("display-offline", doff))
    {
        setting_put("display-offline", 1);
        settings_save();
        doff = 1;
    }
    Display_all_uid = doff;
    ui.actionShow_Offline_Users->setChecked(doff);
    if(doff)
        mstate += "offline, ";

    if(!setting_get("display-chat-users", doff))
    {
        setting_put("display-chat-users", 1);
        settings_save();
        doff = 1;
    }
    Display_chat_users = doff;
    ui.actionShow_Public_Chat_Users->setChecked(doff);
    if(doff)
        mstate += "chat, ";

    if(!setting_get("display-pics-in-user-list", doff))
    {
        setting_put("display-pics-in-user-list", 1);
        settings_save();
        doff = 1;
    }
    Display_pics_in_user_list = doff;
    ui.actionShow_pics_in_user_list->setChecked(doff);
    if(doff)
        mstate += "pics, ";

    if(!setting_get("allow-multiple-windows", doff))
    {
        setting_put("allow-multiple-windows", 0);
        settings_save();
        doff = 0;
    }
    Allow_multiple_windows = doff;
    ui.actionAllow_multiple_profiles_and_browsers->setChecked(doff);
    if(doff)
        mstate += "multi, ";

    int invis = dwyco_get_invisible_state();
    ui.actionMake_me_invisible->setChecked(invis);

    if(invis)
        mstate += "invis";
    dwyco_field_debug(mstate.c_str(), 1);

    QString ttl = windowTitle();
    ttl += "(";
    ttl += QCoreApplication::applicationVersion();
    ttl += ")";
    setWindowTitle(ttl);

    connect(this, SIGNAL(pal_event(DwOString)), this, SLOT(set_refresh_user(DwOString)));
    connect(this, SIGNAL(pal_event(DwOString)), this, SLOT(record_recent_operation(DwOString)));
    connect(this, SIGNAL(pal_event(DwOString)), this, SLOT(pal_relogin(DwOString)));
    connect(this, SIGNAL(camera_change(int)), this, SLOT(camera_event(int)));
    connect(this, SIGNAL(ignore_event(DwOString)), this, SLOT(record_recent_operation(DwOString)));
    connect(this, SIGNAL(ignore_event(DwOString)), this, SLOT(set_refresh_user(DwOString)));
    connect(this, SIGNAL(send_msg_event(DwOString)), this, SLOT(record_recent_operation(DwOString)));
    connect(this, SIGNAL(invalidate_profile(DwOString)), this, SLOT(set_refresh_user(DwOString)));
    connect(this, SIGNAL(uid_info_event(DwOString)), this, SLOT(set_refresh_user(DwOString)));
    connect(this, SIGNAL(pals_only(int)), this, SLOT(update_pals_only(int)));

    connect(this, SIGNAL(audio_recording(int,int)), this, SLOT(audio_recording_event(int,int)));
    connect(this, SIGNAL(uid_status_change(DwOString)), this, SLOT(online_status_change(DwOString)));


    if(TheConfigForm)
    {
        connect(TheConfigForm, SIGNAL(content_filter_event(int)), this, SIGNAL(content_filter_event(int)));
        connect(TheConfigForm, SIGNAL(pals_only(int)), this, SIGNAL(pals_only(int)));
    }
    new profpv;


}

mainwinform::~mainwinform()
{
    tray_icon->hide();
}

void
mainwinform::refetch_user_list()
{
    dwyco_load_users2(!Display_archived_users, &User_count);
}

#if 0
QDockWidget *
mainwinform::get_current_tab()
{
    QList<QTabBar *> tbl = findChildren<QTabBar *>();
    if(tbl.count() < 1)
        return 0;
    int current = tbl[0]->currentIndex();
    return 0;

}
#endif

bool
mainwinform::event(QEvent *event)
{
    return QMainWindow::event(event);
}

void
mainwinform::next_tab()
{
    QList<QTabBar *> tabList = findChildren<QTabBar *>();
    if(tabList.isEmpty())
        return;
    QTabBar * tb = tabList.at(0);
    int i = tb->currentIndex();
    ++i;
    if(i >= tb->count())
        i = tb->count() - 1;
    tb->setCurrentIndex(i);
}

void
mainwinform::prev_tab()
{
    QList<QTabBar *> tabList = findChildren<QTabBar *>();
    if(tabList.isEmpty())
        return;
    QTabBar * tb = tabList.at(0);
    int i = tb->currentIndex();
    --i;
    if(i < 0)
        i = 0;
    tb->setCurrentIndex(i);

}

void
mainwinform::update_pals_only(int)
{
    if(dwyco_get_pals_only())
    {
        pals_only_status->setText("<font color=\"#FF0000\">Pals only</font>");
        pals_only_status->show();
    }
    else
    {
        pals_only_status->hide();
    }
}

void
mainwinform::show_archive_status(int n, int total)
{
    if(n < total)
    {
        archive_status->setText(QString("<font color=\"#FF0000\">Archived %1 users</font>").arg(total - n));
        archive_status->show();
    }
    else
    {
        archive_status->hide();
    }
}

void
mainwinform::record_recent_operation(DwOString uid)
{
    QByteArray b(uid.c_str(), uid.length());
    if(Recent_uid_operations.contains(b))
        return;
    Recent_uid_operations.insert(b);
    set_refresh_user(uid);
}

void
mainwinform::load_browsebox()
{
#ifdef CDCX_WEBKIT
    browse_box->reload_triggered();
#endif
}

void
mainwinform::manage_audio_sends(DwOString /*uid*/, int chan, bool state)
{
    int estate;
    int echan;
    dwyco_get_exclusive_audio(&estate, &echan);
    if(!estate)
        return;
    if(chan == echan)
    {
        if(state == 0)
        {
            // exclusive on, audio is being muted
            dwyco_set_exclusive_audio(1, -1);
        }
    }
    else if(state == 1)
    {
        // exclusive on, audio is being  switched to another
        // channel
        dwyco_set_exclusive_audio(1, chan);
    }
}

void
mainwinform::audio_recording_event(int chan, int rec)
{

}

void
mainwinform::trayiconActivated(QSystemTrayIcon::ActivationReason a)
{
    if(a == QSystemTrayIcon::DoubleClick)
    {
        Mainwinform->show();
        Mainwinform->raise();
        Mainwinform->showNormal();
    }
}

void
mainwinform::set_refresh_user(DwOString)
{
    cdcx_set_refresh_users(1);
}

void
mainwinform::on_actionExit_triggered(bool)
{
    close();
}

void
mainwinform::on_actionOpen_triggered(bool)
{
    Mainwinform->show();
    Mainwinform->raise();
    Mainwinform->showNormal();
}


void
mainwinform::camera_event(int on)
{
    if(on)
    {
        camera_status->setText("Cam On");
        ui.actionShow_Video_Preview->setEnabled(1);
        on_actionShow_Video_Preview_triggered(1);
    }
    else
    {
        if(!HasCamHardware)
            camera_status->setText("<font color=\"#FF0000\">No Cam</font>");
        else
            camera_status->setText("<font color=\"#FF0000\">Cam Off</font>");

        ui.actionShow_Video_Preview->setEnabled(0);
        //vid_preview->hide();
        Last_preview_image = QImage();
    }
}

void
mainwinform::update_ignore(DwOString uid)
{
    // note: technically i don't think we need this since
    // refresh users probably gets set when the dwyco_ignore/unignore
    // is done.
    if(dwyco_is_ignored(uid.c_str(), uid.length()))
        call_destroy_by_uid(uid);
    dwyco_pal_relogin();
}

QList<QByteArray>
mainwinform::get_selection(int only_one)
{
    QItemSelectionModel *sm = ui.usertree_view->selectionModel();
    QModelIndexList idxs = sm->selectedIndexes();
    QList<QByteArray> ret;
    if(idxs.count() == 0 || (idxs.count() > 1 && only_one))
        return ret;

    for(int i = 0; i < idxs.count(); ++i)
    {
        if(idxs[i].isValid())
        {
            QByteArray uid = sort_proxy_model.data(idxs[i], Qt::UserRole).toByteArray();
            if(uid.length() == 0)
                continue;
            ret.append(uid);
        }
    }
    if(ret.count() == 1)
        emit uid_selected(DwOString(ret[0].constData(), 0, ret[0].length()), 4);
    return ret;

}

void
mainwinform::contextMenuEvent(QContextMenuEvent *ev)
{
    QItemSelectionModel *sm = ui.usertree_view->selectionModel();

    QList<QByteArray> uids = get_selection(0);
    int multi = uids.count() > 1;
    int single = uids.count() == 1;
    if(uids.count() == 0)
    {
        return;
    }
    ui.actionAccept_call->setVisible(0);
    ui.actionReject_call->setVisible(0);
    ui.actionHangup->setVisible(0);
    if(single)
    {
        QByteArray uid = uids[0];
        //ui.actionAlert_when_online->setChecked(dwyco_get_alert(uid.constData(), uid.length()));
        int pal = dwyco_is_pal(uid.constData(), uid.length());

        int ignored = dwyco_is_ignored(uid.constData(), uid.length());

        ui.actionCompose_Message->setVisible(!ignored);
        ui.actionDemote_to_Non_pal->setVisible(pal);
        ui.actionPromote_to_Pal->setVisible(!pal);
        ui.actionBlock_user->setVisible(!ignored);
        ui.actionUnblock_user->setVisible(ignored);
        ui.actionSend_file->setVisible(!ignored);
        ui.actionSend_message->setVisible(!ignored);
        ui.actionView_Profile->setVisible(!ignored);
        //ui.actionAlert_when_online->setVisible(1);
        ui.actionBrowse_saved_msgs->setVisible(1);

        DwOString u(uid.constData(), 0, uid.length());
#if 0
        if(call_exists_by_uid(u))
            ui.actionHangup->setVisible(1);
        else
        {
            ui.actionHangup->setVisible(0);
        }
#endif

    }
    if(multi)
    {
        //ui.actionHangup->setVisible(1);
        //ui.actionAlert_when_online->setVisible(0);
        ui.actionCompose_Message->setVisible(0);
        ui.actionDemote_to_Non_pal->setVisible(1);
        ui.actionPromote_to_Pal->setVisible(1);
        ui.actionBlock_user->setVisible(1);
        ui.actionUnblock_user->setVisible(1);
        ui.actionSend_file->setVisible(0);
        ui.actionSend_message->setVisible(0);
        ui.actionView_Profile->setVisible(0);
        ui.actionBrowse_saved_msgs->setVisible(0);

    }
    popup_menu->popup(ev->globalPos());
}

void
mainwinform::closeEvent(QCloseEvent *ce)
{
    QByteArray b(saveGeometry());
    QSettings s;
    s.setValue("mainwin-geometry", b);

    QWidget::closeEvent(ce);
}


static void
DWYCOCALLCONV
cul_callback(const char *cmd, void *arg, int succ, const char *fail)
{
    printf("%s arg %p succ %d fail %s\n", cmd, arg, succ, fail);
    fflush(stdout);
}


void
mainwinform::on_login_button_clicked()
{
    Initial_login_processing = 1;
}


void
mainwinform::on_actionAbout_triggered(bool)
{
    aboutform foo;
    foo.exec();
}


void
mainwinform::on_actionUser_profile_triggered(bool)
{
    composer_profile *c;
    c = composer_profile::get_composer_profile(My_uid);
    if(!c)
    {
        c = new composer_profile(this);
        c->start();
        c->show();
        c->raise();
    }
    else
    {
        c->show();
        c->raise();
    }
}

void
mainwinform::on_actionVideo_triggered(bool)
{
    if(!DevSelectBox)
        DevSelectBox = new VidSel;
    DevSelectBox->show();
    DevSelectBox->raise();
}

void
mainwinform::on_actionCompose_Message_triggered(bool)
{
    dwyco_field_debug("ul-compose", 1);
    composer *c = 0;
    QList<QByteArray> uids = get_selection(1);
    if(uids.count() == 0)
    {
        return;
    }
    QByteArray uid = uids[0];

    c = new composer(COMP_STYLE_REGULAR, 0, this);
    c->set_uid(DwOString(uid.constData(), 0, uid.length()));
    c->show();
    c->raise();

}

void
mainwinform::on_actionSend_message_triggered(bool)
{
    QList<QByteArray> uids = get_selection(1);
    if(uids.count() == 0)
    {
        return;
    }
    QByteArray uid = uids[0];
    DwOString duid(uid.constData(), 0, uid.count());
    simple_call *sc = simple_call::get_simple_call(duid);
    sc->setVisible(1);
    sc->raise();
}

void
mainwinform::on_actionFetch_Lost_Password_triggered(bool)
{
#if 0
    if(!TheFetchPwForm)
        new fetchpwform;
    TheFetchPwForm->show();
    TheFetchPwForm->fetch();
#endif
}

void
mainwinform::on_actionAdvanced_triggered(bool)
{
    if(!TheConfigForm)
        new configform;
    TheConfigForm->show();
    TheConfigForm->raise();
}

void
mainwinform::on_actionUpdate_info_triggered(bool)
{
    QList<QByteArray> uids = get_selection(0);
    for(int i = 0; i < uids.count(); ++i)
        dwyco_fetch_info(uids[i].constData(), uids[i].length());
    cdcx_set_refresh_users(1);
}

void
mainwinform::on_actionPromote_to_Pal_triggered(bool)
{
    dwyco_field_debug("ul-pal", 1);
    QList<QByteArray> uids = get_selection(0);
    for(int i = 0; i < uids.count(); ++i)
    {
        QByteArray uid = uids[i];
        dwyco_pal_add(uid.constData(), uid.length());
        emit pal_event(DwOString(uid.constData(), 0, uid.length()));
    }
    cdcx_set_refresh_users(1);
    //load_users();
    //decorate_users();
}

void
mainwinform::on_actionDemote_to_Non_pal_triggered(bool)
{
    QList<QByteArray> uids = get_selection(0);
    for(int i = 0; i < uids.count(); ++i)
    {
        QByteArray uid = uids[i];
        dwyco_pal_delete(uid.constData(), uid.length());
        emit pal_event(DwOString(uid.constData(), 0, uid.length()));
    }
    cdcx_set_refresh_users(1);
    //load_users();
    //decorate_users();
}

void
mainwinform::on_actionBlock_user_triggered(bool)
{
    dwyco_field_debug("ul-block", 1);
    if(!confirm_ignore())
        return;
    QList<QByteArray> uids = get_selection(0);
    for(int i = 0; i < uids.count(); ++i)
    {
        QByteArray uid = uids[i];
        dwyco_ignore(uid.constData(), uid.length());
        emit ignore_event(DwOString(uid.constData(), 0, uid.length()));
    }
}

void
mainwinform::on_actionUnblock_user_triggered(bool)
{
    QList<QByteArray> uids = get_selection(0);
    for(int i = 0; i < uids.count(); ++i)
    {
        QByteArray uid = uids[i];
        dwyco_unignore(uid.constData(), uid.length());
        emit ignore_event(DwOString(uid.constData(), 0, uid.length()));
    }
}

void
mainwinform::on_actionView_Profile_triggered(bool)
{
    dwyco_field_debug("ul-show-profile", 1);
    viewer_profile *c;
    QList<QByteArray> uids = get_selection(1);
    if(uids.count() == 0)
    {
        return;
    }
    QByteArray uid = uids[0];
    DwOString duid(uid.constData(), 0, uid.length());
    if(!Allow_multiple_windows)
        viewer_profile::delete_all();
    c = viewer_profile::get_viewer_profile(duid);
    if(!c)
    {
        c = new viewer_profile(this);
        c->set_uid(duid);
        c->show();
        c->start_fetch();
    }
    else
    {
        c->show();
        c->raise();
        c->showNormal();
    }
}

void
mainwinform::on_actionBrowse_saved_msgs_triggered(bool)
{
    dwyco_field_debug("ul-browse-old", 1);
    QList<QByteArray> uids = get_selection(0);
    if(uids.count() != 1)
        return;
    on_actionShow_Msg_Browser_triggered(1);
    QByteArray uid = uids[0];
    DwOString duid = DwOString(uid.constData(), 0, uid.length());

    emit uid_selected(duid, 6);
}

// we don't want to allow chatboxes to proliferate unless the
// user has explicitly undocked/floated one. so here is where
// we look for multiple chatboxes, and hide all but one of them
// unless it is floated.
void
mainwinform::organize_chatboxes(simple_call *c)
{
    QList<simple_call *> scl = findChildren<simple_call *>();
    for(int i = 0; i < scl.count(); ++i)
    {
        if(c == scl[i])
            continue;
        if(scl[i]->isFloating())
            continue;
        scl[i]->hide();
    }
}

QString
get_a_filename(QString filter)
{
    static QString last_directory = QDir::homePath();
#if (!defined(LINUX) && !defined(MAC_CLIENT))
// windows wants to change the cwd on us, so tell it to
// use the qt dialog.
    // apparently despite our request otherwise, qt decides to use the native dialog
    //QString fn = QFileDialog::getOpenFileName(0, "File to send", QString(), QString(), 0, QFileDialog::DontUseNativeDialog);

    QFileDialog fd;

    fd.setOption(QFileDialog::DontUseNativeDialog);
    fd.setDirectory(last_directory);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setNameFilter(filter);
    if(fd.exec() == QDialog::Rejected)
        return 0;
    QStringList fnl = fd.selectedFiles();
    QString fn = fnl[0];
    if(fn.length() > 0)
    {
        last_directory = fn;
        last_directory.truncate(last_directory.lastIndexOf("/"));
    }

    // note: apparently on windows, qt does you the favor of changing
    // directory separator to '/', which confuses the rest of the dll...
    // so we'll just convert them back.
    fn.replace('/', '\\');
#else


    QString fn = QFileDialog::getOpenFileName(0, "Select file", last_directory, filter);
    if(fn.length() > 0)
    {
        last_directory = fn;
        last_directory.truncate(last_directory.lastIndexOf("/"));

    }
#endif

    return fn;
}

int
send_file_common(const QByteArray& uid)
{
    QString fn = get_a_filename("");
    if(fn.length() == 0)
        return 0;
    composer_file *c = new composer_file(fn, 0);
    c->set_uid(DwOString(uid.constData(), 0, uid.length()));

    c->show();
    c->raise();
    return 1;
}

void
mainwinform::on_actionSend_file_triggered(bool)
{
    QList<QByteArray> uids = get_selection(1);
    if(uids.count() == 0)
    {
        return;
    }
    QByteArray uid = uids[0];
    send_file_common(uid);
}

void
mainwinform::on_actionRemove_user_triggered(bool)
{
    QList<QByteArray> uids = get_selection(0);
    int n = uids.count();
    QMessageBox::StandardButton s = QMessageBox::question(this, "Remove user",
                                    QString("Are you sure you want to remove %1 selected users? (NO UNDO)").arg(n),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if(s == QMessageBox::No)
    {
        return;
    }
    int reload = 0;
    for(int i = 0; i < n; ++i)
    {
        QByteArray uid = uids[i];
        DwOString duid(uid.constData(), 0, uid.length());
        // delete all unviewed msgs from the index
        del_unviewed_uid(duid);
        QList<simple_call *> sc = simple_call::Simple_calls.query_by_member(duid, &simple_call::uid);
        if(sc.count() == 1)
        {
            sc[0]->close();
            // note: this triggers a race condition: the call_died
            // callback may be called before or after the
            // simple_call is deleted... should be ok for now,
            // but really needs a fix.
            sc[0]->deleteLater();

        }
        if(duid == msg_browse_dock->uid)
            msg_browse_dock->reset();
        viewer_profile *p = viewer_profile::get_viewer_profile(duid);
        if(p)
        {
            p->close();
            delete p;
        }

        int was_pal = dwyco_is_pal(uid.constData(), uid.length());
        dwyco_delete_user(uid.constData(), uid.length());
        Session_remove.insert(duid);
        Seen_in_chat.removeAll(duid);
        // we know the core deletes a user from our pal list automatically, so
        // generate an event here so filtering and stuff can be updated based on
        // pal-ness
        if(was_pal)
        {
            emit pal_event(duid);
        }
        reload = 1;

    }
    // certain things are really bad in the current core:
    // if a user is added to the system, the user list is automatically
    // updated behind the scenes, you just have to refetch it.
    // if you delete a user, it is *not* updated. you have to reload the
    // entire thing from disk again (this seems like a bug that needs to be
    // fixed.)
    if(reload)
    {
        refetch_user_list();
        load_users();
        cdcx_set_refresh_users(1);
    }
}

void
mainwinform::on_actionAlert_when_online_triggered(bool state)
{
//    QList<QByteArray> uids = get_selection(0);
//    int n = uids.count();
//    for(int i = 0; i < n; ++i)
//    {
//        dwyco_set_alert(uids[i].constData(), uids[i].length(), state);
//    }
}

void
mainwinform::on_actionShow_Ignore_List_triggered(bool)
{
    if(!TheIglistForm)
        TheIglistForm = new iglistform;
    TheIglistForm->show();
    TheIglistForm->raise();
}

void
mainwinform::on_actionHow_to_buy_a_subscription_triggered(bool)
{
    QDesktopServices::openUrl(QUrl("http://www.dwyco.com/order.html"));
}


void
mainwinform::on_usertree_view_doubleClicked(const QModelIndex& idx)
{
    QByteArray uid = sort_proxy_model.data(idx, Qt::UserRole).toByteArray();
    if(uid.length() == 0)
        return;
    DwOString duid(uid.constData(), 0, uid.length());
    simple_call *sc = simple_call::get_simple_call(duid);
    sc->setVisible(1);

    sc->raise();
    organize_chatboxes(sc);
    emit uid_selected(duid, 4);
}

void
mainwinform::on_usertree_view_clicked(const QModelIndex& idx)
{
    QByteArray uid = sort_proxy_model.data(idx, Qt::UserRole).toByteArray();
    if(uid.length() == 0)
        return;
    DwOString duid(uid.constData(), 0, uid.length());
    simple_call *sc = simple_call::get_simple_call(duid);
    sc->setVisible(1);
    if(!msg_browse_dock->vis)
        sc->raise();
    organize_chatboxes(sc);

    emit uid_selected(duid, 6);
}


QString
get_extended(QString txt)
{
    int i = txt.indexOf("<html>", 0, Qt::CaseInsensitive);
    if(i == -1)
        return txt;
    txt.remove(0, i);
    return txt;
}


QString
dwyco_info_to_display(const DwOString& uid)
{
    DWYCO_LIST l;
    int cant_resolve = 0;
    if(uid.length() == 0)
        return QString("<<empty uid>>");
    l = dwyco_uid_to_info(uid.c_str(), uid.length(), &cant_resolve);
    if(cant_resolve)
    {
        dwyco_fetch_info(uid.c_str(), uid.length());
    }
    const char *val;
    int len, type;
    dwyco_list_get(l, 0, DWYCO_INFO_HANDLE, &val, &len, &type);
    if(type != DWYCO_TYPE_STRING)
    {
        dwyco_list_release(l);
        return QString("<<bad>>");
    }
    QString ret(val);
    dwyco_list_release(l);
    return ret;
}

int
dwyco_info_to_display(const DwOString& uid, QString& handle, QString& desc, QString& loc, QString& email)
{
    DWYCO_LIST l;
    int cant_resolve = 0;
    if(uid.length() == 0)
        return 0;
    l = dwyco_uid_to_info(uid.c_str(), uid.length(), &cant_resolve);
    if(cant_resolve)
    {
        dwyco_fetch_info(uid.c_str(), uid.length());
    }
    const char *val;
    int len, type;
    dwyco_list_get(l, 0, DWYCO_INFO_HANDLE, &val, &len, &type);
    if(type != DWYCO_TYPE_STRING)
    {
        dwyco_list_release(l);
        return 0;
    }
    handle = val;
    dwyco_list_get(l, 0, DWYCO_INFO_DESCRIPTION, &val, &len, &type);
    if(type != DWYCO_TYPE_STRING)
    {
        dwyco_list_release(l);
        return 0;
    }
    desc = val;
    dwyco_list_get(l, 0, DWYCO_INFO_LOCATION, &val, &len, &type);
    if(type != DWYCO_TYPE_STRING)
    {
        dwyco_list_release(l);
        return 0;
    }
    loc = val;
    if(uid == My_uid)
    {
        dwyco_list_get(l, 0, DWYCO_INFO_EMAIL, &val, &len, &type);
        if(type == DWYCO_TYPE_STRING)
        {
            email = val;
        }
        else if(type == DWYCO_TYPE_NIL)
        {
            email = "";
        }
        else
        {
            dwyco_list_release(l);
            return 0;
        }

    }

    dwyco_list_release(l);
    return 1;
}

int
confirm_show_profile()
{
    int val = 1;
    if(setting_get("show-profile-confirm", val) == 0)
    {
        setting_put("show-profile-confirm", 1);
        settings_save();
    }
    if(!val)
        return 1;
    QMessageBox msgBox(QMessageBox::Question, "Really show profile?",
                       "You have selected to mask some material, but you clicked to "
                       "display the profile. This displays explicit, unreviewed material. "
                       "Are you sure you want to display it?"
                      );
    QPushButton *yes = msgBox.addButton("Yes", QMessageBox::AcceptRole);
    QPushButton *no = msgBox.addButton("No", QMessageBox::RejectRole);
    QPushButton *yesalways = msgBox.addButton("Yes, and don't ask again.", QMessageBox::YesRole);
    msgBox.exec();

    if (msgBox.clickedButton() == yes) {
        return 1;
    } else if (msgBox.clickedButton() == no) {
        return 0;
    }
    else if(msgBox.clickedButton() == yesalways)
    {
        setting_put("show-profile-confirm", 0);
        settings_save();
        return 1;
    }
    return 0;
}

int
show_profile(const DwOString& uid, int reviewed, int regular)
{
    int is_pal = dwyco_is_pal(uid.c_str(), uid.length());
    if(is_pal)
        return 1;
    int show = 0;
    setting_get("chat_show_unreviewed", show);

    if(show)
        return 1;
    int ok = reviewed && regular;
    return ok;
}

static int
dwyco_get_attr_int(DWYCO_LIST l, int row, const char *col, int& int_out)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_INT)
        return 0;
    DwOString str_out = DwOString(val, 0, len);
    int_out = atoi(str_out.c_str());
    return 1;
}


void
get_review_status(const DwOString& uid, int& reviewed, int& regular)
{
    DWYCO_LIST l;
    int cant_resolve;
    l = dwyco_uid_to_info(uid.c_str(), uid.length(), &cant_resolve);
    reviewed = 0;
    regular = 0;
    dwyco_get_attr_int(l, 0, DWYCO_INFO_REVIEWED, reviewed);
    dwyco_get_attr_int(l, 0, DWYCO_INFO_REGULAR, regular);
    dwyco_list_release(l);
}


void
cursor_to_bottom(QTextEdit *te)
{
    // if the user has moved the scrollbar back up, don't
    // set to the bottom of the text.
    QScrollBar *sb = te->verticalScrollBar();
    if(sb && sb->value() == sb->maximum())
        return;
    QTextCursor tc = te->textCursor();
    tc.movePosition(QTextCursor::End);
    te->setTextCursor(tc);
    te->ensureCursorVisible();
    if(sb)
        sb->setValue(sb->maximum());
}

void
remove_first_block(QTextEdit *te)
{
#if 0
    // i fuckin' give up. i've tried so many things
    // here that don't work to delete the first block of
    // text in the edit.
    te->setReadOnly(0);
    QTextCursor tc = te->textCursor();
    tc.movePosition(QTextCursor::Start);
    tc.movePosition(QTextCursor::NextCharacter);
    tc.select(QTextCursor::BlockUnderCursor);
    tc.removeSelectedText();
    te->setReadOnly(1);
#endif

}

void
append_msg_to_textedit(QString txt, QTextEdit *te, DwOString mid)
{
    QTextFrame *tf = te->document()->rootFrame();
    QTextCursor tc = tf->lastCursorPosition();
    QTextBlockFormat tbf;
    //tbf.setBackground(QBrush(QColor(Qt::cyan)));
    tc.insertBlock(tbf);
    QTextDocumentFragment tdf = QTextDocumentFragment::fromHtml(txt);
    tc.insertFragment(tdf);
    QTextBlock tb = te->document()->lastBlock();
    //printf("---------------\n");
    //printf("%s\n", te->document()->toHtml().toAscii().constData());
    //printf("||||||||||||||\n");

#if 0
    if(tb.isValid())
    {
        chatform_user_data *ud = new chatform_user_data;
        ud->mid = mid;
        ud->time_received = time(0);
        ud->seq = ++chatform_user_data::Seq;
        tb.setUserData(ud);
    }
    if(te->document()->blockCount() > 10)
        remove_first_block(te);
#endif


}


void
append_table_to_textedit(DWYCO_SAVED_MSG_LIST sm, QTextEdit *te, DwOString mid)
{

    QTextFrame *tf = te->document()->rootFrame();
    QTextCursor tc = tf->lastCursorPosition();
    QTextTable *tt = tc.insertTable(1, 1);
    msg_to_table(sm, tt);

#if 0
    QTextBlock tb = te->document()->lastBlock();
    if(tb.isValid())
    {
        chatform_user_data *ud = new chatform_user_data;
        ud->mid = mid;
        ud->time_received = time(0);
        ud->seq = ++chatform_user_data::Seq;
        tb.setUserData(ud);
    }
#endif

}

simple_call *
display_msg(const DwOString& uid, const DwOString& txt, const DwOString& mid)
{
    simple_call *sc = simple_call::get_simple_call(uid);
    Mainwinform-> emit new_msg(uid, txt, mid);
    return sc;
}

int
check_pw_against_local_hash(const DwOString& pw)
{
    DwOString ss;
    char *salt;
    int salt_len = 0;
    char *hash;
    int len_hash;
    if(!setting_get("salt", ss))
        return 0;
    salt = (char *)ss.c_str();
    salt_len = ss.length();
    dwyco_gen_pass(pw.c_str(), pw.length(), &salt, &salt_len,
                   &hash, &len_hash);
    DwOString hs(hash, 0, len_hash);
    DwOString spw;
    if(!setting_get("server-pw", spw))
        return 0;
    if(hs != spw)
        return 0;
    return 1;
}


void
mainwinform::relogin()
{
    dwyco_database_login();
}

void
mainwinform::pal_relogin(DwOString)
{
    Pal_relogin = 1;
}

void
mainwinform::idle()
{
    int spin;
    if(DieDieDie)
    {
        close();
        return;
    }
    QDateTime d = QDateTime::currentDateTime();
    QByteArray dt = d.time().toString("hh:mm:ss.zzz").toAscii();
    dwyco_add_entropy_timer(dt.data(), dt.count());
    // we need this to block all dll actions during some modal
    // dialogs, (qt will still process events, just not input
    // events, which means file transfers and stuff will continue
    // even when a modal dialog is up, which is not what we want.)
    if(Block_DLL)
        return;
    // these popups don't make a lot of sense on a touch screen
    // this is braindead hover checking because the qt enter/leave stuff
    // doesn't work right, nor do the hover events
    static QPoint last_point;
    static int emit_stopped;
    QPoint Curpoint = QCursor::pos();
    if(last_point == Curpoint)
    {
        if(emit_stopped)
        {
            emit mouse_stopped(last_point);
            emit_stopped = 0;
        }
    }
    else
    {
        last_point = Curpoint;
        emit_stopped = 1;
    }
    if(DoWizard)
    {
        DoWizard = 0;
        dowiz();
        return;
    }
    if(First25)
    {
        First25 = 0;
        QMessageBox msgBox(QMessageBox::Question, "New Feature Alert",
                           "Dwyco now reviews profiles. "
                           "By default, profile filtering is enabled. "
                           "Do you want to turn off filtering now? Click "
                           "YES to show UNREVIEWED and EXPLICIT profiles "
                           "(ie. it works like it did before.) "
                           "Click NO to show only reviewed, non-explicit profiles. "
                           "For more information, visit <a href=\"http://www.dwyco.com/profile_info.html\">Profile Info</a>",
                           QMessageBox::Yes|QMessageBox::No//, Mainwinform, Qt::Dialog
                          );
        //QPushButton *yes = msgBox.addButton("Yes", QMessageBox::AcceptRole);
        //QPushButton *no = msgBox.addButton("No", QMessageBox::RejectRole);
        //QPushButton *yesalways = msgBox.addButton("Yes, and don't ask again.", QMessageBox::YesRole);
        msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Yes) {
            setting_put("chat_show_unreviewed", 1);
            if(TheConfigForm)
                TheConfigForm->load();
        } else if (ret == QMessageBox::No) {

        }
        setting_put("first25", 0);
    }
    if(First214)
    {
        First214 = 0;
        Block_DLL = 1;
        QMessageBox msgBox(QMessageBox::Information, "New Feature Alert",
                           "To make CDC-X work better, CDC-X "
                           "may archive saved chats if they are not recent. "
                           "Your old chats are not gone, they are simply temporarily "
                           "hidden. If CDC-X "
                           "has archived some of your chats, you will see a "
                           "<font color=\"#FF0000\">Archived X users</font> "
                           "in the status bar at the bottom. To view archived chats "
                           "click Windows|Show archived users...",
                           QMessageBox::Ok
                          );
        msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
        int ret = msgBox.exec();


        InitCleanThread ict;

        QProgressDialog prg("Checking messages... please be patient.", "Cancel", 0, 100, this);
        prg.setWindowModality(Qt::WindowModal);
        prg.setAutoReset(false);
        prg.setCancelButton(0);
        prg.show();
        ict.start();
        int total = 1;
        int done = 1;
        while(!ict.isFinished())
        {
            //dwyco_power_clean_progress_hack(&done, &total);

            if(total > 0)
                prg.setValue(done * 100 / total);
            if(prg.value() == 100)
            {
                prg.setLabelText("Scan done, just a little longer...");

            }
            if(prg.wasCanceled())
            {
                total = -1;
                //dwyco_power_clean_progress_hack(&done, &total);
                break;
            }
            QWaitCondition wc;
            QMutex mutex;
            QMutexLocker locker(&mutex);
            wc.wait(&mutex, 250);
            QApplication::processEvents();

        }

        if(total == -1)
        {
            while(!ict.isFinished())
                ;
        }
        prg.reset();
        Block_DLL = 0;
        setting_put("first214", 0);

    }

    static int chat_on = -1;
    if(dwyco_chat_online())
    {
        if(chat_on != 1)
        {

            if(Initial_chat_mute == 1)
            {
                dwyco_chat_mute(0, 1);
            }

            emit chat_server_status(1);
            chat_on = 1;
        }
    }
    else
    {
        if(chat_on != 0)
        {
            //Current_server_id = "";
            //Current_server = -1;
            emit chat_server_status(0);
            chat_on = 0;
        }

    }
    // note: the reason for all this static variable stuff in
    // here is because apparently it takes about 8% of a
    // 3ghz CPU to update a pixmap every 10ms (!!)
    static int db_on = -1;
    if(dwyco_database_online())
    {
        //server_status->setText("Online");
        if(db_on != 1)
        {
            server_status->setPixmap(QPixmap(":/new/prefix1/ggtiny.png"));
            db_on = 1;
        }
    }
    else
    {
        if(db_on != 0)
        {
            server_status->setText("Offline");
            db_on = 0;
        }
        if(DB_logged_in)
        {
            DB_logged_in = 0;
        }
    }

    static int db_auth = -1;
    if(dwyco_database_online() && dwyco_database_auth_remote())
    {
        //db_status->setText("Logged in");
        if(db_auth != 1)
        {
            db_status->setPixmap(QPixmap(":/new/prefix1/ggtiny.png"));
            db_auth = 1;
        }
    }
    else
    {
        if(db_auth != 0)
        {
            db_status->setText("Not logged in");
            db_auth = 0;
        }
    }

    static int cam_disp = -1;
    int cur_cam = (!!HasCamHardware << 2) | (!!HasCamera << 1) | !!is_video_streaming_out();

    if(cam_disp != cur_cam)
    {
        switch(cur_cam)
        {
        case 0:
            camera_status->setText("No Cam");
            break;
        case 6:
            camera_status->setText("Cam on");
            break;
        case 4:
            camera_status->setText("Cam off");
            break;
        case 7:
            camera_status->setText("Cam sending");
            break;
        case 5:
            camera_status->setText("Cam not sending");
            break;
        default:
            camera_status->setText("Cam ??");
            break;
        }
        cam_disp = cur_cam;
    }

    if(Initial_login_processing)
    {
        if(dwyco_get_create_new_account())
        {
#if 0
            loginform& lf = *new loginform;

            lf.show();
            lf.raise();
            lf.new_account();
            Block_DLL = 1;
            lf.exec();
            Block_DLL = 0;
            // temporary, don't rely on server since it is ignoring this anyway
            if(lf.save_pw())
            {
                // low security mode, just save the computer generated
                // password.
                save_low_sec(lf.gen_pw);
            }
            else
            {
                save_high_sec(lf.pw());
            }

            delete &lf;
#endif
            // just start out in low sec mode, they can change it
            // later.
            save_low_sec("");
        }
        dwyco_set_local_auth(1);
        dwyco_finish_startup();
        Initial_login_processing = 0;
        const char *uid;
        int len_uid;
        dwyco_get_my_uid(&uid, &len_uid);
        My_uid = DwOString(uid, 0, len_uid);
    }

    dwyco_service_channels(&spin);
#if 1
    static int last_spin;
    if(spin != last_spin)
    {
        if(spin)
        {
            timer.setInterval(1);
        }
        else
        {
            timer.setInterval(20);
        }
        last_spin = spin;
    }
#endif
    static int last_mute;
    int all_mute = dwyco_get_all_mute();
    if(last_mute != all_mute)
    {
        last_mute = all_mute;
        emit mic_mute_changed(all_mute);
    }
    if(!DB_logged_in)
        return;
    if(Ask_lobby_pw)
    {
        Ask_lobby_pw = 0;
        Block_DLL = 1;
        cspwform *cspw = new cspwform;
        cspw->ui.username_label->setText("The lobby you tried to enter");
        cspw->ui.label_2->setText("requires a password.");
        if(cspw->exec() == QDialog::Accepted)
        {
            QByteArray pw = cspw->ui.line_edit->text().toAscii();
            Current_lobby_pw = DwOString(pw.constData(), 0, pw.length());
            if(dwyco_check_chat_server_pw(Ask_lobby_pw_id.c_str(),
                                          Current_lobby_pw.c_str()) == -1)
            {
                QMessageBox::critical(this, "Password incorrect", "Lobby password incorrect");

            }
            else
                set_current_server(Ask_lobby_pw_id);
        }
        delete cspw;
        Block_DLL = 0;
    }
    static int initial_load;
    if(!initial_load)
    {
        reload_msgs();
        refetch_user_list();
        load_users();
        // trigger initial msg rescan in case background thingy
        // downloaded some messages
        dwyco_set_rescan_messages(1);

        //dwyco_get_server_list(&Dwyco_server_list, &dum);
        // note: this won't work because we don't yet have
        // the list of user defined servers. so we have to
        // just log into their last "regular" server, then let
        // them change to their user-defined server.
        if(Last_server != -1)
            set_current_server(Last_server);
        else if(!Last_server_id.eq(""))
            set_current_server(Last_server_id);

        decorate_users();
        initial_load = 1;

        // was having troubles with the camera initialization in windows
        // (big surprise).
        // i'm going to try and put the camera setup later in the
        // init process just as a guess
        if(!DevSelectBox)
        {
            DevSelectBox = new VidSel;
            DevSelectBox->init();
        }

        if(HasCamera)
        {
            camera_status->setText("Cam On");
            ui.actionShow_Video_Preview->setEnabled(1);
        }
        else
        {
            if(!HasCamHardware)
            {
                camera_status->setText("<font color=\"#FF0000\">No Cam</font>");
                vid_preview->hide();
            }
            else
                camera_status->setText("<font color=\"#FF0000\">Cam Off</font>");
            ui.actionShow_Video_Preview->setEnabled(0);

        }
    }

    DwOString uid;
    DwOString txt;
    DwOString mid;
    int viewer_id = 0;
    if(dwyco_get_rescan_messages())
    {
        dwyco_set_rescan_messages(0);
        while(dwyco_new_msg(uid, txt, viewer_id, mid))
        {
            display_msg(uid, txt, mid);
            cdcx_set_refresh_users(1);
            //emit new_msg(uid, txt, mid);
        }
    }
    if(cdcx_get_refresh_users())
    {
        cdcx_set_refresh_users(0);
        //chatform2::update_chat_displays();
        dwyco_load_users2(!Display_archived_users, 0);
        load_users();
        decorate_users();
        emit refresh_users();
    }
    if(Pal_relogin)
    {
        Pal_relogin = 0;
        dwyco_pal_relogin();
    }
}

void
mainwinform::animate()
{
    static int toggle;
    QHash<DwOString, DwOString>::const_iterator mi = Animate_uids.constBegin();
    for(; mi != Animate_uids.constEnd(); ++mi)
    {
        handle_animate_uid(mi.key(), mi.value(), toggle);
    }
    QHash<DwOString, font_animation_timer *>::const_iterator mi2 = Animate_uids_font.constBegin();
    for(; mi2 != Animate_uids_font.constEnd(); ++mi2)
    {
        handle_animate_uid_font(mi2.key(), mi2.value()->which, toggle);
    }
    static int stop_setting_icon;
    if(has_unviewed_msgs())
    {
        if(toggle)
            tray_icon->setIcon(*blank_tray_icon);
        else
            tray_icon->setIcon(*gg_tray_icon);
        stop_setting_icon = 0;
    }
    else
    {
        if(!stop_setting_icon)
        {
            tray_icon->setIcon(*gg_tray_icon);
            stop_setting_icon = 1;
        }
    }
    toggle = !toggle;
    int n = Reset_animations_uids.count();
    for(int i = 0; i < n; ++i)
    {
        handle_animate_uid(Reset_animations_uids[i], "", 0);
        handle_animate_uid_font(Reset_animations_uids[i], 0, 0);
    }
    Reset_animations_uids.clear();
    if(n > 0)
        load_users();
}

DwOString
dwyco_get_attr(DWYCO_LIST l, int row, const char *col)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        ::abort();
    if(type != DWYCO_TYPE_STRING && type != DWYCO_TYPE_NIL)
        ::abort();
    return DwOString(val, 0, len);
}

int
dwyco_get_attr_int(DWYCO_LIST l, int row, const char *col)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        ::abort();
    if(type != DWYCO_TYPE_INT)
        ::abort();
    return atoi(val);
}

static
int
display_uid_in_list(const DwOString& uid)
{
    if(Display_all_uid)
        return 1;
    if(uid_has_unviewed_msgs(uid))
        return 1;
    if(any_unread_msg(uid))
        return 1;
    if(dwyco_uid_online(uid.c_str(), uid.length()) || uid_in_lobby(uid))
        return 1;
    if(recent_uid_operation(uid))
        return 1;
    return 0;
}

QPixmap
circlize(QPixmap pm)
{
    QImage img = pm.toImage();
    img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage imageOut(img.size(),QImage::Format_ARGB32_Premultiplied);
    //painter on it
    QPainter painter(&imageOut);
    painter.eraseRect(imageOut.rect());
    //set opacity
//    painter.setOpacity(0);
//    painter.setBrush(Qt::white);
//    painter.setPen(Qt::NoPen);
//    //draw transparent image
//    painter.drawImage(0,0,img);
    //set opacity
    painter.setOpacity(1);
    QPainterPath path(QPointF(0,0));
    //your mask - ellipse
    path.addEllipse(0,0,pm.width(), pm.height());
    painter.setClipPath(path);
    //draw untransparent part of image
    painter.drawImage(0,0,img);
    return QPixmap::fromImage(imageOut);

}

// in the model, UserRole = uid, UserRole + 1 is the update number (for
// controlling deletes), UserRole + 2 indicates if the user can be "removed"
// (ie, you have messages from them. you can't remove users that are just in the list
// because they happen to be in the public chat)

void
mainwinform::load_users()
{
    DWYCO_LIST l;
    int n;

    sort_proxy_model.setDynamicSortFilter(0);
    static int cur_update;
    dwyco_get_user_list2(&l, &n);
    show_archive_status(n, User_count);
    int relogin = 0;
    for(int i = 0; i < n; ++i)
    {
        DwOString uid = dwyco_get_attr(l, i, DWYCO_NO_COLUMN);
#if 0
        // filter out pals, so they aren't listed twice
        if(dwyco_is_pal(uid.c_str(), uid.length()))
            continue;
#endif
        if(!display_uid_in_list(uid))
            continue;
        QVariant quid(QByteArray(uid.c_str(), uid.length()));

        QModelIndexList ql = umodel->match(umodel->index(0, 0), Qt::UserRole, quid, 1, Qt::MatchExactly);
        if(ql.count() == 0)
        {
            if(!umodel->insertRow(0))
                cdcxpanic("wtf");
            if(!umodel->setData(umodel->index(0, 0), dwyco_info_to_display(uid)))
                cdcxpanic("wtf2");
            if(!umodel->setData(umodel->index(0, 0), quid, Qt::UserRole))
                cdcxpanic("wtf3");
            if(!umodel->setData(umodel->index(0, 0), cur_update, Qt::UserRole + 1))
                cdcxpanic("wtf4");
            relogin = 1;
        }
        else if(ql.count() == 1)
        {
            if(!umodel->setData(ql[0], cur_update, Qt::UserRole + 1))
                cdcxpanic("wtf5");
            QString info = dwyco_info_to_display(uid);
            auto disp_name = umodel->data(ql[0]).toString();
            if(info == disp_name)
                continue;
            else
            {
                if(!umodel->setData(ql[0], info, Qt::DisplayRole))
                    cdcxpanic("wtf6");
            }

        }
        else
            cdcxpanic("multiple uids on list");
    }

    dwyco_list_release(l);

    // unless i change the dll to make pals into an attribute like it was
    // in the distant past, there could be pals that are not in the user list.
    // add them here.
    l = dwyco_pal_get_list();
    dwyco_list_numelems(l, &n, 0);

    for(int i = 0; i < n; ++i)
    {
        DwOString uid = dwyco_get_attr(l, i, DWYCO_NO_COLUMN);
        if(!display_uid_in_list(uid))
            continue;
        QVariant quid(QByteArray(uid.c_str(), uid.length()));
        QModelIndexList ql = umodel->match(umodel->index(0, 0), Qt::UserRole, quid, 1, Qt::MatchExactly);
        if(ql.count() == 0)
        {
            if(!umodel->insertRow(0))
                cdcxpanic("wtf");
            if(!umodel->setData(umodel->index(0, 0), dwyco_info_to_display(uid)))
                cdcxpanic("wtf2");
            if(!umodel->setData(umodel->index(0, 0), quid, Qt::UserRole))
                cdcxpanic("wtf3");
            if(!umodel->setData(umodel->index(0, 0), cur_update, Qt::UserRole + 1))
                cdcxpanic("wtf4");
            relogin = 1;

        }
        else
        {
            // it's already in there, check to see if the info has changed
            if(!umodel->setData(ql[0], cur_update, Qt::UserRole + 1))
                cdcxpanic("wtf5");
            QString info = dwyco_info_to_display(uid);
            auto disp_name = umodel->data(ql[0]).toString();
            if(info == disp_name)
                continue;
            else
            {
                if(!umodel->setData(ql[0], info, Qt::DisplayRole))
                    cdcxpanic("wtf4");
            }

        }
    }

    if(Display_chat_users)
    {
        // add users that are in the current lobby
        QList<DwOString> lobby_users = uid_attrs_get_uids();
        n = lobby_users.count();
        for(int i = 0; i < n; ++i)
        {
            DwOString uid = lobby_users[i];
            if(!display_uid_in_list(uid))
                continue;
            if(Session_remove.contains(uid))
                continue;
            QVariant quid(QByteArray(uid.c_str(), uid.length()));
            QModelIndexList ql = umodel->match(umodel->index(0, 0), Qt::UserRole, quid, 1, Qt::MatchExactly);
            if(ql.count() == 0)
            {
                if(!umodel->insertRow(0))
                    cdcxpanic("wtf");
                if(!umodel->setData(umodel->index(0, 0), dwyco_info_to_display(uid)))
                    cdcxpanic("wtf2");
                if(!umodel->setData(umodel->index(0, 0), quid, Qt::UserRole))
                    cdcxpanic("wtf3");
                if(!umodel->setData(umodel->index(0, 0), cur_update, Qt::UserRole + 1))
                    cdcxpanic("wtf4");
            }
            else
            {
                // it's already in there, check to see if the info has changed
                if(!umodel->setData(ql[0], cur_update, Qt::UserRole + 1))
                    cdcxpanic("wtf5");
                QString info = dwyco_info_to_display(uid);
                auto disp_name = umodel->data(ql[0]).toString();
                if(info == disp_name)
                    continue;
                else
                {
                    if(!umodel->setData(ql[0], info, Qt::DisplayRole))
                        cdcxpanic("wtf4");
                }
            }
        }

        // add last 30 or so users we have seen come through the chat server
        n = Seen_in_chat.count();
        for(int i = 0; i < n; ++i)
        {
            DwOString uid = Seen_in_chat[i];
#if 0
            if(!display_uid_in_list(uid))
                continue;
#endif
            if(Session_remove.contains(uid))
                continue;
            QVariant quid(uid);
            QModelIndexList ql = umodel->match(umodel->index(0, 0), Qt::UserRole, quid, 1, Qt::MatchExactly);
            if(ql.count() == 0)
            {
                if(!umodel->insertRow(0))
                    cdcxpanic("wtf");
                if(!umodel->setData(umodel->index(0, 0), dwyco_info_to_display(uid)))
                    cdcxpanic("wtf2");
                if(!umodel->setData(umodel->index(0, 0), quid, Qt::UserRole))
                    cdcxpanic("wtf3");
                if(!umodel->setData(umodel->index(0, 0), cur_update, Qt::UserRole + 1))
                    cdcxpanic("wtf4");
            }
            else
            {
                // it's already in there, check to see if the info has changed
                if(!umodel->setData(ql[0], cur_update, Qt::UserRole + 1))
                    cdcxpanic("wtf5");
                QString info = dwyco_info_to_display(uid);
                auto disp_name = umodel->data(ql[0]).toString();
                if(info == disp_name)
                    continue;
                else
                {
                    if(!umodel->setData(ql[0], info, Qt::DisplayRole))
                        cdcxpanic("wtf4");
                }
            }
        }
    }



    // scan the model and find all the entries that have not been updated
    // to the latest cur_update and delete them
    n = umodel->rowCount();
    for(int i = 0; i < n; ++i)
    {
        QModelIndex idx = umodel->index(i, 0);
        QVariant cu = umodel->data(idx, Qt::UserRole + 1);
        if(cu.toInt() < cur_update)
        {
            QVariant quid = umodel->data(idx, Qt::UserRole);
            QByteArray b = quid.toByteArray();
            DwOString bo(b.constBegin(), 0, b.length());
            if(Animate_uids_font.contains(bo))
                continue;
            umodel->removeRow(i);
            --i;
            --n;
            relogin = 1;
        }

    }

    sort_proxy_model.invalidate();
    sort_proxy_model.sort(0);
    sort_proxy_model.setDynamicSortFilter(1);
    dwyco_list_release(l);
    ++cur_update;
    if(relogin)
        dwyco_pal_relogin();

}


static void
decorate(const DwOString& uid, QStandardItemModel *umodel, QModelIndex mi)
{
    int inhibit_default_decorations = 0;

    QStandardItem *si = umodel->itemFromIndex(mi);
    QFont fnt = si->font();
    fnt.setItalic(0);
    fnt.setBold(0);
    fnt.setStrikeOut(0);
    int uid_from_online = 0;
    int uid_from_lobby = 0;
    int sz = Display_pics_in_user_list ? 32 : 16;
    if(is_streaming_uid(uid))
    {
        umodel->setData(mi, QImage(":/new/red32/icons/PrimaryCons_Red_KenSaunders/PNGs/32x32/Eye-32x32.png").scaled(QSize(sz, sz)), Qt::DecorationRole);
        inhibit_default_decorations = 1;
        umodel->setData(mi, QColor(255, 128, 242), Qt::ForegroundRole);
    }
    else if((uid_from_online = dwyco_uid_online(uid.c_str(), uid.length())) || (uid_from_lobby = uid_in_lobby(uid)))
    {
        if(uid_has_unviewed_msgs(uid))
        {
            umodel->setData(mi, QImage(":/new/red32/icons/PrimaryCons_Red_KenSaunders/PNGs/32x32/arrow right-32x32.png").scaled(QSize(sz, sz)), Qt::DecorationRole);
            inhibit_default_decorations = 1;
            umodel->setData(mi, QColor(255, 128, 242), Qt::ForegroundRole);
        }
        else
        {
            if(uid_from_online)
                umodel->setData(mi, QColor(Qt::red), Qt::ForegroundRole);
            else
                umodel->setData(mi, QColor(Qt::darkMagenta), Qt::ForegroundRole);
            if(Display_pics_in_user_list)
            {
                QPixmap pm = ThePreviewCache->get_preview_by_uid(uid);
                if(!pm.isNull())
                {
                    umodel->setData(mi, circlize(pm.scaled(32, 32)), Qt::DecorationRole);
                    inhibit_default_decorations = 1;
                }
                else
                {
                    umodel->setData(mi, QVariant(), Qt::DecorationRole);
                }
                if(dwyco_is_pal(uid.c_str(), uid.length()))
                {
                    fnt.setBold(1);
                }
            }
        }

        if(uid_active(uid))
        {
            fnt.setItalic(1);
        }
    }
    else
    {
        if(uid_has_unviewed_msgs(uid))
        {
            umodel->setData(mi, QImage(":/new/red32/icons/PrimaryCons_Red_KenSaunders/PNGs/32x32/arrow right-32x32.png").scaled(QSize(sz, sz)), Qt::DecorationRole);
            inhibit_default_decorations = 1;
            umodel->setData(mi, QColor(Qt::blue), Qt::ForegroundRole);
        }
        else
        {
            umodel->setData(mi, QColor(Qt::black), Qt::ForegroundRole);
            if(Display_pics_in_user_list)
            {
                QPixmap pm = ThePreviewCache->get_preview_by_uid(uid);
                if(!pm.isNull())
                {
                    umodel->setData(mi, circlize(pm.scaled(32, 32)), Qt::DecorationRole);
                    inhibit_default_decorations = 1;
                }
                else
                {
                    umodel->setData(mi, QVariant(), Qt::DecorationRole);
                }
                if(dwyco_is_pal(uid.c_str(), uid.length()))
                {
                    fnt.setBold(1);
                }
            }

        }
    }
    if(dwyco_is_ignored(uid.c_str(), uid.length()))
    {
        umodel->setData(mi, QImage(":/new/prefix1/stop.png").scaled(QSize(12, 12)), Qt::DecorationRole);
    }
    else if(!inhibit_default_decorations)
    {
        if(dwyco_is_pal(uid.c_str(), uid.length()))
        {
            umodel->setData(mi, QImage(":/new/prefix1/ggtiny.png").scaled(QSize(12, 12)), Qt::DecorationRole);
        }
        else
#if 0
            if(dwyco_is_always_visible(uid.c_str(), uid.length()))
            {
                umodel->setData(mi, QImage(":/new/prefix1/ggtiny.png").scaled(QSize(12, 12)), Qt::DecorationRole);
            }
            else if(dwyco_is_never_visible(uid.c_str(), uid.length()))
            {
                umodel->setData(mi, QImage(":/new/prefix1/ggtiny-nevervis.png").scaled(QSize(12, 12)), Qt::DecorationRole);
            }
            else
#endif
            {
                umodel->setData(mi, QVariant(), Qt::DecorationRole);
            }
    }
    si->setFont(fnt);
}

// note: on decorate user, we don't re-sort the list, which
// is different than "decorate_users"... in general, there could
// be an ordering change needed after a decorate_user, but it is
// expensive to resort every time, and the order is just advisory
// anyways.
void
mainwinform::decorate_user(const DwOString& uid)
{
    QVariant quid(QByteArray(uid.c_str(), uid.length()));
    QModelIndexList ql = umodel->match(umodel->index(0, 0), Qt::UserRole, quid, 1, Qt::MatchExactly);
    QModelIndex mi;
    if(ql.count() == 0)
    {
        return;
    }
    else
    {
        mi = ql[0];
    }
    if(ql.count() > 1)
        cdcxpanic("multiple uids on list");
    decorate(uid, umodel, mi);
}

void
mainwinform::handle_animate_uid(const DwOString& uid, const DwOString& icon, int toggle)
{
    QVariant quid(QByteArray(uid.c_str(), uid.length()));

    QModelIndexList ql = umodel->match(umodel->index(0, 0), Qt::UserRole, quid, 1, Qt::MatchExactly);
    if(ql.count() == 0)
    {
        return;
    }
    if(ql.count() > 1)
        cdcxpanic("multiple uids on list");
    QModelIndex mi = ql[0];
    int sz = Display_pics_in_user_list ? 32 : 16;
    if(toggle)
    {
        umodel->setData(mi, QImage(icon.c_str()).scaled(QSize(sz, sz)), Qt::DecorationRole);
    }
    else
    {
        decorate(uid, umodel, mi);
    }
}

void
mainwinform::handle_animate_uid_font(const DwOString& uid, int what, int toggle)
{
    QVariant quid(QByteArray(uid.c_str(), uid.length()));

    QModelIndexList ql = umodel->match(umodel->index(0, 0), Qt::UserRole, quid, 1, Qt::MatchExactly);
    if(ql.count() == 0)
    {
        return;
    }
    if(ql.count() > 1)
        cdcxpanic("multiple uids on list");
    QModelIndex mi = ql[0];
    QStandardItem *si = umodel->itemFromIndex(mi);
    QFont fnt = si->font();
    if(what == 0)
    {
        fnt.setBold(toggle);
        fnt.setStrikeOut(0);
    }
    else if(what == 1)
    {
        fnt.setBold(0);
        fnt.setStrikeOut(toggle);
    }
    si->setFont(fnt);
}

void
mainwinform::add_call(const DwOString& uid, int chan_id)
{

}

void
mainwinform::del_call(const DwOString& uid, int chan_id)
{

}


void
mainwinform::decorate_users()
{
    sort_proxy_model.setDynamicSortFilter(0);
    int mn = umodel->rowCount();
    for(int i = 0; i < mn; ++i)
    {
        QModelIndex mi = umodel->index(i, 0);
        QByteArray quid = umodel->data(mi, Qt::UserRole).toByteArray();
        DwOString uid(quid.constData(), 0, quid.count());
        decorate(uid, umodel, mi);
    }

    // no need to sort, since decorations are not included in the
    // sort function
    sort_proxy_model.invalidate();
    sort_proxy_model.sort(0);
    sort_proxy_model.setDynamicSortFilter(1);
}


// remember: the stuff that comes in from the cdclibs
// has been contaminated by microsoft. they are
// upside down, and bgr... sigh
static
QImage
dwyco_img_to_qimg(void *vimg, int cols, int rows, int depth)
{
    unsigned char **img = (unsigned char **)vimg;

    QImage qi(cols, rows, QImage::Format_RGB888);

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
    return qi;
}

void
DWYCOCALLCONV
dwyco_video_display(int ui_id, void *vimg, int cols, int rows, int depth)
{
    // don't do grayscale for the moment
    if(depth != 3)
        return;

    QImage qi = dwyco_img_to_qimg(vimg, cols, rows, depth);
    if(ui_id == DWYCO_VIDEO_PREVIEW_CHAN)
    {
        if(!Public_chat_video_pause)
        {
            Last_preview_image = qi;
        }
        Mainwinform-> emit video_capture_preview(qi);

    }
    Mainwinform-> emit video_display(ui_id, qi);
}

#if 0
void
DWYCOCALLCONV
dwyco_unregister(int what, const char *msg, int, void *)
{
    DwOString a(msg);
    int i;
    if((i = a.find(":")) != DwOString::npos)
        a.remove(0, i + 1);
#ifndef ICUII_CHAT
    // in cdcx, when the trial period expires, nothing happens
    // except the advanced features become unusable
    if(what == DWYCO_UNREG_TRIAL_EXPIRED)
    {
        // maybe do something local here
        // update status bar
        Mainwinform->lic_status->setText("<font color=\"#FF0000\">Trial Expired</font>");
        return;
    }
#endif
    QMessageBox mb(QMessageBox::Critical, "Subscription Required", a.c_str());
    QAbstractButton *bsb = mb.addButton("Buy Subscription...", QMessageBox::AcceptRole);
    QAbstractButton *rcb = mb.addButton("Enter Subscription Code...", QMessageBox::AcceptRole);
    QAbstractButton *eb = mb.addButton("Quit program...", QMessageBox::RejectRole);
    mb.exec();
    if(mb.clickedButton() == rcb)
    {
        entercodeform e;
        e.exec();
        DieDieDie = 1;
    }
    else if(mb.clickedButton() == eb)
    {
        DieDieDie = 1;
    }
    else if(mb.clickedButton() == bsb)
    {
        QDesktopServices::openUrl(QUrl("http://www.dwyco.com/order.html"));
        DieDieDie = 1;
    }
    else
        cdcxpanic("bogus return");
}
#endif

void
DWYCOCALLCONV
dwyco_call_bandwidth_callback(int chan_id, const char *txt, int, void *)
{
#if 0
    chatform2 *c = 0;
    c = chatform2::chatform2_by_member(ui_id, &chatform2::call_them_ui_id);
    if(!c)
        return;
    c->ui.statusbar->showMessage(txt, 10000);
#endif
    QList<simple_call *> scl = simple_call::Simple_calls.query_by_member(chan_id, &simple_call::chan_id);
    if(scl.count() != 1)
        return;
    scl[0]->ui->debug_label->setText(txt);
}

// warning: relying on the simple_call context being valid until
// the call is terminated is a bit of a problem. it is ok for now,
// but maybe need to avoid deleting the sc until the call has
// died. problems in: remove user processing, and ignore processing
void
DWYCOCALLCONV
call_died(int chan_id, void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    simple_call *sc = (simple_call *)(void *)vp;

    stop_animation(sc->uid);
    Mainwinform->del_call(sc->uid, chan_id);
    call_del(chan_id);
    sc->chan_id = -1;
    sc->call_id = -1;
    // we know we stopped it before because we accepted a call from it.
    sc->on_hangup_clicked();
    sc->connect_state_machine->start();

    Mainwinform-> emit channel_died(chan_id, sc->uid);

}


void
DWYCOCALLCONV
dwyco_call_accepted(int chan_id, const char *name, const char *location, const char *uid, int len_uid, const char *call_type, int len_call_type)
{
    DwOString suid(uid, 0, len_uid);

    DwOString dct(call_type, 0, len_call_type);

    simple_call *sc = simple_call::get_simple_call(suid);

    dwyco_set_channel_destroy_callback(chan_id, call_died, (void *)sc->vp.cookie);
    Mainwinform->add_call(suid, chan_id);
    stop_animation(suid);
    dwyco_call_type ct(chan_id, call_type, len_call_type, uid, len_uid);

    if(!call_find(chan_id, ct))
    {
        call_add(ct);
    }
    sc->chan_id = chan_id;
    // note: once we receive a call successfully, we turn ourselves
    // into a slave, and avoid trying to set up control call from this
    // end.
    sc->connect_state_machine->stop();
    sc->call_setup_state_machine->start();
    sc->mute_state_machine->start();
    sc->set_connection_status_display(1);
}


int
DWYCOCALLCONV
dwyco_call_screening_callback(int chan_id,
                              int remote_wants_to_recv_your_video, int remote_wants_to_send_you_video,
                              int remote_wants_to_recv_your_audio, int remote_wants_to_send_you_audio,
                              int remote_wants_to_exchange_pubchat, int remote_wants_to_exchange_privchat,
                              const char *call_type, int len_call_type,
                              const char *uid, int len_uid,
                              int *accept_call_style,
                              char **error_msg)
{
    DwOString suid(uid, 0, len_uid);
    DwOString ct(call_type, 0, len_call_type);

    // need to check if ignore processing is handled in the dll anymore
    if(dwyco_is_ignored(uid, len_uid))
    {
        *accept_call_style = DWYCO_CSC_REJECT_CALL;
        return 0;
    }
    // new style: the "call" is just a set up of the channels without
    // any streaming going, so we just test for duplicates (not allowed.)
    if(call_exists_by_uid(suid))
    {
        *accept_call_style = DWYCO_CSC_REJECT_CALL;
        return 0;
    }
    // if we are invisible, and we are not "visible to user", then
    // reject the call. note this is an info leak if it got this far,
    // but better than allowing the call.
    if(dwyco_get_invisible_state())
    {
        if(!Visible_to.contains(suid))
        {
            *accept_call_style = DWYCO_CSC_REJECT_CALL;
            return 0;
        }
    }

    if(dwyco_get_pals_only() && !dwyco_is_pal(uid, len_uid))
    {
        *accept_call_style = DWYCO_CSC_REJECT_CALL;
        return 0;
    }
    dwyco_call_type cto(chan_id, call_type, len_call_type, uid, len_uid, DWYCO_CT_RECV);
    call_add(cto);
    *accept_call_style = DWYCO_CSC_ACCEPT_CALL;
    return 0;

}


void
DWYCOCALLCONV
dwyco_user_control(int chan_id, const char *suid, int len_uid, const char *data, int len_data)
{
    DwOString uid(suid, 0, len_uid);
    DwOString com(data, 0, len_data);
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
    Mainwinform->emit user_control(chan_id, uid, com);
}


void
mainwinform::on_actionCheck_for_Updates_triggered(bool)
{
    extern autoupdateform *The_autoupdateform;

    The_autoupdateform->show();
    The_autoupdateform->raise();
    The_autoupdateform->forced = 1;

    dwyco_force_autoupdate_check();
}

void
mainwinform::on_actionDiagnostics_triggered(bool)
{
    char *b;
    int len;
    dwyco_network_diagnostics2(&b, &len);

    composer *c = new composer;
    c->show();
    c->raise();
    c->ui.textEdit->setPlainText(QByteArray(b, len));
    dwyco_free_array(b);
    // need a set recipients call to set the god users
    c->set_uid(TheManUID);

}

void
mainwinform::on_actionBandwidth_test_triggered(bool)
{
    int bw_in;
    int bw_out;

    dwyco_estimate_bandwidth2(&bw_out, &bw_in);
    QString a = QString("Download %1 kbps, Upload %2 kbps").arg(QString::number(bw_in / 1000), QString::number(bw_out / 1000));
    QMessageBox::information(0, "Bandwidth report", a);

}

void
mainwinform::on_actionShow_Directory_triggered(bool state)
{
#ifdef CDCX_WEBKIT
    dir_listing->setVisible(1);
    dir_listing->setFloating(0);
    dir_listing->setFocus();
    dir_listing->raise();
    ui.actionShow_Directory->setVisible(0);
#endif
}


void mainwinform::on_actionShow_Profile_Grid_triggered(bool state)
{
#ifdef CDCX_WEBKIT
    browse_box->setVisible(1);
    browse_box->setFloating(0);
    browse_box->setFocus();
    browse_box->raise();
    ui.actionShow_Profile_Grid->setVisible(0);
#endif

}
void mainwinform::on_actionShow_Public_Chat_triggered(bool state)
{
    pubchat->setVisible(1);
    pubchat->setFloating(0);
    pubchat->setFocus();
    pubchat->raise();
    ui.actionShow_Public_Chat->setVisible(0);
}

void
mainwinform::on_actionShow_Video_Preview_triggered(bool state)
{
    vid_preview->setVisible(1);
    vid_preview->setFloating(0);
    vid_preview->setFocus();
    vid_preview->raise();
    ui.actionShow_Video_Preview->setVisible(0);
}

void
mainwinform::on_actionChange_password_triggered(bool)
{
    ThePwForm->reset();
    ThePwForm->exec();
    //ThePwForm->raise();
}


int
DWYCOCALLCONV
dwyco_init_public_chat(int ui_id)
{
    if(simple_public::Simple_publics.count() > 0)
    {
        simple_public *s = (simple_public *)(void*)simple_public::Simple_publics[0];
        s->ui_id = ui_id;
    }
    return 0;
}



int
DWYCOCALLCONV
dwyco_display_public_chat(const char *who, int len_who, const char *txt, int len_txt, const char *uid, int len_uid)
{
    Mainwinform->emit_public_chat_display(DwOString(who, 0, len_who), DwOString(txt, 0, len_txt), DwOString(uid, 0, len_uid));
    return 1;
}

void
DWYCOCALLCONV
dwyco_sys_event_callback(int cmd, int id,
                         const char *uid, int len_uid,
                         const char *name, int len_name,
                         int type, const char *val, int len_val,
                         int qid,
                         int extra_arg)
{
    if(!uid)
        return;
    DwOString suid(uid, 0, len_uid);
    DwOString str_data;
    if(type == DWYCO_TYPE_STRING)
        str_data = DwOString(val, 0, len_val);
    DwOString namestr;
    if(name)
    {
        namestr = DwOString(name, 0, len_name);
    }

    if(cmd == DWYCO_SE_USER_UID_RESOLVED)
    {
        Mainwinform-> emit uid_info_event(suid);
    }
    else if(cmd == DWYCO_SE_USER_PROFILE_INVALIDATE)
    {
        Mainwinform->emit invalidate_profile(suid);
    }
    switch(cmd)
    {
    case DWYCO_SE_MSG_SEND_START:
    case DWYCO_SE_MSG_SEND_FAIL:
    case DWYCO_SE_MSG_SEND_SUCCESS:
    case DWYCO_SE_MSG_SEND_CANCELED:
        Mainwinform->emit msg_send_status(cmd, str_data, suid);
        break;
    case DWYCO_SE_MSG_SEND_STATUS:
        Mainwinform->emit msg_progress(str_data, suid, namestr, extra_arg);
        break;
    case DWYCO_SE_USER_STATUS_CHANGE:
        Mainwinform->emit uid_status_change(suid);
        break;
    case DWYCO_SE_GRP_JOIN_OK:
        dwyco_set_setting("group/alt_name", namestr.c_str());
        QMessageBox::information(Mainwinform, "Device group changed", QString("Linked to %1, CDC-X must quit now").
                                 arg(namestr.length() == 0 ? "<no group>" : namestr.c_str()));
        DieDieDie = 1;
        break;
    case DWYCO_SE_GRP_JOIN_FAIL:
        dwyco_set_setting("group/alt_name", "");
        QMessageBox::information(Mainwinform, "Device group change failed", QString("Account linking failed, try again later (%1)").arg(namestr.c_str()));
        break;
    default:
        break;
    }
}

void
mainwinform::emit_public_chat_display(DwOString who, DwOString txt, DwOString uid)
{
    emit public_chat_display(who, txt, uid);
}

static
const char *
dwyco_col(int i)
{
    static char a[30];
    sprintf(a, "%03d", i);
    return a;
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
            ::cdcxpanic("bogus dwyco int");
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
            ::cdcxpanic("multidimensional array unimp");
        for(int i = 0; i < n; ++i)
        {
            const char *tmpo;
            int tmplen;
            int tmptype;
            if(!dwyco_list_get(dl, 0, dwyco_col(i), &tmpo, &tmplen, &tmptype))
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
mainwinform::emit_chat_event(int cmd, int id, const char *uid, int len_uid, const char *name, int len_name,
                             int type, const char *val, int len_val,
                             int qid, int extra_arg)
{
    QByteArray buid(uid, len_uid);
    QString sname(QByteArray(name, len_name));
    QVariant v = dwyco_type_to_qv(type, val, len_val);
    emit chat_event(cmd, id, buid, sname, v, qid, extra_arg);
}



void
DWYCOCALLCONV
dwyco_chat_ctx_callback(int cmd, int id,
                        const char *uid, int len_uid,
                        const char *name, int len_name,
                        int type, const char *val, int len_val,
                        int qid,
                        int extra_arg)
{
    DwOString dname;
    if(name)
        dname = DwOString(name, 0, len_name);
    DwOString duid;
    QByteArray buid;
    if(uid)
    {
        duid = DwOString(uid, 0, len_uid);
        buid = QByteArray(uid, len_uid);
    }
    DwOString dval;
    if(type == DWYCO_TYPE_STRING)
        dval = DwOString(val, 0, len_val);
    static int Batch;
    int Me = 0;
    if(duid == My_uid)
        Me = 1;

    Mainwinform->emit_chat_event(cmd, id, uid, len_uid, name, len_name,
                                 type, val, len_val,
                                 qid, extra_arg);

#if 0
// note: can't access val that way without causing crashes in some cases
    printf("chat ctx cmd %d id %d uid %s name %s type %d val %s qid %d\n",
           cmd, id, to_hex(duid).c_str(), dname.c_str(), type, val, qid);
#endif

    switch(cmd)
    {
    case DWYCO_CHAT_CTX_NEW:
        uid_attrs_clear();
        user_lobbies_clear();
        Mainwinform-> emit admin_event(0);
        Mainwinform-> emit clear_user_lobbies();
        Mainwinform->emit set_refresh_user(duid);
        break;

    case DWYCO_CHAT_CTX_DEL:
    {
        // note: might want to just keep the attrs around
        // since the info might be useful even if you are not
        // logged into a chat server.
        //Uid_attrs.clear();
        break;
    }
    case DWYCO_CHAT_CTX_ADD_LOBBY:
    {
        user_lobby ul;
        DWYCO_LIST l = (DWYCO_LIST)val;
        ul.id = dwyco_get_attr(l, 0, DWYCO_SL_ULOBBY_ID);
        ul.hostname = dwyco_get_attr(l, 0, DWYCO_SL_ULOBBY_HOSTNAME);
        ul.ip = dwyco_get_attr(l, 0, DWYCO_SL_ULOBBY_IP);
        ul.port = dwyco_get_attr_int(l, 0, DWYCO_SL_ULOBBY_PORT);
        ul.rating = dwyco_get_attr(l, 0, DWYCO_SL_ULOBBY_RATING);
        ul.dispname = dwyco_get_attr(l, 0, DWYCO_SL_ULOBBY_DISPLAY_NAME);
        ul.category = dwyco_get_attr(l, 0, DWYCO_SL_ULOBBY_CATEGORY);
        ul.subgod_uid = dwyco_get_attr(l, 0, DWYCO_SL_ULOBBY_SUBGOD_UID);
        ul.max_users = dwyco_get_attr_int(l, 0, DWYCO_SL_ULOBBY_MAX_USERS);
        user_lobbies_add(ul);
        if(Mainwinform)
            Mainwinform->emit add_user_lobby(ul);
    }
    break;
    case DWYCO_CHAT_CTX_DEL_LOBBY:
    {
        user_lobbies_del(dval.c_str());
        if(Mainwinform)
            Mainwinform->emit del_user_lobby(dval);
    }
    break;

    case DWYCO_CHAT_CTX_ADD_USER:
        // clear all previous attrs
        uid_attrs_del_uid(duid);
        uid_attrs_add(duid, "added");
        add_seen_in_chat(duid);
        Mainwinform->decorate_user(duid);
        Mainwinform->emit set_refresh_user(duid);
        start_font_animation(duid, 0);

        break;

    case DWYCO_CHAT_CTX_DEL_USER:
        uid_attrs_del_uid(duid);
        Mainwinform->emit set_refresh_user(duid);
        start_font_animation(duid, 1);
        break;

    case DWYCO_CHAT_CTX_UPDATE_AH:
        break;
    case DWYCO_CHAT_CTX_START_UPDATE:
        Batch = 1;
        break;
    case DWYCO_CHAT_CTX_END_UPDATE:
        Batch = 0;
        break;
    case DWYCO_CHAT_CTX_Q_ADD:
    {
    }
    break;
    case DWYCO_CHAT_CTX_Q_DEL:
    {
    }
    break;
    case DWYCO_CHAT_CTX_Q_GRANT:
    {
    }
    break;
    case DWYCO_CHAT_CTX_ADD_GOD:
    {
        godrec g;
        DWYCO_LIST l = (DWYCO_LIST)val;
        g.uid = buid;
        g.name = dwyco_get_attr(l, 0, DWYCO_GT_NAME).c_str();
        g.where = dwyco_get_attr(l, 0, DWYCO_GT_WHERE).c_str();
        g.server_id = dwyco_get_attr(l, 0, DWYCO_GT_SERVER_ID).c_str();
        g.god = dwyco_get_attr(l, 0, DWYCO_GT_GOD).length() > 0;
        g.demigod = dwyco_get_attr(l, 0, DWYCO_GT_DEMIGOD).length() > 0;
        g.subgod = dwyco_get_attr(l, 0, DWYCO_GT_SUBGOD).length() > 0;
        Gods.insert(buid, g);

        Mainwinform->emit god_add(g);
    }
    break;
    case DWYCO_CHAT_CTX_DEL_GOD:
    {
        Gods.remove(buid);
        Mainwinform->emit god_del(buid);
    }
    break;

    case DWYCO_CHAT_CTX_SYS_ATTR:
    {
        if(dname.eq("gods-only"))
        {
        }
        else if(dname.eq("demigods-only"))
        {
        }
        else if(dname.eq("uid-only"))
        {
        }
        else if(dname.eq("us-server-rules"))
        {
        }
        else if(dname.eq("us-audio-codec-chats"))
        {
        }
        else if(dname.eq("us-proxy-bw-limit"))
        {
        }
        else if(dname.eq("popup"))
        {
            DWYCO_LIST dl = (DWYCO_LIST)val;

            DwOString uid = dwyco_get_attr(dl, 0, DWYCO_POPUP_FROM_UID);
            DwOString handle = dwyco_get_attr(dl, 0, DWYCO_POPUP_FROM_HANDLE);
            DwOString msg = dwyco_get_attr(dl, 0, DWYCO_POPUP_MSG);
            QString a = QString("<font color=\"#ff00ff\">popup from %1 \"%2\" \"%3\"</font>")
                        .arg(to_hex(uid).c_str())
                        .arg(handle.c_str())
                        .arg(msg.c_str())
                        ;
            //append_msg_to_textedit(a, c->ui.text_display, "");
            //cursor_to_bottom(c->ui.text_display);
        }
        break;
    }

    case DWYCO_CHAT_CTX_UPDATE_ATTR:
    {
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
        else if(dname.eq("ui-activity"))
        {
            if(type == DWYCO_TYPE_NIL)
                uid_attrs_del(duid, dname);
            else
                uid_attrs_add(duid, dname);
            Mainwinform->emit set_refresh_user(duid);
        }
        else
        {
        }

        break;
    }
    default:
        break;
    }

}
void
DWYCOCALLCONV
dwyco_chat_ctx_callback2(int cmd, int id,
                         const char *uid, int len_uid,
                         const char *name, int len_name,
                         DWYCO_LIST lst,
                         int qid,
                         int extra_arg)
{
    DwOString dname(name, 0, len_name);
    static int Batch;

    switch(cmd)
    {
    case DWYCO_CHAT_CTX_SYS_ATTR:
    {
        if(dname.eq("ui-server-list"))
        {
            if(Dwyco_server_list)
                dwyco_list_release(Dwyco_server_list);
            Dwyco_server_list = dwyco_list_copy(lst);
            if(Adminform)
                Adminform->replace_servers(Dwyco_server_list);
        }
        break;
    }

    default:
        break;
    }

}

#if 0
void
DWYCOCALLCONV
dwyco_chat_server_status_callback(int id, const char *msg, int /*percent_done*/, void * /*user_arg*/)
{
    if(strcmp(msg, "online") == 0)
    {
        if(Initial_chat_mute == 1)
        {
            dwyco_chat_mute(0, 1);
        }
        int ard = 1;
        if(!setting_get("auto-refresh-directory", ard) ||
                ard == 1)
        {
            //dwyco_refresh_directory();
        }
        Mainwinform-> emit chat_server_status(id, 1);
    }
    else if(strcmp(msg, "offline") == 0)
    {
        // issue offline stuff
        // don't clear the lobbies when we go offline, since we
        // may need the info to get into another user defined lobby.
        //if(Mainwinform)
        //    Mainwinform->emit_clear_lobbies_event();
        //user_lobbies_clear();
        Current_server_id = "";
        Current_server = -1;
        Mainwinform-> emit chat_server_status(id, 0);
    }
}
#endif

void
DWYCOCALLCONV
dwyco_emergency_callback(int problem, int must_exit, const char *dll_msg)
{

    Block_DLL = 1;
    if(problem == DWYCO_EMERGENCY_DB_CHANGE)
    {
        QMessageBox::information(Mainwinform, "Server Changes", dll_msg);
        if(must_exit)
            DieDieDie = 1;
        return;
    }
#if 0
#ifndef CDCX_RELEASE
    ::abort();
#endif
#endif
    QMessageBox::information(Mainwinform, "Unrecoverable error", dll_msg);
    if(must_exit)
        DieDieDie = 1;
}

#if 0
void
DWYCOCALLCONV
dwyco_pal_auth_callback(const char *uid, int len_uid, int what)
{
    switch(what)
    {
    case DWYCO_PAL_AUTH_STATUS_FAILED:
        // hmmm, really need to mark it as "provisional" in some way
        // so we try again, or just delete it from the pal list so they
        // have to try again manually.
#if 0
// don't do this for now, since during the upgrade process, there
// may be a lot of accounts imported that won't have pal auth
// state available. really need a way of importing all the accounts
// into the new database to prevent this sort of thing.
        QMessageBox::information(Mainwinform, "Server error", "Can't determine pal auth state for user, either the users account has been deactivated, or you may not be logged into the server.");
        dwyco_pal_delete(uid, len_uid);
#endif
        break;
    case DWYCO_PAL_AUTH_STATUS_REQUIRES_AUTHORIZATION:
        // fire off a pal auth message automatically
    {
        QMessageBox::StandardButton b;
        b = QMessageBox::information(Mainwinform, "Ask for pal authorization", "This user requires you to ask permission to add them to your pal list. Do you wish to ask?", QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
        if(b == QMessageBox::Yes)
        {
            composer *c = new composer(COMP_STYLE_SPECIAL, DWYCO_PAL_AUTH_ASK);
            c->show();
            c->ui.textEdit->setPlainText("Pal authorization request");
            c->set_uid(DwOString(uid, 0, len_uid));
        }
    }
    break;
    case DWYCO_PAL_AUTH_STATUS_ALREADY_AUTHORIZED:
        // do nothing, the pal add will work fine.
        QMessageBox::information(Mainwinform, "Reask authorization?", "You are already authorized by this person. If you want to ask again, demote the user to non-pal, and then promote them to pal again.");
        break;
    default:
        break;
    }
}
#endif

int
DWYCOCALLCONV
dwyco_private_chat_init(int chan_id, const char *)
{
    QList<simple_call *> scl = simple_call::Simple_calls.query_by_member(chan_id, &simple_call::chan_id);
    if(scl.count() != 1)
        return 0;
#if 0
    scl[0]->display_new_msg(scl[0]->uid, QString("priv chat init chan %1").arg(chan_id).toAscii().constData(), "");
#endif
    return 1;
}

int
DWYCOCALLCONV
dwyco_private_chat_display(int chan_id, const char *com, int arg1, int arg2, const char *str, int len)
{
    QList<simple_call *> scl = simple_call::Simple_calls.query_by_member(chan_id, &simple_call::chan_id);
    if(scl.count() != 1)
        return 0;
#if 0
    scl[0]->display_new_msg(scl[0]->uid,
                            QString("priv chat com chan %1 com %2 a1 %3 a2 %4 str %5 len %6").arg(chan_id).arg(com).arg(arg1).arg(arg2).arg(str).arg(len).toAscii().constData(), "");
#endif
    Mainwinform->emit control_msg(chan_id, DwOString(com, 0, strlen(com)), arg1, arg2, DwOString(str, 0, len));
    return 1;
}


void mainwinform::on_actionShow_Offline_Users_triggered(bool checked)
{
    dwyco_field_debug("show-offline", 1);
    Display_all_uid = checked;
    setting_put("display-offline", !!checked);
    settings_save();
    refetch_user_list();
    load_users();
    cdcx_set_refresh_users(1);
}

void mainwinform::on_actionHide_All_triggered()
{
    simple_call::hide_all();
    viewer_profile::delete_all();

}

void mainwinform::on_actionHide_All_Msg_Browsers_triggered()
{
}

void mainwinform::on_actionHide_All_Chatboxes_triggered()
{
    simple_call::hide_all();
}

void mainwinform::on_actionHangup_triggered()
{
    QList<QByteArray> uids = get_selection(0);
    int n = uids.count();
    for(int i = 0; i < n; ++i)
    {
        QByteArray uid = uids[i];
        call_destroy_by_uid(DwOString(uid.constData(), 0, uid.length()));
    }
}

void mainwinform::on_actionAccept_call_triggered()
{
    QList<QByteArray> uids = get_selection(1);
    if(uids.count() == 0)
    {
        return;
    }
}

void mainwinform::on_actionReject_call_triggered()
{
    QList<QByteArray> uids = get_selection(1);
    if(uids.count() == 0)
    {
        return;
    }
}


void mainwinform::on_actionAllow_multiple_profiles_and_browsers_triggered(bool checked)
{
    dwyco_field_debug("allow-multiple", 1);
    Allow_multiple_windows = checked;
    setting_put("allow-multiple-windows", !!checked);
    settings_save();
}

void mainwinform::on_actionShow_Lobby_List_toggled(bool )
{

}

void
mainwinform::show_lobby_list(bool vis)
{
    if(vis)
    {
        ui.actionShow_Lobby_List->setVisible(0);
    }
    else
    {
        ui.actionShow_Lobby_List->setVisible(1);
        ui.actionShow_Lobby_List->setEnabled(1);
    }
}

void
mainwinform::show_pubchat(bool vis)
{
    dwyco_field_debug("tab-pubchat", 1);
    if(vis)
    {
        ui.actionShow_Public_Chat->setVisible(0);

        lbox->show();
        if(HasCamHardware)
            vid_preview->show();
    }
    else
    {
        ui.actionShow_Public_Chat->setVisible(1);
        ui.actionShow_Public_Chat->setEnabled(1);
    }
}

void
mainwinform::simplify_view(bool a)
{
    if(a)
    {
        lbox->hide();
        //vid_preview->hide();
        //pubchat->hide();
    }
}


void
mainwinform::show_dir_listing(bool vis)
{
    dwyco_field_debug("tab-visitors", 1);
    if(vis)
    {
        ui.actionShow_Directory->setVisible(0);
    }
    else
    {
        ui.actionShow_Directory->setVisible(1);
        ui.actionShow_Directory->setEnabled(1);
    }
}

void
mainwinform::show_vid_preview(bool vis)
{
    if(vis)
    {
        ui.actionShow_Video_Preview->setVisible(0);
    }
    else
    {
        if(HasCamHardware)
        {
            ui.actionShow_Video_Preview->setVisible(1);
            ui.actionShow_Video_Preview->setEnabled(1);
        }
    }
}

void
mainwinform::show_profiles(bool vis)
{
    dwyco_field_debug("tab-profiles", 1);
    if(vis)
    {
        ui.actionShow_Profile_Grid->setVisible(0);
    }
    else
    {
        ui.actionShow_Profile_Grid->setVisible(1);
        ui.actionShow_Profile_Grid->setEnabled(1);
    }
}

void
mainwinform::show_msgbrowse(bool vis)
{
    dwyco_field_debug("tab-msgb", 1);
    if(vis)
    {
        ui.actionShow_Msg_Browser->setVisible(0);
    }
    else
    {
        ui.actionShow_Msg_Browser->setVisible(1);
        ui.actionShow_Msg_Browser->setEnabled(1);
    }
}



void mainwinform::on_actionShow_Lobby_List_triggered(bool checked)
{
    lbox->setVisible(1);
    lbox->setFloating(0);
    ui.actionShow_Lobby_List->setVisible(0);
}

void mainwinform::on_actionReload_recent_visitors_triggered()
{
    //dwyco_refresh_directory();

}

static void
DWYCOCALLCONV
user_lobby_create_callback(const char *cmd, void *arg, int succ, const char *fail)
{
    ///printf("%s arg %d succ %d fail %s\n", cmd, (int)arg, succ, fail);
    //fflush(stdout);
    if(!succ)
    {
        QMessageBox::information(0, "Lobby create failed", fail);
    }
    else
    {
        QMessageBox::information(0,
                                 "Lobby create success.",
                                 "Scroll down in the lobby list to find your new lobby."
                                 " Your account has admin access for your new lobby."
                                 " Learn about being an admin from http://www.dwyco.com/admin ."
                                 " Lobbies are automatically removed if they are not used."
                                 " Please also read the Terms of Service at www.dwyco.com"
                                 " for details on acceptable use of user-created lobbies."
                                );
        //Mainwinform->lbox->user_lobbies->setExpanded(1);

    }
}

void mainwinform::on_actionCreate_new_lobby_triggered()
{
    croomform c;
    int res = c.exec();
    if(res == QDialog::Accepted)
    {
        dwyco_chat_create_user_lobby(
            c.ui.lineEdit->text().toAscii().constData(),
            "user",
            My_uid.c_str(), My_uid.length(),
            c.ui.lineEdit_2->text().toAscii().constData(),
            50,
            user_lobby_create_callback, 0);
    }

}

static void
DWYCOCALLCONV
user_lobby_remove_callback(const char *cmd, void *arg, int succ, const char *fail)
{
    //printf("%s arg %d succ %d fail %s\n", cmd, (int)arg, succ, fail);
    //fflush(stdout);
    if(!succ)
    {
        QMessageBox::information(0, "Lobby remove failed", fail);
    }
    else
    {
        QMessageBox::information(0,
                                 "Lobby remove success.", "success"

                                );
    }
}

void mainwinform::on_actionRemove_lobby_triggered()
{
    // this is a bit odd, we are only allowing you to
    // remove a lobby you can get in to... but ok for now.
    if(Current_server_id.length() > 0)
    {
        dwyco_chat_remove_user_lobby(Current_server_id.c_str(), user_lobby_remove_callback, 0);
    }

}

void mainwinform::on_actionMake_me_invisible_triggered(bool checked)
{
    int tmp1 = Last_selected_idx;
    DwOString tmp2 = Last_selected_id;
    dwyco_set_invisible_state(checked);
    // chat server will disconnect synchronously at this point, so we need to reissue a login

    Current_server = -1;
    Current_server_id = "";

    if(tmp1 != -1)
        set_current_server(tmp1);
    else if(tmp2.length() > 0)
        set_current_server(tmp2);

    if(dwyco_get_invisible_state())
    {
        invis_status->show();
        invis_status->setText("<font color=\"#FF0000\">Invis</font>");
    }
    else
    {
        invis_status->hide();
    }

    setting_put("invis", checked);
    settings_save();


}



void mainwinform::on_actionShow_Msg_Browser_triggered(bool checked)
{
    msg_browse_dock->setVisible(1);
    msg_browse_dock->setFloating(0);
    msg_browse_dock->setFocus();
    msg_browse_dock->raise();
    ui.actionShow_Msg_Browser->setVisible(0);
}

void mainwinform::on_actionHide_triggered()
{
    dwyco_field_debug("hide-me", 1);
    setVisible(0);
    //dwyco_suspend();
}

void mainwinform::on_actionExit_2_triggered()
{
    DieDieDie = 2;
}



void mainwinform::on_actionShow_Public_Chat_Users_triggered(bool checked)
{
    dwyco_field_debug("show-public-chat-users", 1);
    Display_chat_users = checked;
    setting_put("display-chat-users", !!checked);
    settings_save();
    refetch_user_list();
    load_users();
    cdcx_set_refresh_users(1);

}

void mainwinform::on_actionShow_pics_in_user_list_triggered(bool checked)
{
    dwyco_field_debug("show-pics", 1);
    Display_pics_in_user_list = checked;
    setting_put("display-pics-in-user-list", !!checked);
    settings_save();
    refetch_user_list();
    load_users();
    cdcx_set_refresh_users(1);

}

static QSet<DwOString> Online;
void
mainwinform::online_status_change(DwOString uid)
{
    // just for online alerts
    int online = dwyco_uid_online(uid.constData(), uid.length());
    if(online && !Online.contains(uid))
    {
//        if(dwyco_get_alert(uid.constData(), uid.length()))
//        {
//            play_sound("relaxed-online.wav");
//        }
        Online.insert(uid);
    }
    if(!online)
    {
        Online.remove(uid);
    }

}

void mainwinform::on_actionShow_Archived_Users_triggered(bool checked)
{
    dwyco_field_debug("show-archived", 1);
    Display_archived_users = !!checked;
    setting_put("display-archived-users", !!checked);
    settings_save();
    refetch_user_list();
    load_users();
    cdcx_set_refresh_users(1);

}

void mainwinform::on_actionIncrease_text_size_triggered()
{
    QFont f(QGuiApplication::font());
    int sz = f.pointSize();
    sz += 2;
    setting_put("pointsize", sz);
    settings_save();
    QMessageBox::information(this, "Text size change", "Please quit and restart to see new size", QMessageBox::Ok);
}

void mainwinform::on_actionDecrease_text_size_triggered()
{
    QFont f(QGuiApplication::font());
    int sz = f.pointSize();
    sz -= 1;
    if(sz < 0)
        sz = 0;
    setting_put("pointsize", sz);
    settings_save();
    QMessageBox::information(this, "Text size change", "Please quit and restart to see new size", QMessageBox::Ok);
}

void mainwinform::on_actionReset_to_default_text_size_triggered()
{
    setting_put("pointsize", 0);
    settings_save();
    QMessageBox::information(this, "Text size change", "Please quit and restart to see new size", QMessageBox::Ok);
}
