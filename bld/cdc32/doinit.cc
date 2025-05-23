
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/doinit.cc 1.27 1999/01/10 16:09:31 dwight Checkpoint $
 */

#include "ezset.h"
#ifndef LINUX
#include <direct.h>
#include <io.h>
#endif
#include "dirth.h"
#include "qtab.h"
#include "jqtab.h"
#include "qpol.h"
#include "sqrs.h"

#include "vc.h"
#include "vcwsock.h"

#include "aq.h"
#include "dwlog.h"
#include "jchuff.h"
#include "jdhuff.h"
#include "audchk.h"
#include "doinit.h"
#include "jccolor.h"
#include "jdcolor.h"
#include "dwrtlog.h"
#include "qauth.h"
#include "qmsg.h"
#include "profiledb.h"
#include "mmchan.h"
#include "callq.h"
#include "asshole.h"
#include "netdiag.h"
#include "calllive.h"
#include "aconn.h"
#include "sysattr.h"
#include "fnmod.h"
#include "dhsetup.h"
#include "dwyco_rand.h"
#include "dirth.h"
#include "cdcpal.h"
#include "se.h"
#include "qdirth.h"
#include "ta.h"
#include "dhgsetup.h"
#include "pgdll.h"

using namespace dwyco;

CRITICAL_SECTION Audio_lock;
extern int Media_select;
extern CRITICAL_SECTION Audio_mixer_shutdown_lock;

