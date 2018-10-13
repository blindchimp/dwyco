
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef _Windows
#ifdef __BORLANDC__
#include <dir.h>
#else
#include <direct.h>
#endif
#if __BORLANDC__ >= 0x560 || defined(_MSC_VER)
#include <io.h>
#endif
#endif
#include "qauth.h"
#include "dwstr.h"
#include "filetube.h"
#include "zapadv.h"
#include "ratetwkr.h"
#include "mmchan.h"
#include "audout.h"
#include "qmsg.h"
#include "qdirth.h"
#include "msgdisp.h"
#include "vidinput.h"
#include "audchk.h"
#include "ta.h"
#include "usercnfg.h"
#include "dirth.h"
#include "codec.h"
#include "fnmod.h"
#include "mcc.h"
#include "se.h"
#include "sysattr.h"
#include "xinfo.h"
#undef index

#include "dlli.h"
namespace dwyco {
extern double Audio_delay;
}

extern DwycoStatusCallback dbg_msg_callback;
using namespace dwyco;

static int
msgbox(const char *s, const char *s2, int)
{
    if(!s)
        s = "";
    if(!s2)
        s2 = "";
    DwString msg = s;
    msg += " -- ";
    msg += s2;

    if(dbg_msg_callback)
        (*dbg_msg_callback)(0, msg.c_str(), 0, 0);

    return IDCANCEL;
}

DwVec<ValidPtr> Composers;
DwVec<ValidPtr> CompositionDeleteQ;
DwVec<DwString> FileDeleteQ;

static int Zaps_recording;

extern int Auto_squelch;
extern int All_mute;
// note: if you are in a conference and
// record a zap, the audio from the zap
// will be transmitted to everyone (probably
// want to change this somehow.)
static int Old_All_mute;
static int Old_auto_squelch;


extern CRITICAL_SECTION Audio_lock;
extern int Inhibit_audio_control_changes;

static void
restore_audio()
{
    EnterCriticalSection(&Audio_lock);
    All_mute = Old_All_mute;
    LeaveCriticalSection(&Audio_lock);
    Auto_squelch = Old_auto_squelch;
    Inhibit_audio_control_changes = 0;

}
static void
save_audio()
{
    EnterCriticalSection(&Audio_lock);
    Old_All_mute = All_mute;
    LeaveCriticalSection(&Audio_lock);
    Old_auto_squelch = Auto_squelch;
    Inhibit_audio_control_changes = 1;
}
//
// this is a little bogus...
// if a file cannot be deleted but it exists,
// then we try to delete it periodically.
// this is usually because some other part of
// the system has the file open for some
// reason, and it can't be deleted right away.
// rather than try to find and fix all the
// little race conditions, we just brute force it here.
//

void
DeleteFileQ(const char *fn)
{
    if(DeleteFile(newfn(fn).c_str()))
        return;
    if(access(newfn(fn).c_str(), 0) != 0)
        return;
    FileDeleteQ.append(fn);
}

void
TryDeletes()
{
    int n = FileDeleteQ.num_elems();
    for(int i = 0; i < n; ++i)
    {
        if(!DeleteFile(newfn(FileDeleteQ[i]).c_str()))
            continue;
        if(access(newfn(FileDeleteQ[i]).c_str(), 0) == 0)
            continue;
        FileDeleteQ.del(i);
        --n;
        --i;
    }
    if(FileDeleteQ.num_elems() > 20)
    {
        // screw it, this hack ain't
        // working, since we clean up
        // at startup and exit, just
        // punt, so it doesn't get too
        // big.
        FileDeleteQ.set_size(0);
    }
}

