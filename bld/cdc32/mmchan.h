
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/mmchan.h 1.26 1999/01/10 16:10:50 dwight Checkpoint $
 */
#ifndef MMCHAN_H
#define MMCHAN_H
#include <stdio.h>
#include <stdint.h>
#ifndef _Windows
#include <netdb.h>
#include <arpa/inet.h>
#endif
#ifdef _MSC_VER
#include <sys/types.h>
#endif

#include "dwvecp.h"
#include "dwtimer.h"
#include "vc.h"
#include "dwrate.h"
#include "matcom.h"
#include "bcastq.h"
#include "dwstr.h"
//#include "audstats.h"
//#include "sd.h"
#include "syncvar.h"
#include "pval.h"
//#include "sd3.h"
#include "dwmapr.h"
#include "dlli.h"
#include "netvid.h"
#include "sproto.h"
#include "ssns.h"
#include "audconv.h"
#include "aconn.h"

class MMTube;
class VidAcquire;
class DwCoder;
class DwDecoder;
class MMChannel;
class ChatDisplay;
class KeyboardAcquire;
class AudioOutput;
class AudioAcquire;
class MessageDisplay;

namespace dwyco {
class DirectSend;
class DwQSend;
struct QckMsg;
}

typedef DwVecP<MMChannel> ChanList;
typedef DwVecPIter<MMChannel> ChanListIter;

typedef void (*DestroyCallback)(MMChannel *, vc, void *, ValidPtr);
typedef void (*StatusCallback)(MMChannel *, vc, void *, ValidPtr);
typedef int (*StatusCallback2)(MMChannel *, vc, void *);
typedef void (*StatusCallbackChan)(MMChannel *, int chan, vc, void *, ValidPtr);
typedef void (*CallAppearedCallback)(MMChannel *, DwString, int);
typedef ChatDisplay* (*ChatDisplayCallback)(MMChannel *);
typedef HWND (*GetWindowCallback)(MMChannel *);
typedef int (*UIPopupCallback)(MMChannel *, vc , vc , vc);
typedef int (*CallScreeningCallback)(MMChannel *,
                                     int remote_wants_to_recv_your_video, int remote_wants_to_send_you_video,
                                     int remote_wants_to_recv_your_audio, int remote_wants_to_send_you_audio,
                                     int remote_wants_to_exchange_pubchat, int remote_wants_to_exchange_privchat,
                                     const char *call_type, int len_call_type,
                                     const char *uid, int len_uid,
                                     int *accept_call_style,
                                     char **error_msg
                                    );
extern "C" int DWYCOCALLCONV dwyco_enable_video_capture_preview(int);
class MMChannel
{

public:
    // this is only here because of a bug in the
    // borland linker, seems calling "new" locally
    // causes it to take a crap on itself... sigh.
    static MMChannel *gen_chan();
    // initialized at create time so anyone that
    // caches pointers to these channels can
    // decide if one is still valid
    ValidPtr vp;

    // the tube is a full-duplex channel to
    // a remote channel. IO operations on
    // the tube are point-to-point. any
    // broadcasting must be done via
    // helper channels that are set up to
    // distribute pre-coded data.
    MMTube *tube;

    // the sampler is a device that can provide
    // uncoded data. if this member is null, the
    // channel produces no original coded video
    // (though, in the future it may be possible to
    // use channels as reflectors, but that isn't
    // possible now.)
    VidAcquire *sampler;

    // the coder member can be null, in which
    // case, the channel does no image/voice sampling or
    // or coding. the channel can pick up pre-coded
    // data from another channel by setting the
    // grab_coded_id member to the channel that is
    // producing the data. such channels are
    // helper channels that provide only comm services
    // and no coding/acquisition.
    DwCoder *coder;
    int grab_coded_id;
    int myid;

    // set to one to show frames even tho they
    // are not coded and sent
    static int Moron_dork_mode;
    DWBYTE *grab_and_code(int& len, unsigned long& captime, int& ref, int showonly);
    DWBYTE *real_grab_and_code(int& len, unsigned long& captime, int& ref, int showonly);

    DwDecoder *decoder;
    vc common_cfg;
    vc remote_cfg;
    vc config;

    static void turn_off_color();
    static int service_channels(int *spin_out = 0);
    void stop_service();
    void start_service();
    // note: safe to call this even if channel
    // has been whacked
    static int is_serviced(ValidPtr);
    void schedule_cancel();
    enum destroy_how {KEEP = 0, SOFT, HARD};
    void schedule_destroy(enum destroy_how = SOFT);

