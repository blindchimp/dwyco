
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//$Header: g:/dwight/repo/cdc32/rcs/mmchan.cc 1.37 1999/01/10 16:09:35 dwight Checkpoint $
#include <typeinfo>

#include "chatdisp.h"
#include "dwstr.h"
#include "mmchan.h"
#include "netvid.h"
#include "codec.h"
#include "vidaq.h"
#include "aqkey.h"

#include "tpgmdec.h"
#include "aq.h"
#include "tpgmcod.h"
#include "gvchild.h"
#include "doinit.h"
#include "macs.h"
#include "vcwsock.h"
#include "dwlog.h"

#include "aqaud.h"
#include "audout.h"
#include "audwin.h"
#include "audo.h"
#include "audi.h"
#include "audth.h"
#include "audchk.h"
//#include "lpcconv.h"

#include "dirth.h"

#include "qauth.h"
#include "qmsg.h"
#include "cdcver.h"
#include "filetube.h"
#include "vidaq.h"
#include "callq.h"
#ifdef SPAZ_CODEC
#include "cdcspaz.h"
#include "pgmspaz.h"
#include "pgmjc3.h"
#endif
#ifndef DWYCO_NO_THEORA_CODEC
#include "theoracol.h"
#endif
#include "aconn.h"

#include "dwrtlog.h"
#include "ta.h"
#include "fnmod.h"
#include "sysattr.h"
#include "dhsetup.h"
#include "vcudh.h"
#include "profiledb.h"
#include "sha.h"
#include "qdirth.h"
#include "dwscoped.h"
#include "dhgsetup.h"
#include "ezset.h"
#include "qmsgsql.h"
#include "netlog.h"
#include "vccrypt2.h"

using namespace dwyco;

extern vc Current_room;
extern vc My_connection;
extern DwString dwyco::Schema_version_hack;

#ifdef _Windows
#define strcasecmp strcmpi
#endif

#undef index
using namespace CryptoPP;

#define NEGO_TIMEOUT (120 * 1000)
#define PACKET_DROP_INTERVAL 10000


#define FAILRET(x) do { {fail_reason = (x); Log_make_entry(x); return 0;} } while(0)

GetWindowCallback MMChannel::get_mdi_client_window_callback;
GetWindowCallback MMChannel::get_main_window_callback;
StatusCallback MMChannel::connection_list_changed_callback;
vc MMChannel::clc_arg1;
void *MMChannel::clc_arg2;
ValidPtr MMChannel::clc_arg3;
StatusCallback MMChannel::call_accepted_callback;
int MMChannel::Exclusive_audio;
int MMChannel::Exclusive_audio_id;

ChatDisplayCallback MMChannel::gen_public_chat_display_callback;
ChatDisplayCallback MMChannel::gen_private_chat_display_callback;
CallAppearedCallback MMChannel::call_appeared_callback;
//UIPopupCallback MMChannel::popup_zap_accept_box_callback;
UIPopupCallback MMChannel::set_progress_status_callback;
UIPopupCallback MMChannel::popup_message_box_callback;
UIPopupCallback MMChannel::popup_update_box_callback;

StatusCallback2 MMChannel::uc_message_callback;
CallScreeningCallback MMChannel::call_screening_callback;

int MMChannel::Moron_dork_mode;
int MMChannel::Session_id = 1;

// Conference mode, mostly defunct
int Conf;
int Soft_preview_on;
int Auto_squelch = 1;

DwVecP<MMChannel> MMChannel::MMChannels;
DwTreeKaz<MMChannel *, int> *MMChannel::AllChan2;
int MMChannel::Sync_receivers = 1;
int MMChannel::Auto_sync = 1;
DwTimer MMChannel::Bw_adj_timer("bw_adj");
dwyco::sigprop<vc> MMChannel::My_disposition;
//#define DWYCO_THREADED_ENCODE

#if defined(DWYCO_THREADED_ENCODE) && defined(LINUX)
#include "dwpipe.h"
#include <pthread.h>
struct coder_input
{
    coder_input() {
        coder = 0;
        ref = 0;
        cols = 0;
        rows = 0;
        y = 0;
        cb = 0;
        cr = 0;
        bits = 0;
        fmt = 0;
        captime = 0;
        show_only = 0;
    }
    DwCoder *coder;
    int ref;
    int cols;
    int rows;
    void *bits;
    void *y;
    void *cb;
    void *cr;
    int fmt;
    unsigned long captime;
    int show_only;

};
struct coder_finished
{
    coder_finished() {
        result = 0;
        len = 0;
        captime = 0;
        ref = 0;
        inp = 0;
    }

    DWBYTE *result;
    int len;
    unsigned long captime;
    int ref;
    coder_input *inp;
};

pthread_mutex_t sampler_mutex = PTHREAD_MUTEX_INITIALIZER;


static DwPipeline<coder_input, coder_finished> *Coder_pipe;
struct CoderOp : public DwPipelineOp<coder_input, coder_finished>
{
    virtual int execute(coder_input *, coder_finished*&);
};

static
void
background_code(coder_input *ci, coder_finished *f)
{
    void *bits;
    int cols, rows;
    int fmt;
    void *y, *oblue, *ored;
#if 0
    pthread_mutex_lock(&sampler_mutex);
    bits = sampler->get_data(cols, rows, y, oblue, ored, fmt, f->captime);
    pthread_mutex_unlock(&sampler_mutex);
#endif
    cols = ci->cols;
    rows = ci->rows;
    bits = ci->bits;
    fmt = ci->fmt;
    int showonly = ci->show_only;
    y = ci->y;
    oblue = ci->cb;
    ored = ci->cr;
    f->captime = ci->captime;
    int ref = ci->ref;
    DwCoder *coder = ci->coder;

    if(showonly)
    {
        if(is_ppm(fmt))
        {
            int old_inhibit = coder->inhibit_coding;
            coder->inhibit_coding = 1;
            coder->code_preprocess(0, 0, 0, cols, rows, 1, f->len, bits);
            coder->inhibit_coding = old_inhibit;
            ppm_freearray((pixel **)bits, rows);
            f->result = 0;
        }
    }


    if(is_luma_only(fmt))
    {
        int samp = coder->get_sampling();
        coder->set_sampling(SAMP400);
        gray **g = (gray **)bits;
        int old_inhibit = coder->inhibit_coding;
        coder->inhibit_coding = showonly;

        f->result = coder->code_preprocess(g, 0, 0, cols, rows, ref, f->len);
        f->ref = ref;
        coder->inhibit_coding = old_inhibit;
        coder->set_sampling(samp);
        return;
    }
    if(is_ppm(fmt))
    {
        // note: might want to reverse 1 and 2 to avoid
        // microsoft reverse redblue display braindamage.
        //coder->set_sampling(CTUserDefaults.get_color_quality());
        if(coder->get_sampling() == SAMP400)
        {
            // overriding what acquisition is giving us.
            DWBYTE *b = 0;
            if(y)
            {
                int old_inhibit = coder->inhibit_coding;
                coder->inhibit_coding = showonly;
                b = coder->code_preprocess((gray **)y, 0, 0, cols, rows, ref, f->len);
                coder->inhibit_coding = old_inhibit;
            }
            ppm_freearray((pixel **)bits, rows);
            if(oblue) pgm_freearray((gray **)oblue, rows);
            if(ored) pgm_freearray((gray **)ored, rows);
            f->result = b;
            f->ref = ref;
            return;
        }
        DWBYTE *b = 0;
        if(y && oblue && ored)
        {
            int old_inhibit = coder->inhibit_coding;
            coder->inhibit_coding = showonly;
            b = coder->code_preprocess((gray **)y, (gray **)oblue, (gray **)ored,
                                       cols, rows, ref, f->len, bits);
            coder->inhibit_coding = old_inhibit;
        }
        ppm_freearray((pixel **)bits, rows);
        f->result = b;
        f->ref = ref;
        return;
    }
    if(is_color_yuv(fmt))
    {
        gray **g = (gray **)bits;
        gray **cb = (gray **)oblue;
        gray **cr = (gray **)ored;
        int samp = coder->get_sampling();
        if(samp == SAMP400)
        {
            pgm_freearray(cb, rows / (is_color_yuv9(fmt) ? 4 : 2));
            pgm_freearray(cr, rows / (is_color_yuv9(fmt) ? 4 : 2));
            cb = 0;
            cr = 0;
        }
        coder->set_sampling(is_color_yuv9(fmt) ? SAMPYUV9 : SAMPYUV12);
        DWBYTE *b;
        int old_inhibit = coder->inhibit_coding;
        coder->inhibit_coding = showonly;
        b = coder->code_preprocess(g, cb, cr, cols, rows, ref, f->len);
        coder->inhibit_coding = old_inhibit;
        coder->set_sampling(samp);
        f->result = b;
        f->ref = ref;
        return;
    }
    oopanic("bg wtf");
}

int
CoderOp::execute(coder_input *ci, coder_finished *&fin)
{

    fin = new coder_finished;
    background_code(ci, fin);
    fin->inp = ci;
    return 1;
}

static
void
init_coder_pipe()
{
    if(Coder_pipe)
        return;
    // WARNING: DO NOT SET THIS TO MORE THAN 1 THREAD!
    // the coding objects are not thread-safe or re-entrant.
    // this pipeline is just being used to allow the coding to
    // happen in the background so we don't block for a long
    // period of time in "service_channels"
    Coder_pipe = new DwPipeline<coder_input, coder_finished>(1, 1);
    Coder_pipe->add_operation(new CoderOp);
    Coder_pipe->init();
}

static void
kill_coder_pipe()
{
    if(!Coder_pipe)
        return;
    while(!Coder_pipe->idle())
    {
        if(Coder_pipe->next_result_ready())
        {
            coder_finished *f = Coder_pipe->get2();
            delete f->inp;
            delete f;
        }

    }
}

#endif


void
MMChannel::exit_mmchan()
{

restart:;

    auto n = MMChannels.num_elems();
    for(int i = 0; i < n; ++i)
    {
        MMChannel *m = MMChannels[i];

        if(m)
        {
            ValidPtr vp = m->vp;
            if(!vp.is_valid())
                continue;
            m->schedule_destroy(HARD);
            m->destroy();
            if(vp.is_valid())
            {
                m->stop_service();
                delete m;
            }
            goto restart;
        }
    }
    MMChannels = DwVecP<MMChannel>();

}


vc
MMChannel::codec_number_to_name(int n)
{
    switch(n)
    {
    case 0:
    default:
        return "dct";
    case 1:
        return "spaz";
    case 2:
        return "dct3";
    case 3:
        return "theora";
    }
}

int
MMChannel::codec_name_to_number(vc n)
{
    if(n == vc("dct"))
        return 0;
    else if(n == vc("spaz"))
        return 1;
    else if(n == vc("dct3"))
        return 2;
    else if(n == vc("theora"))
        return 3;
    else
        return 0;
}

MMChannel::MMChannel() :
    vp(this),
    fps_recv(1000), fps_send(1000), bps_send(1000), bps_recv(1000),
    bps_audio_recv(1000), bps_audio_send(1000), bps_file_xfer(500),
    ref_timer("ref"), frame_timer("frame"),
    audio_decoders(PAYLOAD_NUM, DWVEC_FIXED, !DWVEC_AUTO_EXPAND),
    audio_coders(PAYLOAD_NUM, DWVEC_FIXED, !DWVEC_AUTO_EXPAND),
    drop_timer("drop"),
    sync_timer("sync"),
    pinger_timer("pinger"),
    timer1("mmchan-timer1"),
    ctrl_timer("ctrl-timer"),
    keepalive_timer("keepalive"),
    nego_timer("nego"),
    //resolve_timer("resolve"),
    ctrl_send_watchdog("ctrl-send-watchdog"),
    remote_vars(),
    sync_manager(),
    //bw_limit_incoming("bw_limit_incoming", &sync_manager),
    //bw_limit_outgoing("bw_limit_outgoing", &sync_manager),
    //available_audio_decoders("available_audio_decoders", &sync_manager),
    //available_audio_coders("available_audio_coders", &sync_manager),
    pinger("pinger", &sync_manager),
    pause_audio("pause_audio", &sync_manager),
    pause_video("pause_video", &sync_manager),
    incoming_throttle("incoming_throttle", &sync_manager),

    //rem_bw_limit_incoming("bw_limit_incoming", &remote_vars),
    //rem_bw_limit_outgoing("bw_limit_outgoing", &remote_vars),
    //rem_available_audio_decoders("available_audio_decoders", &remote_vars),
    //rem_available_audio_coders("available_audio_coders", &remote_vars),
    rem_pinger("pinger", &remote_vars),
    rem_pause_audio("pause_audio", &remote_vars),
    rem_pause_video("pause_video", &remote_vars),
    rem_incoming_throttle("incoming_throttle", &remote_vars),

    //stun_socks(2, DWVEC_FIXED, !DWVEC_AUTO_EXPAND),
    granted(10, DWVEC_FIXED, !DWVEC_AUTO_EXPAND),
    ctrl_q(VC_VECTOR),
    sync_pinger("sync-pinger")

{
    tube = 0;
    sampler = 0;
    coder = 0;
    grab_coded_id = -1;
    grab_chat_id = -1;
    decoder = 0;
    do_destroy = KEEP;
    negotiating = 0;
    user_accept = NONE;
    //accept_box = 0;
    ctrl_send_watchdog.set_interval(10 * 60 * 1000);
    nego_timer.load(NEGO_TIMEOUT);
    nego_timer.set_oneshot(1);

    ref_timer.set_interval(10000);
    ref_timer.set_autoreload(1);
    //ref_timer.start();

    ready_for_ref = 1;

    // frame rate control timer
    frame_interval = 98;
    frame_timer.set_interval(frame_interval);
    frame_timer.set_autoreload(1);
    //frame_timer.start();

    frame_send = 0;

    // dropped packet control timer
    drop_timer.set_interval(PACKET_DROP_INTERVAL);
    drop_timer.set_autoreload(1);
    drop_send = 0;
    remote_percent_dropped = 0;
    percent_dropped = 0;

    // rate estimators
    rate_update = 0;
    fps_recv.start();
    fps_send.start();
    bps_send.start();
    bps_recv.start();
    bps_audio_send.start();
    bps_audio_recv.start();
    bps_file_xfer.start();

    pause = 0;

    pstate = IDLE;
    chat_state = CHAT_NO_STATE;
    myid = vp.cookie;
    if(!AllChan2)
        AllChan2 = new DwTreeKaz<MMChannel *, int>(0);
    AllChan2->add(myid, this);

    last_coded_buf = 0;
    len_last_coded_buf = 0;
    last_time_index = 0;
    last_frame_type = 0;

    cancel = 0;

    last_msg_index = 0;
    kaq = 0;
    chat_display = 0;
    chatbox_id = -1;
    chat_master = 0;
    chat_proxy = 0;

    audio_output = 0;
    audio_sampler = 0;
    grab_audio_id = -1;
    last_audio_buf = 0;
    last_audio_len = 0;
    last_audio_index = 0;
    last_acq_time = 0;
    audio_timecode = 0;
    audio_chan = -1;
    audio_state = MEDIA_ERR;
    force_unreliable_audio = 1;
    current_payload_type = -1;

    force_unreliable_video = 1;
    video_chan = -1;
    video_state = MEDIA_ERR;

    msync_chan = -1;
    msync_state = MEDIA_ERR;
    is_sync_chan = false;

    auto_quality_boost = 0;
    private_chatbox_id = -1;
    private_chat_display = 0;
    private_kaq = 0;

    read_fd = 0;
    write_fd = 0;
    write_fd_offset = 0;
    seekto = 0;
    expected_size = 0;
    total_got = 0;

    gv_id = 0;

#if 0
    // resolve and connection
    hr = 0;
    resolve_done = 0;
    resolve_len = 0;
    resolve_buf = 0;
    resolve_result = -1;
    resolve_failed = 0;
#endif
    memset(&addr_out, 0, sizeof(addr_out));

    msg_output = 0;
    call_setup = 0;

    // sync variables at this rate
    //sync_timer.set_autoreload(1);
    //sync_timer.set_interval(1000);
    //sync_timer.start();

    pinger_timer.set_autoreload(1);
    pinger_timer.set_interval(60 * 1000);
    pinger_timer.start();
    pinger = 0;

    sync_manager.snapshot();

    destroy_callback = 0;
    status_callback = 0;
    established_callback = 0;
    store_message_callback = 0;
    got_ok_callback = 0;
    timer1_callback = 0;
    ctrl_timer_callback = 0;
    chan_established_callback = 0;
    stun_established_callback = 0;
    ecb_arg2 = 0;
    dcb_arg2 = 0;
    scb_arg1 = 0;
    t1cb_arg2 = 0;
    ctcb_arg2 = 0;
    gcb_arg2 = 0;
    smcb_arg2 = 0;
    cec_arg2 = 0;
    sec_arg2 = 0;
    xfer_failed = 1;
    connect_failed = 1;

    pause_video = 0;
    pause_audio = 0;

    incoming_throttle = 0;

    msg_chan = 0;
    user_control_chan = 0;

    internal_0_offset = 0;
    make_first_0 = 0;
    client_timecode_adjustment = 0;

    vid_internal_0_offset = 0;
    vid_make_first_0 = 0;
    vid_client_timecode_adjustment = 0;

    zap_send = 0;
    zap_recv = 0;
    direct_zap_attachment_declined = 0;

    port = 0;
    server_channel = 0;
    secondary_server_channel = 0;

    // 5 minute keepalives for servers
    keepalive_timer.set_autoreload(1);
    keepalive_timer.set_interval(300 * 1000);
    keepalive_timer.start();

    codec_tweak = 0;

    force_unreliable_config = 0;
    use_stun = 0;
    sent_stun = 0;

    rejected = 0;

    video_output_device_blocked = 0;

    low_level_connect_callback = 0;
    llcc_arg2 = 0;

    proxy_outgoing_bw_limit = -1;
    call_setup_canceled_callback = 0;
    call_appearance_death_callback = 0;

    ucmcb_arg2 = 0;

    selective_chat_output = 0;
    selective_chat_mode = 0;

    step_done_callback = 0;
    step_mode = 0;
    step_frames = 0;

    session_id = gen_random_int();
    video_output_device_blocked = 0;
    prescreened = 0;
    ca_arg2 = 0;

    established_callback_called = 0;
    call_accepted_callback_called = 0;

    file_xfer = 0;
    zreject = 0;

    mms_sync_state = MMSS_NONE;
    mmr_sync_state = MMSS_NONE;
    mms_sync_state.value_changed.connect_memfun(this, &MMChannel::mms_sync_state_changed);
    mmr_sync_state.value_changed.connect_memfun(this, &MMChannel::mmr_sync_state_changed);
    package_index_future = nullptr;
    unpack_index_future = nullptr;
    eager_pull_timer_active = true;
}

