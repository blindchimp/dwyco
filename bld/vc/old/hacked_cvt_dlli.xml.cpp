// note: 5/22/2008, i updated the dlli.h file to include some
// new stuff i added, but didn't add all of the new functions.
// you'll need to diff against the earlier version of dlli.h
// to figure out which stuff is missing.
//
// callbacks are handled like this:
//
// when LH calls a static callback set function, like
// set_directory_downloaded_callback(fun-name)
// the fun-name is a string
// in the convert function, we return a constant C function
// that will get installed in the dll and be called by it.
// this C function will get the callback, and the C function
// will use the stored value of the string to lookup a function
// and eval that function in LH.
//
// for functions that get callbacks that are associated with
// specific non-static contexts (like calls and zap sends)
// the callback arg package is explicitly saved, and an entry
// in a map is made that maps the context to the arg package.
// we then usurp the void * usually associated with such callbacks
// to use as a context identifier.
// again, we return a contant C function for the callback to the wrapper.
// this C function, when it gets the callback, uses the void * arg
// to find the user arg package, and then evals the users function
// with the right args.
// note: there is no way to know when an arg package is done
// being used, since some callbacks can be called repeatedly
// (like the status callback). should figure out a way of re-using
// arg packages, which will reduce the number that end up hanging around.
// alternately, we could piecemeal figure out when a context is done and
// delete the arg packages then.
//
// "out" parameters are handled in the convert function by
// allocating memory with malloc and returning a pointer to this
// memory. it is this pointer that is sent into the wrapped call, and
// into which the "out" result is presumably put.
// calls in LH send in strings as the "out" parameter, and
// this is used as a variable that is local-binded to the
// "out" parameter being returned by the function. the convert function
// appends a (name, ptr) pair to a global vector that is set up
// in the calling wrapper. when the wrapped function call returns
// the wrapper function traverses the vector and performs the
// lbinds, and then disposes of the temporaries.
//
// in-out parameters cannot be handled very easily in LH, and so in those
// cases, the LH function has a slightly different signature, usually
// an "in" parameter, and a separate "out" parameter.

#include "dwmapr.h"

static long unsigned int hash(const long& l)
{
	return l * 0x31415926;
}
static long AP_id;
static vc Arg_packages;
// an entry is made in this vector each time
// a callback ctx is created that needs to be killed
// when the callback ctx is finished.
struct idpair
{
	long ctx;
	long arg_package;
	int operator==(const idpair& i1) const {if(ctx == i1.ctx) return 1; return 0;}
};
static DwVec<struct idpair> Ctx_kill;
template class DwVec<struct idpair>;

// a "static context" that we are unlikely to ever get to
// during normal course of events.
#define AUTOUPDATE_CTX 0x10000000

static
DWYCOCALLCONV
void
real_dwyco_status_callback_bounce(int id, const char *msg, int percent_done, void *user_arg)
{
	// we stole the user_arg to use in finding the arg package to
	// send to the user.
	vc args;
	if(!Arg_packages.find((long)user_arg, args))
		oopanic("unregistered status callback");
	vc fn = args[0];
	VCArglist nargs;
	nargs[0] = id;
	nargs[1] = msg;
	nargs[2] = percent_done;
	nargs[3] = args[1];
	fn(&nargs);
}

static
DWYCOCALLCONV
void
real_dwyco_autoupdate_download_callback_bounce(int status)
{
	// we stole the user_arg to use in finding the arg package to
	// send to the user.
	vc args;
	if(!Arg_packages.find((long)AUTOUPDATE_CTX, args))
		oopanic("unregistered autoupdate download status callback");
	// note: this is kinda special compared to other arg packages since
	// we put two functions into one package. that's cuz autoupdate is
	// kinda not like the others... which probably is retarded.
	vc fn = args[2];
	VCArglist nargs;
	nargs[0] = status;
	fn(&nargs);
}

static
DWYCOCALLCONV
void
real_dwyco_message_send_callback_bounce(int id, int what, char *recip, int reciplen, const char *server_msg, void *user_arg)
{
	// we stole the user_arg to use in finding the arg package to
	// send to the user.
	vc args;
	if(!Arg_packages.find((long)user_arg, args))
		oopanic("unregistered message send callback");
	vc fn = args[0];
	VCArglist nargs;
	nargs[0] = id;
	nargs[1] = what;
	nargs[2] = vc(VC_BSTRING, recip, reciplen);
	if(server_msg)
		nargs[3] = server_msg;
	else
		nargs[3] = vcnil;
	nargs[4] = args[1];
	fn(&nargs);
}


