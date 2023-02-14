#include "ezset.h"
#ifdef DWYCO_TRACE
#include "dwyco_rename.h"
#endif
#include "dlli.h"
#include "fetch_to_inbox.h"
#include "dwstr.h"
#include "dwrtlog.h"
#include "ssns.h"
#include "aconn.h"
#include "mmchan.h"
#include "qmsgsql.h"
#include "vcwsock.h"
#include "dwycolist2.h"
#include "qdirth.h"
#include "qmsg.h"
#include "backandroid.h"
#include <thread>
#include <future>
#include <chrono>

using namespace dwyco;

#ifndef WIN32
#include "poll.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

namespace dwyco {
extern DwycoPublicChatDisplayCallback dwyco_bgapp_msg_callback;
}

// this is a simple condition variable we use to signal
// the java stuff to re-check for messages (so it can post
// a notification.)
static pthread_cond_t Msg_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t Msg_cond_mutex = PTHREAD_MUTEX_INITIALIZER;
static int New_msg;
#endif

#ifdef ANDROID
#include <android/log.h>
#define LOG_TAG "dwyco-bg"
#define ALOGI(msg, ...) \
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, (msg), __VA_ARGS__)
#define ALOGE(msg, ...) \
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, (msg), __VA_ARGS__)
#else
#define ALOGI(msg, ...)
#define ALOGE(msg, ...)
#endif
//#undef ANDROID
void
android_log_stuff(const char *str, const char *s1, int s2)
{
    ALOGI("als %s %s %d", str, s1, s2);
}

DWYCOEXPORT
void
dwyco_signal_msg_cond()
{
#ifndef WIN32
    pthread_mutex_lock(&Msg_cond_mutex);
    New_msg = 1;
    pthread_cond_signal(&Msg_cond);
    pthread_mutex_unlock(&Msg_cond_mutex);
#endif

}

// the java stuff calls into this in another thread
// when we signal, it means a new message has arrived
DWYCOEXPORT
void
dwyco_wait_msg_cond(int ms)
{
#ifndef WIN32
    pthread_mutex_lock(&Msg_cond_mutex);
    while(New_msg == 0)
        pthread_cond_wait(&Msg_cond, &Msg_cond_mutex);
    New_msg = 0;
    pthread_mutex_unlock(&Msg_cond_mutex);
    // the "work" to be done in this case is done when we
    // return to java and it sets the notification
#endif

}

DWYCOEXPORT
void
dwyco_stop_msg_cond()
{
#ifndef WIN32
    pthread_mutex_lock(&Msg_cond_mutex);
    New_msg = -1;
    pthread_cond_signal(&Msg_cond);
    pthread_mutex_unlock(&Msg_cond_mutex);
#endif
}

#if defined(ANDROID)
// this is needed on android because when the battery saver
// comes on, it interferes with the networking so that we
// can't tell our worker thread to exit. this uses unix abstract
// domain sockets instead of internet loopback.

DWYCOEXPORT
int
dwyco_request_singleton_lock(const char *name, int port)
{
    int s = socket(AF_UNIX, SOCK_STREAM,  0);
    if(s == -1)
        return -1;

    if(fcntl(s, F_SETFL, O_NONBLOCK) == -1)
        return -1;

    DwString aname;
    aname += '\0';
    aname += name;
    aname += DwString::fromInt(port);
    struct sockaddr_un sap;
    memset(&sap, 0, sizeof(sap));
    sap.sun_family = AF_UNIX;
    memcpy(sap.sun_path, aname.c_str(), dwmin(aname.length(), sizeof(sap.sun_path)));
    int tlen = offsetof(sockaddr_un, sun_path) + aname.length();

    while(1)
    {
        if(bind(s, (const struct sockaddr *)&sap, tlen) == -1)
        {
            if(errno == EADDRINUSE)
            {
                // issue a connect and see if we can get
                // the background guy to relinquish the lock
                int s2;
                s2 = socket(AF_UNIX, SOCK_STREAM, 0);
                if(fcntl(s2, F_SETFL, O_NONBLOCK) == -1)
                    return -1;

                if(connect(s2, (const struct sockaddr *)&sap, tlen) == -1)
                {
                    close(s2);
                    usleep(10000);
                    continue;
                }
                close(s2);
                usleep(10000);
                continue;
            }
            close(s);
            return -1;
        }
        else
            break;
    }
    if(listen(s, 5) == -1)
    {
        close(s);
        return -1;
    }
    return s;
}

