
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DLLI_H
#define DLLI_H

#ifdef ANDROID
#define CDCCORE_STATIC
#endif
#if (defined(_Windows) || defined(_WIN32)) && !defined(CDCCORE_STATIC)
#ifdef MINGW_CLIENT
#define DWYCOEXPORT __declspec(dllexport) DWYCOCALLCONV
#define DWYCOCALLCONV __cdecl
#else
#define DWYCOCALLCONV __stdcall
#if defined(_MSC_VER) && defined(DWYCOIMPORT)
#define DWYCOEXPORT __declspec(dllimport) DWYCOCALLCONV
#else
#define DWYCOEXPORT __declspec(dllexport) DWYCOCALLCONV
#endif
#endif
#elif defined(CDCCORE_STATIC) && defined(_Windows)
#define DWYCOCALLCONV
#define DWYCOEXPORT
#else
// probably linux
#ifdef MINGW_CLIENT
#define DWYCOEXPORT __declspec(dllexport) DWYCOCALLCONV
#define DWYCOCALLCONV __cdecl
#else
#define DWYCOEXPORT
#define DWYCOCALLCONV
#endif
#endif

#ifdef DWYCO_APP_DEBUG
#define APP_GRTLOG(fmt, v1, v2) do {dwyco_app_debug1( __FILE__, __LINE__, (fmt), (v1), (v2)); } while(0)
#define APP_GRTLOGA(fmt, v1, v2, v3, v4, v5) do {dwyco_app_debug2( __FILE__, __LINE__, (fmt), (v1), (v2), (v3), (v4), (v5)); } while(0)
#else
#define APP_GRTLOG(fmt, v1, v2) do {} while(0)
#define APP_GRTLOGA(fmt, v1, v2, v3, v4, v5) do {} while(0)
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef void *DWYCO_LIST;
typedef DWYCO_LIST DWYCO_USER_LIST;
typedef DWYCO_LIST DWYCO_UNFETCHED_MSG_LIST;
typedef DWYCO_LIST DWYCO_SAVED_MSG_LIST;
typedef DWYCO_LIST DWYCO_SERVER_LIST;
typedef DWYCO_LIST DWYCO_QUERY_RESULTS_LIST;
typedef DWYCO_LIST DWYCO_MSG_IDX;
typedef DWYCO_LIST DWYCO_QD_MSG_LIST;

// WARNING: for "connect_all" processing, channel_destroy and call_established
// callbacks may act funny starting ca 4/2006 (ie, the id's may not match up
// between status callbacks and other callbacks. this is due to the new server
// assisted calling. use "connect_all2" interface instead.
// destroy callbacks still work as usual for other parts of the interface.
typedef void (DWYCOCALLCONV *DwycoChannelDestroyCallback)(int id, void *user_arg);

// see below for DWYCO_CALLDISP_* constants for the "what" parameter in this callback
typedef  void (DWYCOCALLCONV *DwycoCallDispositionCallback)(int call_id, int chan_id, int what, void *user_arg, const char *uid, int len_uid, const char *call_type, int len_call_type);

typedef  void (DWYCOCALLCONV *DwycoStatusCallback)(int id, const char *msg, int percent_done, void *user_arg);
typedef  void (DWYCOCALLCONV *DwycoVideoDisplayCallback)(int chan_id, void *img, int cols, int rows, int depth);
typedef  void (DWYCOCALLCONV *DwycoCallAppearanceCallback)(int chan_id, const char *name, const char *location, const char *uid, int len_uid, const char *call_type, int len_call_type);
typedef  void (DWYCOCALLCONV *DwycoZapAppearanceCallback)(int chan_id, const char *name, int size, const char *uid, int len_uid);
typedef  void (DWYCOCALLCONV *DwycoCallAppearanceDeathCallback)(int chan_id);
// NOTE: the call_type is a string that can be sent in by the client on the remote
// end. This can help in cases where different screening rules apply for different
// calls in the UI.
typedef  int (DWYCOCALLCONV *DwycoCallScreeningCallback)(int chan_id, int remote_wants_to_recv_your_video, int remote_wants_to_send_you_video, int remote_wants_to_recv_your_audio, int remote_wants_to_send_you_audio, int remote_wants_to_exchange_pubchat, int remote_wants_to_exchange_privchat, const char *call_type, int len_call_type, const char *uid, int len_uid, int *accept_call_style_out, char **error_msg_out);
// WARNING: recipient and reciplen will always be 0 (it isn't implemented right now.)
typedef  void (DWYCOCALLCONV *DwycoMessageSendCallback)(int id, int what, const char *recipient, int reciplen, const char *server_msg, void *user_arg);
typedef  int (DWYCOCALLCONV *DwycoPublicChatInitCallback)(int chan_id);
typedef  int (DWYCOCALLCONV *DwycoPrivateChatInitCallback)(int chan_id, const char *not_used);

// NOTE: see comment at dwyco_command_from_keyboard regarding the commands
// and parameters that will show up here during a private chat.
typedef  int (DWYCOCALLCONV *DwycoPrivateChatDisplayCallback)(int chan_id, const char *com, int arg1, int arg2, const char *str, int len);

typedef  int (DWYCOCALLCONV *DwycoPublicChatDisplayCallback)(const char *who, int len_who, const char *line, int len_line, const char *uid, int len_uid);
#if 0
typedef  void (DWYCOCALLCONV *DwycoVideoDisplayInitCallback)(int chan_id, int ui_id);
#endif
typedef  void (DWYCOCALLCONV *DwycoCommandCallback)(const char *cmd, void *user_arg, int succ, const char *failed_reason);
typedef  void (DWYCOCALLCONV *DwycoAutoUpdateDownloadCallback)(int status);
// see "autoupdate check status" below for details on what the status is for this callback
typedef  void (DWYCOCALLCONV *DwycoAutoUpdateStatusCallback)(int status, const char *desc);
typedef  void (DWYCOCALLCONV *DwycoProfileCallback)(int succ, const char *reason, const char *s1, int len_s1, const char *s2, int len_s2, const char *s3, int len_s3, const char *filename, const char *uid, int len_uid, int reviewed, int regular, void *user_arg);
typedef  void (DWYCOCALLCONV *DwycoServerLoginCallback)(const char *str, int what);
typedef  void (DWYCOCALLCONV *DwycoMessageDownloadCallback)(int id, int what, const char *msgid, void *mdc_arg1);
// note: elide value since interpretation depends on type arg, so printing it
// generically can cause problems
typedef  void (DWYCOCALLCONV *DwycoChatCtxCallback)(int cmd, int ctx_id, const char *uid, int len_uid, const char *name, int len_name, int type, const char *value_elide, int val_len, int qid, int extra_arg);

typedef  void (DWYCOCALLCONV *DwycoChatCtxCallback2)(int cmd, int ctx_id, const char *uid, int len_uid, const char *name, int len_name, DWYCO_LIST lst, int qid, int extra_arg);

// note: this is the same as the chat server callback for now
typedef DwycoChatCtxCallback DwycoSystemEventCallback;

// the DLL calls this callback when it gets an
// unrecoverable problem. if must_exit is non-zero, the application MUST
// cleanup and EXIT as soon as possible, otherwise the DLL state could
// be corrupted in any number of ways.  the UI should popup a box with
// an informative message for the user before performing cleanup and exit.
// if must_exit is 0, it doesn't matter if the UI exits (either because
// the error wasn't a big deal, OR the dll is going to exit immediately
// when the callback returns anyways.)
//
// the args are:
// whats_the_problem - one of
#define DWYCO_EMERGENCY_DB_CHANGE 1 // when the callback returns, program continues as usual
#define DWYCO_EMERGENCY_GENERAL_PANIC 2       // on callback return, the dll exits the program
// the "dll_msg" should be helpful to tech support or for debugging.
//
typedef  void (DWYCOCALLCONV *DwycoEmergencyCallback)(int whats_the_problem, int must_exit, const char *dll_msg);

#define DWYCO_PAL_AUTH_STATUS_FAILED 0
#define DWYCO_PAL_AUTH_STATUS_REQUIRES_AUTHORIZATION 1
#define DWYCO_PAL_AUTH_STATUS_ALREADY_AUTHORIZED 2

//typedef  void (DWYCOCALLCONV *DwycoPalAuthCallback)(const char *uid, int len_uid, int what);
typedef  void (DWYCOCALLCONV *DwycoUserControlCallback)(int chan_id, const char *uid, int len_uid, const char *data, int len_data);

// this is called when the activity timer expires.
// the timeout arg is the current timeout value in seconds
typedef void (DWYCOCALLCONV *DwycoActivityCallback)(int timeout);

// activity reporting
// as a default, the
// basic idea is that the dll sets a timer and starts it
// running. if the timer expires, the dll goes into "inactive"
// state. you could imagine all kind of stuff regarding
// this state, like giving up the camera, turning off audio, etc.
// but for now, we just tell the servers our state so it can
// be reflected to others.
// IF you want to implement your own activity checking, you
// can call this function, which automatically disables all of the
// dll's internal activity monitoring and status updating.
// you'll have to perform all your own checking, and status updating
// (see chatq_send_activity below).
// "timeout" is in seconds
// calling this function with on = 1, timeout > 0, cb != 0
// results in the timer being reset and the timer to start counting down.
// when the countdown == 0, the callback is called.
// calling this function with on = 0 will cause the timer to stop counting down.
//
void DWYCOEXPORT dwyco_enable_activity_checking(int on, int timeout, DwycoActivityCallback cb);
// you can call this if you want to use the internal dll activity checking, but
// just want to tweak the timer value it uses.
// when you call this, it resets the current timer immediately.
void DWYCOEXPORT dwyco_set_inactivity_time(int secs);

void DWYCOEXPORT dwyco_set_fn_prefixes(
    const char *sys_pfx,
    const char *user_pfx,
    const char *tmp_pfx
);
void DWYCOEXPORT dwyco_get_fn_prefixes(char *sys_pfx, int *sys_len_in_out, char *user_pfx, int *user_len_in_out, char *tmp_pfx, int *tmp_len_in_out);

void DWYCOEXPORT dwyco_set_public_chat_init_callback(DwycoPublicChatInitCallback cb);
void DWYCOEXPORT dwyco_set_private_chat_init_callback(DwycoPrivateChatInitCallback cb);
void DWYCOEXPORT dwyco_set_private_chat_display_callback(DwycoPrivateChatDisplayCallback cb);
void DWYCOEXPORT dwyco_set_public_chat_display_callback(DwycoPublicChatDisplayCallback cb);
void DWYCOEXPORT dwyco_set_video_display_callback(DwycoVideoDisplayCallback cb);
#if 0
void DWYCOEXPORT dwyco_set_video_display_init_callback(DwycoVideoDisplayInitCallback cb);
#endif
void DWYCOEXPORT dwyco_set_debug_message_callback(DwycoStatusCallback cb);
#if 0
void DWYCOEXPORT dwyco_set_pal_auth_callback(DwycoPalAuthCallback cb);
#endif
void DWYCOEXPORT dwyco_set_emergency_callback(DwycoEmergencyCallback cb);
void DWYCOEXPORT dwyco_set_user_control_callback(DwycoUserControlCallback cb);
void DWYCOEXPORT dwyco_set_alert_callback(DwycoCommandCallback cb);
void DWYCOEXPORT dwyco_set_call_bandwidth_callback(DwycoStatusCallback cb);

// Warning: call screening is in the process of changing
// some of this may not be as advertised... XXX clean it up XXX
/*
if the callback is not set, normal screening occurs as it always has.
the callback is called each time a new call is received.
if the callback returns non-zero, normal call screening is performed.
otherwise, the dll checks the return value of the callbacks "accept_call_style"
to determine what to do with the call.
* ACCEPT causes the dll to immediately accept the call
and attempt media setup. the call accepted callback is called
when the devices are set up.

* REJECT causes the dll to reject the call and sends the "error message"
string back to the remote end. i think you get a destroy
callback on the channel eventually on this one.
NOTE: the DLL copies out the error string when the callback returns,
so it must be allocated or in static storage. it does NOT free the string,
that is up to the app to deal with that problem.

* DEFER causes the dll to hold the call pending "accept" "reject"
events from the UI (in this case, the "call appeared" callback is called
immediately for compatibility purposes, tho you can ignore it
if you want). the call accepted callback is called
on an accepted event, otherwise a destroy callback is
called on reject or timeout.
note also that the call-appearance-death callback is called
as usual in the DEFER cases as well.
*/

#define DWYCO_CSC_ACCEPT_CALL 1
#define DWYCO_CSC_DEFER_CALL 2
#define DWYCO_CSC_REJECT_CALL 3

void DWYCOEXPORT dwyco_set_call_screening_callback(DwycoCallScreeningCallback cb);
//
// the chat context callback is called when you are connected to a
// chatroom. the id's are session unique. names and uids must be copied out
// before the callback returns. a START_UPDATE callback is called before
// any update to the context. this can be used to batch updates in cases
// where you are initially logging in, and doing a single UI update for each
// user in the room would cause too much flicker.
// note: ca 3/15/2006, only one chat ctx is alive at once
#define DWYCO_CHAT_CTX_NEW 0		// when ctx is first created, id is new, all other parms 0
#define DWYCO_CHAT_CTX_DEL 1		// when ctx is deleted, means you have logged out of the chat server
#define DWYCO_CHAT_CTX_ADD_USER 2 	// for each user that comes into the chat room, uid and name parms valid.
#define DWYCO_CHAT_CTX_DEL_USER 3 	// for each user that leaves, only uid is valid
#define DWYCO_CHAT_CTX_UPDATE_AH 4	// update asshole-factor
#define DWYCO_CHAT_CTX_START_UPDATE 5 // happens before all update ops, can safely be ignored if flicker is not an issue
#define DWYCO_CHAT_CTX_END_UPDATE 6 // at the end of a batch of updates
#define DWYCO_CHAT_CTX_Q_ADD 7		// uid added to q. uid, qid, extra_arg = 0 for end-of-q, 1 for top of q
#define DWYCO_CHAT_CTX_Q_DEL 8		// uid removed from q
#define DWYCO_CHAT_CTX_Q_GRANT 9	// uid granted the podium, extra_arg = time in seconds to talk
// note: the following is useful for displaying status info such as
// "user is transmitting" or something like that. in the case of
// audio, you don't have to do anything with the data, it is handled automatically
// before you get this callback
#define DWYCO_CHAT_CTX_Q_DATA 10		// uid is sending data. name param contains the data