TMsgCompose::TMsgCompose()
    : vp(this)
{
    cancel_button_enabled = 1;
    record_button_enabled = 1;
    record_video = 0;
    record_audio = 0;
    record_pic = 0;
    stop_button_enabled = 1;
    play_button_enabled = 1;
    send_button_enabled = 1;
    start_over_button_enabled = 1;

    do_append = 0;
    rid_list = vc(VC_VECTOR);
    recipient_list = vc(VC_VECTOR);
    stop_id = -1;
    stop_audio_id = -1;
    view_id = -1;
    last_audio_timecode = 0;
    changed = 0;
    forward = 0;
    recording = 0;
    recording_audio = 0;
    force_close = 0;
    destroy_callback = 0;
    dcb_arg1 = 0;
    dcb_arg2 = 0;
    status_callback = 0;
    scb_arg1 = 0;
    inhibit_hashing = 0;
    composer = 0;
    pal_auth_mode = 0;
    pal_auth_ok_mode = 0;
    pal_auth_rej_mode = 0;
    no_forward = 0;
    limited_forward = 0;
    max_frames = -1;
    max_bytes = 100 * 1024 * 1024;
    hiq = 0;
    dont_save_sent = 0;
    special_type = 0;
}

TMsgCompose::TMsgCompose(const TMsgCompose& m)
{
    cancel_button_enabled  = m.    cancel_button_enabled;
    record_button_enabled  = m.    record_button_enabled;
    record_video  = m.record_video;
    record_audio  = m.record_audio;
    record_pic  = m.record_pic;
    stop_button_enabled  = m.    stop_button_enabled;
    play_button_enabled  = m.    play_button_enabled;
    send_button_enabled  = m.    send_button_enabled;
    start_over_button_enabled  = m.    start_over_button_enabled;

    vp = ValidPtr(this);

    do_append  = m.    do_append ;
    // note: these are vectors, but i don't think they are used
    rid_list  = m.	 rid_list.copy() ;
    recipient_list  = m.	 recipient_list.copy() ;
    stop_id  = m.    stop_id ;
    stop_audio_id  = m.    stop_audio_id ;
    last_audio_timecode  = m.	 last_audio_timecode ;
    changed  = m.	 changed ;
    forward  = m.	 forward ;
    recording  = m.	recording ;
    recording_audio  = m.	recording_audio ;
    force_close  = m.	force_close ;
    destroy_callback  = m.    destroy_callback ;
    dcb_arg1  = m.    dcb_arg1 ;
    dcb_arg2  = m.    dcb_arg2 ;
    status_callback  = m.    status_callback ;
    scb_arg1  = m.    scb_arg1 ;
    inhibit_hashing  = m.    inhibit_hashing ;
    composer  = m.    composer ;
    pal_auth_mode  = m.	pal_auth_mode ;
    pal_auth_ok_mode  = m.	pal_auth_ok_mode ;
    pal_auth_rej_mode  = m.	pal_auth_rej_mode ;
    no_forward  = m.	no_forward ;
    limited_forward  = m.    limited_forward ;
    max_frames  = m.	max_frames ;
    max_bytes  = m.	max_bytes ;

    file_basename = m.file_basename;
    actual_filename = m.actual_filename;
    qfn = m.qfn;
    msg_text = m.msg_text;

    // can't support this now because copy isn't a deep copy,
    // so we just nil it out and understand that this gets
    // initialized when the "send" occurs
    msg_to_send = vcnil;

    filehash = m.filehash;
    // this is ok, since this is read-only, despite being a
    // multi-level composite
    body_to_forward = m.body_to_forward;
    user_filename = m.user_filename;
    dont_save_sent = m.dont_save_sent;
    special_type = m.special_type;
}

TMsgCompose::~TMsgCompose()
{

    if(composer)
    {
        if(qfn.length() > 0)
            DeleteFileQ(qfn.c_str());
        if(actual_filename.length() > 0)
            DeleteFileQ(actual_filename.c_str());
    }
#ifdef DWYCO_TRACE
    void invalidate_cb_ctx(int);
    invalidate_cb_ctx(vp.cookie);
#endif
    vp.invalidate();
}