static
DWYCOCALLCONV
void
real_dwyco_call_disposition_callback_bounce(int call_id, int chan_id, int what, void *user_arg, const char *uid, int len_uid, const char *call_type, int len_call_type)
{
	// we stole the user_arg to use in finding the arg package to
	// send to the user.
	vc args;
	if(!Arg_packages.find((long)user_arg, args))
		oopanic("unregistered call disp callback");
	vc fn = args[0];
	VCArglist nargs;
	nargs[0] = call_id;
	nargs[1] = chan_id;
	nargs[2] = what;
	nargs[3] = args[1];
	nargs[4] = vc(VC_BSTRING, uid, len_uid);
	nargs[5] = vc(VC_BSTRING, call_type, len_call_type);
	fn(&nargs);
}

static
DWYCOCALLCONV
void
real_dwyco_message_download_callback_bounce(int id, int what, const char *msg_id, void *mdc_arg1)
{
	// we stole the user_arg to use in finding the arg package to
	// send to the user.
	vc args;
	if(!Arg_packages.find((long)mdc_arg1, args))
		oopanic("unregistered message download callback");
	vc fn = args[0];
	VCArglist nargs;
	nargs[0] = id;
	nargs[1] = what;
	nargs[2] = msg_id;
	nargs[3] = args[1];
	fn(&nargs);
}

static vc
cvt__int(int foo)
{
	return foo;
}

static vc
cvt_p_void(void* foo)
{
	return (long)foo;
}

// bad: this assumed zero termination. you'll have to ferret out
// cases where this is not right for the call and munge it by hand.
static vc
cvt_pq_char(const char* foo)
{
	return foo;
}


static vc dwyco_autoupdate_status_callback;
static
void
DWYCOCALLCONV
real_dwyco_auto_update_status_callback(int status, const char *desc)
{
	VCArglist args;
	args[0] = status;
	args[1] = desc;
	dwyco_autoupdate_status_callback(&args);
}

static
DwycoAutoUpdateStatusCallback
cvt_vc_DwycoAutoUpdateStatusCallback(vc a)
{
	dwyco_autoupdate_status_callback = a;
	if(a.is_nil())
		return 0;
	return real_dwyco_auto_update_status_callback;
}

static vc dwyco_call_appearance_callback;
static void
DWYCOCALLCONV
real_dwyco_call_appearance_callback(int id, const char *name, const char *loc, const char *uid, int len_uid,
	const char *call_type, int len_call_type)
{
	VCArglist args;
	args[0] = id;
	args[1] = name;
	args[2] = loc;
	args[3] = vc(VC_BSTRING, uid, len_uid);
	args[4] = vc(VC_BSTRING, call_type, len_call_type);
	dwyco_call_appearance_callback(&args);
}

static
DwycoCallAppearanceCallback
cvt_vc_DwycoCallAppearanceCallback(vc a)
{
	dwyco_call_appearance_callback = a;
	if(a.is_nil())
		return 0;
	return real_dwyco_call_appearance_callback;
}

static vc dwyco_call_acceptance_callback;
static void
DWYCOCALLCONV
real_dwyco_call_acceptance_callback(int id, const char *name, const char *loc, const char *uid, int len_uid, const char *call_type, int len_call_type)
{
	VCArglist args;
	args[0] = id;
	args[1] = name;
	args[2] = loc;
	args[3] = vc(VC_BSTRING, uid, len_uid);
	args[4] = vc(VC_BSTRING, call_type, len_call_type);
	dwyco_call_acceptance_callback(&args);
}

// note: looks wrong, but isn't. type is being used
// kinda overloaded here
static
DwycoCallAppearanceCallback
cvt_vc_DwycoCallAcceptanceCallback(vc a)
{
	dwyco_call_acceptance_callback = a;
	if(a.is_nil())
		return 0;
	return real_dwyco_call_acceptance_callback;
}

static vc dwyco_call_appearance_death_callback;
static void
DWYCOCALLCONV
real_dwyco_call_appearance_death_callback(int id)
{
	VCArglist args;
	args[0] = id;
	dwyco_call_appearance_death_callback(&args);
}

static
DwycoCallAppearanceDeathCallback
cvt_vc_DwycoCallAppearanceDeathCallback(vc a)
{
	dwyco_call_appearance_death_callback = a;
	if(a.is_nil())
		return 0;
	return real_dwyco_call_appearance_death_callback;
}