    // WARNING: be very careful using this in callbacks, as it is easy
    // to destroy your own MMChannel object
    static void synchronous_destroy(int id, enum destroy_how = HARD);

    MMChannel();
    ~MMChannel();

    int active();
    int send_ref();
    int send_frame();
    int tick();

    void send_ctrl(vc v);
    void ctrl_flush();

    // start connection protocol in caller
    void start_crypto();
    // start connection protocol in callee
    void wait_for_crypto();

private:
    // private protocol state handlers
    vc load_crypto();
    int crypto_agree(vc their_crypto, int caller);
    void recv_crypto(vc their_crypto);
    void recv_matching_crypto(vc their_crypto);

    void start_handshake();
    void wait_for_config();

    DwTimer nego_timer;
public:

    int build_outgoing(int locally_invoked = 0, int inhibit_coder_display = 0);
    int build_incoming_video();
#if 0
    int cfg_has_duplex(vc cfg, const char *what);
    int remote_recv();
    int local_recv();
    int remote_send();
    int local_send();
#endif
    vc remote_iam();
    unsigned short remote_listening_port();
    vc remote_username();
    vc remote_uid();
    vc remote_connection();
    int remote_session_id();
    vc remote_call_type();
    DwCoder *coder_from_config();
    DwDecoder *decoder_from_config();
    static MMChannel *already_connected(vc uid, int is_msg_chan = 0, int is_user_control_chan = 0);

    static int Sync_receivers;
    static int Auto_sync;
    static int compute_sync();

    static int some_coder_active();

private:
    // this vector holds the channels that are
    // currently being serviced by the main loop.
    // a channel can come and go from this vector,
    // no assumptions should be made about the
    // index of the channel in this vector.

    static DwVecP<MMChannel> MMChannels;	// list of serviced channels

    // this holds the channels that exist
    // and the key is the channel id.
    // once the id is set, it remains the same for the life
    // of the channel.

    static DwTreeKaz<MMChannel *, int> *AllChan2;

    // grayview that is used to display various
    // things about the channel
    int gv_id;

public:
    static MMChannel *find_xmitter(int return_dying = 0);
    static MMChannel *find_msg_xmitter();
    static int all_outgoing_paused();
    static int num_video_recvs();
    static int num_audio_recvs();
    static int num_audio_output_feeders();
    static int num_video_sends();
    static int num_audio_sends();
    static int num_chats();
    static int num_chats_private();
    static int num_calls();
    static MMChannel *channel_by_id(int);
    static int Session_id;
    static MMChannel *get_channel_by_session_id(int sid);

    static void clean_video_sampler_refs(VidAcquire *v);
    static void clean_audio_sampler_refs(AudioAcquire *v);
    static void clean_keyboard_sampler_refs(KeyboardAcquire *v);

private:

    int destroy();
    enum destroy_how do_destroy;
    int other_receivers();
    int other_receivers(int MMChannel::*);
    static int num_chans_with_tube(MMTube *);


    void init_config(int caller);
    vc match_config(vc, int caller);
    DwString match_notes;
    vc requested_config();

    void recv_config(vc);
    void recv_matches(vc);
    void recv_confirm(vc);
    void regular_control_procedures(vc);
    void regular_control_procedures2(vc);
    // this is newer-school, where it knows how to
    // use the same response matching used when
    // talking to a server.
    void process_with_response(vc);
    void send_error(vc err);

    int file_xfer; // 1  means it is a singleton file transfer channel

//XXX NASTY, need to encapsulate callbacks better so
// they can be friends of this class...
public:
    enum state {
        IDLE,
        // initial connection setup
        RESOLVING_NAME,
        CONNECTING,

        // post-connection, initial capability negotiation

        WAIT_FOR_CRYPTO,
        WAIT_FOR_MATCH_CRYPTO,
        WAIT_FOR_CONFIG,
        WAIT_FOR_MATCH,
        ESTABLISHED,
        FAILED,
        //RECV_FAILED,
        WAIT_FOR_CLOSE,
        // state used while waiting for STUN setup
        WAIT_FOR_STUN,
        WAIT_FOR_USER_ACCEPT,
        // protocol for talking to database server
        SERVER_OPS,
        // for chat server (hosted at our site)
        CHAT_OPS
    };
    enum state pstate;

