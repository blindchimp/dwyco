#ifndef DLLI_H
#define DLLI_H
// DLL interface for cdccore
// (c) Dwyco, Inc., 1995-2006, All rights reserved.
//  * NOTICE: This file contains TRADE SECRETS that are the
//  * property of Dwyco, Inc. The contents may not be used or disclosed
//  * without EXPRESS WRITTEN PERMISSION of the owner.
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
#ifdef __GNUG__
#define __fastcall
#endif
#endif
#endif

extern "C" {

typedef void *DWYCO_LIST;
typedef DWYCO_LIST DWYCO_USER_LIST;
typedef DWYCO_LIST DWYCO_DIR_LIST;
typedef DWYCO_LIST DWYCO_UNSAVED_MSG_LIST;
typedef DWYCO_LIST DWYCO_SAVED_MSG_LIST;
typedef DWYCO_LIST DWYCO_SERVER_LIST;
typedef DWYCO_LIST DWYCO_QUERY_RESULTS_LIST;
typedef DWYCO_LIST DWYCO_MSG_IDX;

// WARNING: for "connect_all" processing, channel_destroy and call_established
// callbacks may act funny starting ca 4/2006 (ie, the id's may not match up
// between status callbacks and other callbacks. this is due to the new server
// assisted calling. use "connect_all2" interface instead.
// destroy callbacks still work as usual for other parts of the interface.
typedef void (DWYCOCALLCONV *DwycoChannelDestroyCallback)(int id, void *user_arg);
typedef void (DWYCOCALLCONV *DwycoCallEstablishedCallback)(int id, void *user_arg, const char *uid, int len_uid);
typedef void (DWYCOCALLCONV *DwycoCallEstablishedCallback2)(int call_id, int chan_id, void *user_arg, const char *uid, int len_uid);

// see below for DWYCO_CALLDISP_* constants for the "what" parameter in this callback
typedef  void (DWYCOCALLCONV *DwycoCallDispositionCallback)(int call_id, int chan_id, int what, void *user_arg, const char *uid, int len_uid, const char *call_type, int len_call_type);

typedef  void (DWYCOCALLCONV *DwycoStatusCallback)(int id, const char *msg, int percent_done, void *user_arg);
typedef  void (DWYCOCALLCONV *DwycoVideoDisplayCallback)(int chan_id, void *img, int cols, int rows, int depth);
typedef  void (DWYCOCALLCONV *DwycoCallAppearanceCallback)(int chan_id, const char *name, const char *location, const char *uid, int len_uid, const char *call_type, int len_call_type);
typedef  void (DWYCOCALLCONV *DwycoZapAppearanceCallback)(int chan_id, const char *name, int size, const char *uid, int len_uid);
typedef  void (DWYCOCALLCONV *DwycoCallAppearanceDeathCallback)(int chan_id);
// NOTE: the call_type is a string that can be sent in by the client on the remote
// end. this can help in cases where different screening rules apply for different
// calls in the UI.
typedef  int (DWYCOCALLCONV *DwycoCallScreeningCallback)(int chan_id,
	int remote_wants_to_recv_your_video, int remote_wants_to_send_you_video,
    int remote_wants_to_recv_your_audio, int remote_wants_to_send_you_audio,
    int remote_wants_to_exchange_pubchat, int remote_wants_to_exchange_privchat,
	const char *call_type, int len_call_type,
	const char *uid, int len_uid,
    int *accept_call_style,
    char **error_msg);
typedef  void (DWYCOCALLCONV *DwycoDirectoryDownloadedCallback)(DWYCO_DIR_LIST dl);
typedef  void (DWYCOCALLCONV *DwycoMessageSendCallback)(int id, int what, char *recipient, int reciplen, const char *server_msg, void *user_arg);
typedef  int (DWYCOCALLCONV *DwycoPublicChatInitCallback)(int ui_id);
typedef  int (DWYCOCALLCONV *DwycoPrivateChatInitCallback)(int chan_id, int ui_id, const char *);
typedef  int (DWYCOCALLCONV *DwycoPrivateChatDisplayCallback)(int ui_id, const char *com, int arg1, int arg2, const char *str, int len);
typedef  int (DWYCOCALLCONV *DwycoPublicChatDisplayCallback)(const char *who, int len_who, const char *line, int len_line, const char *uid, int len_uid);
typedef  void (DWYCOCALLCONV *DwycoVideoDisplayInitCallback)(int chan_id, int ui_id);
typedef  void (DWYCOCALLCONV *DwycoCommandCallback)(const char *cmd, void *user_arg, int succ, const char *failed_reason);
typedef  void (DWYCOCALLCONV *DwycoAutoUpdateDownloadCallback)(int status);
typedef  void (DWYCOCALLCONV *DwycoAutoUpdateStatusCallback)(int status, const char *desc);
typedef  void (DWYCOCALLCONV *DwycoProfileCallback)(int succ, const char *reason,
	const char *s1, int len_s1, const char *s2, int len_s2, const char *s3, int len_s3,
	const char *filename,
	const char *uid, int len_uid,
	 void *user_arg);
typedef  void (DWYCOCALLCONV *DwycoServerLoginCallback)(const char *str, int what);
typedef  void (DWYCOCALLCONV *DwycoMessageDownloadCallback)(int id, int what, const char *msgid, void *mdc_arg1);
typedef  void (DWYCOCALLCONV *DwycoQueryFieldCallback)(int id, DWYCO_QUERY_RESULTS_LIST ql);
typedef  void (DWYCOCALLCONV *DwycoChatCtxCallback)(int cmd, int ctx_id,
	const char *uid, int len_uid, 
    const char *name, int len_name,
	int type, const char *value, int val_len,
    int qid,
    int extra_arg
    );
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

// External IM errors.
// this is the only way errors and statuses are
// returned at the moment. it is very primitive now...
// str can be:
//  "login" - means the eim has been successfully logged in. what = 1
//  "error" - means an error occurred on the eim, what = error code. the codes are defined below.
//          note: in the current setup, there is no way to matchup
//          errors with certain operations (in the case of remove users, msg sends, etc.)
//          read the errors below carefully to figure out what will happen
//          when the error occurs.
//  "disconnect" - the eim has been disconnected by the system. what = 0
//
// note: in order to avoid having to include the
// imw headers, these statuses are copied from
// the imw headers, bad, but the alternative is worse.

#define  DWYCO_EIM_ERROR_CANT_ADD_USER				0x0001	//error occurs while adding user	(i.e. such user doesn't exist)
#define  DWYCO_EIM_ERROR_CANT_RENAME_USER			0x0002 	//error occurs while renaming user (i.e. no such user in CL)
#define  DWYCO_EIM_ERROR_CANT_REMOVE_USER			0x0003	//error occurs while removing user (i.e. no such user in CL)
#define  DWYCO_EIM_ERROR_CANT_SEND_MESSAGE			0x0004	//error occurs while sending message (for example user is offline in MSN)
#define  DWYCO_EIM_ERROR_RECONNECTING				0x0007	//connection fails and transport reconnects
// NOTE: when any of these errors occurs, the DLL automatically
// turns off auto-login for that EIM. this is because
// it is unlikely the login will ever succeed until the
// user fixes something. also, this avoids a problem
// where yahoo corrupts its address book while two
// clients fight over the account.
#define  DWYCO_EIM_ERROR_ACCOUNT_USED_ON_ANOTHER_PC	0x0005	//somebody uses the same account on another PC and our connection has been failed
#define  DWYCO_EIM_ERROR_WRONG_LOGIN_PASSWORD		0x0006	//authorization error
#define  DWYCO_EIM_ERROR_CONNECTION_FAILED			0x0008	//connection to the service has been failed after several tries of reconnect
#define  DWYCO_EIM_ERROR_CANT_RESOLVE_NAME			0x0009	//can't connect to the service because of inability to resolve server name
typedef  void (DWYCOCALLCONV *DwycoEIMServerLoginCallback)(int eim, const char *str, int what);

#define DWYCO_PAL_AUTH_STATUS_FAILED 0
#define DWYCO_PAL_AUTH_STATUS_REQUIRES_AUTHORIZATION 1
#define DWYCO_PAL_AUTH_STATUS_ALREADY_AUTHORIZED 2

typedef  void (DWYCOCALLCONV *DwycoPalAuthCallback)(const char *uid, int len_uid, int what);
typedef  void (DWYCOCALLCONV *DwycoUserControlCallback)(int chan_id, const char *uid, int len_uid, const char *data, int len_data);

void DWYCOEXPORT dwyco_set_fn_prefixes(
    const char *sys_pfx,
    const char *user_pfx,
    const char *tmp_pfx,
    const char *user_list_pfx
);
void DWYCOEXPORT dwyco_set_public_chat_init_callback(DwycoPublicChatInitCallback cb);
void DWYCOEXPORT dwyco_set_private_chat_init_callback(DwycoPrivateChatInitCallback cb);
void DWYCOEXPORT dwyco_set_private_chat_display_callback(DwycoPrivateChatDisplayCallback cb);
void DWYCOEXPORT dwyco_set_public_chat_display_callback(DwycoPublicChatDisplayCallback cb);
void DWYCOEXPORT dwyco_set_video_display_callback(DwycoVideoDisplayCallback cb);
void DWYCOEXPORT dwyco_set_video_display_init_callback(DwycoVideoDisplayInitCallback cb);
void DWYCOEXPORT dwyco_set_debug_message_callback(DwycoStatusCallback cb);
void DWYCOEXPORT dwyco_set_motd_callback(DwycoStatusCallback cb);
// the following codes are sent in to the "id" parameter on the
// unregister callback to give the application more info about
// what exactly the server is trying to say:
#define DWYCO_UNREG_TRIAL_EXPIRED 0
#define DWYCO_UNREG_INVALID_CODE 1
#define DWYCO_UNREG_CODE_IN_USE 2
#define DWYCO_UNREG_POLICY_VIOLATION 3
void DWYCOEXPORT dwyco_set_unregister_callback(DwycoStatusCallback cb);
void DWYCOEXPORT dwyco_set_pal_auth_callback(DwycoPalAuthCallback cb);
void DWYCOEXPORT dwyco_set_emergency_callback(DwycoEmergencyCallback cb);
void DWYCOEXPORT dwyco_set_user_control_callback(DwycoUserControlCallback cb);
void DWYCOEXPORT dwyco_set_alert_callback(DwycoCommandCallback cb);
void DWYCOEXPORT dwyco_set_call_bandwidth_callback(DwycoStatusCallback cb);

// Application defined call screening
// not advisable unless you know what you are doing, as it can become
// complicated.
//
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
#define DWYCO_CHAT_CTX_UPDATE_AH 4	// update asshole-factor, doesn't work in this version, should be ignored
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
// "full" - server has too many users
// "podium-filter" - string representing the current filtering
// note: UID has no meaning for this message, as it is a system-wide
// attribute, rather than for any particular user.
#define DWYCO_CHAT_CTX_SYS_ATTR 11  // extra-arg contains filter info

// names of attributes current are (with values):
// "muted" - t/nil
// "god" - t/nil
// "demigod" - t/nil
// "unblock" - int (delta-T in seconds from recp of msg)
// "blocked" - string (perma-block (old-style block)) string is reason
// "dict-err" - users name fails smell-test
// "away" - t/nil
// "idle" - int, number of second they have been idle
// "profile" - t/nil
// 
#define DWYCO_CHAT_CTX_UPDATE_ATTR 12		 // name is name of attribute, value as value for attr ala DWYCO_LIST

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
// into the chat server. this is useful mostl for testing without audio
int DWYCOEXPORT dwyco_chat_talk(int q);

// don't send me any audio if on == 1
int DWYCOEXPORT dwyco_chat_mute(int q, int on);
// only allows gods, demigods, or a particular uid to have the podium. gods can set any flag. demigods can set the
// demigod flag and the uid. whenever you call this function, everyone is kicked off the q and must readd themselves.
int DWYCOEXPORT dwyco_chat_set_filter(int q, int gods_only,  int demigods_only, const char *uid, int len_uid);
// only gods can use the next 2 functions
int DWYCOEXPORT dwyco_chat_set_demigod(const char *uid, int len_uid, int on);
int DWYCOEXPORT dwyco_chat_clear_all_demigods();

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
void DWYCOEXPORT dwyco_set_chat_server_status_callback(DwycoStatusCallback cb);
int DWYCOEXPORT dwyco_get_ah(const char *uid, int len_uid, char out[3]);

void DWYCOEXPORT dwyco_debug_dump();
void DWYCOEXPORT dwyco_random_string(const char **str_out, int len);
void DWYCOEXPORT dwyco_eze(const char *str, int len_str, const char **out, int *len_out);
void DWYCOEXPORT dwyco_ezd(const char *str, int len_str, const char **out, int *len_out);
int DWYCOEXPORT dwyco_get_profile(const char *uid, int len_uid, DwycoProfileCallback cb, void *user_arg);
int DWYCOEXPORT dwyco_set_profile(
	const char *s1, int len_s1,
    const char *s2, int len_s2,
    const char *s3, int len_s3,
    const char *filename,
    DwycoProfileCallback cb,
    void *user_arg);

int DWYCOEXPORT dwyco_set_profile_from_composer(int compid, const char *text, int len_text,
	DwycoProfileCallback cb, void *arg);
// the interface for this call is a bit odd.
// the callback will eventually be called with the "succ"
// parameter either 0 (for failed), -1 (for success, but there is no
// attachment available, just text in "s1"), or some number not 0 or -1, which will be
// a viewer id, that can be used with the "zap_view" functions to play back
// the attachment for the profile. note that this will only work if you
// used the "set_profile_from_composer" function to create the profile (or it was
// created by cdc32)
int DWYCOEXPORT dwyco_get_profile_to_viewer(const char *uid, int len_uid, DwycoProfileCallback cb, void *arg);

void DWYCOEXPORT dwyco_command_from_keyboard(int ui_id, int com,
	int arg1, int arg2, char *str, int len);

void DWYCOEXPORT dwyco_line_from_keyboard(int ui_id, const char *line, int len);
int DWYCOEXPORT dwyco_selective_chat_recipient_enable(const char *id, int len_uid, int enable);
int DWYCOEXPORT dwyco_selective_chat_enable(int e);
void DWYCOEXPORT dwyco_reset_selective_chat_recipients();
int DWYCOEXPORT dwyco_is_selective_chat_recipient(const char *uid, int len_uid);

void DWYCOEXPORT dwyco_destroy_channel(int chan_id);
void DWYCOEXPORT dwyco_destroy_by_ui_id(int ui_id);
void DWYCOEXPORT dwyco_hangup_all_calls();
void DWYCOEXPORT dwyco_pause_all_channels(int);
void DWYCOEXPORT dwyco_set_channel_destroy_callback(int chan_id,
	DwycoChannelDestroyCallback cb, void *user_arg);

// password change return values for "succ" arg in
// callback, if email, then "error" arg is email
// address to which password was sent, and if
// password, then the "error" arg is the password
// that was requested.

#define DWYCO_CMD_FETCH_PW_EMAIL 2
#define DWYCO_CMD_FETCH_PW_PASSWORD 3

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
#ifdef CDCX
#define DWYCO_AUTOUPDATE_MUTEX_NAME "dwyco cdcx mutex2"
#elif defined(DWYCO_POWERBROWSE)
#define DWYCO_AUTOUPDATE_MUTEX_NAME "dwyco powerbrowse mutex2"
#else
#define DWYCO_AUTOUPDATE_MUTEX_NAME "dwyco cdc32 mutex2"
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



// ratings
#define DWYCO_RATING_ADULT 0
#define DWYCO_RATING_GENERAL 1

// sort columns for directory
#define  DWYCO_SORT_HANDLE 0
#define  DWYCO_SORT_DESCRIPTION 1
#define  DWYCO_SORT_LOCATION 2

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
int DWYCOEXPORT dwyco_enable_video_capture_preview(int on, int *ui_id_in_out);
void DWYCOEXPORT dwyco_add_entropy_timer(char *crap, int crap_len);

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
int DWYCOEXPORT dwyco_uid_g(const char *uid, int len_uid);
int DWYCOEXPORT dwyco_load_users();
int DWYCOEXPORT dwyco_get_user_list(DWYCO_USER_LIST *list_out, int *nelems);
int DWYCOEXPORT dwyco_get_message_index(DWYCO_MSG_IDX *list_out, const char *uid, int len_uid);
int DWYCOEXPORT dwyco_get_message_bodies(DWYCO_SAVED_MSG_LIST *list_out, const char *uid, int uid_len, int load_sent);
int DWYCOEXPORT dwyco_get_unsaved_messages(DWYCO_UNSAVED_MSG_LIST *list_out, const char *uid, int len_uid);
int DWYCOEXPORT dwyco_get_unsaved_message(DWYCO_UNSAVED_MSG_LIST *list_out, const char *msg_id);
int DWYCOEXPORT dwyco_unsaved_message_to_body(DWYCO_SAVED_MSG_LIST *list_out, const char *msg_id);
int DWYCOEXPORT dwyco_delete_unsaved_message(const char *msg_id);
int DWYCOEXPORT dwyco_delete_saved_message(const char *user_id, int len_uid, const char *msg_id);
int DWYCOEXPORT dwyco_save_message(const char *msg_id);
int DWYCOEXPORT dwyco_get_saved_message(DWYCO_SAVED_MSG_LIST *list_out, const char *user_id, int len_uid, const char *msg_id);
int DWYCOEXPORT dwyco_fetch_server_message(const char *msg_id, DwycoMessageDownloadCallback cb, void *mdc_arg1,
	DwycoStatusCallback scb, void *scb_arg1);
void DWYCOEXPORT dwyco_cancel_message_fetch(int fetch_id);
DWYCO_LIST DWYCOEXPORT dwyco_get_body_text(DWYCO_SAVED_MSG_LIST m);
DWYCO_LIST DWYCOEXPORT dwyco_get_body_array(DWYCO_SAVED_MSG_LIST m);
int DWYCOEXPORT dwyco_authenticate_body(DWYCO_SAVED_MSG_LIST m, const char *recip_uid, int len_uid, int unsaved);
void DWYCOEXPORT dwyco_update_user_info(const char *uid, int len_uid);

void DWYCOEXPORT dwyco_pal_add(const char *user_id, int len_uid);
void DWYCOEXPORT dwyco_pal_delete(const char *user_id, int len_uid);
int DWYCOEXPORT dwyco_is_pal(const char *user_id, int len_uid);
DWYCO_LIST DWYCOEXPORT dwyco_pal_get_list();
void DWYCOEXPORT dwyco_pal_relogin();

void DWYCOEXPORT dwyco_set_alert(const char *uid, int len_uid, int val);
int DWYCOEXPORT dwyco_get_alert(const char *uid, int len_uid);

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

int DWYCOEXPORT dwyco_is_ignored(const char *user_id, int len_uid);
void DWYCOEXPORT dwyco_ignore(const char *user_id, int len_uid);
void DWYCOEXPORT dwyco_unignore(const char *user_id, int len_uid);
void DWYCOEXPORT dwyco_session_ignore(const char *user_id, int len_uid);
void DWYCOEXPORT dwyco_session_unignore(const char *user_id, int len_uid);
void DWYCOEXPORT dwyco_always_visible(const char *uid, int len_uid, int val);
void DWYCOEXPORT dwyco_never_visible(const char *uid, int len_uid, int val);
int DWYCOEXPORT dwyco_is_never_visible(const char *uid, int len_uid);
int DWYCOEXPORT dwyco_is_always_visible(const char *uid, int len_uid);
DWYCO_LIST DWYCOEXPORT dwyco_ignore_list_get();
DWYCO_LIST DWYCOEXPORT dwyco_session_ignore_list_get();
void DWYCOEXPORT dwyco_add_old_timing(const char *user_id, int len_uid);
void DWYCOEXPORT dwyco_del_old_timing(const char *user_id, int len_uid);
DWYCO_LIST DWYCOEXPORT dwyco_uid_to_info(const char *user_id, int len_uid, int *cant_resolve_now);
void DWYCOEXPORT dwyco_set_dnd(int dnd);
int DWYCOEXPORT dwyco_get_dnd();
int DWYCOEXPORT dwyco_delete_user(const char *uid, int uid_len);
int DWYCOEXPORT dwyco_empty_trash();
#ifdef __WIN32__
int DWYCOEXPORT dwyco_is_capturing_video();
#endif
int DWYCOEXPORT dwyco_count_trashed_users();
void DWYCOEXPORT dwyco_set_moron_dork_mode(int);
int DWYCOEXPORT dwyco_get_moron_dork_mode();
void DWYCOEXPORT dwyco_set_autosave_config(int);
void DWYCOEXPORT dwyco_set_auto_authenticate(int);
void DWYCOEXPORT dwyco_simple_diagnostics(const char **res, int *len_res);
void DWYCOEXPORT dwyco_network_diagnostics(const char **res, int *len_res);
void DWYCOEXPORT dwyco_estimate_bandwidth(int *out_bw, int *in_bw, const char **res, int* len);

// misc pw hashing functions, all this for cdc compat... DO NOT USE

void DWYCOEXPORT dwyco_gen_pass(const char *pw, int len_pw, char **salt_in_out, int *len_salt_in_out, char **hash_out, int *len_hash_out);
void DWYCOEXPORT dwyco_get_auth_file(
	const char **pw_hash_out, int *len_hash_out,
	const char **salt_out, int *len_salt_out,
	const char **local_pw_hash_out, int *len_local_pw_hash_out,
	const char **local_salt_out, int *len_local_salt_out);
void DWYCOEXPORT dwyco_free(char *p);
void DWYCOEXPORT dwyco_free_array(char *p);
void DWYCOEXPORT dwyco_finish_startup(int use_server_password, int save_pw);
void DWYCOEXPORT dwyco_set_cdc_compat(int a);
int DWYCOEXPORT dwyco_get_create_new_account();
int DWYCOEXPORT dwyco_check_password_against_local_hash(const char *pw, int len_pw);
void DWYCOEXPORT dwyco_set_local_auth(int a);
void DWYCOEXPORT dwyco_update_password(int save_pw);

// END compat functions DO NOT USE

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

// User list records
#define DWYCO_UL_DIRNAME "000" 	//xxx.usr
#define DWYCO_UL_INFO "001"		// info vec from server
#define DWYCO_UL_QIR_FROM "001000"
#define DWYCO_UL_QIR_HANDLE "001001"
#define DWYCO_UL_QIR_EMAIL "001002"
#define DWYCO_UL_QIR_ICQ "001003"
#define DWYCO_UL_QIR_FIRST "001004"
#define DWYCO_UL_QIR_LAST "001005"
#define DWYCO_UL_QIR_DESCRIPTION "001006"
#define DWYCO_UL_QIR_LOCATION "001007"
// note: may be runtime fields added here by cdc UI stuff, which
// you can ignore. it shouldn't really do that, but...

// Directory records
		// v[0] is ip with port address
		// v[1] is vector of info
		// v[2] is inhibit flag
		// v[3] is extra info, if non-nil
#define DWYCO_DIR_IP "000"
#define DWYCO_DIR_INFO_HOSTNAME "001000"
#define DWYCO_DIR_INFO_USERNAME "001001"
#define DWYCO_DIR_INFO_DESC "001002"
#define DWYCO_DIR_INFO_CALLACCEPT "001003"
#define DWYCO_DIR_INFO_CA_MAX_AUDIO "001003000"
#define DWYCO_DIR_INFO_CA_MAX_VIDEO "001003001"
#define DWYCO_DIR_INFO_CA_CHAT "001003002"
#define DWYCO_DIR_INFO_CA_MAX_AUDIO_RECV "001003003"
#define DWYCO_DIR_INFO_CA_MAX_VIDEO_RECV "001003004"
#define DWYCO_DIR_INFO_UID "001004"
#define DWYCO_DIR_INFO_REG "001005"
#define DWYCO_DIR_INFO_ONLINE "001006"
#define DWYCO_DIR_INFO_SHITLISTED "001007"
#define DWYCO_DIR_INFO_RATING "001008"
#define DWYCO_DIR_INFO_DIRECT "001009"
#define DWYCO_DIR_INFO_LOCATION "001010"
#define DWYCO_DIR_INFO_CLIENT_TYPE "001011"
#define DWYCO_DIR_INFO_PROFILE "001012"
#define DWYCO_DIR_INFO_FW_PORTS "001013"
#define DWYCO_DIR_INFO_FW_PORTS_PRIMARY "001013000"
#define DWYCO_DIR_INFO_FW_PORTS_SECONDARY "001013001"
#define DWYCO_DIR_INFO_FW_PORTS_PAL "001013002"
#define DWYCO_DIR_INFO_IS_GOD "001014"
#define DWYCO_DIR_INHIBIT "002"
#define DWYCO_DIR_EXTRA "003"

// Message summary records
		// 0: who from
		// 1: len
		// 2: id on server
		// 3: date vector
#define DWYCO_QMS_FROM "000"
#define DWYCO_QMS_LEN "001"
#define DWYCO_QMS_ID "002"
#define DWYCO_QMS_DATE_SENT "003"
#define DWYCO_QMS_DS_YEAR "003000"
#define DWYCO_QMS_DS_JDAY "003001"
#define DWYCO_QMS_DS_HOUR "003002"
#define DWYCO_QMS_DS_MINUTE "003003"
#define DWYCO_QMS_DS_SECOND "003004"
#define DWYCO_QMS_DS_SECONDS_SINCE_JAN1_1970 "003005"
		// added locally:
#define DWYCO_QMS_PENDING_DEL "004"
#define DWYCO_QMS_IS_DIRECT "005"
		// oops, server has to put this way out here because
		// old software expects to see the above structure...
		// 6: rating of sender
#define DWYCO_QMS_SENDER_RATING "006"
#define DWYCO_QMS_ONLINE "007"
#define DWYCO_QMS_FW_INFO "008"
#define DWYCO_QMS_SPECIAL_TYPE "009"

// Message bodies after they are saved locally
#define DWYCO_QM_BODY_ID "000"
#define DWYCO_QM_BODY_FROM "001"
#define DWYCO_QM_BODY_TEXT_do_not_use_use_get_msg_array_instead "002"
#define DWYCO_QM_BODY_TEXT2_obsolete "002"
#define DWYCO_QM_BODY_ATTACHMENT "003"
#define DWYCO_QM_BODY_DATE "004"
#define DWYCO_QM_BODY_DATE_YEAR "004000"
#define DWYCO_QM_BODY_DATE_JDAY "004001"
#define DWYCO_QM_BODY_DATE_HOUR "004002"
#define DWYCO_QM_BODY_DATE_MINUTE "004003"
#define DWYCO_QM_BODY_DATE_SECOND "004004"
#define DWYCO_QM_BODY_DATE_SECONDS_SINCE_JAN1_1970 "004005"
// locally set
#define DWYCO_QM_BODY_SENT "005"
// from server
#define DWYCO_QM_BODY_RATING "006"
//#define DWYCO_QM_BODY_TIME_RECV "007" (defunct)
//#define DWYCO_QM_BODY_AUTOPLAY "008"  (defunct, never used)

// there are some subelements in here, but they aren't needed by client
#define DWYCO_QM_BODY_AUTH_VEC "007"
#define DWYCO_QM_BODY_FORWARDED_BODY "008"
#define DWYCO_QM_BODY_NEW_TEXT_do_not_use_directly_use_get_msg_array "009"
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

// DWYCO_MSG_IDX is an index of the saved messages for a particular UID.
// The index is sorted in order of descending date, and each element in the
// index is a (date, mid) pair.
// This index is fixed at fetch time, and can be used for browsing
// and displaying msgs using views.

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

// records returned from the query field callback
// ID Handle Email Icq First Last User-info

#define DWYCO_QR_ID "000"
#define DWYCO_QR_HANDLE "001"
#define DWYCO_QR_EMAIL "002"
#define DWYCO_QR_ICQ "003"
#define DWYCO_QR_FIRST "004"
#define DWYCO_QR_LOCATION "004" // note: location and first are same slot
#define DWYCO_QR_LAST "005"
#define DWYCO_QR_DESCRIPTION "006"

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
int DWYCOEXPORT dwyco_is_special_message2(DWYCO_UNSAVED_MSG_LIST ml, int *what_out);

// "what" returns from the dwyco_is_special_message function
#define DWYCO_SUMMARY_PAL_AUTH_REQ 0
#define DWYCO_SUMMARY_PAL_OK 1
#define DWYCO_SUMMARY_PAL_REJECT 2
#define DWYCO_SUMMARY_SPECIAL_USER_DEFINED 3
#define DWYCO_PAL_AUTH_REQ 4
#define DWYCO_PAL_OK 5
#define DWYCO_PAL_REJECT 6
#define DWYCO_SPECIAL_USER_DEFINED 7

#define DWYCO_EIM_SYSTEM_DWYCO 0 // note: returned by pal_type, not used otherwise
#ifdef IMW32
// EXTERNAL INSTANT MESSAGE SYSTEM INTERFACE
// note: the DLL internally maps external IM
// systems ids (uin for icq, login names for aim, for example)
// into dwyco uids (normally 80-bit unique identifiers).
// these uids act the same as normal DLL uids, except
// some operations are no-ops (like adding to
// the dwyco pal list), and some are handled internally
// for whatever system the uid is from (send message, for
// example, needs to send the messages via the proper
// messenger.
// unfortunately, there isn't an easy mapping back and
// forth between these uids because of differences in
// length and so on, so unless you are actively logged
// into a system, you won't be able to know what type
// of uid it is. this normally isn't a problem.

// external IM handling defines and function (ICQ, AIM)
// note: all uids will return as "dwyco" type if you
// are not logged into icq/aim, whatever. this is
// because we can't map uids to uin and back
// before we know the complete pal list from these
// other systems.
//#define DWYCO_EIM_SYSTEM_CANT_RESOLVE -1
#define DWYCO_EIM_SYSTEM_ICQ 1
#define DWYCO_EIM_SYSTEM_AIM 2
#define DWYCO_EIM_SYSTEM_MSN 3
#define DWYCO_EIM_SYSTEM_YAHOO 4

// these can be (and probably should be) called before
// dwyco_init in order to set up external IM auto-logins
void DWYCOEXPORT dwyco_eim_stop(int which);
void DWYCOEXPORT dwyco_eim_start(int which, DwycoEIMServerLoginCallback slb);
// returns 1 if the EIM is set for auto-login, and 0 if it is disabled.
int DWYCOEXPORT dwyco_eim_status(int which);
void DWYCOEXPORT dwyco_eim_set_acct_info(int which, const char *uin, const char *pw);
// END call before dwyco_init
// this takes a dwyco uid, and tells you what type of IM system it
// is from. should be used for display, and ui tweaks only.
int DWYCOEXPORT dwyco_pal_type(const char *uid, int len_uid);

void DWYCOEXPORT dwyco_eim_pal_add(int eim,  const char *sn, int len_sn, const char *nick, int len_nick);
void DWYCOEXPORT dwyco_eim_pal_del(int eim,  const char *sn, int len_sn);
int DWYCOEXPORT
dwyco_zap_send_sms(int compid,
	char *phone_number,
    char *text,
	DwycoMessageSendCallback mscb, void *mscb_arg1,
	DwycoStatusCallback dscb, void *dscb_arg1
);
#endif

// END external IM functions

int DWYCOEXPORT dwyco_init();
int DWYCOEXPORT dwyco_exit();
int DWYCOEXPORT dwyco_service_channels(int *spin);
void DWYCOEXPORT dwyco_set_login_password(const char *pw, int len_pw);
void DWYCOEXPORT dwyco_set_login_result_callback(DwycoServerLoginCallback cb);
void DWYCOEXPORT dwyco_database_login();
int DWYCOEXPORT dwyco_database_online();
int DWYCOEXPORT dwyco_database_auth_remote();
void DWYCOEXPORT dwyco_inhibit_database(int i);
void DWYCOEXPORT dwyco_inhibit_pal(int i);
void DWYCOEXPORT dwyco_inhibit_sac(int i);
void DWYCOEXPORT dwyco_set_old_pal_recv(int i);
void DWYCOEXPORT dwyco_change_login_password(const char *old_pw, int len_old_pw, const char *new_pw, int len_new_pw,
	DwycoCommandCallback cb);
void DWYCOEXPORT dwyco_change_login_password2(const char *uid, int len_uid, const char *new_pw, int len_new_pw,
	const char *new_email, DwycoCommandCallback cb);
int DWYCOEXPORT dwyco_query_field(const char *field, const char *val, int len_val,
	DwycoQueryFieldCallback cb);
void DWYCOEXPORT dwyco_fetch_login_password(int typed_password, DwycoCommandCallback cb);
int DWYCOEXPORT dwyco_get_user_info_sync();

int DWYCOEXPORT dwyco_get_audio_hw(int *has_audio_input, int *has_audio_output, int *audio_hw_full_duplex);
int DWYCOEXPORT dwyco_set_all_mute(int);
int DWYCOEXPORT dwyco_get_all_mute();
int DWYCOEXPORT dwyco_set_auto_squelch(int a);
int DWYCOEXPORT dwyco_get_auto_squelch();
void DWYCOEXPORT dwyco_set_full_duplex(int a);
int DWYCOEXPORT dwyco_get_full_duplex();
int DWYCOEXPORT dwyco_get_audio_output_in_progress();
int DWYCOEXPORT dwyco_get_squelched();

// deprecated, use connect_all2 now
#if 0
void DWYCOEXPORT
dwyco_connect_all(char **ip_list, char **uid_list, int num, 
	DwycoChannelDestroyCallback dcb, void *dcb_arg1,
	DwycoCallEstablishedCallback ecb, void *ecb_arg1,
    DwycoStatusCallback scb, void *scb1_arg1,
	int send_video, int recv_video,
	int send_audio, int recv_audio,
	int private_chat, int public_chat,
    char *pw);
#endif

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
void DWYCOEXPORT
dwyco_connect_all2(char **ip_list, char **uid_list, int num, 
	DwycoCallDispositionCallback cdc, void *cdc_arg1,
    DwycoStatusCallback scb, void *scb1_arg1,
	int send_video, int recv_video,
	int send_audio, int recv_audio,
	int private_chat, int public_chat,
    const char *pw,
    const char *call_type, int len_call_type);

void DWYCOEXPORT
dwyco_connect_msg_chan(char **ip_list, char **uid_list, int num,
	DwycoCallDispositionCallback cdc, void *cdc_arg1,
    DwycoStatusCallback scb, void *scb_arg1);

void DWYCOEXPORT dwyco_cancel_call(int call_id);
int DWYCOEXPORT dwyco_chan_to_call(int chan_id);
int DWYCOEXPORT dwyco_send_user_control(const char *uid, int len_uid, const char *data, int len_data);


void DWYCOEXPORT dwyco_set_call_appearance_callback(DwycoCallAppearanceCallback cb);
void DWYCOEXPORT dwyco_set_call_acceptance_callback(DwycoCallAppearanceCallback cb);
void DWYCOEXPORT dwyco_set_zap_appearance_callback(DwycoZapAppearanceCallback cb);
void DWYCOEXPORT dwyco_set_call_appearance_death_callback(DwycoCallAppearanceDeathCallback cb);
int DWYCOEXPORT dwyco_call_accept(int id);
int DWYCOEXPORT dwyco_call_reject(int id, int session_ignore);
int DWYCOEXPORT dwyco_zap_accept(int id, int always_accept);
int DWYCOEXPORT dwyco_zap_reject(int id, int session_ignore);

void DWYCOEXPORT dwyco_set_directory_downloaded_callback(DwycoDirectoryDownloadedCallback cb);
int DWYCOEXPORT dwyco_directory_starting();
int DWYCOEXPORT dwyco_directory_online();
int DWYCOEXPORT dwyco_directory_downloading();
void DWYCOEXPORT dwyco_set_directory_sort_column(int);
int DWYCOEXPORT dwyco_get_server_list(DWYCO_SERVER_LIST *list_out, int *numlines);
int DWYCOEXPORT dwyco_switch_to_server(int i);
int DWYCOEXPORT dwyco_disconnect_server();
int DWYCOEXPORT dwyco_switch_to_chat_server(int i);
int DWYCOEXPORT dwyco_disconnect_chat_server();
void DWYCOEXPORT dwyco_room_enter(const char *name, const char *pw, DwycoCommandCallback cb, void *user_arg);
void DWYCOEXPORT dwyco_room_create(const char *name, const char *pw, DwycoCommandCallback cb, void *user_arg);
void DWYCOEXPORT dwyco_room_delete(const char *name, DwycoCommandCallback cb, void *user_arg);
void DWYCOEXPORT dwyco_refresh_directory();
void DWYCOEXPORT dwyco_set_invisible_state(int invis);
int DWYCOEXPORT dwyco_get_invisible_state();

// message composition functions
int DWYCOEXPORT dwyco_make_zap_composition(char *must_be_zero);
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
		int len
);
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
int DWYCOEXPORT dwyco_is_file_zap(int compid);