static
int
get_singleton_lock(const char *name, int port)
{
    int s = socket(AF_UNIX, SOCK_STREAM,  0);
    if(s == -1)
        return -1;

    if(fcntl(s, F_SETFL, O_NONBLOCK) == -1)
        return -1;

    DwString aname;
    aname += '\0';
    aname += name;
    aname += DwString::fromInt(port);
    struct sockaddr_un sap;
    memset(&sap, 0, sizeof(sap));
    sap.sun_family = AF_UNIX;
    memcpy(sap.sun_path, aname.c_str(), dwmin(aname.length(), sizeof(sap.sun_path)));
    int tlen = offsetof(sockaddr_un, sun_path) + aname.length();

    int tries = 2000;
    int i;
    for(i = 0; i < tries; ++i)
    {
        if(bind(s, (const struct sockaddr *)&sap, tlen) == -1)
        {
            if(errno == EADDRINUSE)
            {
                usleep(10000);
                continue;
            }
            close(s);
            return -1;
        }
        else
            break;
    }
    if(i == tries)
    {

        close(s);
        return -1;
    }
    if(listen(s, 5) == -1)
    {
        close(s);
        return -1;
    }
    return s;
}
#endif


// this is just a goofy way to keep this background
// processing from interfering with the UI (which does
// its own processing for everything.)

// return -1 if there is some error
// return 0 if the port appears locked
// return 1 is the port looks unlocked
DWYCOEXPORT
int
dwyco_test_funny_mutex(int port)
{
#ifdef WIN32
    if(vc_winsock::startup() == 0)
        return -1;
#endif
    int s = socket(AF_INET, SOCK_STREAM,  0);
    if(s == -1)
        return -1;
#ifdef WIN32
    {
        u_long on = 1;
        if(ioctlsocket(s, FIONBIO, &on) != 0)
            return -1;
    }
#else
    if(fcntl(s, F_SETFL, O_NONBLOCK) == -1)
        return -1;
#endif
    struct sockaddr_in sap;
    memset(&sap, 0, sizeof(sap));
    sap.sin_family = AF_INET;
    sap.sin_addr.s_addr = inet_addr("127.0.0.1");
    sap.sin_port = htons(port);
    int i;
    for(i = 0; i < 2; ++i)
    {
        if(bind(s, (struct sockaddr *)&sap, sizeof(sap)) == -1)
        {
#ifdef WIN32
            int e = WSAGetLastError();
            if(e == WSAEADDRINUSE)
            {
                SleepEx(10, 0);
                continue;
            }
            closesocket(s);
#else
            if(errno == EADDRINUSE)
            {
                usleep(10000);
                continue;
            }
            close(s);
#endif

            return -1;
        }
        else
            break;
    }
#ifdef WIN32
        closesocket(s);
#else
        close(s);
#endif
    if(i == 2)
    {
        return 0;
    }

    return 1;
}

static
int
get_funny_mutex(int port)
{
#ifdef WIN32
    if(vc_winsock::startup() == 0)
        return -1;
#endif
    int s = socket(AF_INET, SOCK_STREAM,  0);
    if(s == -1)
        return -1;
#ifdef WIN32
    {
        u_long on = 1;
        if(ioctlsocket(s, FIONBIO, &on) != 0)
            return -1;
    }
#else
    if(fcntl(s, F_SETFL, O_NONBLOCK) == -1)
        return -1;
#endif
    struct sockaddr_in sap;
    memset(&sap, 0, sizeof(sap));
    sap.sin_family = AF_INET;
    sap.sin_addr.s_addr = inet_addr("127.0.0.1");
    sap.sin_port = htons(port);
    int i;
    const int tries = 1000;
    for(i = 0; i < tries; ++i)
    {
        if(bind(s, (struct sockaddr *)&sap, sizeof(sap)) == -1)
        {
#ifdef WIN32
            int e = WSAGetLastError();
            if(e == WSAEADDRINUSE)
            {
                SleepEx(10, 0);
                continue;
            }
            closesocket(s);
#else
            if(errno == EADDRINUSE)
            {
                usleep(10000);
                continue;
            }
            close(s);
#endif

            return -1;
        }
        else
            break;
    }
    if(i == tries)
    {
#ifdef WIN32
        closesocket(s);
#else
        close(s);
#endif
        return -1;
    }
    if(listen(s, 5) == -1)
    {
#ifdef WIN32
        closesocket(s);
#else
        close(s);
#endif
        return -1;
    }
    return s;
}