    enum cstate {CHAT_NO_STATE, CHAT_WAIT_FOR_CHALLENGE, CHAT_NORMAL};
    enum cstate chat_state;
    // DO NOT CHANGE THESE, USED IN EXTERNAL INTERFACE
    enum call_accept {
        MMCHAN_US_ACCEPT_CALL = 1,
        MMCHAN_US_DEFER_CALL = 2,
        MMCHAN_US_REJECT_CALL = 3
    };

    // note: some of these states are the same as the returns from
    // the MMTube gen_channel call. bad idea, but ok for now.
    // this is used for connection setup for media channels. after the
    // channel is set up, the normal "CHAN_*" states are used.
    enum media_chan_state {
        MEDIA_ERR = SSERR,
        MEDIA_TRYAGAIN = SSTRYAGAIN,
        MEDIA_CONNECTED = 1,
        MEDIA_SESSION_UP = 2,
        MEDIA_SESSION_SETUP = -5,
        MEDIA_STOP_RETRYING = -6
    };

private:
    vc ctrl_q;

    friend void serv_recv_online(MMChannel *mc, vc prox_info, void *, ValidPtr mcv);
    friend void stun_connect(vc host, vc port, vc prox, vc uid, MessageDisplay *);
    friend class dwyco::DwQSend;
    friend class dwyco::DirectSend;

    void drop_subchannel(int chan);

    DwVecP<sproto> simple_protos;

    int handle_channels();

    int pause;

    void react_to_droppage(int ndropped);
    void react_to_delay_change(vc);

    DwTimer ref_timer;
    DwTimer frame_timer;
    DwTimer drop_timer;

    unsigned long frame_interval;
    int ready_for_ref;
    int frame_send;
    int drop_send;
    int remote_percent_dropped;
    int percent_dropped;

    // estimate for fps on channel
public:
#ifdef DWYCO_RATE_DISPLAY
    int rate_updated(int);
#else
    int rate_updated(int) {return 0;}
#endif
    void enable_packet_drop_reporting(int);

private:
    DwRateMonitor fps_send;
    DwRateMonitor fps_recv;
    DwRateMonitor bps_send;
    DwRateMonitor bps_recv;
    DwRateMonitor bps_audio_send;
    DwRateMonitor bps_audio_recv;
    DwRateMonitor bps_file_xfer;
    int rate_update;
    DwString info_format(int);

// a channel can broadcast its coded data to other
// channels by making its data available here.
// other channels (that may not be connected to an
// acq dev) find this channel via channel id, and
// sample this member when they need data.
    char *last_coded_buf;
    long len_last_coded_buf;
    long last_acq_time;
    long last_time_index;
    int last_frame_type; // 1 = reference frame
public:

    static ChanList *get_serviced_channels();
    static ChanList *get_serviced_channels_net();
    static int some_serviced_channels_net();
    static int any_ctrl_q_pending();
    static ChanList *get_serviced_channels_server();

    int is_coder();
#if 0
    int is_sender();
    int is_receiver();
    int is_audio_sender();
    int is_audio_receiver();
    int is_chatter();
    int is_private_chatter();
#endif

    // these allow you to give a string name to
    // a channel so you can see what they are while
    // debugging
    void set_string_id(const DwString &);
    DwString get_string_id();
private:
    DwString str_id;

public:
    vc fail_reason;
    // set to 1 when the remote side actively refuses a connection
    // after connection setup and negotiation.
    int rejected;
    // set to non zero when a call is being abandoned (ie
    // the entire call process needs to be stopped, can't use
    // just "destroy" even any more, because we may need to get
    // several destroys during a call setup.
    int cancel;

    // set to 1 for public chat to be routed
    // to a subset of current public chat
    // channels
    int selective_chat_mode;

    // set to one if the channel is to receive
    // the selective chat output.
    int selective_chat_output;

private:
    int negotiating;

    // chat message processing related things

    friend void chat_online(MMChannel *, vc, void *, ValidPtr);
    friend void chat_offline(MMChannel *, vc, void *, ValidPtr);
    int build_outgoing_chat(int locally_invoked);
    int remote_chat();
    int local_chat();
    vc username();
    void display_chat(vc who, vc msg, vc from_uid);
    void send_chat(vc msg);
    void send_selective_chat(vc msg, int send_to_remote);
    int all_msg_broadcast_done();
    int other_chat_receivers();
public:
    static int selective_chat_recipient_enable(vc uid, int enable);
    static int selective_chat_enable(int e);
    static void reset_selective_chat_recipients();
    static int is_selective_chat_recipient(vc uid);
private:

    int last_msg_index;
    vc last_msg;
    KeyboardAcquire *kaq;
    ChatDisplay *chat_display;
public:
    int chatbox_id;
private:
    int grab_chat_id;
    int chat_master;
    int chat_proxy;

    int private_chatbox_id;
    ChatDisplay *private_chat_display;
public:
    KeyboardAcquire *private_kaq;
    void display_chat_private(vc msg);
#if 0
    int remote_chat_private();
    int local_chat_private();
#endif
    void send_chat_private(vc msg);
    int build_outgoing_chat_private(int locally_invoked);

public:
    static MMChannel *find_chat_master();

    // audio related members

public:

    AudioAcquire *audio_sampler;
    AudioOutput *audio_output;
    DwVecP<AudioConverter> audio_coders;
    DwVecP<AudioConverter> audio_decoders;

    static MMChannel *find_audio_xmitter(int any = 0);
    static MMChannel *find_audio_player(int any = 0);

    void set_audio_timecode(unsigned int);
    unsigned int get_audio_timecode();

private:
    int last_audio_index;
    DWBYTE *last_audio_buf;
    int last_audio_len;
    // changed these to explicit sizes (even that may be wrong
    // since it may be an "at least" thing... need to check the
    // specs to see) because they are directly encoded in
    // audio output stream, which in retrospect, was probably a bad decision.
    uint32_t last_audio_timecode;
    int32_t last_payload_type;
    uint32_t audio_timecode;
    int audio_chan;
    int audio_state;
    int force_unreliable_audio;
    int32_t current_payload_type;

    // reliable video stuff
    int force_unreliable_video;
    int video_chan;
    int video_state;
    int video_output_device_blocked;
    int process_incoming_reliable_video();
    int send_reliable_video_proxy();

    int audio_decoder_from_config();
    int audio_coder_from_config();
    static int str_to_payload_type(vc str);
    int local_recv_audio();
    int remote_recv_audio();
    int local_send_audio();
    int remote_send_audio();

    int internal_0_offset;
    //sd3 preprocess_w_vad;

public:
    int build_outgoing_audio(int);
    int build_incoming_audio(int);
    int grab_audio_id;

public:
    void (*audio_callback)(ValidPtr, double);
    ValidPtr *audio_gain_gauge;
    long client_timecode_adjustment;
    int make_first_0;

    // set vid_make_first_0 to 1, and the next frame
    // that is acquired will be set to captime 0, and
    // successive frames will be properly offset from
    // there. this is so you can use the proper acq timing
    // but use a different offset from whatever timing
    // is being generated by the acquisition device.
    int vid_make_first_0;
    long vid_client_timecode_adjustment;
private:
    long vid_internal_0_offset;

private:
    void reject_call();
    void finish_connection();
    void finish_connection_new();
public:
    enum accpt {NONE, ACCEPT, REJECT, ZACCEPT, ZACCEPT_ALWAYS,
                ZREJECT, ZREJECT_IGNORE
               } user_accept;
    int accept_box;

private:
    int auto_quality_boost;

private:
    // file xfer stuff
    FILE *read_fd;
    FILE *write_fd;
    off_t write_fd_offset; // for resumed pull recvs
    off_t seekto; // for resumed server sends

    void set_progress_abort(const DwString&);
    void set_progress_done(const DwString&);
    void set_progress_got(int len);
    void set_progress_status(const DwString&, int = -1);

    DwString name_id();

public:
    vc remote_filename;
    vc local_filename;
    int zap_send;
    int zap_recv;
    int direct_zap_attachment_declined;
    int total_got;
    int expected_size;

private:
    int attempt_audio_q_send(int& out_len);
    int send_reliable_audio(DWBYTE *buf, int len);
    BroadcastQ raw_audioq;
    BroadcastQ audioq;
    //BroadcastQ videoq;
    //BroadcastQ dataq;

    int process_incoming_audio();
    void process_incoming_audio2(DWBYTE *buf, int len, int subchan, int timecode);
    void process_incoming_reliable_audio();
    int process_outgoing_audio();
    void process_audio_sampler();
    void handle_audio_duplex();

    // these are related to connection/name lookup