void
TMsgCompose::forceClose()
{
    force_close = 1;
    Close();
}

void
TMsgCompose::Close()
{
    bool canclose = 1;
    FormCloseQuery(canclose);
    if(!canclose)
        return;
    if(CompositionDeleteQ.index(vp) == -1)
        CompositionDeleteQ.append(vp);
}

//---------------------------------------------------------------------------
void  TMsgCompose::cancel_buttonClick()
{
    if(stop_id != -1)
    {
        MMChannel *mc;
        if((mc = MMChannel::channel_by_id(stop_id)))
        {
            MMChannel::synchronous_destroy(stop_id);
            stop_id = -1;
            if(recording_audio && --Zaps_recording <= 0)
            {
                restore_audio();
                recording_audio = 0;
                Zaps_recording = 0;
            }
        }
        recording = 0;

    }
    Close();
}
//---------------------------------------------------------------------------

static void
reset_buttons(MMChannel *m, vc, void *, ValidPtr v)
{
    if(m->tube)
        m->tube->flush();

    if(!v.is_valid())
        return;
    TMsgCompose *q = (TMsgCompose *)(void *)v;
    // avoid calling destroy again
    q->stop_id = -1;
    q->stop_audio_id = -1;
    q->stop_buttonClick();
    if(q->actual_filename.length() > 0 && !q->inhibit_hashing)
        q->filehash = gen_hash(q->actual_filename);
    if(q->destroy_callback)
        (*q->destroy_callback)(v.cookie, q->dcb_arg1);
}

static void
record_status(MMChannel *mc, vc msg, void *, ValidPtr vp)
{
    if(!vp.is_valid())
        return;
    TMsgCompose *q = (TMsgCompose *)(void *)vp;
    if(q)
    {
        int e = dynamic_cast<SlippyTube *>(mc->tube)->bytes_left;
        int p = (int)(((double)e * 100) / (q->max_bytes == 0 ? 1 : q->max_bytes));
        if(q->status_callback)
            (*q->status_callback)(vp.cookie, msg, p, q->scb_arg1);
    }
}

