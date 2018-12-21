
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VC_H
#define VC_H
// $Header: g:/dwight/repo/vc/rcs/vc.h 1.54 1998/12/14 05:10:41 dwight Exp $

#ifdef VCCFG_FILE
#include "vccfg.h"
#endif

#ifdef _MSC_VER
#ifndef _Windows
#define _Windows
#endif
#endif

#ifdef _Windows
#define USE_WINSOCK
#else
#define USE_BERKSOCK
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define dwmax(x, y) (((x) < (y)) ? (y) : (x))
#define dwmin(x, y) (((x) > (y)) ? (y) : (x))
#include "useful.h"
#include "vcfext.h"
class vcxstream;
#define EXIN_DEV -1
#define EXIN_PARSE -2
class VcLexer;

#define FALSE 0
#define TRUE 1

void oopanic(const char *);
void user_panic(const char *);
void user_warning(const char *);
class VcIOHack;

typedef VcIOHack& VcIO;

extern VcIOHack VcError;
extern VcIOHack VcOutput;
extern VcIOHack VcInput;

struct VcNoinit {VcNoinit() {}};

// DO NOT JACK WITH THIS ENUM
// THESE NUMBERS ARE ENCODED IN STREAMS
enum vc_type {
	VC_INT = 1,
	VC_STRING = 2,
	VC_DOUBLE = 3,
	VC_NIL = 4,
	VC_CVAR = 5,
	VC_FUNC = 6,
	VC_SET = 7,
	VC_MAP = 8,
	VC_VECTOR = 9,
	VC_LIST = 10,
	VC_BAG = 11,
	VC_REGEX = 12,
	VC_FILE = 13,
    VC_SOCKET = 14,
	VC_SOCKET_STREAM = 15,
	VC_SOCKET_DGRAM = 16,
	VC_CHIT = 17,
	VC_OBJECT = 18,
	VC_MEMSEL = 19,
	VC_MEMFUN = 20,
	VC_HOMOVCHAR = 21,
	VC_HOMOVLONG = 22,
	VC_HOMOVFLOAT = 23,
	VC_HOMOVDOUBLE = 24,
	VC_RECORD = 25,
	VC_MEMBUF = 26,
	VC_BSTRING = 27,
	VC_FILTER = 28,
	VC_SOCKET_STREAM_UNIX = 29,
	VC_INT_DTOR = 30,
	VC_TREE = 31,
	// note: these are never encoded, they are
	// just hacks to allow us to send extra
	// parameters to the VC_SET and VC_BAG
	// constructors. the resulting types are
	// VC_SET and VC_BAG (likewise for VC_BSTRING,
	// the resulting type is VC_STRING)
	VC_SET_N = 32,
    VC_BAG_N = 33,
    VC_UVSOCKET_STREAM = 34,
    VC_UVSOCKET_DGRAM = 35
};

// note: the new gcc's are too pissy about
// enums these days, so just get rid of enum vcfilemode.
typedef int vcfilemode;
#define VCFILE_READ 1
#define VCFILE_WRITE 2
#define VCFILE_APPEND 4
#define VCFILE_BINARY 8

enum vcerrormode {EXCEPTIONS, RETURN_CODES};

// socket stuff
// roughly speaking, this is a glom of socket and fd options.
// not really any rhyme or reason here
enum vcsocketmode {
	VC_BLOCKING,
	VC_NONBLOCKING,
	VC_WSAASYNC_SELECT,
	VC_CLOSE_ON_EXEC,
	VC_NO_CLOSE_ON_EXEC,
	VC_SET_RECV_BUF_SIZE,
	VC_GET_RECV_BUF_SIZE,
	VC_SET_SEND_BUF_SIZE,
	VC_GET_SEND_BUF_SIZE,
	VC_SET_TCP_NO_DELAY,
	VC_BUFFER_SOCK
};
#define  VC_SOCK_ERROR 4
#define  VC_SOCK_READ  2
#define  VC_SOCK_WRITE 1
#define  VC_SOCK_READ_NOT_WRITE 8
#define  VC_SOCK_WRITE_NOT_READ 16
#define VC_SOCK_SHUTDOWN_RD 0
#define VC_SOCK_SHUTDOWN_WR 1
#define VC_SOCK_SHUTDOWN_BOTH 2
const char *vc_wsget_errstr(int);