    friend struct  BodyView;
    friend MMChannel *fetch_attachment(vc, DestroyCallback, vc, void *, ValidPtr, StatusCallback, void *,ValidPtr, vc ,vc);
    friend void secondary_db_offline(MMChannel *, vc, void *, ValidPtr);
    friend class TMsgCompose;
    friend void send_qd_and_attachment(DwString qfn, vc m);
    friend int DWYCOCALLCONV dwyco_enable_video_capture_preview(int on);
    friend void dwyco::poll_listener();
private:

    friend void async_lookup_handler(HANDLE, DWORD);
    void cancel_resolve();
    HANDLE hr;
    DwTimer resolve_timer;
    int resolve_done;
    int resolve_len;
public:
    int resolve_failed;
    vc attempt_uid;
private:
    char *resolve_buf;
    int resolve_result;
    char official_name[255];
    char addrstr[255];
    char ouraddr[255];
public:
    int call_setup;
    vc call_type;

private:

    int poll_connect();
    int poll_resolve();
    void show_winsock_error(int err);
public:
    enum resolve_how {BYNAME=0, BYADDR=1};
    int start_resolve(enum resolve_how how, unsigned long addr, const char *hostname);
    int start_connect();
    in_addr addr_out;
    int port;

private:
    int start_negotiation();
    int poll_negotiation();
    //int local_media_setup();
    int local_media_setup_new();
public:
    MessageDisplay *msg_output;
    void msg_out(const char *msg);

    // these are for keeping sets of variables synced
    // across the link
    SyncMap syncmap;
    SyncManage sync_manager;
    SyncVar bw_limit_incoming;
    SyncVar bw_limit_outgoing;
    SyncVar available_audio_decoders;
    SyncVar available_audio_coders;
    SyncVar pinger;
    SyncVar pause_audio;
    SyncVar pause_video;
    SyncVar incoming_throttle;

    SyncMap remote_vars;
    SyncVar rem_bw_limit_incoming;
    SyncVar rem_bw_limit_outgoing;
    SyncVar rem_available_audio_decoders;
    SyncVar rem_available_audio_coders;
    SyncVar rem_pinger;
    SyncVar rem_pause_audio;
    SyncVar rem_pause_video;
    SyncVar rem_incoming_throttle;
    int remote_updated;

    void sync_send();
    void sync_recv(vc);
    DwTimer sync_timer;
    DwTimer pinger_timer;

    int xfer_failed;
    int connect_failed;
    DestroyCallback destroy_callback;
    vc dcb_arg1;
    void *dcb_arg2;
    ValidPtr dcb_arg3;
    // sends the id of the channel being destroyed
    ssns::signal1<int> destroy_signal;

    StatusCallback status_callback;
    void *scb_arg1;
    ValidPtr scb_arg2;
    StatusCallback established_callback;
    vc ecb_arg1;
    void *ecb_arg2;
    ValidPtr ecb_arg3;
    // this is set after the established callback is called, and is used to provide
    // a synchronization point so that regular control procedures and their callbacks
    // are only delivered after the callER has seen the established state, and the
    // callEE has gotten (or is within) the call_accepted callback.
    int established_callback_called;
    DwVec<vc> deferred_delivery_q;

    // be careful using this timer, it is used all over the place
    // for messaging and connection timeouts and stuff.
    DwTimer timer1;
    StatusCallback timer1_callback;
    vc t1cb_arg1;
    void *t1cb_arg2;
    ValidPtr t1cb_arg3;

    // this timer is lesser used,only for timing out
    // seconddary database connections now.
    DwTimer ctrl_timer;
    StatusCallback ctrl_timer_callback;
    vc ctcb_arg1;
    void *ctcb_arg2;
    ValidPtr ctcb_arg3;

    // this timer is used to keep the control q from growing
    // endlessly if the connection has gone away
    DwTimer ctrl_send_watchdog;

    StatusCallback2 store_message_callback;
    void *smcb_arg2;
    StatusCallback got_ok_callback;
    vc gcb_arg1;
    void *gcb_arg2;
    ValidPtr gcb_arg3;
    StatusCallbackChan chan_established_callback;
    vc cec_arg1;
    void *cec_arg2;
    ValidPtr cec_arg3;
    StatusCallback stun_established_callback;
    vc sec_arg1;
    void *sec_arg2;
    ValidPtr sec_arg3;
    // this is for low level connection tracking, mainly
    // for checking internal operations are working as
    // expected.
    StatusCallback low_level_connect_callback;
    vc llcc_arg1;
    void *llcc_arg2;
    ValidPtr llcc_arg3;