// new arg in callback contains a DWYCO_LIST style attribute value.
// the "name" arg in the callback contains the name of the attribute that is
// being updated, the "val" is the DWYCO_LIST style value.

// names of system attributes are as follows:
// <reference document here>
// note: UID has no meaning for this message, as it is a system-wide
// attribute, rather than for any particular user.
#define DWYCO_CHAT_CTX_SYS_ATTR 11

// names of attributes current are (with values):
// "muted" - t/nil
// "god" - t/nil
// "demigod" - t/nil
// "dict-err" - users name fails smell-test
//
#define DWYCO_CHAT_CTX_UPDATE_ATTR 12		 // name is name of attribute, value as value for attr ala DWYCO_LIST

// User-defined lobbies
// Only available via ChatCtxCallback2, since there is a fair bit of
// info that comes with these msgs. Essentially, these are lobbies that
// act more or less like the old static chat lobbies, except instead of using
// switch_to_chat_server(index), you use dwyco_switch_to_chat_server2(lobby-id)
// where the lobby-id is available from the add_lobby msg.
// note: for compatibility, a directory server comes with the
// the chat server "for free".
#define DWYCO_CHAT_CTX_ADD_LOBBY 13
#define DWYCO_CHAT_CTX_DEL_LOBBY 14

#define DWYCO_SL_ULOBBY_ID "000"
#define DWYCO_SL_ULOBBY_HOSTNAME "001"
#define DWYCO_SL_ULOBBY_IP "002"
#define DWYCO_SL_ULOBBY_PORT "003"
#define DWYCO_SL_ULOBBY_RATING "004"
#define DWYCO_SL_ULOBBY_DISPLAY_NAME "005"
#define DWYCO_SL_ULOBBY_CATEGORY "006"
#define DWYCO_SL_ULOBBY_BACKUP "007"
// note: internal pvt name at 008
#define DWYCO_SL_ULOBBY_MAX_USERS "009"
#define DWYCO_SL_ULOBBY_SUBGOD_UID "010"
// used internally, do not use
#define DWYCO_SL_ULOBBY_PW_do_not_use "011"

// GOD tracking, this is dynamic. When a god comes online you
// get an add_god message, no matter where they are.
// As a god moves around between servers, you will get
// a del-god when they leave, and a subsequent add when they
// enter a new server. You get a record of info indicating what
// kind of god-powers they have, and where they are (what server.)
#define DWYCO_CHAT_CTX_ADD_GOD 15   // a vector of extra info is sent as well in the callback
#define DWYCO_CHAT_CTX_DEL_GOD 16   // just the UID of the god that is gone

// This the info you get when you get a add-god msg
#define DWYCO_GT_NAME "000"
#define DWYCO_GT_WHERE "001"
#define DWYCO_GT_GOD "002"
#define DWYCO_GT_DEMIGOD "003"
#define DWYCO_GT_SUBGOD "004"
#define DWYCO_GT_SERVER_ID "005"

// Direct chat (broadcast) messages
// These are different from the "Q" calls above because
// there is no "q" of users waiting to stream.
// Data is sent via the chat server, and all users that are
// not filtering will receive the data (possibly interleaved
// arbitrarily.) The data is emphemeral (no delivery guarantees.)
#define DWYCO_CHAT_CTX_RECV_DATA 17

//
// when you get a ui-call-vector message, the
// value vector is accessed as a DWYCO_LIST with
// these columns
#define DWYCO_CV_MAX_AUDIO "000"
#define DWYCO_CV_MAX_VIDEO "001"
#define DWYCO_CV_CHAT "002"
#define DWYCO_CV_MAX_AUDIO_RECV "003"
#define DWYCO_CV_MAX_VIDEO_RECV "004"

//
// "popup" sysattr
#define DWYCO_POPUP_FROM_UID "000"
#define DWYCO_POPUP_FROM_HANDLE "001"
#define DWYCO_POPUP_MSG "002"


// this is a generic function used to set persistent sys-attrs in the
// server. some sys attrs are used by the dll to configure bandwidths
// and so on. you must be a god in order to use this command.
// sysattrs that are set with this command are automatically sent to all
// users of the chat server (via a SYS_ATTR msg).
// NOTE: there is no limit on the name and value (except the name has to be
// a string). so you can set your own sysattrs, and they
// will automatically be remembered by the server. you might use this for
// example to implement client-specific functions such as server-wide
// popups, etc. NOTE: currently ANY SYSATTR THAT STARTS WITH "us-" is
// reserved for the DLL.
// you can send in either a DWYCO_TYPE_NIL (in which case all the val
// params are ignored) DWYCO_TYPE_INT, in which case val_int param
// is used, or DWYCO_TYPE_STRING, in which case val and val_len are
// used.
int DWYCOEXPORT dwyco_chat_set_sys_attr(const char *name, int name_len,
                                        int dwyco_type, const char *val, int val_len, int val_int);

// this is the request to take the podium. you will receive
// a "grant" message if and when you get the podium (which may be
// never.) this always adds you to the end. if you call it multiple times
// while on the q, each time, you are put back down to the end of the q.
int DWYCOEXPORT dwyco_chat_addq(int q);

// if you set uid = 0 in this call, it uses the UID of the current user.
// if uid != 0 and uid != current user, you must be a god.
// use this call to recind your q add request. calling it if you are not
// on the q is a noop.
int DWYCOEXPORT dwyco_chat_delq(int q, const char *uid, int len_uid);

// use this to indicate that you want to transition thru the grace-period
// timeout directly to the "talk-time" limit. normally this is handled
// automatically by the dll when audio is acquired and starts streaming
// into the chat server. this is useful mostly for testing without audio
int DWYCOEXPORT dwyco_chat_talk(int q);

// don't send me any audio if on == 1
int DWYCOEXPORT dwyco_chat_mute(int q, int on);
// only allows gods, demigods, or a particular uid to have the podium. gods can set any flag. demigods can set the
// demigod flag and the uid. whenever you call this function, everyone is kicked off the q and must readd themselves.
int DWYCOEXPORT dwyco_chat_set_filter(int q, int gods_only,  int demigods_only, const char *uid, int len_uid);
// only gods can use the next 2 functions
int DWYCOEXPORT dwyco_chat_set_demigod(const char *uid, int len_uid, int on);
int DWYCOEXPORT dwyco_chat_clear_all_demigods();
// send the ui-activity attribute to chat-local users
// WARNING: DLL uses nil and "idle" for internal states
// if active == 1, the dll will send "nil" for the attribute
// if active == 0, the dll will send "idle"
// in both cases, "state" argument is ignored
// if active == -1, the dll will send the value of "state"
// make it something descriptive, but you must coordinate with other
// clients, since there will be incompatibilities otherwise.
int DWYCOEXPORT dwyco_chat_set_activity_state(int active, const char *state, int len_state);

// send a popup message to all local users as a "sys_attr".
// if global == 1, then it sends it to *all* users connected to all
// chat servers in the system. you can send local popups if you
// are a sub-god or demi-god or god.
// you must be a god to send a global popup.
int DWYCOEXPORT dwyco_chat_send_popup(const char *text, int len_text, int global);


// users are normally in an unblocked state.
// sets the unix time at which a users input will be unblocked to the current time
// PLUS the tm parameter (tm in seconds). set tm to -1 to immediately unblock a user.
// set tm to a positive number to block input from the user (for example, setting tm
// to 60 blocks a user for 60 seconds). only demigods can use this function.
// when a user is blocked, all of their input is discarded.
// as of 5/14/07, this isn't true anymore: "however,
// that user STILL RECEIVES all chat server output."
// a user that is blocked is immediately kicked off the q/podium.
int DWYCOEXPORT dwyco_chat_set_unblock_time(int q, const char *uid, int len_uid, int tm);
int DWYCOEXPORT dwyco_chat_set_unblock_time2(int q, const char *uid, int len_uid, int tm, const char *reason);

// this function prompts the server to belch out the current demigod and
// block lists. they come in as SYS_ATTRs, and the block records come in
// as vectors, and you can use these constants to access the record:
#define DWYCO_BLOCK_REC_BLOCKEE_UID "000"
#define DWYCO_BLOCK_REC_UNBLOCK_TIME "001"  // absolute time (unix time) when record become invalid
#define DWYCO_BLOCK_REC_BLOCKER_UID "002"
#define DWYCO_BLOCK_REC_BLOCKED_WHEN "003"	// absolute time (unix time) when the block was issued
#define DWYCO_BLOCK_REC_REASON "004"

int DWYCOEXPORT dwyco_chat_get_admin_info();

void DWYCOEXPORT dwyco_set_chat_ctx_callback(DwycoChatCtxCallback cb);
void DWYCOEXPORT dwyco_set_chat_ctx_callback2(DwycoChatCtxCallback2 cb);
//void DWYCOEXPORT dwyco_set_chat_server_status_callback(DwycoStatusCallback cb);

void DWYCOEXPORT dwyco_chat_create_user_lobby(const char *dispname,
        const char *category,
        const char *sub_god_uid, int len_sub_god_uid,
        const char *pw, int user_limit, DwycoCommandCallback cb, void *user_arg);

void DWYCOEXPORT dwyco_chat_remove_user_lobby(const char *lobby_id, DwycoCommandCallback cb, void *user_arg);

// this is used normally to send a "snap chat" txt + picture messages
// thru the chat server. The receiver will get a RECV_DATA
// messages, with a DWYCO_LIST of the form (txt, pic_type, pic_data)
// note the dll doesn't try to interpret the pic data, so if you want to
// compress it or whatever, you'll have to do that yourself.
// the "pic_type" just serves as some user data in situations where
// multiple clients use different pic formats.
// the data is broadcast to all chat users
void DWYCOEXPORT dwyco_chat_send_data(const char *txt, int txt_len, int pic_type, const char *pic_data, int pic_data_len);
#define DWYCO_CHAT_DATA_TXT "000"
#define DWYCO_CHAT_DATA_PIC_TYPE "001"
#define DWYCO_CHAT_DATA_PIC_DATA "002"
// two reserved pic-types
#define DWYCO_CHAT_DATA_PIC_TYPE_NONE 0
#define DWYCO_CHAT_DATA_PIC_TYPE_JPG 1

// This callback is designed to try and replace the "refresh users"
// and "rescan messages" flags. it may also replace a variety of other
// flag based status report eventually.
//
// emitted when a user changes some status that might be interesting
// for display purposes, like online, offline, has messages, etc.
#define DWYCO_SE_USER_STATUS_CHANGE 1
// a new user has been added to the user list. also emitted when a user
// has their info updated or refetched from the server
#define DWYCO_SE_USER_ADD 2
// a user has been completely removed from the system
#define DWYCO_SE_USER_DEL 3
// a connection to a server has been initiated, but not completed
#define DWYCO_SE_SERVER_CONNECTING 4
// a connection to a server has been successful, but no protocol
// has been initiated.
#define DWYCO_SE_SERVER_CONNECTION_SUCCESSFUL 5
// a connection to a server has been severed
#define DWYCO_SE_SERVER_DISCONNECT 6
// connect to server successful, user is authenticated and can perform
// commands
#define DWYCO_SE_SERVER_LOGIN 7
// connect is successful, but protocol failed for some reason
#define DWYCO_SE_SERVER_LOGIN_FAILED 8
// a zap has been received from the user (could be either notification of
// a new server summary, or a direct msg)
#define DWYCO_SE_USER_MSG_RECEIVED 9
//
// emitted when a uid is resolved to a name internally
#define DWYCO_SE_USER_UID_RESOLVED 10
//
// emitted when a users profile has changed or been deleted
#define DWYCO_SE_USER_PROFILE_INVALIDATE 11
// emitted when new messages are added or removed from the message idx for uid
#define DWYCO_SE_USER_MSG_IDX_UPDATED 12
// special case, one msg is prepended to index at index 0
#define DWYCO_SE_USER_MSG_IDX_UPDATED_PREPEND 13

// message send status
#define DWYCO_SE_MSG_SEND_START 14
#define DWYCO_SE_MSG_SEND_FAIL 15
#define DWYCO_SE_MSG_SEND_SUCCESS 16
#define DWYCO_SE_MSG_SEND_STATUS 17
#define DWYCO_SE_MSG_SEND_DELIVERY_SUCCESS 18
#define DWYCO_SE_MSG_SEND_CANCELED 19

// message receive status
#define DWYCO_SE_MSG_DOWNLOAD_START 20
#define DWYCO_SE_MSG_DOWNLOAD_FAILED 21
#define DWYCO_SE_MSG_DOWNLOAD_FETCHING_ATTACHMENT 22
#define DWYCO_SE_MSG_DOWNLOAD_ATTACHMENT_FETCH_FAILED 23
#define DWYCO_SE_MSG_DOWNLOAD_OK 24
#define DWYCO_SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED 26
#define DWYCO_SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED_DECRYPT_FAILED 27

#define DWYCO_SE_CHAT_SERVER_CONNECTING 28
#define DWYCO_SE_CHAT_SERVER_CONNECTION_SUCCESSFUL 29
#define DWYCO_SE_CHAT_SERVER_DISCONNECT 30
#define DWYCO_SE_CHAT_SERVER_LOGIN 31
#define DWYCO_SE_CHAT_SERVER_LOGIN_FAILED 32

#define DWYCO_SE_MSG_DOWNLOAD_PROGRESS 33


void DWYCOEXPORT dwyco_set_system_event_callback(DwycoSystemEventCallback cb);

#ifdef DWYCO_ASSHAT
int DWYCOEXPORT dwyco_get_ah(const char *uid, int len_uid, char out[3]);
// use this version if you just want an integer back.
// returns -1 if the asshat factor isn't valid yet
// returns -2 if the users isn't registered and the trial as expired
// otherwise returns an integer between 0 and 99 (inclusive)
int DWYCOEXPORT dwyco_get_ah2(const char *uid, int len_uid);
#endif

void DWYCOEXPORT dwyco_trace_init();
void DWYCOEXPORT dwyco_field_debug(const char *var, int num);
void DWYCOEXPORT dwyco_debug_dump();
void DWYCOEXPORT dwyco_app_debug1(const char *fn, int line, const char *fmt, int a1, int a2);
void DWYCOEXPORT dwyco_app_debug2(const char *fn, int line, const char *fmt, int a1, int a2, int a3, int a4, int a5);
// you must call dwyco_free_array on the returned pointer
void DWYCOEXPORT dwyco_random_string2(char **str_out, int len);
// you must call dwyco_free_array on the returned pointer
void DWYCOEXPORT dwyco_eze2(const char *str, int len_str, char **out, int *len_out);
// you must call dwyco_free_array on the returned pointer
void DWYCOEXPORT dwyco_ezd2(const char *str, int len_str, char **out, int *len_out);