int DWYCOEXPORT dwyco_is_forward_composition(int compid);
int DWYCOEXPORT dwyco_flim(int compid);

int DWYCOEXPORT dwyco_delete_zap_composition(int compid);

int DWYCOEXPORT dwyco_zap_record(int compid, int video, int audio, int pic, int frames,
	DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *ui_id_out);

int DWYCOEXPORT dwyco_zap_record2(int compid, int video, int audio,
	int max_frames, int max_bytes, int hi_quality,
	DwycoStatusCallback scb, void *scb_arg1,
	DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *ui_id_out);

int DWYCOEXPORT dwyco_zap_stop(int compid);

int DWYCOEXPORT dwyco_zap_play(int compid,
	DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *ui_id_out);

// this will use the value stored in zap_data no_forward_default
int DWYCOEXPORT dwyco_zap_send(int compid,
	char **recipient_list,
	int nrecip,
	char *text,
	DwycoMessageSendCallback mscb, void *mscb_arg1,
	DwycoStatusCallback dscb, void *dscb_arg1
);

// this uses whatever no forward flag you send in, otherwise it is
// identical to the above call.
int DWYCOEXPORT dwyco_zap_send2(int compid,
	char **recipient_list,
	int nrecip,
	char *text,
	int no_forward,
	DwycoMessageSendCallback mscb, void *mscb_arg1,
	DwycoStatusCallback dscb, void *dscb_arg1
);

