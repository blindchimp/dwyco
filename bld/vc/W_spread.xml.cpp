
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "vc.h"
#include "W_cvt_spread.xml.cpp"
	static vc
wrap_SP_error(VCArglist *a)
{
START_WRAP
	SP_error(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_SP_equal_group_ids(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_equal_group_ids(cvt_vc__dummy_group_id_((*a)[0]),
cvt_vc__dummy_group_id_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_SP_poll(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_poll(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_SP_scat_get_vs_set_members(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_scat_get_vs_set_members(cvt_vc_pq_dummy_scatter_((*a)[0]),
cvt_vc_pq_dummy_vs_set_info_((*a)[1]),
cvt_vc_p_a_31__char((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_SP_scat_get_vs_sets_info(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_scat_get_vs_sets_info(cvt_vc_pq_dummy_scatter_((*a)[0]),
cvt_vc_p_dummy_vs_set_info_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_unsigned_int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_SP_scat_get_memb_info(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_scat_get_memb_info(cvt_vc_pq_dummy_scatter_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_dummy_membership_info_((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_SP_get_vs_set_members(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_get_vs_set_members(cvt_vc_pq_char((*a)[0]),
cvt_vc_pq_dummy_vs_set_info_((*a)[1]),
cvt_vc_p_a_31__char((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_SP_get_vs_sets_info(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_get_vs_sets_info(cvt_vc_pq_char((*a)[0]),
cvt_vc_p_dummy_vs_set_info_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_unsigned_int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_SP_get_memb_info(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_get_memb_info(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_dummy_membership_info_((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_SP_scat_receive(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_scat_receive(cvt_vc__int((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc_p_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_p_int((*a)[4]),
cvt_vc_p_a_31__char((*a)[5]),
cvt_vc_p_short_int((*a)[6]),
cvt_vc_p_int((*a)[7]),
cvt_vc_p_dummy_scatter_((*a)[8])));
WRAP_END
return ret;

}
static vc
wrap_SP_receive(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_receive(cvt_vc__int((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc_p_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_p_int((*a)[4]),
cvt_vc_p_a_31__char((*a)[5]),
cvt_vc_p_short_int((*a)[6]),
cvt_vc_p_int((*a)[7]),
cvt_vc__int((*a)[8]),
cvt_vc_p_char((*a)[9])));
WRAP_END
return ret;

}
static vc
wrap_SP_multigroup_scat_multicast(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_multigroup_scat_multicast(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pq_a_31__char((*a)[3]),
cvt_vc__short_int((*a)[4]),
cvt_vc_pq_dummy_scatter_((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_SP_multigroup_multicast(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_multigroup_multicast(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pq_a_31__char((*a)[3]),
cvt_vc__short_int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc_pq_char((*a)[6])));
WRAP_END
return ret;

}
static vc
wrap_SP_scat_multicast(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_scat_multicast(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__short_int((*a)[3]),
cvt_vc_pq_dummy_scatter_((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_SP_multicast(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_multicast(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__short_int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc_pq_char((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_SP_leave(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_leave(cvt_vc__int((*a)[0]),
cvt_vc_pq_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_SP_join(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_join(cvt_vc__int((*a)[0]),
cvt_vc_pq_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_SP_kill(VCArglist *a)
{
START_WRAP
	SP_kill(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_SP_disconnect(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_disconnect(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_SP_connect_timeout(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_connect_timeout(cvt_vc_pq_char((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_p_int((*a)[4]),
cvt_vc_p_char((*a)[5]),
cvt_vc__dummy_time_((*a)[6])));
WRAP_END
return ret;

}
static vc
wrap_SP_connect(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_connect(cvt_vc_pq_char((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_p_int((*a)[4]),
cvt_vc_p_char((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_SP_set_auth_methods(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_set_auth_methods(cvt_vc__int((*a)[0]),
cvt_vc_ppq_char((*a)[1]),
cvt_vc_ppF__int__int_x_p_void((*a)[2]),
cvt_vc_pp_void((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_SP_set_auth_method(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_set_auth_method(cvt_vc_pq_char((*a)[0]),
cvt_vc_pF__int__int_x_p_void((*a)[1]),
cvt_vc_p_void((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_SP_version(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_version(cvt_vc_p_int((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc_p_int((*a)[2])));
WRAP_END
return ret;

}
ignoring function E_exit_events 
ignoring function E_handle_events 
ignoring function E_num_active 
ignoring function E_deactivate_fd 
ignoring function E_activate_fd 
ignoring function E_set_active_threshold 
ignoring function E_detach_fd 
ignoring function E_attach_fd 
ignoring function E_delay 
ignoring function E_dequeue 
ignoring function E_queue 
ignoring function E_compare_time 
ignoring function E_add_time 
ignoring function E_sub_time 
ignoring function E_get_time 
ignoring function E_init 
ignoring function __builtin_expect 
ignoring function __builtin_prefetch 
ignoring function __builtin_return 
ignoring function __builtin_return_address 
ignoring function __builtin_frame_address 
ignoring function __builtin_nansl 
ignoring function __builtin_nansf 
ignoring function __builtin_nans 
ignoring function __builtin_infl 
ignoring function __builtin_inff 
ignoring function __builtin_inf 
ignoring dummy_time 
ignoring dummy_time 
ignoring dummy_membership_info 
ignoring dummy_membership_info 
ignoring dummy_vs_set_info 
ignoring dummy_vs_set_info 
ignoring dummy_group_id 
ignoring dummy_group_id 
ignoring dummy_scatter 
ignoring dummy_scatter 
ignoring dummy_scat_element 
ignoring dummy_scat_element 

static void
makefun(const char *name, const vc& fun)
{
    vc(name).bind(fun);
}
void
wrapper_init_spread_xml ()
{

makefun("wrap_SP_multicast", vc(wrap_SP_multicast, "wrap_SP_multicast", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_scat_multicast", vc(wrap_SP_scat_multicast, "wrap_SP_scat_multicast", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_scat_get_vs_set_members", vc(wrap_SP_scat_get_vs_set_members, "wrap_SP_scat_get_vs_set_members", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_error", vc(wrap_SP_error, "wrap_SP_error", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_poll", vc(wrap_SP_poll, "wrap_SP_poll", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_scat_receive", vc(wrap_SP_scat_receive, "wrap_SP_scat_receive", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_get_vs_set_members", vc(wrap_SP_get_vs_set_members, "wrap_SP_get_vs_set_members", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_multigroup_multicast", vc(wrap_SP_multigroup_multicast, "wrap_SP_multigroup_multicast", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_version", vc(wrap_SP_version, "wrap_SP_version", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_connect", vc(wrap_SP_connect, "wrap_SP_connect", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_multigroup_scat_multicast", vc(wrap_SP_multigroup_scat_multicast, "wrap_SP_multigroup_scat_multicast", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_receive", vc(wrap_SP_receive, "wrap_SP_receive", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_connect_timeout", vc(wrap_SP_connect_timeout, "wrap_SP_connect_timeout", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_set_auth_method", vc(wrap_SP_set_auth_method, "wrap_SP_set_auth_method", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_scat_get_vs_sets_info", vc(wrap_SP_scat_get_vs_sets_info, "wrap_SP_scat_get_vs_sets_info", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_set_auth_methods", vc(wrap_SP_set_auth_methods, "wrap_SP_set_auth_methods", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_join", vc(wrap_SP_join, "wrap_SP_join", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_get_memb_info", vc(wrap_SP_get_memb_info, "wrap_SP_get_memb_info", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_equal_group_ids", vc(wrap_SP_equal_group_ids, "wrap_SP_equal_group_ids", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_kill", vc(wrap_SP_kill, "wrap_SP_kill", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_disconnect", vc(wrap_SP_disconnect, "wrap_SP_disconnect", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_leave", vc(wrap_SP_leave, "wrap_SP_leave", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_scat_get_memb_info", vc(wrap_SP_scat_get_memb_info, "wrap_SP_scat_get_memb_info", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_get_vs_sets_info", vc(wrap_SP_get_vs_sets_info, "wrap_SP_get_vs_sets_info", VC_FUNC_BUILTIN_LEAF));
}
