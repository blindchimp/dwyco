
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "vcdeflt.h"
#include "vcmap.h"
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcdeflt.cpp 1.52 1998/08/06 04:44:57 dwight Exp $";

#ifdef OBJTRACK
#include "dwmapr.h"

class Int
{
public:
	int i;
	Int() {i = 0;}
	operator int() const {return i;}
	Int(int i) {this->i = i;}
	int operator==(const Int& i2) const {return i == i2.i;}
	unsigned long hashValue() const {return i;}
};
struct TrackInfo
{
	int serial;
	vc_default *obj;
	
};

DwMapR<TrackInfo, Int> Objmap(TrackInfo(), Int(), 2017);
typedef DwAssocImp<TrackInfo, Int> ObjmapAssoc;
typedef DwMapRIter<TrackInfo, Int> ObjmapIter;

static int Serial = 1;
static int Cleared_serial;

void
dump_objmap()
{
	ObjmapIter i(&Objmap);
	VcError << "\n-----------------" << Serial << "\n";
	for(; !i.eol(); i.forward())
	{
		ObjmapAssoc oa = i.get();
		TrackInfo t = oa.peek_value();
		if(t.serial < Cleared_serial)
			continue;
		VcError << "serial: " << t.serial ;
		VcError << " age: " << Serial - t.serial;
		VcError << " nage: " << (double)(Serial - t.serial) / Serial;
		VcError << " rc: " << t.obj->ref_count;
		VcError << " ### ";
		t.obj->print_top(VcError);
		VcError << "@@@\n";
	}
	VcError << "\n====\n";
	VcError.flush();
}

void
clear_objmap()
{
	Cleared_serial = Serial - 1;
}

#endif

vc_default::vc_default()
{
	ref_count = 1;
#ifdef VCDBG
	break_tag = BREAK_NONE;
#endif
#ifdef OBJTRACK
	serial = Serial++;
	TrackInfo tinfo;
	tinfo.serial = serial;
	tinfo.obj = this;
	Objmap.add(serial, tinfo);
#endif
}

// we don't want to allow objects to be
// copied verbatim because that would leave
// the reference count messed up.
vc_default::vc_default(const vc_default&)
{
	ref_count = 1;
#ifdef VCDBG
	break_tag = BREAK_NONE;
#endif
}

#ifdef OBJTRACK
vc_default::~vc_default()
{
	if(rep != 0)
		return;
	TrackInfo tinfo;
	if(!Objmap.find(serial, tinfo))
	{
		::abort();
	}
	Objmap.del(serial);
}
#endif

vc
vc_default::funmeta() const 
{
	USER_BOMB("no meta-data available", vcnil);
}

int
vc_default::member_select(const vc& member, vc& out, int toplev, vc_object *topobj)
{
	USER_BOMB("member selection not defined on item", 0);
}

vc_fundef *
vc_default::get_fundef() const
{
	oopanic("get fundef on non-fundef");
	return 0;
}

vc_object *
vc_default::get_obj() const
{
	oopanic("get obj on non-obj");
	return 0;
}

int
vc_default::open(const vc& , vcfilemode) {USER_BOMB("can't open non-file", 0);}
int
vc_default::close() {USER_BOMB("can't close non-file",0);}
int
vc_default::seek(long , int ) {USER_BOMB("can't seek on non-file",0);}
int
vc_default::read(void *, long& ) {USER_BOMB("can't read on non-file", 0);}
int
vc_default::fgets(void *, long ) {USER_BOMB("can't fgets on non-file", 0);}
int
vc_default::write(const void *, long& ) {USER_BOMB("can't write on non-file", 0);}
int
vc_default::eof() {USER_BOMB("can't eof on non-file", 0);}
vc
vc_default::get_special() const {return vcnil;}
int
vc_default::is_nil() const {return FALSE;}

int
vc_default::is_atomic() const {oopanic("unimp atomic"); return 0;}
int
vc_default::is_varadic() const {oopanic("unimp varadic"); return 0;}
int
vc_default::must_eval_args() const {oopanic("unimp must-eval"); return 0;}
	
int
vc_default::is_quoted() const {oopanic("unimp quote"); return 0;}
int
vc_default::is_map_inhibited() const {oopanic("unimp inhibit"); return 0;}
void
vc_default::bind(const vc& ) const {USER_BOMB2("can't bind to that");}
void
vc_default::local_bind(const vc& ) const {USER_BOMB2("can't local bind to that");}
void
vc_default::global_bind(const vc& ) const {USER_BOMB2("can't global bind to that");}
void
vc_default::bremove() const {USER_BOMB2("can't remove binding for that");}
void
vc_default::local_bremove() const {USER_BOMB2("can't remove local binding for that");}
void
vc_default::global_bremove() const {USER_BOMB2("can't remove global binding for that");}
enum vc_type 
vc_default::type() const {oopanic("unknown type");return VC_INT;}

