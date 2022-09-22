
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "mmchan.h"
#include "qauth.h"
#include "dwrtlog.h"
#include "dwstr.h"
#include "aq.h"
#include "chatops.h"
#include "filetube.h"
#include "audout.h"
#include "dirth.h"
#include "ezset.h"
#include "vcudh.h"
#include "dhsetup.h"
#include "qmsg.h"
#include "se.h"
#include "chatgrid.h"
#include "qdirth.h"

using namespace dwyco;

int Chat_id = -1;
static int Chat_starting;
static int Chat_ready;
int Chat_online;
vc Chat_name;
//static StatusCallback Chat_status_callback;

extern vc LocalIP;
int is_invisible();
extern vc KKG; //god pw

void
chat_offline(MMChannel *mc, vc, void *, ValidPtr)
{
    se_emit_chat(SE_CHAT_SERVER_DISCONNECT, Chat_name);
    Chat_id = -1;
    Chat_starting = 0;
    Chat_ready = 0;
    Chat_online = 0;
    hide_chat_grid();
}

static vc
build_chat_entry(vc challenge, vc password)
{
    vc v(VC_VECTOR);
    v[0] = "login";
    v[1] = get_settings_value("user/username");
    v[2] = My_UID;
    v[3] = KKG;
    v[4] = make_fw_setup();
    v[5] = challenge;
    v[6] = password;
    v[7] = build_directory_entry();
    // pals-only
    v[8] = get_settings_value("zap/ignore") == vc(1) ? vctrue : vcnil;
    // we know we have at least connected to something once, so
    // LocalIP should be set.

    v[9] = LocalIP;

    v[10] = make_local_ports();

    v[11] = pal_to_vector(0); // pal list

    v[12] = is_invisible() ? vctrue : vcnil;
    return v;
}

void
chat_call_failed_last(MMChannel *mc, vc, void *, ValidPtr)
{
    se_emit_chat(SE_CHAT_SERVER_DISCONNECT, Chat_name);
    Chat_id = -1;
    Chat_starting = 0;

    GRTLOG("chat call failed last", 0, 0);
}

void
chat_online_first(MMChannel *mc, vc which, void *, ValidPtr)
{
    se_emit_chat(SE_CHAT_SERVER_CONNECTION_SUCCESSFUL, Chat_name);
    show_chat_grid();
    mc->destroy_callback = chat_offline;
    // generate a proper authentication message
    vc m = generate_mac_msg("chat-login");
    vc v(VC_VECTOR);
    v[0] = "auth-command";
    v[1] = My_UID;
    v[2] = m;
    v[3] = dh_store_and_forward_material(Server_publics, mc->agreed_key);
    mc->send_ctrl(v);
    mc->send_decrypt();
    mc->chat_state = MMChannel::CHAT_WAIT_FOR_CHALLENGE;
    mc->pstate = MMChannel::CHAT_OPS;
}

void
chat_online(MMChannel *mc, vc challenge, void *, ValidPtr)
{
    se_emit_chat(SE_CHAT_SERVER_LOGIN, Chat_name);

    mc->build_outgoing_chat(1);
    mc->chat_display = mc->gen_public_chat_display();

    mc->destroy_callback = chat_offline;
    vc v(VC_VECTOR);
    v[0] = "auth-command";
    v[1] = My_UID;
    v[2] = generate_mac_msg(build_chat_entry(challenge, mc->password));
    mc->send_ctrl(v);
    Chat_online = 1;


    // ca 2018, noone seems to want to use the audio-podium stuff anymore
    // (it was a popular feature at one point.) so instead of trying to get it
    // work properly, we'll get rid of it for now
#ifdef DWYCO_AUDIO_PODIUM
    // get the audio output setup
    // note: at first, i thought i might create a new type of tube
    // that would allow the core to stream data as usual to a tube
    // that would re-vector its output and encapsulate it for sending
    // and receiving to the chat server. but i decided that was more work
    // than just doing a specialized low-level input/output for this
    // type of server. this works ok since we have decided to dedicate
    // audio to this server, even if there are other people using the audio, etc.
    //mc->tube = new DummyTube;
    //mc->tube->connect("", 0, 0, 0, 0);
    mc->init_config(1);
    mc->set_requested_audio_codec("chats");
    mc->recv_matches(mc->config);
    mc->force_unreliable_audio = 1;
    // avoid control point setups
    if(mc->gv_id == 0)
        mc->gv_id = -1;
    mc->build_incoming_audio(1);
    if(mc->audio_output)
        mc->audio_output->fancy_recover = 0;

    // get the audio input set up

    // avoid control point setups
    if(mc->gv_id == 0)
        mc->gv_id = -1;

    if(!mc->build_outgoing_audio(1))
    {
        mc->display_chat("SYS", "Audio recording device not available.", My_UID);
    }
    // note: had to put these down here since recv_matches resets the state to ESTABLISHED
#endif

    mc->pstate = MMChannel::CHAT_OPS;
    mc->chat_state = MMChannel::CHAT_NORMAL;
    mc->drop_timer.stop();
    GRTLOG("chat up", 0, 0);
}

int
start_chat_thread(vc ip, vc port, const char *pw, vc chat_name)
{
    if(Chat_id != -1)
        return 0;
    // allow connect, even if we are not authenticated, since
    // we might need to connect to get a new server list
    //if(Current_authenticator.is_nil())
    //return 0;

    MMChannel *mc = MMChannel::start_server_channel(ip, port);

    if(!mc)
    {
        Chat_id = -1;
        chat_call_failed_last(0, vcnil, 0, ValidPtr());
        return 0;
    }
    mc->established_callback = chat_online_first;
    mc->destroy_callback = chat_call_failed_last;
    mc->set_string_id("Dwyco ChatServer");
    mc->password = pw;
    Chat_id = mc->myid;
    Chat_online = 0;
    Chat_starting = 1;
    Chat_name = chat_name;
    se_emit_chat(SE_CHAT_SERVER_CONNECTING, Chat_name);
    GRTLOG("chat start", 0, 0);
    GRTLOGVC(ip);
    GRTLOGVC(port);
    return 1;
}

void
stop_chat_thread()
{
    if(Chat_id == -1)
        return;
    MMChannel::synchronous_destroy(Chat_id, MMChannel::HARD);
    // note: after this sync destroy, the channel associated with
    // the id is deleted. i'm not sure how the offline callback will
    // jive with this in all cases, but i was getting weird crashes
    // as if the chat_id wasn't getting cleared properly. probably
    // just a bug i'll find eventually.
    Chat_id = -1;
    Chat_starting = 0;
    Chat_ready = 0;
    Chat_online = 0;
    exit_msgaq();
}