MMChannel::~MMChannel()
{
    long i;
    if((i = MMChannels.index(this)) != -1)
        MMChannels[i] = 0;
    if(!AllChan2->del(myid))
        oopanic("channel not mapped");
    delete [] last_coded_buf;
    last_coded_buf = 0;
    if(read_fd)
    {
        fclose(read_fd);
        read_fd = 0;
    }
    if(write_fd)
    {
        fclose(write_fd);
        write_fd = 0;
    }
    for(i = 0; i < simple_protos.num_elems(); ++i)
    {
        delete simple_protos[i];
        simple_protos[i] = 0;
    }
    vp.invalidate();
}

MMChannel *
MMChannel::channel_by_id(int id)
{
    if(id < 0 || !AllChan2)
        return 0;

    MMChannel *ret;
    if(AllChan2->find(id, ret))
        return ret;
    return 0;
}

ChanList *
MMChannel::get_serviced_channels()
{
    ChanList *cl = new ChanList;
    int n = MMChannels.num_elems();
    for(int i = 0; i < n; ++i)
    {
        MMChannel *mc = MMChannels[i];
        if(!mc || mc->do_destroy != KEEP)
            continue;
        cl->append(mc);
    }
    return cl;
}

int
MMChannel::any_ctrl_q_pending()
{
    int n = MMChannels.num_elems();
    for(int i = 0; i < n; ++i)
    {
        MMChannel *mc = MMChannels[i];
        if(!mc)
            continue;
        // note: if there is pending destroy, we want to hurry up
        // in that case too
        if(mc->do_destroy != KEEP)
            return 1;
        if(mc->ctrl_q.num_elems() > 0)
            return 1;
    }
    return 0;
}

MMChannel *
MMChannel::get_channel_by_session_id(int sid)
{
    int n = MMChannels.num_elems();
    for(int i = 0; i < n; ++i)
    {
        MMChannel *mc = MMChannels[i];
        if(!mc || mc->do_destroy != KEEP || mc->server_channel || !mc->tube || !mc->tube->is_network())
            continue;
        if(mc->remote_session_id() == sid)
            return mc;
    }
    return 0;
}

ChanList *
MMChannel::get_serviced_channels_net()
{
    ChanList *cl = new ChanList;
    int n = MMChannels.num_elems();
    for(int i = 0; i < n; ++i)
    {
        MMChannel *mc = MMChannels[i];
        if(!mc || mc->do_destroy != KEEP || mc->server_channel || !mc->tube || !mc->tube->is_network())
            continue;
        cl->append(mc);
    }
    return cl;
}

int
MMChannel::some_serviced_channels_net()
{
    int n = MMChannels.num_elems();
    for(int i = 0; i < n; ++i)
    {
        MMChannel *mc = MMChannels[i];
        if(!mc || mc->do_destroy != KEEP || mc->server_channel || !mc->tube || !mc->tube->is_network())
            continue;
        return 1;
    }
    return 0;
}

ChanList *
MMChannel::get_serviced_channels_server()
{
    ChanList *cl = new ChanList;
    int n = MMChannels.num_elems();
    for(int i = 0; i < n; ++i)
    {
        MMChannel *mc = MMChannels[i];
        if(!mc || mc->do_destroy != KEEP)
            continue;
        if(mc->server_channel)
            cl->append(mc);
    }
    return cl;
}


DwString
MMChannel::get_string_id()
{
    return str_id;
}

void
MMChannel::set_string_id(const DwString& s)
{
    str_id = s;
    char a[200];
    sprintf(a, "-%d", myid);
    str_id += a;
}

int
MMChannel::is_coder()
{
    if(coder)
        return 1;
    return 0;
}

#if 0
int
MMChannel::is_sender()
{
    if(local_send() && tube)
        return 1;
    return 0;
}

int
MMChannel::is_receiver()
{
    if(local_recv() && tube && decoder)
        return 1;
    return 0;
}
#endif



void
MMChannel::start_service()
{
    long i;

    if(MMChannels.index(this) != -1)
        return;
    if((i = MMChannels.index(0)) == -1)
        MMChannels.append(this);
    else
        MMChannels[i] = this;
}

void
MMChannel::stop_service()
{
    long i;
    if((i = MMChannels.index(this)) == -1)
        return;
    MMChannels[i] = 0;
    // this is the point in the channel destruction
    // process where the channel become invisible to
    // the bandwidth thing, so redo that now just
    // to let the system respond to new bandwidth
    // allocation.
    adjust_outgoing_bandwidth();
    adjust_incoming_bandwidth();
}

int
MMChannel::is_serviced(ValidPtr vp)
{
    if(!vp.is_valid())
        return 0;
    return MMChannels.index((MMChannel *)(void *)vp) != -1;
}

void
MMChannel::schedule_destroy(enum destroy_how how)
{
    do_destroy = how;
    negotiating = 0;
    drop_timer.stop();
    drop_send = 0;
}

void
MMChannel::clean_video_sampler_refs(VidAcquire *v)
{
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m)
            continue;
        if(m->do_destroy == KEEP && v == m->sampler)
            m->sampler = 0;
    }
}

void
MMChannel::clean_audio_sampler_refs(AudioAcquire *v)
{
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m)
            continue;
        if(m->do_destroy == KEEP && v == m->audio_sampler)
            m->audio_sampler = 0;
    }
}

void
MMChannel::clean_keyboard_sampler_refs(KeyboardAcquire *v)
{
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m)
            continue;
        if(m->do_destroy == KEEP && v == m->kaq)
            m->kaq = 0;
    }
}

int
MMChannel::other_receivers(int MMChannel::*grab_id)
{
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m || m == this)
            continue;
        if(m->do_destroy == KEEP &&
                m->*grab_id != -1 && m->*grab_id == myid)
            return 1;
    }
    return 0;
}

int
MMChannel::other_receivers()
{
    return other_receivers(&MMChannel::grab_coded_id);
}

int
MMChannel::num_chans_with_tube(MMTube *t)
{
    int cnt = 0;
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m)
            continue;
        if(t == m->tube)
            ++cnt;
    }
    return cnt;
}

int
MMChannel::other_chat_receivers()
{
    return other_receivers(&MMChannel::grab_chat_id);
}

// this function just counts the total number of
// calls that have been set up (more or less, the number
// of control channels that are being serviced for
// network users receiving any kind of media).
// it is used by the callq to try and figure out
// how to start up new calls based on the current
// state of the system (both accepted and originated calls.)
int
MMChannel::num_calls()
{
    int cnt = 0;
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m || m->do_destroy)
            continue;
        if(!m->tube || !m->tube->is_network())
            continue;
        if(m->decoder)
            ++cnt;
        else if(m->audio_output)
            ++cnt;
        else if(m->coder)
            ++cnt;
        else if(m->grab_coded_id != -1 && channel_by_id(m->grab_coded_id))
            ++cnt;
        else if(m->grab_audio_id != -1 && channel_by_id(m->grab_audio_id))
            ++cnt;
        else if(m->private_kaq)
            ++cnt;
        else if(m->chat_master)
            ++cnt;
        else if(m->grab_chat_id != -1 && channel_by_id(m->grab_chat_id))
            ++cnt;
    }
    return cnt;
}

int
MMChannel::num_video_recvs()
{
    int cnt = 0;
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m)
            continue;
        if(m->decoder && m->tube && m->tube->is_network())
            ++cnt;
    }
    return cnt;
}

int
MMChannel::num_audio_recvs()
{
    int cnt = 0;
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m)
            continue;
        if(!m->do_destroy && m->tube && m->tube->is_network() && m->audio_output)
            ++cnt;
    }
    return cnt;
}

// this returns the number of channels actually
// feeding audio packets into the audio output thread,
// which is useful to know if you want to know when to
// shutdown the output device. the above call is
// used for call screening, which shouldn't take into
// account zaps that are playing and so on.
int
MMChannel::num_audio_output_feeders()
{
    int cnt = 0;
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m)
            continue;
        if(!m->do_destroy && m->tube && m->audio_output)
            ++cnt;
    }
    return cnt;
}

// note: the following 2 calls probably count
// zap records as "sends", which may be a problem from
// time to time. for now, we don't change it cuz i don't have
// time to test it properly.
int
MMChannel::num_video_sends()
{
    int cnt = 0;
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m)
            continue;
        if(!m->do_destroy && m->tube &&
                (m->coder || (m->grab_coded_id != -1 && channel_by_id(m->grab_coded_id))))
            ++cnt;
    }
    return cnt;

}

int
MMChannel::num_audio_sends()
{
    int cnt = 0;
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m)
            continue;
        if(!m->do_destroy && m->tube &&
                ((m->grab_audio_id != -1 && channel_by_id(m->grab_audio_id))))
            ++cnt;
    }
    return cnt;

}

int
MMChannel::num_chats_private()
{
    int cnt = 0;
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m)
            continue;
        if(!m->do_destroy && m->tube && m->private_kaq)
            ++cnt;
    }
    return cnt;

}

int
MMChannel::num_chats()
{
    int cnt = 0;
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m)
            continue;
        if(!m->do_destroy && m->tube &&
                (m->chat_master || (m->grab_chat_id != -1 && channel_by_id(m->grab_chat_id))))
            ++cnt;
    }
    return cnt;

}

void
MMChannel::synchronous_destroy(int id, enum destroy_how how)
{
    if(id < 0)
        return;
    MMChannel *mc;
    if(!(mc = channel_by_id(id)))
        return;
    mc->schedule_destroy(how);
    mc->destroy();
    mc->stop_service();
    delete mc;
}

int
MMChannel::destroy()
{
    int i;

    // need this to sync the state of some record tubes
    // because some important security related stuff happens
    // in the destroy handler.
    if(tube)
        tube->flush();

    // do a callback before anything else happens
    if(destroy_callback)
        (*destroy_callback)(this, dcb_arg1, dcb_arg2, dcb_arg3);
    destroy_signal.emit(myid);

    // make sure we clean up any pending callbacks on
    // this channel

    dirth_dead_channel_cleanup(myid);

    // note: need to get rid of accept box if that is
    // around.

    if(call_setup)
    {
        if(call_setup_canceled_callback)
            (*call_setup_canceled_callback)(this, csc_arg1, csc_arg2, csc_arg3);
    }
    if(call_appearance_death_callback)
    {
        (*call_appearance_death_callback)(this, cad_arg1, cad_arg2, cad_arg3);
    }

    for(i = 0; i < PAYLOAD_NUM; ++i)
    {
        delete audio_decoders[i];
        audio_decoders[i] = 0;
    }
    delete decoder;
    decoder = 0;
    // XXX DON'T DELETE IF OTHER REFS TO TUBE
    if(tube)
    {
        if(num_chans_with_tube(tube) == 1)
        {
            delete tube;
            tube = 0;
        }
        else
        {
            // someone else has a forked-off version
            // of this channel. just make sure we
            // shut down the control channel so the
            // other side senses the shutdown, but
            // leave the other guy intact so he can
            // finish xferring files or whatever.
            //tube->disconnect_ctrl();
            tube = 0;
            Log_make_entry("disconnect shared link to ", remote_iam(), "");
        }
    }
    if(private_kaq)
    {
        delete private_kaq;
        private_kaq = 0;
        delete private_chat_display;
        private_chat_display = 0;
        Log_make_entry("private chat shutdown");
    }

    if(do_destroy == HARD || (do_destroy == SOFT &&
                              !other_receivers() && !Soft_preview_on))
    {
#if defined(DWYCO_THREADED_ENCODE) && defined(LINUX)
        kill_coder_pipe();
#endif
        delete coder;
        coder = 0;
        if(sampler)
        {
            GRTLOG("destroy stop", 0, 0);
            sampler->stop();
            sampler = 0;
            // we know there is only one sampler
            // in this version...
            exitaq();
            Log_make_entry("sampler shutdown");
        }
    }
    grab_coded_id = -1;
    grab_audio_id = -1;
    if(do_destroy == HARD || (do_destroy == SOFT &&
                              !other_chat_receivers()))
    {
        if(kaq)
        {
            kaq = 0;
            //exit_msgaq();
        }
    }
    if(do_destroy == HARD || (do_destroy == SOFT &&
                              !other_receivers(&MMChannel::grab_audio_id)))
    {
        for(i = 0; i < PAYLOAD_NUM; ++i)
        {
            delete audio_coders[i];
            audio_coders[i] = 0;
        }
        if(audio_sampler)
        {
            exit_audio_input();
            Log_make_entry("audio sampler shutdown");
            audio_sampler = 0;
        }
        if(audio_output)
        {
            // we know there is only one...
            if(audio_output == TheAudioOutput)
            {
                exit_audio_mixer();
                exit_audio_output();
            }
            else
                delete audio_output;
            audio_output = 0;
            Log_make_entry("audio output shutdown");
        }
    }
    // check to see if there are any other audio
    // receivers alive, if not, get rid of the audio
    // output objects
    if(num_audio_output_feeders() == 0)
    {
        MMChannel *c = find_audio_player();
        if(c && c != this)
        {
            c->schedule_destroy(HARD);
        }
    }
    // check to see if there are any other audio
    // senders alive, if not, shut down the acquisition objects
    if(num_audio_sends() == 0)
    {
        MMChannel *c = find_audio_xmitter();
        if(c && c != this)
        {
            c->schedule_destroy(HARD);
        }
    }
    // if there are other clients
    // receiving the video from this
    // channel, just disassociate the
    // channel from the net, but keep
    // the sampler and coder going
    // for the benefit of others
    if(do_destroy == SOFT)
    {
        int ov = other_receivers();
        int oa = other_receivers(&MMChannel::grab_audio_id);
        int oc = other_chat_receivers();
        if(ov || oa || oc)
        {
            char a[255];
            sprintf(a, "<<%s%s%s-in>>-%d", ov ? "video " : "",
                    oa ? "audio " : "", oc ? "public-chat " : "", myid);
            set_string_id(a);
            do_destroy = KEEP;
            return 0;
        }
    }
    // XXX note there is a small leak here with "chat_display"
    // but since i'm not sure if it is referenced in multiple places
    // at the same time, i'm going to live with it for now
    return 1;
}


void
MMChannel::wait_for_config()
{
    init_config(0);
    pstate = WAIT_FOR_CONFIG;
    negotiating = 1;
    //ctrl_flush();
    nego_timer.reset();
    nego_timer.load(NEGO_TIMEOUT);
    nego_timer.start();
}

void
MMChannel::schedule_cancel()
{
    cancel = 1;
}


#define FAILNEGO(x) {negotiating = 0; FAILRET(x)}
#define WARNNEGO(x) {msgs.append(x);}