int DWYCOEXPORT dwyco_zap_send_cancel(int compid);
int DWYCOEXPORT dwyco_zap_still_active(int compid);

// functions for just viewing a zap message attachment
int DWYCOEXPORT dwyco_make_zap_view(DWYCO_SAVED_MSG_LIST list, const char *recip_uid, int len_uid, int unsaved);
int DWYCOEXPORT dwyco_make_zap_view_file(const char *filename);
int DWYCOEXPORT dwyco_delete_zap_view(int viewid);
int DWYCOEXPORT dwyco_zap_play_view(int viewid, DwycoChannelDestroyCallback dcb, void *dcb_arg1,
    int *ui_id_out);
int DWYCOEXPORT dwyco_zap_play_view_no_audio(int viewid, DwycoChannelDestroyCallback dcb, void *dcb_arg1,
    int *ui_id_out);
int DWYCOEXPORT dwyco_zap_play_preview(int viewid,
	DwycoChannelDestroyCallback dcb, void *dcb_arg1,
    int *ui_id_out);
int DWYCOEXPORT dwyco_zap_stop_view(int viewid);
int DWYCOEXPORT dwyco_zap_quick_stats_view(int viewid, int *has_video, int *has_audio, int *short_video);


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

#ifdef __WIN32__
void DWYCOEXPORT dwyco_set_main_msg_window(void *h);
void DWYCOEXPORT dwyco_handle_msg(const char *msg, int msg_len, unsigned int message, unsigned int wp, unsigned int lp);

