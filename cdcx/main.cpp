
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QApplication>
#include <QDialog>
#include <QSettings>
#include <QtNetwork/QHostInfo>
#include <QScreen>
#include <QTcpServer>
#include <QTcpSocket>
#include <QProcess>
#include <QDesktopServices>
#ifdef DWYCO_QT5
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#endif
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mainwin.h"
#include "adminw.h"
#include "autoupdate.h"
#include "dlli.h"
#include "dvp.h"
#include "tfhex.h"
#if 0
#if defined(LINUX) && !defined(MAC_CLIENT)
#include "v4lcapexp.h"
#include "esdaudin.h"
#include "aextsdl.h"
#endif
#endif

#if defined(LINUX) && !defined(DWYCO_FORCE_DESKTOP_VGQT)
#include "v4lcapexp.h"
#endif

#if defined(LINUX) || defined(MAC_CLIENT)
#include <signal.h>
#include <unistd.h>
#endif

#if defined(MAC_CLIENT) || defined(LINUX) || defined(_Windows)
#include "vgqt.h"
#include "audi_qt.h"
#include "audo_qt.h"
#endif

#include "ssmap.h"
#include "config.h"
#include "snd.h"
#include "dwyco_new_msg.h"
#include "login.h"
#ifdef WIN32
#include <windows.h>
#include <direct.h>
#include <time.h>
#endif
#ifndef NO_BUILDTIME
#include "buildtime.h"
#endif
#include "pfx.h"


DWYCO_SERVER_LIST Dwyco_server_list;
void DWYCOCALLCONV dwyco_video_display(int ui_id, void *img, int cols, int rows, int depth);
//void DWYCOCALLCONV dwyco_unregister(int id, const char *msg, int, void *);
void DWYCOCALLCONV dwyco_call_appeared(int id, const char *name, const char *location, const char *uid, int len_uid, const char *call_type, int len_call_type);
void DWYCOCALLCONV dwyco_call_appearance_died(int id);
void DWYCOCALLCONV dwyco_call_accepted(int id, const char *name, const char *location, const char *uid, int len_uid, const char *call_type, int len_call_type);
void DWYCOCALLCONV dwyco_video_display_init(int chan_id, int ui_id);
void DWYCOCALLCONV dwyco_user_control(int chan_id, const char *uid, int len_uid, const char *data, int len_data);
void DWYCOCALLCONV dwyco_check_for_update_done(int status, const char *desc);
int DWYCOCALLCONV dwyco_init_public_chat(int ui_id);
int DWYCOCALLCONV dwyco_display_public_chat(const char *who, int len_who, const char *txt, int len_txt, const char *uid, int len_uid);
void DWYCOCALLCONV dwyco_chat_ctx_callback(int cmd, int id, const char *uid, int len_uid, const char *name, int len_name, int type, const char *val, int len_val, int qid, int extra_arg);
void DWYCOCALLCONV dwyco_chat_ctx_callback2(int cmd, int id, const char *uid, int len_uid, const char *name, int len_name, DWYCO_LIST lst, int qid, int extra_arg);
//void DWYCOCALLCONV dwyco_chat_server_status_callback(int id, const char *msg, int percent_done, void *user_arg);
void DWYCOCALLCONV dwyco_emergency_callback(int problem, int must_exit, const char *dll_msg);
void DWYCOCALLCONV dwyco_db_login_result(const char *str, int what);
//void DWYCOCALLCONV dwyco_pal_auth_callback(const char *uid, int len_uid, int what);
int DWYCOCALLCONV dwyco_call_screening_callback(int chan_id,
        int remote_wants_to_recv_your_video, int remote_wants_to_send_you_video,
        int remote_wants_to_recv_your_audio, int remote_wants_to_send_you_audio,
        int remote_wants_to_exchange_pubchat, int remote_wants_to_exchange_privchat,    const char *call_type, int len_call_type,
        const char *uid, int len_uid,
        int *accept_call_style,
        char **error_msg);
//void DWYCOCALLCONV dwyco_alert_callback(const char *cmd, void *, int, const char *);
void DWYCOCALLCONV dwyco_debug_callback(int status, const char *desc, int, void *);
void DWYCOCALLCONV dwyco_call_bandwidth_callback(int chan_id, const char *txt, int, void *);