#if 0
int
MMChannel::local_media_setup()
{
    vc msgs(VC_VECTOR);
    msgs.append(vc(match_notes.c_str()));
    // we merge all the media to one control
    // point on the UI
    if(msg_chan || user_control_chan)
        goto finish;
    // in cdc32, there is a control point for
    // audio. other system treat audio
    // differently, so we don't generate a gray
    // unless we are actually going to put
    // video in it.
#ifdef CDC32
    if(local_send_audio() ||
            local_recv() || local_recv_audio())
    {
        gv_id = gen_new_grayview();
    }
#else
    if(local_recv())
    {
        gv_id = gen_new_grayview(myid);
    }
    else
    {
        gv_id = -1; // inhibits grayview generation
    }
#endif
    if(local_send())
    {
        // if needed, this will set up its own gv
        // note: in cdc32, display of the coder always
        // happens, but some system don't want that, but
        // the manuf. preview. so we inhibit it
#ifdef CDC32
        build_outgoing(1);
#else
        build_outgoing(1, 1);
#endif
    }

    if(local_send_audio())
    {
        build_outgoing_audio(1);
    }

    if(local_recv())
    {
        if(!build_incoming_video())
        {
            pstate = FAILED;
            negotiating = 0;
            return 0;
        }
    }
    if(local_recv_audio())
    {
        if(!build_incoming_audio(1))
            FAILNEGO("can't build audio output");
    }

    if(local_chat())
    {
        if(!build_outgoing_chat(1))
        {
            negotiating = 0;
            return 0;
        }
#if 0
        if(remote_chat())
        {
            // this might be useful for private channels
            //chat_display = new ChatDisplay;
#endif
            chat_display = gen_public_chat_display();
            //chat_display = new ChatCharOwl;
#if 0
        }
        else
        {
            FAILNEGO("remote user not accepting chat requests");
        }
#endif
    }
    if(local_chat_private())
    {
        if(!remote_chat_private())
        {
            WARNNEGO("remote user not accepting private chat");
            goto finish;
        }
        if(!build_outgoing_chat_private(1))
        {
            negotiating = 0;
            return 0;
        }
    }
finish:
    vc riam = remote_username();
    char b[255];
    DwString a((const char *)riam);
    a += "-";
    sprintf(b, "%d", myid);
    a += b;
    set_string_id(a.c_str());
    if(connection_list_changed_callback)
        (*connection_list_changed_callback)(0, clc_arg1, clc_arg2, clc_arg3);
    if(!msg_chan && !user_control_chan)
    {
        audio_chan = -1;
        int ret;
        vc aux_r(VC_VECTOR);
        aux_r.append("aux_r");
        if(!force_unreliable_audio)
        {
            if(!proxy_info.is_nil())
                send_ctrl(aux_r);    // this prompts the callee to setup a channel to the proxy
            if((ret = tube->gen_channel(proxy_info.is_nil() ? remote_listening_port() : (int)proxy_info[1], audio_chan)) == SSERR)
            {
                audio_chan = -1;
                force_unreliable_audio = 1;
                //FAILNEGO("can't establish reliable audio channel");
            }
            else
            {
                protos[audio_chan] = CHAN_ESTABLISH;
                audio_state = ret;
            }
        }
        video_chan = -1;
        if(!force_unreliable_video)
        {
            if(!proxy_info.is_nil())
                send_ctrl(aux_r);  // this prompts the callee to setup a channel to the proxy
            if((ret = tube->gen_channel(proxy_info.is_nil() ? remote_listening_port() : (int)proxy_info[1], video_chan)) == SSERR)
            {
                video_chan = -1;
                force_unreliable_video = 1;
                //FAILNEGO("can't establish reliable audio channel");
            }
            else
            {
                protos[video_chan] = CHAN_ESTABLISH;
                video_state = ret;
            }
        }
    }
    negotiating = 0;
    return 1;
}
#endif

int
MMChannel::local_media_setup_new()
{
    // we merge all the media to one control
    // point on the UI

    vc riam = remote_username();

    DwString a((const char *)riam);
    set_string_id(a);

    if(connection_list_changed_callback)
        (*connection_list_changed_callback)(0, clc_arg1, clc_arg2, clc_arg3);

    // note: for msg and uc chans, don't set up devices either, since
    // we aren't expecting media to come in on those things anyway.
    vc aux_r(VC_VECTOR);
    aux_r.append("aux_r");
    if(!msg_chan && !user_control_chan && call_type != vc("sync"))
    {
        // always set ourselves up to receive video
        if(!build_incoming_video())
        {
            // maybe set flags to say we can't recv video
        }
        // ... and audio
        if(!build_incoming_audio(1))
        {
            // set flags saying we can't receive audio
        }
        // use old private chat channel for app-level control msgs
        // that need to be tightly-bound to channel existence
        if(!build_outgoing_chat_private(1))
        {
        }

        audio_chan = -1;
        int ret;

        if(!force_unreliable_audio)
        {
            if(!proxy_info.is_nil())
                send_ctrl(aux_r);    // this prompts the callee to setup a channel to the proxy
            if((ret = tube->gen_channel(proxy_info.is_nil() ? remote_listening_port() : (int)proxy_info[1], audio_chan)) == SSERR)
            {
                audio_chan = -1;
                force_unreliable_audio = 1;
            }
            else
            {
                audio_state = ret;
                sproto *s = new sproto(audio_chan, media_x_setup, vp);
                s->start();
                simple_protos[audio_chan] = s;
            }
        }
        video_chan = -1;
        if(!force_unreliable_video)
        {
            if(!proxy_info.is_nil())
                send_ctrl(aux_r);  // this prompts the callee to setup a channel to the proxy
            if((ret = tube->gen_channel(proxy_info.is_nil() ? remote_listening_port() : (int)proxy_info[1], video_chan)) == SSERR)
            {
                video_chan = -1;
                force_unreliable_video = 1;
            }
            else
            {
                video_state = ret;
                sproto *s = new sproto(video_chan, media_x_setup, vp);
                s->start();
                simple_protos[video_chan] = s;
            }
        }
    }
    else if(call_type == vc("sync"))
    {
        // for now, just set up a sync channel for each connection, like media channels
        // this is ok for testing, but probably needs more thought
        if(!proxy_info.is_nil())
            send_ctrl(aux_r);  // this prompts the callee to setup a channel to the proxy
        msync_chan = -1;
        int ret;
        if((ret = tube->gen_channel(proxy_info.is_nil() ? remote_listening_port() : (int)proxy_info[1], msync_chan)) == SSERR)
        {
            msync_chan = -1;
        }
        else
        {
            msync_state = ret;
            sproto *s = new sproto(msync_chan, media_x_setup, vp);
            s->start();
            simple_protos[msync_chan] = s;
        }
    }
    negotiating = 0;
    return 1;
}

#if 0
int
MMChannel::cfg_has_duplex(vc , const char *what)
{
    // note: duplex isn't matched any more
#if 0
    vc v;
    static vc duplex("duplex matches");
    if(!common_cfg.is_nil() && common_cfg.find(duplex,  v) &&
            v.contains(what))
        return 1;
#endif
    return 0;

}

int
MMChannel::remote_recv()
{
    return cfg_has_duplex(remote_cfg, "recv");
}

int
MMChannel::local_recv()
{
    return cfg_has_duplex(config, "recv");
}

int
MMChannel::remote_send()
{
    return cfg_has_duplex(remote_cfg, "send");
}
int
MMChannel::local_send()
{
    return cfg_has_duplex(config, "send");
}

int
MMChannel::remote_chat_private()
{
    return cfg_has_duplex(remote_cfg, "pchat");
}

int
MMChannel::local_chat_private()
{
    return cfg_has_duplex(config, "pchat");
}

int
MMChannel::remote_chat()
{
    return cfg_has_duplex(remote_cfg, "chat");
}

int
MMChannel::local_chat()
{
    return cfg_has_duplex(config, "chat");
}

int
MMChannel::remote_send_audio()
{
    return cfg_has_duplex(remote_cfg, "send audio");
}

int
MMChannel::local_send_audio()
{
    return cfg_has_duplex(config, "send audio");
}

int
MMChannel::remote_recv_audio()
{
    return cfg_has_duplex(remote_cfg, "recv audio");
}

int
MMChannel::local_recv_audio()
{
    return cfg_has_duplex(config, "recv audio");
}


int
MMChannel::is_audio_receiver()
{
    if(tube && local_recv_audio())
        return 1;
    return 0;
}

int
MMChannel::is_audio_sender()
{
    if(tube && local_send_audio())
        return 1;
    return 0;
}

int
MMChannel::is_chatter()
{
    if(tube && local_chat() && remote_chat())
        return 1;
    return 0;
}

int
MMChannel::is_private_chatter()
{
    if(tube && local_chat_private() && remote_chat_private())
        return 1;
    return 0;
}
#endif

vc
MMChannel::remote_iam()
{
    vc v;
    if(!remote_cfg.is_atomic())
    {
        v = remote_username();
        if(v.is_nil())
            v = "<<unknown-username>>";
        vc id = remote_uid();
        if(id.is_nil())
            id = "<<unknown-uid>>";
        DwString a((const char *)v);
        a += " (";
        a += (const char *)to_hex(id);
        a += ")";
        return vc(a.c_str());

    }
    else
        return vc("<<bad-remote>>");
}

vc
MMChannel::remote_uid()
{
    vc v;
    if(call_setup)
        return attempt_uid;
    if(!remote_cfg.is_atomic())
        remote_cfg.find("my uid", v);
    else
        return vcnil;
    return v;
}

int
MMChannel::remote_session_id()
{
    if(call_setup)
        return 0;
    vc v(0);
    if(!remote_cfg.is_atomic())
        remote_cfg.find("session", v);
    else
        return 0;
    return (int)v;
}

vc
MMChannel::username()
{
    vc v(get_settings_value("user/username"));
    return v;
}

vc
MMChannel::remote_username()
{
    vc v;
    if(!remote_cfg.is_atomic())
        remote_cfg.find("username", v);
    else
        return vcnil;
    return v;
}

vc
MMChannel::remote_call_type()
{
    vc v;
    if(!remote_cfg.is_atomic())
    {
        if(!remote_cfg.find("call type", v))
            return vcnil;
    }
    else
        return vcnil;
    return v;
}

// return background for older software that doesn't
// have a disposition, because foreground is treated
// specially and gets higher priority.
vc
MMChannel::remote_disposition()
{
    vc v;
    if(!remote_cfg.is_atomic())
    {
        if(!remote_cfg.find("disposition", v))
            return "background";
    }
    else
        return "background";
    return v;
}

unsigned short
MMChannel::remote_listening_port()
{
    vc v;
    // note: we have to use the other clients
    // indication of what port to use, since they
    // may be advertising NAT ports on the outside
    // of their firewall.
    if(!remote_cfg.is_atomic())
        remote_cfg.find("my fw", v);
    else
        return 0;
    if(v.type() != VC_VECTOR || v[1].type() != VC_INT)
        return 0;
    vc tmp = v[1]; // secondary port
    v = tmp;
    return (int)v;
}

vc
MMChannel::remote_connection()
{
    vc v;
    if(!remote_cfg.is_atomic())
        remote_cfg.find("my connection", v);
    else
        // note: if they are old software, just pretend
        // they are "open" internet, so we don't struggle
        // trying to deal with them.
        return "open";
    return v;
}


DwCoder *
MMChannel::coder_from_config()
{
    vc v;
    vc v2;
    DwCoder *c;
    GRTLOG("CODER from config", 0, 0);
    GRTLOGVC(config);
    // note: the remote side must configure
    // itself to use our requested coder (at the moment)
    // so we don't use the matched config...
    if(!config.find("requested config", v2))
        FAILRET("no requested config?");
    if(!v2.find("codec", v))
        FAILRET("no requested coder?");

#ifdef SPAZ_CODEC
    if(v.contains("spaz"))
        c = new CDCSpazCoderColorDisplay;
    else if(v.contains("dct3"))
        c = new JPEGTCoder3ColorDisplay;
    else
#endif
#ifndef DWYCO_NO_THEORA_CODEC
        if(v.contains("theora"))
            c = new CDCTheoraCoderColor;
        else
#endif
#ifdef DWYCO_DCT_CODER
            if(v.contains("dct"))
                c = new TMSWCoderColor;

            else
#endif
                FAILRET("incompatible coding style");
    c->set_crop(1);
    //c->set_subsample(CTUserDefaults.get_subsample());
    c->set_policy("better-yet");
    c->set_sampling(SAMP422);
    //int x = 0, y = 0, w = 0, h = 0;
    //CTUserDefaults.get_crop_rect(x, y, w, h);
    //c->set_crop_window(x, y, w, h);
    c->set_pad(0);
#if 0
    c->set_special_parms(CTUserDefaults.get_mae_hold(), CTUserDefaults.get_mae_switch(),
                         CTUserDefaults.get_mad(), CTUserDefaults.get_temporal_comp(),
                         CTUserDefaults.get_noise_clip(), CTUserDefaults.get_entropy_code(),
                         CTUserDefaults.get_cpusave(), CTUserDefaults.get_motion_comp(), codec_tweak);
#endif
    return c;
}

DwDecoder *
MMChannel::decoder_from_config()
{
    vc v;
    DwDecoder *d;
    if(!remote_cfg.find("requested config", v))
    {
        FAILRET("decoder: no requested config?");
    }
    vc v2;
    if(!v.find("codec", v2))
    {
        FAILRET("decoder: no codec attributes?");
    }

#ifdef SPAZ_CODEC
    if(v2.contains("spaz"))
        d = new CDCSpazDecoderColorDisplay;
    else if(v2.contains("dct3"))
        d = new JPEGTDecoder3ColorDisplay;
    else
#endif
#ifndef DWYCO_NO_THEORA_CODEC
        if(v2.contains("theora"))
            d = new CDCTheoraDecoderColor;
        else
#endif
            if(v2.contains("dct"))
                d = new TPGMMSWDecoderColor;
            else
            {
                FAILRET("decoder: can't decode stream");
            }
    d->set_tweak(codec_tweak);
    return d;
}

void
MMChannel::init_config(int caller)
{
    config = vc(VC_MAP, "", 31);
    config.add_kv("username", get_settings_value("user/username"));
    config.add_kv("user description", get_settings_value("user/description"));
    // bumping the protocol version will make direct connections (even via
    // server) fail. this means that calling and direct messaging will fail, and
    // all messages will be sent via server (to older version).
    // version 5, ca 3/2015, simplified direct messages which will allow
    // most messages to bypass storage on the server if both users are online.
    config.add_kv("protocol version", MMCHAN_PROTOCOL_VERS);
    config.add_kv("cdc32 version", dwyco_get_version_string());
    //config.add_kv("my listening port", (int)tube->listen_port);

    vc fw = make_fw_setup();
    config.add_kv("my fw", fw);

    config.add_kv("my uid", My_UID);
    config.add_kv("my connection", My_connection);
    config.add_kv("room", Current_room);
    config.add_kv("msg_chan", msg_chan);
    config.add_kv("user_control_chan", user_control_chan);

    if(!force_unreliable_config)
    {
        config.add_kv("reliable audio", 1);
        config.add_kv("reliable video", 1);
    }

    config.add_kv("location", get_settings_value("user/location"));
    config.add_kv("call type", call_type);
    vc v(VC_VECTOR);
    v.append("dct");
    v.append("theora");
#if 0
    v.append("spaz");
    v.append("dct3");
#endif

    config.add_kv("codec capabilities", v);
    config.add_kv("notes", "(c) 1995-present, Dwyco, Inc. ");

    // note: these are capabilities for xmit/recv
    v = vc(VC_VECTOR);
    v.append("ms-gsm610");
    //v.append("lpc4800");
    //v.append("raw-8khz");
#ifdef DWYCO_USE_SPEEX
    v.append("speex");
#endif
#ifndef DWYCO_NO_VORBIS
#ifdef UWB_SAMPLING
    v.append("vorbis");
#endif
    v.append("vorbis8khz");
#endif
    config.add_kv("audio codec", v);

    v = vc(VC_VECTOR);
#ifdef __LP64__
    v.append("64-bit");
#else
    v.append("32-bit");
#endif
#ifdef _Windows
    v.append("MS-Windows");
#endif
    config.add_kv("software configuration", v);
    config.add_kv("stun", use_stun);
    // this is used to force a particular way of
    // setting up the media channels (ie, stun, proxy, etc.)
    config.add_kv("media setup", media_setup);

    config.add_kv("session", session_id);

    config.add_kv("requested config", requested_config());

    // determine the supported channel requirements for
    // caller/callee
    // for a caller, the type of call is on a per-call
    // basis, configured in the connect dialog box.
    // for a callee, the type of call is determined by
    // how the "server/call accept" dialog box is set up.
    //
    v = vc(VC_VECTOR);
    if(caller)
    {
        MMCall *mmc = MMCall::channel_to_call(myid);
        if(mmc)
        {
            if((int)get_settings_value("video_input/no_video") == 0 && mmc->send_video)
                v.append("send");
            if(mmc->recv_video)
                v.append("recv");
            if(mmc->send_audio)
                v.append("send audio");
            if(mmc->recv_audio)
                v.append("recv audio");
            if(mmc->pub_chat)
                v.append("chat");
            if(mmc->priv_chat)
                v.append("pchat");
            config.add_kv("pw", mmc->password);
        }
        // this is new: this provides a plain control
        // channel between users that can be used for anything.
        v.append("uc");

    }
    else
    {
        //if(!VidInputData.get_no_video() && CallAcceptanceData.get_max_video() > 0)
            v.append("send");
        //if(CallAcceptanceData.get_max_audio() > 0)
            v.append("send audio");
        //if(CallAcceptanceData.get_max_chat() > 0)
            v.append("chat");
        //if(CallAcceptanceData.get_max_pchat() > 0)
            v.append("pchat");
        //if(CallAcceptanceData.get_max_video_recv() > 0)
            v.append("recv");
        //if(CallAcceptanceData.get_max_audio_recv() > 0)
            v.append("recv audio");
        v.append("uc"); // see above
    }
    config.add_kv("channel duplex", v);
    config.add_kv("disposition", My_disposition);
}