int DWYCOEXPORT dwyco_set_profile_from_composer(int compid, const char *text, int len_text,
        DwycoProfileCallback cb, void *arg);

// the interface for this call is a bit odd.
// the callback will eventually be called with the "succ"
// parameter either
// * 0 (for failed)
// * -1 (for success, but there is no attachment available, just text in "s1")
// * -2 (for success, but the attachment is a user speced file)
// * some number not 0 or -1, which will be a viewer id that can be used with the "zap_view"
//	functions to play back the attachment for the profile. note that this will only work if you
// used the "set_profile_from_composer" function to create the profile (or it was
// created by cdc32)
int DWYCOEXPORT dwyco_get_profile_to_viewer(const char *uid, int len_uid, DwycoProfileCallback cb, void *arg);
// synchronous version. this returns whatever is currently in the cache
// without querying the server. the return value is the same as the callback above.
// the fn_out will be filled in with a fully usable filename if the return is -2.
// you must call dwyco_free_array when you are done with fn_out
//
int DWYCOEXPORT dwyco_get_profile_to_viewer_sync(const char *uid, int len_uid, char **fn_out, int *len_fn_out);
// not impl.
int DWYCOEXPORT dwyco_remove_profile(DwycoProfileCallback cb, void *arg);
int DWYCOEXPORT dwyco_update_profile(const char *text, int len_text, DwycoProfileCallback cb, void *arg);
// end not impl.

// these are used on first-run to set up a users profile before they are connected to a server
int DWYCOEXPORT dwyco_create_bootstrap_profile(const char *handle, int len_handle, const char *desc, int len_desc, const char *loc, int len_loc, const char *email, int len_email);
// note: output is to static area, must be copied out immediately
int DWYCOEXPORT dwyco_make_profile_pack(const char *handle, int len_handle, const char *desc, int len_desc, const char *loc, int len_loc, const char *email, int len_email, const char **str_out, int *len_str_out);

//
// WARNING: this char-by-char chat interface isn't used any more, as most
// applications these days do line-by-line chat.
//
// special char input stuff. since every
// framework has a different way of handling
// keyboard input, it is the callers responsibility
// to map the framework stuff into the following
// command set that the core can send over the network
// and reconstruct on the other side.
// NOTE: you have to test RECONSTRUCTION ON ALL DIFFERENT CLIENTS, not
// just a client that you wrote.
//
//  commands:
//	k char - insert char at current cursor location
//		note: \r, \n, \b may have to be handled specially on the receiver
//		since these a lot of times do whacky things. this also means
//		that deleting one of these chars may need special handling.
//	m loc - move the cursor to the given integer location (char position)
//	d - delete the character at the current location or the selection
//	s pos1 pos2 - select the chars starting at pos1 and going to and including pos2
//	c - clear the entire buffer and reposition to the beginning
//	p string - paste the string into the buffer at the current loc, replacing selected text
//	b - ring bell on other side
//	u arg1 arg1 str - user-defined

void DWYCOEXPORT dwyco_command_from_keyboard(int chan_id, int com,
        int arg1, int arg2, const char *str, int len);

// these are line-by-line public chat functions that are used if you create a
// connection using the real-time calling "connect_all" call and ask for a public-chat
// channel. they are not used when dealing with the chat server.
void DWYCOEXPORT dwyco_line_from_keyboard(int chan_id, const char *line, int len);
int DWYCOEXPORT dwyco_selective_chat_recipient_enable(const char *uid, int len_uid, int enable);
int DWYCOEXPORT dwyco_selective_chat_enable(int e);
void DWYCOEXPORT dwyco_reset_selective_chat_recipients();
int DWYCOEXPORT dwyco_is_selective_chat_recipient(const char *uid, int len_uid);

void DWYCOEXPORT dwyco_destroy_channel(int chan_id);
//void DWYCOEXPORT dwyco_destroy_by_ui_id(int ui_id);
void DWYCOEXPORT dwyco_hangup_all_calls();
void DWYCOEXPORT dwyco_pause_all_channels(int);
// if either pause_* args are -1, that state is left unchanged
int DWYCOEXPORT dwyco_pause_channel_media_set(int chan_id, int pause_video, int pause_audio);
int DWYCOEXPORT dwyco_pause_channel_media_get(int chan_id, int *pause_video_out, int *pause_audio_out);
int DWYCOEXPORT dwyco_remote_pause_channel_media_get(int chan_id, int *pause_video_out, int *pause_audio_out);

void DWYCOEXPORT dwyco_set_channel_destroy_callback(int chan_id,
        DwycoChannelDestroyCallback cb, void *user_arg);

// message send callback status
#define DWYCO_MSG_SEND_OK 1
#define DWYCO_MSG_SEND_FAILED 2
#define DWYCO_MSG_IGNORED 3
#define DWYCO_MSG_ATTACHMENT_DECLINED 4
#define DWYCO_MSG_ATTACHMENT_FAILED 5

// message download (from server) status
#define DWYCO_MSG_DOWNLOAD_FAILED 0
#define DWYCO_MSG_DOWNLOAD_SAVE_FAILED 1
#define DWYCO_MSG_DOWNLOAD_FETCHING_ATTACHMENT 3
#define DWYCO_MSG_DOWNLOAD_ATTACHMENT_FETCH_FAILED 4
#define DWYCO_MSG_DOWNLOAD_OK 5
#define DWYCO_MSG_DOWNLOAD_RATHOLED 6
#define DWYCO_MSG_DOWNLOAD_DECRYPT_FAILED 7

// autoupdate download status
#define DWYCO_AUTOUPDATE_SIGNATURE_FAILED 0
#define DWYCO_AUTOUPDATE_DOWNLOAD_FAILED 1
#define DWYCO_AUTOUPDATE_RUN_FAILED 2
// NOTE: on Windows, if this status is returned
// to the callback, the autoupdate has been started
// and is waiting on a MUTEX that is created when dwyco_init
// is called.
// The autoupdate process waits in the
// background until the mutex is released, and then
// performs whatever update it needs to. This is
// primarily because you can't update an executable
// while it is being used under Windows
#define DWYCO_AUTOUPDATE_IN_PROGRESS 3

#ifndef DWYCO_AUTOUPDATE_MUTEX_NAME
#define DWYCO_AUTOUPDATE_MUTEX_NAME "dwyco cdcx mutex2"
#endif

// autoupdate check status

// by convention, "AVAILABLE" means an optional update (ie, user
// should click a button to do the update.)
// the COMPULSORY1 should be automatically start the download
// in full view of the user at program startup, and do the update
// when the download finishes. this gives maximal opportunity for
// the update to work in situations where the program may be
// crashing due to some nasty bug. there should be no way for the user
// to cancel the update.
// COMPULSORY2 should download in the background automatically
// without notifying the user, then either at program shutdown or
// some other convenient time, perform the update.
#define DWYCO_AUTOUPDATE_CHECK_FAILED 0
#define DWYCO_AUTOUPDATE_CHECK_NOT_NEEDED 1
#define DWYCO_AUTOUPDATE_CHECK_AVAILABLE 2
#define DWYCO_AUTOUPDATE_CHECK_AVAILABLE_COMPULSORY1 3
#define DWYCO_AUTOUPDATE_CHECK_AVAILABLE_COMPULSORY2 4
#define DWYCO_AUTOUPDATE_CHECK_USER1 5
#define DWYCO_AUTOUPDATE_CHECK_USER2 6


// these are returned from
// dwyco_authenticate_body (ORed together)
// NOTE: because some old messages
// have no authentication AUTH_OK
// can be or'ed with NO_INFO.
// which would warrant a warning to
// the user. also note, if a message
// is a composite message (like it was
// forwarded), these bits are set
// independently for EACH component.
// so you might have a string of "ok"
// parts, but one is corrupted so both
// AUTH_OK and AUTH_FAILED could be set
// at the same time.
#define DWYCO_VERF_AUTH_NO_INFO 0x1
#define DWYCO_VERF_AUTH_FAILED 0x2
#define DWYCO_VERF_AUTH_OK 0x4


void DWYCOEXPORT dwyco_get_my_uid(const char **uid_out, int *len_out);
#define DWYCO_VIDEO_PREVIEW_CHAN 1
int DWYCOEXPORT dwyco_enable_video_capture_preview(int on);
void DWYCOEXPORT dwyco_add_entropy_timer(const char *crap, int crap_len);

int DWYCOEXPORT dwyco_get_refresh_users();
void DWYCOEXPORT dwyco_set_refresh_users(int);
int DWYCOEXPORT dwyco_get_rescan_messages();
void DWYCOEXPORT dwyco_set_rescan_messages(int);
int DWYCOEXPORT dwyco_uid_online(const char *uid, int len_uid);
// this returns:
// 0 for offline
// 1 for online, not available
// 3 for online, available
int DWYCOEXPORT dwyco_uid_status(const char *uid, int len_uid);
void DWYCOEXPORT dwyco_uid_to_ip(const char *uid, int len_uid, int *can_do_direct, char **str_out);
int DWYCOEXPORT dwyco_uid_to_ip2(const char *uid, int len_uid, int *can_do_direct_out, char **str_out);
int DWYCOEXPORT dwyco_uid_g(const char *uid, int len_uid);
int DWYCOEXPORT dwyco_load_users();
int DWYCOEXPORT dwyco_load_users2(int recent, int *total_out);
int DWYCOEXPORT dwyco_get_user_list2(DWYCO_USER_LIST *list_out, int *nelems_out);
int DWYCOEXPORT dwyco_get_message_index(DWYCO_MSG_IDX *list_out, const char *uid, int len_uid);
int DWYCOEXPORT dwyco_get_message_index2(DWYCO_MSG_IDX *list_out, const char *uid, int len_uid, int *available_count_out, int load_count);
int DWYCOEXPORT dwyco_get_new_message_index(DWYCO_MSG_IDX *list_out, const char *uid, int len_uid, long logical_clock);
int DWYCOEXPORT dwyco_get_message_bodies(DWYCO_SAVED_MSG_LIST *list_out, const char *uid, int uid_len, int load_sent);
int DWYCOEXPORT dwyco_get_unfetched_messages(DWYCO_UNFETCHED_MSG_LIST *list_out, const char *uid, int len_uid);
int DWYCOEXPORT dwyco_get_unfetched_message(DWYCO_UNFETCHED_MSG_LIST *list_out, const char *msg_id);
// this doesn't make sense anymore
//int DWYCOEXPORT dwyco_unsaved_message_to_body(DWYCO_SAVED_MSG_LIST *list_out, const char *msg_id);
int DWYCOEXPORT dwyco_delete_unfetched_message(const char *msg_id);
int DWYCOEXPORT dwyco_delete_saved_message(const char *user_id, int len_uid, const char *msg_id);
int DWYCOEXPORT dwyco_save_message(const char *msg_id);
int DWYCOEXPORT dwyco_get_saved_message(DWYCO_SAVED_MSG_LIST *list_out, const char *user_id, int len_uid, const char *msg_id);
int DWYCOEXPORT dwyco_fetch_server_message(const char *msg_id, DwycoMessageDownloadCallback cb, void *mdc_arg1,
        DwycoStatusCallback scb, void *scb_arg1);
void DWYCOEXPORT dwyco_cancel_message_fetch(int fetch_id);

// lists of messages that are q'd for send
void DWYCOEXPORT dwyco_get_qd_messages(DWYCO_QD_MSG_LIST *list_out, const char *uid, int len_uid);
int DWYCOEXPORT dwyco_qd_message_to_body(DWYCO_SAVED_MSG_LIST *list_out, const char *pers_id, int len_pers_id);


// get_body_text returns a list with a single row (=0)
// in it, and no columns. the item in the list is
// the formatted text of the message, which may include
// forwarded messages. the formatting is simple and fixed. see get_body_array
// if you need more control over message formatting.
DWYCO_LIST DWYCOEXPORT dwyco_get_body_text(DWYCO_SAVED_MSG_LIST m);
//
// get_body_array returns a list of lists
// in the same way the directory functions do, except
// in this case, each entry in the list is a message.
// each message is a forwarded component, with the latest
// component being at index 0, and earlier components coming
// in order at larger indexes.
// if the message has no forwarded parts,
// then just the top level message will be in the list.
// this allows you to format forwarded message in any
// way you want (instead of just being stuck with the
// formatting provided by dwyco_get_body_text.)
// each message is formatted in exactly the same way
// as a DWYCO_QM_BODY_...
//
DWYCO_LIST DWYCOEXPORT dwyco_get_body_array(DWYCO_SAVED_MSG_LIST m);
int DWYCOEXPORT dwyco_authenticate_body(DWYCO_SAVED_MSG_LIST m, const char *recip_uid, int len_uid, int unsaved);
void DWYCOEXPORT dwyco_fetch_info(const char *uid, int len_uid);

void DWYCOEXPORT dwyco_pal_add(const char *user_id, int len_uid);
void DWYCOEXPORT dwyco_pal_delete(const char *user_id, int len_uid);
int DWYCOEXPORT dwyco_is_pal(const char *user_id, int len_uid);
DWYCO_LIST DWYCOEXPORT dwyco_pal_get_list();
void DWYCOEXPORT dwyco_pal_relogin();
int DWYCOEXPORT dwyco_get_pal_logged_in();

void DWYCOEXPORT dwyco_set_fav_msg(const char *mid, int fav);
int DWYCOEXPORT dwyco_get_fav_msg(const char *mid);
// this clears all the messages for a user except for ones that are "favorites"
int DWYCOEXPORT dwyco_clear_user_unfav(const char *uid, int len_uid);

void DWYCOEXPORT dwyco_set_msg_tag(const char *mid, const char *tag);
void DWYCOEXPORT dwyco_unset_msg_tag(const char *mid, const char *tag);
void DWYCOEXPORT dwyco_unset_all_msg_tag(const char *tag);

// WARNING: the uid is returned in ASCII hex instead of binary like the
// rest of the interface. this api is a bit sketchy so i'm not sure i want
// to fix it. also note, setting a (tag, mid) pair will not be returned
// here, because there wouldn't be an associated uid...
#define DWYCO_TAGGED_MIDS_MID "001"
#define DWYCO_TAGGED_MIDS_HEX_UID "000"
int DWYCOEXPORT dwyco_get_tagged_mids(DWYCO_LIST *list_out, const char *tag);

