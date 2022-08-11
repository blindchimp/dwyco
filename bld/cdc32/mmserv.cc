
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "mmchan.h"
#include "qdirth.h"
#include "qauth.h"
#include "qmsg.h"
#include "chatgrid.h"
#include "asshole.h"
#include "pgdll.h"
#include "audout.h"
#include "se.h"
#include "ta.h"
#include "profiledb.h"
#include "dwscoped.h"
#include "netlog.h"

using namespace dwyco;

extern vc Online;
extern vc Chat_ips;
extern vc Chat_ports;
void update_server_list(vc m, void *, vc, ValidPtr);

int
MMChannel::start_server_ops()
{
    pstate = SERVER_OPS;
    call_setup = 0;
    if(established_callback)
        (*established_callback)(this, ecb_arg1, ecb_arg2, ecb_arg3);
    established_callback_called = 1;
    return 1;
}

MMChannel *
MMChannel::get_server_channel()
{
    scoped_ptr<ChanList> cl(get_serviced_channels_server());
    ChanListIter cli(cl.get());
    MMChannel *mcs = 0;
    for(; !cli.eol(); cli.forward())
    {
        MMChannel *mc = cli.getp();
        //note: commented out "server_ops" check because
        // we want to return the channel even if it is
        // being setup (ie, hasn't connected yet.)
        if(mc->server_channel /*&& mc->pstate == SERVER_OPS*/ &&
                mc->do_destroy == KEEP)
        {
            mcs = mc;
            break;
        }
    }
    return mcs;
}

MMChannel *
MMChannel::get_secondary_server_channel()
{
    scoped_ptr<ChanList> cl(get_serviced_channels_server());
    ChanListIter cli(cl.get());
    MMChannel *mcs = 0;
    for(; !cli.eol(); cli.forward())
    {
        MMChannel *mc = cli.getp();
        if(mc->secondary_server_channel &&
                mc->do_destroy == KEEP)
        {
            mcs = mc;
            break;
        }
    }
    return mcs;
}

void
MMChannel::send_to_db(const QckMsg& m, int chan_id)
{
    MMChannel *mcs;
    mcs = MMChannel::channel_by_id(chan_id);
    if(!mcs)
    {
        vc bad_resp(VC_VECTOR);
        bad_resp.append(m.v[0]);
        bad_resp.append(vcnil);
        bad_resp.append("not connected to directory.");
        Response_q.append(bad_resp);
    }
    else
    {
        mcs->send_ctrl(m.v);
    }
}

void
MMChannel::server_response(vc v)
{
    static vc sync("c");
    GRTLOG("server resp %d", myid, 0);
    GRTLOGVC(v);
    if(v == sync)
    {
        vc s(VC_VECTOR);
        vc s2(VC_VECTOR);
        s2.append("sync");
        s2.append(0);
        s[0] = s2;
        Response_q.append(s);
    }
    else
    {
        Response_q.append(v);
    }
}

void
background_check_for_update_done(vc m, void *, vc, ValidPtr)
{
    if(m[1].is_nil())
    {
        return;
    }
    if(m[1].type() == VC_STRING && m[1] == vc("t"))
    {
        return;
    }
    if(m[1].type() != VC_VECTOR)
        return;
    // vector(desc type file version cur-hash new-hash vector(ip port))
    if(MMChannel::popup_update_box_callback)
        (*MMChannel::popup_update_box_callback)(0, m, vcnil, vcnil);
}