void
MMChannel::set_requested_codec(int codec)
{
    // this function is used when playing back
    // recorded material, since we "negotiate" the
    // codec from the file, instead from a remote
    // client.
    vc rc;
    if(!config.find("requested config", rc))
        return;
    vc r(VC_VECTOR);
    r.append(codec_number_to_name(codec));
    rc.add_kv("codec", r);
}

void
MMChannel::set_requested_audio_codec(const char *recording_mode)
{
    vc rc;
    if(!config.find("requested config", rc))
        return;

    DwString rm("us-audio-coder-");
    rm += recording_mode;
    vc r(VC_VECTOR);
    r.append(sysattr_get_vc(rm.c_str()));
    rc.add_kv("audio codec", r);
}

int
MMChannel::get_requested_codec()
{
    // this function is  used before recording
    // in cases where we are recording while
    // a conference is in progress. since we
    // only have one coder in the system at any
    // time, we have to force the format of the
    // resulting recording to whatever is being
    // used in the conference. klugey, but
    // otherwise we would have to set up for
    // multiple coders.
    vc rc;
    if(!config.find("requested config", rc))
        return codec_name_to_number("dct");
    if(rc.contains("dct"))
        return codec_name_to_number("dct");
    if(rc.contains("dct3"))
        return codec_name_to_number("dct3");
    if(rc.contains("spaz"))
        return codec_name_to_number("spaz");
    if(rc.contains("theora"))
        return codec_name_to_number("theora");
    return codec_name_to_number("dct");
}

vc
MMChannel::requested_config()
{
    vc m(VC_MAP, "", 7);
    vc r(VC_VECTOR);
    r.append(sysattr_get_vc("us-video-coder"));
    m.add_kv("codec", r);

    r = vc(VC_VECTOR);
    r.append(sysattr_get_vc("us-audio-coder"));

    m.add_kv("audio codec", r);

    m.add_kv("channel bandwidth", get_settings_value("rate/kbits_per_sec_out"));
    m.add_kv("channel bandwidth recv", get_settings_value("rate/kbits_per_sec_in"));
    //m.add_kv("max udp bytes", RTUserDefaults.get_max_udp_bytes());
    return m;
}

// this is a simplified key derivation function that includes the
// uid's ala the NIST recommendation.
static
vc
kdf(vc agreed, int caller, vc my_uid, vc rem_uid)
{
    SHA256 sha;
    vc u;
    vc v;
    sha.Update((const byte *)(const char *)agreed, agreed.len());
    if(caller)
    {
        u = my_uid;
        v = rem_uid;
    }
    else
    {
        v = my_uid;
        u = rem_uid;
    }

    sha.Update((const byte *)(const char *)u, u.len());
    sha.Update((const byte *)(const char *)v, v.len());
    SecByteBlock h(sha.DigestSize());
    sha.Final(h);

    return vc(VC_BSTRING, (const char *)h.BytePtr(), h.SizeInBytes());

}



// this function takes a remote configuration
// and tries to find the best match between the
// argument and config. it returns a configuration
// that is the match.

vc
MMChannel::match_config(vc cfg, int caller)
{
    vc pvers;
    if(!cfg.find("protocol version", pvers))
        return vcnil;
    vc ourvers;
    if(!config.find("protocol version", ourvers))
        return vcnil;
    // if the version isn't exact, just bail now.
    // we have other channels to send messages that are less
    // sensitive to the client versions, and this helps cut down
    // on compatibility issues.
    if(pvers != ourvers)
        return vcnil;


    vc rq;
    vc codec;
    if(!cfg.find("requested config", rq) ||
            !rq.find("codec", codec))
        return vcnil;
    vc ourcodec;
    if(!config.find("codec capabilities", ourcodec))
        return vcnil;
    int n = codec.num_elems();
    vc matches(VC_VECTOR);
    int i;
    for(i = 0; i < n; ++i)
    {
        if(ourcodec.contains(codec[i]))
            matches.append(codec[i]);
    }
    // find audio codec matches
    if(!rq.find("audio codec", codec))
        return vcnil;
    if(!config.find("audio codec", ourcodec))
        return vcnil;
    n = codec.num_elems();
    vc audio_matches(VC_VECTOR);
    for(i = 0; i < n; ++i)
    {
        if(ourcodec.contains(codec[i]))
            audio_matches.append(codec[i]);
    }
    // pick minimum bandwidth and udp packet sizes.
    vc udpsz;
    vc ourudpsz;
    vc bw;
    vc ourbw;
    if(!rq.find("channel bandwidth", bw))
        return vcnil;
    if(!config.find("requested config", rq))
        return vcnil;
    if(!rq.find("channel bandwidth", ourbw))
        return vcnil;
    bw = dw_min(bw, ourbw);

    // match reliable audio capabilities
    vc relaud;
    vc our_relaud;
    if(!cfg.find("reliable audio", relaud) ||
            !config.find("reliable audio", our_relaud))
    {
        force_unreliable_audio = 1;
    }
    else
    {
        force_unreliable_audio = ((int)relaud == 0 || (int)our_relaud == 0);
    }
    vc relvid;
    vc our_relvid;
    if(!cfg.find("reliable video", relvid) ||
            !config.find("reliable video", our_relvid))
    {
        force_unreliable_video = 1;
    }
    else
    {
        force_unreliable_video = ((int)relvid == 0 || (int)our_relvid == 0);
    }


    // note: there is no asymmetric stun setup. if the caller
    // wants stun, then we have to do it, even if they are
    // accessible directly. recipients of calls never
    // set the stun flag.
    vc stun;
    vc our_stun;
    if(!cfg.find("stun", stun) ||
            !config.find("stun", our_stun))
    {
        use_stun = 0;
    }
    else
    {
        use_stun = ((int)stun == 1 || (int)our_stun == 1);
    }

    // figure out how to set up the media channels
    //

    vc mycon;	// my connection
    vc omycon;	// other my connection
    if(!cfg.find("my connection", omycon) ||
            !config.find("my connection", mycon))
    {
        // old software, only use direct tcp
        use_stun = 0;
    }
    else
    {
        // check to see if the caller or callee is forcing
        // a particular kind of call setup
        vc ms;
        vc oms;
        if(!cfg.find("media setup", oms) ||
                !config.find("media setup", ms))
        {
            // old software again, do nothing, let the
            // stun selector above be the rule
        }
        else
        {
            // the caller determines which call setup style
            // is selected
            if((caller && ms == vc("via handshake"))
                    || (!caller && oms == vc("via handshake")))
            {

                // note: the reason we select tcp proxy at this point
                // is because we *always* try direct tcp first.
                // if that fails despite one or the other
                // being marked as "open", means there is a firewall
                // issue, and we will try via proxy.

                if(!caller)
                {
                    // just swap them around, so we are guaranteed to
                    // get the same answer as the remote end
                    vc tmp = mycon;
                    mycon = omycon;
                    omycon = tmp;
                }
                if(omycon == vc("open") || omycon == vc("no-udp") ||
                        mycon == vc("nat-symmetric") ||
                        mycon == vc("no-udp") || // UDP unlikely to work
                        omycon == vc("nat-symmetric"))
                {
                    // tcp proxy
                    // might want to check to see if they are registered here,
                    // reduce the video quality to a reasonable level if not.
                    TRACK_ADD(CL_tcp_handshake, 1);
                    force_unreliable_config = 0;
                    use_stun = 0;
                    GRTLOG("remote needs TCP media proxy", 0, 0);
                    nego_media_select = "tcp via proxy";
                }
                else if(omycon == vc("nat-restricted"))
                {
                    // try udp via stun
                    TRACK_ADD(CL_udp_handshake, 1);
                    force_unreliable_config = 1;
                    use_stun = 1;
                    GRTLOG("remote can try STUN media", 0, 0);
                    nego_media_select = "udp via stun";
                }
                else
                {
                    GRTLOG("can't figure out what remote wants", 0, 0);
                    TRACK_ADD(CL_huh_handshake, 1);
                }
            }
            else
            {
                if(!caller)
                {
                    ms = oms;
                }
                // just take whatever the caller wants
                nego_media_select = ms;
            }
        }
    }

    // match the channel duplex requirements
    vc rduplex;
    vc ourduplex;
    vc match_duplex(VC_VECTOR);
    if(!cfg.find("channel duplex", rduplex))
        return vcnil;
    if(!config.find("channel duplex", ourduplex))
        return vcnil;

    GRTLOGVC("ourduplex");
    GRTLOGVC(ourduplex);
    GRTLOGVC("remote duplex");
    GRTLOGVC(rduplex);

    GRTLOGVC("match duplex");
    GRTLOGVC(match_duplex);


    vc ret = cfg.copy(); // this gets all the other stuff (notes, etc.)
    vc cc;
    vc rcc;

    ret.add_kv("matched config", matches);
    ret.add_kv("audio matches", audio_matches);
    ret.add_kv("protocol version", pvers);
    ret.add_kv("channel bandwidth", bw);
    ret.add_kv("duplex matches", match_duplex);
    GRTLOGVC("match ret");
    GRTLOGVC(ret);
    return ret;
}

// called in caller to start direct channel setup

void
MMChannel::start_crypto()
{
    vc crypto_setup;
    crypto_setup = load_crypto();
    ctrl_flush();
    send_ctrl(crypto_setup);
    pstate = WAIT_FOR_MATCH_CRYPTO;
}

void
MMChannel::wait_for_crypto()
{
    pstate = WAIT_FOR_CRYPTO;
    negotiating = 1;
    ctrl_flush();
    nego_timer.reset();
    nego_timer.load(NEGO_TIMEOUT);
    nego_timer.start();
}

vc
MMChannel::load_crypto()
{
    vc crypto_setup(VC_MAP);
    crypto_setup.add_kv("my uid", My_UID);
    if(!channel_keys.is_nil())
        oopanic("something may be going wrong");
    channel_keys = dh_gen_combined_keys();
    crypto_setup.add_kv("udh pubkeys", udh_just_publics(channel_keys));
    crypto_setup.add_kv("protocol version", MMCHAN_PROTOCOL_VERS);
    return crypto_setup;
}

int
MMChannel::crypto_agree(vc crypto, int caller)
{
    // note: our private key material is in the channel object itself, not
    // in the config.
    if(!agreed_key.is_nil())
        oopanic("should only call once");
    vc their_pubkeys;
    vc rem_uid;
    // queasy: the uid is made into filenames in a lot of places, and if
    // we don't check it properly, could cause problems
    if(!crypto.find("my uid", rem_uid) || rem_uid.type() != VC_STRING)
        return 0;

    // get a key setup
    if(!crypto.find("udh pubkeys", their_pubkeys) || their_pubkeys.type() != VC_VECTOR)
        return 0;
    // check the public key, and store it for future use.
    // unfortunately, there isn't much we can do except
    // notify the user that there is a problem, we don't really
    // want to limit functionality based on the encryption
    // not working out. also, this is fairly low-grade in the
    // trust area. we are assuming a lot about the uid presented
    // and so on. it might make some sense to include the
    // current authenticator in the handshake just to give some
    // assurance that the uid did log into the server recently.
    vc pk;
    GRTLOG("THEIR PUBKEYS", 0, 0);
    GRTLOGVC(their_pubkeys);

    if(get_pk(rem_uid, pk))
    {
        vc s = dh_static_material(their_pubkeys);
        if(pk[DH_STATIC_PUBLIC] != s[DH_STATIC_PUBLIC])
        {
            // hmmm, how to alert on this
            // for now, lets just override it since this is
            // more or less an experiment
            GRTLOG("pubkey mismatch", 0, 0);
            GRTLOGVC(rem_uid);
            GRTLOG("local cache", 0, 0);
            GRTLOGVC(pk);
            GRTLOG("remote", 0, 0);
            GRTLOGVC(s);
            pk_invalidate(rem_uid);
            put_pk(rem_uid, s, vcnil);
            TRACK_ADD(PK_static_changed, 1);
            TRACK_ADD_VC(to_hex(rem_uid), 1);
            vc pks = (pk[DH_STATIC_PUBLIC].type() == VC_STRING ? to_hex(pk[DH_STATIC_PUBLIC]) : vc("wow"));
            vc ss =  (s[DH_STATIC_PUBLIC].type() == VC_STRING ? to_hex(s[DH_STATIC_PUBLIC]) : vc("wow2"));

            TRACK_ADD_VC(pks, 1);
            TRACK_ADD_VC(ss, 1);
        }
    }
    else
    {
        // NOTE: XXX HAVE TO BE CAREFUL HERE, THIS might OBLITERATE or block
        // acquisition of
        // ALT KEYS (check and fix)
        put_pk(rem_uid, dh_static_material(their_pubkeys), vcnil);
    }

    vc agreed = udh_agree_auth(channel_keys, their_pubkeys);
    agreed = kdf(agreed, caller, My_UID, rem_uid);
    agreed = vclh_sha(agreed);
    // chop down to 128 bits, since we are hardwired to that with
    // GCM mode
    agreed_key = vc(VC_BSTRING, (const char *)agreed, 16);
    return 1;

}

// called in the CALLEE when the caller's public keys
// come in.

void
MMChannel::recv_crypto(vc their_crypto)
{
    if(their_crypto.type() != VC_MAP)
    {
        pstate = FAILED;
        return;
    }
    vc pvers;
    if(!their_crypto.find("protocol version", pvers) ||
            int(pvers) != MMCHAN_PROTOCOL_VERS)
    {
        pstate = FAILED;
        return;
    }

    vc crypto_setup = load_crypto();
    if(!crypto_agree(their_crypto, 0))
    {
        pstate = FAILED;
        return;
    }
    ctrl_flush();
    send_ctrl(crypto_setup);
    send_decrypt();
    wait_for_config();

}

// called in the caller when we receive callee's keys
void
MMChannel::recv_matching_crypto(vc their_crypto)
{
    if(their_crypto.type() != VC_MAP)
    {
        pstate = FAILED;
        return;
    }
    vc pvers;
    if(!their_crypto.find("protocol version", pvers) ||
            (int)pvers != MMCHAN_PROTOCOL_VERS)
    {
        pstate = FAILED;
        return;
    }
    if(!crypto_agree(their_crypto, 1))
    {
        pstate = FAILED;
        return;
    }
    // should have agreed key setup at this point, so start decrypting
    ctrl_flush();
    send_decrypt();
    start_handshake();
}

void
MMChannel::start_handshake()
{
    // send our configuration info out
    // over control channel to start
    // negotiation on capabilities.
    init_config(1);
    //ctrl_flush();
    send_ctrl(config);
    pstate = WAIT_FOR_MATCH;
    // another timer is handling the timeout for this side,
    // so don't bother starting nego_timer.
}

// this is called in the CALLER when we
// receive the CALLEE's config
void
MMChannel::recv_matches(vc cfgs)
{
    // receiving config back is a confirmation.
    // otherwise we get an error indicator.
    if(cfgs.type() == VC_VECTOR)
    {
        recv_confirm(cfgs);
        // don't schedule destroy here since we want to
        // give the negotiate routine time to display
        // a message.
        //mc->schedule_destroy();
        return;
    }
    else
    {
        common_cfg = match_config(cfgs, 1);
        remote_cfg = cfgs;
    }
    // ignore unknown...
    pstate = ESTABLISHED;
}


MMChannel *
MMChannel::already_connected(vc uid, int is_msg_chan, int is_user_control_chan)
{
    // check current connections for uid
    scoped_ptr<ChanList> cl(get_serviced_channels_net());
    ChanListIter cli(cl.get());
    for(; !cli.eol(); cli.forward())
    {
        MMChannel *mc = cli.getp();
        if(uid == mc->remote_uid() && mc->msg_chan == is_msg_chan &&
                mc->user_control_chan == is_user_control_chan)
        {
            return mc;
        }
    }
    return 0;
}