int DWYCOEXPORT dwyco_get_tagged_idx(DWYCO_MSG_IDX *list_out, const char *tag);
int DWYCOEXPORT dwyco_mid_has_tag(const char *mid, const char * tag);
int DWYCOEXPORT dwyco_uid_has_tag(const char *uid, int len_uid, const char *tag);
int DWYCOEXPORT dwyco_uid_count_tag(const char *uid, int len_uid, const char *tag);
int DWYCOEXPORT dwyco_count_tag(const char *tag);

void DWYCOEXPORT dwyco_set_alert(const char *uid, int len_uid, int val);
int DWYCOEXPORT dwyco_get_alert(const char *uid, int len_uid);

#if 0
// NOTE: ca nov 2009, pal authorization is defunct in favor
// of "pals only" filtering and mutual ignore
//
// NOTE: callback "succ" arg has special meaning:
// 1: user requires authorization
// 0: FAILED (couldn't get state for some reason)
// -1: user doesn't require authorization
void DWYCOEXPORT dwyco_get_pal_auth_state(const char *uid, int len_uid, DwycoCommandCallback cb, void *user_arg);
// sets our pal auth state in the server so others
// know what our preference is.
// state is:
// 1: requires authorization
// 0: doesn't require authorization
void DWYCOEXPORT dwyco_set_pal_auth_state(int state, DwycoCommandCallback cb, void *user_arg);
// gets the local pal auth state: 1=requires auth, 0=not
int DWYCOEXPORT dwyco_get_my_pal_auth_state();
// this will return 1 if the authorization list you have
// does not match your id# (ie, you changed your id#
// by changing the auth file or something.)
// when this happens, the user should have the option
// of removing all the authorizations (they won't work
// anymore), or leaving them.
int DWYCOEXPORT dwyco_get_pal_auth_warning();
int DWYCOEXPORT dwyco_pal_auth_granted(const char *uid, int len_uid);
void DWYCOEXPORT dwyco_revoke_pal_auth(const char *uid, int len_uid);
void DWYCOEXPORT dwyco_clear_pal_auths();
// msg_id should refer to a local message that is
// a special pal auth message.
// call this function in response to receiving a
// pal authorization request and getting a positive user response:
// suggestions for buttons on request dialog:
// YES = grant authorization, but do not take advantage of
// reverse authorization (ie, don't put them on your list.)
// YES+ADD = grant authorization and accept pre-authorization from user. (set add_them to 1)
// NO = do not call this function. just delete message and do nothing.
// NO+IGNORE = do not call this function. delete message and ignore the user.
//
// also, call this function when you get a "palrej" message
//
// if uid == 0, msg_id must refer to an unsaved msg that has been fetched
// from the server.
// if uid != 0, msg_id must refer to a saved msg from uid (NOTE: this is broken)
int DWYCOEXPORT dwyco_handle_pal_auth(const char *uid, int len_uid, const char *msg_id, int add_them);
int DWYCOEXPORT dwyco_handle_pal_auth2(DWYCO_UNSAVED_MSG_LIST ml, int add_them);
#endif

int DWYCOEXPORT dwyco_is_ignored(const char *user_id, int len_uid);
void DWYCOEXPORT dwyco_ignore(const char *user_id, int len_uid);
void DWYCOEXPORT dwyco_unignore(const char *user_id, int len_uid);
void DWYCOEXPORT dwyco_session_ignore(const char *user_id, int len_uid);
void DWYCOEXPORT dwyco_session_unignore(const char *user_id, int len_uid);
#if 0
void DWYCOEXPORT dwyco_always_visible(const char *uid, int len_uid, int val);
void DWYCOEXPORT dwyco_never_visible(const char *uid, int len_uid, int val);
int DWYCOEXPORT dwyco_is_never_visible(const char *uid, int len_uid);
int DWYCOEXPORT dwyco_is_always_visible(const char *uid, int len_uid);
#endif
DWYCO_LIST DWYCOEXPORT dwyco_ignore_list_get();
DWYCO_LIST DWYCOEXPORT dwyco_session_ignore_list_get();

// pals-only filtering
// if on == 1, then filtering is turned on, and all
// messages and calls will be rejected from users not on the pal list.
// if on == 0, then no pal filtering is done. to test: turning pal
// filtering on/off while there are q'd messages is likely to be
// broken in some way (ie, the filtering is done when the msg is
// received, not when it is accessed.)
// in addition, this function informs chat-local users of your
// pal-filtering state.
void DWYCOEXPORT dwyco_set_pals_only(int on);
int DWYCOEXPORT dwyco_get_pals_only();
// To toggle sending auto-replies, see set_zap_data/send_auto_reply.
//
// By default, if there is no auto-reply message set, the dll will send
// a default text auto-reply stating the user is only accepting calls and
// messages from pals.
// This call makes it possible for the message to be any zap message.
// The text and media referenced by the composer are saved into a local
// message that is stored as a "sent" message in the current users own
// message box. It becomes a normal message, and the msgid is stored
// by the DLL to be used for auto-replies. To form the auto-reply, the
// DLL forwards the message to the recipient.
// To revert back to the default auto-reply, call this function with
// "text" == 0 or compid == 0.
// The message is not removed, just the msgid refering to it.
// There is no way to reinstate a previous message with this simple api at the moment.
// There is no way to remove the message other than the usual remove message call
// There is no way to get the msgid.
// obviously, this api needs to be improved a bit to make it possible to
// provide an easier UI.
int DWYCOEXPORT dwyco_set_auto_reply_msgNA(const char *text, int len_text, int compid);

DWYCO_LIST DWYCOEXPORT dwyco_uid_to_info(const char *user_id, int len_uid, int *cant_resolve_now);
int DWYCOEXPORT dwyco_delete_user(const char *uid, int uid_len);
int DWYCOEXPORT dwyco_clear_user(const char *uid, int len_uid);

#if defined(_WIN32) || defined(_Windows)
int DWYCOEXPORT dwyco_is_capturing_video();
#endif

void DWYCOEXPORT dwyco_set_moron_dork_mode(int);
int DWYCOEXPORT dwyco_get_moron_dork_mode();

void DWYCOEXPORT dwyco_network_diagnostics2(char **res, int *len_res);
// results are in BITS/second, you can leave any of these pointers NULL
// if you don't need that result.
void DWYCOEXPORT dwyco_estimate_bandwidth2(int *out_bw, int *in_bw);

void DWYCOEXPORT dwyco_gen_pass(const char *pw, int len_pw, char **salt_in_out, int *len_salt_in_out, char **hash_out, int *len_hash_out);

void DWYCOEXPORT dwyco_free(char *p);
void DWYCOEXPORT dwyco_free_array(char *p);
void DWYCOEXPORT dwyco_free_image(char *p, int rows);
void DWYCOEXPORT dwyco_finish_startup();
int DWYCOEXPORT dwyco_get_create_new_account();
//int DWYCOEXPORT dwyco_check_password_against_local_hash(const char *pw, int len_pw);
void DWYCOEXPORT dwyco_set_local_auth(int a);


// there are some access functions for
// getting at the components of these
// lists. they are accessed basically like
// a 2-d array. all results are strings that
// must be copied out.

void DWYCOEXPORT dwyco_list_release(DWYCO_LIST l);
int DWYCOEXPORT dwyco_list_numelems(DWYCO_LIST l, int *rows, int *cols);
int DWYCOEXPORT dwyco_list_get(DWYCO_LIST l, int row, const char *col,
                               const char **out, int *len_out, int *type_out);
int DWYCOEXPORT dwyco_list_print(DWYCO_LIST l);
DWYCO_LIST DWYCOEXPORT dwyco_list_copy(DWYCO_LIST l);

// simple serialization of a single column DWYCO_LIST
DWYCO_LIST DWYCOEXPORT dwyco_list_new();
void DWYCOEXPORT dwyco_list_append(DWYCO_LIST l, const char *val, int len, int type);
void DWYCOEXPORT dwyco_list_append_int(DWYCO_LIST l, int i);
// must call dwyco_free_array on returned string after copying out
void DWYCOEXPORT dwyco_list_to_string(DWYCO_LIST l, const char **str_out, int *len_out);
int DWYCOEXPORT dwyco_list_from_string(DWYCO_LIST *list_out, const char *str, int len_str);

// types returned by get_list
#define DWYCO_TYPE_NIL 0
#define DWYCO_TYPE_STRING 1
#define DWYCO_TYPE_INT 2
#define DWYCO_TYPE_VECTOR 3

// use this for dwyco_lists that are a
// single column
#define DWYCO_NO_COLUMN ""

// return from dwyco_uid_to_info
#define DWYCO_INFO_HANDLE "000"
#define DWYCO_INFO_LOCATION "001"
#define DWYCO_INFO_DESCRIPTION "002"
#define DWYCO_INFO_REVIEWED "003"
#define DWYCO_INFO_REGULAR "004"
#define DWYCO_INFO_EMAIL "005"

// return from dwyco_get_vfw_drivers
#define DWYCO_VFW_DRIVER_INDEX "000"
#define DWYCO_VFW_DRIVER_NAME "001"
#define DWYCO_VFW_DRIVER_VERSION "002"

// index strings for various data structures used by the core.
// Server list records
#define DWYCO_SL_SERVER_HOSTNAME "000"
#define DWYCO_SL_SERVER_IP "001"
#define DWYCO_SL_SERVER_PORT "002"
#define DWYCO_SL_SERVER_RATING "003"
#define DWYCO_SL_SERVER_NAME "004"
#define DWYCO_SL_SERVER_CATEGORY "005"

// Message summary records
// 0: who from
// 1: len
// 2: id on server
// 3: date vector
#define DWYCO_QMS_FROM "000"
#define DWYCO_QMS_LEN "001"
#define DWYCO_QMS_ID "002"
// the broken-out time is dead now, just the unix time is sent, the
// rest are nil
#if 0
#define DWYCO_QMS_DATE_SENT "003"
#define DWYCO_QMS_DS_YEAR "003000"
#define DWYCO_QMS_DS_JDAY "003001"
#endif
// this is the local time, for display mostly since
// remote clients would have a hard time calculating this
#define DWYCO_QMS_DS_HOUR "003002"
#define DWYCO_QMS_DS_MINUTE "003003"
#define DWYCO_QMS_DS_SECOND "003004"
#define DWYCO_QMS_DS_SECONDS_SINCE_JAN1_1970 "003005"
// added locally:
//#define DWYCO_QMS_PENDING_DEL "004"
#define DWYCO_QMS_IS_DIRECT "005"
// oops, server has to put this way out here because
// old software expects to see the above structure...
// 6: rating of sender
//#define DWYCO_QMS_SENDER_RATING "006"
//#define DWYCO_QMS_ONLINE "007"
//#define DWYCO_QMS_FW_INFO "008"
#define DWYCO_QMS_SPECIAL_TYPE "009"
#define DWYCO_QMS_LOGICAL_CLOCK "011"

// Message bodies after they are saved locally
#define DWYCO_QM_BODY_ID "000"
#define DWYCO_QM_BODY_FROM "001"
//#define DWYCO_QM_BODY_TEXT_do_not_use_use_get_msg_array_instead "002"
//#define DWYCO_QM_BODY_TEXT2_obsolete "002"
#define DWYCO_QM_BODY_ATTACHMENT "003"
#if 0
#define DWYCO_QM_BODY_DATE "004"
#define DWYCO_QM_BODY_DATE_YEAR "004000"
#define DWYCO_QM_BODY_DATE_JDAY "004001"
#endif
// local time sent
#define DWYCO_QM_BODY_DATE_HOUR "004002"
#define DWYCO_QM_BODY_DATE_MINUTE "004003"
#define DWYCO_QM_BODY_DATE_SECOND "004004"
#define DWYCO_QM_BODY_DATE_SECONDS_SINCE_JAN1_1970 "004005"
// locally set
#define DWYCO_QM_BODY_SENT "005"
// from server
//#define DWYCO_QM_BODY_RATING "006"
//#define DWYCO_QM_BODY_TIME_RECV "007" (defunct)
//#define DWYCO_QM_BODY_AUTOPLAY "008"  (defunct, never used)

// there are some subelements in here, but they aren't needed by client
#define DWYCO_QM_BODY_AUTH_VEC "007"
#define DWYCO_QM_BODY_FORWARDED_BODY "008"
//#define DWYCO_QM_BODY_NEW_TEXT_do_not_use_directly_use_get_msg_array "009"
#define DWYCO_QM_BODY_NEW_TEXT2 "009"
#define DWYCO_QM_BODY_SPECIAL_TYPE_VECTOR "010"
// this is palreq, palok, palrej, or "user"
#define DWYCO_QM_BODY_SPECIAL_TYPE_TYPE "010000"
// for palreq, this is the "preauth" cookie added by the server
// for palok, this is the cookie you add that indicated they GRANTED pal auth
// for palrej, this is nothing
#define DWYCO_QM_BODY_SPECIAL_TYPE_BA "010001000"
// for palreq, this is empty
// for palok, this is the cookie you add to indicate YOU granted THEM pal auth
// for palrej, this is nothing
#define DWYCO_QM_BODY_SPECIAL_TYPE_AB "010001001"

#define DWYCO_QM_BODY_FILE_ATTACHMENT "012"
#define DWYCO_QM_BODY_LOGICAL_CLOCK "017"

// DWYCO_MSG_IDX is an index of the saved messages for a particular UID.
// The index is mostly-sorted in order of descending date.
// This index is fixed at fetch time, and can be used for browsing
// and displaying msgs using views. the sort is by order of a logical
// clock, with the date being used as a backup if the logical clock on the
// message isn't available. the logical clock provides better ordering when the
// dates on the two computers sending messages are not the same.
// note: ca 2018, ASSOC_UID is set to who a message is from (or to, for sent msgs)
// this is for indexes that refer to messages for multiple UID, like
// indexes derived from a tag, or group message.

#define DWYCO_MSG_IDX_DATE "000"
#define DWYCO_MSG_IDX_MID "001"
#define DWYCO_MSG_IDX_IS_SENT "002"
#define DWYCO_MSG_IDX_IS_FORWARDED "003"
#define DWYCO_MSG_IDX_IS_NO_FORWARD "004"
#define DWYCO_MSG_IDX_IS_FILE "005"
#define DWYCO_MSG_IDX_SPECIAL_TYPE "006"
#define DWYCO_MSG_IDX_HAS_ATTACHMENT "007"
#define DWYCO_MSG_IDX_ATT_HAS_VIDEO "008"
#define DWYCO_MSG_IDX_ATT_HAS_AUDIO "009"
#define DWYCO_MSG_IDX_ATT_IS_SHORT_VIDEO "010"
#define DWYCO_MSG_IDX_LOGICAL_CLOCK "011"
#define DWYCO_MSG_IDX_ASSOC_UID "012"
#define DWYCO_MSG_IDX_IS_DELIVERED "013"
#define DWYCO_MSG_IDX_IS_VIEWED "014"