#endif

// must be called before dwyco_init
int DWYCOEXPORT dwyco_set_cmd_path(const char *cmd, int len);

void DWYCOEXPORT dwyco_setup_autoupdate(const char *f1, const char *f2, const char *f3, const char *f4);
void DWYCOEXPORT dwyco_force_autoupdate_check();
void DWYCOEXPORT dwyco_set_autoupdate_status_callback(DwycoAutoUpdateStatusCallback sb);
int DWYCOEXPORT dwyco_start_autoupdate_download(DwycoStatusCallback cb, void *arg1, DwycoAutoUpdateDownloadCallback dcb);
void DWYCOEXPORT dwyco_abort_autoupdate_download();

int DWYCOEXPORT dwyco_get_expired();
int DWYCOEXPORT dwyco_is_registered();
void DWYCOEXPORT dwyco_set_regcode(const char *s);
void DWYCOEXPORT dwyco_final_setup();
void DWYCOEXPORT dwyco_sub_get(const char **reg_out, int *len_out);

//------------------
// EXTERNAL VIDEO CAPTURE INTERFACE
// use this when the internal stuff in the DLL is not
// available (MAC, Linux) or not useful on windows.
//-------------------
typedef void (DWYCOCALLCONV *DwycoVVCallback)(void *);
typedef int (DWYCOCALLCONV *DwycoIVCallback)(void *);
typedef int (DWYCOCALLCONV *DwycoIVICallback)(void *, int);
typedef void (DWYCOCALLCONV *DwycoVVIICallback)(void *, int, int);
typedef void * (DWYCOCALLCONV *DwycoVidGetDataCallback)(void *,
	int *r, int *c,
    int *bytes, int *fmt, unsigned long *captime);
