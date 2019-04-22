
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "vcnil.h"
#include "vcmap.h"
#include "vcio.h"
#include <new>
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcnil.cpp 1.46 1996/11/17 05:58:59 dwight Stable $";

//
// TheNil holds the reference count for all nil's in the
// system. It cannot be const. vcnilrep always points
// to TheNil, and so is consted. This gives a unique
// pointer for nil atoms in the system.
// vcnil is a constant that can be used by clients
// of the package as the nil atom.
//
// the construction of this static is a bit
// problematic: since there can be a lot of
// other static "vc" objects that get initialized
// to nil, the order of initialization is
// important... this should be done before anything
// else. unfortunately, there is no easy way to
// ensure this. the main problem is that the reference
// count will be incorrect if there are static inits
// to vcnil that happen before the initial init of
// TheNil. this may cause the deletion of nil, which
// is bad. so, we kluge: we assume there are never
// more than a few hundered static initializations that
// happen before nil is initialized, and set the initial
// value of the reference count of nil to (say) 200 and hope
// for the best. another solution would be to ignore the
// reference count of nil, but that would require an
// extra test in the vc dtor, operator=, etc., which needs to be fast.
// note: on second thought, i think i'll put the test
// in, since it is guaranteed to work... and we're
// going to put in a real gc at some point...
//
static vc_nil TheNil;
vc_nil *const vc_nil::vcnilrep = &TheNil;
const vc vcnil;

void *
vc_nil::operator new(size_t )
{
	oopanic("new nil?");
    throw std::bad_alloc();
    //return 0;
}

vc_nil::vc_nil() {}
vc_nil::~vc_nil() { }

vc_nil::operator double() const {return 0.0; }
vc_nil::operator int() const {return 0;}
vc_nil::operator long() const {return 0;}
vc_nil::operator const char *() const {return "nil";}

const char *
vc_nil::peek_str() const { return ""; }

vc
vc_nil::bomb_nil() const {USER_BOMB("can't use arithmetic on nil",vcnil);}
vc
vc_nil::operator+(const vc &) const {return bomb_nil();}
vc
vc_nil::operator-(const vc &) const {return bomb_nil();}
vc
vc_nil::operator*(const vc &) const {return bomb_nil();}
vc
vc_nil::operator/(const vc &) const {return bomb_nil();}
vc
vc_nil::operator%(const vc &) const {return bomb_nil();}

vc
vc_nil::int_add(const vc&) const {return bomb_nil();}
vc
vc_nil::int_sub(const vc&) const {return bomb_nil();}
vc
vc_nil::int_mul(const vc&) const {return bomb_nil();}
vc
vc_nil::int_div(const vc&) const {return bomb_nil();}
vc
vc_nil::int_mod(const vc&) const {return bomb_nil();}

vc
vc_nil::double_add(const vc&) const {return bomb_nil();}
vc
vc_nil::double_sub(const vc&) const {return bomb_nil();}
vc
vc_nil::double_mul(const vc&) const {return bomb_nil();}
vc
vc_nil::double_div(const vc&) const {return bomb_nil();}
vc
vc_nil::double_mod(const vc&) const {return bomb_nil();}

int
vc_nil::int_lt(const vc&) const {return TRUE;}
int
vc_nil::int_le(const vc&) const {return TRUE;}
int
vc_nil::int_gt(const vc&) const {return FALSE;}
int
vc_nil::int_ge(const vc&) const {return FALSE;}
int
vc_nil::int_eq(const vc&) const {return FALSE;}
int
vc_nil::int_ne(const vc&) const {return TRUE;}

int
vc_nil::double_lt(const vc&) const {return TRUE;}
int
vc_nil::double_le(const vc&) const {return TRUE;}
int
vc_nil::double_gt(const vc&) const {return FALSE;}
int
vc_nil::double_ge(const vc&) const {return FALSE;}
int
vc_nil::double_eq(const vc&) const {return FALSE;}
int
vc_nil::double_ne(const vc&) const {return TRUE;}

int
vc_nil::func_eq(const vc&) const {return FALSE;}

enum vc_type
vc_nil::type() const { return VC_NIL; }
int
vc_nil::is_nil() const { return TRUE;}

int
vc_nil::operator <(const vc &v) const { if(v.is_nil()) return FALSE; return TRUE; }
int
vc_nil::operator <=(const vc &) const { return TRUE; }
int
vc_nil::operator >(const vc &) const { return FALSE; }
int
vc_nil::operator >=(const vc &v) const { if(v.is_nil()) return TRUE; return FALSE; }
int
vc_nil::operator ==(const vc &v) const { if(v.is_nil()) return TRUE; return FALSE; }
int
vc_nil::operator !=(const vc &v) const { if(v.is_nil()) return FALSE; return TRUE; }

hashValueType
vc_nil::hashValue() const {return 0;}
void
vc_nil::printOn(VcIO outputStream) {outputStream << "nil";}
void
vc_nil::stringrep(VcIO os) const
{
	os << "nil";
}

long
vc_nil::xfer_out(vcxstream& )
{
	return 0;
}

long
vc_nil::xfer_in(vcxstream& )
{
	return 0;
}