// DWYCO_QD_MSG_LIST, list of messages that are not sent yet.
#define DWYCO_QD_MSG_RECIPIENT "000"
#define DWYCO_QD_MSG_PERS_ID "001"

// Pal authorization and special message passing options
// For pal authorization, dwyco defines the special messages
// for the various requests.
// User defined requests are passed with an id
// that should be checked by the receiver, and ignored
// if it doesn't understand it.
//

// these are used as arguments to the
// dwyco_make_special_zap_composition
#define DWYCO_PAL_AUTH_ASK 1
#define DWYCO_PAL_AUTH_REJECT 2
#define DWYCO_PAL_AUTH_ACCEPT 3
#define DWYCO_SPECIAL_TYPE_USER 4
#define DWYCO_SPECIAL_TYPE_BACKUP 5
#define DWYCO_SPECIAL_TYPE_DELIVERED 6
#define DWYCO_SPECIAL_TYPE_VIEWED 7

// the following id's show up in
// the message summary field DWYCO_QMS_BODY_SPECIAL_TYPE,
// and saved message field DWYCO_QM_BODY_SPECIAL_TYPE
// for pal authorization message.
// for user-defined special types, the id you
// send into make_special_zap_composition
// show up instead. you shouldn't use these, they are low
// level. use the constants below and the
// "is_special" functions
#define DWYCO_PAL_AUTH_ASK_ID "palreq"
#define DWYCO_PAL_AUTH_REJECT_ID "palrej"
#define DWYCO_PAL_AUTH_ACCEPT_ID "palok"

// returns 1 if it is special, and what_out will
// be set to one of the following
// if uid == 0, msg_id must refer to an unsaved msg. if the msg hasn't been
// fetched from the server, what_out will be one of the *SUMMARY* types.
// if uid != 0, msg_id must refer to a saved msg from uid (NOTE: THIS IS BROKEN)
int DWYCOEXPORT dwyco_is_special_message(const char *uid, int len_uid, const char *msg_id, int *what_out);
int DWYCOEXPORT dwyco_is_special_message2(DWYCO_UNFETCHED_MSG_LIST ml, int *what_out);
int DWYCOEXPORT dwyco_get_user_payload(DWYCO_UNFETCHED_MSG_LIST ml, const char **str_out, int *len_out);

// "what" returns from the dwyco_is_special_message function
#define DWYCO_SUMMARY_PAL_AUTH_REQ 0
#define DWYCO_SUMMARY_PAL_OK 1
#define DWYCO_SUMMARY_PAL_REJECT 2
#define DWYCO_SUMMARY_SPECIAL_USER_DEFINED 3
#define DWYCO_PAL_AUTH_REQ 4
#define DWYCO_PAL_OK 5
#define DWYCO_PAL_REJECT 6
#define DWYCO_SPECIAL_USER_DEFINED 7
// note: these messages are never fetched, the summaries
// exist in the server message list, and are just deleted
// when they are no longer needed. the summary is generated by
// the server, and cannot be generated by clients.
// note: VIEWED implies DELIVERED, so you avoid
// sending both.
#define DWYCO_SUMMARY_DELIVERED 8
#define DWYCO_SUMMARY_VIEWED 9

int DWYCOEXPORT dwyco_is_delivery_report(const char *mid, const char **uid_out, int *len_uid_out, const char **msg_id_out, int *what_out);


int DWYCOEXPORT dwyco_init();
int DWYCOEXPORT dwyco_exit();

// use these to do just background message sending
// you can ignore all callbacks, as this will happen
// internally. after a dwyco_bg_exit, the process should be
// on its way to exiting.
int DWYCOEXPORT dwyco_bg_init();
int DWYCOEXPORT dwyco_bg_exit();

void DWYCOEXPORT dwyco_power_clean_safe();
void DWYCOEXPORT dwyco_power_clean_progress_hack(int *done_out, int *total_out);
int DWYCOEXPORT dwyco_empty_trash();
int DWYCOEXPORT dwyco_count_trashed_users();
void DWYCOEXPORT dwyco_untrash_users();

// use these before going into suspend state (on a mobile device, for example)
// experimental.
void DWYCOEXPORT dwyco_suspend();
void DWYCOEXPORT dwyco_resume();

int DWYCOEXPORT dwyco_service_channels(int *spin);
void DWYCOEXPORT dwyco_set_client_version(const char *str, int len_str);
//void DWYCOEXPORT dwyco_set_login_password(const char *pw, int len_pw);
void DWYCOEXPORT dwyco_set_login_result_callback(DwycoServerLoginCallback cb);
void DWYCOEXPORT dwyco_database_login();
int DWYCOEXPORT dwyco_database_online();
int DWYCOEXPORT dwyco_chat_online();
int DWYCOEXPORT dwyco_database_auth_remote();
void DWYCOEXPORT dwyco_inhibit_database(int i);
void DWYCOEXPORT dwyco_inhibit_pal(int i);
void DWYCOEXPORT dwyco_inhibit_sac(int i);
void DWYCOEXPORT dwyco_inhibit_lanmap(int i);
//void DWYCOEXPORT dwyco_inhibit_chat(int i);

int DWYCOEXPORT dwyco_get_audio_hw(int *has_audio_input, int *has_audio_output, int *audio_hw_full_duplex);
int DWYCOEXPORT dwyco_set_all_mute(int);
int DWYCOEXPORT dwyco_get_all_mute();
int DWYCOEXPORT dwyco_set_exclusive_audio(int a, int recv_chan_id);
int DWYCOEXPORT dwyco_get_exclusive_audio(int *state_out, int *chan_id_out);
int DWYCOEXPORT dwyco_set_auto_squelch(int a);
int DWYCOEXPORT dwyco_get_auto_squelch();
void DWYCOEXPORT dwyco_set_full_duplex(int a);
int DWYCOEXPORT dwyco_get_full_duplex();
int DWYCOEXPORT dwyco_get_audio_output_in_progress();
int DWYCOEXPORT dwyco_get_squelched();
int DWYCOEXPORT dwyco_get_authenticator(const char **out, int *len_out);

// This function only affects the limit used by the call-q when
// setting up multiple queued calls. If you don't call this, the default
// is 4. NOTE: this does NOT affect the number of calls you can queue up.
// This limits the number of concurrent calls that are streaming that
// have originated from the call queue.
// This function returns the previous value of the limit.

int DWYCOEXPORT dwyco_set_max_established_originated_calls(int n);

// these are the "what" parameter values for the call disposition callback
#define DWYCO_CALLDISP_ESTABLISHED 1	// call is connected and accepted, use "chan_id" to destroy call
#define DWYCO_CALLDISP_FAILED 2			// call setup failed, never was able to connect to callee, ignore "chan_id"
#define DWYCO_CALLDISP_TERMINATED 3		// call which was "established" is now ended (from either side.)
// NOTE: you get the more specific REJECT codes below when the
// the DLL can figure that out. there are lots of reasons a call may be
// rejected related to device errors and stuff.
#define DWYCO_CALLDISP_REJECTED 4		// call connected, but call screening rejected the call on the callee side
#define DWYCO_CALLDISP_REJECTED_PASSWORD_INCORRECT 5 // call screened by password checking
#define DWYCO_CALLDISP_REJECTED_BUSY 6 // call screened by resource limitation, or specific user action
#define DWYCO_CALLDISP_CANCELED 7		// call setup was canceled locally before connect succeeded
#define DWYCO_CALLDISP_STARTED 8		// call initiated

// create a full duplex channel to uid.
int DWYCOEXPORT
dwyco_channel_create(const char *uid, int len_uid,
                     DwycoCallDispositionCallback cdc, void *cdc_arg1,
                     DwycoStatusCallback scb, void *scb1_arg1,
                     const char *pw,
                     const char *call_type, int len_call_type, int q_call);

int DWYCOEXPORT dwyco_channel_send_video(int chan_id, int vid_dev);
int DWYCOEXPORT dwyco_channel_stop_send_video(int chan_id);
int DWYCOEXPORT dwyco_channel_send_audio(int chan_id, int aud_dev);
int DWYCOEXPORT dwyco_channel_stop_send_audio(int chan_id);

// connect to a single uid, with optional q-ing
int DWYCOEXPORT
dwyco_connect_uid(const char *uid, int len_uid,
                  DwycoCallDispositionCallback cdc, void *cdc_arg1,
                  DwycoStatusCallback scb, void *scb1_arg1,
                  int send_video, int recv_video,
                  int send_audio, int recv_audio,
                  int private_chat, int public_chat,
                  const char *pw,
                  const char *call_type, int len_call_type, int q_call);

void DWYCOEXPORT
dwyco_connect_all4(const char **uid_list, int *len_uid_list, int num,
                   DwycoCallDispositionCallback cdc, void *cdc_arg1,
                   DwycoStatusCallback scb, void *scb1_arg1,
                   int send_video, int recv_video,
                   int send_audio, int recv_audio,
                   int private_chat, int public_chat,
                   const char *pw,
                   const char *call_type, int len_call_type, int q_calls);

int DWYCOEXPORT
dwyco_connect_msg_chan(const char *uid, int len_uid,
                       DwycoCallDispositionCallback cdc, void *cdc_arg1,
                       DwycoStatusCallback scb, void *scb_arg1);

void DWYCOEXPORT dwyco_cancel_call(int call_id);
int DWYCOEXPORT dwyco_chan_to_call(int chan_id);
int DWYCOEXPORT dwyco_channel_streams(int chan_id, int *send_video, int *recv_video, int *send_audio, int *recv_audio, int *pubchat, int *privchat);
int DWYCOEXPORT dwyco_send_user_control(const char *uid, int len_uid, const char *data, int len_data);


void DWYCOEXPORT dwyco_set_call_appearance_callback(DwycoCallAppearanceCallback cb);
void DWYCOEXPORT dwyco_set_call_acceptance_callback(DwycoCallAppearanceCallback cb);
void DWYCOEXPORT dwyco_set_zap_appearance_callback(DwycoZapAppearanceCallback cb);
void DWYCOEXPORT dwyco_set_call_appearance_death_callback(DwycoCallAppearanceDeathCallback cb);
int DWYCOEXPORT dwyco_call_accept(int id);
int DWYCOEXPORT dwyco_call_reject(int id, int session_ignore);
int DWYCOEXPORT dwyco_zap_accept(int id, int always_accept);
int DWYCOEXPORT dwyco_zap_reject(int id, int session_ignore);

int DWYCOEXPORT dwyco_get_server_list(DWYCO_SERVER_LIST *list_out, int *numlines);

// this returns 1 if it found the
// id, and 0 otherwise. the list returned will be a singleton list
// (1 row, no columns).
int DWYCOEXPORT dwyco_get_lobby_name_by_id2(const char *id, DWYCO_LIST *list_out);
//__declspec(dllexport) const char * DWYCOCALLCONV dwyco_get_lobby_name_by_id(const char *id);
int DWYCOEXPORT dwyco_switch_to_chat_server(int i);

// this can be used to check a lobby password without
// affecting current connections. you *must* be connected to
// some lobby for this to work.
// Return values:
// 0 = cid is invalid, or some other error. note that if you are not
// connected to a lobby, this will always return 0.
// 1 = lobby doesn't require a password
// 2 = lobby requires a password and password is ok
// -1 = lobby requires a password but supplied password is incorrect.

int DWYCOEXPORT dwyco_check_chat_server_pw(const char *cid, const char *pw);

// returns:
// -1 : cid is invalid, or not connected to a server
// 0 : cid does not require a password
// 1 : cid has a password set
int DWYCOEXPORT dwyco_chat_server_has_pw(const char *cid);

// Return values:
// 0 = cid is invalid, or some immediate resource problem connecting to server.
// in both cases, the current lobby info is still valid.
// -1 = password incorrect. the current lobby info and connection is unaffected
// 1 = switch is in progress, all current lobby info is invalidated until
// the server switch is completed successfully.
// if the server switch fails, you will have to switch back to a system
// server using switch_to_chat_server in order to revalidate user lobby info.
int DWYCOEXPORT dwyco_switch_to_chat_server2(const char *cid, const char *pw);
int DWYCOEXPORT dwyco_disconnect_chat_server();

// should only be called *before* dwyco_init
void DWYCOEXPORT dwyco_set_initial_invis(int invis);
// should only be called *after* dwyco_init
void DWYCOEXPORT dwyco_set_invisible_state(int invis);
int DWYCOEXPORT dwyco_get_invisible_state();

// message composition functions
int DWYCOEXPORT dwyco_make_zap_composition(char *must_be_zero);
int DWYCOEXPORT dwyco_make_zap_composition_raw(const char *filename, const char *possible_extension);
// WARNING: dup-ing should only be used in very specific cases.
int DWYCOEXPORT dwyco_dup_zap_composition(int compid);
int DWYCOEXPORT dwyco_make_forward_zap_composition(
    const char *uid, // must be 0 to forward unsaved message, uid for saved messages
    int len_uid,
    const char *msg_id,
    int strip_forward_text
);
int DWYCOEXPORT dwyco_make_special_zap_composition(
    int special_type,
    const char *user_id,
    const char *user_block,
    int len_user_block
);
int DWYCOEXPORT dwyco_set_special_zap(int compid, int special_type);

int DWYCOEXPORT
dwyco_make_file_zap_composition(
    const char *filename,
    int len_filename
);
int DWYCOEXPORT
dwyco_copy_out_file_zap(
    const char *uid,
    int len_uid,
    const char *msg_id,
    const char *dst_filename
);

int
DWYCOEXPORT
dwyco_copy_out_file_zap_buf(const char *uid, int len_uid, const char *msg_id, const char **buf_out, int *buf_len_out, int max_out);

int DWYCOEXPORT
dwyco_copy_out_unsaved_file_zap(DWYCO_UNFETCHED_MSG_LIST m, const char *dst_filename);

int DWYCOEXPORT dwyco_is_file_zap(int compid);

int DWYCOEXPORT dwyco_is_forward_composition(int compid);
int DWYCOEXPORT dwyco_flim(int compid);