#ifdef _Windows
// can't do this, it causes compile errors, sigh.
//#include <winsock.h>
typedef unsigned int SOCKET;
#else
typedef int SOCKET;
#endif

#if 0
#ifdef USE_BERKSOCK
#include "vcberk.h"
#endif
#endif

typedef unsigned long hashValueType;

class vc;
class vc_default;
class vc_object;
class vc_fundef;

#ifdef __GNUG__
#include "dwvec.h"
#else
template<class T> class DwVec;
#endif

//typedef DwVec<vc> VCArglist;
#include "dwsvec.h"
typedef DwSVec<vc> VCArglist;

typedef vc (*VCFUNCP0)();
typedef vc (*VCFUNCP1)(vc);
typedef vc (*VCFUNCP2)(vc, vc);
typedef vc (*VCFUNCP3)(vc, vc, vc);
typedef vc (*VCFUNCP4)(vc, vc, vc, vc);
typedef vc (*VCFUNCP5)(vc, vc, vc, vc, vc);
typedef vc (*VCFUNCPv)(VCArglist *);
typedef vc (*VCFUNCPVP)(void *);

typedef vc (*VCTRANSFUNCP)(VCArglist *, VcIO);

typedef int (*VC_ERR_CALLBACK)(vc *);
typedef void (*VC_FOREACH_CALLBACK)(vc);
typedef void (*VC_FOREACH_CALLBACK2)(vc, vc);

extern const vc vcnil;
extern const vc vctrue;
extern const vc vcone;
extern const vc vczero;
extern const vc vcfzero;
extern vc vcbitbucket;

#ifdef VCDBG
extern int Eval_break;
#endif

class vc
{
friend class vc_default;
private:
// NOTE NOTE NOTE VERY IMPORTANT!
// the TOP level vc has NO virtual functions.
// this makes using them a lot faster. vc_default
// does the dispatching and that is where the "virtuals"
// are.
#define notvirtual
	vc_default *rep;
	
	void vc_init(enum vc_type, const char *, long extra_parm);
	// this is an awful hack, should get rid of it.
friend class vc_memberfun;
friend class vc_object;
	notvirtual vc_object *get_obj() const;
	notvirtual vc_fundef *get_fundef() const;
	
public:
	// more awful hacks, though useful when not using
	// full hierarchy...
	vc_default *nonono() {return rep;}
	notvirtual int is_atomic() const ;
	notvirtual int is_varadic() const ;
	notvirtual int must_eval_args() const;
	
private:
	static void init_rest(void);
public:
	static void init(void);
	static void thread_init(void);
	static void exit(void);
	static void thread_exit(void);
	static void non_lh_init(void);
	static void non_lh_exit(void);
	static void setup_logs(void);
	static void shutdown_logs(void);

        static vc set_to_vector(vc set);
        static vc map_to_vector(vc map);

    // these are needed to support the lazy-Qmap stuff
	void *operator new(size_t, vc *v) {return v;}
	void *operator new(size_t s) {return ::operator new(s);}