void
MMChannel::chat_response(vc v)
{
    static vc chat("chat");
    static vc chat_on("chat-on");
    static vc chat_off("chat-off");
    static vc currently_on("currently-on");
    static vc asshole_update("ah");
    static vc addq("addq");
    static vc delq("delq");
    static vc grant("grant");
    static vc data("data");
    static vc sysattr("us");
    static vc userattr("ua");
    static vc admin_info("admin-info");
    static vc server_list("server-list");
    static vc add_lobby("add-lobby");
    static vc del_lobby("del-lobby");
    static vc god_online("god-online");
    static vc god_offline("god-offline");
    static vc dbgreq("dbgreq");
    static vc invalidate_profile("invalidate-profile");
    static vc chatc("chatc");


    if(chat_state == CHAT_WAIT_FOR_CHALLENGE)
    {
        if(v.type() == VC_VECTOR && v[0] == vc("server-list"))
        {
            // this is an emergency server list, process it
            update_server_list(v, 0, vcnil, ValidPtr());
            return;
        }
        // call the next phase of the login procedure with the random
        // number we just got. this proves we logged into the main
        // server and aren't replaying some old crap into the server.
        chat_online(this, v, 0, ValidPtr());
        // note: the chat_online q's up the final part of the authentication
        // which, if it fails, these commands will be flushed.
        // if it succeeds, these will be sent and processed.
        dirth_send_get_server_list2(My_UID, QckDone(update_server_list, 0));
        dirth_send_check_for_update(My_UID, QckDone(background_check_for_update_done, 0));
        return;
    }
    if(v[0].type() == VC_VECTOR)
    {
        Response_q.append(v);
        return;
    }
    if(v[0] == chat)
    {
        display_chat(v[1], v[2], v[3]);
    }
    else if(v[0] == chatc)
    {
        // from chat server: (chan name data uid)
        if(TheChatGrid)
        {
            if(!uid_ignored(v[4]))
                TheChatGrid->simple_data(v[1], v[2], v[4], v[3]);
        }
    }
    else if(v[0] == chat_on)
    {
        GRTLOGVC(v);
        vc uid = v[2];
        vc name = v[1];
        vc ip = v[3];
        vc ah = v[4];
        vc ports = v[5];
        vc attrs = v[6];
#ifdef DWYCO_ASSHAT
        new_asshole(uid, ah);
#endif
        if(!uid_ignored(uid))
        {
            if(TheChatGrid)
            {
                TheChatGrid->start_update();
                TheChatGrid->add_user(name, uid);
                // attrs is a list of other attributes which we
                // simulate receiving messages for.
                int n = attrs.num_elems();
                for(int i = 0; i < n; ++i)
                {
                    TheChatGrid->update_attr(uid, attrs[i][0], attrs[i][1]);
                }

                TheChatGrid->end_update();
            }
            Chat_ports.add_kv(uid, ports);
            Chat_ips.add_kv(uid, strip_port(ip));

        }
    }
    else if(v[0] == chat_off)
    {
        vc uid = v[1];
        if(!uid_ignored(uid))
        {
            if(TheChatGrid)
            {
                TheChatGrid->start_update();
                TheChatGrid->remove_user(v[1]);
                TheChatGrid->end_update();
            }
        }
    }
    else if(v[0] == currently_on)
    {
        vc ul = v[1];
        GRTLOG("CURRENT", 0, 0);
        GRTLOGVC(ul);
        int n = v[1].num_elems();
        if(TheChatGrid)
            TheChatGrid->start_update();
        for(int i = 0; i < n; ++i)
        {
            vc uid = ul[i][0][1];
            vc name = ul[i][0][0];
            vc ports = ul[i][0][3];
            vc ah = ul[i][2];
            vc ip = ul[i][1];
            vc attrs = ul[i][3];
            GRTLOGVC(uid);
            GRTLOGVC(name);
            GRTLOGVC(ports);
            GRTLOGVC(ah);
            GRTLOGVC(ip);
            GRTLOGVC(attrs);
#ifdef DWYCO_ASSHAT
            new_asshole(uid, ah);
#endif
            if(!uid_ignored(uid))
            {
                if(TheChatGrid)
                {
                    TheChatGrid->add_user(name, uid);
                    int n = attrs.num_elems();
                    for(int i = 0; i < n; ++i)
                    {
                        TheChatGrid->update_attr(uid, attrs[i][0], attrs[i][1]);
                    }
                }
                Chat_ports.add_kv(uid, ports);
                Chat_ips.add_kv(uid, strip_port(ip));

            }
        }
        if(TheChatGrid)
            TheChatGrid->end_update();
    }
    else if(v[0] == asshole_update)
    {
#ifdef DWYCO_ASSHAT
        new_asshole(v[1], v[2]);
#endif
        if(TheChatGrid)
        {
            TheChatGrid->start_update();
            TheChatGrid->update_ah(v[1]);
            TheChatGrid->end_update();
        }
    }
    else if(v[0] == addq)
    {
        if(v[2] == My_UID)
        {
            granted[v[1]] = 0;
        }
        if(TheChatGrid)
        {
            TheChatGrid->addq(v[1], v[2], v[3]);
        }
    }
    else if(v[0] == delq)
    {
        if(v[2] == My_UID)
        {
            granted[v[1]] = 0;
        }
        if(TheChatGrid)
        {
            TheChatGrid->delq(v[1], v[2]);
        }
    }
    else if(v[0] == grant)
    {
        // make sure we tell the audio device a new timecode stream
        // may be coming
        if(audio_output)
            audio_output->switch_streams();
        if(v[2] == My_UID)
        {
            // we have been granted the podium on the q
            // we can start streaming audio or whatever to the
            // server.
            granted[v[1]] = 1;
        }
        else
        {
            //someone else is granted. just to make sure, we'll
            // ungrant ourselves
            granted[v[1]] = 0;
        }
        if(TheChatGrid)
        {
            TheChatGrid->grant(v[1], v[2], v[3]);
        }
    }
    else if(v[0] == data)
    {
#ifdef DWYCO_AUDIO_PODIUM
        if(v[2] != My_UID)
        {
            // shouldn't happen that the server sends us our own stuff,
            // or anything from somebody that hasn't been granted,
            // but we filter it out anyways
            // << send data to audio output device >>
            if(!uid_ignored(v[2]))
            {
                //note: this won't work unless we have called "build_incoming_audio"
                // on this MMChannel object, which i'm not sure can be done properly
                // if this is a server object.
                const char *buf = (const char *)v[3];
                int len = v[3].len();
                // the buffer ownership is being transferred to the audio subsystem, which is
                // why we do a copy
                DWBYTE *b = new DWBYTE[len];
                memmove(b, buf, len);

                int payload = *(int *)&b[len - sizeof(current_payload_type)];
                int timecode = *(int *)&b[len - sizeof(current_payload_type) - sizeof(audio_timecode)];
                process_incoming_audio2(b, len - sizeof(audio_timecode) - sizeof(current_payload_type),
                                        subchan(AUDIO_SUBCHANNEL, payload), timecode);
            }
        }
#endif

        if(TheChatGrid)
        {
            TheChatGrid->data(v[1], v[2], v[3]);
        }
    }
    else if(v[0] == sysattr)
    {
        GRTLOGVC(v);
        // form is: vector(us attr-name attr-value)
        if(TheChatGrid)
        {
            TheChatGrid->sys_attr(vc(""), v[1], v[2]);
        }
        // special processing, we set up the audio stuff at connect time
        // in order to get the device early. but we won't get the codec
        // we need to use until now, so we just tweak the payload type
        // accordingly.
        // WARNING: there is a typo here, it needs to be "us-audio-codeR-chats"
        // but this needs fixing in both server and here.
        if(v[1] == vc("us-audio-codec-chats"))
        {
            // NOTE: if streaming has already started, have to check if this
            // might cause glitches
            current_payload_type = str_to_payload_type(v[2]);
        }
        if(v[1] == vc("ui-global-block"))
        {
            // inhibit auto-reconnect to the database
            extern int Inhibit_auto_connect;
            Inhibit_auto_connect = 1;
        }
        if(v[1] == vc("us-ts"))
        {
            Track_stats = !v[2].is_nil();
        }
    }
    else if(v[0] == userattr)
    {
        // form is: vector(ua uid attr-name attr-value)
        GRTLOGVC(v);
        if(TheChatGrid)
        {
            TheChatGrid->update_attr(v[1], v[2], v[3]);
        }
    }
    else if(v[0] == admin_info)
    {
        if(TheChatGrid)
        {
            TheChatGrid->start_update();
            TheChatGrid->sys_attr(vc(""), "ui-demigod-list-clear", vcnil);
            TheChatGrid->sys_attr(vc(""), "ui-block-list-clear", vcnil);
            // v[1] is a vector of vectors
            vc aiv = v[1];
            int n = v[1].num_elems();
            for(int i = 0; i < n; ++i)
            {
                vc iv = aiv[i];
                if(iv[0] == vc("demigod"))
                {
                    TheChatGrid->sys_attr(vc(""), "ui-demigod-entry", iv[1]);
                }
                else if(iv[0] == vc("block"))
                {
                    TheChatGrid->sys_attr(vc(""), "ui-block-entry", iv[1]);
                }
            }
            TheChatGrid->end_update();
        }
    }
    else if(v[0] == server_list)
    {
        if(TheChatGrid)
        {
            TheChatGrid->start_update();
            // note: we get the entire "server list" from the
            // server, which includes things like the internal
            // service that are not directory servers. for compat,
            // i peel that out, since in the past "servers" just
            // referred to directory servers, and not all the other
            // stuff.
            TheChatGrid->sys_attr(vc(""), "ui-server-list", v[2][0]);
#if 0
            TheChatGrid->sys_attr(vc(""), "ui-server-list-clear", vcnil);
            // v[2] is a vector of vectors
            vc aiv = v[2];
            int n = v[2].num_elems();
            for(int i = 0; i < n; ++i)
            {
                vc iv = aiv[i];
                TheChatGrid->sys_attr(vc(""), "ui-server-list-entry", iv);
            }
#endif
            TheChatGrid->end_update();
        }
        // here is where we need *all* the servers, so they can be
        // broken out into their components and saved for use by
        // other parts of the DLL
        void update_server_list(vc m, void *, vc, ValidPtr);
        vc m(VC_VECTOR);
        m[1] = v[2];
        update_server_list(m, 0, vcnil, ValidPtr());
    }
    else if(v[0] == add_lobby)
    {
        if(TheChatGrid)
        {
            TheChatGrid->start_update();
            TheChatGrid->add_user_lobby(v[2]);
            TheChatGrid->end_update();
        }
    }
    else if(v[0] == del_lobby)
    {
        if(TheChatGrid)
        {
            TheChatGrid->start_update();
            TheChatGrid->del_user_lobby(v[2]);
            TheChatGrid->end_update();
        }
    }
    else if(v[0] == god_online)
    {
        GRTLOG("god on %s", (const char *)to_hex(v[1]), 0);
        GRTLOGVC(v[3]);
        if(TheChatGrid)
        {
            TheChatGrid->start_update();
            TheChatGrid->god_online(v[1], v[3]);
            TheChatGrid->end_update();
        }
    }
    else if(v[0] == god_offline)
    {
        GRTLOG("god off %s", (const char *)to_hex(v[1]), 0);
        if(TheChatGrid)
        {
            TheChatGrid->start_update();
            TheChatGrid->god_offline(v[1]);
            TheChatGrid->end_update();
        }

    }
    else if(v[0] == dbgreq)
    {
        // here is where we can turn on and off some debugging
        // related stuff in the client.
        if((int)v[1] == 1)
        {
            Track_stats = v[2];
        }
    }
    else if(v[0] == invalidate_profile)
    {
        GRTLOG("chats invalidate profile %s", (const char *)to_hex(v[1]), 0);
        prf_invalidate(v[1]);
        se_emit(SE_USER_PROFILE_INVALIDATE, v[1]);
    }

}