//---------------------------------------------------------------------------
int  TMsgCompose::record_buttonClick()
{
    // avoid re-entering if we are in the middle of
    // trying to shut down the video capture device...
    // this is thanks to microsoft... the system will
    // probably crash if you don't watch out here...

    extern int Sleeping;

    if(Sleeping)
        return 0;

    if(record_pic)
    {
        return do_record_pic();
    }
    int do_vid = record_video;
    int do_aud = record_audio;

    if(!do_vid && !do_aud)
    {
        return 0;
    }

    record_button_enabled = 0;
    play_button_enabled = 0;
    send_button_enabled = 0;
    cancel_button_enabled = 0;

    const char *mode;
    int appending = 0;
    if(actual_filename.length() == 0 || !do_append)
    {
        DwString s = gen_random_filename();
        s += ".dyc";
        actual_filename = newfn(s);
        file_basename = s;
        mode = "wb+";
    }
    else
    {
        mode = "ab+";
        appending = 1;
    }
#if 0
    DwString filename = "test.dyc";
    char *mode = "wb+";
    int do_vid = 1;
    int do_aud = 1;
    int appending = 0;
#endif

    SlippyTube *ft =  new SlippyTube(actual_filename.c_str(), mode, FileTube::SINK);
    ft->bytes_left = max_bytes;
    ft->packet_count = max_frames;
    ft->prefer_old_timing = ZapAdvData.get_use_old_timing();
    // output regardless, so indexer knows new timing
    // info should be used, but the output block
    // is ignored by the old software
    //if(do_append)
    ft->reset_timer(MMChannel::codec_name_to_number(sysattr_get_vc("us-video-coder-qms")));

    // this is a hack: we swap in some reasonable defaults
    // for recording messages, because most of the
    // rest of the system still uses these global
    // variables. should fix it sometime...

    RateTweakerXferValid save_rt;
    save_rt = RTUserDefaults;
    RTUserDefaults.set_max_frame_rate(hiq ? 20 : 10);

    MMChannel *mc = MMChannel::gen_chan();
    mc->tube = ft;
    mc->init_config(1);
    mc->set_requested_audio_codec("qms");
    mc->recv_matches(mc->config);
    mc->force_unreliable_audio = 1;
    mc->force_unreliable_video = 1;
    mc->set_string_id("<<record proxy>>");
    stop_id = mc->myid;
    mc->start_service();
    mc->destroy_callback = reset_buttons;
    mc->dcb_arg2 = 0;
    mc->dcb_arg3 = vp;
    mc->status_callback = record_status;
    mc->scb_arg2 = vp;
    // this inhibits building grayview any control point
    // on the GUI, the only thing you see is the coder
    // window if video is recording.

    if(do_vid)
    {
        view_id = mc->myid;
        mc->gv_id = -1;
        if(!mc->build_outgoing(1, 1))
        {
            msgbox("Video recording device not available.", 0, MB_OK);
            stop_buttonClick();
            RTUserDefaults = save_rt;
            do_append = 1;
            return 0;
        }
        // note: build_outgoing sets the b/w to the initially
        // negotiated b/w on the network, so we need to reset it to
        // our "virtual" b/w for the file record.
        ft->set_est_baud(hiq ? 5 * 1024 * 1024 : 384 * 1024);
        MMChannel *mcx = MMChannel::find_xmitter();
        if(mcx)
        {
            // note: this is really subterfuge... we are displaying
            // the preview capture frames instead of what is actually
            // recorded into the file. sigh... marketing
            mcx->coder->gvs.append(view_id);
            // start with a reference frame if possible
            mcx->ready_for_ref = 1;
        }
        mc->vid_make_first_0 = 1;
    }
    if(do_aud)
    {
        // inhibit creation of a grayview in case
        // we didn't do video setup above.
        if(mc->gv_id == 0)
            mc->gv_id = -1;

        if(!mc->build_outgoing_audio(1))
        {
            msgbox("Audio recording device not available.", 0, MB_OK);
            stop_buttonClick();
            RTUserDefaults = save_rt;
            do_append = 1;
            return 0;
        }
        if(appending)
        {
            mc->make_first_0 = 1;
            mc->client_timecode_adjustment = last_audio_timecode + 1;
        }
        recording_audio = 1;
        if(Zaps_recording <= 0)
        {
            save_audio();
        }
        EnterCriticalSection(&Audio_lock);
        All_mute = 0;
        LeaveCriticalSection(&Audio_lock);
        Auto_squelch = 0;
        ++Zaps_recording;
    }
    RTUserDefaults = save_rt;
    do_append = 1;
    recording = 1;
    stop_button_enabled = 1;
    start_over_button_enabled = 0;
    return 1;
}
//---------------------------------------------------------------------------
void  TMsgCompose::stop_buttonClick()
{
    MMChannel *mv = 0;
    MMChannel *ma = 0;
    if(stop_id != -1)
    {
        mv = MMChannel::channel_by_id(stop_id);
        stop_id = -1;
        if(recording_audio && --Zaps_recording <= 0)
        {
            restore_audio();
            Zaps_recording = 0;
            recording_audio = 0;
        }
    }
    if(stop_audio_id != -1)
    {
        ma = MMChannel::channel_by_id(stop_audio_id);
        stop_audio_id = -1;
    }
    if(mv)
    {
        mv->schedule_destroy();
    }
    // remove the displayer id from the
    // coder display so we don't have it
    // updating from another stream.
    MMChannel *mcx = MMChannel::find_xmitter();
    if(mcx)
    {
        int i = mcx->coder->gvs.index(view_id);
        if(i >= 0)
            mcx->coder->gvs.del(i);
        view_id = -1;
    }
#if 0
    // the reason we dont' do this anymore
    // is because it is really more appropriate
    // for the timing to be derived from the info
    // available in the tube at decode time.
    // because the sampler is shared, we can't
    // just set the timecode, because there may
    // be more than one channel recording its
    // output.
    //
    MMChannel *mt = MMChannel::find_audio_xmitter();
    if(mt)
        last_audio_timecode = mt->get_audio_timecode();
#else
    // this is the timecode of the last audio
    // packet sent to the tube.
    if(mv && recording_audio)
        last_audio_timecode = mv->get_audio_timecode();
#endif

    recording = 0;
    recording_audio = 0;

    record_button_enabled = 1;
    stop_button_enabled = 0;
    start_over_button_enabled = 1;
    play_button_enabled = 1;
    send_button_enabled = 1;
    cancel_button_enabled = 1;
}
//---------------------------------------------------------------------------