MMChannel *
MMChannel::channel_by_call_type(vc uid, vc call_type)
{
    // check current connections for uid
    scoped_ptr<ChanList> cl(get_serviced_channels_net());
    ChanListIter cli(cl.get());
    for(; !cli.eol(); cli.forward())
    {
        MMChannel *mc = cli.getp();
        if(mc->do_destroy == KEEP && uid == mc->remote_uid() && call_type == mc->remote_call_type())
        {
            return mc;
        }
    }
    return 0;
}

ChanList
MMChannel::channels_by_call_type(vc uid, vc call_type)
{
    // return all channels for uid of call_type
    scoped_ptr<ChanList> cl(get_serviced_channels_net());
    ChanListIter cli(cl.get());
    ChanList ret;
    for(; !cli.eol(); cli.forward())
    {
        MMChannel *mc = cli.getp();
        if(mc->do_destroy == KEEP && uid == mc->remote_uid() && (call_type == mc->remote_call_type() || call_type == mc->call_type))
        {
            ret.append(mc);
        }
    }
    return ret;
}

void
MMChannel::destroy_by_uid(vc uid)
{
    scoped_ptr<ChanList> cl(get_serviced_channels_net());
    ChanListIter cli(cl.get());

    for(; !cli.eol(); cli.forward())
    {
        MMChannel *mc = cli.getp();
        if(mc->do_destroy == KEEP && uid == mc->remote_uid())
        {
            mc->schedule_destroy(HARD);
        }
    }

}

ChanList
MMChannel::channels_by_call_type(vc call_type)
{
    // check current connections for call_type
    scoped_ptr<ChanList> cl(get_serviced_channels_net());
    ChanListIter cli(cl.get());
    ChanList ret;
    for(; !cli.eol(); cli.forward())
    {
        MMChannel *mc = cli.getp();
        if(mc->do_destroy == KEEP && call_type == mc->remote_call_type())
        {
            ret.append(mc);
        }
    }
    return ret;
}

// this is called in the CALLEE when we receive
// the CALLER's config
void
MMChannel::recv_config(vc cfg)
{
    vc riam;
    DwString wants_to_send;
    DwString wants_to_recv;
    vc v;

    int regular_screening = 1;
    // note: this is a hack to get around a
    // bug in the deserializer, seems that sometimes
    // it just returns garbage from the stream.
    // gotta check this out and fix it. for now
    // we want to avoid a panic so we just reject
    // the connection.
    if(cfg.type() != VC_MAP)
    {
        //error("bad negotiation");
        pstate = FAILED;
        return;
    }
    vc matches = match_config(cfg, 0);

    remote_cfg = cfg;
    vc uid = remote_uid();
    remote_cfg = vcnil;
    MMChannel *m = 0;
    // if they're not trying to set up a message channel
    // and we are in a conference, rebuff calls that
    // might result in multiple channels between users.
    // multiple message channels are not so annoying,
    // as they aren't really seen by the user.
    vc mcm;
    if(!(cfg.find("msg_chan", mcm) && (int)mcm == 1) &&
            !(cfg.find("user_control_chan", mcm) && (int)mcm == 1) &&
            Conf)
    {
        m = already_connected(uid);
        if(m)
        {
            if(!m->call_setup)
            {
                // remote party already fully connect, so drop
                // this connection.
                send_error("already connected");
                goto cleanup;
            }
            else
            {
                // stop the call setup to the remote party and continue
                // our connection to the remote end.
                m->schedule_cancel();
            }
        }
    }

    // for now, allow sender to see
    // lack of match, maybe in the future
    // it could adjust and try again...
    if(matches.is_nil())
    {
        //error("bad negotiations");
        pstate = FAILED;
        return;
    }
    common_cfg = matches;
    remote_cfg = cfg;

    GRTLOG("our config myid %d", myid, 0);
    GRTLOGVC(config);
    GRTLOG("remote config", 0, 0);
    GRTLOGVC(remote_cfg);
    GRTLOG("common config", 0, 0);
    GRTLOGVC(common_cfg);

    {
        // ignore calls from people on the ignore list
        vc uid;
        vc r;
        if(!remote_cfg.is_nil() && remote_cfg.find("my uid", uid))
        {
            if(uid_ignored(uid))
            {
                Log_make_entry("call rejected, on ignore list");
                send_error("busy");
                goto cleanup;
            }
            if(!call_screening_callback && (int)get_settings_value("zap/ignore") == 1 && !pal_user(uid))
            {
                Log_make_entry("call rejected, pals-only mode");
                send_error("pals-only");
                goto cleanup;
            }
        }

    }
    {
        // if the remote is requesting a msg xfer channel
        // ignore all the password and other settings and
        // just create a clean channel
        vc m;
        if(!remote_cfg.is_nil() && remote_cfg.find("msg_chan", m) &&
                (int)m == 1)
        {
            msg_chan = 1;
            cfg.del("pw");
            common_cfg.del("pw");
            remote_cfg.del("pw");
            finish_connection_new();
            store_message_callback = store_direct;
            return;
        }
    }
    // NOTE: same logic for user control channels
    {
        // if the remote is requesting a user control channel
        // ignore all the password and other settings and
        // just create a clean channel
        vc m;
        vc r;
        if(!remote_cfg.is_nil() && remote_cfg.find("user_control_chan", m) &&
                (int)m == 1)
        {
            user_control_chan = 1;
            cfg.del("pw");
            common_cfg.del("pw");
            remote_cfg.del("pw");
            finish_connection_new();
            return;
        }
    }

    {
        // if the remote is requesting a sync channel
        // make sure we are at least in a group with
        // the same hash (this is not a security check, just
        // making sure we don't accept sync links from obviously
        // wrong group)
        vc m;
        vc r;
        if(!remote_cfg.is_nil() && remote_call_type() == vc("sync"))
        {
            vc pw;
            if(!cfg.find("pw", pw))
            {
                Netlog_signal.emit(tube->mklog("event", "no sync pw"));
                send_error("sync needs pw");
                goto cleanup;
            }
            if(!Current_alternate)
            {
                Netlog_signal.emit(tube->mklog("event", "wrong sync group"));
                send_error("sync wrong group");
                goto cleanup;
            }
            vc ipw = Current_alternate->hash_key_material();
            DwString ips((const char *)ipw, ipw.len());
            ips += dwyco::Schema_version_hack;
            ipw = vc(VC_BSTRING, ips.c_str(), ips.length());
            if(pw != ipw)
            {
                send_error("sync wrong group or schema");
                Netlog_signal.emit(tube->mklog("event", "wrong sync pw"));
                GRTLOG("wrong group pw them %s us %s", (const char *)to_hex(pw), (const char *)to_hex(ipw));
                goto cleanup;
            }
            cfg.del("pw");
            common_cfg.del("pw");
            remote_cfg.del("pw");
            is_sync_chan = true;

            ChanList cl = channels_by_call_type(remote_uid(), "sync");
            if(cl.num_elems() > 1)
            {
                // we can do several things:
                // (1) destroy this connection before it has a chance to do anything more.
                // (2) destroy the existing connection.
                // (3) *someday* let multiple connections exist.
                //
                // ca 8/2024, there seem to be some problems (bugs) in the
                // syncing stuff that get triggered if we kill the existing connection.
                // this is especially true if we are aggressively trying to get
                // connections set up, as it is really likely two clients will be
                // trying to connect to each other at the same time. the bugs manifest as
                // the "delta" generation stuff getting confused, and resulting in
                // entire indexes being exchanged (this will take some intensive
                // debugging to figure out.)
                send_error("already sync connected");
                Netlog_signal.emit(tube->mklog("event", "already sync connected"));
                GRTLOG("already sync connected", 0, 0);
                goto cleanup;


#if 0

                // maybe there is a channel the process of timing out,
                // we just assume this current one will be better than
                // the previous one (we definitely don't want more than
                // one at a time.
                // NOTE: have to check that the cleanup of the old
                // channel won't interfere with setup of this one.
                // alternatively, we can just kill both of them and let
                // it reconnect, as this should be fairly rare.
                for(int i = 0; i < cl.num_elems(); ++i)
                {
                    if(cl[i] != this)
                    {
                        if(cl[i]->tube)
                        {
                            Netlog_signal.emit(cl[i]->tube->mklog("event", "dup sync"));
                        }
                        cl[i]->schedule_destroy();
                    }
                }
#endif
            }

            finish_connection_new();
            return;
        }
    }

    if(!Conf)
    {
        // note: this is the case where we are not in
        // conference mode (ie, we have left the room)
        // *but* we appear to be in the room to another user
        // that has clicked "join", we want to reject the call
        // because we are not in that conference any more.
        // note: this has a bug that will arise if there are
        // multiple rooms with the same name in different servers,
        // not likely to happen, so i'm not going to fix it now.
        // would need to check the directory we are in too...
        vc r;
        if(!remote_cfg.is_nil() && remote_cfg.find("room", r))
        {
            if(strcasecmp(r, Current_room) != 0)
            {
                send_error("called declined, user has left the room.");
                goto cleanup;
            }
        }
    }
    if(Conf)
    {
        vc r;
        if(remote_cfg.find("room", r))
        {
            if(strcasecmp(r, Current_room) != 0)
            {
                send_error("called declined, user is in another room.");
                goto cleanup;
            }
        }
        else
        {
            send_error("internal protocol error");
            goto cleanup;
        }
    }



    // make sure this call actually went to the person we wanted.
    // before, when calls were only direct, this wasn't as much of
    // a problem, since the "attempt_uid" was ignored, and you couldn't
    // really exploit the situation by fiddling with the uid, etc.
    // but now (ca 2005) we have server assisted calling, so
    // there are cases where you could enter someones uid, and a wrong
    // ip address, and the server would arrange a proxy and cause
    // your target uid to connect to it. not a huge deal because it would
    // be hard to exploit without some protocol knowledge.
    // the other problem is that if you put in the right ip address and
    // the wrong uid, the ip address might be unreachable, and the
    // server would gladly hook up the wrong uid to the proxy, and the
    // call would go to the wrong person. so we just make sure we
    // are calling the right person and drop the call if not.
    // note: we allow a nil attempt uid, in cases where you don't have
    // it, and are just interested in direct connecting only to an IP.
    if(!attempt_uid.is_nil() && attempt_uid != remote_uid())
    {
        send_error("call dropped, wrong UID on the other end.");
        goto cleanup;
    }

    if(call_screening_callback)
    {
        char dum[1];
        dum[0] = 0;
        char *error_msg = dum;
        int accept_call_style = -1;
        vc uid = remote_uid();
        vc ct = remote_call_type();
        if(ct.is_nil())
            ct = "";
        ca_arg1 = ct;
        // NOTE: need a better way of handling the "error msg" return.
        // as it is now, either the client has to use a static, or if it
        // is allocated, it ends up leaking.
        regular_screening = (*call_screening_callback)(this,
#if 0
                            local_send(),
                            local_recv(),
                            local_send_audio(),
                            local_recv_audio(),
                            local_chat(),
                            local_chat_private(),
#else
                            0,0,0,0,0,0,
#endif
                            (const char *)ct, ct.len(),
                            (const char *)uid, uid.len(),
                            &accept_call_style,
                            &error_msg);

        if(!regular_screening)
        {
            switch(accept_call_style)
            {
            case MMCHAN_US_REJECT_CALL:
                send_error(error_msg);
                goto cleanup;
            case MMCHAN_US_ACCEPT_CALL:
                prescreened = 1;
                finish_connection_new();
                return;
            case MMCHAN_US_DEFER_CALL:
                prescreened = 1;
                if(call_appeared_callback)
                    (*call_appeared_callback)(this, DwString(""), myid);
                pstate = WAIT_FOR_USER_ACCEPT;
                return;
            default:
                oopanic("bogus user-defined call screening return");
                // NOTREACHED
            }
        }
    }
    // if we get here, we are doing regular screening. for compat, this
    // test is also done previously. might be able to eliminate that one
    // eventually.
    //
    {
        vc uid;
        if(!remote_cfg.is_nil() && remote_cfg.find("my uid", uid))
        {
            if((int)get_settings_value("zap/ignore") == 1 && !pal_user(uid))
            {
                Log_make_entry("call rejected, pals-only mode");
                send_error("pals-only");
                goto cleanup;
            }
        }
    }

    // move password screening down here so that the app defined screening
    // more or less disabled password screening. may need to update app screening
    // to include stuff for doing password screening in the future.
    if(!Conf && (int)get_settings_value("call_acceptance/require_pw") == 1)
    {
        vc pw;
        if(!cfg.find("pw", pw))
        {
            Log_make_entry("call rejected, password required");
            send_error("password required to connect");
            goto cleanup;
        }
        if(pw != get_settings_value("call_acceptance/pw"))
        {
            Log_make_entry("call rejected, incorrect pw.");
            send_error("incorrect password");
            goto cleanup;
        }
    }

    // nuke the remote password out of the config
    // so it doesn't get into the displays
    // XXX note: must fix log files before too long,
    // cuz they will have passwords in them...
    cfg.del("pw");
    common_cfg.del("pw");
    remote_cfg.del("pw");

#if 0
    if(local_recv())
    {
        if(num_video_recvs() >= CallAcceptanceData.get_max_video_recv())
        {
            Log_make_entry("video rejected from ", remote_iam(), " (busy)");
            send_error("video busy");
            goto cleanup;
        }
        wants_to_send += "video ";
    }
    if(local_recv_audio())
    {
        // somebody want to send us audio
        // note: this needs to be setup proper with
        // a proxy for playing back audio from
        // several sources (a mixer...)
        if(num_audio_recvs() >= CallAcceptanceData.get_max_audio_recv())
        {
            send_error("audio busy");
            Log_make_entry("audio rejected from ", remote_iam(), " (busy)");
            goto cleanup;
        }
        wants_to_send += "audio ";
    }
    if(local_send())
    {
        // we are acting as a server
        if(num_video_sends() >= CallAcceptanceData.get_max_video())
        {
            send_error("video busy");
            Log_make_entry("video send req rejected from ", remote_iam(), " (busy)");
            goto cleanup;
        }
        wants_to_recv += "video ";
    }
    if(local_send_audio())
    {
        // we are a server for audio
        if(num_audio_sends() >= CallAcceptanceData.get_max_audio())
        {
            send_error("audio busy");
            Log_make_entry("audio send req rejected from ", remote_iam(), " (busy)");
            goto cleanup;
        }
        wants_to_recv += "audio ";
    }
    if(local_chat())
    {
        if(num_chats() >= CallAcceptanceData.get_max_chat())
        {
            send_error("chat busy");
            Log_make_entry("chat req rejected from ", remote_iam(), " (busy)");
            goto cleanup;
        }
        wants_to_recv += "chat ";
    }
    if(local_chat_private())
    {
        if(num_chats_private() >= CallAcceptanceData.get_max_pchat())
        {
            send_error("private chat busy");
            Log_make_entry("private chat req rejected from ", remote_iam(), " (busy)");
            goto cleanup;
        }
        wants_to_recv += "private-chat";
    }
#endif

    // now pop up a dialog and wait for the user
    // to accept or reject the call.
    //play_call_alert();
    if((int)get_settings_value("call_acceptance/auto_accept") == 0)
    {
        vc name;
        if(!remote_cfg.is_nil() && !remote_cfg.find("username", name))
            name = "?";
        DwString note;
        note += (const char *)name;

        note += " wants to send you (";
        note += wants_to_send;
        note += ") and wants to recv (";
        note += wants_to_recv;
        note += ") from you.";
        if(call_appeared_callback)
            (*call_appeared_callback)(this, note, myid);
        pstate = WAIT_FOR_USER_ACCEPT;
        return;
    }
    finish_connection_new();
    return;

cleanup:
    pstate = WAIT_FOR_CLOSE;
    negotiating = 0;
    // we're through monkeying with devices, so allow new connects.
    turn_accept_on();
    // note: leave nego_timer going in this case because
    // we may not get a disconnect is odd cases.
}