void DWYCOCALLCONV dwyco_sys_event_callback(int cmd, int id, const char *uid, int len_uid, const char *name, int len_name, int type, const char *val, int len_val, int qid, int extra_arg);
// private chat is used internally for "call screening" and other control
// messages once a call is set up, not for chat.
int DWYCOCALLCONV dwyco_private_chat_init(int chan_id, const char *);
int DWYCOCALLCONV dwyco_private_chat_display(int ui_id, const char *com, int arg1, int arg2, const char *str, int len);
#if defined(WIN32) && !defined(CDCCORE_STATIC)
class DwAllocator;
DwAllocator *Default_alloc;
#endif

DwOString ZeroUID;
DwOString My_uid;
DwOString TheManUID;
int ClientGod;
int FirstRun;
int First25;
int First214;
int First217;
extern int DoWizard;
int Current_server = -1;
DwOString Current_server_id;
int Last_server = -1;
DwOString Last_server_id;
DwOString Last_selected_id;
int Last_selected_idx = -1;
extern int AvoidCamera;
int AvoidSSL;
int Askup;
DwOString Version;
QRect MainScreenRect;
QTcpServer *BGLockSock;
int Inhibit_powerclean;

static void
takeover_from_background(int port)
{
    //return;
    if(!BGLockSock)
        BGLockSock = new QTcpServer;

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

#ifdef WIN32
static HWND Mainhwnd;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    if(lParam == GetWindowThreadProcessId(hwnd, 0))
    {
        Mainhwnd = hwnd;
        return 0;
    }
    return 1;

}

void
set_main_win()
{
    EnumWindows(EnumWindowsProc, GetCurrentProcessId());
    dwyco_set_main_msg_window(Mainhwnd);
}
#endif

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
        DwOString tfile = add_pfx(Tmp_pfx, "servers2.tmp");
        QFile file(tfile.c_str());
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
    // not needed anymore in qt6
    //r.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkReply *reply = manager->get(r);
}

static
void
myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
#if 1
    if(msg.contains("Timers cannot be stopped from another thread"))
        ::abort();
    if(msg.contains("Timers can only be used with threads started with QThread"))
        ::abort();
#endif

}