static ssns::signal0 Bg_login_failed;

static
void
DWYCOCALLCONV
dwyco_background_db_login_result(const char *str, int what)
{
    if(what == 0)
    {
        // exit the process since there isn't anything more
        // we can do really, unless there are direct connects...
        // if there are no direct connections, for sure quit
        GRTLOG("bg db login fail %s", str, 0);
        Bg_login_failed.emit();
    }
    else
    {
        GRTLOG("bg db login ok", 0, 0);
    }
}

static
void
bail_out()
{
    dwyco_bg_exit();
    exit(0);
}

// need this in case a join was initiated in the UI and
// it is completed in the background. we need to exit immediately
// and let it re-initialize with the new group member.
static
void
check_join_simple(int cmd, int ctx_id, const char *uid, int len_uid, const char *name, int len_name, int type, const char *value_elide, int val_len, int qid, int extra_arg)
{
    DwString nm;
    if(name)
    {
        nm = DwString(name, len_name);
    }
    if(cmd == DWYCO_SE_GRP_JOIN_OK)
    {
        GRTLOG("JOIN GROUP %s OK", nm.c_str(), 0);
        //dwyco_set_setting("group/alt_name", nm.c_str());
        exit(0);
    }
    else if(cmd == DWYCO_SE_GRP_JOIN_FAIL)
    {
        GRTLOG("JOIN GROUP %s FAIL", nm.c_str(), 0);
        exit(1);
    }

}

static
int
do_android_backup()
{
    GRTLOG("backup start", 0, 0);
    android_backup();
    GRTLOG("backup done", 0, 0);
    return 0;
}

static
int
background_android_backup()
{
    static std::future<int> *backup_done = nullptr;

    if(backup_done != nullptr)
    {
        if(backup_done->wait_for(std::chrono::seconds(0)) == std::future_status::timeout)
            return -1;
        auto ret = backup_done->get();
        delete backup_done;
        backup_done = nullptr;
        return ret;
    }
    if(android_days_since_last_backup() < 1)
        return -1;
    auto f = new std::future<int>;
    backup_done = f;
    *f = std::async(std::launch::async, &do_android_backup);
    return 1;
}

// why all this threading mickey-mouse?
// we need something that is synchronous (most of the
// rest of the msg processing doesn't really work if
// it gets "busy", something that i should fix) and interruptible
// if user starts up the main app. in that case,
// we just exit, leaving the backup in whatever
// state... sqlite takes care of keeping it
// reasonably sane.
// NOTE: this assumes you are in a separate process
// that can be exited. it probably needs to return and
// let the caller decide how to clean up.
static
void
check_background_backup(vc asock, bool just_check_once)
{
    static DwTimer bu_poll;
    static int been_here;
    static bool already_checked = false;

    if(already_checked)
        return;
    if(!been_here)
    {
        bu_poll.set_autoreload(1);
        bu_poll.set_interval(60 * 60 * 1000);
        bu_poll.start();
        been_here = 1;
    }
    if(!just_check_once)
    {
        if(!bu_poll.is_expired())
            return;
        bu_poll.ack_expire();
    }
    int state = background_android_backup();
    already_checked = just_check_once;
    if(state == 1)
    {
        do
        {
            if(asock.socket_poll(VC_SOCK_READ|VC_SOCK_ERROR, 1, 0) == 0)
            {
                if(background_android_backup() == 0)
                {
                    // backup done
                    return;
                }
            }
            else
            {
                // user requests an exit
                dwyco_bg_exit();
                exit(0);
            }
        }
        while(1);
    }
}