MMChannel *
MMChannel::start_server_channel(enum resolve_how how, unsigned long addr, const char *name, int port)
{
    MMChannel *m = new MMChannel;
    m->server_channel = 1;
    m->port = port;
    if(how == BYNAME)
    {
        if(!m->start_resolve(how, addr, name))
        {
            delete m;
            return 0;
        }
    }
    else
    {
        // start straight with the ip
        m->addr_out.s_addr = addr;
        if(!m->start_connect())
        {
            delete m;
            return 0;
        }
    }
    m->start_service();
    m->frame_timer.stop();
    m->ref_timer.stop();
    m->drop_timer.stop();
    m->fps_send.stop();
    m->fps_recv.stop();
    m->bps_send.stop();
    m->bps_recv.stop();
    m->bps_audio_send.stop();
    m->bps_audio_recv.stop();
    m->bps_file_xfer.stop();
    m->pinger_timer.stop();

    Netlog_signal.emit(m->tube->mklog("event", "server"));

    return m;
}

void
MMChannel::keepalive_processing()
{
    switch(pstate)
    {
    case SERVER_OPS:
    case CHAT_OPS:
        // note: we don't send pings to secondaries
        // since they are transient and we want them
        // to drop if no activity is going on.
        if(!secondary_server_channel)
            send_ctrl("!");
        break;
    default:; // do nothing
    }
}