void DWYCOEXPORT dwyco_set_external_video_capture_callbacks(
	DwycoVVCallback nw,
	DwycoVVCallback del,
	DwycoIVICallback init,
	DwycoIVCallback has_data,
	DwycoVVCallback need,
	DwycoVVCallback pass,
	DwycoVVCallback stop,
	DwycoVidGetDataCallback get_data,
    DwycoVVCallback free_data
);

//-----------------
// EXTERNAL AUDIO CAPTURE INTERFACE
// use this for MAC and LINUX, for Windows
// internal stuff works great.
//---------------
typedef void * (DWYCOCALLCONV *DwycoAudGetDataCallback)(void *, int *, int *);

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
	
void DWYCOEXPORT dwyco_set_rating(int);
int DWYCOEXPORT dwyco_get_rating();

int DWYCOEXPORT dwyco_set_setting(const char *name, const char *value);
int DWYCOEXPORT dwyco_get_setting(const char *name, const char **value, int *len, int *dwyco_type);
// abusive use of macros, this whole thing is
// retarded. should just have all the options
// settable by one function using a string
// or something.

#define DWUIDECLARG_BEGIN
#define DWUIDECLARG(type, name) type name,
#define DWUIDECLARG_OUT(type, name) type *name,
#define DWUIDECLARG_END int dummy = 0

