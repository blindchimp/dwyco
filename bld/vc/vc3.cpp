
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "vcdeflt.h"
#include "vccomp.h"
#include "vcmap.h"
#include "vcio.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vc3.cpp 1.47 1997/11/27 03:05:54 dwight Stable $";

int
vc::member_select(const vc& member, vc& out, int toplev, vc_object *topobj)
{
	return rep->member_select(member, out, toplev, topobj);
}

vc_fundef *
vc:: get_fundef() const
{
	return rep->get_fundef();
}

vc_object *
vc::get_obj() const
{
	return rep->get_obj();
}

int
vc::is_atomic() const {return rep->is_atomic();}
int
vc::is_varadic() const {return rep->is_varadic();}
int
vc::must_eval_args() const { return rep->must_eval_args(); }
int
vc::is_quoted() const {return rep->is_quoted();}
int
vc::is_map_inhibited() const { return rep->is_map_inhibited();}
void
vc::bind(const vc& v) const { rep->bind(v);}
void
vc::local_bind(const vc& v) const { rep->local_bind(v);}
void
vc::global_bind(const vc& v) const { rep->global_bind(v);}
void
vc::bremove() const { rep->bremove();}
void
vc::local_bremove() const { rep->local_bremove();}
void
vc::global_bremove() const { rep->global_bremove();}
enum vc_type
vc::type() const { return rep->type(); }
#ifdef VC_DBG_ENVELOPE
int
vc::is_nil() const { return rep->is_nil(); }
#endif
int
vc::is_decomposable() const { return rep->is_decomposable();}
vc
vc::copy() const {vc v; vc_default *v2 = rep->do_copy(); v.redefine(v2); return v;}
vc
vc::get_special() const {return rep->get_special();}
const char *
vc::peek_str() const { return rep->peek_str(); }
long
vc::len() const {return rep->len();}
int
vc::operator <(const vc &v) const {  return rep->operator<(v); }
int
vc::operator <=(const vc &v) const { return rep->operator<=(v); }
int
vc::operator >(const vc &v) const { return rep->operator>(v); }
int
vc::operator >=(const vc &v) const {return rep->operator>=(v); }
int
vc::operator ==(const vc &v) const {return rep->operator==(v); }
int
vc::operator !=(const vc &v) const {return rep->operator!=(v); }

vc
vc::operator+(const vc &v) const {return rep->operator+(v); }
vc
vc::operator-(const vc &v) const {return rep->operator-(v); }
vc
vc::operator*(const vc &v) const {return rep->operator*(v); }
vc
vc::operator/(const vc &v) const {return rep->operator/(v); }
vc
vc::operator%(const vc &v) const {return rep->operator%(v); }

//
// meta-data ops
//
vc
vc::funmeta() const {return rep->funmeta();}
#if 0
// functor action
vc
vc::operator()(void) const {return (*rep)(); }
//vc
//vc::operator()(void *p) const { return (*rep)(p);}
vc
vc::operator()(VCArglist *al) const { return (*rep)(al);}

vc
vc::operator()(vc v0) const { return (*rep)(v0);}
vc
vc::operator()(vc v0, vc v1) const {return (*rep)(v0,v1);}
vc
vc::operator()(vc v0, vc v1, vc v2) const {return (*rep)(v0, v1, v2);}
vc
vc::operator()(vc v0, vc v1, vc v2, vc v3) const {return (*rep)(v0, v1, v2, v3);}
#endif