	notvirtual void flush_cache() {}

#ifndef VC_DBG_ENVELOPE
	inline vc() ;
	inline vc(const vc& v);
	inline notvirtual ~vc() ;
	inline notvirtual vc& operator=(const vc& v);
#if !defined(_MSC_VER) || _MSC_VER != 1500
        inline vc(vc&& v);
        inline vc& operator=(vc&& v);
#endif
#else
	vc() ;
	vc(const vc& v);
	notvirtual ~vc() ;
	notvirtual vc& operator=(const vc& v);
#if !defined(_MSC_VER) || _MSC_VER != 1500
        vc(vc&& v);
        vc& operator=(vc&& v);
#endif

#endif
	vc(double d);
	vc(int i);
	vc(long i);
	vc(const char *s);
        vc(const char *s, int len);
	vc(VcLexer&);
	vc(enum vc_type, const char * = "nil", long extra_parm = 0);
    vc(VCFUNCP0, const char *, const char *, int style = VC_FUNC_NORMAL, VCTRANSFUNCP = 0);
    vc(VCFUNCP1, const char *, const char *, int style = VC_FUNC_NORMAL, VCTRANSFUNCP = 0);
    vc(VCFUNCP2, const char *, const char *, int style = VC_FUNC_NORMAL, VCTRANSFUNCP = 0);
    vc(VCFUNCP3, const char *, const char *, int style = VC_FUNC_NORMAL, VCTRANSFUNCP = 0);
    vc(VCFUNCP4, const char *, const char *, int style = VC_FUNC_NORMAL, VCTRANSFUNCP = 0);
    vc(VCFUNCP5, const char *, const char *, int style = VC_FUNC_NORMAL, VCTRANSFUNCP = 0);
    vc(VCFUNCPVP, const char *, const char *, int style = VC_FUNC_NORMAL, VCTRANSFUNCP = 0);
    vc(VCFUNCPv,const char *, const char *, int style = VC_FUNC_NORMAL|VC_FUNC_VARADIC, VCTRANSFUNCP = 0);

	notvirtual long xfer_in(vcxstream&);
	notvirtual long xfer_out(vcxstream&);
	notvirtual long overflow(vcxstream&, char *buf, long len);
	notvirtual long underflow(vcxstream&, char *buf, long min, long max);
	notvirtual long devopen(vcxstream&, int style, int stat);
	notvirtual long devclose(vcxstream&, int style);
private:
    notvirtual long real_xfer_in(vcxstream&);
public:


	notvirtual int is_quoted() const ;
    notvirtual int is_map_inhibited() const ;
	notvirtual void bind(const vc& v) const ;
	notvirtual void local_bind(const vc& v) const ;
	notvirtual void global_bind(const vc& v) const ;
	notvirtual void bremove() const ;
	notvirtual void local_bremove() const ;
	notvirtual void global_bremove() const ;
	notvirtual enum vc_type type() const ;
	notvirtual int is_nil() const ;
    notvirtual int is_decomposable() const ;
	notvirtual vc copy() const ;
	notvirtual vc get_special() const ;
	notvirtual const char *peek_str() const ;
	notvirtual long len() const;

	// relationals
	notvirtual int operator <(const vc &v) const ;
	notvirtual int operator <=(const vc &v) const ;
	notvirtual int operator >(const vc &v) const ;
	notvirtual int operator >=(const vc &v) const ;
	notvirtual int operator ==(const vc &v) const ;
	notvirtual int operator !=(const vc &v) const ;

	// arithmetic ops
	notvirtual vc operator+(const vc &v) const ;
	notvirtual vc operator-(const vc &v) const ;
	notvirtual vc operator*(const vc &v) const ;
	notvirtual vc operator/(const vc &v) const ;
	notvirtual vc operator%(const vc &v) const ;

	//
	// meta-data ops
	//
	notvirtual vc funmeta() const ;

	// functors 
	notvirtual vc operator()(void) const ;
	//notvirtual vc operator()(void *p) const ;
	notvirtual vc operator()(VCArglist *al) const ;

	notvirtual vc operator()(vc v0) const ;
	notvirtual vc operator()(vc v0, vc v1) const ;
	notvirtual vc operator()(vc v0, vc v1, vc v2) const ;
	notvirtual vc operator()(vc v0, vc v1, vc v2, vc v3) const ;

