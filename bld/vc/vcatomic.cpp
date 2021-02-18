
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "vcatomic.h"
#include "vcmap.h"
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcatomic.cpp 1.47 1997/11/27 03:05:58 dwight Stable $";
static vc bogus;
void
vc_atomic::bomb_setop() const
{
    // 99 times out of a 100 it is probably nil, but we'll
    // add the type in here anyways
    enum vc_type t = type();
    char a[1024];
    sprintf(a, "can't do set operation on atomic (%d)", (int)t);
    USER_BOMB2(a);
}

void
vc_atomic::bomb_str_rel() const
{
	USER_BOMB2("can't compare string to non-string with relational");
}

void
vc_atomic::bomb_call_atom() const
{
	USER_BOMB2("attempt to funcall non-function.");
}


#define bombvec(op, msg) \
vc \
vc_atomic::vec_##op(const vc&) const \
{ \
	USER_BOMB("can't perform vector " msg " on this type of atom", vcone); \
}
bombvec(add, "addition");
bombvec(sub, "subtraction");
bombvec(mul, "multiplication");
bombvec(div, "division");
bombvec(mod, "modulo");
#undef bombvec

vc_atomic::vc_atomic()
{
}

int
vc_atomic::visited()
{
	return FALSE;
}

void
vc_atomic::set_visited()
{
}

int
vc_atomic::is_atomic() const {return TRUE;}
int
vc_atomic::is_quoted() const {return TRUE;}
int
vc_atomic::is_map_inhibited() const {return FALSE;}

// defaults for string comparisons are that no non-string atom is
// equal to any string, and all non-string atoms are not eq
// to any string. might make sense to change this at some
// time in the future to either disallow string-non-string
// comparisons, or do some sort of coercion.
int
vc_atomic::str_eq(const vc&) const {return FALSE;}
int
vc_atomic::str_ne(const vc&) const {return TRUE;}
int
vc_atomic::str_le(const vc&) const {bomb_str_rel(); return FALSE;}
int
vc_atomic::str_lt(const vc&) const {bomb_str_rel(); return FALSE;}
int
vc_atomic::str_gt(const vc&) const {bomb_str_rel(); return FALSE;}
int
vc_atomic::str_ge(const vc&) const {bomb_str_rel(); return FALSE;}

// atomics are not equal to any set or function
int
vc_atomic::set_eq(const vc&) const {return FALSE;}
int
vc_atomic::func_eq(const vc&) const {return FALSE;}

vc
vc_atomic::operator()(void) const {bomb_call_atom();return vcnil;}
//vc
//vc_atomic::operator()(void *) const {bomb_call_atom();return vcnil;}
vc
vc_atomic::operator()(VCArglist *) const {bomb_call_atom();return vcnil;}

vc
vc_atomic::operator()(vc ) const {bomb_call_atom();return vcnil;}
vc
vc_atomic::operator()(vc , vc ) const {bomb_call_atom();return vcnil;}
vc
vc_atomic::operator()(vc , vc , vc ) const {bomb_call_atom();return vcnil;}
vc
vc_atomic::operator()(vc , vc , vc , vc ) const {bomb_call_atom();return vcnil;}

int
vc_atomic::num_elems() const {bomb_setop(); return 0;}
int
vc_atomic::is_empty() const {bomb_setop(); return FALSE;}
vc&
vc_atomic::operator[](const vc& ) {bomb_setop(); return bogus;}
vc&
vc_atomic::operator[](int){bomb_setop(); return bogus;}
vc&
vc_atomic::operator[](long){bomb_setop(); return bogus;}
const vc&
vc_atomic::operator[](const vc& ) const {bomb_setop(); return bogus;}
const vc&
vc_atomic::operator[](int) const {bomb_setop(); return bogus;}
const vc&
vc_atomic::operator[](long) const {bomb_setop(); return bogus;}
int
vc_atomic::contains(const vc&){bomb_setop(); return FALSE;}
int
vc_atomic::find(const vc&, vc& ){bomb_setop(); return FALSE;}
void
vc_atomic::add(const vc&){bomb_setop();}
void
vc_atomic::add_kv(const vc&, const vc&){bomb_setop();}
int
vc_atomic::del(const vc&){bomb_setop(); return FALSE;}
void
vc_atomic::append(const vc&){bomb_setop();}
vc
vc_atomic::remove_last(){bomb_setop(); return vcnil;}
void
vc_atomic::prepend(const vc&){bomb_setop();}
vc
vc_atomic::remove_first(){bomb_setop(); return vcnil;}
vc
vc_atomic::remove_first2(vc&){bomb_setop(); return vcnil;}
void
vc_atomic::remove(const vc& , const vc& ){bomb_setop();}
void
vc_atomic::remove(const vc& , long ){bomb_setop();}
void
vc_atomic::remove(long , const vc& ){bomb_setop();}
void
vc_atomic::remove(long , long ){bomb_setop();}
void
vc_atomic::insert(const vc& , const vc& ){bomb_setop();}
void
vc_atomic::insert(const vc& , long ){bomb_setop();}