// note: if exit_if_outq_empty is non-zero, this function returns
// when it discovers there is no more stuff to send. otherwise, this
// function runs until it is stopped either by the main UI starting, or
// the OS kills it.
// if exit_if_outq_empty & 2 != 0, it disables the "check once per hour"
// filtering for the android backup checking. which means it will check
// once (the first time it is called.)
DWYCOEXPORT
int
dwyco_background_processing(int port, int exit_if_outq_empty, const char *sys_pfx, const char *user_pfx, const char *tmp_pfx, const char *token)
{
#ifndef WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    //alarm(3600);
    srand(time(0));
    ALOGI("bg-start %d", 0);

    int s;
#ifdef ANDROID
    s = get_singleton_lock("mumble", 0);
#else
    s = get_funny_mutex(port);
#endif

    ALOGI("mutex %d", s);
    // first run, if the UI is blocking us, something is wrong
    if(s == -1)
        return 1;
    int just_suspend = false;
    if(dwyco_get_suspend_state())
    {
        // if we are here and we are in the suspended state, it
        // is because this background processing is being
        // started while the UI is temporarily suspended in another
        // thread.
        // so just resume, and start processing without
        // re-establishing state.
        dwyco_resume();
        just_suspend = true;
    }
    else
    {

    //dwyco_set_login_result_callback(dwyco_db_login_result);
    dwyco_set_fn_prefixes(sys_pfx, user_pfx, tmp_pfx);

    dwyco_set_client_version("dwycobg", 7);
    dwyco_set_initial_invis(1);
    dwyco_set_login_result_callback(dwyco_background_db_login_result);
    dwyco_set_system_event_callback(check_join_simple);
    dwyco_set_disposition("background", 10);
    dwyco_bg_init();
    if(token)
        dwyco_write_token(token);

    set_listen_state(1);
    // allowing incoming calls needs to happen for
    // syncing. but, we really beed some extra logic to
    // handle audio/video call stuff if we are just
    // syncing in the background in our own process
    // on desktop, as most people don't want calls unless
    // the client is up and running. dunno, needs some more
    // thought.
    dwyco_inhibit_incoming_sac(0);
    dwyco_inhibit_outgoing_sac(0);
    dwyco_inhibit_pal(0);

    if(dwyco_get_create_new_account())
    {
        // only run on existing accounts
        return 1;
    }

    dwyco_set_local_auth(1);
    dwyco_finish_startup();
    }

    //int comsock = -1;
#ifdef ANDROID
    // WARNING: creating a socket in VC like this tweaks some
    // global structures used to provide bulk poll results.
    // YOU MUST BE CERTAIN OTHER THREADS ARE NOT CALLING INTO
    // THE MAIN API or VC, the lib itself is not thread-safe
    vc asock = vc(VC_SOCKET_STREAM_UNIX);
#else
    vc asock = vc(VC_SOCKET_STREAM);
#endif
    asock.socket_init(s, vctrue);

    int signaled = 0;
    int started_fetches = 0;
    vc nicename = get_settings_value("app/nicename");
    DwString notify_str;
    bool desktop_notify = false;
#if defined(MACOSX) && !defined(DWYCO_IOS)
                //system("/usr/bin/osascript -e 'display notification \"New message\" with title \"CDC-X\" sound name \"default\"'");
    notify_str = DwString("/usr/bin/osascript -e 'display notification \"New message\" with title \"%1\" sound name \"default\"'").arg((const char *)nicename);
    desktop_notify = true;
#endif
#ifdef WIN32
                //system("notify-send.exe \"CDC-X\" \"New message\"");
                notify_str = DwString("notify-send.exe \"%1\" \"New message\"").arg((const char *)nicename);
                desktop_notify = true;
#endif
#if defined(LINUX) && !defined(MACOSX) && !defined(DWYCO_IOS) && !defined(ANDROID)
                notify_str = DwString("notify-send \"%1\" \"New message\"").arg((const char *)nicename);
                desktop_notify = true;
                    //system("notify-send " DWYCO_APP_NICENAME " \"New message\"");
#endif

    while(1)
    {
        int spin = 0;
        int snooze = dwyco_service_channels(&spin);

        if(!just_suspend)
        {
            // note: this currently doesn't work nice
            // if this background processing is done
            // in a thread (via workmanager).
            // in that case, it might make more sense to just
            // define the backup as another workrequest
            check_background_backup(asock, (exit_if_outq_empty & 2) ? true : false);
        }
        if(exit_if_outq_empty && msg_outq_empty())
            break;
#ifdef WIN32
        if(accept(s, 0, 0) != INVALID_SOCKET)
        {
            break;
        }
        else
        {
            int e = WSAGetLastError();
            if(e != WSAEWOULDBLOCK)
                return 1;
        }
#else
        if(accept(s, 0, 0) != -1)
        {
            ALOGI("accept to exit %d", s);
            break;
        }
        else if(!(errno == EWOULDBLOCK || errno == EAGAIN))
        {
            ALOGI("bad accept %d", errno);
            close(s);
            return 1;
        }
#endif
        if(dwyco_get_rescan_messages())
        {
            GRTLOG("rescan %d %d", started_fetches, signaled);
            dwyco_set_rescan_messages(0);
            ns_dwyco_background_processing::fetch_to_inbox();
            GRTLOG("rescan2 %d %d", started_fetches, signaled);
            int tmp;
            if((tmp = sql_count_tag("_inbox")) > signaled)
            {
                GRTLOG("signaling newcount %d", tmp, 0);
                signaled = tmp;
                dwyco_signal_msg_cond();
#ifndef DWYCO_IOS
                if(desktop_notify)
                    system(notify_str.c_str());
#endif
            }
        }
        // note: this is a bit sloppy... rather than trying to
        // identify each socket that is waiting for write and
        // creating an exact poll call to check for that, we just
        // check to see if there is anything waiting to write
        // and just check a little more often. since this is a situation
        // that is pretty rare, it shouldn't be a huge problem (i hope.)
        if(
                snooze < 20 || // clamp so we always wait at least 20ms even if service_channels wants otherwise
                spin || // service_channels wants us to spin
                Response_q.num_elems() > 0 || // we have items that need processing now
                MMChannel::any_ctrl_q_pending() || // we have ctrl messages waiting to send
                SimpleSocket::any_waiting_for_write() // we are waiting to write/connect to some socket
                )
        {
            GRTLOG("spin %d snooze %d", spin, snooze);
            ALOGI("fast poll snooze %d spin %d rq %d cq %d writes %d",
                  snooze,
                  spin,
                  Response_q.num_elems(),
                  MMChannel::any_ctrl_q_pending(),
                  SimpleSocket::any_waiting_for_write()
                  );
            // override, make it 20ms
            // in the background, we aren't in a huge hurry to get
            // things done, and don't want to spin too fast.
            snooze = 20;
        }
        Socketvec res;

        int secs = snooze / 1000;
        // avoid problems with overflow, there is nothing here
        // that requires usec accuracy
        int usecs = (snooze % 1000) * 1000;
        GRTLOG("longsleep %d %d", secs, usecs);
        ALOGI("long sleep %d %d", secs, usecs);
        int n = vc_winsock::poll_all(VC_SOCK_READ, res, secs, usecs);
        GRTLOG("wakeup %d", n, 0);
        ALOGI("wakeup %d", n);
        if(n < 0)
            return 1;

        for(int i = 0; i < res.num_elems(); ++i)
        {
            if(asock.socket_local_addr() == res[i]->socket_local_addr())
            {
                GRTLOG("req to exit", 0, 0);
                ALOGI("post sleep exit", 0);
                goto out;
            }
        }

    }
out:
    ;
    // note: we don't close the "sync" socket here, so the
    // requester will be blocked until we can clean up, and the
    // caller can clean up too and exit the process

    // explicitly stop transfers, even though we are not
    // going to resume, just doing an exit may be too abrupt
    // sometimes.
    //dwyco_suspend();
    if(just_suspend)
    {
        // note in the case of running in the same thread
        // as the main ui, the "lock socket" will be closed
        // when the destructor asock is run.
        dwyco_suspend();
        ALOGI("exit thread", 0);
    }
    else
    {
        dwyco_bg_exit();
        ALOGI("exit proc", 0);
    }
    //exit(0);
    return 0;
}

