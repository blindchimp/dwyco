
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "vccomp.h"
#include "vcmap.h"
#include "dwstr.h"
#include "vcio.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vccomp.cpp 1.49 1998/06/22 03:56:25 dwight Exp $";

static vc bogus;
long vc_composite::Visit_SN;

vc
vc_composite::bomb_funcall() const
{
	USER_BOMB("attempt to funcall type with no function semantics",vcnil);
}

void
vc_composite::bomb_op() const
{
	USER_BOMB2("attempt to do arithmetic on composite");
}

void
vc_composite::bomb_func(const char *type) const
{
	char s[1000];
	sprintf(s, "can't convert composite to %s\n", type);
	USER_BOMB2(s);
}

void
vc_composite::bomb_setop() const
{
	USER_BOMB2("operation not supported on composite");
}
#define bombvec(op, msg) \
vc \
vc_composite::vec_##op(const vc&) const \
{ \
	USER_BOMB("can't perform vector " msg " on this type of composite", vcone); \
}
bombvec(add, "addition");
bombvec(sub, "subtraction");
bombvec(mul, "multiplication");
bombvec(div, "division");
bombvec(mod, "modulo");
#undef bombvec

#if 0
DwListA<vc_composite *> vc_composite::Comp_list;
#endif

vc_composite::vc_composite()
{
	quoted = 0;
	dont_map = 0;
	visit = 0;
#if 0
	Comp_list.prepend(this);
	Comp_list.rewind();
    backptr = Comp_list.getpos();
#endif
    err_callback = 0;
}

vc_composite::~vc_composite()
{
#if 0
	Comp_list.setpos(backptr);
    Comp_list.remove();
#endif
}

int
vc_composite::is_atomic() const {return FALSE;}
int
vc_composite::is_quoted() const {return quoted;}
int
vc_composite::is_map_inhibited() const {return dont_map;}


int
vc_composite::visited()
{
	return visit == Visit_SN;
}

void
vc_composite::set_visited()
{
	visit = Visit_SN;
}

void
vc_composite::new_dfs()
{
	++Visit_SN;
	if(Visit_SN == 0)
	{
		oopanic("out of dfs");
#if 0
    	vc_composite *v;
		dwlista_foreach(v, Comp_list)
		{
			v->visit = 0;
		}
#endif
		Visit_SN = 1;
	}
}

#if 0
void
vc_composite::flush_all_cache()
{
	vc_composite *v;
	dwlista_foreach(v, Comp_list)
	{
		v->flush_cache();
	}
}
#endif

//
// peeking at a composite gets you the stringrep for it.
//

const char *
vc_composite::peek_str() const
{
	static DwString *a;
	VcIOHackStr o;
	stringrep(o);
	o << '\0';
	delete a;
	a = new DwString(o.ref_str(), 0, o.pcount());
	return a->c_str();
}


// set default set/func equal st no composite is equal
// to set for function unless defined otherwise

int
vc_composite::set_eq(const vc&) const {return FALSE;}
int
vc_composite::func_eq(const vc&) const {return FALSE;}


//vc
//vc_composite::operator()(void *) const {return bomb_funcall();}
vc
vc_composite::operator()(VCArglist *) const {return bomb_funcall();}

vc
vc_composite::operator()() const {return bomb_funcall();}
vc
vc_composite::operator()(vc v0) const {return bomb_funcall();}
vc
vc_composite::operator()(vc v0, vc v1) const {return bomb_funcall();}
vc
vc_composite::operator()(vc v0, vc v1, vc v2) const {return bomb_funcall();}
vc
vc_composite::operator()(vc v0, vc v1, vc v2, vc v3) const {return bomb_funcall();}

vc_composite::operator double() const {bomb_func("double"); return 0.0; }
vc_composite::operator int() const {bomb_func("int"); return 0;}
vc_composite::operator long() const {bomb_func("long"); return 0;}
vc_composite::operator const char *() const {bomb_func("const char *"); return "";}

vc
vc_composite::operator+(const vc &) const {bomb_op();return vc(0);}
vc
vc_composite::operator-(const vc &) const {bomb_op();return vc(0);}
vc
vc_composite::operator*(const vc &) const {bomb_op();return vc(0);}
vc
vc_composite::operator/(const vc &) const {bomb_op();return vc(0);}
vc
vc_composite::operator%(const vc &) const {bomb_op();return vc(0);}

