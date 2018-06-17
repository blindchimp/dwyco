
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

#include "uicfg.h"
#ifndef LINUX
#ifdef __BORLANDC__
#include <dir.h>
#else
#include <direct.h>
#endif
#include <io.h>
#endif
#include "dirth.h"
#include "ratetwkr.h"
#include "usercnfg.h"
#include "vfwinvst.h"
#include "rawfiles.h"
#include "vidinput.h"
#include "cllaccpt.h"
#include "zapadv.h"
#include "qtab.h"
#include "jqtab.h"
#include "qpol.h"
#include "sqrs.h"

#include "vc.h"
#include "vcwsock.h"
#include "lhboot.h"
#include "vccrypt2.h"

#include "aq.h"
#include "dwlog.h"
#include "gvchild.h"
#include "jchuff.h"
#include "jdhuff.h"
#include "audchk.h"
#include "snds.h"
#include "doinit.h"
#include "jccolor.h"
#include "jdcolor.h"
#include "dwrtlog.h"
#include "qauth.h"
#include "qmsg.h"
#include "prfcache.h"
#include "pkcache.h"
#include "mmchan.h"
#include "callq.h"
#include "asshole.h"
#include "netdiag.h"
#include "calllive.h"
#include "aconn.h"
#include "sysattr.h"
#include "fnmod.h"
#include "xinfo.h"
#include "dhsetup.h"
#include "dwyco_rand.h"

vc Myhostname;
DwLog *Log;
CRITICAL_SECTION Audio_lock;
int GodMode = 0;
vc TheMan;
int Disable_upnp;

extern vc Current_user_lobbies;
extern CRITICAL_SECTION Audio_mixer_shutdown_lock;

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
        void init_stats();
        init_stats();
        Log = new DwLog(newfn(logname).c_str());
#ifdef DW_RTLOG
        if(!RTLog)
        {
            // leave RTLog 0 to inhibit logging in newfn
            DwRTLog *tmp = new DwRTLog(newfn("rtlog.out").c_str(), 1000);
            RTLog = tmp;
            init_rtlog();
        }
#endif
        Log->make_entry("system starting up");
#ifndef LHCDC32
        // note: this ought to be fixed, so that there is nothing
        // going to stdout from this lib. it mucks up things like
        // curses
        // attach stdout to a file
#if 0
        if(!freopen(newfn("c.out").c_str(), "w", stdout))
            Log->make_entry("can't redirect stdout");
        if(!freopen(newfn("c.out").c_str(), "w", stderr))
            Log->make_entry("can't redirect stderr");
        setbuf(stdout, 0);
        setbuf(stderr, 0);
#endif
#endif
        if(access(newfn("inprogress").c_str(), 0) == -1)
            if(mkdir(newfn("inprogress").c_str()) == -1)
                Log->make_entry("can't create inprogress dir");
        if(access(newfn("outbox").c_str(), 0) == -1)
            if(mkdir(newfn("outbox").c_str()) == -1)
                Log->make_entry("can't create outbox dir");
        if(access(newfn("inbox").c_str(), 0) == -1)
            if(mkdir(newfn("inbox").c_str()) == -1)
                Log->make_entry("can't create inbox dir");
        if(access(newfn("trash").c_str(), 0) == -1)
            if(mkdir(newfn("trash").c_str()) == -1)
                Log->make_entry("can't create trash dir");
        if(access(newfn("xfer").c_str(), 0) == -1)
            if(mkdir(newfn("xfer").c_str()) == -1)
                Log->make_entry("can't create xfer dir");
        // this is so important, don't leave it till later
        init_prf_cache();
        init_pk_cache();

        //Cur_msgs = vc(VC_VECTOR);
        InitializeCriticalSection(&Audio_lock);
        InitializeCriticalSection(&Audio_mixer_shutdown_lock);

        recover_inprogress();
        //init_snds();
        rgb_ycc_start();
        build_ycc_rgb_table();
        void init_dct();
        init_dct();
        init_huff_encode();
        init_huff_decode();
        QTAB::qtab_init();
        JQTAB::jqtab_init();
        qpol_init();
        gen_sqr();
        gen_abs();
#ifdef DWYCO_CODEC
        gen_magqtabs();
#endif

        vc::setup_logs();
        vc::non_lh_init();
        vc_winsock::startup();

        // ok, this is bad, but the man pages do spec numbers for upper bound
        // and HOST_NAME_MAX seems to be missing in action some places.
        char hostname[1024];
        if(gethostname(hostname, sizeof(hostname)) != 0)
            Myhostname = "unknown";
        else
            Myhostname = hostname;

        init_entropy();
        extern vc DH_public;

        vclh_dh_init_std();
        DH_public = vclh_dh_setup();

        dh_init();

        RTUserDefaults.load();