#if 0
void
MMChannel::finish_connection()
{
    vc riam;
    if(msg_chan || user_control_chan)
        goto done;
    // ok, do the setup for real (with device allocation)
    if(local_recv())
    {
        if(!prescreened && num_video_recvs() >= CallAcceptanceData.get_max_video_recv())
        {
            Log_make_entry("video rejected from ", remote_iam(), " (busy)");
            send_error("video busy");
            goto cleanup;
        }
        // somebody is barging in on us.
        if(!gv_id)
            gv_id = gen_new_grayview(myid);
        if(!build_incoming_video())
        {
            send_error("can't create decoder (either not enough memory or incompatible)");
            goto cleanup;
        }
        Log_make_entry("video coming from ", remote_iam(), "");

    }
    // must set up outgoing audio first to audio sampler
    // is setup prior to mixer setup (hack).
    if(local_send_audio())
    {
        // we are a server for audio
        if(!prescreened && num_audio_sends() >= CallAcceptanceData.get_max_audio())
        {
            send_error("audio busy");
            Log_make_entry("audio send req rejected from ", remote_iam(), " (busy)");
            goto cleanup;
        }
#ifdef CDC32
        if(!gv_id)
            gv_id = gen_new_grayview();
#else
        // see comments in local_media_setup for details on this
        if(!gv_id)
            gv_id = -1;
#endif
        if(!build_outgoing_audio(0))
        {
            send_error(fail_reason);
            goto cleanup;
        }
        Log_make_entry("serving audio to ", remote_iam(), "");
    }
    if(local_recv_audio())
    {
        // somebody want to send us audio
        // note: this needs to be setup proper with
        // a proxy for playing back audio from
        // several sources (a mixer...)
        if(prescreened || num_audio_recvs() < CallAcceptanceData.get_max_audio_recv())
        {
#ifdef CDC32
            if(!gv_id)
                gv_id = gen_new_grayview();
#else
            if(!gv_id)
                gv_id = -1;
#endif
            build_incoming_audio(0);
            Log_make_entry("audio coming from ", remote_iam(), "");
        }
        else
        {
            send_error("audio busy");
            Log_make_entry("audio rejected from ", remote_iam(), " (busy)");
            goto cleanup;
        }
    }
    if(local_send())
    {
        // we are acting as a server
        if(!prescreened && num_video_sends() >= CallAcceptanceData.get_max_video())
        {
            send_error("video busy");
            Log_make_entry("video send req rejected from ", remote_iam(), " (busy)");
            goto cleanup;
        }
#ifdef CDC32
        if(!gv_id)
            gv_id = gen_new_grayview();
        if(!build_outgoing())
#else
        if(!gv_id)
            gv_id = -1;
        if(!build_outgoing(0, 1))
#endif
        {
            send_error(fail_reason);
            goto cleanup;
        }
        Log_make_entry("serving video to ", remote_iam(), "");
    }

    if(local_chat())
    {
        if(!prescreened && num_chats() >= CallAcceptanceData.get_max_chat())
        {
            send_error("chat busy");
            Log_make_entry("chat req rejected from ", remote_iam(), " (busy)");
            goto cleanup;
        }
        if(!build_outgoing_chat(0))
        {
            send_error(fail_reason);
            goto cleanup;
        }
        chat_display = gen_public_chat_display();
        //chat_display = new ChatCharOwl;
        Log_make_entry("remote chat from ", remote_iam(), " accepted");
    }
    if(local_chat_private())
    {
        if(!prescreened && num_chats_private() >= CallAcceptanceData.get_max_pchat())
        {
            send_error("private chat busy");
            Log_make_entry("private chat req rejected from ", remote_iam(), " (busy)");
            goto cleanup;
        }
        if(!build_outgoing_chat_private(0))
        {
            send_error(fail_reason);
            goto cleanup;
        }
        Log_make_entry("remote private chat from ", remote_iam(), " accepted");
    }
    //vc v(VC_VECTOR);
    //v[0] = matches;
    //v[1] = config;
    // send back both common and full remote config
    // note: we only want to callback on
    // non-message channels, since msg channels
    // are not really "calls"
    //
    // NOTE: ca 7/2013: when to call the call_accepted callback is
    // a bit odd. at first, it seemed like a bug to call it *after*
    // performing things like the video_init and chat_init callbacks.
    // but after some thought, i think doing it here is better... because
    // at this point, you know the call can complete and there hasn't been
    // some device allocation problem. in the event that you need a callback
    // after call screening has completed and the call passes call screening,
    // but before final device acquisition, that can be added above. for
    // now, i'm not going to change the ordering.
    // part of the problem is that there isn't a lot of context available
    // in the video_display_init callback (like a uid and other things), so
    // figuring out where to put the ui_id may be difficult during call setup.
    //
    if(call_accepted_callback)
        (*call_accepted_callback)(this, ca_arg1, ca_arg2, ca_arg3);
done:
    send_ctrl(config);
    if(use_stun)
        pstate = WAIT_FOR_STUN;
    else
        pstate = ESTABLISHED;

    riam = remote_username();
    {
        DwString a((const char *)riam);
        a += "-";
        vc mid(myid);
        a += mid.peek_str();
        set_string_id(a.c_str());
        if(connection_list_changed_callback)
            (*connection_list_changed_callback)(0, clc_arg1, clc_arg2, clc_arg3);
        negotiating = 0;
        turn_listen_on();
        nego_timer.reset();
    }
    return;
cleanup:
    pstate = WAIT_FOR_CLOSE;
    negotiating = 0;
    // we're through monkeying with devices, so allow new connects.
    turn_listen_on();
    // note: leave nego_timer going in this case because
    // we may not get a disconnect is odd cases.
}
#endif

void
MMChannel::finish_connection_new()
{
    vc riam;
    bool media_channel = !(msg_chan || user_control_chan || is_sync_chan);
    if(media_channel)
    {
        if(call_accepted_callback)
            (*call_accepted_callback)(this, ca_arg1, ca_arg2, ca_arg3);
        call_accepted_callback_called = 1;
    }

    send_ctrl(config);
    if(use_stun)
        pstate = WAIT_FOR_STUN;
    else
        pstate = ESTABLISHED;

    if(media_channel)
    {
        // always set ourselves up to receive video
        if(!build_incoming_video())
        {
            // maybe set flags to say we can't recv video
        }
        // ... and audio
        if(!build_incoming_audio(0))
        {
            // set flags saying we can't receive audio
        }
        if(!build_outgoing_chat_private(0))
        {
            // error, prolly need to bail
        }
    }

    riam = remote_username();

    DwString a((const char *)riam);

    set_string_id(a);

    if(connection_list_changed_callback)
        (*connection_list_changed_callback)(0, clc_arg1, clc_arg2, clc_arg3);
    negotiating = 0;
    turn_accept_on();
    nego_timer.reset();

    return;
cleanup:
    pstate = WAIT_FOR_CLOSE;
    negotiating = 0;
    turn_accept_on();
    // note: leave nego_timer going in this case because
    // we may not get a disconnect is odd cases.
}

void
MMChannel::reject_call()
{
    send_error("busy");
    pstate = WAIT_FOR_CLOSE;
    negotiating = 0;
    // we're through monkeying with devices, so allow new connects.
    turn_accept_on();
    // note: leave nego_timer going in this case because
    // we may not get a disconnect is odd cases.
}

void
MMChannel::recv_confirm(vc v)
{
    static vc ok("ok");
    static vc nok("nok");

    // format is a vector with a string followed by
    // possibly another string giving the reason for
    // failure.

    vc str = v[0];
    if(str == ok)
    {
        pstate = ESTABLISHED;
    }
    else if(str == nok)
    {
        fail_reason = v[1];
        pstate = FAILED;
        rejected = 1;
    }
    else
    {
        // ignore unknown messages
        pstate = ESTABLISHED;
    }
}

int
MMChannel::compute_sync()
{
    // calculate whether we should sync
    // receivers. if we have more than one
    // outgoing video tube, then don't sync.
    // this includes tubes for recording zaps
    // and stuff, so they can run at full speed
    // even tho you are on a modem.
    // also, if there is anyone in step mode,
    // make sure we don't sync so the step
    // can be completed at the next frame
    // captured.
    int n = MMChannels.num_elems();
    int k = 0;
    for(int i = 0; i < n; ++i)
    {
        MMChannel *mc = MMChannels[i];
        if(!mc || mc->do_destroy != KEEP || mc->server_channel || !mc->tube ||
                mc->grab_coded_id == -1)
            continue;
        // if we have a tube using stun (which means unreliable
        // transport), make it not sync, because it looks better
        // in most cases.
        if(mc->use_stun)
            return 0;
        if(mc->step_mode)
            return 0;
        if(k == 1)
            return 0;
        ++k;
    }
    return 1;
}

int
MMChannel::send_ref()
{
    if(Auto_sync)
    {
        if(compute_sync() == 0)
        {
            GRTLOG("compute sync ref", 0, 0);
            return 1;
        }
    }
    else if(!Sync_receivers)
    {
        GRTLOG("send_ref sync_receivers ref", 0, 0);
        return 1;
    }
    if(ready_for_ref)
    {
        ready_for_ref = 0;
        GRTLOG("synced send ref", 0, 0);
        return 1;
    }
    GRTLOG("no ref", 0, 0);
    return 0;
}

MMChannel *
MMChannel::find_audio_player(int any)
{
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m) continue;
        if(m->audio_output &&
                m->audio_output == TheAudioOutput && (any || !m->do_destroy))
            return m;
    }
    return 0;
}

MMChannel *
MMChannel::find_audio_xmitter(int any)
{
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m) continue;
        if(any || !m->do_destroy)
        {
            if(m->audio_sampler)
                return m;
        }
    }
    return 0;
}

MMChannel *
MMChannel::find_xmitter(int return_dying)
{
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m) continue;
        if(m->coder && (return_dying || !m->do_destroy))
            return m;
    }
    return 0;
}

MMChannel *
MMChannel::find_msg_xmitter()
{
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m) continue;
        if(m->kaq && !m->do_destroy)
            return m;
    }
    return 0;
}

MMChannel *
MMChannel::find_chat_master()
{
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m) continue;
        if(m->chat_master && !m->do_destroy)
            return m;
    }
    return 0;
}

int
MMChannel::all_outgoing_paused()
{
    if(Soft_preview_on)
        return 0;
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m) continue;
        if(!m->do_destroy &&
                !m->pause && m->tube &&
                (m->coder || (m->grab_coded_id != -1 && channel_by_id(m->grab_coded_id))))
            return 0;
    }
    return 1;

}
// this function returns 1 when a new
// frame should be acquired, coded, and
// made available to send to receivers.
// If receivers are being synced, this
// function only returns 1 when the
// last coded video has been transmitted
// to all receivers.
//
int
MMChannel::send_frame()
{
    if(frame_send)
    {
        int sr;
        if(Auto_sync)
        {
            sr = compute_sync();
        }
        else
            sr = Sync_receivers;
        if(sr)
        {
            int n = MMChannels.num_elems();
            MMChannel *xmitter = find_xmitter();
            if(xmitter == 0)
            {
                frame_send = 0;
                return 0;
            }
            int xmit_time_index = xmitter->last_time_index;
            for(int i = 0; i < n; ++i)
            {
                MMChannel *m = MMChannels[i];
                if(!m) continue;
                if(!m->coder && m->tube && m->grab_coded_id != -1 &&
                        m->grab_coded_id == xmitter->myid && !m->pause)
                {
                    if((!m->force_unreliable_video && m->video_output_device_blocked) ||
                            (m->force_unreliable_video && (!m->tube->buf_drained() || !m->tube->can_write_mmdata()))
                            || xmit_time_index != m->last_time_index) // hasn't xmitted yet.
                    {
                        // don't reset frame_send, cuz
                        // we want to come in here until all
                        // video is transmitted.
                        return 0;
                    }
                }
            }
            GRTLOG("sync send_frame", 0, 0);
        }
        else
        {
            // sync to highest rate by checking to see
            // if everyone is blocked on output, and returning
            // 0 in that case, dropping any pending frames in
            // the acq queue.
            int outs = 0;
            int ready = 0;
            for(int i = 0; i < MMChannels.num_elems(); ++i)
            {
                MMChannel *m = MMChannels[i];
                if(!m) continue;
                if(!m->do_destroy && m->tube &&
                        (m->grab_coded_id != -1 && channel_by_id(m->grab_coded_id)))
                {
                    ++outs;
                    if(m->force_unreliable_video && m->tube->buf_drained() && m->tube->can_write_mmdata())
                        // hmmm, maybe this was a fix from the past, have to check it out.
                        //if(m->tube->buf_drained())
                    {
                        GRTLOG("drained %d cw %d", m->tube->buf_drained(), m->tube->can_write_mmdata());
                        GRTLOG("no sync send_frame drained %p", m->tube, 0);
                        ready = 1;
                        break;
                    }
                    else if(!m->force_unreliable_video &&!m->video_output_device_blocked)
                    {
                        GRTLOG("(bogus)reliable drained %d cw %d", m->tube->buf_drained(), m->tube->can_write_data(VIDEO_CHANNEL));
                        ready = 1;
                        break;
                    }
                }
            }
            if(!ready)
                return 0;
        }
        frame_send = 0;
        GRTLOG("send frame 1", 0, 0);
        return 1;
    }
    return 0;
}

int
MMChannel::all_msg_broadcast_done()
{
    int n = MMChannels.num_elems();
    MMChannel *xmitter = find_msg_xmitter();
    if(xmitter == 0)
    {
        return 1;
    }
    int xmit_time_index = xmitter->last_msg_index;
    for(int i = 0; i < n; ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m) continue;
        if(xmitter->selective_chat_mode && !m->selective_chat_output)
            continue;
        if(!m->kaq && m->tube && m->grab_chat_id != -1 &&
                m->grab_chat_id == xmitter->myid)
        {
            if(xmit_time_index != m->last_msg_index) // hasn't xmitted yet.
                return 0;
        }
    }
    return 1;
}
#ifdef DWYCO_RATE_DISPLAY
int
MMChannel::rate_updated(int who)
{
    if(rate_update & who)
    {
        return 1;
    }
    return 0;
}
#endif
DwString
MMChannel::info_format(int who)
{
    char a[4096];
    int fps = 0;
    long bps = 0;

    // remove refs to "name" to avoid security problems.
    // note: this needs to be redone, passing this buffer
    // all over hell's half-acre is a cluster
    if(who & SENDER)
    {
        fps = fps_send.get_rate();
        bps = bps_send.get_rate();
        GRTLOG("send %dfps %dkbps", fps, bps);
    }
    else if(who & RECEIVER)
    {
        fps = fps_recv.get_rate();
        bps = bps_recv.get_rate();
    }
    else if(who & AUDIO_RATE)
    {
        long bpsr = bps_audio_recv.get_rate();
        long bpss = bps_audio_send.get_rate();
        if(force_unreliable_audio)
            sprintf(a, "r:%02ld(%02d%%) s:%02ld(%02d%%) kbps",
                    bpsr / 1000, percent_dropped, bpss / 1000, remote_percent_dropped);
        else
            sprintf(a, "r:%02ld s:%02ld kbps",
                    bpsr / 1000, bpss / 1000);
        return a;
    }

    if(force_unreliable_video)
    {
        sprintf(a, "%s %03dfps %04ldkbps (%02d%%)",
                // tube ? tube->get_est_baud() / 1000 : -1,
                auto_quality_boost ? "*" : "",
                //(const char *)name, //chop,
                fps, bps / 1000, (who & RECEIVER) ? percent_dropped :
                ((who & SENDER) ? remote_percent_dropped : 0));
    }
    else
    {
        sprintf(a, "%s %03dfps %04ldkbps",
                auto_quality_boost ? "*" : "",
                //(const char *)name,
                fps, bps / 1000);
    }
    return a;
}


int
MMChannel::tick()
{
    if(nego_timer.is_expired())
    {
        nego_timer.ack_expire();
        schedule_destroy();
        GRTLOG("nego timed out on chan %d", myid, 0);
        return 0;
    }

    if(tube)
    {
        if(!tube->tick())
        {
            schedule_destroy();
            return 0;
        }
    }

    if(pstate == ESTABLISHED && !server_channel)
    {
        if(sync_manager.update_available() && !sync_timer.is_running())
        {
            sync_timer.set_interval(5 * 1000);
            sync_timer.set_autoreload(0);
            sync_timer.start();
        }
        if(sync_timer.is_expired())
        {
            sync_timer.ack_expire();
            // note: this resets update_available
            sync_send();

        }
        // this is a hack to check to see if the other side is still there
        // in cases where they drop off without cleanly closing.
        // we know the ping-rate will be the same on both sides, so that
        // if the remote and local ping values diverge too much, there is something
        // wrong.
        int p1 = (int)pinger;
        int p2 = (int)rem_pinger;
        if(abs(p1 - p2) > 3)
        {
            if(tube)
            {
                Netlog_signal.emit(tube->mklog("event", "timeout pinger"));
            }
            schedule_destroy();
        }
    }

    if(pinger_timer.is_expired())
    {
        pinger_timer.ack_expire();
        pinger = (int)pinger + 1;
    }

    if(keepalive_timer.is_expired())
    {
        keepalive_timer.ack_expire();
        keepalive_processing();
    }

    if(ref_timer.is_expired())
    {
        ref_timer.ack_expire();
        ready_for_ref = 1;
    }

    if(frame_timer.is_expired())
    {
        frame_timer.ack_expire();
        frame_send = 1;
        //GRTLOG("frame send = 1", 0, 0);
    }

    if(drop_timer.is_expired())
    {
        drop_timer.ack_expire();
        drop_send = 1;
    }
    if(fps_send.is_expired())
        rate_update |= FPS_SEND;
    if(fps_recv.is_expired())
        rate_update |= FPS_RECV;
    if(bps_send.is_expired())
        rate_update |= BPS_SEND;
    if(bps_recv.is_expired())
        rate_update |= BPS_RECV;
    if(bps_audio_recv.is_expired())
        rate_update |= BPS_AUDIO_RECV;
    if(bps_audio_send.is_expired())
        rate_update |= BPS_AUDIO_SEND;

    if(audio_output && audio_output != TheAudioOutput)
        audio_output->tick();

#ifdef DW_RTLOG
    if(RTLog)
    {
        RTLog->tick();
    }
#endif
    return 1;
}