int main(int argc, char *argv[])
{
#if !defined(CDCX_RELEASE)
    qInstallMessageHandler(myMessageOutput);
#endif

    QApplication app(argc, argv);
    srand(time(0));
    AvoidSSL = !QSslSocket::supportsSsl();
    if(!AvoidSSL)
    {
        // this is a silly hack for linux desktop where we end up
        // having to compile on some old stuff, but the newer
        // desktops have openssl with newer versions with missing
        // symbols...
#if defined(LINUX) && !(defined(MAC_CLIENT) || defined(ANDROID))
        QString ssl1 = QSslSocket::sslLibraryBuildVersionString();
        ssl1.truncate(9);
        QString ssl2 = QSslSocket::sslLibraryVersionString();
        ssl2.truncate(9);
        if(ssl1 != ssl2)
            AvoidSSL = 1;
#endif
    }

#if defined(MAC_CLIENT) && defined(CDCX_MAC_USE_DEFAULT_LOCATION)
    void EstablishRunDirectory();
    EstablishRunDirectory();
    QString userdir("./");
    QString syspath("./");
    AvoidSSL = 1;

#endif
#if defined(LINUX) && !defined(MAC_CLIENT) && defined(GNOME_STARTUP)
    if(argc <= 1)
        exit(1);
    if(chdir(argv[1]) == -1)
        exit(1);
    --argc;
    ++argv;
#endif
#if defined(LINUX) || defined(MAC_CLIENT)
    signal(SIGPIPE, SIG_IGN);
#endif

#if /*defined(LINUX) &&*/ !defined(MAC_CLIENT)
    // this is mainly for appimage-type installation. the read-only stuff
    // it in the appimage, and the rest of the user data will be in
    // documents/dwyco/cdc-x.
    // for debugging, you can use the --path /mumble
    // to override the default
    QStandardPaths::StandardLocation filepath = QStandardPaths::DocumentsLocation;
    QString userdir;
    {
    QStringList args = QGuiApplication::arguments();

    for(int i = 1; i < args.count(); ++i)
    {
            if(args[i] == "--path")
            {
                if(i + 1 < args.count())
                {
                    userdir = args[i + 1];
                    break;
                }
            }
    }

    if(userdir.length() == 0)
    {
        userdir = QStandardPaths::writableLocation(filepath);
        userdir += "/dwyco/cdc-x/";
    }
    else
    {
        userdir += "/";
    }

    {
        QDir d(userdir);
        d.mkpath(userdir);
        // this is just a stopgap, really need to do something to obfuscate the files
        // a little bit to avoid apps indexing temp images on all platforms
        QString fp = d.filePath(".nomedia");
        QFile f(fp);
        f.open(QIODevice::WriteOnly);
        f.putChar(0);
        f.close();
    }
    {
        QDir shares(userdir + "shares");
        shares.mkpath(userdir + "shares");
    }
    }
    QString syspath = QCoreApplication::applicationDirPath() + QDir::separator();
    //syspath = "/home/dwight/cdcx/";
#endif

#if 0 &&  WIN32
// still expecting to start in the directory where all our data is
    QString userdir("./");
    QString syspath("./");
#endif


#define FPATH "./"

    dwyco_set_fn_prefixes(syspath.toLatin1().constData(), userdir.toLatin1().constData(), (userdir.toLatin1() + QByteArray("/tmp/")).constData());
    {
        char sys[1024];
        char user[1024];
        char tmp[1024];
        int len_sys = sizeof(sys);
        int len_user = sizeof(user);
        int len_tmp = sizeof(tmp);

        dwyco_get_fn_prefixes(sys, &len_sys, user, &len_user, tmp, &len_tmp);
        Sys_pfx = DwOString(sys, 0, len_sys - 1);
        User_pfx = DwOString(user, 0, len_user - 1);
        Tmp_pfx = DwOString(tmp, 0, len_tmp - 1);
        if(Sys_pfx.length() == 0)
            Sys_pfx = ".";
        if(User_pfx.length() == 0)
            User_pfx = ".";
        if(Tmp_pfx.length() == 0)
            Tmp_pfx = ".";

    }
    {
        QDir d;
        d.mkpath(User_pfx);
    }
    {
        QDir d;
        d.mkpath(Tmp_pfx);
    }
    // this is the one goofy thing... we have a servers2 that is
    // distributed with the read-only stuff, but it needs to be
    // writable so the servers can update it if needed.
    // so we "install" it once if it doesn't exists. if it
    // doesn't exist, and the "install" fails, the core dll
    // has a compiled in version as a last resort.
    if(!QFile(userdir + "servers2").exists())
        QFile::copy(Sys_pfx + "/servers2", userdir + "servers2");
    QFile::setPermissions(userdir + "servers2", QFile::ReadOwner|QFile::WriteOwner);

    dwyco_trace_init();
    int dum;
#ifdef WIN32
    //dwyco_set_main_msg_window(::GetDesktopWindow());
    set_main_win();
    // note: in 2.9, qtwebkit + openssl seems to crash randomly on
    // first run, so we are nixing it for now.
    AvoidSSL = 1;
#endif
#ifndef NO_BUILDTIME
    // the build time gives us a way to tell users they need
    // a new version during testing, but also allows us to keep
    // quiet if they have a newer version (which happens a lot.)
    Version += "%";
    Version += BUILDTIME;
    Version += "%";
#endif
    Version += "Dwyco-CDC-X-";
    Version += __DATE__;
    Version += " (Qt ";
    Version += QT_VERSION_STRING;
    Version += ")";


    dwyco_set_client_version(Version.c_str(), Version.length());
    ZeroUID = from_hex("00000000000000000000");
    TheManUID = from_hex("5a098f3df49015331d74");
    DVP::init_dvp();
    settings_load();
    DwOString sport;
    int port;
    if(!setting_get("syncport", sport))
    {
        port = (rand() % 50000) + 10000;
        sport = QByteArray::number(port);
        setting_put("syncport", sport);
    }
    port = sport.toInt();

    takeover_from_background(port);
    load_unviewed();
    DwOString sdum;
    FirstRun = !setting_get("first-run", sdum);
    if(FirstRun)
    {
        DoWizard = 1;
        AvoidSSL = 1;
    }
    First25 = !setting_get("first25", sdum);
    First214 = !setting_get("first214", sdum);
    First217 = !setting_get("first217", sdum);

#if defined(WIN32)
    if(!setting_get("askup", Askup))
    {
        setting_put("askup", 1);
        Askup = 1;
        settings_save();
    }
#endif
    if(!setting_get("server", Last_server))
    {
        Last_server = 0;
        setting_put("server", Last_server);
        settings_save();
    }
    if(!setting_get("server_id", Last_server_id))
    {
        Last_server_id = "";
        setting_put("server_id", Last_server_id);
        settings_save();
    }


    init_sound();

    // note: qt seems to use some of these names in constructing
    // file names. this can be a problem if different FS's with different
    // naming conventions are being used. this manifests itself with
    // file names conflicting, and problems copying existing files from
    // different machines. it gets worse if you are using file syncing
    // things like dropbox and btsync.
    QCoreApplication::setOrganizationName("dwyco");
    QCoreApplication::setOrganizationDomain("dwyco.com");
    QCoreApplication::setApplicationName(QString("cdc-x"));
    QString ver(DWYCO_NICE_VERSION);
#ifndef CDCX_RELEASE
    ver += " debug ";
#endif
    QCoreApplication::setApplicationVersion(ver);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    // note: need to set the path to the right place, same as fn_pfx for dll
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, User_pfx);

    QStringList args = QGuiApplication::arguments();
    for(int i = 1; i < args.count(); ++i)
    {
        if(args[i] == "--avoid-camera")
            AvoidCamera = 1;
        else if(args[i] == "--avoid-ssl")
            AvoidSSL = 1;
    }

    //QDesktopWidget *desktop = QApplication::desktop();
    //MainScreenRect = desktop->availableGeometry(desktop->primaryScreen());

    QScreen *screen = QGuiApplication::primaryScreen();
    MainScreenRect = screen->availableGeometry();


    //Q_INIT_RESOURCE(icons);

    //dwyco_set_cmd_path(argv[0], strlen(argv[0]));

    dwyco_set_login_result_callback(dwyco_db_login_result);

    // autoupdate hashs are not used anymore because they are not
    // flexible enough in the multi-platform situation. we rely on
    // just compiled-in dates from the executable. this makes startup a little faster,
    // and people whose machines are infected by viruses will not be
    // confused anymore by our autoupdate notifications repeatedly
    // popping up because the exe has been contaminated. win win!