int
vc_default::is_decomposable() const {return 0;}
vc
vc_default::copy() const {USER_BOMB("unimplemented copy operation", vcnil);}

const char *
vc_default::peek_str() const {USER_BOMB("unimplemented substitution-rep", "");}

// default is that len is the length of the peek_str
long
vc_default::len() const
{
	const char *a = peek_str();
	return strlen(a);
}

vc_default *
vc_default::do_copy() const {oopanic("unimp copy");return 0;}
	
vc
vc_default::socket_init(const vc& local_addr, int listen, int reuse, int syntax) { USER_BOMB("socket op on non-socket", vcnil); }
vc
vc_default::socket_init(SOCKET, vc listening, int syntax) { USER_BOMB("socket op on non-socket", vcnil); }
vc
vc_default::socket_accept(vc& addr_info) { USER_BOMB("socket op on non-socket", vcnil); }
vc
vc_default::socket_connect(const vc& addr_info) { USER_BOMB("socket op on non-socket", vcnil); }
vc
vc_default::socket_close(int close_info) { USER_BOMB("socket op on non-socket", vcnil); }
vc
vc_default::socket_shutdown(int how) { USER_BOMB("socket op on non-socket", vcnil); }
int
vc_default::socket_poll(int what_for, long to_sec, long to_usec) { USER_BOMB("socket op on non-socket", 0); }
vc
vc_default::socket_send(vc item, const vc& send_info, const vc&) { USER_BOMB("socket op on non-socket", vcnil); }
vc
vc_default::socket_recv(vc& item, const vc& recv_info, vc&) { USER_BOMB("socket op on non-socket", vcnil); }
vc
vc_default::socket_send_raw(void *buf, long& len, long timeout, const vc&) { USER_BOMB("socket op on non-socket", vcnil); }
vc
vc_default::socket_recv_raw(void *buf, long& len, long timeout, vc&) { USER_BOMB("socket op on non-socket", vcnil); }
vc
vc_default::socket_error()
{
	USER_BOMB("can't get socket_error from non-socket", vcnil);
}
vc
vc_default::socket_error_desc()
{
	USER_BOMB("can't get socket_error_desc from non-socket", vcnil);
}
void
vc_default::socket_set_error(vc)
{
	USER_BOMB2("can't set socket error on non-socket");
}
vc
vc_default::socket_set_option(vcsocketmode, unsigned long, unsigned long, unsigned long)
{
	USER_BOMB("can't set socket option on non-socket", vcnil);
}

vcsocketmode
vc_default::socket_get_mode()
{
	USER_BOMB("can't get socket mode on non-socket", (vcsocketmode)-1);
}
vc
vc_default::socket_local_addr()
{
	USER_BOMB("can't get local addr from non-socket", vcnil);
}

vc
vc_default::socket_peer_addr()
{
	USER_BOMB("can't get peer addr from non-socket", vcnil);
}

void vc_default::socket_add_read_set() {USER_BOMB2("can't add non-socket to r/w set");}
void vc_default::socket_add_write_set() {USER_BOMB2("can't add non-socket to r/w set");}
void vc_default::socket_del_read_set() {USER_BOMB2("can't add non-socket to r/w set");}
void vc_default::socket_del_write_set() {USER_BOMB2("can't add non-socket to r/w set");}
vc 
vc_default::socket_recv_buf(vc& item, int to_get, const vc& recv_info, vc& addr_info)
{USER_BOMB("socket_recv_buf on non-socket", vcnil);}
vc 
vc_default::socket_send_buf(vc item, const vc& send_info, const vc&) 
{USER_BOMB("socket_send_buf on non-socket", vcnil);}

vc vc_default::socket_get_obj(int& avail, vc& addr_info) {USER_BOMB("get obj not defined for socket", vcnil);}
vc vc_default::socket_put_obj(vc obj, const vc& to_addr, int syntax) {USER_BOMB("put obj not defined for socket", vcnil);}
int vc_default::socket_get_write_q_size() {USER_BOMB("get-write-q not defined for socket", vcnil);}
int vc_default::socket_get_read_q_len() {USER_BOMB("get-read-q-len not defined for socket", -1);}

VC_ERR_CALLBACK
vc_default::set_err_callback(VC_ERR_CALLBACK) 
{
	oopanic("unimp error callback");
	/*NOTREACHED*/
	return 0;
}
int
vc_default::re_match(const vc&, int, int ) const {USER_BOMB("re-match not available with non-regex's",0);}
int
vc_default::re_match(const vc&) const {USER_BOMB("re-match not available with non-regex's",0);}
int
vc_default::re_match_info(int&, int&, int) const {USER_BOMB("re-match-info not available with non-regex's",0);}
int
vc_default::re_search(const vc&, int, int&, int) const {USER_BOMB("re-search not available with non-regex's",0);}