void  TMsgCompose::play_buttonClick( int no_audio)
{
    if(actual_filename.length() == 0
            // TMP HACK TO PREVENT CRASH
            || actual_filename.contains(".jpg"))
        return;
    record_button_enabled = 0;
    start_over_button_enabled = 0;
    play_button_enabled = 0;
    send_button_enabled = 0;
    cancel_button_enabled = 0;

    MMChannel *mc = MMChannel::gen_chan();

    int codec = 0;
    {
        SlippyTube *st;
        mc->tube = st = new SlippyTube(actual_filename.c_str(), "rb", FileTube::SOURCE);
        if(no_audio)
        {
            st->set_chan_delay(0, 0);
            st->set_chan_enables(1, 0);
            // override hack that stops channel automatically
            st->set_auto_stop_delay(-300);
        }
        else
        {
            st->set_chan_delay(0, 1000 * Audio_delay);
            st->set_auto_stop_delay(1000 * Audio_delay);
            // this is a bit broken... in the "composer", we assume we
            // are playing our own video, which was recorded using
            // our cap system. if we have the timing hack engaged because
            // of a broken driver (use old timing) we use the global
            // "ZapAdv.get_use_old_timing". but now these objects can
            // be used to playback any media, including from other
            // users, so we really need to select the use old timing based
            // on the uid (for local override) if that's what we want.
            if(composer)
            {
                st->use_old_timing = ZapAdvData.get_use_old_timing();
            }
        }
        int dummy;
        st->quick_stats(-1, 0, 0, dummy, dummy, dummy, codec);
    }

    mc->init_config(1);
    mc->set_requested_codec(codec);
    mc->recv_matches(mc->config);
    mc->force_unreliable_audio = 1;
    mc->force_unreliable_video = 1;

    view_id = mc->myid;
    mc->gv_id = view_id;
    mc->build_incoming_video();
    mc->set_string_id("<<file source>>");
    mc->start_service();
    mc->destroy_callback = reset_buttons;
    mc->dcb_arg2 = 0;
    mc->dcb_arg3 = vp;
    stop_id = mc->myid;
    mc->build_incoming_audio(1);
    if(mc->audio_output)
        mc->audio_output->fancy_recover = 0;

    stop_button_enabled = 1;

}

static void
just_stop(MMChannel *m, vc, void *, ValidPtr)
{
    m->schedule_destroy(MMChannel::HARD);
}