static vc dwyco_chat_ctx_callback;
static void
DWYCOCALLCONV
real_dwyco_chat_ctx_callback(int cmd, int id, 
	const char *uid, int len_uid, 
	const char *name, int len_name,
	int type, const char *val, int val_len,
	int qid,
	int extra_arg)
{
	VCArglist args;
	args[0] = cmd;
	args[1] = id;
	args[2] = vc(VC_BSTRING, uid, len_uid);
	args[3] = vc(VC_BSTRING, name, len_name);
	args[4] = type;
	args[5] = vc(VC_BSTRING, val, val_len);
	args[6] = qid;
	args[7] = extra_arg;
	dwyco_chat_ctx_callback(&args);
}

static
DwycoChatCtxCallback
cvt_vc_DwycoChatCtxCallback(vc a)
{
	dwyco_chat_ctx_callback = a;
	if(a.is_nil())
		return 0;
	return real_dwyco_chat_ctx_callback;
}

static vc dir_downloaded_callback;
static
DWYCOCALLCONV
void
real_dir_downloaded_callback(DWYCO_DIR_LIST l)
{
	VCArglist args;
	args[0] = (long)l;
	dir_downloaded_callback(&args);
}

static
DwycoDirectoryDownloadedCallback
cvt_vc_DwycoDirectoryDownloadedCallback(vc a)
{
	dir_downloaded_callback = a;
	if(a.is_nil())
		return 0;
	return real_dir_downloaded_callback;
}

static vc dwyco_unregister_callback;
static
DWYCOCALLCONV
void
real_dwyco_unregister_callback(int id, const char *msg, int percent_done, void *user_arg)
{
	VCArglist args;
	args[0] = id;
	args[1] = msg;
	args[2] = percent_done;
	args[3] = (long)user_arg;
	dwyco_unregister_callback(&args);
}

static vc dwyco_server_login_callback;
static
DWYCOCALLCONV
void
real_dwyco_server_login_callback(const char *str, int what)
{
	VCArglist args;
	args[0] = str;
	args[1] = what;
	dwyco_server_login_callback(&args);
}

static vc dwyco_private_chat_init_callback;

static
DWYCOCALLCONV
int
real_dwyco_private_chat_init_callback(int chan_id, int ui_id, const char *caption)
{
	VCArglist args;
	args[0] = chan_id;
	args[1] = ui_id;
	args[2] = caption;
	vc ret = dwyco_private_chat_init_callback(&args);
	return ret;
}

static
DwycoPrivateChatInitCallback
cvt_vc_DwycoPrivateChatInitCallback(vc a)
{
	dwyco_private_chat_init_callback = a;
	if(a.is_nil())
		return 0;
	return real_dwyco_private_chat_init_callback;
}

static vc dwyco_public_chat_init_callback;

static
DWYCOCALLCONV
int
real_dwyco_public_chat_init_callback(int ui_id)
{
	VCArglist args;
	args[0] = ui_id;
	vc ret = dwyco_public_chat_init_callback(&args);
	return ret;
}

static
DwycoPublicChatInitCallback
cvt_vc_DwycoPublicChatInitCallback(vc a)
{
	dwyco_public_chat_init_callback = a;
	if(a.is_nil())
		return 0;
	return real_dwyco_public_chat_init_callback;
}

static
DwycoPalAuthCallback
cvt_vc_DwycoPalAuthCallback(vc a)
{
}

static
DwycoPrivateChatDisplayCallback
cvt_vc_DwycoPrivateChatDisplayCallback(vc a)
{
}


static vc dwyco_public_chat_display_callback;
static
DWYCOCALLCONV
int
real_dwyco_public_chat_display_callback(
	const char *who, int len_who,
	const char *line, int len_line,
	const char *uid, int len_uid)
{
	VCArglist args;
	args[0] = vc(VC_BSTRING, who, len_who);
	args[1] = vc(VC_BSTRING, line, len_line);
	args[2] = vc(VC_BSTRING, uid, len_uid);
	dwyco_public_chat_display_callback(&args);
	return 0;
}

static
DwycoPublicChatDisplayCallback
cvt_vc_DwycoPublicChatDisplayCallback(vc a)
{
	dwyco_public_chat_display_callback = a;
	if(a.is_nil())
		return 0;
	return real_dwyco_public_chat_display_callback;
}