vc_default::operator int() const { USER_BOMB("unimp cast to int", 0); }
vc_default::operator double() const { USER_BOMB("unimp cast to double", 0.); }
vc_default::operator const char *() const { USER_BOMB("unimp cast to char *", ""); }
vc_default::operator long() const { USER_BOMB("unimp cast to long", 0); }
vc_default::operator char() const { USER_BOMB("unimp cast to char", 0); }
vc_default::operator void *() const { USER_BOMB("unimp cast to void *", 0); }
void
vc_default::stringrep(VcIO ) const {USER_BOMB2("unimp stringrep");}

void
vc_default::printOn(VcIO outputStream)
{
#ifdef OBJTRACK
outputStream << "<<unimp printon>>";
#else
USER_BOMB2("unimp printon");
#endif
}
vc
vc_default::force_eval() const {USER_BOMB("unimp forceeval", vcnil);}
vc
vc_default::eval() const {USER_BOMB("unimp eval", vcnil);}
void
vc_default::eval(vc& res) const {USER_BOMB2("unimp eval");}
//hashValueType vc_default::hashValue() const {USER_BOMB("unimp hashval", 0);}
hashValueType vc_default::hashValue() const {return (hashValueType)this;}

// for eq and ne, we have a default behavior of checking
// the exact memory location used by the representation.
// this is ok for a lot of things...
int
vc_default::operator ==(const vc& v) const {return this == v.rep;}
int
vc_default::operator !=(const vc& v) const {return this != v.rep;}

void
vc_default::foreach(const vc& , const vc& ) const 
{
	USER_BOMB2("can't iterate over that");
}

void
vc_default::foreach(const vc& , VC_FOREACH_CALLBACK ) const 
{
	USER_BOMB2("can't iterate over that");
}

void
vc_default::foreach(const vc& , VC_FOREACH_CALLBACK2 ) const
{
    USER_BOMB2("can't iterate over that");
}

#ifdef VCDBG
void
vc_default::set_break_on(int m)
{
	break_tag |= m;
}

void
vc_default::clear_break_on(int m)
{
	break_tag &= ~m;
}

int
vc_default::break_on(int m) const
{
	return (break_tag & m);
}

#endif

long
vc_default::overflow(vcxstream&, char *, long)
{
	oopanic("undefined overflow");
	/*NOTREACHED*/
	return -1;
}

long
vc_default::underflow(vcxstream&, char *, long, long)
{
	oopanic("undefined underflow");
	/*NOTREACHED*/
	return -1;
}

long
vc_default::devopen(vcxstream&, int, int)
{
	return 1;
}

long
vc_default::devclose(vcxstream&, int)
{
	return 1;
}

long
vc_default::xfer_out(vcxstream&)
{
	USER_BOMB("transfer out method not defined for type", -1);
}

long
vc_default::xfer_in(vcxstream&)
{
	USER_BOMB("transfer in method not defined for type", -1);
}

vc
vc_default::set_device(vc)
{
	USER_BOMB("can't set the device on that thing", vcnil);
}

vc
vc_default::remove_first2(vc&)
{
	USER_BOMB("can't remove_first2 on that", vcnil);
}

#define decl_arith(type) \
	vc vc_default::type##_add(const vc&) const { USER_BOMB("undefined " #type " add", vczero); } \
	vc vc_default::type##_sub(const vc&) const { USER_BOMB("undefined " #type " sub", vczero); }  \
	vc vc_default::type##_mul(const vc&) const { USER_BOMB("undefined " #type " mul", vczero); }  \
	vc vc_default::type##_div(const vc&) const { USER_BOMB("undefined " #type " div", vczero); }  \
	vc vc_default::type##_mod(const vc&) const { USER_BOMB("undefined " #type " mod", vczero); }  
#define decl_rel(type) \
	int vc_default::type##_lt(const vc&) const { USER_BOMB("undefined " #type " lt", 0); } \
	int vc_default::type##_le(const vc&) const { USER_BOMB("undefined " #type " le", 0); }  \
	int vc_default::type##_gt(const vc&) const { USER_BOMB("undefined " #type " gt", 0); }  \
	int vc_default::type##_ge(const vc&) const { USER_BOMB("undefined " #type " ge", 0); }   \
	int vc_default::type##_eq(const vc&) const { USER_BOMB("undefined " #type " eq", 0); }   \
	int vc_default::type##_ne(const vc&) const { USER_BOMB("undefined " #type " ne", 0); } 


decl_arith(int)
decl_arith(double)
decl_rel(int)
decl_rel(double)
decl_arith(vec)
decl_rel(str)

#undef decl_arith
#undef decl_rel