    static StatusCallback2 uc_message_callback;
    void *ucmcb_arg2;

    StatusCallback call_setup_canceled_callback;
    vc csc_arg1;
    void *csc_arg2;
    ValidPtr csc_arg3;

    StatusCallback call_appearance_death_callback;
    vc cad_arg1;
    void *cad_arg2;
    ValidPtr cad_arg3;

    static StatusCallback connection_list_changed_callback;
    static vc clc_arg1;
    static void *clc_arg2;
    static ValidPtr clc_arg3;

    // not sure if this being static is really necessary
    static StatusCallback call_accepted_callback;
    vc ca_arg1;
    void *ca_arg2;
    ValidPtr ca_arg3;
    // another sync-point flag, see comment near established_callback
    int call_accepted_callback_called;

    static CallAppearedCallback call_appeared_callback;

    static ChatDisplayCallback gen_public_chat_display_callback;
    static ChatDisplayCallback gen_private_chat_display_callback;

    static GetWindowCallback get_mdi_client_window_callback;
    static GetWindowCallback get_main_window_callback;

    ChatDisplay * gen_public_chat_display();
    ChatDisplay * gen_private_chat_display();

    static UIPopupCallback popup_zap_accept_box_callback;
    static UIPopupCallback set_progress_status_callback;
    static UIPopupCallback popup_message_box_callback;
    static UIPopupCallback popup_update_box_callback;

    static CallScreeningCallback call_screening_callback;
    // this is set to 1 if the call_screening callback
    // has been called to screen the call. this means
    // the normal call screening should *not* be performed
    // when device allocation is finally performed.
    int prescreened;

    DwTimer keepalive_timer; // used for pinging servers
    void keepalive_processing();

private:
    static int adjust_outgoing_bandwidth();
    static int adjust_incoming_bandwidth();
    static int get_available_output_bandwidth();
    static int get_available_input_bandwidth();
    int adjust_outgoing_throttle(int);
    int outgoing_throttle_limit;
    static DwTimer Bw_adj_timer;

    // messaging related stuff
public:
    int msg_chan;	// 1 for link dedicated to messages
    int user_control_chan; // 1 for link dedicated to simple control msgs

public:

    int start_server_ops();
    static MMChannel * get_server_channel();
    static MMChannel * get_secondary_server_channel();
    static void send_to_db(dwyco::QckMsg& m, int chan_id);
    void server_response(vc v);
    void chat_response(vc);
    int disconnect_server();
    static MMChannel *start_server_channel(enum resolve_how,
                                           unsigned long addr, const char *name, int port);
    int server_channel;
    int secondary_server_channel;
    vc password;

    // used to tell if a channel is being set up, so we
    // can q things to it before it is connected.
    vc attempt_ip;
    vc attempt_port;
    vc attempt_name;

    // this is used to lightly scramble the  output
    // of a coder (input to decoder). for use with
    // the forward control stuff. leave it as 0 for
    // normal coding and decoding.
    unsigned int codec_tweak;

    // synchronously destroy all channels, useful when going to
    // sleep to terminate all activity
    static void exit_mmchan();


    friend void stun_connect_ok(MMChannel *mc, vc, void *, ValidPtr vp);
    friend void tcp_proxy_connect_and_decide(MMChannel *mc, vc, void *, ValidPtr vp);

    // used for server-assisted calls, this is the
    // ip and port of the proxy to rendezvous with
    vc proxy_info;

    // force the channel setup to signal it has no
    // reliable media setup channels. this is used for
    // situations where the control channel goes via
    // tcp proxy, and the media goes via UDP directly ala STUN
    int force_unreliable_config;
    // set to 1 will cause the config to contain
    // a "stun" setting to let the peer know what we need
    int use_stun;
    int sent_stun;

    // set this to the desired media setup only
    // as text (MEDIA_* from callive.h)
    // and it will take part in the capabilities negotiation.
    vc media_setup;
    // this is the result of the negotiations
    vc nego_media_select;