static
DwycoServerLoginCallback
cvt_vc_DwycoServerLoginCallback(vc a)
{
	dwyco_server_login_callback = a;
	if(a.is_nil())
		return 0;
	return real_dwyco_server_login_callback;
}

static vc dwyco_chat_server_status_callback;
static
DWYCOCALLCONV
void
real_dwyco_chat_server_status_callback(int id, const char *msg, int percent_done, void *user_arg)
{
	VCArglist args;
	args[0] = id;
	args[1] = msg;
	args[2] = 0;
	args[3] = 0;
	dwyco_chat_server_status_callback(&args);
}

static
DwycoStatusCallback
cvt_vc_DwycoChatServerStatusCallback(vc a)
{
	dwyco_chat_server_status_callback = a;
	if(a.is_nil())
		return 0;
	return real_dwyco_chat_server_status_callback;
}

static vc dwyco_video_display_callback;
static
DWYCOCALLCONV
void
real_dwyco_video_display_callback(int ui_id, void *img, int cols, int rows, int depth)
{
	VCArglist args;
	args[0] = ui_id;
	args[1] = (long)img;
	args[2] = cols;
	args[3] = rows;
	args[4] = depth;
	dwyco_video_display_callback(&args);
}

static
DwycoVideoDisplayCallback
cvt_vc_DwycoVideoDisplayCallback(vc a)
{
	dwyco_video_display_callback = a;
	if(a.is_nil())
		return 0;
	return real_dwyco_video_display_callback;
}

static vc dwyco_video_display_init_callback;
static
DWYCOCALLCONV
void
real_dwyco_video_display_init_callback(int chan_id, int ui_id)
{
	VCArglist args;
	args[0] = chan_id;
	args[1] = ui_id;
	dwyco_video_display_init_callback(&args);
}

static
DwycoVideoDisplayInitCallback
cvt_vc_DwycoVideoDisplayInitCallback(vc a)
{
	dwyco_video_display_init_callback = a;
	if(a.is_nil())
		return 0;
	return real_dwyco_video_display_init_callback;
}

static
DwycoZapAppearanceCallback
cvt_vc_DwycoZapAppearanceCallback(vc a)
{
}

static
DwycoStatusCallback
cvt_vc_DwycoUnregisterCallback(vc a)
{
	dwyco_unregister_callback = a;
	if(a.is_nil())
		return 0;
	return real_dwyco_unregister_callback;
}

static
DwycoStatusCallback
cvt_vc_DwycoMotdCallback(vc a)
{
}

static
DwycoUserControlCallback
cvt_vc_DwycoUserControlCallback(vc a)
{
}
//-------------end mostly static single callbacks

static
DwycoAutoUpdateDownloadCallback
cvt_vc_DwycoAutoUpdateDownloadCallback(vc a)
{
	if(a.is_nil())
		return 0;
	return real_dwyco_autoupdate_download_callback_bounce;
}

static
DwycoAudGetDataCallback
cvt_vc_DwycoAudGetDataCallback(vc a)
{
}

static
DwycoCallDispositionCallback
cvt_vc_DwycoCallDispositionCallback(vc a)
{
	if(a.is_nil())
		return 0;
	return real_dwyco_call_disposition_callback_bounce;
}
static
DwycoCallEstablishedCallback
cvt_vc_DwycoCallEstablishedCallback(vc a)
{
}
static
DwycoChannelDestroyCallback
cvt_vc_DwycoChannelDestroyCallback(vc a)
{
}
static
DwycoCommandCallback
cvt_vc_DwycoCommandCallback(vc a)
{
}
static
DwycoDevDoneCallback
cvt_vc_DwycoDevDoneCallback(vc a)
{
}
static
DwycoDevOutputCallback
cvt_vc_DwycoDevOutputCallback(vc a)
{
}


static
DwycoEIMServerLoginCallback
cvt_vc_DwycoEIMServerLoginCallback(vc a)
{
}
static
DwycoIVCallback
cvt_vc_DwycoIVCallback(vc a)
{
}
static
DwycoIVICallback
cvt_vc_DwycoIVICallback(vc a)
{
}
static

DwycoMessageDownloadCallback
cvt_vc_DwycoMessageDownloadCallback(vc a)
{
	if(a.is_nil())
		return 0;
	return real_dwyco_message_download_callback_bounce;
}