// functors 
vc vc_default::operator()(void) const {USER_BOMB("undefined functor", vcnil);}
//vc vc_default::operator()(void *p) const {USER_BOMB("undefined functor", vcnil);}
vc vc_default::operator()(VCArglist *al) const {USER_BOMB("undefined functor", vcnil);}

vc vc_default::operator()(vc v0) const {USER_BOMB("undefined functor", vcnil);}
vc vc_default::operator()(vc v0, vc v1) const {USER_BOMB("undefined functor", vcnil);}
vc vc_default::operator()(vc v0, vc v1, vc v2) const {USER_BOMB("undefined functor", vcnil);}
vc vc_default::operator()(vc v0, vc v1, vc v2, vc v3) const {USER_BOMB("undefined functor", vcnil);}

	// relationals
	 int vc_default::operator <(const vc &v) const {USER_BOMB("undefined op", 0);}
	 int vc_default::operator <=(const vc &v) const {USER_BOMB("undefined op", 0);}
	 int vc_default::operator >(const vc &v) const {USER_BOMB("undefined op", 0);}
	 int vc_default::operator >=(const vc &v) const {USER_BOMB("undefined op", 0);}
	 //int vc_default::operator ==(const vc &v) const {USER_BOMB("undefined op", 0);}
	 //int vc_default::operator !=(const vc &v) const {USER_BOMB("undefined op", 0);}

	// arithmetic ops
	 vc vc_default::operator+(const vc &v) const {USER_BOMB("undefined op", vczero);}
	 vc vc_default::operator-(const vc &v) const {USER_BOMB("undefined op", vczero);}
	 vc vc_default::operator*(const vc &v) const {USER_BOMB("undefined op", vczero);}
	 vc vc_default::operator/(const vc &v) const {USER_BOMB("undefined op", vczero);}
	 vc vc_default::operator%(const vc &v) const {USER_BOMB("undefined op", vczero);}

	// decomposable operations
int vc_default::is_empty() const {USER_BOMB("undefined decomposable", 1);}
int vc_default::num_elems() const {USER_BOMB("undefined decomposable", 0);}
vc& vc_default::operator[](const vc& v) {USER_BOMB("undefined decomposable", vcbitbucket);}
vc& vc_default::operator[](int i) {USER_BOMB("undefined decomposable", vcbitbucket);}
vc& vc_default::operator[](long i) {USER_BOMB("undefined decomposable", vcbitbucket);}
const vc& vc_default::operator[](const vc& v) const{USER_BOMB("undefined decomposable", vcbitbucket);}
const vc& vc_default::operator[](int) const{USER_BOMB("undefined decomposable", vcbitbucket);}
const vc& vc_default::operator[](long) const{USER_BOMB("undefined decomposable", vcbitbucket);}
int vc_default::contains(const vc& v) {USER_BOMB("undefined decomposable", 0);}
int vc_default::find(const vc& v, vc& out) {USER_BOMB("undefined decomposable", 0);}
void vc_default::add(const vc& v) {USER_BOMB2("undefined decomposable");}
void vc_default::add_kv(const vc& k, const vc& v) {USER_BOMB2("undefined decomposable");}
int vc_default::del(const vc& v) {USER_BOMB("undefined decomposable", 0);}
void vc_default::append(const vc& v) {USER_BOMB2("undefined decomposable");}
vc vc_default::remove_last() {USER_BOMB("undefined decomposable", vcnil);}
void vc_default::prepend(const vc& v) {USER_BOMB2("undefined decomposable");}
vc vc_default::remove_first() {USER_BOMB("undefined decomposable", vcnil);}
void vc_default::remove(const vc& strt, const vc& len) {USER_BOMB2("undefined decomposable");}
void vc_default::remove(const vc& strt, long len) {USER_BOMB2("undefined decomposable");}
void vc_default::remove(long strt, const vc& len) {USER_BOMB2("undefined decomposable");}
void vc_default::remove(long strt, long len) {USER_BOMB2("undefined decomposable");}
void vc_default::insert(const vc& item, const vc& idx) {USER_BOMB2("undefined decomposable");}
void vc_default::insert(const vc& item, long idx) {USER_BOMB2("undefined decomposable");}

int vc_default::func_eq(const vc& v) const  {USER_BOMB("undefined func eq", 0);}
int vc_default::set_eq(const vc& v) const {USER_BOMB("undefined set eq", 0);}
int vc_default::visited(){USER_BOMB("undefined visit", 0);}
void vc_default::set_visited(){USER_BOMB2("undefined set visit");}
void vc_default::operator<<(vc){USER_BOMB2("undefined op<<");}
#include "vcio.h"
vc vc_default::translate(VcIO o) const
{
    o << "\n#error failed translation\n";
    USER_BOMB("#error cant translate", vcnil);
}