#if 0
#if defined(CDCX_RELEASE) && !defined(LINUX) && !defined(MAC_CLIENT)
    dwyco_setup_autoupdate(argv[0], "cdcdll8.dll", 0, 0);
#else
    dwyco_setup_autoupdate(argv[0], 0, 0, 0); // for testing
#endif
#endif

    // this inhibits database login and allows us to
    // set the server password later on after the user
    // has a chance to enter one.
    //dwyco_set_cdc_compat(1);

    int invis = 0;
    if(!setting_get("invis", invis))
    {
        setting_put("invis", 0);
        settings_save();
    }
    dwyco_set_initial_invis(invis);

#if defined(MAC_CLIENT) || defined(LINUX)
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

#if defined(DWYCO_FORCE_DESKTOP_VGQT)
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

                vgqt_get_video_devices,
                vgqt_free_video_devices,
                vgqt_set_video_device,
                vgqt_stop_video_device, 0, 0, 0, 0

    );

#elif defined(LINUX) && !defined(DWYCO_FORCE_DESKTOP_VGQT)

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

    dwyco_set_disposition("foreground", 10);
    dwyco_init();
    //printf("%s\n", a);
    //fflush(stdout);
    dwyco_set_moron_dork_mode(1);

    if(!setting_get("first27", sdum))
    {
        dwyco_set_setting("zap/no_forward_default", "0");
        setting_put("first27", 0);
    }


    if(!FirstRun)
    {
        extern int Block_DLL;
        Block_DLL = 1;
        DwOString a;
        if(setting_get("mode", a) && a.eq("1"))
        {
            loginform *lf = new loginform(0, Qt::WindowStaysOnTopHint);
            if(lf->exec() == QDialog::Rejected)
                exit(0);
            delete lf;
        }
        Block_DLL = 0;
    }
    dwyco_get_server_list(&Dwyco_server_list, &dum);



    {
        const char *uid;
        int len_uid;
        dwyco_get_my_uid(&uid, &len_uid);
        My_uid = DwOString(uid, 0, len_uid);
    }

    new configform;
    TheConfigForm->load();

    dwyco_set_autoupdate_status_callback(dwyco_check_for_update_done);
    dwyco_set_all_mute(0);
    dwyco_set_auto_squelch(0);
    dwyco_set_full_duplex(1);
    // normally audio is reflected to all connections, but in this
    // mode, audio is sent exclusively to one channel regardless of
    // how many listeners there are.
    //dwyco_set_exclusive_audio(1, -1);

    // note: BUG these are not persistent, should find a place to
    // store them
    dwyco_set_codec_data(0, 0, .5);

    new configform;
    TheConfigForm->load();


    dwyco_set_video_display_callback(dwyco_video_display);
    //dwyco_set_unregister_callback(dwyco_unregister);
    dwyco_set_emergency_callback(dwyco_emergency_callback);
    //dwyco_set_call_appearance_callback(dwyco_call_appeared);
    //dwyco_set_call_appearance_death_callback(dwyco_call_appearance_died);
    dwyco_set_call_acceptance_callback(dwyco_call_accepted);
    //dwyco_set_video_display_init_callback(dwyco_video_display_init);
    dwyco_set_user_control_callback(dwyco_user_control);

    dwyco_set_public_chat_init_callback(dwyco_init_public_chat);
    dwyco_set_public_chat_display_callback(dwyco_display_public_chat);
    dwyco_set_private_chat_init_callback(dwyco_private_chat_init);
    dwyco_set_private_chat_display_callback(dwyco_private_chat_display);

    dwyco_set_chat_ctx_callback(dwyco_chat_ctx_callback);
    dwyco_set_chat_ctx_callback2(dwyco_chat_ctx_callback2);
    //dwyco_set_chat_server_status_callback(dwyco_chat_server_status_callback);

    //dwyco_set_pal_auth_callback(dwyco_pal_auth_callback);
    dwyco_set_call_screening_callback(dwyco_call_screening_callback);
    //dwyco_set_alert_callback(dwyco_alert_callback);
