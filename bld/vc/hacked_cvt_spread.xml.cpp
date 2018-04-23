
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

static vc
cvt__int(int foo)
{
	return foo;
}
static
dummy_group_id
cvt_vc__dummy_group_id_(vc a)
{
	group_id g;
	g.id[0] = a[0];
	g.id[1] = a[1];
	g.id[2] = a[2];
	return g;
}
#if 0
static
dummy_time
cvt_vc__dummy_time_(vc a)
{
}
#endif
static
int
cvt_vc__int(vc a)
{
	return a;
}
static
short int
cvt_vc__short_int(vc a)
{
	return (int)a;
}
#if 0
static
pF__int__int_x_p_void
cvt_vc_pF__int__int_x_p_void(vc a)
{
}

static
char*
cvt_vc_p_a_31__char(vc a)
{
}
#endif

// always out
static
char*
cvt_vc_p_char(vc a)
{
	if(a.is_nil())
		return 0;
	char *v = (char *)malloc(MAX_GROUP_NAME);
	vc bv(VC_VECTOR);
	bv[0] = a;
	bv[1] = (long)v;
	outbind_str->append(bv);
	push_ptr(v);
	return v;
}

#if 0
static
dummy_membership_info*
cvt_vc_p_dummy_membership_info_(vc a)
{
}
static
dummy_scatter*
cvt_vc_p_dummy_scatter_(vc a)
{
}
static
dummy_vs_set_info*
cvt_vc_p_dummy_vs_set_info_(vc a)
{
}
#endif

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

#if 0
static
short int*
cvt_vc_p_short_int(vc a)
{
}

static
unsigned int*
cvt_vc_p_unsigned_int(vc a)
{
}

static
void*
cvt_vc_p_void(vc a)
{
}
static
ppF__int__int_x_p_void
cvt_vc_ppF__int__int_x_p_void(vc a)
{
}
static
void**
cvt_vc_pp_void(vc a)
{
}
static
const char**
cvt_vc_ppq_char(vc a)
{
}
#endif

// always in
// this lib isn't reentrant, so we just use a static buffer
static
const char*
cvt_vc_pq_a_31__char(vc a)
{
	static char grps[256][MAX_GROUP_NAME];
	int n = a.num_elems();
	for(int i = 0; i < n; ++i)
	{
		if(a[i].type() != VC_STRING ||
			a[i].len() >= MAX_GROUP_NAME)
			oopanic("group names must be strings or group name too long");
		strcpy(&grps[i][0], (const char *)a[i]);
	}
	return &grps[0][0];
}
static
const char*
cvt_vc_pq_char(vc a)
{
	if(a.is_nil())
		return 0;
	return a;
}
#if 0
static
const dummy_scatter*
cvt_vc_pq_dummy_scatter_(vc a)
{
}
static
const dummy_vs_set_info*
cvt_vc_pq_dummy_vs_set_info_(vc a)
{
}
#endif