    // for connections whose data is being proxied thru
    // a server, we explicitly limit the outgoing
    // bandwidth, regardless of what the outgoing/incoming
    // throttles are set to between the parties.
    int proxy_outgoing_bw_limit;
#if 0
    // STUN setup stuff
#define MM_STUN_SOCK_COUNT 2
    DwVec<vc> stun_socks;
    int poll_stun();
    // called when the peer's stun info is received
    void got_peer_stun(vc);
    vc peer_stun;
#endif

public:
    static vc codec_number_to_name(int);
    static int codec_name_to_number(vc);
    void set_requested_codec(int);
    void set_requested_audio_codec(const char *recording_mode);
    int get_requested_codec();

    // step record and playback, for stills
    // note: assumes using video only, and that
    // the next frame to be captured must be a
    // reference frame (which means Sync_receivers
    // should be turned off before starting step capture
    // unless you want to wait for the next ref
    // frame to show up.)
    // for playout, just the number of frames are played, then
    // the channels stop playing and holds that position.

    int step_mode;

    // set to the number of frames to capture or
    // playback
    int step_frames;

    // step done callback is called when frames
    // decrements to 0

    StatusCallback step_done_callback;
    vc sdc_arg1;
    void *sdc_arg2;
    ValidPtr sdc_arg3;

    static void pause_all_channels(int);


    // these items are used to control and associate
    // new connections. in the old way, listening
    // connections were associated directly with
    // a channel. the port associated with the
    // channel was sent in the protocol to the clients.
    // this is not firewall or NAT friendly. now,
    // we get connections on a fixed port, and then
    // when the protocol has progressed thru the
    // initial handshake, we associate based on the
    // SESSION id. note: we can't use UID's or IP
    // for this because someone might have multiple
    // sessions going between two computers (ie, one UID
    // has multiple sessions, and we need to keep
    // those squared away.) and single IP's might have
    // multiple clients behind it (note: this problem
    // isn't solved in this version of the protocol, we'll
    // have to put port information in addition to
    // IP's in the directory and other IP sources if
    // we want to be completely NAT friendly.)

    int session_id;

    // this is for audio q-ing stuff
    DwVec<int> granted;

    // this is for exclusive audio send rather than the default
    // broadcast send. Set Exclusive_audio to 1 and the id to
    // the channel id you want audio to stream to. If you set the id
    // to -1, audio is not sent to any channel, however the audio
    // timecode will continue to be incremented as long as there are
    // audio sampler objects in the system.
public:
    static int Exclusive_audio;
    static int Exclusive_audio_id;

public:
    // these are used for channel encryption, NOT for message encryption
    vc channel_keys;
    vc agreed_key;
    void send_decrypt();

    // note: message encryption generates a message with an
    // embedded key that can only be extracted with the private
    // key of the recipient. this is done independently of
    // transmission of the message.
    // messages that are sent using these channels (ie, direct
    // messages that are not stored on the server) are not
    // encrypted, the channel provides the encryption.
    // this allows us to provide "in flight" encryption without
    // changing a lot of the data management for messages locally.
    // messages are not encrypted before they are stored on disk.

public:
    int init_connect(int subchan, sproto *p, const char *ev);
    int send_file_info(int subchan, sproto *p, const char *ev);
    int send_file_info_server(int subchan, sproto *p, const char *ev);
    int check_file_response(int subchan, sproto *p, const char *ev);
    int send_chunk(int subchan, sproto *p, const char *ev);
    int wait_for_ok(int subchan, sproto *p, const char *ev);
    int send_rej_ack(int subchan, sproto *p, const char *ev);
    int get_what_to_do(int subchan, sproto *p, const char *ev);
    int recv_chunk(int subchan, sproto *p, const char *ev);
    int accept_att(int subchan, sproto *p, const char *ev);
    int send_pull_info(int subchan, sproto *p, const char *ev);
    int recv_pull_info(int subchan, sproto *p, const char *ev);
    int send_recv_reject(int subchan, sproto *p, const char *ev);
    int send_recv_ok(int subchan, sproto *p, const char *ev);
    int send_ok(int subchan, sproto *p, const char *ev);
    int send_sf_material(int subchan, sproto *p, const char *ev);

    int send_media_ok(int subchan, sproto *p, const char *ev);
    int check_media_response(int subchan, sproto *p, const char *ev);
    int send_media_id(int subchan, sproto *p, const char *ev);
    int init_connect_media(int subchan, sproto *p, const char *ev);
    int send_media_ping(int subchan, sproto *p, const char *ev);
    // set to 1 if attachment actively rejected (can never be sent,
    // usually because size is too big.
    int zreject;

};