#ifndef CDCX_RELEASE
    dwyco_set_debug_message_callback(dwyco_debug_callback);
#endif
    dwyco_set_call_bandwidth_callback(dwyco_call_bandwidth_callback);
    dwyco_set_system_event_callback(dwyco_sys_event_callback);

#if 0
    dwyco_set_zap_data(
        0, // don't always send via server
        1, // always accept direct zaps
        0, 1, 0, // accept msgs from all ratings immediately
        0, // don't use old timing
        1, // don't save sent msgs
        1 // don't allow forwarding of msgs
    );

    dwyco_set_call_accept(
        0, // max audio send, linux audio is a mystery right now
        0, // max public chats (we don't public chat)
        1, // max video streams to send
        0, // max audio recv, linux audio is a mystery
        1, // max vidoe recv
        0, // we don't use private chat
        "", // pw
        0, // don't auto accept
        0, // don't require a password
        1, // accept from other ratings
        0 // always open listening socket
    );


    dwyco_set_video_input(
        "",
        0,
        0,
        1,
        0,
        0
    );
#endif
#if 0
    dwyco_set_rate_tweaks(
        256,
        12,
        65535,
        10000,
        256,
        256
    );
#endif
    dwyco_set_setting("call_acceptance/max_chat", "100");
    dwyco_set_setting("call_acceptance/max_pchat", "100");
    dwyco_set_setting("net/listen", "1");
    dwyco_set_setting("zap/always_accept", "1");
    dwyco_set_setting("zap/always_server", "0");
    // TCP only calling
    dwyco_set_setting("net/call_setup_media_select", "1");
    //dwyco_set_setting("net/force_non_firewall_friendly", "0");
    dwyco_set_setting("sync/eager", "1");
    dwyco_set_setting("net/app_id", "phoo");
    dwyco_set_setting("net/broadcast_port", "48903");

    // note: for video capture,
    // Linux & Mac ignore the setting and always uses external video
    // on windows, MINGW has to use external video (dx9)
    // on msvc version, you can use either dx9 (external) or vfw (native.)
#ifdef USE_VFW
// set this to 0 if you want to override and
// always use vfw. useful for testing if you don't have the
// mtcap.dll thing working.
#if 1
    int use_vfw;
    if(!setting_get("use vfw", use_vfw))
    {
        use_vfw = 0;
        setting_put("use vfw", 0);
        settings_save();
    }
    if(use_vfw)
    {
        dwyco_set_external_video(0);
    }
    else
    {
        dwyco_set_external_video(1);
    }
#else
    setting_put("use vfw", 1);
    settings_save();
    dwyco_set_external_video(0);
#endif
#else
    dwyco_set_external_video(1);
    //dwyco_start_vfw(0, 0, 0);
