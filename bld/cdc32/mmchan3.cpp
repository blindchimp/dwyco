
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <sys/types.h>
#include <sys/stat.h>
#ifdef LINUX
#include <unistd.h>
#endif
#include <stdio.h>
#include "mmchan.h"
#include "sproto.h"
#include "fnmod.h"
#include "dwrtlog.h"
#include "qmsg.h"
#include "dhsetup.h"
#include "vcudh.h"
#include "ta.h"
#include "qauth.h"
#include "dhgsetup.h"
#include "qmsgsql.h"

using namespace dwyco;

#ifdef _MSC_VER
#define ftello ftell
#define fseeko fseek
#endif

// this is the protocol for sending directly to another client
// note that there is no "restart" for these sends, it always
// starts from the beginning
struct strans file_send[] = {
    {"start", "i", &MMChannel::init_connect, "send-file-info"},
    {"send-file-info", "w", &MMChannel::send_file_info, "wait-for-ok"},
    {"wait-for-ok", "r", &MMChannel::check_file_response, "send-file-data"},
    {"send-file-data", "w", &MMChannel::send_chunk, "end-xfer"},
    {"end-xfer", "r", &MMChannel::wait_for_ok, "end"},
    {0}
};

// slightly different key setup for sending to server...
// this brings up an idea tho, before, the protocol was identical
// for sending to server vs. sending to a peer directly. possibly
// we could use the same store and forward protocol for direct peer?
// that would avoid having two protocols... hmmm, public key distribution
// might make that not work so well, because the server the public key
// is a-priori installed, whereas for clients, it may not be immediately
// available.
struct strans file_send_server[] = {
    {"start", "i", &MMChannel::init_connect, "send-crypto"},
    {"send-crypto", "w", &MMChannel::send_sf_material, "send-file-info"},
    {"send-file-info", "w", &MMChannel::send_file_info_server, "wait-for-ok"},
    {"wait-for-ok", "r", &MMChannel::check_file_response, "send-file-data", "finalize-reject"},
    {"send-file-data", "w", &MMChannel::send_chunk, "end-xfer"},
    {"end-xfer", "r", &MMChannel::wait_for_ok, "end"},
    {"finalize-reject", "i", &MMChannel::send_rej_ack, "end"},
    {0}
};

// this is where we associate an incoming channel by looking at the
// initial command regarding what it wants to do.
// it will find the channel association, and actually create new
// protocol objects depending on the initial command.
struct strans recv_command[] = {
    {"get-what-to-do", "r", &MMChannel::get_what_to_do, "stop"},
    {0}
};

// this is protocol for receiving an attachment directly
static struct strans recv_attachment[] = {
    {"start", "i", &MMChannel::accept_att, "send-ok", "send-reject"},
    {"send-ok", "w", &MMChannel::send_recv_ok, "recv-data"},
    {"recv-data",  "r", &MMChannel::recv_chunk, "final-ack"},
    {"final-ack", "w", &MMChannel::send_ok, "end"},
    {"send-reject", "w", &MMChannel::send_recv_reject, "end"},
    {0}
};

// this is for connecting to a server to pull a file
struct strans recv_attachment_pull[] = {
    {"start", "i", &MMChannel::init_connect, "send-crypto"},
    {"send-crypto", "w", &MMChannel::send_sf_material, "send-file-info"},
    {"send-file-info", "w", &MMChannel::send_pull_info, "wait-for-ok"},
    {"wait-for-ok", "r", &MMChannel::recv_pull_info, "send-ok"},
    {"send-ok", "w", &MMChannel::send_recv_ok, "recv-data"},
    {"recv-data",  "r", &MMChannel::recv_chunk, "final-ack"},
    {"final-ack", "w", &MMChannel::send_ok, "end"},
    {0}
};