// decomposable operations
int
vc::is_empty() const {return rep->is_empty();}
int
vc::num_elems() const {return rep->num_elems();}
vc&
vc::operator[](const vc& v) {return (*rep)[v];}
vc&
vc::operator[](int i) {return (*rep)[i];}
vc&
vc::operator[](long i) {return (*rep)[i];}
const vc&
vc::operator[](const vc& v) const {return (*rep)[v];}
const vc&
vc::operator[](int i) const {return (*rep)[i];}
const vc&
vc::operator[](long i) const {return (*rep)[i];}
int
vc::contains(const vc& v) const {return rep->contains(v);}
int
vc::find(const vc& v, vc& out) {return rep->find(v, out);}
void
vc::add(const vc& v) {rep->add(v);}
void
vc::add_kv(const vc& k, const vc& v) {rep->add_kv(k, v);}
int
vc::del(const vc& v) {return rep->del(v);}
void
vc::append(const vc& v) {rep->append(v);}
vc
vc::remove_last() {return rep->remove_last();}
void
vc::prepend(const vc& v) {rep->prepend(v);}
vc
vc::remove_first() {return rep->remove_first();}
vc
vc::remove_first2(vc& v) {return rep->remove_first2(v);}
void
vc::remove(const vc& strt, const vc& len) {rep->remove(strt, len);}
void
vc::remove(const vc& strt, long len) {rep->remove(strt, len);}
void
vc::remove(long strt, const vc& len) {rep->remove(strt, len);}
void
vc::remove(long strt, long len) {rep->remove(strt, len);}
void
vc::insert(const vc& item, const vc& idx) {rep->insert(item, idx);}
void
vc::insert(const vc& item, long idx) {rep->insert(item, idx);}

void
vc::foreach(const vc& v, const vc& expr) const {rep->foreach(v, expr);}
void
vc::foreach(const vc& v, VC_FOREACH_CALLBACK cb) const {rep->foreach(v, cb);}
void
vc::foreach(const vc& v, VC_FOREACH_CALLBACK2 cb) const {rep->foreach(v, cb);}

// some arithmetics

vc
vc::int_add(const vc& v) const {return rep->int_add(v);}
vc
vc::int_sub(const vc& v) const {return rep->int_sub(v);}
vc
vc::int_mul(const vc& v) const {return rep->int_mul(v);}
vc
vc::int_div(const vc& v) const {return rep->int_div(v);}
vc
vc::int_mod(const vc& v) const {return rep->int_mod(v);}

vc
vc::double_add(const vc& v) const {return rep->double_add(v);}
vc
vc::double_sub(const vc& v) const {return rep->double_sub(v);}
vc
vc::double_mul(const vc& v) const {return rep->double_mul(v);}
vc
vc::double_div(const vc& v) const {return rep->double_div(v);}
vc
vc::double_mod(const vc& v) const {USER_BOMB("can't modulo floating numbers",vcone); }

int
vc::int_lt(const vc& v) const { return rep->int_lt(v);}
int
vc::int_le(const vc& v) const { return rep->int_le(v);}
int
vc::int_gt(const vc& v) const { return rep->int_gt(v);}
int
vc::int_ge(const vc& v) const  { return rep->int_ge(v);}
int
vc::int_eq(const vc& v) const  { return rep->int_eq(v);}
int
vc::int_ne(const vc& v) const  { return rep->int_ne(v);}

int
vc::double_lt(const vc& v) const  { return rep->double_lt(v);}
int
vc::double_le(const vc& v) const  { return rep->double_le(v);}
int
vc::double_gt(const vc& v) const  { return rep->double_gt(v);}
int
vc::double_ge(const vc& v) const  { return rep->double_ge(v);}
int
vc::double_eq(const vc& v) const  { return rep->double_eq(v);}
int
vc::double_ne(const vc& v) const  { return rep->double_ne(v);}

int
vc::str_eq(const vc& v) const { return rep->str_eq(v);}
int
vc::str_ne(const vc& v) const { return rep->str_ne(v);}
int
vc::str_lt(const vc& v) const { return rep->str_lt(v);}
int
vc::str_le(const vc& v) const { return rep->str_le(v);}
int
vc::str_gt(const vc& v) const { return rep->str_gt(v);}
int
vc::str_ge(const vc& v) const { return rep->str_ge(v);}

vc
vc::vec_add(const vc& v) const {return rep->vec_add(v);}
vc
vc::vec_sub(const vc& v) const {return rep->vec_sub(v);}
vc
vc::vec_mul(const vc& v) const {return rep->vec_mul(v);}
vc
vc::vec_div(const vc& v) const {return rep->vec_div(v);}
vc
vc::vec_mod(const vc& v) const {return rep->vec_mod(v);}
VC_ERR_CALLBACK
vc::set_err_callback(VC_ERR_CALLBACK c) {return rep->set_err_callback(c);}