#endif
#if 0
    {
        extern int HasCamera;
        extern int HasCamHardware;
// for easier testing, setup for raw file acq
        dwyco_set_video_input(
            "",
            0,
            1, // raw files
            0,
            0,
            0
        );
        dwyco_set_raw_files(
            "vidfile.lst",
            "",
            1, // use list of files
            0,
            0 // preload
        );
#endif
#if 0
        HasCamera = 1;
        HasCamHardware = 1;
    }
#endif
    // ultimately, it was a bad idea to automatically add
    // people to pal list when messaging. it caused too many
    // messages to be retained.
    // this just clears the entire pal list, which will cause some
    // people problems, but since we have no idea which pals were
    // automatic and which were manual, there isn't a lot we can do.
    // only thing is that *probably* only a tiny fraction are
    // manual, because most people don't bother with pal stuff anyways.
    if(First217)
    {
        DWYCO_LIST l = dwyco_pal_get_list();
        int n;
        dwyco_list_numelems(l, &n, 0);
        for(int i = 0; i < n; ++i)
        {
            const char *val;
            int len;
            int type;
            if(!dwyco_list_get(l, i, DWYCO_NO_COLUMN, &val, &len, &type))
                continue;
            if(type != DWYCO_TYPE_STRING && type != DWYCO_TYPE_NIL)
                continue;
            DwOString uid = DwOString(val, 0, len);
            dwyco_pal_delete(uid.c_str(), uid.length());
        }
        dwyco_list_release(l);
        // since we are killing the pal list, turn off pals-only too
        dwyco_set_pals_only(0);
        setting_put("first217", 0);
        First217 = 0;
    }

    int first_bug217 = !setting_get("first_bug217", sdum);
    if(first_bug217)
    {
        // need to remove any full backup so it gets recreated.
        // this is because of a bug we fixed in 2.17 that might
        // have created a partial full backup if an attachment was
        // missing.
        dwyco_remove_backup();
        setting_put("first_bug217", 0);
    }

    int first_bug218 = !setting_get("first_bug218b", sdum);
    if(first_bug218)
    {
        // schema change in backups so redo them
        dwyco_remove_backup();
        setting_put("first_bug218b", 0);
    }


    ClientGod = !!getenv("kk27g");
    int psz;
    if(!setting_get("pointsize", psz))
    {
        setting_put("pointsize", 0);
        settings_save();
        psz = 0;
    }
    if(psz != 0)
    {
        QFont f(QGuiApplication::font());
        f.setPointSize(psz);
        QGuiApplication::setFont(f);
    }
    mainwinform mainwin;
    adminw *admin = new adminw;

    admin->replace_servers(Dwyco_server_list);

    autoupdateform *autoup = new autoupdateform;


    QSettings settings;
    mainwin.restoreGeometry(settings.value("mainwin-geometry").toByteArray());
    mainwin.show();
    setup_emergency_servers();

    int i = app.exec();
    // this is more or less an emergency where the system state may be
    // goofy, like after a panic or restore operation.
    if(DieDieDie == 1)
        return 0;
    //dwyco_empty_trash();
    if(!Inhibit_powerclean)
        dwyco_power_clean_safe();
    //dwyco_untrash_users();
    int d = 0;
    setting_get("disable_backups", d);
    if(!d)
    {
        if(dwyco_create_backup(7, 30))
        {
#ifdef DWYCO_QT5
            QStringList sl = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
            QString loc = sl[0];
#else
            QString loc = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#endif
            QByteArray b = loc.toLatin1();
            dwyco_copy_out_backup(b.constData(), 0);
        }
    }
    dwyco_exit();
    exit_sound();
    setting_get("disable_bg", d);
    if(!d)
    {
        QProcess::startDetached(QCoreApplication::applicationDirPath() + QDir::separator() + QString("dwycobg"), QStringList(sport), User_pfx);
    }

#ifdef LEAK_CLEANUP
    void mainwin_leak_cleanup();
    mainwin_leak_cleanup();
#endif
#ifdef LINUX
    // on linux + qt5.12, there is some problem with the exit processing, maybe having something
    // to do with webengine that causes the "return" to just hang for seconds.
    // as much as i hate to do this, i think flushing fd's is already
    // done at this point, so i'm just hacking this to quit immediately.
    //
    _exit(0);
#endif
    return i;
}