	// decomposable operations
	notvirtual int is_empty() const ;
    notvirtual int num_elems() const ;
	notvirtual vc& operator[](const vc& v) ;
	notvirtual vc& operator[](int i) ;
	notvirtual vc& operator[](long i) ;
	notvirtual const vc& operator[](const vc& v) const;
	notvirtual const vc& operator[](int) const;
	notvirtual const vc& operator[](long) const;
	notvirtual int contains(const vc& v) ;
    notvirtual int find(const vc& v, vc& out) ;
	notvirtual void add(const vc& v) ;
	notvirtual void add_kv(const vc& k, const vc& v) ;
	notvirtual int del(const vc& v) ;
	notvirtual void append(const vc& v) ;
    notvirtual vc remove_last() ;
	notvirtual void prepend(const vc& v) ;
    notvirtual vc remove_first() ;
    notvirtual vc remove_first2(vc& key_out) ;
	notvirtual void remove(const vc& strt, const vc& len) ;
	notvirtual void remove(const vc& strt, long len = 1) ;
	notvirtual void remove(long strt, const vc& len) ;
	notvirtual void remove(long strt, long len = 1) ;
	notvirtual void insert(const vc& item, const vc& idx) ;
	notvirtual void insert(const vc& item, long idx) ;

// someone at nokia/trolltech decided to define a macro
// with the name "foreach", wtf.
#undef foreach
	notvirtual void foreach(const vc& v, const vc& expr) const ;
	notvirtual void foreach(const vc& v, VC_FOREACH_CALLBACK cb) const ;
    notvirtual void foreach(const vc& v, VC_FOREACH_CALLBACK2 cb) const ;
	
	notvirtual VC_ERR_CALLBACK set_err_callback(VC_ERR_CALLBACK);
	
	// socket ops
	notvirtual vc socket_init(const vc& local_addr, int listen, int reuse, int syntax = 0) ;
	notvirtual vc socket_init(SOCKET os_handle, vc listening, int syntax = 0) ;
	notvirtual vc socket_accept(vc& addr_info) ;
	notvirtual vc socket_connect(const vc& addr_info) ;
	notvirtual vc socket_close(int close_info) ;
	notvirtual vc socket_shutdown(int how);
	notvirtual int socket_poll(int what_for, long to_sec, long to_usec) ;
	notvirtual vc socket_send(vc item, const vc& send_info, const vc& to = vcnil) ;
	notvirtual vc socket_recv(vc& item, const vc& recv_info, vc& addr_info = vcbitbucket) ;
	notvirtual vc socket_send_raw(void *buf, long& len, long timeout, const vc& to = vcnil) ;
	notvirtual vc socket_recv_raw(void *buf, long& len, long timeout, vc& addr_info = vcbitbucket) ;
	notvirtual vc socket_error();
	notvirtual vc socket_error_desc();
	notvirtual void socket_set_error(vc v);
	notvirtual vc socket_set_option(vcsocketmode m, unsigned long = 0,
		unsigned long = 0, unsigned long = 0);
	notvirtual vcsocketmode socket_get_mode();
	notvirtual vc socket_local_addr();
	notvirtual vc socket_peer_addr();
	notvirtual void socket_add_read_set();
	notvirtual void socket_add_write_set();
	notvirtual void socket_del_read_set();
	notvirtual void socket_del_write_set();
	notvirtual vc socket_recv_buf(vc& item, int to_get, const vc& recv_info, vc& addr_info = vcbitbucket);
	notvirtual vc socket_send_buf(vc item, const vc& send_info, const vc& to = vcnil);

    notvirtual vc socket_get_obj(int& avail, vc& addr_info);
    notvirtual vc socket_put_obj(vc obj, const vc& to_addr, int syntax);
    notvirtual int socket_get_write_q_size();
    notvirtual int socket_get_read_q_len();
	// some arithmetics
#define decl_arith(type) \
	notvirtual vc type##_add(const vc&) const ; \
	notvirtual vc type##_sub(const vc&) const ; \
	notvirtual vc type##_mul(const vc&) const ; \
	notvirtual vc type##_div(const vc&) const ; \
	notvirtual vc type##_mod(const vc&) const ; 
#define decl_rel(type) \
	notvirtual int type##_lt(const vc&) const ; \
	notvirtual int type##_le(const vc&) const ; \
	notvirtual int type##_gt(const vc&) const ; \
	notvirtual int type##_ge(const vc&) const ;  \
	notvirtual int type##_eq(const vc&) const ;  \
	notvirtual int type##_ne(const vc&) const ;

decl_arith(int)
decl_arith(double)
decl_rel(int)
decl_rel(double)
decl_arith(vec)
decl_rel(str)

#undef decl_arith
#undef decl_rel

	notvirtual int func_eq(const vc& v) const  ;
	notvirtual int set_eq(const vc& v) const ;

	// internal control
	void redefine(vc_default *v);
	void attach(vc_default *v);
	notvirtual int visited();
	notvirtual void set_visited();