static
DwycoMessageSendCallback
cvt_vc_DwycoMessageSendCallback(vc a)
{
	if(a.is_nil())
		return 0;
	return real_dwyco_message_send_callback_bounce;
}
static
DwycoProfileCallback
cvt_vc_DwycoProfileCallback(vc a)
{
}

static
DwycoStatusCallback
cvt_vc_DwycoStatusCallback(vc a)
{
	if(a.is_nil())
		return 0;
	return real_dwyco_status_callback_bounce;
}

static
DwycoVVIICallback
cvt_vc_DwycoVVIICallback(vc a)
{
}
static
DwycoVidGetDataCallback
cvt_vc_DwycoVidGetDataCallback(vc a)
{
}
static
bool
cvt_vc__bool(vc a)
{
	return !a.is_nil();
}
static
double
cvt_vc__double(vc a)
{
	return a;
}
static
int
cvt_vc__int(vc a)
{
	return a;
}
static
long int
cvt_vc__long_int(vc a)
{
	return a;
}

// always out
static
bool*
cvt_vc_p_bool(vc a)
{
	if(a.is_nil())
		return 0;
	bool *v = (bool *)malloc(sizeof(bool));
	vc bv(VC_VECTOR);
	bv[0] = a;
	bv[1] = (long)v;
	outbind_bool->append(bv);
	push_ptr(v);
	return v;
}

// always in
static
char*
cvt_vc_p_char(vc a)
{
	if(a.is_nil())
		return 0;
	return (char *)(const char *)a;
}

// always out
static
double*
cvt_vc_p_double(vc a)
{
	if(a.is_nil())
		return 0;
	double *v = (double *)malloc(sizeof(double));
	vc bv(VC_VECTOR);
	bv[0] = a;
	bv[1] = (long)v;
	outbind_double->append(bv);
	push_ptr(v);
	return v;
}


// always out
static
int*
cvt_vc_p_int(vc a)
{
	if(a.is_nil())
		return 0;
	int *v = (int *)malloc(sizeof(int));
	vc bv(VC_VECTOR);
	bv[0] = a;
	bv[1] = (long)v;
	outbind_int->append(bv);
	push_ptr(v);
	return v;
}

static
long int*
cvt_vc_p_long_int(vc a)
{
	if(a.is_nil())
		return 0;
	long *v = (long *)malloc(sizeof(long));
	vc bv(VC_VECTOR);
	bv[0] = a;
	bv[1] = (long)v;
	outbind_long->append(bv);
	push_ptr(v);
	return v;
}

static
void*
cvt_vc_p_void(vc a)
{
	return a;
}


static
char**
cvt_vc_pp_char(vc a)
{
	int n = a.num_elems();
	char **v = (char **)malloc(n * sizeof(char *));
	for(int i = 0; i < n; ++i)
	{
		v[i] = (char *)(const char *)a[i];
	}
	push_ptr(v);
	return v;
}

// for out params, the LH function sends in a string that is
// to be lbinded with the results.
static
char**
cvt_vc_pp_char_out(vc a)
{
	if(a.is_nil())
		return 0;
	char **v = (char **)malloc(sizeof(char *));
	vc bv(VC_VECTOR);
	bv[0] = a;
	bv[1] = (long)v;
	outbind_str->append(bv);
	push_ptr(v);
	return v;
}

// for out params, the LH function sends in a string that is
// to be lbinded with the results.
static
const char**
cvt_vc_ppq_char_out(vc a)
{
	if(a.is_nil())
		return 0;
	const char **v = (const char **)malloc(sizeof(const char *));
	vc bv(VC_VECTOR);
	bv[0] = a;
	bv[1] = (long)v;
	outbind_str->append(bv);
	push_ptr(v);
	return v;
}

// always out
static
void**
cvt_vc_pp_void(vc a)
{
	if(a.is_nil())
		return 0;
	void **v = (void **)malloc(sizeof(void *));
	vc bv(VC_VECTOR);
	bv[0] = a;
	bv[1] = (long)v;
	outbind_pvoid->append(bv);
	push_ptr(v);
	return v;
}

// in
static
const char**
cvt_vc_ppq_char(vc a)
{
	int n = a.num_elems();
	const char **v = (const char **)malloc(n * sizeof(char *));
	for(int i = 0; i < n; ++i)
	{
		v[i] = (const char *)a[i];
	}
	push_ptr(v);
	return v;
}

static
const char*
cvt_vc_pq_char(vc a)
{
	if(a.is_nil())
		return 0;
	return a;
}