#ifdef _Windows
        if(vc_winsock::wsa_data.iMaxUdpDg != 0)
        {
            if(RTDefaults.get_max_udp_bytes() > vc_winsock::wsa_data.iMaxUdpDg)
                RTDefaults.set_max_udp_bytes(vc_winsock::wsa_data.iMaxUdpDg);
            if(RTUserDefaults.get_max_udp_bytes() > vc_winsock::wsa_data.iMaxUdpDg)
                RTUserDefaults.set_max_udp_bytes(vc_winsock::wsa_data.iMaxUdpDg);
        }
#endif
        TProfile t("admin", INI_FILENAME);
        // note: initvfw makes tweaks to default values
        // based on whether we are doing the wizard or not.
        initvfw();
        check_audio_device();
        if(!Audio_hw_full_duplex)
            Log->make_entry("audio hardware is half-duplex");
        else
            Log->make_entry("audio hardware claims full-duplex");

        VFWInvestigateData.load();
        //CTUserDefaults.load();
        RawFilesData.load();
        VidInputData.load();
        CallAcceptanceData.load();

        ZapAdvData.load();
        DwNetConfigData.load();
        Disable_upnp = DwNetConfigData.get_disable_upnp();
        extern int Media_select;
        switch(DwNetConfigData.get_call_setup_media_select())
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

        //ShowDirectoryData.load();

#ifdef EMODEL
        EntropyModel::init_all();
#endif
        // note: mdiclient menus get loaded when client is inited
        // in cdc16App::InitMainWindow
        // note: need this down here because some of this
        // initialization depends on the rating the user has
        // selected.
        init_dirth();
        init_qauth();
        UserConfigData.load();
        // note: qmsg now depends on My_UID
        init_qmsg();
        //init_home_server(); note: it is a mistake that this got in here
        // need to delete it in parents of this branch
        // can't load the inbox until we have a uid
        // figured out
        //stun_pool_init();
#ifndef MACOSX
        init_netdiag(); // note: can't do netdiag until stun_server is known.
#endif
        init_callq();
        init_assholes();
        init_sysattr();
        init = 1;
        Log->make_entry("init done");
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
        void init_stats();
        init_stats();
#ifdef DW_RTLOG
        Log = new DwLog(logname);
        if(!RTLog)
        {
            // leave RTLog 0 to inhibit logging in newfn
            DwRTLog *tmp = new DwRTLog(newfn("rtlog.out").c_str(), 1000);
            RTLog = tmp;
            init_rtlog();
        }
#endif
        Log->make_entry("system starting up");
#ifndef LHCDC32
        // note: this ought to be fixed, so that there is nothing
        // going to stdout from this lib. it mucks up things like
        // curses
        // attach stdout to a file
        if(!freopen(newfn("c.out").c_str(), "w", stdout))
            Log->make_entry("can't redirect stdout");
        if(!freopen(newfn("c.out").c_str(), "w", stderr))
            Log->make_entry("can't redirect stderr");
        setbuf(stdout, 0);
        setbuf(stderr, 0);
#endif
        //Cur_msgs = vc(VC_VECTOR);
        InitializeCriticalSection(&Audio_lock);
        InitializeCriticalSection(&Audio_mixer_shutdown_lock);

        rgb_ycc_start();
        build_ycc_rgb_table();
        void init_dct();
        init_dct();
        init_huff_encode();
        init_huff_decode();

        QTAB::qtab_init();
        JQTAB::jqtab_init();
        qpol_init();
        gen_sqr();
        gen_abs();
#ifdef DWYCO_CODEC
        gen_magqtabs();
#endif

        vc::setup_logs();
        vc::non_lh_init();
        vc_winsock::startup();

        char hostname[255];
        if(gethostname(hostname, sizeof(hostname)) != 0)
            Myhostname = "unknown";
        else
            Myhostname = hostname;

        RTUserDefaults.load();
#ifdef _Windows
        if(vc_winsock::wsa_data.iMaxUdpDg != 0)
        {
            if(RTDefaults.get_max_udp_bytes() > vc_winsock::wsa_data.iMaxUdpDg)
                RTDefaults.set_max_udp_bytes(vc_winsock::wsa_data.iMaxUdpDg);
            if(RTUserDefaults.get_max_udp_bytes() > vc_winsock::wsa_data.iMaxUdpDg)
                RTUserDefaults.set_max_udp_bytes(vc_winsock::wsa_data.iMaxUdpDg);
        }