	// regular expression matching

	notvirtual int re_match(const vc& v, int len, int pos = 0) const ;
	notvirtual int re_match(const vc& v) const ;
	notvirtual int re_search(const vc& v, int len, int& matchlen, int pos = 0) const ;
	notvirtual int re_match_info(int& strt, int& len, int nth = 0) const ;

	// File I/O
	notvirtual int open(const vc& name, vcfilemode m)  ;
	notvirtual int close()  ;
	notvirtual int seek(long pos, int whence)  ;
	notvirtual int read(void *buf, long& len)  ;
	notvirtual int write(const void *buf, long& len)  ;
	notvirtual int fgets(void *buf, long len) ;
    notvirtual int eof();

	// objects
	notvirtual int member_select(const vc& member, vc& out, int toplev, vc_object *topobj);

    // conversions, misc.
	notvirtual operator int() const ;
	notvirtual operator double() const ;
	notvirtual operator const char *() const ;
	notvirtual operator long() const ;
	notvirtual operator char() const ;
	notvirtual operator void *() const;
	
	void print_top(VcIO o);
	void print(VcIO o);
	notvirtual void printOn(VcIO outputStream) ;
	
	notvirtual void operator<<(vc);

	notvirtual hashValueType hashValue() const ;

	notvirtual vc force_eval() const ;
	notvirtual vc eval() const ;

	notvirtual void stringrep(VcIO o) const ;

    notvirtual vc translate(VcIO o) const;

	// debugging
#ifdef VCDBG
#define BREAK_ENABLE (1)
#define BREAK_DISABLE (2)

#define BREAK_NONE 0
#define BREAK_ACCESS 1
#define BREAK_CALL 2
#define BREAK_BIND 4
#define BREAK_MODIFY 8
#define BREAK_EVAL 16
#define BREAK_MEMCALL 32
#define BREAK_ALL (BREAK_ACCESS|BREAK_CALL|BREAK_BIND|BREAK_MODIFY|BREAK_EVAL|BREAK_MEMCALL)
  static void set_global_break_on(int);
  static void clear_global_break_on(int);
	notvirtual void set_break_on(int);
	notvirtual void clear_break_on(int);
	notvirtual int break_on(int) const;
#endif

	// device and filtering
	notvirtual vc set_device(vc);
	

#undef notvirtual
};

#ifndef VC_DBG_ENVELOPE
#include "vcdeflt.h"
#include "vcnil.h"
#ifdef USE_RCT
#include "rct.h"
#endif

inline
vc::vc()
{
	rep = vc_nil::vcnilrep;
}


inline
vc::vc(const vc& v)
{
	// note: only need this first case if you allow
	// copy constructors for classes derived from vc.
	// if you can avoid this, then you can remove the following
	// if(..)
//	if(v.rep == 0)
//	{
//		rep = 0;
//		return;
//	}
#ifdef USE_RCT
RCQINC(v.rep)
#else
	++v.rep->ref_count;
#endif
	rep = v.rep;
}

inline
vc::~vc()
{
        if(rep == vc_nil::vcnilrep) // base destruct
		return;
	// ignore nil destructs, see comment in vcnil.cpp
#ifdef USE_RCT
RCQDEC(rep)
#else
        if(--rep->ref_count == 0)
	{
		delete rep;
	}
#endif
}

inline
vc&
vc::operator=(const vc& v)
{
	if(&v != this)
	{
#ifdef USE_RCT
RCQINC(v.rep)
RCQDEC(rep)
#else
		++v.rep->ref_count;
		if(rep != vc_nil::vcnilrep && --rep->ref_count == 0) 
		{
			delete rep;
		}
#endif
		rep = v.rep;
	}
	return *this;
}

#if !defined(_MSC_VER) || _MSC_VER != 1500
inline
vc::vc(vc&& v)
{
    rep = v.rep;
    v.rep = vc_nil::vcnilrep;
}

inline
vc&
vc::operator=(vc&& v)
{
    if(this != &v)
    {
    vc_default *tmp = rep;
    rep = v.rep;
    v.rep = tmp;

    }
    return *this;
}
#endif

#endif
#endif