void
MMChannel::turn_off_color()
{
    for(int i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel *m = MMChannels[i];
        if(!m) continue;
        if(!m->do_destroy)
        {
            if(m->coder)
                m->coder->set_sampling(SAMP400);
#if 0
            if(m->decoder)
            {
                m->decoder->decode_color = 0;
                m->decoder->force_gray = 1;
            }
#endif
        }
    }

}

int
MMChannel::some_coder_active()
{
    return find_xmitter() != 0;
}



DWBYTE *
MMChannel::grab_and_code(int& len, unsigned long& captime, int& ref, int showonly)
{
#if defined(DWYCO_THREADED_ENCODE) && defined(LINUX)
//    if(showonly)
//        return real_grab_and_code(len, captime, ref, showonly);

    // start a background frame code
    init_coder_pipe();
    coder_input *ci = new coder_input;
    ci->coder = coder;
    // WARNING: coding the video frames out of order only makes
    // sense if they don't depend on each other, ie, they are all
    // reference frames. this mean the bit rate is going to be
    // increased (a lot).
    // HOWEVER, since we are only using this pipeline to avoid blocking
    // the UI thread, this is ok. it just puts the coding in the background
    // thread, but the results are still being produced in order.
    //
    // as a side note: if you are willing to buffer up some frames, you
    // *might* be able to overlap some of the coding by grouping the
    // frames into IPPP groups or something. you would still need
    // separate coding objects tho unless you went to the trouble to make
    // them independent.
    ci->ref = send_ref();
    ci->bits = sampler->get_data(ci->cols, ci->rows, ci->y, ci->cb, ci->cr, ci->fmt, ci->captime);
    ci->show_only = showonly;
    Coder_pipe->put(ci);
    return 0;
#else
    return real_grab_and_code(len, captime, ref, showonly);
#endif
}


DWBYTE *
MMChannel::real_grab_and_code(int& len, unsigned long& captime, int& ref, int showonly)
{
    void *bits;
    int cols, rows;
    int fmt;
    void *y, *oblue, *ored;
    if(showonly)
    {
        bits = sampler->get_data(cols, rows, y, oblue, ored, fmt, captime, 1);
        if(is_ppm(fmt))
        {
            int old_inhibit = coder->inhibit_coding;
            coder->inhibit_coding = 1;
            coder->code_preprocess(0, 0, 0, cols, rows, 1, len, bits);
            coder->inhibit_coding = old_inhibit;
            ppm_freearray((pixel **)bits, rows);
            return 0;
        }
    }
    else
        bits = sampler->get_data(cols, rows, y, oblue, ored, fmt, captime);
    if(is_luma_only(fmt))
    {
        int samp = coder->get_sampling();
        coder->set_sampling(SAMP400);
        gray **g = (gray **)bits;
        int old_inhibit = coder->inhibit_coding;
        coder->inhibit_coding = showonly;
        DWBYTE *b = coder->code_preprocess(g, 0, 0, cols, rows, (ref = send_ref()), len);
        coder->inhibit_coding = old_inhibit;
        coder->set_sampling(samp);
        return b;
    }
    if(is_ppm(fmt))
    {
        // note: might want to reverse 1 and 2 to avoid
        // microsoft reverse redblue display braindamage.
        //coder->set_sampling(CTUserDefaults.get_color_quality());
        if(coder->get_sampling() == SAMP400)
        {
            // overriding what acquisition is giving us.
            DWBYTE *b = 0;
            if(y)
            {
                int old_inhibit = coder->inhibit_coding;
                coder->inhibit_coding = showonly;
                b = coder->code_preprocess((gray **)y, 0, 0, cols, rows, (ref = send_ref()), len);
                coder->inhibit_coding = old_inhibit;
            }
            ppm_freearray((pixel **)bits, rows);
            if(oblue) pgm_freearray((gray **)oblue, rows);
            if(ored) pgm_freearray((gray **)ored, rows);
            return b;
        }
        DWBYTE *b = 0;
        if(y && oblue && ored)
        {
            int old_inhibit = coder->inhibit_coding;
            coder->inhibit_coding = showonly;
            b = coder->code_preprocess((gray **)y, (gray **)oblue, (gray **)ored,
                                       cols, rows, (ref = send_ref()), len, bits);
            coder->inhibit_coding = old_inhibit;
        }
        ppm_freearray((pixel **)bits, rows);
        return b;
    }
    if(is_color_yuv(fmt))
    {
        gray **g = (gray **)bits;
        gray **cb = (gray **)oblue;
        gray **cr = (gray **)ored;
        int samp = coder->get_sampling();
        if(samp == SAMP400)
        {
            pgm_freearray(cb, rows / (is_color_yuv9(fmt) ? 4 : 2));
            pgm_freearray(cr, rows / (is_color_yuv9(fmt) ? 4 : 2));
            cb = 0;
            cr = 0;
        }
        coder->set_sampling(is_color_yuv9(fmt) ? SAMPYUV9 : SAMPYUV12);
        DWBYTE *b;
        int old_inhibit = coder->inhibit_coding;
        coder->inhibit_coding = showonly;
        b = coder->code_preprocess(g, cb, cr, cols, rows, (ref = send_ref()), len);
        coder->inhibit_coding = old_inhibit;
        coder->set_sampling(samp);
        return b;
    }
    return 0;
}


int
inet_sim()
{
    return 0;

    static int run;
#ifdef DWTESTPANEL
    if(run)
    {
        --run;
        return 1;
    }

    if(random(100) < TestPanelData.get_percent_drop())
    {
        run = TestPanelData.get_run_length();
        return 1;
    }
#endif
    return 0;
}