#define AUDIO_NUM_STATES 8
#define AUDIO_TALK 0
#define AUDIO_NOTALK 1
#define AUDIO_MUTE 2
#define AUDIO_NONE 3
#define AUDIO_CALIBRATE 4
#define AUDIO_SQUELCH 5
#define AUDIO_NO_HW 6
#define AUDIO_PLAYING 7

#define SQUELCH_NUM_STATES 2
#define SQUELCH_OFF 1
#define SQUELCH_ON 0

// these are used for talking to old 0.95
// systems, where both were multiplexed onto
// one socket.
#define AUDIO_CHANNEL 1
#define VIDEO_CHANNEL 0

#define VIDEO_CHANNEL_V2 (FIRST_UNRELIABLE_CHAN)
#define AUDIO_CHANNEL_V2 (FIRST_UNRELIABLE_CHAN + 1)
#define AUDIO_SUBCHANNEL 0


#define PAYLOAD_LPC4800 0
#define PAYLOAD_MSGSM610 1
#define PAYLOAD_RAW8KHZ 2
#define PAYLOAD_SPEEX 3
#define PAYLOAD_UWB_SPEEX 4
#define PAYLOAD_VORBIS 5
#define PAYLOAD_VORBIS8KHZ 6
#define PAYLOAD_NUM 7

#define subchan(id, pt) (((id) & 0x7)|(((pt) << 3) & 0xf8))
#define get_chan_num(sc) ((sc) & 0x7)
#define get_payload_type(sc) (((sc) >> 3) & 0x7)

#define FPS_SEND 1
#define FPS_RECV 2
#define BPS_SEND 4
#define BPS_RECV 8
#define BPS_AUDIO_SEND 16
#define BPS_AUDIO_RECV 32
#define SENDER (FPS_SEND|BPS_SEND)
#define RECEIVER (FPS_RECV|BPS_RECV)
#define AUDIO_RATE (BPS_AUDIO_SEND|BPS_AUDIO_RECV)

// this is here to work around routers that time-out idle tcp
// connections after a certain amount of time, leaving the connection
// in an error state. the server drops the connection after 10 minutes,
// but some routers do this before 10 minutes, which will filter out
// any low-level tcp messages that would indicate to the client that
// the connection has been closed.
#define SECONDARY_CHANNEL_DROP_TIMEOUT (2 * 60 * 1000)
// this is the same thing, but for idle audio and video channels
#define VIDEO_IDLE_TIMEOUT (2 * 60 * 1000)
#define AUDIO_IDLE_TIMEOUT (2 * 60 * 1000)

//#define VIDEO_IDLE_TIMEOUT (15 * 1000)
//#define AUDIO_IDLE_TIMEOUT (15 * 1000)


// previewing captured video is a bit of a special case compared to
// displaying decoded video.
// we reserve this "channel id" for when we call client-callbacks for
// displaying preview video. if the client needs to display captured
// video in multiple places (for example, when showing approximate
// video during step recording for short video messages), it can
// tell the coder object to use other arbitrary "channels ids", but it
// has to tell the client somehow what to do with the video.
//
// as an example, the TMessageComposer class will insert the composer
// id into the coder's output list, and the client knows that when
// it gets a "display_video" callback with that composer id it needs to
// associate that video with the composer.
//
// to simplify things, i have merged all of the id "spaces" into one
// space so that channel id, ui_ids, viewer ids and so on all come
// from the same place, and are session unique. this removes the need
// for lots of mapping between ids. for example, now the viewer id
// can be used directly in the video display callback and you know
// the video frame will be associated with that viewer from the
// id provided in the video_frame callback.
//
// previously, we had two different spaces
// for the channel id's and other ids in the system, and they could
// overlap. we then had to have yet another id (ui id) to get all the
// video routed to the right place. this was ok, but forced the client
// to do another mapping.
#define MMCHAN_PREVIEW_CHAN_ID 1

// protocol used for direct connections.
// direct connections are rejected if you don't have the
// exact protocol version (note: this disables things like
// direct messaging and streaming, but not server related things
// like messaging, since those are handled with separate
// protocols.)
#define MMCHAN_PROTOCOL_VERS (6)

extern struct strans recv_command[];
extern struct strans file_send[];
extern struct strans file_send_server[];
extern struct strans media_x_setup[];
extern struct strans recv_attachment_pull[];

#endif