int DWYCOEXPORT dwyco_delete_zap_composition(int compid);

int DWYCOEXPORT dwyco_zap_record(int compid, int video, int audio, int pic, int frames,
                                 DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *chan_id_out);

int DWYCOEXPORT dwyco_zap_record2(int compid, int video, int audio,
                                  int max_frames, int max_bytes, int hi_quality,
                                  DwycoStatusCallback scb, void *scb_arg1,
                                  DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *chan_id_out);

int DWYCOEXPORT dwyco_zap_composition_chan_id(int compid);

int DWYCOEXPORT dwyco_zap_stop(int compid);

int DWYCOEXPORT dwyco_zap_play(int compid,
                               DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *chan_id_out);

// multiple-recipient sends went away some time ago, you have
// to do it at the app level now with multiple contexts.

// you get status and progress messages as "DWYCO_SE*" system events.
// these messages are delivered for interrupted background sends too.
int DWYCOEXPORT dwyco_zap_send4(int compid, const char *uid, int len_uid, const char *text, int len_text, int no_forward, const char **pers_id_out, int *len_pers_id_out);
int DWYCOEXPORT dwyco_zap_send5(int compid, const char *uid, int len_uid, const char *text, int len_text, int no_forward, int save_sent, const char **pers_id_out, int *len_pers_id_out);
int DWYCOEXPORT dwyco_zap_send6(int compid, const char *uid, int len_uid, const char *text, int len_text, int no_forward, int save_sent, int defer, const char **pers_id_out, int *len_pers_id_out);

int DWYCOEXPORT dwyco_zap_cancel(int compid);
int DWYCOEXPORT dwyco_zap_still_active(int compid);
// this stops any send in progress, and then removes the message so it is not
// retried
int DWYCOEXPORT dwyco_kill_message(const char *pers_id, int len_pers_id);

// functions for just viewing a zap message attachment
int DWYCOEXPORT dwyco_make_zap_view(DWYCO_SAVED_MSG_LIST list, const char *recip_uid, int len_uid, int unsaved);
int DWYCOEXPORT dwyco_make_zap_view_file(const char *filename);
int DWYCOEXPORT dwyco_make_zap_view_file_raw(const char *filename);
int DWYCOEXPORT dwyco_delete_zap_view(int viewid);
int DWYCOEXPORT dwyco_zap_play_view(int viewid, DwycoChannelDestroyCallback dcb, void *dcb_arg1,
                                    int *chan_id_out);
int DWYCOEXPORT dwyco_zap_play_view_no_audio(int viewid, DwycoChannelDestroyCallback dcb, void *dcb_arg1,
        int *chan_id_out);
int DWYCOEXPORT dwyco_zap_play_preview(int viewid,
                                       DwycoChannelDestroyCallback dcb, void *dcb_arg1,
                                       int *chan_id_out);
int DWYCOEXPORT dwyco_zap_stop_view(int viewid);
int DWYCOEXPORT dwyco_zap_quick_stats_view(int viewid, int *has_video, int *has_audio, int *short_video);
int DWYCOEXPORT dwyco_zap_create_preview(int viewid, const char *filename, int len_filename);
// note: this is a hack, since the buf that comes back is really a ppm
// (which isn't a straight block of memory), we can't really dump the
// contents of buf_out in a generic way, so to avoid crashing the
// debug stuff, i just mis-lable buf_out as buf, so it doesn't try to print it.
int DWYCOEXPORT dwyco_zap_create_preview_buf(int viewid, const char **buf_out_elide, int *len_out, int *cols_out, int *rows_out);


// see notes in dlli.cpp regarding how these work on
// non-windows platforms
DWYCO_LIST DWYCOEXPORT dwyco_get_vfw_drivers();
int DWYCOEXPORT dwyco_start_vfw(int idx, void *main_hwnd, void *client_hwnd);
int DWYCOEXPORT dwyco_shutdown_vfw();
int DWYCOEXPORT dwyco_change_driver(int new_idx);
int DWYCOEXPORT dwyco_is_preview_on();
int DWYCOEXPORT dwyco_preview_on(void *display_window);
int DWYCOEXPORT dwyco_preview_off();
int DWYCOEXPORT dwyco_vfw_format();
int DWYCOEXPORT dwyco_vfw_source();
int DWYCOEXPORT dwyco_set_external_video(int v);

#if defined(_Windows) || defined(_WIN32)
void DWYCOEXPORT dwyco_set_main_msg_window(void *h);
void DWYCOEXPORT dwyco_handle_msg(const char *msg, int msg_len, unsigned int message, unsigned int wp, unsigned int lp);

#endif

// normally, an autoupdate query command is automatically sent when you
// connect to a chat server. if you need to do it at some later point, like
// during a dialog, call this function.
void DWYCOEXPORT dwyco_force_autoupdate_check();
// set the callback to be called when an autoupdate check is complete.
// see "autoupdate check status" codes above for possible outcomes.
void DWYCOEXPORT dwyco_set_autoupdate_status_callback(DwycoAutoUpdateStatusCallback sb);
int DWYCOEXPORT dwyco_start_autoupdate_download(DwycoStatusCallback cb, void *arg1, DwycoAutoUpdateDownloadCallback dcb);
int DWYCOEXPORT dwyco_start_autoupdate_download_bg();
int DWYCOEXPORT dwyco_run_autoupdate();
void DWYCOEXPORT dwyco_abort_autoupdate_download();

// defunct
void DWYCOEXPORT dwyco_set_regcode(const char *s);
void DWYCOEXPORT dwyco_sub_get(const char **reg_out, int *len_out);

// this is a little builtin mini-applet that does some synchronization
// with another app (which is assumed to be using this API as well.)
// it is an artifact of the way android makes us use
// "services". essentially, the android main interface app gets a "lock"
// and works normally when it is visible. when the main app is paused or
// killed, an android service can run in the background, and call this
// function to grab the "lock" and perform send/recv of messages.
// as soon as the main application comes back to life, it requests
// the "lock", which will cause this function to cleanup and return,
// at which time, the process that called this function should exit.
// the exit will release the "lock" and allow the main app to continue
// normally.
int DWYCOEXPORT dwyco_background_processing(int port, int exit_if_outq_empty, const char *sys_pfx, const char *user_pfx, const char *tmp_pfx, const char *token);
// some more helper functions called from java for android related stuff
// strings in this case are utf-8, null terminated i hope
void DWYCOEXPORT dwyco_set_aux_string(const char *str);
void DWYCOEXPORT dwyco_write_token(const char *str);
void DWYCOEXPORT dwyco_clear_contact_list();
int DWYCOEXPORT dwyco_add_contact(const char *name, const char *phone, const char *email);
void DWYCOEXPORT dwyco_signal_msg_cond();
void DWYCOEXPORT dwyco_wait_msg_cond(int ms);
int DWYCOEXPORT dwyco_test_funny_mutex(int port);

// api for creating a simple backup of messages and account info
// "create_backup" creates an initial backup, then subsequent calls
// create a smaller incremental backup.
// calling "copy out" copies the current backup to an app specified folder.
// calling "remove" just removes the internal backup, causing a full backup
// to be created on the next call to "create_backup".
// restore takes a backup, and adds it to the current set of messages
// non-destructively. if msgs_only is false, it attempts to restore the
// actual account info as well, instead of just messages.
void DWYCOEXPORT dwyco_create_backup();
int DWYCOEXPORT dwyco_copy_out_backup(const char *dir, int force);
void DWYCOEXPORT dwyco_remove_backup();
int DWYCOEXPORT dwyco_restore_from_backup(const char *bu_fn, int msgs_only);


// not called from java
#define DWYCO_CONTACT_LIST_NAME "000"
#define DWYCO_CONTACT_LIST_PHONE "001"
#define DWYCO_CONTACT_LIST_EMAIL "002"
int DWYCOEXPORT dwyco_get_contact_list(DWYCO_LIST *list_out);
// you must call "dwyco_free_array" on the returned pointer
// after copying it out
int DWYCOEXPORT dwyco_get_aux_string(const char **str_out, int *len_str_out);

//------------------
// EXTERNAL VIDEO CAPTURE INTERFACE
// use this when the internal stuff in the DLL is not
// available (MAC, Linux) or not useful on windows.
//-------------------
//
// VIDEO CAPTURE INTERFACE
// This interface has grown from the internal Windows capture to
// be a simple encapsulation of video capture for other platforms.
// There are two parts of video capture:
//  (1) - device control (finding video capture devices, selecting one, setting its
//   capture rate, setting frame size and format, etc.) This usually happens
//   while streaming is not enabled.
//  (2) - video streaming (capturing frames into memory, where the DLL can pick
//   them up and process them.)
//
// The DLL provides an "internal" implementation of video capture
// (which uses Windows VFW, very old-school, but works better for older cams/drivers)
// and an "external" interface that can be filled in to provide video capture
// services to the DLL if the internal implementation doesn't work.
//
// The default is to use the internal capture.
//
// Currently on Windows, if you tell the DLL to use "external" video capture, it
// uses an implementation of video capture that encapsulates DX9 capture using a
// commercial component Dwyco licenses from a 3rd party (currently mitov.com ca 2018).
//
// This is how we provide both VFW and DX9 capture, which is a win for Windows users
// since the quality of video capture drivers varies wildly from cam to cam.
//
// On Linux, there is no "internal" mode. External mode is selected by default, and
// it is implemented using a wrapper for V4L2.
//
// NOTE: ca 10/2009, we had urania consulting implement a native video capture interface
// for MACOS 10.5+. ca 2018: This works well, but uses API's on the mac that have been
// deprecated for some time. This means that you cannot compile the urania driver
// using xcode9. I've been using an old VM with xcode7 on it to perform the
// compilation. The APIs are still supported at runtime as of 2018 for High Sierra.
//
// -------
// WRITING a video capture "driver" for the DLL
// -------
// This interface is used by the DLL for capturing streaming video. See below for a
// description of the interface used for device control.
//
// The DLL calls the following functions in order to get video data, which it
// processes and uses for video streaming, recording, etc.
//
// note: in the following, the names of the functions are just descriptive.
// your driver can call them whatever it wants, as long as you can get a pointer to the function
// that is callable from C++.
// in addition, the "ptr" arg is a pointer that is passed in that can be used as an identifier
// for a particular video capture "object", in case there are multiple video capture
// objects (NOTE: as of 8/2009, there is only one video capture object active at a time,
// so you can safely ignore the "ptr" arg in your driver.)
//
// new-cap-obj(void *ptr) - called by the DLL when it creates a new video capture object
//
// del-cap-obj(void *ptr) - called when the vid capture object is deleted. all streaming should be stopped
//  and whatever video capture resources are in use should be released.
//
// init(void *ptr, int frame_rate) - initialize the capture device and
//  get set up to capture using a max
//  frame rate of frame_rate frames/second. the frame rate is provided
//  because many video capture devices have a fixed rate while capturing
//  that needs to be specified before starting capture. in addition,
//  USB cameras can hog the bus if the hardware capture rate is set too
//  high. unfortunately, this is highly device dependent, so we allow the
//  user to specify a max frame rate, usually at the direction of tech
//  support. note that even if the frame rate is set to N fps, the DLL has
//  no obligation to actually produce streams with N fps. The frame rate
//  arg is simply provided as a guide to program the capture hardware.
//  You can safely ignore this parameter and use some fixed value if you
//  know your hardware can accommodate it.  init is called once before
//  streaming is to be started, but your driver should be robust and
//  ignore multiple calls while streaming has already started.
//
// has-data(void *ptr) - return 1 if there is frame data that can be fetched using get-data (see below.)
//  this is called frequently, so it should be fast.
//
// need(void *ptr) - this is called by the DLL if has-data returns 0, and it wants a frame to process.
//  you can safely ignore this call, as it is only an advisory. however, if streaming has stopped for some
//  reason, you can use the "need" call to restart it.
//
// pass(void *ptr) - this is called by the DLL if has-data returns 1 BUT the DLL cannot use the data and the
//  data should be discarded. essentially this is how the DLL tells the driver to drop a frame. this is an
//  important part of the DLL's bandwidth control and delay-management.
//
// stop(void *ptr) - stop capture, but don't release the device if you don't have to. when a stop is issued
//  the driver should discard all buffered frames.
//
// get-data(void *ptr, int *rows_out, int *cols_out,
// 		int *bytes_out, int *fmt_out, unsigned long *captime_out) - get frame data from driver
//  this function should return a pointer to a buffer of frame data for a single video frame.
//  the driver fills in the dimensions of the frame in the rows_out and cols_out pointers.
//  the driver fills in the raw number of bytes in the memory block in the bytes_out pointer.
//  the driver gives the format of the frame in the fmt_out pointer (see below.)
//  the driver fills in the time the frame was captured (in milliseconds, the origin of the
//   time is not important, it can be from stream time, or wall-clock, or any clock as long
//   as it is monotonically increasing. the better your clock, the better the sync will be with
//   audio. most hardware drivers these days provide some kind of stream-capture time, which is usually
//   the best to use. if you can't get this, wall-clock time will do in a pinch.)
//   NOTE: THE BLOCK OF DATA RETURNED IS NOT COPIED BY THE DLL. your driver should arrange for
//   the data block to be owned by the DLL. when the DLL is done with the block, it will call
//   free-data. the data block should not be molested by the driver in any way after get-data returns.
//
//	 NOTE: video format selection has been simplified by new drivers in most
//	 systems. only VFW still requires a lot of format mumbo-jumbo.
//	 most DLL drivers i have written using newer OS drivers, i just pick a format, tell the
//	 OS driver that is what i want. it simplifies life for everyone.
//	 (most OS drivers support RGB24 or YUV12, the dll has a long list it understands, but just
//	 pick RGB24 if you are unsure. set the *fmt_out to AQ_COLOR|AQ_RGB24)
//
// free-data(void *data) - the DLL is done with the data block, and it can be freed.
//
// DEVICE CONTROL
// I readily admit the following sucks (since it is based on VFW), but for now, it should suffice. I'll
// probably update this to include these calls into some kinda "external_video_device_control"
// call so you don't have to link it with the DLL,
// but for now you'll have to name the functions as shown and link them with the
// DLL explicitly.
//
//        char ** vgget_video_devices();
//         return a list of the video devices available to the user. the list of
//         pointers is terminated by a 0 pointer. each string is a normal 0 terminated
//         string. it should be
//         simple and user-readable, as the interface will probably display
//         the returned strings in a listbox or something.
//
//        vgfree_video_devices(char **);
//         free the list of strings returned from vgget_video_devices
//
//        vgset_video_device(int idx);
//         this tells the driver which of the devices to operate on from now on.
//         the idx arg is the index in the list of strings returned from
//         vgget_video_devices
//
//		  vgstop_video_device();
//		   stop the device "enough" so that you can perform format and size changes on the
//		   device. it depends on the device/platform how much "enough" is.
//
//        vgpreview_on(display_window);
//         turn on the HARDWARE preview for the device. this is different from software preview
//         (what you would normally use). the reason for this is that sometimes a driver may
//         require the device not to be streaming in order to show the contrast or other
//         adjustment dialogs, and you normally want to allow the user to see the results
//         of the adjustments in real-time. this is very dependent on hardware/platform/window system, etc.etc.
//         that it requires probably making changes to the DLL to accommodate this functionality.
//         normally i have stopped providing it as many drivers now allow you to tweak simple things
//         while streaming is in progress.
//
//        vgpreview_off();
//         turn off the HARDWARE preview.
//
//        vgshow_source_dialog();
//         this is a VFW hold-over, where "source" dialogs normally would allow the user to
//         adjust device-dependent items like contrast and white-balance.
//
//        vg_set_appdata(0);
//         this is used on windows to allow the main app to send some platform specific
//        initialization stuff into the "driver".