int DWYCOEXPORT
dwyco_set_codec_data(
DWUIDECLARG_BEGIN
DWUIDECLARG(int , agc)
DWUIDECLARG(int , denoise)
DWUIDECLARG(double, audio_delay)
DWUIDECLARG_END
);


int DWYCOEXPORT
dwyco_get_codec_data(
DWUIDECLARG_BEGIN
DWUIDECLARG_OUT(int , agc)
DWUIDECLARG_OUT(int , denoise)
DWUIDECLARG_OUT(double, audio_delay)
DWUIDECLARG_END
);

int DWYCOEXPORT
dwyco_set_user_data(
DWUIDECLARG_BEGIN
DWUIDECLARG(char *, description)
DWUIDECLARG(char *, username)
DWUIDECLARG(char *, email)
DWUIDECLARG(char *, last_name)
DWUIDECLARG(char *, first_name)
DWUIDECLARG_END
);

int DWYCOEXPORT
dwyco_get_user_data(
DWUIDECLARG_BEGIN
DWUIDECLARG_OUT(char *, description)
DWUIDECLARG_OUT(char *, username)
DWUIDECLARG_OUT(char *, email)
DWUIDECLARG_OUT(char *, last_name)
DWUIDECLARG_OUT(char *, first_name)
DWUIDECLARG_END
);

