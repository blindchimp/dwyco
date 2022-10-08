
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "vc.h"
#include "vccomp.h"
#include "sp.h"
#include "dwvecp.h"
#include "vcxstrm.h"

static DwVecP<void> Arg_stk;
static DwVec<vc> *outbind_bool;
static DwVec<vc> *outbind_int;
static DwVec<vc> *outbind_pvoid;
static DwVec<vc> *outbind_long;
static DwVec<vc> *outbind_str;
static DwVec<vc> *outbind_double;
typedef DwVec<vc> vvc;

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

#include "hacked_cvt_spread.xml.cpp"
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
#if 0
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
#endif
#if 0
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
#endif
#if 0
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
#endif
#if 0
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
#endif
#if 0
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
#endif
static vc
wrap_SP_receive(VCArglist *a)
{
// this function is too whacky for auto-wrapping, so
// this just wraps a subset of the functionality.
// note: there is no good solution for non-blocking use
// of this library right now.
	int mbox = (*a)[0];
	service srv = 0;
	char msg[128 * 1024];
	int msg_len;

	char sender[MAX_GROUP_NAME + 1];
	sender[sizeof(sender) - 1] = 0;
#define MAX_GROUPS 256
	char groups[MAX_GROUPS][MAX_GROUP_NAME];
	int num_groups;
	short msg_type;
	int endian_mismatch;

	int sz;

	sz = SP_receive(mbox, &srv, sender, MAX_GROUPS, &num_groups,
		groups, &msg_type, &endian_mismatch, sizeof(msg), msg);

	switch(sz)
	{
	case ILLEGAL_SESSION:
		return "illegal session";
	case ILLEGAL_MESSAGE:
		return "illegal message";
	case CONNECTION_CLOSED:
		return "closed";
	case GROUPS_TOO_SHORT:
	case BUFFER_TOO_SHORT:
		return "groups or buffer too short, implementation error";
	}

	vc ret(VC_VECTOR);

	int group_overflow = 0;
	if(Is_regular_mess(srv))
	{
		ret[0] = "data";
		ret[1] = sender;
		if(num_groups < 0)
		{
			group_overflow = 1;
			num_groups = MAX_GROUPS;
		}
		vc grps(VC_VECTOR);
		for(int i = 0; i < num_groups; ++i)
		{
			grps.append(groups[i]);
		}
		ret[2] = grps;
		ret[3] = (int)msg_type;

		vcxstream strm(msg, sz, vcxstream::FIXED);

		if(!strm.open(vcxstream::READABLE))
		{
			return vcnil;
		}
		vc v;
		if(v.xfer_in(strm) != sz)
		{
			return vcnil;
		}
		ret[4] = v;
	}
	else if(Is_membership_mess(srv))
	{
		if(Is_transition_mess(srv))
		{
			ret[0] = "transition";
			ret[1] = sender;
			group_id *g = (group_id *)msg;
			vc v(VC_VECTOR);
			v[0] = g->id[0];
			v[1] = g->id[1];
			v[2] = g->id[2];
			ret[2] = v;
		}
		else if(Is_reg_memb_mess(srv))
		{
			ret[0] = "membership";
			if(num_groups < 0)
			{
				group_overflow = 1;
				num_groups = MAX_GROUPS;
			}
			vc grps(VC_VECTOR);
			for(int i = 0; i < num_groups; ++i)
			{
				grps.append(groups[i]);
			}
			ret[1] = sender;
			ret[2] = grps;

			membership_info m;
			if(SP_get_memb_info(msg, srv, &m) < 0)
				return vcnil;
			if(Is_caused_join_mess(srv) || Is_caused_leave_mess(srv) ||
				Is_caused_disconnect_mess(srv))
			{
				vc mi(VC_VECTOR);
				vc v(VC_VECTOR);
				v[0] = m.gid.id[0];
				v[1] = m.gid.id[1];
				v[2] = m.gid.id[2];
				mi[0] = v;
				mi[1] = m.changed_member;
				// ok, there is a lot of other stuff in here about
				// the membership sets, but we are going to ignore it for
				// the moment;
				ret[3] = mi;
				if(Is_caused_join_mess(srv))
					ret[4] = "join";
				else if(Is_caused_leave_mess(srv))
					ret[4] = "leave";
				else if(Is_caused_disconnect_mess(srv))
					ret[4] = "disconnect";
				else
					return "unknown service type, implementation error";
			}
			else if(Is_caused_network_mess(srv))
			{
				vs_set_info *vs_sets = new vs_set_info[m.num_vs_sets];
				unsigned int my_idx;
				if(SP_get_vs_sets_info(msg, vs_sets, m.num_vs_sets, &my_idx) < 0)
					return "vsset implementation error";
				vc sets(VC_VECTOR);
				for(int i = 0; i < m.num_vs_sets; ++i)
				{
					// ok, this is dicey. the docs aren't clear what
					// happens when a group is exactly MAX_GROUP_NAME
					// chars long. from the source, it appears that
					// groups are truncated at MAX_GROUP_NAME - 1.
					// and then null terminated.
					char *mem_names = new char[vs_sets[i].num_members * MAX_GROUP_NAME];
					if(SP_get_vs_set_members(msg, &vs_sets[i], (char(*)[32])mem_names,
						vs_sets[i].num_members) < 0)
						return "vsset member implementation error";
					vc mems(VC_VECTOR);
					for(int j = 0; j < vs_sets[i].num_members; ++j)
					{
						mems[j] = vc(VC_STRING, &mem_names[j * MAX_GROUP_NAME]);
					}
					sets[i] = mems;
					delete [] mem_names;
				}
				ret[3] = sets;
				ret[4] = "network";
				ret[5] = (int)my_idx;
			}
			else
			{
				return vcnil;
			}
		}
		else if(Is_self_leave(srv))
		{
			ret[0] = "self-leave";
			if(num_groups < 0)
			{
				group_overflow = 1;
				num_groups = MAX_GROUPS;
			}
			vc grps(VC_VECTOR);
			for(int i = 0; i < num_groups; ++i)
			{
				grps.append(groups[i]);
			}
			ret[1] = sender;
			ret[2] = grps;

		}
		else
		{
			return "unknown msg type";
		}
	}
	else
	{
		return "unknown msg type";
	}
	if(group_overflow)
		ret.append("group overflow");
	return ret;

#if 0

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
#endif

}
#if 0
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
#endif
#if 0
static vc
wrap_SP_multigroup_multicast(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(SP_multigroup_multicast(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
(const char [][MAX_GROUP_NAME])cvt_vc_pq_a_31__char((*a)[3]),
cvt_vc__short_int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc_pq_char((*a)[6])));
WRAP_END
return ret;

}
#endif
#if 0
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
#endif
static vc
wrap_SP_multicast(VCArglist *a)
{
	char buf[1024 * 128];
	int len;
	vcxstream strm(buf, sizeof(buf), vcxstream::FIXED);

	if(!strm.open(vcxstream::WRITEABLE))
	{
		return vcnil;
	}
	vc_composite::new_dfs();
    vc tvc = (*a)[4];
    if((len = tvc.xfer_out(strm)) < 0)
	{
		return vcnil;
	}

START_WRAP
	vc ret = cvt__int(SP_multicast(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__short_int((*a)[3]),
len,
buf));
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
#if 0
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
#endif
static vc
wrap_SP_connect(VCArglist *a)
{
	char pgrp[MAX_GROUP_NAME];
START_WRAP
	vc ret = cvt__int(SP_connect(cvt_vc_pq_char((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_p_int((*a)[4]),
pgrp));
//cvt_vc_p_char((*a)[5])));
(*a)[5].local_bind(pgrp);
WRAP_END
return ret;

}
#if 0
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
#endif
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

static void
makefun(const char *name, const vc& fun)
{
    vc(name).bind(fun);
}

#define VC(fun, nicename, attr) vc(fun, nicename, #fun, attr)
#define VC2(fun, nicename, attr, trans) vc(fun, nicename, #fun, attr, trans)
void
wrapper_init_spread_xml ()
{

makefun("wrap_SP_multicast", VC(wrap_SP_multicast, "wrap_SP_multicast", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_SP_scat_multicast", VC(wrap_SP_scat_multicast, "wrap_SP_scat_multicast", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_SP_scat_get_vs_set_members", VC(wrap_SP_scat_get_vs_set_members, "wrap_SP_scat_get_vs_set_members", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_error", VC(wrap_SP_error, "wrap_SP_error", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_poll", VC(wrap_SP_poll, "wrap_SP_poll", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_SP_scat_receive", VC(wrap_SP_scat_receive, "wrap_SP_scat_receive", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_SP_get_vs_set_members", VC(wrap_SP_get_vs_set_members, "wrap_SP_get_vs_set_members", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_SP_multigroup_multicast", VC(wrap_SP_multigroup_multicast, "wrap_SP_multigroup_multicast", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_version", VC(wrap_SP_version, "wrap_SP_version", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_connect", VC(wrap_SP_connect, "wrap_SP_connect", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_SP_multigroup_scat_multicast", VC(wrap_SP_multigroup_scat_multicast, "wrap_SP_multigroup_scat_multicast", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_receive", VC(wrap_SP_receive, "wrap_SP_receive", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_SP_connect_timeout", VC(wrap_SP_connect_timeout, "wrap_SP_connect_timeout", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_SP_set_auth_method", VC(wrap_SP_set_auth_method, "wrap_SP_set_auth_method", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_SP_scat_get_vs_sets_info", VC(wrap_SP_scat_get_vs_sets_info, "wrap_SP_scat_get_vs_sets_info", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_SP_set_auth_methods", VC(wrap_SP_set_auth_methods, "wrap_SP_set_auth_methods", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_join", VC(wrap_SP_join, "wrap_SP_join", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_SP_get_memb_info", VC(wrap_SP_get_memb_info, "wrap_SP_get_memb_info", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_equal_group_ids", VC(wrap_SP_equal_group_ids, "wrap_SP_equal_group_ids", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_kill", VC(wrap_SP_kill, "wrap_SP_kill", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_disconnect", VC(wrap_SP_disconnect, "wrap_SP_disconnect", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_SP_leave", VC(wrap_SP_leave, "wrap_SP_leave", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_SP_scat_get_memb_info", VC(wrap_SP_scat_get_memb_info, "wrap_SP_scat_get_memb_info", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_SP_get_vs_sets_info", VC(wrap_SP_get_vs_sets_info, "wrap_SP_get_vs_sets_info", VC_FUNC_BUILTIN_LEAF));
}