namespace dwyco {

vc Myhostname;
vc TheMan;

void init_dct();
void init_stats();

void
init_codec(const char *logname)
{
    static int init = 0;
    if(!init)
    {
        dwyco_srand(time(0));
        TheMan = vc(VC_BSTRING, "\x5a\x09\x8f\x3d\xf4\x90\x15\x33\x1d\x74", 10);
        //No_direct_msgs = vc(VC_SET);
        Current_user_lobbies = vc(VC_TREE);
        // ok, this is bad, but the man pages do spec numbers for upper bound
        // and HOST_NAME_MAX seems to be missing in action some places.
        char hostname[1024];
        if(gethostname(hostname, sizeof(hostname)) != 0)
            Myhostname = "unknown";
        else
            Myhostname = hostname;

        init_stats();

#ifdef DW_RTLOG
        if(!RTLog)
        {
            // leave RTLog 0 to inhibit logging in newfn
            DwRTLog *tmp = new DwRTLog(newfn("rtlog.out").c_str());
            RTLog = tmp;
            init_rtlog();
        }
#endif
        Log_make_entry("system starting up");

        // note: this ought to be fixed, so that there is nothing
        // going to stdout from this lib. it mucks up things like
        // curses
        // attach stdout to a file
#if 0
        if(!freopen(newfn("c.out").c_str(), "w", stdout))
            Log_make_entry("can't redirect stdout");
        if(!freopen(newfn("c.out").c_str(), "w", stderr))
            Log_make_entry("can't redirect stderr");
        setbuf(stdout, 0);
        setbuf(stderr, 0);
#endif

        if(access(newfn("inprogress").c_str(), 0) == -1)
            if(mkdir(newfn("inprogress").c_str()) == -1)
                Log_make_entry("can't create inprogress dir");
        if(access(newfn("outbox").c_str(), 0) == -1)
            if(mkdir(newfn("outbox").c_str()) == -1)
                Log_make_entry("can't create outbox dir");
        if(access(newfn("trash").c_str(), 0) == -1)
            if(mkdir(newfn("trash").c_str()) == -1)
                Log_make_entry("can't create trash dir");
        if(access(newfn("xfer").c_str(), 0) == -1)
            if(mkdir(newfn("xfer").c_str()) == -1)
                Log_make_entry("can't create xfer dir");
        init_dhgdb();
        init_sql_settings();
        init_aconn();
        init_entropy();
        dh_init();

        // this is so important, don't leave it till later
        init_qauth();
        init_prfdb();

        //Cur_msgs = vc(VC_VECTOR);
        InitializeCriticalSection(&Audio_lock);
        InitializeCriticalSection(&Audio_mixer_shutdown_lock);

        recover_inprogress();
        //init_snds();
        rgb_ycc_start();
        build_ycc_rgb_table();
        init_dct();
#ifdef DWYCO_DCT_CODER
        init_huff_encode();
#endif
        init_huff_decode();
        QTAB::qtab_init();
        JQTAB::jqtab_init();
        qpol_init();
#ifdef DWYCO_DCT_CODER
        gen_sqr();
        gen_abs();
#endif
#ifdef DWYCO_CODEC
        gen_magqtabs();
#endif

        vc::setup_logs();
        vc::non_lh_init();
        vc_winsock::startup();

        //check_audio_device();
        if(!Audio_hw_full_duplex)
            Log_make_entry("audio hardware is half-duplex");
        else
            Log_make_entry("audio hardware claims full-duplex");

        switch((int)get_settings_value("net/call_setup_media_select"))
        {
        default:
        case CSMS_VIA_HANDSHAKE:
            Media_select = MEDIA_VIA_HANDSHAKE;
            break;
        case CSMS_TCP_ONLY:
            Media_select = MEDIA_TCP_VIA_PROXY;
            break;
        case CSMS_UDP_ONLY:
            Media_select = MEDIA_UDP_VIA_STUN;
            break;

        }

        // for testing
        //Media_select = MEDIA_TCP_VIA_PROXY;

#ifdef DWYCO_CODEC
        EntropyModel::init_all();
#endif
        init_dirth();
        init_qmsg();

        //stun_pool_init();
        init_netdiag(); // note: can't do netdiag until stun_server is known.
        init_callq();
#ifdef DWYCO_ASSHAT
        init_assholes();
#endif

        init_sysattr();
        //Current_alternate.value_changed.connect_ptrfun(drop_all_sync_calls);
        init = 1;
        Log_make_entry("init done");
    }
}

// this is internal api used mostly for enabling just enough
// so you can get a preview from a dwyco dyc file and exit
// the program.
void
simple_init_codec(const char *logname)
{
    static int init = 0;
    if(!init)
    {
        init_pval();
        dwyco_srand(time(0));
        TheMan = vc(VC_BSTRING, "\x5a\x09\x8f\x3d\xf4\x90\x15\x33\x1d\x74", 10);
        //No_direct_msgs = vc(VC_SET);
        Current_user_lobbies = vc(VC_TREE);
        char hostname[255];
        if(gethostname(hostname, sizeof(hostname)) != 0)
            Myhostname = "unknown";
        else
            Myhostname = hostname;

        init_stats();
#ifdef DW_RTLOG
        if(!RTLog)
        {
            // leave RTLog 0 to inhibit logging in newfn
            DwRTLog *tmp = new DwRTLog(newfn("rtlog.out").c_str());
            RTLog = tmp;
            init_rtlog();
        }
#endif
        Log_make_entry("system starting up");
        init_sql_settings();
        //init_aconn();
        //init_dhg();

        // note: this ought to be fixed, so that there is nothing
        // going to stdout from this lib. it mucks up things like
        // curses
        // attach stdout to a file
        if(!freopen(newfn("c.out").c_str(), "w", stdout))
            Log_make_entry("can't redirect stdout");
        if(!freopen(newfn("c.out").c_str(), "w", stderr))
            Log_make_entry("can't redirect stderr");
        setbuf(stdout, 0);
        setbuf(stderr, 0);

        //Cur_msgs = vc(VC_VECTOR);
        InitializeCriticalSection(&Audio_lock);
        InitializeCriticalSection(&Audio_mixer_shutdown_lock);

        rgb_ycc_start();
        build_ycc_rgb_table();
        init_dct();
#ifdef DWYCO_DCT_CODER
        init_huff_encode();
#endif
        init_huff_decode();

        QTAB::qtab_init();
        JQTAB::jqtab_init();
        qpol_init();
#ifdef DWYCO_DCT_CODER
        gen_sqr();
        gen_abs();
#endif
#ifdef DWYCO_CODEC
        gen_magqtabs();
#endif

        vc::setup_logs();
        vc::non_lh_init();
        vc_winsock::startup();

        init_sysattr();
        init = 1;
        Log_make_entry("init done");
    }
}

// this is used instead of the main initialization
// when all you want to do is call service_channels to
// send and receive messages that have already been queued. no multimedia capture
// or display stuff is initialized.
static int Bg_msg_send_init;
void
init_bg_msg_send(const char *logname)
{
    if(!Bg_msg_send_init)
    {
        dwyco_srand(time(0));
        TheMan = vc(VC_BSTRING, "\x5a\x09\x8f\x3d\xf4\x90\x15\x33\x1d\x74", 10);
        //No_direct_msgs = vc(VC_SET);
        Current_user_lobbies = vc(VC_TREE);
        InitializeCriticalSection(&Audio_lock);
        InitializeCriticalSection(&Audio_mixer_shutdown_lock);
        char hostname[255];
        if(gethostname(hostname, sizeof(hostname)) != 0)
            Myhostname = "unknown";
        else
            Myhostname = hostname;
        init_stats();

#ifdef DW_RTLOG
        if(!RTLog)
        {
            // leave RTLog 0 to inhibit logging in newfn
            DwRTLog *tmp = new DwRTLog(newfn("rtlog.out").c_str());
            RTLog = tmp;
            init_rtlog();
        }
#endif
        if(access(newfn("inprogress").c_str(), 0) == -1)
            if(mkdir(newfn("inprogress").c_str()) == -1)
                Log_make_entry("can't create inprogress dir");
        if(access(newfn("outbox").c_str(), 0) == -1)
            if(mkdir(newfn("outbox").c_str()) == -1)
                Log_make_entry("can't create outbox dir");
        if(access(newfn("trash").c_str(), 0) == -1)
            if(mkdir(newfn("trash").c_str()) == -1)
                Log_make_entry("can't create trash dir");
        if(access(newfn("xfer").c_str(), 0) == -1)
            if(mkdir(newfn("xfer").c_str()) == -1)
                Log_make_entry("can't create xfer dir");

        Log_make_entry("background system starting up");
        init_dhgdb();
        init_sql_settings();
        init_aconn();
        init_entropy();
        dh_init();
        //init_dhg();
        // note: this ought to be fixed, so that there is nothing
        // going to stdout from this lib. it mucks up things like
        // curses
        // attach stdout to a file
        if(!freopen(newfn("c.out").c_str(), "w", stdout))
            Log_make_entry("can't redirect stdout");
        if(!freopen(newfn("c.out").c_str(), "w", stderr))
            Log_make_entry("can't redirect stderr");
        setbuf(stdout, 0);
        setbuf(stderr, 0);

        // this is important, don't leave it till later
        init_prfdb();
        recover_inprogress();

        //Cur_msgs = vc(VC_VECTOR);

        vc::setup_logs();
        vc::non_lh_init();
        vc_winsock::startup();

        init_dirth();
        init_qauth();
        // note: qmsg now depends on My_UID
        init_qmsg();

        // note: the background send does server-only sends, so this should never
        // get used. however, the syncing stuff might initiate some calls
        // note also there is no reason to exit the callq. it *might* make sense
        // to cancel calls that are being set up if, for example, you are
        // switching the service channels from one thread to another.
        init_callq();

        init_sysattr();

        Bg_msg_send_init = 1;
        Log_make_entry("background init done");
    }
}


void
exit_bg_msg_send()
{
    if(!Bg_msg_send_init)
        return;

    save_qmsg_state();
    save_entropy();
    Log_make_entry("background exit");

    // note: mmchan depends on being able to use some of the
    // other stuff below, so we clean it up first. there
    // may be other dependencies lurking in here as well...
    MMChannel::exit_mmchan();
    // empty out all the system messages
    while(se_process() || dirth_poll_response())
        ;

    //exit_qmsg();
    exit_pal();
    exit_prfdb();

    // note: don't bother turning off network since
    // we are going to exit soon anyways. note this
    // is just to avoid a crash with uv sockets. really
    // the whole init/exit thing needs to be cleaned up
    // in VC.
    //vc::non_lh_exit();
    vc::shutdown_logs();

#ifdef DW_RTLOG
    RTLog->flush_to_file();
#endif

    Bg_msg_send_init = 0;

}

void
exit_codec()
{
    // do this here mainly so we can get some network
    // debugging output when the program exits
    MMChannel::exit_mmchan();
    save_qmsg_state();
    save_entropy();
    // note: this analyzes the database, which can be a huge
    // win with sqlite
    exit_qmsg();
    Log_make_entry("exit");



    vc::non_lh_exit();
    vc::shutdown_logs();

#ifndef DWYCO_POWERBROWSE
    clean_cruft();
#if 0
#ifdef ANDROID
    clean_profile_cache(60, 1000);
#else
    clean_profile_cache(120, 6000);
#endif
    clean_pk_cache(365, 1500);
#endif
#endif
#ifdef DW_RTLOG
    RTLog->flush_to_file();
#endif
#ifdef LEAK_CLEANUP
    // note: mmchan depends on being able to use some of the
    // other stuff below, so we clean it up first. there
    // may be other dependencies lurking in here as well...
    MMChannel::exit_mmchan();
    //void exit_shwdrctr();
    //exit_shwdrctr();
    void exit_qmsg();
    exit_qmsg();
    void exit_pal();
    exit_pal();
    exit_prf_cache();
    exit_pk_cache();
    void exit_dirth();
    exit_dirth();

    stun_pool_exit();
#endif

}

}