vc
vc_composite::int_add(const vc& ) const {bomb_op();return vc(0);}
vc
vc_composite::int_sub(const vc& ) const {bomb_op();return vc(0);}
vc
vc_composite::int_mul(const vc& ) const {bomb_op();return vc(0);}
vc
vc_composite::int_div(const vc& ) const {bomb_op();return vc(0);}
vc
vc_composite::int_mod(const vc& ) const {bomb_op();return vc(0);}

vc
vc_composite::double_add(const vc& ) const {bomb_op();return vc(0.0);}
vc
vc_composite::double_sub(const vc& ) const {bomb_op();return vc(0.0);}
vc
vc_composite::double_mul(const vc& ) const {bomb_op();return vc(0.0);}
vc
vc_composite::double_div(const vc& ) const {bomb_op();return vc(0.0);}
vc
vc_composite::double_mod(const vc& ) const {bomb_op();return vc(0.0);}

int
vc_composite::int_lt(const vc& ) const {bomb_op();return 0;}
int
vc_composite::int_le(const vc& ) const {bomb_op();return 0;}
int
vc_composite::int_gt(const vc& ) const {bomb_op();return 0;}
int
vc_composite::int_ge(const vc& ) const {bomb_op();return 0;}
int
vc_composite::int_eq(const vc& ) const {return FALSE;}
int
vc_composite::int_ne(const vc& ) const {return TRUE;}

int
vc_composite::double_lt(const vc& ) const {bomb_op();return 0;}
int
vc_composite::double_le(const vc& ) const {bomb_op();return 0;}
int
vc_composite::double_gt(const vc& ) const {bomb_op();return 0;}
int
vc_composite::double_ge(const vc& ) const {bomb_op();return 0;}
int
vc_composite::double_eq(const vc& ) const {return FALSE;}
int
vc_composite::double_ne(const vc& ) const {return TRUE;}

int
vc_composite::str_eq(const vc& ) const {return FALSE;}
int
vc_composite::str_ne(const vc& ) const {return TRUE;}
int
vc_composite::str_lt(const vc& ) const {bomb_op();return 0;}
int
vc_composite::str_le(const vc& ) const {bomb_op();return 0;}
int
vc_composite::str_gt(const vc& ) const {bomb_op();return 0;}
int
vc_composite::str_ge(const vc& ) const {bomb_op();return 0;}


int
vc_composite::operator <(const vc &) const {bomb_op();return 0;}
int
vc_composite::operator <=(const vc &) const {bomb_op();return 0;}
int
vc_composite::operator >(const vc &) const {bomb_op();return 0;}
int
vc_composite::operator >=(const vc &) const {bomb_op();return 0;}

vc&
vc_composite::operator[](const vc& ) {bomb_setop(); return bogus;}
vc&
vc_composite::operator[](int){bomb_setop(); return bogus;}
vc&
vc_composite::operator[](long){bomb_setop(); return bogus;}
const vc&
vc_composite::operator[](const vc& ) const {bomb_setop(); return bogus;}
const vc&
vc_composite::operator[](int)const {bomb_setop(); return bogus;}
const vc&
vc_composite::operator[](long)const {bomb_setop(); return bogus;}
int
vc_composite::contains(const vc&){bomb_setop();return 0;}
int
vc_composite::find(const vc&, vc& ){bomb_setop();return 0;}
void
vc_composite::add(const vc&){bomb_setop();}
void
vc_composite::add_kv(const vc&, const vc&){bomb_setop();}
int
vc_composite::del(const vc&){bomb_setop();return 0;}
void
vc_composite::append(const vc&){bomb_setop();}
vc
vc_composite::remove_last(){bomb_setop();return vcnil;}
void
vc_composite::prepend(const vc&){bomb_setop();}
vc
vc_composite::remove_first(){bomb_setop();return vcnil;}
vc
vc_composite::remove_first2(vc&){bomb_setop();return vcnil;}
void
vc_composite::remove(const vc& , const vc& ){bomb_setop();}
void
vc_composite::remove(const vc& , long ){bomb_setop();}
void
vc_composite::remove(long , const vc& ){bomb_setop();}
void
vc_composite::remove(long , long ){bomb_setop();}
void
vc_composite::insert(const vc& , const vc& ){bomb_setop();}
void
vc_composite::insert(const vc& , long ){bomb_setop();}


VC_ERR_CALLBACK
vc_composite::set_err_callback(VC_ERR_CALLBACK cb)
{
	VC_ERR_CALLBACK ocb = err_callback;
	err_callback = cb;
	return ocb;
}