void  TMsgCompose::preview_play()
{
    // HACK TO AVOID CRASHES
    if(actual_filename.contains(".jpg"))
        return;
    play_button_enabled = 0;

    MMChannel *mc = MMChannel::gen_chan();
    mc->force_unreliable_audio = 1;
    mc->force_unreliable_video = 1;

    FILE *f = fopen(actual_filename.c_str(), "rb");
    if(!f)
    {
        play_button_enabled = 1;
        return;
    }
    fclose(f);
    f = 0;
    record_button_enabled = 0;
    start_over_button_enabled = 0;
    play_button_enabled = 0;
    send_button_enabled = 0;
    cancel_button_enabled = 0;
    int preview_play = 1;

    int codec = 0;
    {
        SlippyTube *st;
        mc->tube = st = new SlippyTube(actual_filename.c_str(), "rb", FileTube::SOURCE);
        if(preview_play)
        {
            st->vid_frames_to_index = 1;
            st->set_chan_delay(0, 0);
        }
        else
        {
            st->set_chan_delay(0, 1000 * Audio_delay);
            st->set_auto_stop_delay(1000 * Audio_delay);
        }
        int dummy;
        st->quick_stats(-1, 0, 0, dummy, dummy, dummy, codec);
    }
    mc->init_config(1);
    mc->set_requested_codec(codec);
    mc->recv_matches(mc->config);

    view_id = mc->myid;
    mc->gv_id = view_id;
    mc->build_incoming_video();
    mc->set_string_id("<<file source>>");
    mc->start_service();
    mc->destroy_callback = reset_buttons;
    mc->dcb_arg2 = 0;
    mc->dcb_arg3 = vp;
    stop_id = mc->myid;
    if(preview_play)
    {
        mc->step_mode = 1;
        mc->step_frames = 1;
        mc->step_done_callback = just_stop;
    }
    else
    {
        mc->build_incoming_audio(1);
        if(mc->audio_output)
            mc->audio_output->fancy_recover = 0;
    }

    stop_button_enabled = 1;
    preview_play = 0;
}

//---------------------------------------------------------------------------

void  TMsgCompose::start_over_buttonClick()
{
    if(actual_filename.length() == 0)
        return;
    do_append = 0;
    last_audio_timecode = 0;
    DeleteFileQ(actual_filename.c_str());
    actual_filename = "";
    file_basename = "";
    filehash = vcnil;
    start_over_button_enabled = 0;
    stop_button_enabled = 0;
    record_button_enabled = 1;
    play_button_enabled = 0;
}

void
TMsgCompose::disable_most()
{
    record_button_enabled = 0;
    start_over_button_enabled = 0;
    play_button_enabled = 0;
    send_button_enabled = 0;
    cancel_button_enabled = 0;
}
void
TMsgCompose::enable_most()
{
    record_button_enabled = 1;
    start_over_button_enabled = 1;
    play_button_enabled = 1;
    send_button_enabled = 1;
    cancel_button_enabled = 1;
}

void
TMsgCompose::reset_dialog()
{
    qfn = "";
    actual_filename = "";
    file_basename = "";
    do_append = 0;

    init_av_buttons();

}

void
TMsgCompose::init_av_buttons()
{

    stop_button_enabled = 0;
    play_button_enabled = 0;
    start_over_button_enabled = 0;
    if(VidInputData.get_no_video())
    {
        record_video = 0;
    }
    else
    {
        record_video = 1;
    }
    if(!Has_audio_input)
    {
        record_audio = 0;
    }
    else
    {
        record_audio = 1;
    }
}

//---------------------------------------------------------------------------

void  TMsgCompose::FormCloseQuery(
    bool &CanClose)
{
    int result = 1;
    if(force_close)
        goto out;
    if(stop_id != -1)
        result = 0;
    else if(result /* &&
            (xfer_channel || dirth_pending_callbacks(send_done, this, ReqType())
             || (send_channel && send_channel->ui_assoc.is_valid()))*/
           )
    {
        if(qfn.length() > 0)
            DeleteFileQ(qfn.c_str());
        if(actual_filename.length() > 0)
            DeleteFileQ(actual_filename.c_str());
        reset_dialog();
        result = 1;
#if 0
        dirth_cancel_callbacks(send_done, this, ReqType());
        if(xfer_channel)
            xfer_channel->schedule_destroy(MMChannel::HARD);
#endif
    }
    else if(result && (changed || actual_filename.length() > 0))
    {
        if(actual_filename.length() > 0)
            DeleteFileQ(actual_filename.c_str());
    }

out:
    ;
    // avoid unexpected ops coming in
    if(result)
        vp.invalidate();

    CanClose = result;
}
//---------------------------------------------------------------------------