int
vc::func_eq(const vc& v) const  { return rep->func_eq(v);}
int
vc::set_eq(const vc& v) const { return rep->set_eq(v);}

// regular expression matching

int
vc::re_match(const vc& v, int len, int pos) const { return rep->re_match(v, len, pos); }
int
vc::re_match(const vc& v) const { return rep->re_match(v); }
int
vc::re_search(const vc& v, int len, int& matchlen, int pos) const {return rep->re_search(v, len, matchlen, pos);}
int
vc::re_match_info(int& strt, int& len, int nth) const {return rep->re_match_info(strt, len, nth);}

// File I/O
int
vc::open(const vc& name, vcfilemode m)  {return rep->open(name, m);}
int
vc::close()  {return rep->close();}
int
vc::seek(long pos, int whence)  {return rep->seek(pos, whence);}
int
vc::read(void *buf, long& len)  {return rep->read(buf, len); }
int
vc::fgets(void *buf, long len)  {return rep->fgets(buf, len); }
int
vc::write(const void *buf, long& len)  {return rep->write(buf, len);}
int
vc::eof() {return rep->eof();}

// conversions, misc.
vc::operator int() const { return (int)*rep; }
vc::operator double() const { return (double)*rep;}
vc::operator const char *() const { return (const char *)*rep; }
vc::operator long() const { return (long)*rep;}
vc::operator long long() const { return (long long)*rep;}
vc::operator char() const { return (char)*rep;}
vc::operator void *() const { return (void *)*rep;}

void
vc::print_top(VcIO o)
{
	vc_composite::new_dfs();
	printOn(o);
}

// only use this if you're sure there are no cycles
void
vc::print(VcIO o)
{
	rep->printOn(o);
}

void
vc::operator<<(vc ov)
{
	(*rep) << ov;
}


// Members needed by Borland classlib

hashValueType
vc::hashValue() const {return rep->hashValue();}

void
vc::printOn(VcIO outputStream) 
{
	
	if(visited())
	{
		outputStream << "<<visited>>";
	}
	else
	{
		set_visited();
		rep->printOn(outputStream);
	}
}

vc
vc::force_eval() const
{
#ifdef NO_VCEVAL
    oopanic("eval");
#else
	if(is_atomic())
		return *this;
	return rep->eval();
#endif
}

vc
vc::eval() const
{
#ifdef NO_VCEVAL
    oopanic("eval2");
#else
#ifdef VCDBG
    VcDebugNode *n = VcDbgInfo.get();
    if(n->src_list)
    {
        (*n->src_list)[n->cur_idx].print();
    }
#endif
	if(is_quoted())
		return *this;
	return rep->eval();
#endif
}

void
vc::stringrep(VcIO o) const {rep->stringrep(o);}

vc
vc::translate(VcIO o) const
{
    return rep->translate(o);
}

long
vc::overflow(vcxstream& vcx, char *b, long l)
{
	return rep->overflow(vcx, b, l);
}

long
vc::underflow(vcxstream& vcx, char *b, long min, long max)
{
	return rep->underflow(vcx, b, min, max);
}

long
vc::devopen(vcxstream& vcx, int style, int stat)
{
	return rep->devopen(vcx, style, stat);
}

long
vc::devclose(vcxstream& vcx, int style)
{
	return rep->devclose(vcx, style);
}

vc
vc::set_device(vc d)
{
	return rep->set_device(d);
}

#ifdef VCDBG
void
vc::set_break_on(int m)
{
	rep->set_break_on(m);
}

void
vc::clear_break_on(int m)
{
	rep->clear_break_on(m);
}

int
vc::break_on(int m) const
{
	return rep->break_on(m);
}

void
vc::set_global_break_on(int a)
{
  Eval_break = 1;
}
void
vc::clear_global_break_on(int a)
{
  Eval_break = 0;
}

#endif