typedef void (DWYCOCALLCONV *DwycoVVCallback)(void *);
typedef int (DWYCOCALLCONV *DwycoIVCallback)(void *);
typedef int (DWYCOCALLCONV *DwycoIVICallback)(void *, int);
typedef void (DWYCOCALLCONV *DwycoVVIICallback)(void *, int, int);
typedef void *(DWYCOCALLCONV *DwycoVidGetDataCallback)(void *, int *r, int *c, int *bytes, int *fmt, unsigned long *captime);
typedef char **(DWYCOCALLCONV *DwycoCACallback)();
typedef void (DWYCOCALLCONV *DwycoFCACallback)(char **);
typedef void (DWYCOCALLCONV *DwycoVICallback)(int);
typedef void (DWYCOCALLCONV *DwycoVCallback)();


void DWYCOEXPORT dwyco_set_external_video_capture_callbacks(
    DwycoVVCallback nw,
    DwycoVVCallback del,
    DwycoIVICallback init,
    DwycoIVCallback has_data,
    DwycoVVCallback need,
    DwycoVVCallback pass,
    DwycoVVCallback stop,
    DwycoVidGetDataCallback get_data,
    DwycoVVCallback free_data,

    // device control
    DwycoCACallback get_vid_devices,
    DwycoFCACallback free_vid_list,
    DwycoVICallback set_vid_device,
    DwycoVCallback stop_vid_device,
    DwycoVCallback show_source_dialog,
    DwycoVVCallback hw_preview_on,
    DwycoVCallback hw_preview_off,
    DwycoVVCallback set_app_data

);

//-----------------
// EXTERNAL AUDIO CAPTURE INTERFACE
// use this for MAC and LINUX, for Windows the internal stuff works great.
//---------------
//
// Writing your own audio capture driver
// -------
// Audio capture has a similar flavor to the video capture interface mentioned
// above, except there is not really any "device control" for audio devices.
// If there is any device control needed, it is left up to an external mixer
// app (usually audio device control is so convoluted, most users can't even
// figure out the mixer apps. Not sure what we can do about this now, except
// maybe provide a separate mixer app that provides really simple stuff for the
// handful of things users want to do (like DJ-ing, or just talking into a mic.)
//
// Just like the video capture system, the DLL provides all the codecs,
// streaming, recording, timing, etc.etc. internally without any need for the
// application to adjust anything.
//
// The goal of this interface is to provide a "driver" that can capture audio
// in mono, 16-bit, 44.1Khz, signed integer format. The driver's function *may* be
// called via another thread in the DLL that is responsible for coordinating
// half-duplex audio devices (mostly a thing of the past, but it isn't hard to
// take it into account using a mutex, see the example code.)
//
// The functions the driver should implement are as follows:
//  new(void *, int buflen, int nbufs) - create a new audio acquisition object. currently
//   there is only one acq object alive at a time. The application should arrange to capture
//   audio into nbuf buffers, each buffer being buflen bytes long. The driver should *not*
//   perform any device allocation or anything that might cause a significant pause. the DLL
//   will size buflen and nbufs to be about 3 seconds total, each buffer being 80ms worth
//   of 16-bit signed data at 44.1Khz (mono).
//   these are rough estimates, and yes, 80ms is a magic number
//   related to the frame size of one of the audio codecs :-).
//
//  del(void *) - throw away all pending audio data, release the device, free all buffers.
//
//  init(void *) - called to probe for a suitable audio device. this can be called multiple
//   times, so the driver should be robust about avoiding trying to initialize devices twice
//   and stuff like that. The driver should NOT start streaming data. init should
//   verify the existence of a default audio input device that can be used. it should
//   return 0 if it cannot find any suitable audio input device.
//
//  has_data() - returns 1 if there is data to be fetched (via the get_data call.)
//
//  need() - when this is called, capturing data should start if it isn't already.
//
//  pass() - called to throw away all existing data that is buffered by the driver. streaming
//   should *not* stop. (i'm not sure this is ever used, will have to check.)
//
//  stop() - this isn't used at the moment, i don't think it is called.
//
//	on() - audio capturing should be resumed if it was paused. if the driver
//	 successfully gets streaming going, the "status" call should return 1
//
//	off() - audio capturing should be suspended. this is called in response to the user
//	 using the "mute" button, *and* in cases where the audio device is determined to be
//	 half-duplex, and the output channel needs to be activated.
//
//	reset() - all buffered data should be discarded, capturing stopped, and the device released.
//	 the driver will do a "need" or "on" to request streaming again.
//
//	status() - return 1 if the device is initialized and capturing.
//
//	get_data(int *len_out, int *time_out) - return one buffer of audio data.
//	 NOTE: the returned data is NOT COPIED OUT. the driver should make arrangements for
//	 the data to be owned by the caller. *len_out should be set to the length of the
//	 buffer in bytes. *time_out should be set to the virtual time the block was captured
//	 relative to the time streaming was started. the actual value of the time is not
//	 important, as long as it is monotonically increasing.
//
// note: i can already see a problem here with the "get_data" function. normally
// this driver is linked statically with the rest of the DLL and so the memory
// management issues are pretty much moot. for now, just assume the data block you
// hand back to the caller will be deallocated using "delete []". really need a
// "free-data" function ala the video capture interface.
//
// NOTE: the DLL when it starts up may "probe" for a device by quickly starting
// and checking the status of the audio acq object, then turning it back off. this is normal.
//
typedef void *(DWYCOCALLCONV *DwycoAudGetDataCallback)(void *, int *, int *);

void DWYCOEXPORT dwyco_set_external_audio_capture_callbacks(
    DwycoVVIICallback nw,
    DwycoVVCallback del,
    DwycoIVCallback init,
    DwycoIVCallback has_data,
    DwycoVVCallback need,
    DwycoVVCallback pass,
    DwycoVVCallback stop,
    DwycoVVCallback on,
    DwycoVVCallback off,
    DwycoVVCallback reset,
    DwycoIVCallback status,
    DwycoAudGetDataCallback get_data
);

//---------------
// EXTERNAL AUDIO PLAYOUT
// use this for MAC and Linux
//---------------
// NOTE: BE SURE TO READ THE INFO ABOUT MULTITHREADED
// ISSUES WITH THESE FUNCTIONS (BELOW.)
// -------------------------------------------------------------
// nw() -
//
// create an audio output device object. there is only
// ever one of these in the system at once (all the mixing of
// audio from different sources (net, zap playback, chat server, etc.)
// is handled by the internal mixer.
// do not acquire hardware devices in this function, do that in the
// "init" function below.
//
// -------------------------------------------------------------
// dlete()
//
// turn off and release any devices you may have opened. flush any data that is
// queued up.
//
//-------------------------------------------------------------
// init()
//
// init is called before audio output is to start streaming.
// you should program and acquire devices here. if you can't
// get a suitable output device, return 0. otherwise, return 1.
// you can start streaming silence here if needed.
// you may get multiple inits, if the device is already
// allocated and running, you should just return 1.
//
// the device should be prepared to output 16-bit signed little-endian
// data at 44.1khz (mono).
//
// -----------------------------------------------------------
// device_output(void *ah, void *buf, int len, int user_data)
//
// this is where the data should be output to the device.
// buf points to the data (16-bit signed little-endian 44.1khz
// mono). the buffer is len bytes long. user_data is sent in
// by the caller, and should be regurgitated in the "done" routine
// below.
//
// generally, the dll is expecting a data flow like this:
// it calls device output repeatedly with some number of
// buffers it determines to be a good number that will allow
// the audio system to store and begin playing. the number of
// buffers is usually enough for about 250ms of audio (about 4
// buffers-worth). the dll expects the driver to begin playback as
// soon as it gets the data.
// the dll meanwhile will poll the driver's "device done" function
// in order to retrieve the buffers it sent in, which it then
// assumes the driver has released (the driver will no longer access it.)
//
// note that the buffer passed in is "owned" by the DLL, not the
// driver. the DLL will handle the memory management of the buffer.
// however, it guarantees it will not access the buffer memory
// until the driver releases it via the "done" call.
// generally, the best way to handle this is to use 2 queues:
// (1) a queue for buffers waiting to be handed to a hardware driver
// (2) a queue for buffers that have been played and are
// going to be returned to the DLL. "played" here means the driver no longer
// needs the data, which means it could have copied the data out. it
// doesn't necessarily mean the buffer has been physically emitted from
// the speakers.
//
// for example:
// you stick the buffer on q #1 (which could be a hardware driver q,
// but remember to make a private copy of the data to hand to the
// hardware driver).
// when that is complete, you put the buffer on q #2 (your "done"
// q). device_done will pick up the buffer from the q #2.
// when you put something on your "done q", you should poke the
// audio mixer (see below.)
//
// the DLL expects the buffers to be returned in the exact order
// it gave them to "device_output".
//
// some of this is an artifact of the windows implementation of
// the audio drivers, since it appears that the windows drivers
// might actually be using the memory blocks as the target of
// DMA requests.
//
// another important thing: when a buffer is played out (or
// otherwise irrevocably committed to playback, like it has been
// queued in a hardware driver queue) you need to poke the
// audio mixer thread to update itself:
//
// if(TheAudioMixer)
//  TheAudioMixer->done_callback();
//
// there is no need to specify the exact buffer, the audio mixer
// has a record of what it has sent to the driver and knows what to do.
//
// ---------------------------------------------------------
// device_done(void *pa, void **buf_out, int *len, int *user_data)
//
// this function is called periodically by the DLL in order to retrieve
// buffers from the driver that the driver no longer needs.
// if there is a buffer waiting to be returned,
// this function should fill in *buf_out, *len, and *user_data with
// the data that was previous sent to "device_done", and return 1.
// otherwise it should return 0.
//
// -----------------------------------------------------------
// stop() -
//
// this function is called when the driver should stop playback of audio
// without flushing buffers (ie, just pause the playback.)
// NOTE: i'm not sure this is used without a "reset", so you may be able
// to ignore this function.
//
// -----------------------------------------------------------
// reset() -
//
// this should stop playback as soon as possible, flush any buffers that
// might be playing, and move any buffers that
// have not been returned to the DLL into your "done q" so
// that they can be returned immediately without playing them.
//
// -------------------------------------------------------------
//
// status() -
//
// return 1 if the audio output device has been successfully initialized
// and can play audio out. return 0 otherwise.
//
// --------------------------------------------------------------
//
// close() -
//
// this should reset the device (as for "reset" above), and release the
// audio hardware device. after a close, the driver will receive another init
// to reinitialize the device if audio output is resumed.
//
// ---------------------------------------------------------------
//
// buffer_time(int sz) -
//
// return the number of milliseconds represented by sz bytes.
// just return 80 for now, since it is all hardwired in other
// places.
//
//-----------------------------------------------------------
// play_silence()
//
// call "device_output" with a buffer than represents 80ms of
// silence. NOTE: this could be a problem because it expects the
// "device_done" to be able to free whatever is q'd by this function.
// which is probably wrong. for the time being, use this implementation:
/*
    DWBYTE *sil = new DWBYTE[AUDBUF_LEN];
    memset(sil, 0, AUDBUF_LEN);
    skel_audout_device_output(ah, sil, AUDBUF_LEN, 1);
*/
//
// -----------------------------------------------------------
//
// bufs_playing()
//
// this function should return the number of buffers that
// are currently queued up and playing out via the hardware.
// NOTE: it is IMPORTANT that bufs_playing return something pretty
// close to reality, though it doesn't have to be exact.
// for example, if you have irrevocably given the output device
// 4 buffers worth of data, and it has played back about 1 buffers
// worth, you should return 3 from bufs_playing.
// the DLL uses this to avoid playing too many buffers into the
// hardware (where they can't be fiddled with.) if the numbers
// drops below a threshold, it will also hurry up and feed the
// driver more buffers in order to avoid underruns.
//
//
// --------------
// THREADING AND SYNCHRONIZATION ISSUES WITH AUDIO OUTPUT
//
// Some of the functions mentioned above are called from
// the "mixer" thread in the DLL. The mixer thread is a special process
// in the DLL that is used to combine all the audio from multiple
// sources (calls, chatroom audio, zap playback audio). All of the audio
// sources feed their decoded audio streams into the audio mixer. The
// audio mixer adds them altogether taking into account their
// different timing and buffer issues, and then emits a specially
// timed, fixed-size-packet audio stream to the "audio output driver".
// The "audio output driver" is required to send this stream on to whatever
// host audio output system is in use (for linux: ESOUND, for example.)
// The audio output driver is also supposed to provide some simple "timing"
// information back to the mixer so the output driver can know when to
// give the output driver more audio data.
//
// Because the mixer thread is responsible for some time sensitive operations
// on the audio data, it is run in a separate thread (and if the host allows,
// given a higher priority in order to avoid buffer underruns during audio playback.)
//
//
// Functions guaranteed to only be called from the main DLL thread:
// nw
// dlete
//
// Functions guaranteed to be called only from the mixer thread:
// device_done
// bufs_playing
// device_output
//
// Functions that could be called by DLL or mixer thread:
// init
// reset
// stop
// status
// close
//
// In order to synchronize with the main DLL thread, the mixer thread
// uses critical sections internally.
// The audio output driver is responsible for any synchronization with
// threads present in the OS audio output system.
// SPECIAL NOTE: the audio mixer's "done_callback" mentioned above
// is required. If you don't call done_callback when you are done with a buffer (and the
// buffer is q'd for retrieval from device_done), the audio output will stall.
// The done_callback will acquire a lock that is released when it returns. So,
// don't call the done_callback from code that is sensitive to blocking.
//