struct strans media_x_setup[] = {
    {"start", "i", &MMChannel::init_connect_media, "send-media-id"},
    {"send-media-id", "w", &MMChannel::send_media_id, "wait-for-ok"},
    {"wait-for-ok", "r", &MMChannel::check_media_response, "ping", "send-chal"},
    {"send-chal", "w", &MMChannel::send_sync_challenge, "wait-resp"},
    {"wait-resp", "r", &MMChannel::check_sync_challenge_response, "wait-chal"},
    {"wait-chal", "r", &MMChannel::wait_for_sync_challenge, "send-resp"},
    {"send-resp", "w", &MMChannel::send_sync_resp, "send-delta"},
    {"send-delta", "w", &MMChannel::send_delta, "wait-for-delta"},
    {"wait-for-delta", "r", &MMChannel::wait_for_delta, "sync-ready"},
    {"sync-ready", "i", &MMChannel::sync_ready, "ping"},
    {"ping", "t", 0, "send-ping"},
    {"send-ping", "w", &MMChannel::send_media_ping, "ping"},
    {0}
};

// note: updated this protocol to send pings back. this keeps
// proxies and routers that treat incoming and outgoing
// mappings differently (ie, one-sided pings coming from the
// initiator of the connection may not be enough for some
// proxies if they have a timeout for each direction of traffic.
// note: we don't to "ping-pong" on these channels because old
// software doesn't understand it, but normally, the ping-pong
// going on in the associated control channel would probably
// take care of that here. the main reason we do the pinging at
// all here is to keep NAT routers from timing out our mappings.
static struct strans media_r_setup[] = {
    {"start", "w", &MMChannel::send_media_ok, "ping", "wait-chal"},
    {"wait-chal", "r", &MMChannel::wait_for_sync_challenge, "send-resp"},
    {"send-resp", "w", &MMChannel::send_sync_resp, "send-chal"},
    {"send-chal", "w", &MMChannel::send_sync_challenge, "wait-resp"},
    {"wait-resp", "r", &MMChannel::check_sync_challenge_response, "send-delta"},
    {"send-delta", "w", &MMChannel::send_delta, "wait-for-delta"},
    {"wait-for-delta", "r", &MMChannel::wait_for_delta, "sync-ready"},
    {"sync-ready", "i", &MMChannel::sync_ready, "ping"},
    {"ping", "t", 0, "send-ping"},
    {"send-ping", "w", &MMChannel::send_media_ping, "ping"},
    {0}
};