// for icuii, "automatic" should be set, which
// tells the capture driver to figure it out as
// best it can. warning: there may be bugs in here
// related to the driver overriding some settings
// when it is in "automatic" mode. i have to check
// this out more.
int DWYCOEXPORT
dwyco_set_vidcap_data(
DWUIDECLARG_BEGIN
DWUIDECLARG(char *, device)
DWUIDECLARG(char *, and)
DWUIDECLARG(char *, or)
DWUIDECLARG(char *, xor)
DWUIDECLARG(char *, offset)
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
DWUIDECLARG_OUT(char *, device)
DWUIDECLARG_OUT(char *, and)
DWUIDECLARG_OUT(char *, or)
DWUIDECLARG_OUT(char *, xor)
DWUIDECLARG_OUT(char *, offset)
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

// this is miscellaneous UI config
// that isn't used in ICUII.
int DWYCOEXPORT
dwyco_set_config_display(
DWUIDECLARG_BEGIN
DWUIDECLARG(bool, fit_video)
DWUIDECLARG(bool, integral_zoom)
DWUIDECLARG(bool, jumbo_buttons)
DWUIDECLARG(bool, no_buttons)
DWUIDECLARG(bool, small_buttons)
DWUIDECLARG(bool, mini_toolbar)
DWUIDECLARG(bool, blinky)
DWUIDECLARG_END
);


int DWYCOEXPORT
dwyco_get_config_display(
DWUIDECLARG_BEGIN
DWUIDECLARG_OUT(bool, fit_video)
DWUIDECLARG_OUT(bool, integral_zoom)
DWUIDECLARG_OUT(bool, jumbo_buttons)
DWUIDECLARG_OUT(bool, no_buttons)
DWUIDECLARG_OUT(bool, small_buttons)
DWUIDECLARG_OUT(bool, mini_toolbar)
DWUIDECLARG_OUT(bool, blinky)
DWUIDECLARG_END
);

#if 0
// this information is set when you call
// "connect_all", but could be set directly
// from an "advanced" connect box if needed.
// ICUII doesn't use this.
DWYCOEXPORT
int
dwyco_set_conn_remote_data(
DWUIDECLARG_BEGIN
DWUIDECLARG(bool, connrecv)
DWUIDECLARG(bool, connsend)
DWUIDECLARG(bool, connchat)
DWUIDECLARG(bool, connrecv_audio)
DWUIDECLARG(bool, connsend_audio)
DWUIDECLARG(bool, block_on_connect)
DWUIDECLARG(bool, pchat)
DWUIDECLARG(char *, pw)
DWUIDECLARG_END
);

DWYCOEXPORT
int
dwyco_get_conn_remote_data(
DWUIDECLARG_BEGIN
DWUIDECLARG_OUT(bool, connrecv)
DWUIDECLARG_OUT(bool, connsend)
DWUIDECLARG_OUT(bool, connchat)
DWUIDECLARG_OUT(bool, connrecv_audio)
DWUIDECLARG_OUT(bool, connsend_audio)
DWUIDECLARG_OUT(bool, block_on_connect)
DWUIDECLARG_OUT(bool, pchat)
DWUIDECLARG_OUT(char *, pw)
DWUIDECLARG_END
);
#endif

// this is only used for testing without
// a camera. it is NOT used in ICUII.
int DWYCOEXPORT
dwyco_set_raw_files(
DWUIDECLARG_BEGIN
DWUIDECLARG(char *, raw_files_list)
DWUIDECLARG(char *, raw_files_pattern)
DWUIDECLARG(bool, use_list_of_files)
DWUIDECLARG(bool, use_pattern)
DWUIDECLARG(bool, preload)
DWUIDECLARG_END
);

int DWYCOEXPORT
dwyco_get_raw_files(
DWUIDECLARG_BEGIN
DWUIDECLARG_OUT(char *, raw_files_list)
DWUIDECLARG_OUT(char *, raw_files_pattern)
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
DWUIDECLARG(char *, device_name)	// icuii: use this to store the capture driver name if you need to. NOTE: not used by the DLL otherwise.
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
DWUIDECLARG_OUT(char *, device_name)
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
DWUIDECLARG(char * , pw)			// icuii: connection password required to connect
DWUIDECLARG(bool, auto_accept)		// icuii: 1 if "accept calls automatically" is checked
DWUIDECLARG(bool, require_pw)       // icuii: 1 if "require password" is checked
DWUIDECLARG(bool, accept_any_rating)// icuii: 1 if "accept calls from other ratings" is checked
DWUIDECLARG(bool, no_listen)		// icuii: always 0
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
DWUIDECLARG_OUT(char * , pw)
DWUIDECLARG_OUT(bool, auto_accept)
DWUIDECLARG_OUT(bool, require_pw)
DWUIDECLARG_OUT(bool, accept_any_rating)
DWUIDECLARG_OUT(bool, no_listen)
DWUIDECLARG_END
);

// assorted zap message setup
int DWYCOEXPORT
dwyco_set_zap_data(
DWUIDECLARG_BEGIN
DWUIDECLARG(bool, always_server)	// icuii: always 0
DWUIDECLARG(bool, always_accept)    // icuii: 1 if "auto-accept quick messages" is checked
DWUIDECLARG(bool, ignore)           // icuii: always 0
DWUIDECLARG(bool, recv_all)         // icuii: always 1
DWUIDECLARG(bool, zsave)            // icuii: always 0
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
DWUIDECLARG_OUT(bool, ignore)
DWUIDECLARG_OUT(bool, recv_all)
DWUIDECLARG_OUT(bool, zsave)
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
DWUIDECLARG(long, expected_rate)	// not used
DWUIDECLARG(double, max_frame_rate)	// icuii: set to 10 (could be a slider tho) fractional frame rates are accepted
DWUIDECLARG(long, max_udp_bytes) 	// not used, set to 65535
DWUIDECLARG(long, ref_interval)		// icuii: always 10000
DWUIDECLARG(long, link_speed)       // set to Kbps xmit on link (modem tab)
DWUIDECLARG(long, link_speed_recv)  // set to Kbps recv on link (modem tab)
DWUIDECLARG_END
);

int DWYCOEXPORT
dwyco_get_rate_tweaks(
DWUIDECLARG_BEGIN
DWUIDECLARG_OUT(long, expected_rate)
DWUIDECLARG_OUT(double, max_frame_rate)
DWUIDECLARG_OUT(long, max_udp_bytes)
DWUIDECLARG_OUT(long, ref_interval)
DWUIDECLARG_OUT(long, link_speed)
DWUIDECLARG_OUT(long, link_speed_recv)
DWUIDECLARG_END
);

// used for media select arg in the net data
// WARNING: the values of these defines were copied from aconn.h
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
DWUIDECLARG(bool, force_non_firewall_friendly) // icuii: 0
DWUIDECLARG(int, nat_primary_port)			// icuii: 0
DWUIDECLARG(int, nat_secondary_port)		// icuii: 0
DWUIDECLARG(int, nat_pal_port)				// icuii: 0
DWUIDECLARG(bool, advertise_nat_ports)		// icuii: 0
DWUIDECLARG(int, disable_upnp)				// icuii: 0 , disabled for compat right now
DWUIDECLARG(int, media_select)				// defaults to "handshake", see *MEDIA_SEL* defines
DWUIDECLARG_END
);

int DWYCOEXPORT
dwyco_get_net_data(
DWUIDECLARG_BEGIN
DWUIDECLARG_OUT(int, primary_port)
DWUIDECLARG_OUT(int, secondary_port)
DWUIDECLARG_OUT(int, pal_port)	
DWUIDECLARG_OUT(bool, force_non_firewall_friendly)
DWUIDECLARG_OUT(int, nat_primary_port)
DWUIDECLARG_OUT(int, nat_secondary_port)
DWUIDECLARG_OUT(int, nat_pal_port)
DWUIDECLARG_OUT(bool, advertise_nat_ports)
DWUIDECLARG_OUT(int, disable_upnp)
DWUIDECLARG_OUT(int, media_select)
DWUIDECLARG_END
);

}

#endif