typedef int (DWYCOCALLCONV *DwycoDevOutputCallback)(void *, void *, int, int);
typedef int (DWYCOCALLCONV *DwycoDevDoneCallback)(void *, void **, int *, int *);
typedef int (DWYCOCALLCONV *DwycoIICallback)(void *, int);

void DWYCOEXPORT dwyco_set_external_audio_output_callbacks(
    DwycoVVCallback nw,
    DwycoVVCallback dlete,
    DwycoIVCallback init,
    DwycoDevOutputCallback output,
    DwycoDevDoneCallback done,
    DwycoIVCallback stop,
    DwycoIVCallback reset,
    DwycoIVCallback status,
    DwycoIVCallback close,
    DwycoIICallback buffer_time,
    DwycoIVCallback play_silence,
    DwycoIVCallback bufs_playing
);

// you can use these functions in place of the
// {set,get}_xyz_data functions below if you need to
// set or get just a single setting.
//
// the "name" parameter is the name of the setting to operate on
// and it has the form:
// group/setting-name
//
// where "group" is one of:
// net, call_acceptance, display, raw_files, user, video_format, video_input, zap
//
// the setting name is one of the parameter names below in the set/get_data calls.

// so, for example
// dwyco_set_setting("user/email", "foo@bar.com");
// will set the email address in the user data.
// likewise
// dwyco_get_setting("user/email", &val, &len, &type);
// will get the same setting, where val, len, and type are
// the same as for dwyco_list_get calls.
// note: when setting values that are integers, you still have to pass
// in a string representing the value. the dll knows what type to
// regurgitate in the get_setting (or corresponding get_*_data call.
//
// these functions return 1 if the operation is successful,
// or 0 otherwise (usually you have a wrong setting name.)

// SPECIAL NOTE: there are settings you can set with these functions
// that aren't represented in the older functions below:
// zap/send_auto_reply       = an integer, "0" = don't send, "1" = send auto reply
// zap/auto_reply_mid        = string, the message-id of the auto-reply zap
// video_format/swap_rb
int DWYCOEXPORT dwyco_set_setting(const char *name, const char *value);
int DWYCOEXPORT dwyco_get_setting(const char *name, const char **value, int *len, int *dwyco_type);


// abusive use of macros, this whole thing
// should be nixed. should just have all the options
// settable by one function using a string
// or something.

#define DWUIDECLARG_BEGIN
#define DWUIDECLARG(type, name) type name,
#define DWUIDECLARG_OUT(type, name) type *name,
#define DWUIDECLARG_END int dummy = 0

int DWYCOEXPORT
dwyco_set_codec_data(int agc, int denoise, double audio_delay);

int DWYCOEXPORT
dwyco_get_codec_data(int *agc, int *denoise, double *audio_delay);


// for icuii, "automatic" should be set, which
// tells the capture driver to figure it out as
// best it can. warning: there may be bugs in here
// related to the driver overriding some settings
// when it is in "automatic" mode. i have to check
// this out more.
int DWYCOEXPORT
dwyco_set_vidcap_data(
    DWUIDECLARG_BEGIN
    DWUIDECLARG(const char *, device)
    DWUIDECLARG(const char *, b_and)
    DWUIDECLARG(const char *, b_or)
    DWUIDECLARG(const char *, b_xor)
    DWUIDECLARG(const char *, offset)
    DWUIDECLARG(bool, blue)
    DWUIDECLARG(bool, green)
    DWUIDECLARG(bool, red)
    DWUIDECLARG(bool, rgb16)
    DWUIDECLARG(bool, rgb24)
    DWUIDECLARG(bool, use_one_plane)
    DWUIDECLARG(bool, yuv9)
    DWUIDECLARG(bool, upside_down)	// icuii: set to 1 if "my pic is upside down" is checked
    DWUIDECLARG(bool, palette)
    DWUIDECLARG(bool, automatic)   	// icuii: set to 1, everything else 0
    DWUIDECLARG(bool, enable_color) // icuii: set to 1 if "color camera" is selected
    DWUIDECLARG(bool, yuv12)
    DWUIDECLARG(bool, swap_uv)		// icuii: set to 1 if "my face is blue" is set
    DWUIDECLARG_END
);

int DWYCOEXPORT
dwyco_get_vidcap_data(
    DWUIDECLARG_BEGIN
    DWUIDECLARG_OUT(const char *, device)
    DWUIDECLARG_OUT(const char *, b_and)
    DWUIDECLARG_OUT(const char *, b_or)
    DWUIDECLARG_OUT(const char *, b_xor)
    DWUIDECLARG_OUT(const char *, offset)
    DWUIDECLARG_OUT(bool, blue)
    DWUIDECLARG_OUT(bool, green)
    DWUIDECLARG_OUT(bool, red)
    DWUIDECLARG_OUT(bool, rgb16)
    DWUIDECLARG_OUT(bool, rgb24)
    DWUIDECLARG_OUT(bool, use_one_plane)
    DWUIDECLARG_OUT(bool, yuv9)
    DWUIDECLARG_OUT(bool, upside_down)
    DWUIDECLARG_OUT(bool, palette)
    DWUIDECLARG_OUT(bool, automatic)
    DWUIDECLARG_OUT(bool, enable_color)
    DWUIDECLARG_OUT(bool, yuv12)
    DWUIDECLARG_OUT(bool, swap_uv)
    DWUIDECLARG_END
);

// this is only used for testing without
// a camera. it is NOT used in ICUII.
int DWYCOEXPORT
dwyco_set_raw_files(
    DWUIDECLARG_BEGIN
    DWUIDECLARG(const char *, raw_files_list)
    DWUIDECLARG(const char *, raw_files_pattern)
    DWUIDECLARG(bool, use_list_of_files)
    DWUIDECLARG(bool, use_pattern)
    DWUIDECLARG(bool, preload)
    DWUIDECLARG_END
);

int DWYCOEXPORT
dwyco_get_raw_files(
    DWUIDECLARG_BEGIN
    DWUIDECLARG_OUT(const char *, raw_files_list)
    DWUIDECLARG_OUT(const char *, raw_files_pattern)
    DWUIDECLARG_OUT(bool, use_list_of_files)
    DWUIDECLARG_OUT(bool, use_pattern)
    DWUIDECLARG_OUT(bool, preload)
    DWUIDECLARG_END
);

// this tells the DLL what device should
// be used as a video source.
int DWYCOEXPORT
dwyco_set_video_input(
    DWUIDECLARG_BEGIN
    DWUIDECLARG(const char *, device_name)	// icuii: use this to store the capture driver name if you need to. NOTE: not used by the DLL otherwise.
    DWUIDECLARG(bool, coded)  			// not used.
    DWUIDECLARG(bool, raw)				// icuii: set to 0. if 1 uses "raw files" config
    DWUIDECLARG(bool, vfw)				// icuii: set to 1 if "capture using camera" is checked
    DWUIDECLARG(bool, no_video)			// icuii: set to 1 if "no video" is checked
    DWUIDECLARG(int, device_index)		// not used, but you can use it to store the index if needed.
    DWUIDECLARG_END
);

int DWYCOEXPORT
dwyco_get_video_input(
    DWUIDECLARG_BEGIN
    DWUIDECLARG_OUT(const char *, device_name)
    DWUIDECLARG_OUT(bool, coded)
    DWUIDECLARG_OUT(bool, raw)
    DWUIDECLARG_OUT(bool, vfw)
    DWUIDECLARG_OUT(bool, no_video)
    DWUIDECLARG_OUT(int, device_index)
    DWUIDECLARG_END
);

// this is the call screening stuff
// in ICUII, most of this is not exposed to users, and it is
// set according to whatever mode (single user
// vs. multiuser) you are in. in CDC32, there
// is a dialog the users can tweak.
int DWYCOEXPORT
dwyco_set_call_accept(
    DWUIDECLARG_BEGIN
    DWUIDECLARG(int , max_audio)		// max # of audio streams you will send
    DWUIDECLARG(int , max_chat)         // max # if PUBLIC chat streams you will accept
    DWUIDECLARG(int , max_video)		// max # of video streams you will send
    DWUIDECLARG(int , max_audio_recv)	// max # of audio streams you will receive
    DWUIDECLARG(int , max_video_recv)   // max # of video streams you will receive
    DWUIDECLARG(int , max_pchat)        // max # of private chat streams you will accept
    DWUIDECLARG(const char * , pw)			// icuii: connection password required to connect
    DWUIDECLARG(bool, auto_accept)		// icuii: 1 if "accept calls automatically" is checked
    DWUIDECLARG(bool, require_pw)       // icuii: 1 if "require password" is checked
    DWUIDECLARG_END
);

int DWYCOEXPORT
dwyco_get_call_accept(
    DWUIDECLARG_BEGIN
    DWUIDECLARG_OUT(int , max_audio)
    DWUIDECLARG_OUT(int , max_chat)
    DWUIDECLARG_OUT(int , max_video)
    DWUIDECLARG_OUT(int , max_audio_recv)
    DWUIDECLARG_OUT(int , max_video_recv)
    DWUIDECLARG_OUT(int , max_pchat)
    DWUIDECLARG_OUT(const char * , pw)
    DWUIDECLARG_OUT(bool, auto_accept)
    DWUIDECLARG_OUT(bool, require_pw)
    DWUIDECLARG_END
);

// assorted zap message setup
int DWYCOEXPORT
dwyco_set_zap_data(
    DWUIDECLARG_BEGIN
    DWUIDECLARG(bool, always_server)	// icuii: always 0
    DWUIDECLARG(bool, always_accept)    // icuii: 1 if "auto-accept quick messages" is checked
    DWUIDECLARG(bool, use_old_timing)   // icuii: 1 if "qm's move too fast or not at all" checked (in obscure tab)
    DWUIDECLARG(bool, save_sent)   		// icuii: 1 by default, 0 to turn off automatic "sent" qm saving
    DWUIDECLARG(bool, no_forward_default)	// icuii: 0 by default
    DWUIDECLARG_END
);

int DWYCOEXPORT
dwyco_get_zap_data(
    DWUIDECLARG_BEGIN
    DWUIDECLARG_OUT(bool, always_server)
    DWUIDECLARG_OUT(bool, always_accept)
    DWUIDECLARG_OUT(bool, use_old_timing)
    DWUIDECLARG_OUT(bool, save_sent)
    DWUIDECLARG_OUT(bool, no_forward_default)
    DWUIDECLARG_END
);

// these are networking rates and capture
// frame rates.
int DWYCOEXPORT
dwyco_set_rate_tweaks(
    DWUIDECLARG_BEGIN
    DWUIDECLARG(double, max_frame_rate)	// icuii: set to 10 (could be a slider tho) fractional frame rates are accepted
    DWUIDECLARG(long, max_udp_bytes) 	// not used, set to 65535
    DWUIDECLARG(long, link_speed)       // set to Kbps xmit on link (modem tab)
    DWUIDECLARG(long, link_speed_recv)  // set to Kbps recv on link (modem tab)
    DWUIDECLARG_END
);

int DWYCOEXPORT
dwyco_get_rate_tweaks(
    DWUIDECLARG_BEGIN
    DWUIDECLARG_OUT(double, max_frame_rate)
    DWUIDECLARG_OUT(long, max_udp_bytes)
    DWUIDECLARG_OUT(long, link_speed)
    DWUIDECLARG_OUT(long, link_speed_recv)
    DWUIDECLARG_END
);

// used for media select arg in the net data
// WARNING: the values of these defines were copied from aconn.h
// WARNING: the only value that works is "TCP_ONLY"
// NOTE: ca 2016, all the internal STUN and UPNP stuff does not work.
// STUN/UDP media was abandoned because it is just too problematic
// from a tech support perspective. UPnP, when it was tested, was also
// too flakey to rely on. ca 2018, Possibly UPnP could be revisited now that
// routers are implementing it more reliably.
// ca 2019, UPnP is an option that is handled and set up automatically.
// if it doesn't get a set of ports set up, it automatically fallsback to
// using server assisted calls as usual.

#define DWYCO_MEDIA_SELECT_DIRECT_ONLY 0  	// not impl.
#define DWYCO_MEDIA_SELECT_TCP_ONLY 1 		// force tcp SAC only
#define DWYCO_MEDIA_SELECT_UDP_ONLY 2		// force udp SAC only
#define DWYCO_MEDIA_SELECT_HANDSHAKE 3		// try to figure out cheapest way

int DWYCOEXPORT
dwyco_set_net_data(
    DWUIDECLARG_BEGIN
    DWUIDECLARG(int, primary_port) 				// primary listener, icuii: 2000
    DWUIDECLARG(int, secondary_port) 			// secondary listenter icuii: 9745
    DWUIDECLARG(int, pal_port)		 			// pal listener icuii: 6782
    DWUIDECLARG(int, nat_primary_port)			// icuii: 0
    DWUIDECLARG(int, nat_secondary_port)		// icuii: 0
    DWUIDECLARG(int, nat_pal_port)				// icuii: 0
    DWUIDECLARG(bool, advertise_nat_ports)		// icuii: 0
    DWUIDECLARG(int, disable_upnp)				// icuii: 0 , disabled for compat right now
    DWUIDECLARG(int, media_select)				// defaults to "handshake", see *MEDIA_SEL* defines
    DWUIDECLARG(int, listen)
    DWUIDECLARG_END
);

int DWYCOEXPORT
dwyco_get_net_data(
    DWUIDECLARG_BEGIN
    DWUIDECLARG_OUT(int, primary_port)
    DWUIDECLARG_OUT(int, secondary_port)
    DWUIDECLARG_OUT(int, pal_port)
    DWUIDECLARG_OUT(int, nat_primary_port)
    DWUIDECLARG_OUT(int, nat_secondary_port)
    DWUIDECLARG_OUT(int, nat_pal_port)
    DWUIDECLARG_OUT(bool, advertise_nat_ports)
    DWUIDECLARG_OUT(int, disable_upnp)
    DWUIDECLARG_OUT(int, media_select)
    DWUIDECLARG_OUT(int, listen)
    DWUIDECLARG_END
);


#ifdef __cplusplus
}
#endif

#endif
