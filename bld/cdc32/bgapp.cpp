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

using namespace dwyco;

#ifndef WIN32
#include "poll.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

// this is a simple condition variable we use to signal
// the java stuff to re-check for messages (so it can post
// a notification.)
static pthread_cond_t Msg_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t Msg_cond_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

DWYCOEXPORT
void
dwyco_signal_msg_cond()
{
#ifndef WIN32
    pthread_mutex_lock(&Msg_cond_mutex);
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
    pthread_cond_wait(&Msg_cond, &Msg_cond_mutex);
    pthread_mutex_unlock(&Msg_cond_mutex);
#endif

}


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
    const int tries = 100;
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


DWYCOEXPORT
int
dwyco_background_processing(int port, int exit_if_outq_empty, const char *sys_pfx, const char *user_pfx, const char *tmp_pfx, const char *token)
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

    //dwyco_set_login_result_callback(dwyco_db_login_result);
    dwyco_set_fn_prefixes(sys_pfx, user_pfx, tmp_pfx);

    // quick check, and nothing else
    if(exit_if_outq_empty == 2)
    {
        int tmp = msg_outq_empty();
#ifdef WIN32
        closesocket(s);
#else
        close(s);
#endif
        return tmp;
    }

    dwyco_set_client_version("dwycobg", 7);
    dwyco_set_initial_invis(1);
    dwyco_set_login_result_callback(dwyco_background_db_login_result);
    dwyco_bg_init();
    if(token)
        dwyco_write_token(token);

    set_listen_state(0);
    // for now, don't let any channels get setup via the
    // server ... not strictly necessary, but until we get the
    // calling stuff sorted out (needs a protocol change to alert
    // regarding incoming calls, etc.) we just let everything go
    // via the server.
    dwyco_inhibit_sac(1);
    dwyco_inhibit_pal(1);

    if(dwyco_get_create_new_account())
    {
        // only run on existing accounts
        return 1;
    }

    dwyco_set_local_auth(1);
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
#ifdef WIN32
            SleepEx(100, 0);
#else
            usleep(100000);
#endif
        }
        else
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

static
void
process_joins()
{
    DWYCO_LIST tl;
    dwyco_get_tagged_mids(&tl, "_special");
    simple_scoped qtl(tl);
    for(int i = 0; i < qtl.rows(); ++i)
    {
        DwString mid = qtl.get<DwString>(i, DWYCO_TAGGED_MIDS_MID);
        int d = dwyco_handle_join(mid.c_str());
        if(d == -1)
            continue;
        //dwyco_unset_msg_tag(mid.c_str(), "_special");
        DwString uid = DwString::from_hex(qtl.get<DwString>(i, DWYCO_TAGGED_MIDS_HEX_UID));
        dwyco_delete_saved_message(uid.c_str(), uid.length(), mid.c_str());
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
        dwyco_start_gj("", 0, "");
    }
}

namespace dwyco {
extern int Eager_sync;
}

DWYCOEXPORT
int
dwyco_background_sync(int port, const char *sys_pfx, const char *user_pfx, const char *tmp_pfx, const char *token)
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
    dwyco::Eager_sync = 1;
    //dwyco_set_login_result_callback(dwyco_db_login_result);
    dwyco_set_fn_prefixes(sys_pfx, user_pfx, tmp_pfx);

    dwyco_set_client_version("dwycobg-sync", 12);
    dwyco_set_initial_invis(0);
    dwyco_set_login_result_callback(dwyco_sync_login_result);
    dwyco_bg_init();
    if(token)
        dwyco_write_token(token);

    set_listen_state(1);
    // for now, don't let any channels get setup via the
    // server ... not strictly necessary, but until we get the
    // calling stuff sorted out (needs a protocol change to alert
    // regarding incoming calls, etc.) we just let everything go
    // via the server.
    dwyco_inhibit_sac(0);
    dwyco_inhibit_pal(0);

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
            process_joins();
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
#ifdef WIN32
            SleepEx(100, 0);
#else
            usleep(100000);
#endif
        }
        else
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