enum gops {
    NONE,
    JOIN,
    LEAVE
};

static gops Group_ops;
static DwString Group_to_join;
static DwString Group_pw;

static
void
check_join(int cmd, int ctx_id, const char *uid, int len_uid, const char *name, int len_name, int type, const char *value_elide, int val_len, int qid, int extra_arg)
{
    DwString nm;
    if(name)
    {
        nm = DwString(name, len_name);
    }
    if(cmd == DWYCO_SE_GRP_JOIN_OK)
    {
        GRTLOG("JOIN GROUP %s OK", nm.c_str(), 0);
        dwyco_set_setting("group/alt_name", nm.c_str());
        exit(0);
    }
    else if(cmd == DWYCO_SE_GRP_JOIN_FAIL)
    {
        GRTLOG("JOIN GROUP %s FAIL", nm.c_str(), 0);
        exit(1);
    }

}

static
void
DWYCOCALLCONV
dwyco_sync_login_result(const char *str, int what)
{
    if(what == 0)
    {
        // exit the process since there isn't anything more
        // we can do really, unless there are direct connects...
        // if there are no direct connections, for sure quit
        GRTLOG("bg db login fail %s", str, 0);
        dwyco_bg_exit();
        exit(0);
    }
    else
    {
        GRTLOG("bg db login ok", 0, 0);
        if(Group_ops == JOIN)
        {
            if(!dwyco_start_gj2(Group_to_join.c_str(), Group_pw.c_str()))
            {
                exit(1);
            }
        }
        else if(Group_ops == LEAVE)
        {
            dwyco_start_gj2("", "");
            exit(1);
        }

    }
}