void  TMsgCompose::send_buttonClick()
{
    if(stop_id != -1)
    {
        MMChannel *mc;
        if((mc = MMChannel::channel_by_id(stop_id)))
        {
            MMChannel::synchronous_destroy(stop_id);
            stop_id = -1;
            if(recording_audio && --Zaps_recording <= 0)
            {
                restore_audio();
                Zaps_recording = 0;
                recording_audio = 0;
            }
        }
        recording = 0;

    }
    if(rid_list.num_elems() == 0)
        return;

    disable_most();

    static vc pr("palreq");
    static vc pok("palok");
    static vc pnok("palrej");
    vc sp;
    vc sp_payload;
    sp_payload = vctrue;
    if(pal_auth_mode)
        sp = pr;
    else if(pal_auth_ok_mode)
        sp = pok;
    else if(pal_auth_rej_mode)
        sp = pnok;
    else
    {
        if(special_type != 0)
        {
            switch(special_type)
            {
            case DWYCO_SPECIAL_TYPE_BACKUP:
                sp = "backup";
                break;
            case DWYCO_SPECIAL_TYPE_JOIN1:
                sp = "join1";
                break;
            case DWYCO_SPECIAL_TYPE_JOIN2:
                sp = "join2";
                break;
            case DWYCO_SPECIAL_TYPE_JOIN3:
                sp = "join3";
                break;
            case DWYCO_SPECIAL_TYPE_JOIN4:
                sp = "join4";
                break;
            case DWYCO_SPECIAL_TYPE_USER:
                sp = "user";
                break;
            }
            sp_payload = special_payload;
        }
    }
    // note: this is a little odd, two copies of the text will be
    // in the message, one for old clients, and one for
    // new authenticated clients, tho icuii doesn't really
    // have buttons for authentication, and no forwarding
    // at the moment, so forward authentication isn't an issue.
    vc ufn = user_filename;

    if(!q_message(rid_list, file_basename.c_str(), qfn,
                  body_to_forward, msg_text.c_str(), filehash, sp, sp_payload, no_forward, ufn, !dont_save_sent))
    {
        msgbox("Can't Q message, free up some diskspace and try again.", 0, MB_OK);
        enable_most();
        return;
    }

}
//---------------------------------------------------------------------------



void  TMsgCompose::FormShow()
{
    init_av_buttons();
    if(actual_filename.length() > 0)
        stop_buttonClick();
}
//---------------------------------------------------------------------------

static void
step_done(MMChannel *mc, vc, void *, ValidPtr vp)
{
    if(!vp.is_valid())
        return;
    TMsgCompose *t = (TMsgCompose *)(void *)vp;
    t->step_done(mc);
}