#endif
        TProfile t("admin", INI_FILENAME);

        // have to reconfigure a little bit for 0.80
        UserConfigData.load();
        VFWInvestigateData.load();
        //CTUserDefaults.load();
        RawFilesData.load();
        VidInputData.load();
        CallAcceptanceData.load();
        ZapAdvData.load();
        DwNetConfigData.load();
        Disable_upnp = 1;

        init_sysattr();
        init = 1;
        Log->make_entry("init done");
    }
}

// this is used instead of the main initialization
// when all you want to do is call service_channels to
// send and receive messages that have already been queued. no multimedia capture
// or display stuff is initialized.
void
init_bg_msg_send(const char *logname)
{
    static int init = 0;
    if(!init)
    {
        dwyco_srand(time(0));
        TheMan = vc(VC_BSTRING, "\x5a\x09\x8f\x3d\xf4\x90\x15\x33\x1d\x74", 10);
        //No_direct_msgs = vc(VC_SET);
        Current_user_lobbies = vc(VC_TREE);
        InitializeCriticalSection(&Audio_lock);
        InitializeCriticalSection(&Audio_mixer_shutdown_lock);
        void init_stats();
        init_stats();

        Log = new DwLog(logname);
#ifdef DW_RTLOG
        if(!RTLog)
        {
            // leave RTLog 0 to inhibit logging in newfn
            DwRTLog *tmp = new DwRTLog(newfn("rtlog.out").c_str(), 1000);
            RTLog = tmp;
            init_rtlog();
        }
#endif
        Log->make_entry("background system starting up");
#ifndef LHCDC32
        // note: this ought to be fixed, so that there is nothing
        // going to stdout from this lib. it mucks up things like
        // curses
        // attach stdout to a file
        if(!freopen(newfn("c.out").c_str(), "w", stdout))
            Log->make_entry("can't redirect stdout");
        if(!freopen(newfn("c.out").c_str(), "w", stderr))
            Log->make_entry("can't redirect stderr");
        setbuf(stdout, 0);
        setbuf(stderr, 0);
#endif

        // this is important, don't leave it till later
        init_prf_cache();
        init_pk_cache();
        recover_inprogress();

        //Cur_msgs = vc(VC_VECTOR);

        vc::setup_logs();
        vc::non_lh_init();
        vc_winsock::startup();

        char hostname[255];
        if(gethostname(hostname, sizeof(hostname)) != 0)
            Myhostname = "unknown";
        else
            Myhostname = hostname;

        init_entropy();
        extern vc DH_public;

        vclh_dh_init_std();
        DH_public = vclh_dh_setup();

        dh_init();

        RTUserDefaults.load();
#ifdef _Windows
        if(vc_winsock::wsa_data.iMaxUdpDg != 0)
        {
            if(RTDefaults.get_max_udp_bytes() > vc_winsock::wsa_data.iMaxUdpDg)
                RTDefaults.set_max_udp_bytes(vc_winsock::wsa_data.iMaxUdpDg);
            if(RTUserDefaults.get_max_udp_bytes() > vc_winsock::wsa_data.iMaxUdpDg)
                RTUserDefaults.set_max_udp_bytes(vc_winsock::wsa_data.iMaxUdpDg);
        }
#endif

        init_dirth();
        init_qauth();

        UserConfigData.load();
        // note: qmsg now depends on My_UID
        init_qmsg();

        // note: the background send does server-only sends, so this should never
        // get used.
        //init_callq();

        VFWInvestigateData.load();
        //CTUserDefaults.load();
        RawFilesData.load();
        VidInputData.load();
        CallAcceptanceData.load();
        ZapAdvData.load();
        DwNetConfigData.load();
        Disable_upnp = 1;

        init_sysattr();
        init = 1;
        Log->make_entry("background init done");
    }
}

// this doesn't do a perfect cleanup, mainly because we assume the
// process is going to be exiting afterwards.

void
exit_bg_msg_send()
{
    save_qmsg_state();
    save_entropy();
    Log->make_entry("background exit");

    vc::non_lh_exit();
    vc::shutdown_logs();

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

void
exit_codec()
{
    save_qmsg_state();
    save_entropy();
    Log->make_entry("exit");

    vc::non_lh_exit();
    vc::shutdown_logs();

#ifndef DWYCO_POWERBROWSE
    clean_cruft();
    clean_profile_cache(120, 6000);
    clean_pk_cache(365, 1500);
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