DWYCOEXPORT
int
dwyco_background_sync(int port, const char *sys_pfx, const char *user_pfx, const char *tmp_pfx, const char *token, const char *grpname, const char *grppw)
{
#ifndef WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    //alarm(3600);
    srand(time(0));

    int s;
    s = get_funny_mutex(port);
    // first run, if the UI is blocking us, something is wrong
    if(s == -1)
        return 1;

    dwyco_trace_init();

    dwyco_set_fn_prefixes(sys_pfx, user_pfx, tmp_pfx);

    dwyco_set_client_version("dwycobg-sync", 12);
    dwyco_set_initial_invis(0);
    dwyco_set_login_result_callback(dwyco_sync_login_result);
    dwyco_set_system_event_callback(check_join);
    dwyco_set_disposition("background", 10);
    dwyco_bg_init();
    dwyco_set_setting("sync/eager", "1");
    if(grppw)
        dwyco_set_setting("group/join_key", grppw);
    if(token)
        dwyco_write_token(token);
    DwString gname;
    if(!grpname)
    {
        Group_ops = NONE;
    }
    else
    {
        if(!grppw)
            exit(1);
        gname = grpname;
        // empty group means you want to just enter the "no group" mode
        // and exit.
        if(gname.length() == 0)
        {
            //dwyco_start_gj2("", "");
            Group_ops = LEAVE;
            // note: probably need a "force" leave that will work even if
            // we can't contact the server.
            //exit(0);
        }
        else
        {
        const char *val;
        int tp;
        int len;
        if(!dwyco_get_setting("group/alt_name", &val, &len, &tp))
            exit(1);
        if(tp != DWYCO_TYPE_STRING)
            exit(1);
        DwString alt_name(val, len);
        if(alt_name.length() == 0)
        {
            DWYCO_LIST gs;
            dwyco_get_group_status(&gs);
            simple_scoped qgs(gs);
            gs = 0;
            if(qgs.get_long(DWYCO_GS_IN_PROGRESS))
            {
                if(qgs.get<DwString>(DWYCO_GS_GNAME) != alt_name)
                    exit(1);
                // just wait around until the protocol finishes
                Group_ops = NONE;
            }
            else
            {
                // not current in group, so attempt a join operation, but only
                // after we have successfully logged into the server. if the join
                // is successful, we exit. this may take awhile if it needs to
                // fetch a key from someone else
                Group_ops = JOIN;
                Group_to_join = gname;
                Group_pw = grppw;
            }
        }
        else if(alt_name != gname)
        {
            // asking for a group we aren't a part of, exit and make
            // them exit the group explicitly
            exit(1);
        }
        }

    }

    set_listen_state(1);

    dwyco_inhibit_sac(0);
    dwyco_inhibit_pal(0);

    if(dwyco_get_create_new_account())
    {

        dwyco_create_bootstrap_profile("bg-syncer", 9, gname.c_str(), gname.length(), "", 0, "", 0);
    }
    dwyco_set_local_auth(1);
    // note: if the account didn't exist (ie, fresh install) this
    // will create a new uid. if for some reason, the server cannot be
    // contacted and the account info recorded, you can still run
    // the service loop and process direct connections. once the
    // server is contacted you'll get proper offline messaging and so on.
    dwyco_finish_startup();

    //int comsock = -1;
    vc asock = vc(VC_SOCKET_STREAM);
    asock.socket_init(s, vctrue);
    dwyco_signal_msg_cond();
    int signaled = 0;
    int started_fetches = 0;

    while(1)
    {
        int spin = 0;
        int snooze = dwyco_service_channels(&spin);

#ifdef WIN32
        if(accept(s, 0, 0) != INVALID_SOCKET)
        {
            break;
        }
        else
        {
            int e = WSAGetLastError();
            if(e != WSAEWOULDBLOCK)
                return 1;
        }
#else
        if(accept(s, 0, 0) != -1)
        {
            break;
        }
        else if(!(errno == EWOULDBLOCK || errno == EAGAIN))
            return 1;
#endif
        if(dwyco_get_rescan_messages())
        {
            GRTLOG("rescan %d %d", started_fetches, signaled);
            dwyco_set_rescan_messages(0);
            ns_dwyco_background_processing::fetch_to_inbox();
            GRTLOG("rescan2 %d %d", started_fetches, signaled);
            int tmp;
            if((tmp = sql_count_tag("_inbox")) > signaled)
            {
                GRTLOG("signaling newcount %d", tmp, 0);
                signaled = tmp;
                dwyco_signal_msg_cond();
            }
        }
        // note: this is a bit sloppy... rather than trying to
        // identify each socket that is waiting for write and
        // creating an exact poll call to check for that, we just
        // check to see if there is anything waiting to write
        // and just check a little more often. since this is a situation
        // that is pretty rare, it shouldn't be a huge problem (i hope.)
        if(spin || Response_q.num_elems() > 0 ||
                MMChannel::any_ctrl_q_pending() || SimpleSocket::any_waiting_for_write())
        {
            GRTLOG("spin %d short sleep", spin, 0);
#if 0
#ifdef WIN32
            SleepEx(100, 0);
#else
            usleep(100000);
#endif
#endif
            // override, make is 20ms
            snooze = 20;
        }
        //else
        {
            //usleep(500000);
            Socketvec res;

            int secs = snooze / 1000;
            // avoid problems with overflow, there is nothing here
            // that requires usec accuracy
            int usecs = (snooze % 1000) * 1000;
            GRTLOG("longsleep %d %d", secs, usecs);
            int n = vc_winsock::poll_all(VC_SOCK_READ, res, secs, usecs);
            GRTLOG("wakeup %d", n, 0);
            if(n < 0)
                return 1;

            for(int i = 0; i < res.num_elems(); ++i)
            {
                if(asock.socket_local_addr() == res[i]->socket_local_addr())
                {
                    GRTLOG("req to exit", 0, 0);
                    goto out;
                }
            }
        }
    }
out:
    ;
    // note: we don't close the "sync" socket here, so the
    // requester will be blocked until we can clean up, and the
    // caller can clean up too and exit the process

    // explicitly stop transfers, even though we are not
    // going to resume, just doing an exit may be too abrupt
    // sometimes.
    //dwyco_suspend();
    dwyco_bg_exit();
    //exit(0);
    return 0;
}