int
MMChannel::service_channels(int *spin_out)
{

    int i;
    int ret;
    int not_idle = 0;
    static int lock;
    if(lock)
        return 1;
    lock = 1;
    static int been_here;
    if(!been_here)
    {
        // note: this used to be our only time we adjusted the
        // bandwidth, polling all network items once every
        // 3 seconds and adjusting it based on what channels
        // were setup, rather than what channels were actually
        // doing. this is ok on desktop, but for mobile it was
        // a lot of wakeups for mostly no reason. i keep this
        // here just as a stopgap in case i screw up getting the
        // adjustment calls done at the right time... at least we'll
        // get an adjustment once a minute.
        Bw_adj_timer.set_autoreload(1);
        Bw_adj_timer.set_interval(60 * 1000);
        Bw_adj_timer.start();
        been_here = 1;
    }
    poll_listener();
    if(!TheAudioOutput && !TheAudioInput)
    {
        // this is annoying in a multichat if all the
        // calls go away and you had audio muted, and it
        // flips back on the next time a call is setup.
        // this quick hack didn't work, and caused an eventual
        // gdi failure, have to look into it more.
        reset_audio_menus();
        if(!Has_audio_input && !Has_audio_output)
            set_audio_status(AUDIO_NO_HW);
        else
            set_audio_status(AUDIO_NONE);
    }

    if(Bw_adj_timer.is_expired())
    {
        Bw_adj_timer.ack_expire();
        adjust_outgoing_bandwidth();
        adjust_incoming_bandwidth();
    }
    if(some_serviced_channels_net())
    {
        if(!Bw_adj_timer.is_running())
            Bw_adj_timer.start();
    }
    else
    {
        if(Bw_adj_timer.is_running())
            Bw_adj_timer.stop();
    }
    callq_tick();
    int dont_restart_listening = 0;

    int spin = 0;
    for(i = 0; i < MMChannels.num_elems(); ++i)
    {
        MMChannel * const mc = MMChannels[i];
        if(!mc)
            continue;


        // put this here so we get a timeout before
        // anything else happens. when it was in "tick"
        // before, the timer wouldn't get run if we
        // were in a special state.
        //mc->timer1.tick();
        if(mc->timer1.is_expired())
        {
            mc->timer1.ack_expire();
            if(mc->timer1_callback)
                (*mc->timer1_callback)(mc, mc->t1cb_arg1, mc->t1cb_arg2, mc->t1cb_arg3);
        }

        //mc->ctrl_timer.tick();
        if(mc->ctrl_timer.is_expired())
        {
            mc->ctrl_timer.ack_expire();
            if(mc->ctrl_timer_callback)
                (*mc->ctrl_timer_callback)(mc, mc->ctcb_arg1, mc->ctcb_arg2, mc->ctcb_arg3);
        }


        if((mc->tube && !mc->tube->generates_events()) ||
                (mc->sampler && !mc->sampler->generates_events()))
            spin = 1;

        if(mc->cancel)
        {
            mc->msg_out("Connection stopped.");
            mc->schedule_destroy(HARD);
            mc->destroy();
            mc->stop_service();
            delete mc;
            if(connection_list_changed_callback)
                (*connection_list_changed_callback)(0, clc_arg1, clc_arg2, clc_arg3);
            continue;
        }
        dont_restart_listening |= mc->negotiating;
        if(!mc->negotiating && mc->do_destroy != KEEP)
        {
            if(!mc->vp.is_valid())
                oopanic("invalid mc");
            if(mc->destroy())
            {
                mc->stop_service();
                delete mc;
            }
            if(connection_list_changed_callback)
                (*connection_list_changed_callback)(0, clc_arg1, clc_arg2, clc_arg3);
            continue;
        }
        if(mc->negotiating)
        {
            switch(mc->poll_negotiation())
            {
            case -1:
                break;
            case 0:
                mc->schedule_destroy(HARD);
                continue;
            case 1:
                if(!mc->local_media_setup_new())
                {
                    mc->schedule_destroy(HARD);
                    continue;
                }
                break;
            default:
                oopanic("nego prog error");
            }
        }

        if(mc->pstate == WAIT_FOR_USER_ACCEPT)
        {
            switch(mc->user_accept)
            {
            case REJECT:
                mc->reject_call();
                break;
            case ACCEPT:
                mc->finish_connection_new();
                break;
            default:
                break;
            }
        }

        if(mc->file_xfer)
        {
            // just want to handle sub-channel stuff and
            // nothing else (these objects have been forked
            // off from other stuff in the system, so they
            // don't really have all the usual characteristics
            // anyway (like we don't want them accepting and
            // creating new channels and that sort of
            // thing.)
            mc->handle_channels();
            not_idle = 1;
            spin = 1;
            continue;
        }

try_connect:
        if(mc->pstate == CONNECTING)
        {
            switch(mc->poll_connect())
            {
            case -1:
                continue;
            case 0:
                mc->schedule_destroy();
                continue;
            case 1:
                if(mc->server_channel)
                    mc->start_server_ops();
                else
                    mc->start_negotiation();
                continue;
            default:
                oopanic("poll prog error");
            }
            continue;
        }

        if(mc->pstate == WAIT_FOR_STUN)
        {
            switch(0/*mc->poll_stun()*/)
            {
            case -1:
                break;
            case 0:
                mc->schedule_destroy();
                continue;
            case 1:
                goto stun_done;
            default:
                oopanic("stun prog error");
            }
        }
stun_done:

        mc->handle_audio_duplex();
        if(!mc->tick())
        {
            // destroy for the channel has been scheduled
            continue;
        }
#ifdef DWYCO_RATE_DISPLAY
        {
        int recv_on = 0;
        int send_on = 0;
        int do_update = 0;
        if(mc->rate_updated(AUDIO_RATE)
                && (mc->grab_audio_id != -1 || mc->audio_decoders.num_elems() > 0)
                && mc->gv_id)
        {
            set_status_text_by_id(mc->gv_id, mc->info_format(AUDIO_RATE).c_str());
            mc->rate_update &= ~AUDIO_RATE;
            if(mc->bps_audio_send.get_rate() > 0)
                send_on = 1;
            if(mc->bps_audio_recv.get_rate() > 0)
                recv_on = 1;
            do_update = 1;
        }
        if(mc->rate_updated(SENDER))
        {
            if(mc->bps_send.get_rate() > 0)
            {
                send_on |= 1;
                do_update = 1;
            }
        }
        if(mc->rate_updated(RECEIVER))
        {
            if(mc->bps_recv.get_rate() > 0)
            {
                recv_on |= 1;
                do_update = 1;
            }
        }
        if(do_update && mc->gv_id)
        {
            set_indicator_by_id(mc->gv_id, IND_SEND, send_on);
            set_indicator_by_id(mc->gv_id, IND_RECV, recv_on);
        }
        }
#endif

        MMTube *const t = mc->tube;
        VidAcquire *const va = mc->sampler;
        DwCoder *const coder = mc->coder;
        DwDecoder *const decoder = mc->decoder;
        //AudioAcquire *const aa = mc->audio_sampler;
        //AudioOutput *const aud_output = mc->audio_output;

        // send part of channel
        not_idle = 1;
        if((int)mc->pause_video || (int)mc->rem_pause_video)
        {
            if(decoder)
            {
                const char *who = "";
                if((int)mc->pause_video && !(int)mc->rem_pause_video)
                    who = "you pause";
                else if(!(int)mc->pause_video && (int)mc->rem_pause_video)
                    who = "they pause";
                else
                    who = "both pause";
                decoder->display_info(who);
            }
        }

        mc->handle_channels();
        // audio is highest priority, so do it first
        if(mc->audio_sampler)
            mc->process_audio_sampler();
        if(!mc->process_outgoing_audio())
            continue;

        // special case, just video in channel is sitting there
        // check for other receivers and if none, schedule
        // destroy.
        if(!t && va && !mc->other_receivers() && !Soft_preview_on)
        {
            mc->schedule_destroy();
            continue;
        }
        // if we're waiting for a stun server to get us some mapped sockets
        // and get that all set up, we don't want to be spraying packets
        // all over the place just yet.
        if(mc->pstate != WAIT_FOR_STUN)
        {
            if(!all_outgoing_paused() && va)
            {
#if defined(DWYCO_THREADED_ENCODE) && defined(LINUX)
                // note: this is a hackaround because the API for the sampler
                // has never really been thread-safe.
                // ie, used to assume if has_data was true, you could always
                // assume "get_data" would return something. with
                // multi-threaded capture and compression, sometimes
                // a frame would be dropped leaving the frame-q empty
                // and get_data would return 0.
                // really need to redo the API so there is one call
                // for "give me what is there" to avoid all this mutex stuff
                pthread_mutex_lock(&sampler_mutex);
#endif
                if(!va->has_data())
                {
                    // check to see if device has some
                    // completed data to give us.
                    // if not, continue on.
                    va->need();
                    GRTLOG("no data", 0, 0);
                }
                else if(mc->send_frame())
                {
                    // get the data, code it, and send it.
                    if(coder && !coder->busy())
                    {
                        int len;
                        DWBYTE *b;

                        GRTLOG("grab1", 0, 0);
                        unsigned long captime;
                        int ref;
                        b = mc->grab_and_code(len, captime, ref, !mc->other_receivers());
                        GRTLOG("grab2", 0, 0);
#if defined(DWYCO_THREADED_ENCODE) && defined(LINUX)
                        if(!b && Coder_pipe && Coder_pipe->next_result_ready())
                        {
                            coder_finished *f = Coder_pipe->get2();
                            b = f->result;
                            captime = f->captime;
                            ref = f->ref;
                            len = f->len;
                            delete f->inp;
                            delete f;
                        }
#endif
                        if(b)
                        {
                            // ca 2013, sampler objects never have tubes
                            // directly in them. all data is provided to proxy
                            // objects which pick up the data and send it where it is
                            // needed.
                            if(t) oopanic("fail fail fail");
                            // note: i don't think any of this is executed
                            // in conferencing mode since t will be zero and
                            // all video output is handled by proxy
                            // objects. in zap recording mode tho, this *is*
                            // used, i think.
                            //
                            mc->fps_send.add_units(1);
#if 0
                            if(mc->force_unreliable_video)
                            {
                                if(t && t->full())
                                {
                                    mc->schedule_destroy();
                                    continue;
                                }
                                else if(t && t->buf_drained() && t->can_write_mmdata())
                                {
                                    t->set_acquisition_time(captime);
                                    t->send_mm_data(b, len, VIDEO_CHANNEL, 0,
                                                    mc->use_v2 ? 4 : 1);
                                    if(mc->status_callback)
                                        (*mc->status_callback)(mc, "Recording", mc->scb_arg1, mc->scb_arg2);
                                }
                                else
                                    ++blocked_on_link;
                            }
                            else
                            {
                                // reliable video
                                oopanic("shouldn't get here");
#if 0
                                if(mc->send_reliable_video(b, len))
                                    mc->bps_send.add_units((long)len * 8);
#endif
                            }
#endif
                            mc->bps_send.add_units((long)len * 8);
                            delete [] mc->last_coded_buf;
                            mc->last_coded_buf = (char *)b;
                            mc->len_last_coded_buf = len;
                            mc->last_acq_time = captime;
                            mc->last_frame_type = ref;
                            ++mc->last_time_index;
                            GRTLOG("code %d", mc->last_time_index, 0);
                        }

                        if(mc->rate_updated(SENDER))
                        {
                            GRTLOG("doing send update", 0, 0);
                            coder->display_info(mc->info_format(SENDER).c_str());
                            mc->rate_update &= ~SENDER;
                        }
                    }
                }
                else
                {
                    if(Moron_dork_mode)
                    {
                        int len;

                        GRTLOG("moron grab1", 0, 0);
                        unsigned long captime;
                        int ref;
                        mc->grab_and_code(len, captime, ref, 1);
                        GRTLOG("moron grab2", 0, 0);
                    }
                    else
                    {
                        // don't call pass from this thread, leave it up
                        // to the sampler when to throw data away
                        va->pass();
                        GRTLOG("chuck data", 0, 0);
                    }
                }
#if defined(DWYCO_THREADED_ENCODE) && defined(LINUX)
                pthread_mutex_unlock(&sampler_mutex);
#endif
            }
            // check to pick up already coded video
            // that is being broadcast
            // if the broadcaster is gone, then destroy the
            // channel...
            if(!coder && mc->grab_coded_id != -1 && !channel_by_id(mc->grab_coded_id))
            {
                mc->schedule_destroy();
                continue;
            }
            if(t && !coder && mc->grab_coded_id != -1 &&
                    channel_by_id(mc->grab_coded_id))
            {
                if(mc->force_unreliable_video || mc->video_state != MEDIA_SESSION_UP)
                {
                    if(t->full())
                    {
                        mc->schedule_destroy();
                        continue;
                    }
                    if(t->buf_drained() && t->can_write_mmdata())
                    {
                        MMChannel *bcast = channel_by_id(mc->grab_coded_id);
                        GRTLOG("blast %d ourlast %d", (int)bcast->last_time_index, (int)mc->last_time_index);
                        if(bcast->last_coded_buf && bcast->last_time_index != mc->last_time_index
                                && (!mc->step_mode || (mc->step_mode && bcast->last_frame_type == 1 && mc->step_frames > 0)))
                        {
                            GRTLOG("proxy output", 0, 0);
                            if(!(int)mc->pause_video && !(int)mc->rem_pause_video)
                            {
                                mc->fps_send.add_units(1);
                                if(mc->vid_make_first_0)
                                {
                                    mc->vid_internal_0_offset = -bcast->last_acq_time;
                                    mc->vid_make_first_0 = 0;
                                }
                                t->set_acquisition_time(mc->vid_internal_0_offset + bcast->last_acq_time);
                                t->send_mm_data((DWBYTE *)bcast->last_coded_buf, (int)bcast->len_last_coded_buf,
                                                VIDEO_CHANNEL, 0, 4);
                                if(mc->status_callback)
                                    (*mc->status_callback)(mc, "Recording", mc->scb_arg1, mc->scb_arg2);
                                mc->bps_send.add_units((long)bcast->len_last_coded_buf * 8);
                                if(mc->step_mode && --mc->step_frames == 0)
                                {
                                    if(mc->step_done_callback)
                                        (*mc->step_done_callback)(mc, mc->sdc_arg1, mc->sdc_arg2, mc->sdc_arg3);
                                }
                            }
                            mc->last_time_index = bcast->last_time_index;
                        }
                        // note: no place to display send statistics unless
                        // we cook up something special.
                    }
                }
                else
                {
                    // reliable video
                    mc->send_reliable_video_proxy();
                }
            }
            if(t && mc->msync_state == MEDIA_SESSION_UP)
            {
                mc->process_outgoing_sync();
            }
            //
            // check for availability of keyboard data and send out
            // a chat message if there is some available.
            // note: in this version, if there are multiple people
            // connected to you, you are broadcasting your chat messages...
            //
            if(mc->private_kaq && mc->private_kaq->has_data())
            {
                vc v = mc->private_kaq->get_structured_data();
                mc->send_chat_private(v);
                // don't show it locally, windows echos the chars
                // for us.
            }
            if(mc->kaq && mc->kaq->has_data() && mc->all_msg_broadcast_done())
            {
                int len;
                const char *msg = mc->kaq->get_data(len);
                vc v(VC_BSTRING, msg, len);
                delete [] msg;

                if(!mc->selective_chat_mode)
                {
                    mc->send_chat(v);
                }
                else
                {
                    mc->send_selective_chat(v, mc->selective_chat_output);
                }
                // show it locally as well (we know the keyboard
                // acquisition also has the chat displayer object...)
                mc->display_chat(mc->username(), v, My_UID);
            }

            if(!mc->msg_chan && !mc->user_control_chan && !mc->kaq && !coder && !decoder &&
                    mc->grab_chat_id != -1 && !channel_by_id(mc->grab_chat_id) &&
                    (mc->grab_coded_id == -1 || (mc->grab_coded_id != -1 && !channel_by_id(mc->grab_coded_id))))
            {
                mc->schedule_destroy();
                continue;
            }

            if(!mc->kaq && mc->grab_chat_id != -1 && channel_by_id(mc->grab_chat_id))
            {
                MMChannel *bcast = channel_by_id(mc->grab_chat_id);
                if(!(bcast->selective_chat_mode && !mc->selective_chat_output))
                {
                    if(!bcast->last_msg.is_nil() && bcast->last_msg_index != mc->last_msg_index)
                    {
                        mc->send_ctrl(bcast->last_msg);
                        mc->last_msg_index = bcast->last_msg_index;
                    }
                }
            }
        } // WAIT_FOR_STUN

ctrl_processing:
        ;
        if(t && t->can_write_ctrl())
        {
            // note: we don't check for buf_drained because we assume
            // the amount of data going into this channel due to chat
            // is minute compared to video and voice data.
            if(!mc->negotiating && mc->drop_send)
            {
                int lost = 0;
                int r = 0;
                int s = 0;
                t->mm_packet_stats(lost, s, r);
                vc v(VC_VECTOR);
                v[0] = "drop";
                if(r + lost > 0)
                    v[1] = (lost * 100) / (r + lost);
                else
                    v[1] = 0;
                // the raw received count can be used on the
                // remote side to detect if there is some
                // problem with the link blocking all udp traffic
                v[2] = r;
                v[3] = s;
                mc->percent_dropped = (long)v[1];
                mc->send_ctrl(v);
                mc->drop_send = 0;
                t->set_mm_packet_stats(0, 0, 0);
            }
            while(mc->ctrl_q.num_elems() > 0)
            {
                vc msg = mc->ctrl_q[0];
                GRTLOG("ctrl q attempt out chan id %d", mc->myid, 0);
                GRTLOGVC(msg);
                // this drop thing is so we can drop the
                // channel after gracefully flushing everything
                // in front of it.
                static vc drop("<<drop>>");
                if(msg == drop)
                {
                    mc->schedule_destroy();
                    goto next_iter_ctrl_send;
                }
                int ret;
                if((ret = t->send_ctrl_data(msg)) >= 0)
                {
                    GRTLOG("ctrl q ok out chan id %d", mc->myid, 0);
                    GRTLOGVC(msg);
                    mc->ctrl_q.remove(0, 1);
                    // note: resets the timer to the interval
                    mc->ctrl_send_watchdog.start();
                    static vc dec("decrypt");
                    if(msg == dec)
                    {
                        // when we successfully send a "decrypt" command,
                        // we turn on the encryption in the tube
                        t->set_key_iv(mc->agreed_key, 0);
                        GRTLOG("ENC on chan %d key = %s", mc->myid, (const char *)to_hex(mc->agreed_key));
                        if(!t->start_encrypt_ctrl())
                        {
                            mc->schedule_destroy();
                            goto next_iter_ctrl_send;
                        }
                    }
                }
                else
                {
                    // note: there is a potential for lost
                    // data here. if there are other commands
                    // q'd after this one, then they will be
                    // lost when the channel is destroyed.
                    // this isn't a problem with real-time
                    // connects, just with server related
                    // channels. we could check that and
                    // schedule error responses here, but
                    // that is too hard at this point. instead
                    // it is better that whoever q'd the message
                    // has some kind of timeout, or a cancel
                    // button the user can click if the response
                    // to the message doesn't come back.
                    // note: if try again is returned, msg is
                    // still in q.
                    if(ret == SSERR)
                    {
                        mc->schedule_destroy();
                        goto next_iter_ctrl_send;
                    }
                    if(mc->ctrl_send_watchdog.is_expired())
                    {
                        if(mc->tube)
                        {
                            Netlog_signal.emit(mc->tube->mklog("event", "ctrl watchdog timeout"));
                        }
                        mc->schedule_destroy();
                        goto next_iter_ctrl_send;
                    }
                    break;
                }
            }
        }
        goto resume_ctrl_send;
next_iter_ctrl_send:
        continue;
resume_ctrl_send:
        // receiver part of channel
        // do as many control messages as possible in
        // one batch
        while(t && t->has_ctrl())
        {
            vc v;
            GRTLOG("has ctrl chan %d", mc->myid, 0);
            if((ret = t->recv_ctrl_data(v)) >= 0)
            {
                GRTLOG("got chan %d pstate %d", mc->myid, mc->pstate);
                GRTLOGVC(v);
                static vc dec("decrypt");
                if(v == dec)
                {
                    if(!mc->agreed_key.is_nil())
                    {
                        GRTLOG("DEC on chan %d key = %s", mc->myid, (const char *)to_hex(mc->agreed_key));
                        t->set_key_iv(mc->agreed_key, 0);
                        t->start_decrypt_ctrl();
                    }
                    continue;
                }
                // do something with control messages
                switch(mc->pstate)
                {
                case WAIT_FOR_MATCH:
                    mc->recv_matches(v);
                    break;
                case WAIT_FOR_CRYPTO:
                    mc->recv_crypto(v);
                    break;
                case WAIT_FOR_MATCH_CRYPTO:
                    mc->recv_matching_crypto(v);
                    break;

                case WAIT_FOR_CONFIG:
                    mc->recv_config(v);
                    break;
                case WAIT_FOR_CLOSE:
                    // shouldn't get here... other side
                    // ought not to have sent us anything
                    mc->schedule_destroy(HARD);
                    goto next_iter;

                case ESTABLISHED:
                case WAIT_FOR_STUN:
                    mc->regular_control_procedures(v);
                    break;
                case SERVER_OPS:
                    mc->server_response(v);
                    break;
                case CHAT_OPS:
                    mc->chat_response(v);
                    break;
                default:
                    // instead of crashing, just close off the channel.
                    // this might be because of a program error introduced
                    // from maintenance hacks...
                    mc->schedule_destroy(HARD);
                    mc->pstate = WAIT_FOR_CLOSE;
                    goto next_iter;

                    //oopanic("bad pstate");

                }
            }
            else
            {
                GRTLOG("got err %d", ret, 0);
                if(ret == SSERR)
                {
                    // control channel error, probably
                    // means other side disconnected
                    mc->schedule_destroy();
                    goto next_iter;
                }
                else
                    goto resume;
            }
        }
        goto resume;
next_iter:
        continue;
resume:
        if(mc->sync_pinger.is_expired())
        {
            if(mc->tube)
            {
                Netlog_signal.emit(mc->tube->mklog("event", "sync pinger timeout"));
            }
            mc->schedule_destroy(HARD);
            continue;
        }
        if(t && mc->msync_state == MEDIA_SESSION_UP && t->has_data(mc->msync_chan))
        {
            mc->process_incoming_sync();
        }
        // note: don't check for paused here because
        // we want any data that might be q'd up to
        // come through before stopping (ie, pause
        // is controlled by sender)
        while(t && t->has_data(AUDIO_CHANNEL_V2))
        {
            if(mc->process_incoming_audio() == SSERR)
                break;
        }
        while(t && !mc->force_unreliable_video && mc->video_state == MEDIA_SESSION_UP && t->has_data(mc->video_chan))
        {
            mc->process_incoming_reliable_video();
        }
        if(t && t->has_mmdata() &&
                (!mc->step_mode || (mc->step_mode && mc->step_frames > 0)))
        {
            // some frame info waiting to be processed
            DWBYTE *b;
            int len;
            int chan;
            int seq;
            if((ret = t->recv_mm_data(b, len, chan, seq, 4)) >= 0)
            {
                if(len == 0)
                {
                    // dummy hole punching packet
                    GRTLOG("hole punch from myid %d", mc->myid, 0);
                    delete [] b;
                    goto dropit;
                }
                switch(chan)
                {
                case VIDEO_CHANNEL: // video
                    // NOTE: need to build a decoder here if we are going to use
                    // unreliable video over the net. playing from a file does the
                    // init, but we need to make sure the init gets done properly for
                    // unreliable channels (and doesn't happen multiple times if it takes
                    // awhile for the reliable channel to come up...)
                    if(decoder && !decoder->busy())
                    {
                        decoder->decode_postprocess(b, len);
                        GRTLOG("sent to decoder", 0, 0);
                        mc->fps_recv.add_units(1);
                        if(!mc->pause && mc->rate_updated(RECEIVER))
                        {
                            mc->rate_update &= ~RECEIVER;
                            decoder->display_info(mc->info_format(RECEIVER).c_str());
                        }
                        if(mc->step_mode)
                        {
                            // hack: the coders take a frame at a time, and with the
                            // theora codecs, we may have to give it several frames
                            // before it can actually display it (it seeks for the
                            // first keyframe.) this may be a problem in general if
                            // disk capturing and the stream ends up having a non-key
                            // frame as the first frame, i'm not sure. for now, we just
                            // check for this particular theora case and work around it.
                            // what we really need here is some indication from the codec
                            // that it actually displayed a frame.
#ifndef DWYCO_NO_THEORA_CODEC
                            CDCTheoraDecoderColor *d = dynamic_cast<CDCTheoraDecoderColor *>(decoder);
                            if(d && !d->seeking_keyframe)
                                --mc->step_frames;
                            else if(!d)
#endif
                                --mc->step_frames;
                            if(mc->step_frames <= 0 && mc->step_done_callback)
                                (*mc->step_done_callback)(mc, mc->sdc_arg1, mc->sdc_arg2, mc->sdc_arg3);
                        }
                    }
                    break;
                case AUDIO_CHANNEL: // audio
                    // if the audio output has been turned off,
                    // just drop the packet.
                {
                    mc->bps_audio_recv.add_units(8 * len);
                    int drop = inet_sim();
                    GRTLOG("recv audio got %d (drop %d)", seq, drop);
                    // i don't think any of this can be executed because
                    // the aud_decoder is always 0.
#if 0
                    if(!drop &&  aud_decoder && aud_output && aud_output->status())
                    {
                        // decode it, should buffer up some amount
                        // before starting playback
                        ++used_cpu;
                        DWBYTE *bbuf;
                        int blen;
                        // note: assumes audio decoder returns immediately
                        // with decoded samples. breaks if audio decoder is
                        // asynch.
                        aud_decoder->play_timed(b, len, 0, 0);
                        while(!aud_decoder->device_done2(bbuf, blen))
                        {
                            // spin
                        }
                        if(aud_output)
                        {
                            if(mc->use_v2)
                                aud_output->play_seq_ec(bbuf, blen, seq, 0, 1);
                            else
                                aud_output->play_seq(bbuf, blen, seq, 1);
                        }
                        else
                        {
                            delete [] bbuf;
                        }
                    }
#endif
                }
                break;

                default: // drop it
                    break;
                }
                mc->bps_recv.add_units((long)len * 8);
                delete [] b;
            }
            else
            {
                if(ret == SSERR)
                {
                    // hmmm, what to do, this isn't as
                    // severe as a control channel error.
                }
            }
        }
dropit:
        ;
        while(!mc->force_unreliable_audio && t && mc->audio_state == MEDIA_SESSION_UP && t->has_data(mc->audio_chan))
        {
            mc->process_incoming_reliable_audio();
        }
    }
    if(!dont_restart_listening)
    {
        turn_accept_on();
    }
#if 0
    if(not_idle == 0)
        set_status(STATUS_NONE);
    else if(!(used_cpu == 0 && blocked_on_link == 0))
        set_status((blocked_on_link && !used_cpu) ? STATUS_LINK : STATUS_CPU);
#endif
    lock = 0;
    if(spin_out)
        *spin_out = spin;
    return not_idle;
}