int
TMsgCompose::do_record_pic()
{
    record_button_enabled = 0;
    play_button_enabled = 0;
    send_button_enabled = 0;
    cancel_button_enabled = 0;

    const char *mode;
    int appending = 0;
    if(actual_filename.length() == 0)
    {
        DwString s = gen_random_filename();
        s += ".dyc";
        actual_filename = newfn(s);
        file_basename = s;

    }
    // always overwrite the file on a refresh
    mode = "wb+";

    MMChannel *mc;
    if(recording && (mc = MMChannel::channel_by_id(stop_id)))
    {
        if(mc->tube)
            delete mc->tube;
        SlippyTube *ft =  new SlippyTube(actual_filename.c_str(), mode, FileTube::SINK);
        ft->prefer_old_timing = ZapAdvData.get_use_old_timing();
        ft->reset_timer(MMChannel::codec_name_to_number(sysattr_get_vc("us-video-coder-qms")));
        ft->packet_count = 1;
        mc->tube = ft;
        mc->step_frames = 1;
        mc->step_mode = 1;
        MMChannel *mcx = MMChannel::find_xmitter();
        view_id = mc->myid;
        if(!mcx)
        {
            mc->gv_id = view_id;
        }
        if(mcx)
        {
            // add an output id to the coder's
            // list of output areas
            mcx->coder->gvs.append(view_id);
            // this is pretty heinous
            // since we are picking up a frame
            // from a capture that is in progress
            // we need to set this to the current
            // time index the capture channel has.
            // the reason for this is that if we
            // use an older time, we may end up
            // getting a frame that is already coded
            // and cached in the "last_buf" buffer.
            // problem is, this won't result in a
            // blit from the coder to the screen, which
            // for marketing reason we want to have from
            // the coder, not from the decoded output of
            // whatever frame we find.
            // boosting this time, it forces us to wait
            // until the coder has grabbed a new frame,
            // coded it, and displayed it. we then grab it
            // and put it in the file.
            mc->last_time_index = mcx->last_time_index;
            mcx->ready_for_ref = 1;
        }

        return 1;
    }


    SlippyTube *ft =  new SlippyTube(actual_filename.c_str(), mode, FileTube::SINK);
    ft->prefer_old_timing = ZapAdvData.get_use_old_timing();
    // output regardless, so indexer knows new timing
    // info should be used, but the output block
    // is ignored by the old software
    //if(do_append)
    ft->reset_timer(MMChannel::codec_name_to_number(sysattr_get_vc("us-video-coder-qms")));
    ft->packet_count = 1;

    // this is a hack: we swap in some reasonable defaults
    // for recording messages, because most of the
    // rest of the system still uses these global
    // variables. should fix it sometime...

    RateTweakerXferValid save_rt;
    save_rt = RTUserDefaults;
    RTUserDefaults.set_max_frame_rate(12);

    mc = MMChannel::gen_chan();
    mc->tube = ft;
    mc->init_config(1);
    mc->recv_matches(mc->config);
    mc->set_string_id("<<record proxy>>");
    stop_id = mc->myid;
    mc->start_service();
    mc->step_mode = 1;
    mc->step_frames = 1;
    mc->step_done_callback = ::step_done;
    mc->sdc_arg2 = 0;
    mc->sdc_arg3 = vp;
    view_id = mc->myid;

    mc->gv_id = -1;
    if(!mc->build_outgoing(1, 1))
    {
        msgbox("Video recording device not available.", 0, MB_OK);
        RTUserDefaults = save_rt;
        return 0;
    }
    // set it high, so that a frame is emitted almost immediately
    ft->set_est_baud(1000000);
    MMChannel *mcx = MMChannel::find_xmitter();
    if(mcx)
    {
        // add an output id to the coder's
        // list of output areas
        mcx->coder->gvs.append(view_id);
        // try to make sure we get a reference frame
        mcx->ready_for_ref = 1;
    }
    mc->vid_make_first_0 = 1;

    RTUserDefaults = save_rt;
    recording = 1;
    start_over_button_enabled = 0;
    return 1;
}


void TMsgCompose::step_done(MMChannel *mc)
{
    //TODO: Add your source code here
    record_button_enabled = 1;
    send_button_enabled = 1;
    cancel_button_enabled = 1;
    mc->step_mode = 0;
    if(mc->tube)
        mc->tube->flush();
    if(actual_filename.length() > 0 && !inhibit_hashing)
    {
        filehash = gen_hash(actual_filename);
    }
    // remove the displayer id from the
    // coder display so we don't have it
    // updating from another stream.
    MMChannel *mcx = MMChannel::find_xmitter();
    if(mcx)
    {
        int i = mcx->coder->gvs.index(view_id);
        if(i >= 0)
            mcx->coder->gvs.del(i);
        view_id = -1;
    }
}



//---------------------------------------------------------------------------

int
TMsgCompose::reset_composer()
{
    if(stop_id != -1)
        return 0;

    return 1;
}

