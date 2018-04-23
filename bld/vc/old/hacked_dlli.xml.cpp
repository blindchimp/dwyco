
#include "vc.h"
#include "dwvecp.h"

static DwVecP<void> Arg_stk;
static DwVec<vc> *outbind_bool;
static DwVec<vc> *outbind_int;
static DwVec<vc> *outbind_pvoid;
static DwVec<vc> *outbind_long;
static DwVec<vc> *outbind_str;
static DwVec<vc> *outbind_double;
typedef DwVec<vc> vvc;

template class DwVecP<void>;
static
int
start_wrapper()
{
	return Arg_stk.num_elems();
}

static
void
end_wrapper(int i)
{
	int j = Arg_stk.num_elems();
	for(int k = j - 1; k >= i; --k)
		free(Arg_stk[k]);
	Arg_stk.set_size(i);
}

static void
push_ptr(void *p)
{
	Arg_stk.append(p);
}

#define START_WRAP int i = start_wrapper(); \
DwVec<vc> loutbind_bool; \
DwVec<vc> loutbind_int; \
DwVec<vc> loutbind_pvoid; \
DwVec<vc> loutbind_long; \
DwVec<vc> loutbind_str; \
DwVec<vc> loutbind_double; \
vvc *soutbind_bool = outbind_bool; \
outbind_bool = &loutbind_bool; \
vvc *soutbind_int = outbind_int; \
outbind_int = &loutbind_int; \
vvc *soutbind_pvoid = outbind_pvoid; \
outbind_pvoid = &loutbind_pvoid; \
vvc *soutbind_long = outbind_long; \
outbind_long = &loutbind_long; \
vvc *soutbind_str = outbind_str; \
outbind_str = &loutbind_str; \
vvc *soutbind_double = outbind_double; \
outbind_double = &loutbind_double;