// send file side
int
MMChannel::init_connect(int subchan, sproto *p, const char *ev)
{
    int i = tube->gen_channel(0, subchan);
    file_xfer = 1;
    xfer_failed = 1;
    if(i == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    if(i == 1)
    {
        connect_failed = 0;
        if(chan_established_callback)
            (*chan_established_callback)(this, subchan, cec_arg1, cec_arg2, cec_arg3);
        set_progress_status("Requesting...");
        return sproto::next;
    }
    oopanic("bad gen_channel");
    return sproto::fail;
}

int
MMChannel::init_connect_media(int subchan, sproto *p, const char *ev)
{
    int i = tube->gen_channel(0, subchan);
    if(i == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    if(i == 1)
    {
        connect_failed = 0;
        if(chan_established_callback)
            (*chan_established_callback)(this, subchan, cec_arg1, cec_arg2, cec_arg3);
        set_progress_status("Requesting media...");
        if(subchan == audio_chan)
            audio_state = i;
        else if(subchan == video_chan)
            video_state = i;
        else if(subchan == msync_chan)
            msync_state = i;

        return sproto::next;
    }
    oopanic("bad gen_channel");
    return sproto::fail;

}

int
MMChannel::send_media_id(int subchan, sproto *p, const char *ev)
{
    vc v;
    v = vc(VC_VECTOR);
    if(subchan == audio_chan)
    {
        v[0] = "audio";
    }
    else if(subchan == video_chan)
    {
        v[0] = "video";
    }
    else if(subchan == msync_chan)
    {
        v[0] = "msync";
    }
    else
        oopanic("bad media protocol");
    v[1] = session_id;

    int i;
    if((i = tube->send_data(v, subchan)) == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    return sproto::next;

}

int
MMChannel::send_file_info(int subchan, sproto *p, const char *ev)
{
    vc v(VC_VECTOR);

    vc v2(VC_VECTOR);
    DwString f2 = dwbasename((const char *)remote_filename);
    v2[0] = f2.c_str();
    if(p->fn_to_send.length() == 0)
        p->fn_to_send = newfn(remote_filename);

    struct stat s;
    if(stat(p->fn_to_send.c_str(), &s) != 0)
    {
        v2[1] = 0;
        expected_size = 0;
        return sproto::fail;
    }
    else
    {
        v2[1] = (long)s.st_size;
        expected_size = s.st_size;
    }
    if(zap_send)
        v[0] = "zfile";
    else
        v[0] = "file";
    v[1] = v2;
    v[2] = session_id;

    int i;
    if((i = tube->send_data(v, subchan)) == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    if(v[0] == vc("zfile") && !agreed_key.is_nil())
    {
        tube->set_key_iv(agreed_key, 0);
        tube->start_encrypt_chan(subchan);
        tube->start_decrypt_chan(subchan);
    }
    return sproto::next;
}

int
MMChannel::send_file_info_server(int subchan, sproto *p, const char *ev)
{
    vc v(VC_VECTOR);

    vc v2(VC_VECTOR);
    DwString f2 = dwbasename((const char *)remote_filename);
    v2[0] = f2.c_str();

    if(p->fn_to_send.length() == 0)
        p->fn_to_send = newfn(remote_filename);

    struct stat s;
    if(stat(p->fn_to_send.c_str(), &s) != 0)
    {
        v2[1] = 0;
        expected_size = 0;
        return sproto::fail;
    }
    else
    {
        v2[1] = (long)s.st_size;
        expected_size = s.st_size;
    }

    if(zap_send)
        v[0] = "zfile";
    else
        v[0] = "file";
    v[1] = v2;
    v[2] = session_id;
    v[3] = My_UID;
    v[4] = My_MID;

    int i;
    if((i = tube->send_data(v, subchan)) == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    return sproto::next;
}

int
MMChannel::send_sf_material(int subchan, sproto *p, const char *ev)
{
    vc v;
    vc key;
    if(p->user_info.is_nil())
    {
        v = dh_store_and_forward_material(Server_publics, key);
        p->user_info = key;
    }
    // note: odd issue here with networking stuff. if it says "try again"
    // what it really did was take a copy of what you first gave it, and
    // stored it away, and the next time we come in here, we are basically
    // pushing out the stuff we originally sent in.
    // since dh_store_and_forward_material gives us a new session key for
    // each call, we have to store what we sent and not recompute it on each
    // try.
    // this problem would be fixed if we had a network layer that didn't
    // store things like that... or a call to dh.. that took the key to use

    int i;
    if((i = tube->send_data(v, subchan)) == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    if(!p->user_info.is_nil())
    {
        agreed_key = p->user_info;
        tube->set_key_iv(agreed_key, 0);
        tube->start_encrypt_chan(subchan);
        tube->start_decrypt_chan(subchan);
    }
    return sproto::next;

}

int
MMChannel::check_file_response(int subchan, sproto *p, const char *ev)
{
    vc rvc;
    int i;

    if((i = tube->recv_data(rvc, subchan)) == SSERR)
    {
        return sproto::fail;
    }
    if(i == SSTRYAGAIN)
        return sproto::stay;

    if(rvc.type() == VC_VECTOR)
    {
        if(rvc[0] != vc("restart"))
            return sproto::fail;
        seekto = (off_t)(long)rvc[1];
        return sproto::stay;
    }
    if(rvc == vc("zreject"))
    {
        zreject = 1;
        return sproto::alt_next;
    }

    if(rvc != vc("fileok"))
        return sproto::fail;

    total_got = seekto;

    // user_info should be pre-prefixed version of file we are going to send.

    read_fd = fopen(p->fn_to_send.c_str(), "rb");
    if(read_fd == 0 || fseek(read_fd, seekto, SEEK_SET) != 0)
    {
        return sproto::fail;
    }
    return sproto::next;
}

int
MMChannel::check_media_response(int subchan, sproto *p, const char *ev)
{
    vc rvc;
    int i;

    if((i = tube->recv_data(rvc, subchan)) == SSERR)
    {
        return sproto::fail;
    }
    if(i == SSTRYAGAIN)
        return sproto::stay;
    if(rvc == vc("video ok"))
    {
        p->watchdog.stop();
        video_state = MEDIA_SESSION_UP;
        p->timeout.load(VIDEO_IDLE_TIMEOUT);
        p->timeout.set_oneshot(1);
        p->timeout.start();
        if(!agreed_key.is_nil())
        {
            tube->set_key_iv(agreed_key, 0);
            tube->start_decrypt_chan(subchan);
            tube->start_encrypt_chan(subchan);
        }
        return sproto::next;

    }
    else if(rvc == vc("audio ok"))
    {
        p->watchdog.stop();
        audio_state = MEDIA_SESSION_UP;
        p->timeout.load(AUDIO_IDLE_TIMEOUT);
        p->timeout.set_oneshot(1);
        p->timeout.start();
        if(!agreed_key.is_nil())
        {
            tube->set_key_iv(agreed_key, 0);
            tube->start_decrypt_chan(subchan);
            tube->start_encrypt_chan(subchan);
        }
        return sproto::next;

    }
    else if(rvc == vc("msync ok"))
    {
        // NOTE: make sure timeout is right for syncing
        p->timeout.load(AUDIO_IDLE_TIMEOUT);
        p->timeout.set_oneshot(1);
        p->timeout.start();
        if(!agreed_key.is_nil())
        {
            tube->set_key_iv(agreed_key, 0);
            tube->start_decrypt_chan(subchan);
            tube->start_encrypt_chan(subchan);
        }
        return sproto::alt_next;

    }
    return sproto::fail;

}

int
MMChannel::send_sync_challenge(int subchan, sproto *p, const char *ev)
{
    if(!Current_alternate)
        return sproto::fail;
    vc v;
    vc key;
    if(p->user_info.is_nil())
    {
        vc nonce;
        v = Current_alternate->gen_challenge_msg(nonce);
        p->user_info = nonce;
    }
    // note: odd issue here with networking stuff. if it says "try again"
    // what it really did was take a copy of what you first gave it, and
    // stored it away, and the next time we come in here, we are basically
    // pushing out the stuff we originally sent in.
    // since dh_store_and_forward_material gives us a new session key for
    // each call, we have to store what we sent and not recompute it on each
    // try.
    // this problem would be fixed if we had a network layer that didn't
    // store things like that... or a call to dh.. that took the key to use

    int i;
    if((i = tube->send_data(v, subchan)) == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    return sproto::next;
}

int
MMChannel::check_sync_challenge_response(int subchan, sproto *p, const char *ev)
{
    vc rvc;
    int i;

    if((i = tube->recv_data(rvc, subchan)) == SSERR)
    {
        return sproto::fail;
    }
    if(i == SSTRYAGAIN)
        return sproto::stay;

    if(p->user_info == rvc)
    {
        p->user_info = vcnil;
        return sproto::next;
    }
    return sproto::fail;
}

int
MMChannel::send_delta(int subchan, sproto *p, const char *ev)
{
    if(!Current_alternate)
        return sproto::fail;

    vc delta_id = get_delta_id(remote_uid());
    vc vcx(VC_VECTOR);
    vcx[0] = "delta";
    vcx[1] = delta_id;

    int i;
    if((i = tube->send_data(vcx, subchan)) == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    return sproto::next;

}

int
MMChannel::wait_for_delta(int subchan, sproto *p, const char *ev)
{
    vc rvc;
    int i;

    if((i = tube->recv_data(rvc, subchan)) == SSERR)
    {
        return sproto::fail;
    }
    if(i == SSTRYAGAIN)
        return sproto::stay;

    if(rvc[0] != vc("delta"))
    {
        return sproto::fail;
    }
    mmr_sync_state = RECV_INIT;
    if(rvc[1].is_nil() || !generate_delta(remote_uid(), rvc[1]))
    {
        // couldn't generate a delta, just tell it to send a full index
        mms_sync_state = SEND_INIT;
        return sproto::next;
    }
    mms_sync_state = SEND_DELTA_OK;
    return sproto::next;
}


int
MMChannel::sync_ready(int subchan, sproto *p, const char *ev)
{
    p->watchdog.stop();
    msync_state = MEDIA_SESSION_UP;
    //mms_sync_state = WAIT_FOR_DELTA;
    //mmr_sync_state = SEND_DELTA;
    return sproto::next;
}

int
MMChannel::send_chunk(int subchan, sproto *p, const char *ev)
{
    char buf[2048];
    int len;

    if(read_fd == 0)
        return sproto::fail;
    if(feof(read_fd))
    {
        len = 0;
    }
    else
        len = fread(buf, 1, sizeof(buf), read_fd);
    vc vcbuf;
    if(len > 0)
    {
        vcbuf = vc(VC_BSTRING, buf, len);
    }
    int ret;
    if((ret = tube->send_data(vcbuf, subchan)) == SSERR)
    {
        set_progress_abort("remote end failed");

        fclose(read_fd);
        read_fd = 0;
        return sproto::fail;
    }
    if(ret == SSTRYAGAIN)
    {
        if(fseek(read_fd, -len, SEEK_CUR) != 0)
        {
            set_progress_abort("file operation problem");
            fclose(read_fd);
            read_fd = 0;
            return sproto::fail;
        }
        return sproto::stay;
    }
    else
    {
        // success
        p->rearm_watchdog();
        set_progress_got(len);
        // if we sent a nil, we are done.
        if(len == 0)
        {
            fclose(read_fd);
            read_fd = 0;
            return sproto::next;
        }
    }
    // stay in current state and keep sending
    return sproto::stay;
}

int
MMChannel::send_rej_ack(int subchan, sproto *p, const char *ev)
{

    int i;
    if((i = tube->send_data(vc("rejectok"), subchan)) == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    schedule_destroy(HARD);
    return sproto::next;
}

int
MMChannel::wait_for_ok(int subchan, sproto *p, const char *ev)
{
    vc rvc;
    int i;

    if((i = tube->recv_data(rvc, subchan)) == SSERR)
    {
        return sproto::fail;
    }
    if(i == SSTRYAGAIN)
        return sproto::stay;
    if(rvc == vc("frok"))
    {
        // file xfers are started inside dedicated channels
        xfer_failed = 0;
        schedule_destroy(HARD);
        return sproto::next;
    }
    return sproto::fail;

}

int
MMChannel::get_what_to_do(int subchan, sproto *p, const char *ev)
{
    int i;
    vc rvc;

    if((i = tube->recv_data(rvc, subchan)) == SSERR)
    {
        return sproto::fail;
    }
    if(i == SSTRYAGAIN)
        return sproto::stay;
    if(rvc.type() == VC_VECTOR)
    {
        if(rvc[0] == vc("zfile"))
        {
            // note file receives are associated with another channel rather than
            // stand-alone. we don't get notification that the file is received directly.
            // rather, we wait until a message is received that references the file attachment.
            if(!rvc[2].is_nil())
            {
                // firewall friendly case, v[2] is
                // session id we are with.
                int sid = (long)rvc[2];
                MMChannel *pmc = get_channel_by_session_id(sid);
                if(!pmc)
                {
                    return sproto::fail;
                }
                int nc = pmc->tube->find_chan();
                pmc->tube->set_channel(tube->get_chan_sock(subchan),
                                       tube->get_encrypt_chan(subchan),
                                       tube->get_decrypt_chan(subchan), nc);
                tube->set_channel(0, 0, 0, subchan);

                MMChannel *newchan = new MMChannel;
                newchan->remote_cfg = pmc->remote_cfg;
                // this is horrifying, two channels referencing the same tube
                newchan->tube = pmc->tube;
                newchan->init_config(0);
                newchan->recv_matches(pmc->remote_cfg);
                //newchan->file_xferchan = nc;
                newchan->zap_recv = 1;
                newchan->file_xfer = 1;
                pmc->simple_protos[nc] = 0;

                //newchan->protos[nc] = CHAN_OK;
                newchan->agreed_key = pmc->agreed_key;
                if(!newchan->agreed_key.is_nil())
                {
                    newchan->tube->set_key_iv(newchan->agreed_key, 0);
                    newchan->tube->start_decrypt_chan(nc);
                    newchan->tube->start_encrypt_chan(nc);
                }

                sproto *s = new sproto(nc, recv_attachment, newchan->vp);
                newchan->simple_protos[nc] = s;
                s->user_info = rvc[1]; // file info
                s->start();
                newchan->start_service();
                schedule_destroy(HARD);
                return sproto::next;
            }
            else
                oopanic("unimp");

        }
        else if(rvc[0] == vc("audio") || rvc[0] == vc("video") ||
                rvc[0] == vc("msync"))
        {
            int sid = (int)rvc[1];
            MMChannel *mc = get_channel_by_session_id(sid);
            if(!mc)
            {
                schedule_destroy(HARD);
                return sproto::fail;
            }

            // kluge: take the tube out of the
            // current channel. put it into the
            // associated channel. then destroy ourselves.
            int nc = mc->tube->find_chan();
            mc->tube->set_channel(tube->get_chan_sock(subchan),
                                  tube->get_encrypt_chan(subchan),
                                  tube->get_decrypt_chan(subchan), nc);
            if(rvc[0] == vc("audio"))
            {
                mc->audio_chan = nc;
                mc->audio_state = MEDIA_SESSION_SETUP;
            }
            else if(rvc[0] == vc("video"))
            {
                mc->video_chan = nc;
                mc->video_state = MEDIA_SESSION_SETUP;
            }
            else if(rvc[0] == vc("msync"))
            {
                mc->msync_chan = nc;
                mc->msync_state = MEDIA_SESSION_SETUP;
            }
            tube->set_channel(0, 0, 0, subchan);
            sproto *s = new sproto(nc, media_r_setup, mc->vp);
            s->user_info = rvc;
            mc->simple_protos[nc] = s;
            s->start();
            schedule_destroy(HARD);
            return sproto::next;

        }
    }
    else if(rvc.type() == VC_STRING)
    {
        oopanic("hmmm, is this used any more");
    }
    return sproto::fail;

}

int
MMChannel::accept_att(int subchan, sproto *p, const char *ev)
{
    xfer_failed = 1;
    vc u;

    if(!remote_cfg.is_nil() && remote_cfg.find("my uid", u))
    {
        if(uid_ignored(u))
        {
            return sproto::alt_next;
        }
    }
    vc remote_fn = p->user_info[0];
    // compat, for now. these are in the channel and will be overwritten
    // if we ever transfer more than one file at a time.
    remote_filename = remote_fn;
    expected_size = p->user_info[1];

    if(!is_attachment(remote_fn))
        return sproto::alt_next;
    DwString actual_fn = newfn(remote_fn);
    write_fd = fopen(actual_fn.c_str(), "wb");
    if(!write_fd)
    {
        if(popup_message_box_callback)
            (*popup_message_box_callback)(this, "Problems opening local file (maybe file is read-only?)", MB_OK, vcnil);
        return sproto::fail;
    }

    return sproto::next;
}

int
MMChannel::send_pull_info(int subchan, sproto *p, const char *ev)
{
    // note: in the "get" case, we are always
    // contacting the server, and so we don't
    // need a "firewall_friendly" case

    // if we already have a file of this name tell it
    // to restart the fetch at the offset.
    //
    // note: this "xfer" directory stuff is mainly to protect
    // partially received files from the auto-cleanup thing.
    // since there is no reference to the received attachment
    // (except on the server, and that message isn't stored
    // on disk until the entire attachment is received.)
    // so this is a kluge, when the entire attachment is
    // received, we move it up to where it would be normally
    // for further processing.)
    // if we don't do this, the autocleanup thing will
    // remove the partially transferred file.
    //
    DwString actual_fn = prepend_pfx("xfer", remote_filename);
    FILE *fd = fopen(actual_fn.c_str(), "rb");
    off_t offset = 0;
    if(fd)
    {
        if(fseeko(fd, 0, SEEK_END) == 0)
        {
            offset = ftello(fd);
            if(offset == -1)
                offset = 0;
            if(offset > 0)
            {
                TRACK_ADD(MC3_pull_restart, 1);
            }
        }
        fclose(fd);
    }
    vc v(VC_VECTOR);
    write_fd_offset = offset;
    v[0] = remote_filename;
    v[1] = (long)write_fd_offset;
    v[2] = My_UID;
    v[3] = My_MID;

    int i;
    if((i = tube->send_data(v, subchan)) == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    return sproto::next;
}

int
MMChannel::recv_pull_info(int subchan, sproto *p, const char *ev)
{
    int i;
    vc v;

    if((i = tube->recv_data(v, subchan)) == SSERR)
    {
        return sproto::fail;
    }
    if(i == SSTRYAGAIN)
        return sproto::stay;

    if(!(v.type() == VC_VECTOR && v[0].type() == VC_STRING && v[1].type() == VC_VECTOR &&
            v[1][0].type() == VC_STRING && v[1][1].type() == VC_INT))
        return sproto::fail;
    if(v[0] != vc("sfile"))
        return sproto::fail;

    //file_xferchan = i;
    total_got = 0;
    expected_size = (int)v[1][1];
    set_string_id("<<Zap msg recv>>");
    DwString actual_fn = prepend_pfx("xfer", remote_filename);
    write_fd = fopen(actual_fn.c_str(), "ab");

    if(!write_fd || fseeko(write_fd, write_fd_offset, SEEK_SET) != 0)
    {
        if(popup_message_box_callback)
            (*popup_message_box_callback)(this, "Problems opening local file (maybe you are out of disk space)", MB_OK, vcnil);
        if(write_fd)
        {
            fclose(write_fd);
            write_fd = 0;
        }
        return sproto::fail;
    }
    total_got = write_fd_offset;
    p->recv_fn = actual_fn;
    return sproto::next;
}

int
MMChannel::recv_chunk(int subchan, sproto *p, const char *ev)
{
    int ret;
    vc vcbuf;

    if(write_fd == 0)
    {
        return sproto::fail;
    }

    if((ret = tube->recv_data(vcbuf, subchan)) == SSERR)
    {
        set_progress_abort("remote end failed");
        fclose(write_fd);
        write_fd = 0;
        return sproto::fail;
    }
    if(ret != SSTRYAGAIN)
    {
        if(vcbuf.is_nil())
        {
            if(fclose(write_fd) != 0)
            {
                set_progress_abort("can't close file (out of disk space?)");
                write_fd = 0;
                return sproto::fail;
            }
            else if(total_got != expected_size)
            {
                set_progress_abort("received file size wrong");
                write_fd = 0;
                return sproto::fail;
            }

            set_progress_status("Sending final ack...");
            // send back an indicator that we actually
            // committed the file (as much as we can
            // anyway. this helps fix a problem where the
            // receiver might think it actually sent us
            // a file successfully, when in fact he just sent all the
            // bytes to his end of the socket, and we
            // errored out and closed on him.

            write_fd = 0;
            return sproto::next;

        }
        else if(vcbuf.type() != VC_STRING)
        {
            set_progress_abort("remote end failed");
            fclose(write_fd);
            write_fd = 0;
            return sproto::fail;
        }

        if(fwrite((const char *)vcbuf, vcbuf.len(), 1, write_fd) != 1)
        {
            set_progress_abort("can't write file (out of disk space?)");
            fclose(write_fd);
            write_fd = 0;
            return sproto::fail;
        }

        set_progress_got(vcbuf.len());
        p->rearm_watchdog();
    }
    // stay in current state
    return sproto::stay;

}

int
MMChannel::send_media_ok(int subchan, sproto *p, const char *ev)
{
    int i;

    vc media = p->user_info[0];
    DwString a = (const char *)media;
    a += " ok";
    if((i = tube->send_data(a.c_str(), subchan)) == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    //p->watchdog.stop();
    if(media == vc("audio"))
    {
        p->watchdog.stop();
        p->timeout.load(AUDIO_IDLE_TIMEOUT);
        p->timeout.set_oneshot(1);
        p->timeout.start();
        audio_state = MEDIA_SESSION_UP;
    }
    else if(media == vc("video"))
    {
        p->watchdog.stop();
        p->timeout.load(VIDEO_IDLE_TIMEOUT);
        p->timeout.set_oneshot(1);
        p->timeout.start();
        video_state = MEDIA_SESSION_UP;
    }
    else if(media == vc("msync"))
    {
        p->timeout.load(AUDIO_IDLE_TIMEOUT);
        p->timeout.set_oneshot(1);
        p->timeout.start();
        tube->set_key_iv(agreed_key, 0);
        tube->start_encrypt_chan(subchan);
        tube->start_decrypt_chan(subchan);
        return sproto::alt_next;
    }
    tube->set_key_iv(agreed_key, 0);
    tube->start_encrypt_chan(subchan);
    tube->start_decrypt_chan(subchan);
    return sproto::next;
}

int
MMChannel::wait_for_sync_challenge(int subchan, sproto *p, const char *ev)
{
    if(!Current_alternate)
        return sproto::fail;
    int i;
    vc v;

    if((i = tube->recv_data(v, subchan)) == SSERR)
    {
        return sproto::fail;
    }
    if(i == SSTRYAGAIN)
        return sproto::stay;
    vc resp;
    int ret = Current_alternate->challenge_recv(v, resp);
    if(ret == 0)
        return sproto::fail;
    p->user_info = resp;
    return sproto::next;
}

int
MMChannel::send_sync_resp(int subchan, sproto *p, const char *ev)
{
    int i;
    if((i = tube->send_data(p->user_info, subchan)) == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    p->user_info = vcnil;
    return sproto::next;
}

int
MMChannel::send_media_ping(int subchan, sproto *p, const char *ev)
{
    int i;

    vc v(VC_VECTOR);
    v[0] = "!";
    // so we don't get stuck in this state
    p->rearm_watchdog();
    if((i = tube->send_data(v, subchan)) == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    // note: this happens on both audio and video channels, but
    // the timeout is the same
    p->timeout.load(VIDEO_IDLE_TIMEOUT);
    p->timeout.start();
    p->watchdog.stop();
    return sproto::next;
}

int
MMChannel::send_recv_ok(int subchan, sproto *p, const char *ev)
{
    int i;
    if((i = tube->send_data(vc("fileok"), subchan)) == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    return sproto::next;
}

int
MMChannel::send_recv_reject(int subchan, sproto *p, const char *ev)
{
    int i;
    if((i = tube->send_data(vc("zreject"), subchan)) == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    return sproto::next;
}

int
MMChannel::send_ok(int subchan, sproto *p, const char *ev)
{
    int i;
    if((i = tube->send_data(vc("frok"), subchan)) == SSERR)
        return sproto::fail;
    if(i == SSTRYAGAIN)
        return sproto::stay;
    if(p->recv_fn.length() > 0)
    {
        if(!move_replace(p->recv_fn, newfn(remote_filename)))
        {
            DeleteFile(p->recv_fn.c_str());
            return sproto::fail;
        }
    }

    xfer_failed = 0;
    schedule_destroy();
    return sproto::next;
}