#define START_WRAP_misc int i = start_wrapper(); \
DwVec<vc> loutbind_int; \
DwVec<vc> loutbind_str; \
vvc *soutbind_int = outbind_int; \
outbind_int = &loutbind_int; \
vvc *soutbind_str = outbind_str; \
outbind_str = &loutbind_str;

	
// WARNING: this assumes that strings are 0 terminated
#define outbind(type, ctype) \
	{int n = outbind_##type->num_elems(); \
	for(int i = 0; i < n; ++i) \
	{\
		vc bv = (*outbind_##type)[i]; \
		bv[0].local_bind(*(ctype *)(void *)bv[1]); \
	}\
	outbind_##type = soutbind_##type; \
	}

#define WRAP_END  \
	outbind(bool, bool) \
	outbind(int, int) \
	outbind(pvoid, long) \
	outbind(long, long) \
	outbind(str, const char *) \
	outbind(double, double) \
	end_wrapper(i);

#define WRAP_END_misc  \
	vc bv = (*outbind_str)[0]; \
	vc bv_len = (*outbind_int)[0]; \
	int len = *(int *)(void *)bv_len[1]; \
	bv[0].local_bind(vc(VC_BSTRING, (const char *)(void *)bv[1], len)); \
	outbind_int = soutbind_int; \
	outbind_str = soutbind_str; \
	end_wrapper(i);

#define CTX_INTERPOSE(apid, fp_argnum, vp_argnum) \
void *scb_arg1 = cvt_vc_p_void((*a)[vp_argnum]); \
vc args(VC_VECTOR); \
args[0] = (*a)[fp_argnum]; \
args[1] = (long)scb_arg1; \
long ap_id = apid; \
Arg_packages.add_kv(ap_id, args);

#define CTX_KILL \
{\
struct idpair idp; \
idp.ctx = (long)ret; \
idp.arg_package = ap_id; \
Ctx_kill.append(idp);\
}

#define CTX_INTERPOSE2(apid, fp_argnum, vp_argnum) \
scb_arg1 = cvt_vc_p_void((*a)[vp_argnum]); \
args = vc(VC_VECTOR); \
args[0] = (*a)[fp_argnum]; \
args[1] = (long)scb_arg1; \
long ap_id2 = apid; \
Arg_packages.add_kv(ap_id2, args);

#define CTX_KILL2 \
{\
struct idpair idp; \
idp.ctx = (long)ret; \
idp.arg_package = ap_id2; \
Ctx_kill.append(idp);\
}


#include "dlli.h"
#include "hacked_cvt_dlli.xml.cpp"

	static vc
wrap_dwyco_get_net_data(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_net_data(cvt_vc_p_int((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc_p_int((*a)[2]),
cvt_vc_p_bool((*a)[3]),
cvt_vc_p_int((*a)[4]),
cvt_vc_p_int((*a)[5]),
cvt_vc_p_int((*a)[6]),
cvt_vc_p_bool((*a)[7]),
cvt_vc_p_int((*a)[8]),
cvt_vc_p_int((*a)[9]),
cvt_vc__int((*a)[10])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_net_data(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_net_data(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__bool((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc__bool((*a)[7]),
cvt_vc__int((*a)[8]),
cvt_vc__int((*a)[9]),
cvt_vc__int((*a)[10])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_rate_tweaks(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_rate_tweaks(cvt_vc_p_long_int((*a)[0]),
cvt_vc_p_double((*a)[1]),
cvt_vc_p_long_int((*a)[2]),
cvt_vc_p_long_int((*a)[3]),
cvt_vc_p_long_int((*a)[4]),
cvt_vc_p_long_int((*a)[5]),
cvt_vc__int((*a)[6])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_rate_tweaks(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_rate_tweaks(cvt_vc__long_int((*a)[0]),
cvt_vc__double((*a)[1]),
cvt_vc__long_int((*a)[2]),
cvt_vc__long_int((*a)[3]),
cvt_vc__long_int((*a)[4]),
cvt_vc__long_int((*a)[5]),
cvt_vc__int((*a)[6])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_zap_data(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_zap_data(cvt_vc_p_bool((*a)[0]),
cvt_vc_p_bool((*a)[1]),
cvt_vc_p_bool((*a)[2]),
cvt_vc_p_bool((*a)[3]),
cvt_vc_p_bool((*a)[4]),
cvt_vc_p_bool((*a)[5]),
cvt_vc_p_bool((*a)[6]),
cvt_vc_p_bool((*a)[7]),
cvt_vc__int((*a)[8])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_zap_data(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_zap_data(cvt_vc__bool((*a)[0]),
cvt_vc__bool((*a)[1]),
cvt_vc__bool((*a)[2]),
cvt_vc__bool((*a)[3]),
cvt_vc__bool((*a)[4]),
cvt_vc__bool((*a)[5]),
cvt_vc__bool((*a)[6]),
cvt_vc__bool((*a)[7]),
cvt_vc__int((*a)[8])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_call_accept(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_call_accept(cvt_vc_p_int((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc_p_int((*a)[2]),
cvt_vc_p_int((*a)[3]),
cvt_vc_p_int((*a)[4]),
cvt_vc_p_int((*a)[5]),
cvt_vc_pp_char_out((*a)[6]),
cvt_vc_p_bool((*a)[7]),
cvt_vc_p_bool((*a)[8]),
cvt_vc_p_bool((*a)[9]),
cvt_vc_p_bool((*a)[10]),
cvt_vc__int((*a)[11])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_call_accept(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_call_accept(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc_p_char((*a)[6]),
cvt_vc__bool((*a)[7]),
cvt_vc__bool((*a)[8]),
cvt_vc__bool((*a)[9]),
cvt_vc__bool((*a)[10]),
cvt_vc__int((*a)[11])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_video_input(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_video_input(cvt_vc_pp_char_out((*a)[0]),
cvt_vc_p_bool((*a)[1]),
cvt_vc_p_bool((*a)[2]),
cvt_vc_p_bool((*a)[3]),
cvt_vc_p_bool((*a)[4]),
cvt_vc_p_int((*a)[5]),
cvt_vc__int((*a)[6])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_video_input(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_video_input(cvt_vc_p_char((*a)[0]),
cvt_vc__bool((*a)[1]),
cvt_vc__bool((*a)[2]),
cvt_vc__bool((*a)[3]),
cvt_vc__bool((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_raw_files(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_raw_files(cvt_vc_pp_char_out((*a)[0]),
cvt_vc_pp_char_out((*a)[1]),
cvt_vc_p_bool((*a)[2]),
cvt_vc_p_bool((*a)[3]),
cvt_vc_p_bool((*a)[4]),
cvt_vc__int((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_raw_files(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_raw_files(cvt_vc_p_char((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc__bool((*a)[2]),
cvt_vc__bool((*a)[3]),
cvt_vc__bool((*a)[4]),
cvt_vc__int((*a)[5])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_dwyco_get_conn_remote_data(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_conn_remote_data(cvt_vc_p_bool((*a)[0]),
cvt_vc_p_bool((*a)[1]),
cvt_vc_p_bool((*a)[2]),
cvt_vc_p_bool((*a)[3]),
cvt_vc_p_bool((*a)[4]),
cvt_vc_p_bool((*a)[5]),
cvt_vc_p_bool((*a)[6]),
cvt_vc_pp_char_out((*a)[7]),
cvt_vc__int((*a)[8])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_conn_remote_data(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_conn_remote_data(cvt_vc__bool((*a)[0]),
cvt_vc__bool((*a)[1]),
cvt_vc__bool((*a)[2]),
cvt_vc__bool((*a)[3]),
cvt_vc__bool((*a)[4]),
cvt_vc__bool((*a)[5]),
cvt_vc__bool((*a)[6]),
cvt_vc_p_char((*a)[7]),
cvt_vc__int((*a)[8])));
WRAP_END
return ret;

}
#endif
static vc
wrap_dwyco_get_config_display(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_config_display(cvt_vc_p_bool((*a)[0]),
cvt_vc_p_bool((*a)[1]),
cvt_vc_p_bool((*a)[2]),
cvt_vc_p_bool((*a)[3]),
cvt_vc_p_bool((*a)[4]),
cvt_vc_p_bool((*a)[5]),
cvt_vc_p_bool((*a)[6]),
cvt_vc__int((*a)[7])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_config_display(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_config_display(cvt_vc__bool((*a)[0]),
cvt_vc__bool((*a)[1]),
cvt_vc__bool((*a)[2]),
cvt_vc__bool((*a)[3]),
cvt_vc__bool((*a)[4]),
cvt_vc__bool((*a)[5]),
cvt_vc__bool((*a)[6]),
cvt_vc__int((*a)[7])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_vidcap_data(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_vidcap_data(cvt_vc_pp_char_out((*a)[0]),
cvt_vc_pp_char_out((*a)[1]),
cvt_vc_pp_char_out((*a)[2]),
cvt_vc_pp_char_out((*a)[3]),
cvt_vc_pp_char_out((*a)[4]),
cvt_vc_p_bool((*a)[5]),
cvt_vc_p_bool((*a)[6]),
cvt_vc_p_bool((*a)[7]),
cvt_vc_p_bool((*a)[8]),
cvt_vc_p_bool((*a)[9]),
cvt_vc_p_bool((*a)[10]),
cvt_vc_p_bool((*a)[11]),
cvt_vc_p_bool((*a)[12]),
cvt_vc_p_bool((*a)[13]),
cvt_vc_p_bool((*a)[14]),
cvt_vc_p_bool((*a)[15]),
cvt_vc_p_bool((*a)[16]),
cvt_vc_p_bool((*a)[17]),
cvt_vc__int((*a)[18])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_vidcap_data(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_vidcap_data(cvt_vc_p_char((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc_p_char((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc_p_char((*a)[4]),
cvt_vc__bool((*a)[5]),
cvt_vc__bool((*a)[6]),
cvt_vc__bool((*a)[7]),
cvt_vc__bool((*a)[8]),
cvt_vc__bool((*a)[9]),
cvt_vc__bool((*a)[10]),
cvt_vc__bool((*a)[11]),
cvt_vc__bool((*a)[12]),
cvt_vc__bool((*a)[13]),
cvt_vc__bool((*a)[14]),
cvt_vc__bool((*a)[15]),
cvt_vc__bool((*a)[16]),
cvt_vc__bool((*a)[17]),
cvt_vc__int((*a)[18])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_user_data(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_user_data(cvt_vc_pp_char_out((*a)[0]),
cvt_vc_pp_char_out((*a)[1]),
cvt_vc_pp_char_out((*a)[2]),
cvt_vc_pp_char_out((*a)[3]),
cvt_vc_pp_char_out((*a)[4]),
cvt_vc__int((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_user_data(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_user_data(cvt_vc_p_char((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc_p_char((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc_p_char((*a)[4]),
cvt_vc__int((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_rating(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_rating());
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_rating(VCArglist *a)
{
START_WRAP
	dwyco_set_rating(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_external_audio_output_callbacks(VCArglist *a)
{
START_WRAP
	dwyco_set_external_audio_output_callbacks(cvt_vc_DwycoDirectoryDownloadedCallback((*a)[0]),
cvt_vc_DwycoDirectoryDownloadedCallback((*a)[1]),
cvt_vc_DwycoIVCallback((*a)[2]),
cvt_vc_DwycoDevOutputCallback((*a)[3]),
cvt_vc_DwycoDevDoneCallback((*a)[4]),
cvt_vc_DwycoIVCallback((*a)[5]),
cvt_vc_DwycoIVCallback((*a)[6]),
cvt_vc_DwycoIVCallback((*a)[7]),
cvt_vc_DwycoIVCallback((*a)[8]),
cvt_vc_DwycoIVICallback((*a)[9]),
cvt_vc_DwycoIVCallback((*a)[10]),
cvt_vc_DwycoIVCallback((*a)[11]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_external_audio_capture_callbacks(VCArglist *a)
{
START_WRAP
	dwyco_set_external_audio_capture_callbacks(cvt_vc_DwycoVVIICallback((*a)[0]),
cvt_vc_DwycoDirectoryDownloadedCallback((*a)[1]),
cvt_vc_DwycoIVCallback((*a)[2]),
cvt_vc_DwycoIVCallback((*a)[3]),
cvt_vc_DwycoDirectoryDownloadedCallback((*a)[4]),
cvt_vc_DwycoDirectoryDownloadedCallback((*a)[5]),
cvt_vc_DwycoDirectoryDownloadedCallback((*a)[6]),
cvt_vc_DwycoDirectoryDownloadedCallback((*a)[7]),
cvt_vc_DwycoDirectoryDownloadedCallback((*a)[8]),
cvt_vc_DwycoDirectoryDownloadedCallback((*a)[9]),
cvt_vc_DwycoIVCallback((*a)[10]),
cvt_vc_DwycoAudGetDataCallback((*a)[11]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_external_video_capture_callbacks(VCArglist *a)
{
START_WRAP
	dwyco_set_external_video_capture_callbacks(cvt_vc_DwycoDirectoryDownloadedCallback((*a)[0]),
cvt_vc_DwycoDirectoryDownloadedCallback((*a)[1]),
cvt_vc_DwycoIVICallback((*a)[2]),
cvt_vc_DwycoIVCallback((*a)[3]),
cvt_vc_DwycoDirectoryDownloadedCallback((*a)[4]),
cvt_vc_DwycoDirectoryDownloadedCallback((*a)[5]),
cvt_vc_DwycoDirectoryDownloadedCallback((*a)[6]),
cvt_vc_DwycoVidGetDataCallback((*a)[7]),
cvt_vc_DwycoDirectoryDownloadedCallback((*a)[8]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_final_setup(VCArglist *a)
{
START_WRAP
	dwyco_final_setup();
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_regcode(VCArglist *a)
{
START_WRAP
	dwyco_set_regcode(cvt_vc_p_char((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_is_registered(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_is_registered());
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_expired(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_expired());
WRAP_END
return ret;

}
static vc
wrap_dwyco_abort_autoupdate_download(VCArglist *a)
{
START_WRAP
	dwyco_abort_autoupdate_download();
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_start_autoupdate_download(VCArglist *a)
{
START_WRAP
CTX_INTERPOSE(AUTOUPDATE_CTX, 0, 1)
// note: we can do this because args is put into the
// Arg_package by reference
args[2] = (*a)[2];
	vc ret = cvt__int(dwyco_start_autoupdate_download(cvt_vc_DwycoStatusCallback((*a)[0]),
(void *)AUTOUPDATE_CTX,
cvt_vc_DwycoAutoUpdateDownloadCallback((*a)[2])));
CTX_KILL
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_autoupdate_status_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_autoupdate_status_callback(cvt_vc_DwycoAutoUpdateStatusCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_force_autoupdate_check(VCArglist *a)
{
START_WRAP
	dwyco_force_autoupdate_check();
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_setup_autoupdate(VCArglist *a)
{
START_WRAP
	dwyco_setup_autoupdate(cvt_vc_pq_char((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc_pq_char((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_cmd_path(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_cmd_path(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_zap_quick_stats_view(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_zap_quick_stats_view(cvt_vc__int((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc_p_int((*a)[2]),
cvt_vc_p_int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_zap_stop_view(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_zap_stop_view(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_zap_play_preview(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_zap_play_preview(cvt_vc__int((*a)[0]),
cvt_vc_DwycoChannelDestroyCallback((*a)[1]),
cvt_vc_p_void((*a)[2]),
cvt_vc_p_int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_zap_play_view(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_zap_play_view(cvt_vc__int((*a)[0]),
cvt_vc_DwycoChannelDestroyCallback((*a)[1]),
cvt_vc_p_void((*a)[2]),
cvt_vc_p_int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_delete_zap_view(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_delete_zap_view(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_make_zap_view_file(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_make_zap_view_file(cvt_vc_pq_char((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_make_zap_view(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_make_zap_view(cvt_vc_p_void((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_zap_still_active(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_zap_still_active(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_zap_send_cancel(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_zap_send_cancel(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_zap_send2(VCArglist *a)
{
START_WRAP
CTX_INTERPOSE(AP_id, 7, 8)
AP_id++;
CTX_INTERPOSE2(AP_id, 5, 6)
AP_id++;
	vc ret = cvt__int(dwyco_zap_send2(cvt_vc__int((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc_DwycoMessageSendCallback((*a)[5]),
(void *)ap_id2,
cvt_vc_DwycoStatusCallback((*a)[7]),
(void *)ap_id
));
CTX_KILL
CTX_KILL2
WRAP_END
return ret;

}
static vc
wrap_dwyco_zap_send(VCArglist *a)
{
::abort();
START_WRAP
	vc ret = cvt__int(dwyco_zap_send(cvt_vc__int((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc_DwycoMessageSendCallback((*a)[4]),
cvt_vc_p_void((*a)[5]),
cvt_vc_DwycoStatusCallback((*a)[6]),
cvt_vc_p_void((*a)[7])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_zap_play(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_zap_play(cvt_vc__int((*a)[0]),
cvt_vc_DwycoChannelDestroyCallback((*a)[1]),
cvt_vc_p_void((*a)[2]),
cvt_vc_p_int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_zap_stop(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_zap_stop(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_zap_record2(VCArglist *a)
{
START_WRAP
CTX_INTERPOSE(AP_id, 6, 7)
AP_id++;
	vc ret = cvt__int(dwyco_zap_record2(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc_DwycoStatusCallback((*a)[6]),
(void *)ap_id,
cvt_vc_DwycoChannelDestroyCallback((*a)[8]),
cvt_vc_p_void((*a)[9]),
cvt_vc_p_int((*a)[10])));
CTX_KILL
WRAP_END
return ret;

}
static vc
wrap_dwyco_zap_record(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_zap_record(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc_DwycoChannelDestroyCallback((*a)[5]),
cvt_vc_p_void((*a)[6]),
cvt_vc_p_int((*a)[7])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_delete_zap_composition(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_delete_zap_composition(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_flim(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_flim(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_is_forward_composition(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_is_forward_composition(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_make_special_zap_composition(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_make_special_zap_composition(cvt_vc__int((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_make_forward_zap_composition(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_make_forward_zap_composition(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_make_zap_composition(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_make_zap_composition(cvt_vc_p_char((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_invisible_state(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_invisible_state());
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_invisible_state(VCArglist *a)
{
START_WRAP
	dwyco_set_invisible_state(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_refresh_directory(VCArglist *a)
{
START_WRAP
	dwyco_refresh_directory();
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_room_delete(VCArglist *a)
{
START_WRAP
	dwyco_room_delete(cvt_vc_pq_char((*a)[0]),
cvt_vc_DwycoCommandCallback((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_room_create(VCArglist *a)
{
START_WRAP
	dwyco_room_create(cvt_vc_pq_char((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_DwycoCommandCallback((*a)[2]),
cvt_vc_p_void((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_room_enter(VCArglist *a)
{
START_WRAP
	dwyco_room_enter(cvt_vc_pq_char((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_DwycoCommandCallback((*a)[2]),
cvt_vc_p_void((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_disconnect_chat_server(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_disconnect_chat_server());
WRAP_END
return ret;

}
static vc
wrap_dwyco_switch_to_chat_server(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_switch_to_chat_server(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_disconnect_server(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_disconnect_server());
WRAP_END
return ret;

}
static vc
wrap_dwyco_switch_to_server(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_switch_to_server(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_server_list(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_server_list(cvt_vc_pp_void((*a)[0]),
cvt_vc_p_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_directory_sort_column(VCArglist *a)
{
START_WRAP
	dwyco_set_directory_sort_column(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_directory_downloading(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_directory_downloading());
WRAP_END
return ret;

}
static vc
wrap_dwyco_directory_online(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_directory_online());
WRAP_END
return ret;

}
static vc
wrap_dwyco_directory_starting(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_directory_starting());
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_directory_downloaded_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_directory_downloaded_callback(cvt_vc_DwycoDirectoryDownloadedCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_zap_reject(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_zap_reject(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_zap_accept(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_zap_accept(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_call_reject(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_call_reject(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_call_accept(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_call_accept(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_call_appearance_death_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_call_appearance_death_callback(cvt_vc_DwycoCallAppearanceDeathCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_zap_appearance_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_zap_appearance_callback(cvt_vc_DwycoZapAppearanceCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_call_acceptance_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_call_acceptance_callback(cvt_vc_DwycoCallAcceptanceCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_call_appearance_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_call_appearance_callback(cvt_vc_DwycoCallAppearanceCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_send_user_control(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_send_user_control(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_cancel_call(VCArglist *a)
{
START_WRAP
	dwyco_cancel_call(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_connect_msg_chan(VCArglist *a)
{
START_WRAP
	dwyco_connect_msg_chan(cvt_vc_pp_char((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_DwycoCallDispositionCallback((*a)[3]),
cvt_vc_p_void((*a)[4]),
cvt_vc_DwycoStatusCallback((*a)[5]),
cvt_vc_p_void((*a)[6]));
WRAP_END
return(vcnil);

}

// note: this function is weird because it can start
// a single call (where a call_id return might be appropriate)
// or it can start a conference call, where the call_id
// isn't really appropriate (since we don't really have a
// notion of "conference id"). anyway, this makes it really
// hard to figure out when an arg_package is no longer needed
// because we don't have a context for the arg (there are multiple
// callbacks that will occur, each of which needs the single
// arg package we are generating here.) yikes, what a mess.
static vc
wrap_dwyco_connect_all2(VCArglist *a)
{
vc ret; // DUMMY
START_WRAP
CTX_INTERPOSE(AP_id, 5, 6)
AP_id++;
CTX_INTERPOSE2(AP_id, 3, 4)
AP_id++;
	dwyco_connect_all2(cvt_vc_pp_char((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_DwycoCallDispositionCallback((*a)[3]),
(void *)ap_id2,
cvt_vc_DwycoStatusCallback((*a)[5]),
(void *)ap_id,
cvt_vc__int((*a)[7]),
cvt_vc__int((*a)[8]),
cvt_vc__int((*a)[9]),
cvt_vc__int((*a)[10]),
cvt_vc__int((*a)[11]),
cvt_vc__int((*a)[12]),
cvt_vc_p_char((*a)[13]),
(const char *)((*a)[14]),
(*a)[14].len()
);
CTX_KILL
CTX_KILL2
WRAP_END
return(vcnil);

}

#if 0
static vc
wrap_dwyco_connect_all(VCArglist *a)
{
::abort();
START_WRAP
	dwyco_connect_all(cvt_vc_pp_char((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_DwycoChannelDestroyCallback((*a)[3]),
cvt_vc_p_void((*a)[4]),
cvt_vc_DwycoCallEstablishedCallback((*a)[5]),
cvt_vc_p_void((*a)[6]),
cvt_vc_DwycoStatusCallback((*a)[7]),
cvt_vc_p_void((*a)[8]),
cvt_vc__int((*a)[9]),
cvt_vc__int((*a)[10]),
cvt_vc__int((*a)[11]),
cvt_vc__int((*a)[12]),
cvt_vc__int((*a)[13]),
cvt_vc__int((*a)[14]),
cvt_vc_p_char((*a)[15]));
WRAP_END
return(vcnil);

}
#endif

static vc
wrap_dwyco_get_squelched(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_squelched());
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_audio_output_in_progress(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_audio_output_in_progress());
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_full_duplex(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_full_duplex());
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_full_duplex(VCArglist *a)
{
START_WRAP
	dwyco_set_full_duplex(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_get_auto_squelch(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_auto_squelch());
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_all_mute(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_all_mute());
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_user_info_sync(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_user_info_sync());
WRAP_END
return ret;

}
static vc
wrap_dwyco_fetch_login_password(VCArglist *a)
{
START_WRAP
	dwyco_fetch_login_password(cvt_vc__int((*a)[0]),
cvt_vc_DwycoCommandCallback((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_query_field(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_query_field(cvt_vc_pq_char((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_DwycoChannelDestroyCallback((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_change_login_password2(VCArglist *a)
{
START_WRAP
	dwyco_change_login_password2(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_pq_char((*a)[4]),
cvt_vc_DwycoCommandCallback((*a)[5]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_change_login_password(VCArglist *a)
{
START_WRAP
	dwyco_change_login_password(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_DwycoCommandCallback((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_old_pal_recv(VCArglist *a)
{
START_WRAP
	dwyco_set_old_pal_recv(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_inhibit_sac(VCArglist *a)
{
START_WRAP
	dwyco_inhibit_sac(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_inhibit_pal(VCArglist *a)
{
START_WRAP
	dwyco_inhibit_pal(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_inhibit_database(VCArglist *a)
{
START_WRAP
	dwyco_inhibit_database(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_database_login(VCArglist *a)
{
START_WRAP
	dwyco_database_login();
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_login_result_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_login_result_callback(cvt_vc_DwycoServerLoginCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_login_password(VCArglist *a)
{
START_WRAP
	dwyco_set_login_password(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_service_channels(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_service_channels(cvt_vc_p_int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_exit(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_exit());
WRAP_END
return ret;

}
static vc
wrap_dwyco_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_init());
WRAP_END
return ret;

}
#ifdef IMW32
static vc
wrap_dwyco_eim_pal_del(VCArglist *a)
{
START_WRAP
	dwyco_eim_pal_del(cvt_vc__int((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_eim_pal_add(VCArglist *a)
{
START_WRAP
	dwyco_eim_pal_add(cvt_vc__int((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pq_char((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_pal_type(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_pal_type(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_eim_set_acct_info(VCArglist *a)
{
START_WRAP
	dwyco_eim_set_acct_info(cvt_vc__int((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_pq_char((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_eim_status(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_eim_status(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_eim_start(VCArglist *a)
{
START_WRAP
	dwyco_eim_start(cvt_vc__int((*a)[0]),
cvt_vc_DwycoEIMServerLoginCallback((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_eim_stop(VCArglist *a)
{
START_WRAP
	dwyco_eim_stop(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
#endif
static vc
wrap_dwyco_is_special_message(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_is_special_message(
	cvt_vc_pq_char((*a)[0]),
	cvt_vc__int((*a)[1]),
	cvt_vc_pq_char((*a)[2]),
	cvt_vc_p_int((*a)[3])
));
WRAP_END
return ret;

}
static vc
wrap_dwyco_list_copy(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(dwyco_list_copy(cvt_vc_p_void((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_list_print(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_list_print(cvt_vc_p_void((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_list_get(VCArglist *a)
{
START_WRAP
	int len_out;
	const char *str_out;
	vc ret = cvt__int(dwyco_list_get(cvt_vc_p_void((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2]),
&str_out,
&len_out,
cvt_vc_p_int((*a)[5])));

if(str_out != 0)
	(*a)[3].local_bind(vc(VC_BSTRING, str_out, len_out));
(*a)[4].local_bind(len_out);
WRAP_END
return ret;

}
static vc
wrap_dwyco_list_numelems(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_list_numelems(cvt_vc_p_void((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc_p_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_list_release(VCArglist *a)
{
START_WRAP
	dwyco_list_release(cvt_vc_p_void((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_estimate_bandwidth(VCArglist *a)
{
START_WRAP
	dwyco_estimate_bandwidth(cvt_vc_p_int((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc_ppq_char((*a)[2]),
cvt_vc_p_int((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_network_diagnostics(VCArglist *a)
{
START_WRAP
	dwyco_network_diagnostics(cvt_vc_ppq_char((*a)[0]),
cvt_vc_p_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_simple_diagnostics(VCArglist *a)
{
START_WRAP
	dwyco_simple_diagnostics(cvt_vc_ppq_char((*a)[0]),
cvt_vc_p_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_auto_authenticate(VCArglist *a)
{
START_WRAP
	dwyco_set_auto_authenticate(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_autosave_config(VCArglist *a)
{
START_WRAP
	dwyco_set_autosave_config(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_get_moron_dork_mode(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_moron_dork_mode());
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_moron_dork_mode(VCArglist *a)
{
START_WRAP
	dwyco_set_moron_dork_mode(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_count_trashed_users(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_count_trashed_users());
WRAP_END
return ret;

}
#ifdef _Windows
static vc
wrap_dwyco_is_capturing_video(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_is_capturing_video());
WRAP_END
return ret;

}
#endif
static vc
wrap_dwyco_empty_trash(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_empty_trash());
WRAP_END
return ret;

}
static vc
wrap_dwyco_delete_user(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_delete_user(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_dnd(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_dnd());
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_dnd(VCArglist *a)
{
START_WRAP
	dwyco_set_dnd(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_uid_to_info(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(dwyco_uid_to_info(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_del_old_timing(VCArglist *a)
{
START_WRAP
	dwyco_del_old_timing(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_add_old_timing(VCArglist *a)
{
START_WRAP
	dwyco_add_old_timing(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_session_ignore_list_get(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(dwyco_session_ignore_list_get());
WRAP_END
return ret;

}
static vc
wrap_dwyco_ignore_list_get(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(dwyco_ignore_list_get());
WRAP_END
return ret;

}
static vc
wrap_dwyco_is_always_visible(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_is_always_visible(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_is_never_visible(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_is_never_visible(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_never_visible(VCArglist *a)
{
START_WRAP
	dwyco_never_visible(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_always_visible(VCArglist *a)
{
START_WRAP
	dwyco_always_visible(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_session_unignore(VCArglist *a)
{
START_WRAP
	dwyco_session_unignore(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_session_ignore(VCArglist *a)
{
START_WRAP
	dwyco_session_ignore(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_unignore(VCArglist *a)
{
START_WRAP
	dwyco_unignore(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_ignore(VCArglist *a)
{
START_WRAP
	dwyco_ignore(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_is_ignored(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_is_ignored(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_handle_pal_auth(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_handle_pal_auth(cvt_vc_pq_char((*a)[0]),
	cvt_vc__int((*a)[1]),
	cvt_vc_pq_char((*a)[2]),
	cvt_vc__int((*a)[3])
));
WRAP_END
return ret;

}
static vc
wrap_dwyco_clear_pal_auths(VCArglist *a)
{
START_WRAP
	dwyco_clear_pal_auths();
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_revoke_pal_auth(VCArglist *a)
{
START_WRAP
	dwyco_revoke_pal_auth(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_pal_auth_granted(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_pal_auth_granted(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_pal_auth_warning(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_pal_auth_warning());
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_my_pal_auth_state(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_my_pal_auth_state());
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_pal_auth_state(VCArglist *a)
{
START_WRAP
	dwyco_set_pal_auth_state(cvt_vc__int((*a)[0]),
cvt_vc_DwycoCommandCallback((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_get_pal_auth_state(VCArglist *a)
{
START_WRAP
	dwyco_get_pal_auth_state(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_DwycoCommandCallback((*a)[2]),
cvt_vc_p_void((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_pal_relogin(VCArglist *a)
{
START_WRAP
	dwyco_pal_relogin();
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_pal_get_list(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(dwyco_pal_get_list());
WRAP_END
return ret;

}
static vc
wrap_dwyco_is_pal(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_is_pal(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_pal_delete(VCArglist *a)
{
START_WRAP
	dwyco_pal_delete(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_pal_add(VCArglist *a)
{
START_WRAP
	dwyco_pal_add(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_update_user_info(VCArglist *a)
{
START_WRAP
	dwyco_update_user_info(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_authenticate_body(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_authenticate_body(cvt_vc_p_void((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_body_array(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(dwyco_get_body_array(cvt_vc_p_void((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_body_text(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(dwyco_get_body_text(cvt_vc_p_void((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_cancel_message_fetch(VCArglist *a)
{
START_WRAP
	dwyco_cancel_message_fetch(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_fetch_server_message(VCArglist *a)
{
START_WRAP
CTX_INTERPOSE(AP_id, 3, 4)
AP_id++;
CTX_INTERPOSE2(AP_id, 1, 2)
AP_id++;
	vc ret = cvt__int(dwyco_fetch_server_message(cvt_vc_pq_char((*a)[0]),
cvt_vc_DwycoMessageDownloadCallback((*a)[1]),
(void *)ap_id2,
cvt_vc_DwycoStatusCallback((*a)[3]),
(void *)ap_id
));
CTX_KILL
CTX_KILL2
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_saved_message(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_saved_message(cvt_vc_pp_void((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pq_char((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_save_message(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_save_message(cvt_vc_pq_char((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_delete_saved_message(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_delete_saved_message(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_delete_unsaved_message(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_delete_unsaved_message(cvt_vc_pq_char((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_unsaved_message_to_body(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_unsaved_message_to_body(cvt_vc_pp_void((*a)[0]),
cvt_vc_pq_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_unsaved_message(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_unsaved_message(cvt_vc_pp_void((*a)[0]),
cvt_vc_pq_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_unsaved_messages(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_unsaved_messages(cvt_vc_pp_void((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_message_bodies(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_message_bodies(cvt_vc_pp_void((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_user_list(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_user_list(cvt_vc_pp_void((*a)[0]),
cvt_vc_p_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_load_users(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_load_users());
WRAP_END
return ret;

}
static vc
wrap_dwyco_uid_g(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_uid_g(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_uid_to_ip(VCArglist *a)
{
START_WRAP
	dwyco_uid_to_ip(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_int((*a)[2]),
cvt_vc_pp_char_out((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_uid_status(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_uid_status(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_uid_online(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_uid_online(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_rescan_messages(VCArglist *a)
{
START_WRAP
	dwyco_set_rescan_messages(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_get_rescan_messages(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_rescan_messages());
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_refresh_users(VCArglist *a)
{
START_WRAP
	dwyco_set_refresh_users(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_get_refresh_users(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_refresh_users());
WRAP_END
return ret;

}
static vc
wrap_dwyco_add_entropy_timer(VCArglist *a)
{
START_WRAP
	dwyco_add_entropy_timer(cvt_vc_p_char((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
// note: had to change the wrapper for this, it is too hard to
// have "in_out" params without a lot of gyrating.
// so i changed it to have 3 args instead of 2
static vc
wrap_dwyco_enable_video_capture_preview(VCArglist *a)
{
START_WRAP
	int ui_id_in_out;
	ui_id_in_out = cvt_vc__int((*a)[1]);
	vc ret = cvt__int(dwyco_enable_video_capture_preview(cvt_vc__int((*a)[0]),
	&ui_id_in_out));
	(*a)[2].local_bind(ui_id_in_out);
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_my_uid(VCArglist *a)
{
#if 0
START_WRAP_misc
	dwyco_get_my_uid(cvt_vc_ppq_char((*a)[0]),
cvt_vc_p_int((*a)[1]));
WRAP_END_misc
return(vcnil);
#endif
	const char *uid;
	int len;
	dwyco_get_my_uid(&uid, &len);
	return vc(VC_BSTRING, uid, len);

}
static vc
wrap_dwyco_set_channel_destroy_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_channel_destroy_callback(cvt_vc__int((*a)[0]),
cvt_vc_DwycoChannelDestroyCallback((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_pause_all_channels(VCArglist *a)
{
START_WRAP
	dwyco_pause_all_channels(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_hangup_all_calls(VCArglist *a)
{
START_WRAP
	dwyco_hangup_all_calls();
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_destroy_by_ui_id(VCArglist *a)
{
START_WRAP
	dwyco_destroy_by_ui_id(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_destroy_channel(VCArglist *a)
{
START_WRAP
	dwyco_destroy_channel(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_is_selective_chat_recipient(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_is_selective_chat_recipient(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_reset_selective_chat_recipients(VCArglist *a)
{
START_WRAP
	dwyco_reset_selective_chat_recipients();
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_selective_chat_enable(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_selective_chat_enable(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_selective_chat_recipient_enable(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_selective_chat_recipient_enable(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_line_from_keyboard(VCArglist *a)
{
START_WRAP
	dwyco_line_from_keyboard(cvt_vc__int((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_command_from_keyboard(VCArglist *a)
{
START_WRAP
	dwyco_command_from_keyboard(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_p_char((*a)[4]),
cvt_vc__int((*a)[5]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_get_profile_to_viewer(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_profile_to_viewer(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_DwycoProfileCallback((*a)[2]),
cvt_vc_p_void((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_profile_from_composer(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_profile_from_composer(cvt_vc__int((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_DwycoProfileCallback((*a)[3]),
cvt_vc_p_void((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_set_profile(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_profile(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_pq_char((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc_pq_char((*a)[6]),
cvt_vc_DwycoProfileCallback((*a)[7]),
cvt_vc_p_void((*a)[8])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_get_profile(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_get_profile(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_DwycoProfileCallback((*a)[2]),
cvt_vc_p_void((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_debug_dump(VCArglist *a)
{
START_WRAP
	dwyco_debug_dump();
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_chat_server_status_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_chat_server_status_callback(cvt_vc_DwycoChatServerStatusCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_chat_ctx_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_chat_ctx_callback(cvt_vc_DwycoChatCtxCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_user_control_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_user_control_callback(cvt_vc_DwycoZapAppearanceCallback((*a)[0]));
WRAP_END
return(vcnil);

}
#if 0
static vc
wrap_dwyco_set_emergency_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_emergency_callback(cvt_vc_DwycoMessageDownloadCallback((*a)[0]));
WRAP_END
return(vcnil);

}
#endif
static vc
wrap_dwyco_set_pal_auth_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_pal_auth_callback(cvt_vc_DwycoPalAuthCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_unregister_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_unregister_callback(cvt_vc_DwycoUnregisterCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_motd_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_motd_callback(cvt_vc_DwycoStatusCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_debug_message_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_debug_message_callback(cvt_vc_DwycoStatusCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_video_display_init_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_video_display_init_callback(cvt_vc_DwycoVideoDisplayInitCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_video_display_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_video_display_callback(cvt_vc_DwycoVideoDisplayCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_public_chat_display_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_public_chat_display_callback(cvt_vc_DwycoPublicChatDisplayCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_private_chat_display_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_private_chat_display_callback(cvt_vc_DwycoPrivateChatDisplayCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_private_chat_init_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_private_chat_init_callback(cvt_vc_DwycoPrivateChatInitCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_public_chat_init_callback(VCArglist *a)
{
START_WRAP
	dwyco_set_public_chat_init_callback(cvt_vc_DwycoPublicChatInitCallback((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dwyco_set_fn_prefixes(VCArglist *a)
{
START_WRAP
	dwyco_set_fn_prefixes(cvt_vc_pq_char((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc_pq_char((*a)[3]));
WRAP_END
return(vcnil);

}

static vc
wrap_dwyco_chat_set_unblock_time(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_chat_set_unblock_time(cvt_vc__int((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_chat_clear_all_demigods(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_chat_clear_all_demigods());
WRAP_END
return ret;

}
static vc
wrap_dwyco_chat_set_demigod(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_chat_set_demigod(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_chat_set_filter(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_chat_set_filter(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pq_char((*a)[3]),
cvt_vc__int((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_chat_mute(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_chat_mute(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_chat_talk(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_chat_talk(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_chat_delq(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_chat_delq(cvt_vc__int((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_chat_addq(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_chat_addq(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}

#if 0
static vc
wrap_dwyco_random_string(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_char(dwyco_random_string(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
#endif

static vc
wrap_dwyco_set_auto_squelch(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_auto_squelch(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}

static vc
wrap_dwyco_set_all_mute(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_set_all_mute(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}

static vc
wrap_dwyco_chan_to_call(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_chan_to_call(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}

static vc
wrap_dwyco_sub_get(VCArglist *a)
{
START_WRAP
	dwyco_sub_get(cvt_vc_ppq_char((*a)[0]),
cvt_vc_p_int((*a)[1]));
WRAP_END
return(vcnil);

}

static vc
wrap_dwyco_is_file_zap(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_is_file_zap(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_copy_out_file_zap(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_copy_out_file_zap(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc_pq_char((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_dwyco_make_file_zap_composition(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dwyco_make_file_zap_composition(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}

static void
makefun(const char *name, const vc& fun)
{
    vc(name).bind(fun);
}
void
wrapper_init_dlli_xml ()
{

Arg_packages = vc(VC_MAP);

makefun("wrap_dwyco_set_video_display_callback", vc(wrap_dwyco_set_video_display_callback, "wrap_dwyco_set_video_display_callback", VC_FUNC_BUILTIN_LEAF));

#ifdef IMW32
makefun("wrap_dwyco_eim_stop", vc(wrap_dwyco_eim_stop, "wrap_dwyco_eim_stop", VC_FUNC_BUILTIN_LEAF));
#endif

makefun("wrap_dwyco_inhibit_sac", vc(wrap_dwyco_inhibit_sac, "wrap_dwyco_inhibit_sac", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_fetch_login_password", vc(wrap_dwyco_fetch_login_password, "wrap_dwyco_fetch_login_password", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_zap_play_preview", vc(wrap_dwyco_zap_play_preview, "wrap_dwyco_zap_play_preview", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_call_accept", vc(wrap_dwyco_set_call_accept, "wrap_dwyco_set_call_accept", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_profile_from_composer", vc(wrap_dwyco_set_profile_from_composer, "wrap_dwyco_set_profile_from_composer", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_authenticate_body", vc(wrap_dwyco_authenticate_body, "wrap_dwyco_authenticate_body", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_pal_get_list", vc(wrap_dwyco_pal_get_list, "wrap_dwyco_pal_get_list", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_unignore", vc(wrap_dwyco_unignore, "wrap_dwyco_unignore", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_list_copy", vc(wrap_dwyco_list_copy, "wrap_dwyco_list_copy", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_config_display", vc(wrap_dwyco_set_config_display, "wrap_dwyco_set_config_display", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_zap_data", vc(wrap_dwyco_set_zap_data, "wrap_dwyco_set_zap_data", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_pal_auth_callback", vc(wrap_dwyco_set_pal_auth_callback, "wrap_dwyco_set_pal_auth_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_reset_selective_chat_recipients", vc(wrap_dwyco_reset_selective_chat_recipients, "wrap_dwyco_reset_selective_chat_recipients", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_destroy_channel", vc(wrap_dwyco_destroy_channel, "wrap_dwyco_destroy_channel", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_all_mute", vc(wrap_dwyco_set_all_mute, "wrap_dwyco_set_all_mute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_zap_play_view", vc(wrap_dwyco_zap_play_view, "wrap_dwyco_zap_play_view", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_video_input", vc(wrap_dwyco_set_video_input, "wrap_dwyco_set_video_input", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_profile", vc(wrap_dwyco_get_profile, "wrap_dwyco_get_profile", VC_FUNC_BUILTIN_LEAF));

#ifdef _Windows
makefun("wrap_dwyco_is_capturing_video", vc(wrap_dwyco_is_capturing_video, "wrap_dwyco_is_capturing_video", VC_FUNC_BUILTIN_LEAF));

#endif
makefun("wrap_dwyco_directory_online", vc(wrap_dwyco_directory_online, "wrap_dwyco_directory_online", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_abort_autoupdate_download", vc(wrap_dwyco_abort_autoupdate_download, "wrap_dwyco_abort_autoupdate_download", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_raw_files", vc(wrap_dwyco_get_raw_files, "wrap_dwyco_get_raw_files", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_enable_video_capture_preview", vc(wrap_dwyco_enable_video_capture_preview, "wrap_dwyco_enable_video_capture_preview", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_uid_to_ip", vc(wrap_dwyco_uid_to_ip, "wrap_dwyco_uid_to_ip", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_save_message", vc(wrap_dwyco_save_message, "wrap_dwyco_save_message", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_fetch_server_message", vc(wrap_dwyco_fetch_server_message, "wrap_dwyco_fetch_server_message", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_del_old_timing", vc(wrap_dwyco_del_old_timing, "wrap_dwyco_del_old_timing", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_auto_squelch", vc(wrap_dwyco_get_auto_squelch, "wrap_dwyco_get_auto_squelch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_start_autoupdate_download", vc(wrap_dwyco_start_autoupdate_download, "wrap_dwyco_start_autoupdate_download", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_cancel_message_fetch", vc(wrap_dwyco_cancel_message_fetch, "wrap_dwyco_cancel_message_fetch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_pal_delete", vc(wrap_dwyco_pal_delete, "wrap_dwyco_pal_delete", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_delete_user", vc(wrap_dwyco_delete_user, "wrap_dwyco_delete_user", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_query_field", vc(wrap_dwyco_query_field, "wrap_dwyco_query_field", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_cmd_path", vc(wrap_dwyco_set_cmd_path, "wrap_dwyco_set_cmd_path", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_rating", vc(wrap_dwyco_get_rating, "wrap_dwyco_get_rating", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_vidcap_data", vc(wrap_dwyco_get_vidcap_data, "wrap_dwyco_get_vidcap_data", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_dwyco_set_emergency_callback", vc(wrap_dwyco_set_emergency_callback, "wrap_dwyco_set_emergency_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_profile_to_viewer", vc(wrap_dwyco_get_profile_to_viewer, "wrap_dwyco_get_profile_to_viewer", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_ignore_list_get", vc(wrap_dwyco_ignore_list_get, "wrap_dwyco_ignore_list_get", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_squelched", vc(wrap_dwyco_get_squelched, "wrap_dwyco_get_squelched", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_delete_zap_composition", vc(wrap_dwyco_delete_zap_composition, "wrap_dwyco_delete_zap_composition", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_private_chat_init_callback", vc(wrap_dwyco_set_private_chat_init_callback, "wrap_dwyco_set_private_chat_init_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_unsaved_message", vc(wrap_dwyco_get_unsaved_message, "wrap_dwyco_get_unsaved_message", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_update_user_info", vc(wrap_dwyco_update_user_info, "wrap_dwyco_update_user_info", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_directory_downloading", vc(wrap_dwyco_directory_downloading, "wrap_dwyco_directory_downloading", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_zap_record", vc(wrap_dwyco_zap_record, "wrap_dwyco_zap_record", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_hangup_all_calls", vc(wrap_dwyco_hangup_all_calls, "wrap_dwyco_hangup_all_calls", VC_FUNC_BUILTIN_LEAF));

#ifdef IMW32
makefun("wrap_dwyco_eim_pal_add", vc(wrap_dwyco_eim_pal_add, "wrap_dwyco_eim_pal_add", VC_FUNC_BUILTIN_LEAF));

#endif
makefun("wrap_dwyco_set_old_pal_recv", vc(wrap_dwyco_set_old_pal_recv, "wrap_dwyco_set_old_pal_recv", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_connect_msg_chan", vc(wrap_dwyco_connect_msg_chan, "wrap_dwyco_connect_msg_chan", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_directory_starting", vc(wrap_dwyco_directory_starting, "wrap_dwyco_directory_starting", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_switch_to_server", vc(wrap_dwyco_switch_to_server, "wrap_dwyco_switch_to_server", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_video_display_init_callback", vc(wrap_dwyco_set_video_display_init_callback, "wrap_dwyco_set_video_display_init_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_refresh_users", vc(wrap_dwyco_set_refresh_users, "wrap_dwyco_set_refresh_users", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_uid_online", vc(wrap_dwyco_uid_online, "wrap_dwyco_uid_online", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_is_special_message", vc(wrap_dwyco_is_special_message, "wrap_dwyco_is_special_message", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_zap_record2", vc(wrap_dwyco_zap_record2, "wrap_dwyco_zap_record2", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_regcode", vc(wrap_dwyco_set_regcode, "wrap_dwyco_set_regcode", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_dwyco_set_conn_remote_data", vc(wrap_dwyco_set_conn_remote_data, "wrap_dwyco_set_conn_remote_data", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_uid_g", vc(wrap_dwyco_uid_g, "wrap_dwyco_uid_g", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_login_result_callback", vc(wrap_dwyco_set_login_result_callback, "wrap_dwyco_set_login_result_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_user_info_sync", vc(wrap_dwyco_get_user_info_sync, "wrap_dwyco_get_user_info_sync", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_all_mute", vc(wrap_dwyco_get_all_mute, "wrap_dwyco_get_all_mute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_call_reject", vc(wrap_dwyco_call_reject, "wrap_dwyco_call_reject", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_server_list", vc(wrap_dwyco_get_server_list, "wrap_dwyco_get_server_list", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_room_create", vc(wrap_dwyco_room_create, "wrap_dwyco_room_create", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_flim", vc(wrap_dwyco_flim, "wrap_dwyco_flim", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_zap_send", vc(wrap_dwyco_zap_send, "wrap_dwyco_zap_send", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_make_zap_view", vc(wrap_dwyco_make_zap_view, "wrap_dwyco_make_zap_view", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_force_autoupdate_check", vc(wrap_dwyco_force_autoupdate_check, "wrap_dwyco_force_autoupdate_check", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_rate_tweaks", vc(wrap_dwyco_set_rate_tweaks, "wrap_dwyco_set_rate_tweaks", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_handle_pal_auth", vc(wrap_dwyco_handle_pal_auth, "wrap_dwyco_handle_pal_auth", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_service_channels", vc(wrap_dwyco_service_channels, "wrap_dwyco_service_channels", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_database_login", vc(wrap_dwyco_database_login, "wrap_dwyco_database_login", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_zap_stop", vc(wrap_dwyco_zap_stop, "wrap_dwyco_zap_stop", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_make_zap_view_file", vc(wrap_dwyco_make_zap_view_file, "wrap_dwyco_make_zap_view_file", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_external_video_capture_callbacks", vc(wrap_dwyco_set_external_video_capture_callbacks, "wrap_dwyco_set_external_video_capture_callbacks", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_net_data", vc(wrap_dwyco_set_net_data, "wrap_dwyco_set_net_data", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_rescan_messages", vc(wrap_dwyco_set_rescan_messages, "wrap_dwyco_set_rescan_messages", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_delete_unsaved_message", vc(wrap_dwyco_delete_unsaved_message, "wrap_dwyco_delete_unsaved_message", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_list_release", vc(wrap_dwyco_list_release, "wrap_dwyco_list_release", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_full_duplex", vc(wrap_dwyco_set_full_duplex, "wrap_dwyco_set_full_duplex", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_zap_send2", vc(wrap_dwyco_zap_send2, "wrap_dwyco_zap_send2", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_call_accept", vc(wrap_dwyco_get_call_accept, "wrap_dwyco_get_call_accept", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_private_chat_display_callback", vc(wrap_dwyco_set_private_chat_display_callback, "wrap_dwyco_set_private_chat_display_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_chat_ctx_callback", vc(wrap_dwyco_set_chat_ctx_callback, "wrap_dwyco_set_chat_ctx_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_uid_status", vc(wrap_dwyco_uid_status, "wrap_dwyco_uid_status", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_saved_message", vc(wrap_dwyco_get_saved_message, "wrap_dwyco_get_saved_message", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_body_array", vc(wrap_dwyco_get_body_array, "wrap_dwyco_get_body_array", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_my_pal_auth_state", vc(wrap_dwyco_get_my_pal_auth_state, "wrap_dwyco_get_my_pal_auth_state", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_clear_pal_auths", vc(wrap_dwyco_clear_pal_auths, "wrap_dwyco_clear_pal_auths", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_config_display", vc(wrap_dwyco_get_config_display, "wrap_dwyco_get_config_display", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_zap_data", vc(wrap_dwyco_get_zap_data, "wrap_dwyco_get_zap_data", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_chat_server_status_callback", vc(wrap_dwyco_set_chat_server_status_callback, "wrap_dwyco_set_chat_server_status_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_debug_dump", vc(wrap_dwyco_debug_dump, "wrap_dwyco_debug_dump", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_selective_chat_recipient_enable", vc(wrap_dwyco_selective_chat_recipient_enable, "wrap_dwyco_selective_chat_recipient_enable", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_delete_saved_message", vc(wrap_dwyco_delete_saved_message, "wrap_dwyco_delete_saved_message", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_pal_auth_granted", vc(wrap_dwyco_pal_auth_granted, "wrap_dwyco_pal_auth_granted", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_session_unignore", vc(wrap_dwyco_session_unignore, "wrap_dwyco_session_unignore", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_dnd", vc(wrap_dwyco_set_dnd, "wrap_dwyco_set_dnd", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_call_acceptance_callback", vc(wrap_dwyco_set_call_acceptance_callback, "wrap_dwyco_set_call_acceptance_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_video_input", vc(wrap_dwyco_get_video_input, "wrap_dwyco_get_video_input", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_unregister_callback", vc(wrap_dwyco_set_unregister_callback, "wrap_dwyco_set_unregister_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_selective_chat_enable", vc(wrap_dwyco_selective_chat_enable, "wrap_dwyco_selective_chat_enable", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_moron_dork_mode", vc(wrap_dwyco_set_moron_dork_mode, "wrap_dwyco_set_moron_dork_mode", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_zap_accept", vc(wrap_dwyco_zap_accept, "wrap_dwyco_zap_accept", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_directory_sort_column", vc(wrap_dwyco_set_directory_sort_column, "wrap_dwyco_set_directory_sort_column", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_invisible_state", vc(wrap_dwyco_set_invisible_state, "wrap_dwyco_set_invisible_state", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_is_selective_chat_recipient", vc(wrap_dwyco_is_selective_chat_recipient, "wrap_dwyco_is_selective_chat_recipient", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_pal_auth_state", vc(wrap_dwyco_set_pal_auth_state, "wrap_dwyco_set_pal_auth_state", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_never_visible", vc(wrap_dwyco_never_visible, "wrap_dwyco_never_visible", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_auto_authenticate", vc(wrap_dwyco_set_auto_authenticate, "wrap_dwyco_set_auto_authenticate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_final_setup", vc(wrap_dwyco_final_setup, "wrap_dwyco_final_setup", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_fn_prefixes", vc(wrap_dwyco_set_fn_prefixes, "wrap_dwyco_set_fn_prefixes", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_pause_all_channels", vc(wrap_dwyco_pause_all_channels, "wrap_dwyco_pause_all_channels", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_refresh_users", vc(wrap_dwyco_get_refresh_users, "wrap_dwyco_get_refresh_users", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_message_bodies", vc(wrap_dwyco_get_message_bodies, "wrap_dwyco_get_message_bodies", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_empty_trash", vc(wrap_dwyco_empty_trash, "wrap_dwyco_empty_trash", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_inhibit_database", vc(wrap_dwyco_inhibit_database, "wrap_dwyco_inhibit_database", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_inhibit_pal", vc(wrap_dwyco_inhibit_pal, "wrap_dwyco_inhibit_pal", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_make_special_zap_composition", vc(wrap_dwyco_make_special_zap_composition, "wrap_dwyco_make_special_zap_composition", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_external_audio_output_callbacks", vc(wrap_dwyco_set_external_audio_output_callbacks, "wrap_dwyco_set_external_audio_output_callbacks", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_channel_destroy_callback", vc(wrap_dwyco_set_channel_destroy_callback, "wrap_dwyco_set_channel_destroy_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_load_users", vc(wrap_dwyco_load_users, "wrap_dwyco_load_users", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_is_ignored", vc(wrap_dwyco_is_ignored, "wrap_dwyco_is_ignored", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_add_old_timing", vc(wrap_dwyco_add_old_timing, "wrap_dwyco_add_old_timing", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_estimate_bandwidth", vc(wrap_dwyco_estimate_bandwidth, "wrap_dwyco_estimate_bandwidth", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_setup_autoupdate", vc(wrap_dwyco_setup_autoupdate, "wrap_dwyco_setup_autoupdate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_motd_callback", vc(wrap_dwyco_set_motd_callback, "wrap_dwyco_set_motd_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_user_control_callback", vc(wrap_dwyco_set_user_control_callback, "wrap_dwyco_set_user_control_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_session_ignore_list_get", vc(wrap_dwyco_session_ignore_list_get, "wrap_dwyco_session_ignore_list_get", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_user_data", vc(wrap_dwyco_set_user_data, "wrap_dwyco_set_user_data", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_user_list", vc(wrap_dwyco_get_user_list, "wrap_dwyco_get_user_list", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_unsaved_messages", vc(wrap_dwyco_get_unsaved_messages, "wrap_dwyco_get_unsaved_messages", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_ignore", vc(wrap_dwyco_ignore, "wrap_dwyco_ignore", VC_FUNC_BUILTIN_LEAF));

#ifdef IMW32
makefun("wrap_dwyco_eim_set_acct_info", vc(wrap_dwyco_eim_set_acct_info, "wrap_dwyco_eim_set_acct_info", VC_FUNC_BUILTIN_LEAF));

#endif
makefun("wrap_dwyco_get_full_duplex", vc(wrap_dwyco_get_full_duplex, "wrap_dwyco_get_full_duplex", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_dwyco_connect_all", vc(wrap_dwyco_connect_all, "wrap_dwyco_connect_all", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_make_forward_zap_composition", vc(wrap_dwyco_make_forward_zap_composition, "wrap_dwyco_make_forward_zap_composition", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_zap_still_active", vc(wrap_dwyco_zap_still_active, "wrap_dwyco_zap_still_active", VC_FUNC_BUILTIN_LEAF));

#ifdef IMW32
makefun("wrap_dwyco_eim_status", vc(wrap_dwyco_eim_status, "wrap_dwyco_eim_status", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_eim_pal_del", vc(wrap_dwyco_eim_pal_del, "wrap_dwyco_eim_pal_del", VC_FUNC_BUILTIN_LEAF));
#endif

makefun("wrap_dwyco_delete_zap_view", vc(wrap_dwyco_delete_zap_view, "wrap_dwyco_delete_zap_view", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_zap_quick_stats_view", vc(wrap_dwyco_zap_quick_stats_view, "wrap_dwyco_zap_quick_stats_view", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_dwyco_get_conn_remote_data", vc(wrap_dwyco_get_conn_remote_data, "wrap_dwyco_get_conn_remote_data", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_profile", vc(wrap_dwyco_set_profile, "wrap_dwyco_set_profile", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_body_text", vc(wrap_dwyco_get_body_text, "wrap_dwyco_get_body_text", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_network_diagnostics", vc(wrap_dwyco_network_diagnostics, "wrap_dwyco_network_diagnostics", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_login_password", vc(wrap_dwyco_set_login_password, "wrap_dwyco_set_login_password", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_change_login_password", vc(wrap_dwyco_change_login_password, "wrap_dwyco_change_login_password", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_connect_all2", vc(wrap_dwyco_connect_all2, "wrap_dwyco_connect_all2", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_call_appearance_death_callback", vc(wrap_dwyco_set_call_appearance_death_callback, "wrap_dwyco_set_call_appearance_death_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_rate_tweaks", vc(wrap_dwyco_get_rate_tweaks, "wrap_dwyco_get_rate_tweaks", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_my_uid", vc(wrap_dwyco_get_my_uid, "wrap_dwyco_get_my_uid", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_is_pal", vc(wrap_dwyco_is_pal, "wrap_dwyco_is_pal", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_change_login_password2", vc(wrap_dwyco_change_login_password2, "wrap_dwyco_change_login_password2", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_cancel_call", vc(wrap_dwyco_cancel_call, "wrap_dwyco_cancel_call", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_call_appearance_callback", vc(wrap_dwyco_set_call_appearance_callback, "wrap_dwyco_set_call_appearance_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_zap_appearance_callback", vc(wrap_dwyco_set_zap_appearance_callback, "wrap_dwyco_set_zap_appearance_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_room_enter", vc(wrap_dwyco_room_enter, "wrap_dwyco_room_enter", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_expired", vc(wrap_dwyco_get_expired, "wrap_dwyco_get_expired", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_net_data", vc(wrap_dwyco_get_net_data, "wrap_dwyco_get_net_data", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_debug_message_callback", vc(wrap_dwyco_set_debug_message_callback, "wrap_dwyco_set_debug_message_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_rescan_messages", vc(wrap_dwyco_get_rescan_messages, "wrap_dwyco_get_rescan_messages", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_pal_add", vc(wrap_dwyco_pal_add, "wrap_dwyco_pal_add", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_pal_auth_state", vc(wrap_dwyco_get_pal_auth_state, "wrap_dwyco_get_pal_auth_state", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_list_print", vc(wrap_dwyco_list_print, "wrap_dwyco_list_print", VC_FUNC_BUILTIN_LEAF));

#ifdef IMW32
makefun("wrap_dwyco_eim_start", vc(wrap_dwyco_eim_start, "wrap_dwyco_eim_start", VC_FUNC_BUILTIN_LEAF));

#endif
makefun("wrap_dwyco_exit", vc(wrap_dwyco_exit, "wrap_dwyco_exit", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_send_user_control", vc(wrap_dwyco_send_user_control, "wrap_dwyco_send_user_control", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_vidcap_data", vc(wrap_dwyco_set_vidcap_data, "wrap_dwyco_set_vidcap_data", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_unsaved_message_to_body", vc(wrap_dwyco_unsaved_message_to_body, "wrap_dwyco_unsaved_message_to_body", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_pal_auth_warning", vc(wrap_dwyco_get_pal_auth_warning, "wrap_dwyco_get_pal_auth_warning", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_list_numelems", vc(wrap_dwyco_list_numelems, "wrap_dwyco_list_numelems", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_is_registered", vc(wrap_dwyco_is_registered, "wrap_dwyco_is_registered", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_public_chat_display_callback", vc(wrap_dwyco_set_public_chat_display_callback, "wrap_dwyco_set_public_chat_display_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_dnd", vc(wrap_dwyco_get_dnd, "wrap_dwyco_get_dnd", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_audio_output_in_progress", vc(wrap_dwyco_get_audio_output_in_progress, "wrap_dwyco_get_audio_output_in_progress", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_call_accept", vc(wrap_dwyco_call_accept, "wrap_dwyco_call_accept", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_switch_to_chat_server", vc(wrap_dwyco_switch_to_chat_server, "wrap_dwyco_switch_to_chat_server", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_refresh_directory", vc(wrap_dwyco_refresh_directory, "wrap_dwyco_refresh_directory", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_raw_files", vc(wrap_dwyco_set_raw_files, "wrap_dwyco_set_raw_files", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_moron_dork_mode", vc(wrap_dwyco_get_moron_dork_mode, "wrap_dwyco_get_moron_dork_mode", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_auto_squelch", vc(wrap_dwyco_set_auto_squelch, "wrap_dwyco_set_auto_squelch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_invisible_state", vc(wrap_dwyco_get_invisible_state, "wrap_dwyco_get_invisible_state", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_make_zap_composition", vc(wrap_dwyco_make_zap_composition, "wrap_dwyco_make_zap_composition", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_zap_stop_view", vc(wrap_dwyco_zap_stop_view, "wrap_dwyco_zap_stop_view", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_get_user_data", vc(wrap_dwyco_get_user_data, "wrap_dwyco_get_user_data", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_revoke_pal_auth", vc(wrap_dwyco_revoke_pal_auth, "wrap_dwyco_revoke_pal_auth", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_is_never_visible", vc(wrap_dwyco_is_never_visible, "wrap_dwyco_is_never_visible", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_autosave_config", vc(wrap_dwyco_set_autosave_config, "wrap_dwyco_set_autosave_config", VC_FUNC_BUILTIN_LEAF));

#ifdef IMW32
makefun("wrap_dwyco_pal_type", vc(wrap_dwyco_pal_type, "wrap_dwyco_pal_type", VC_FUNC_BUILTIN_LEAF));
#endif

makefun("wrap_dwyco_zap_reject", vc(wrap_dwyco_zap_reject, "wrap_dwyco_zap_reject", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_directory_downloaded_callback", vc(wrap_dwyco_set_directory_downloaded_callback, "wrap_dwyco_set_directory_downloaded_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_disconnect_chat_server", vc(wrap_dwyco_disconnect_chat_server, "wrap_dwyco_disconnect_chat_server", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_room_delete", vc(wrap_dwyco_room_delete, "wrap_dwyco_room_delete", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_rating", vc(wrap_dwyco_set_rating, "wrap_dwyco_set_rating", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_public_chat_init_callback", vc(wrap_dwyco_set_public_chat_init_callback, "wrap_dwyco_set_public_chat_init_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_command_from_keyboard", vc(wrap_dwyco_command_from_keyboard, "wrap_dwyco_command_from_keyboard", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_line_from_keyboard", vc(wrap_dwyco_line_from_keyboard, "wrap_dwyco_line_from_keyboard", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_session_ignore", vc(wrap_dwyco_session_ignore, "wrap_dwyco_session_ignore", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_always_visible", vc(wrap_dwyco_always_visible, "wrap_dwyco_always_visible", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_is_always_visible", vc(wrap_dwyco_is_always_visible, "wrap_dwyco_is_always_visible", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_uid_to_info", vc(wrap_dwyco_uid_to_info, "wrap_dwyco_uid_to_info", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_simple_diagnostics", vc(wrap_dwyco_simple_diagnostics, "wrap_dwyco_simple_diagnostics", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_init", vc(wrap_dwyco_init, "wrap_dwyco_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_is_forward_composition", vc(wrap_dwyco_is_forward_composition, "wrap_dwyco_is_forward_composition", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_zap_play", vc(wrap_dwyco_zap_play, "wrap_dwyco_zap_play", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_zap_send_cancel", vc(wrap_dwyco_zap_send_cancel, "wrap_dwyco_zap_send_cancel", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_external_audio_capture_callbacks", vc(wrap_dwyco_set_external_audio_capture_callbacks, "wrap_dwyco_set_external_audio_capture_callbacks", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_destroy_by_ui_id", vc(wrap_dwyco_destroy_by_ui_id, "wrap_dwyco_destroy_by_ui_id", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_add_entropy_timer", vc(wrap_dwyco_add_entropy_timer, "wrap_dwyco_add_entropy_timer", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_pal_relogin", vc(wrap_dwyco_pal_relogin, "wrap_dwyco_pal_relogin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_count_trashed_users", vc(wrap_dwyco_count_trashed_users, "wrap_dwyco_count_trashed_users", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_list_get", vc(wrap_dwyco_list_get, "wrap_dwyco_list_get", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_disconnect_server", vc(wrap_dwyco_disconnect_server, "wrap_dwyco_disconnect_server", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_set_autoupdate_status_callback", vc(wrap_dwyco_set_autoupdate_status_callback, "wrap_dwyco_set_autoupdate_status_callback", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dwyco_chan_to_call", vc(wrap_dwyco_chan_to_call, "wrap_dwyco_chan_to_call", VC_FUNC_BUILTIN_LEAF));
makefun("wrap_dwyco_chat_addq", vc(wrap_dwyco_chat_addq, "wrap_dwyco_chat_addq", VC_FUNC_BUILTIN_LEAF));
makefun("wrap_dwyco_chat_clear_all_demigods", vc(wrap_dwyco_chat_clear_all_demigods, "wrap_dwyco_chat_clear_all_demigods", VC_FUNC_BUILTIN_LEAF));
makefun("wrap_dwyco_chat_delq", vc(wrap_dwyco_chat_delq, "wrap_dwyco_chat_delq", VC_FUNC_BUILTIN_LEAF));
makefun("wrap_dwyco_chat_mute", vc(wrap_dwyco_chat_mute, "wrap_dwyco_chat_mute", VC_FUNC_BUILTIN_LEAF));
makefun("wrap_dwyco_chat_set_demigod", vc(wrap_dwyco_chat_set_demigod, "wrap_dwyco_chat_set_demigod", VC_FUNC_BUILTIN_LEAF));
makefun("wrap_dwyco_chat_set_filter", vc(wrap_dwyco_chat_set_filter, "wrap_dwyco_chat_set_filter", VC_FUNC_BUILTIN_LEAF));
makefun("wrap_dwyco_chat_set_unblock_time", vc(wrap_dwyco_chat_set_unblock_time, "wrap_dwyco_chat_set_unblock_time", VC_FUNC_BUILTIN_LEAF));
makefun("wrap_dwyco_chat_talk", vc(wrap_dwyco_chat_talk, "wrap_dwyco_chat_talk", VC_FUNC_BUILTIN_LEAF));
makefun("wrap_dwyco_copy_out_file_zap", vc(wrap_dwyco_copy_out_file_zap, "wrap_dwyco_copy_out_file_zap", VC_FUNC_BUILTIN_LEAF));
makefun("wrap_dwyco_is_file_zap", vc(wrap_dwyco_is_file_zap, "wrap_dwyco_is_file_zap", VC_FUNC_BUILTIN_LEAF));
makefun("wrap_dwyco_make_file_zap_composition", vc(wrap_dwyco_make_file_zap_composition, "wrap_dwyco_make_file_zap_composition", VC_FUNC_BUILTIN_LEAF));
//makefun("wrap_dwyco_random_string", vc(wrap_dwyco_random_string, "wrap_dwyco_random_string", VC_FUNC_BUILTIN_LEAF));
makefun("wrap_dwyco_sub_get", vc(wrap_dwyco_sub_get, "wrap_dwyco_sub_get", VC_FUNC_BUILTIN_LEAF));
}
