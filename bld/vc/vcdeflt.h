
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCDFLT_H
#define VCDFLT_H
// $Header: g:/dwight/repo/vc/rcs/vcdeflt.h 1.51 1998/08/06 04:45:08 dwight Exp $

#include "vc.h"

//
// This class is a place to hang default
// behavior for methods. It really should be
// an abstract class, but that makes debugging
// and extension hard because you have define
// a whole bunch of stuff that really isn't needed
// right away.
//
class vc_default 
: public vc
{
friend class vc;
public:
	long ref_count;
	vc_default();

	vc_default(const vc_default&);
#ifdef OBJTRACK
	virtual ~vc_default();
	int serial;
#else
	virtual ~vc_default() {}
#endif

	virtual vc_object *get_obj() const;
	virtual vc_fundef *get_fundef() const;
	
	virtual int member_select(const vc& member, vc& out, int toplev, vc_object *topobj);

	virtual int is_atomic() const ;
	virtual int is_varadic() const ;
	virtual int must_eval_args() const;
	
	virtual int is_quoted() const ;
    virtual int is_map_inhibited() const ;
	virtual void bind(const vc& v) const ;
	virtual void local_bind(const vc& v) const ;
	virtual void global_bind(const vc& v) const ;
	virtual void bremove() const ;
	virtual void local_bremove() const ;
	virtual void global_bremove() const ;
	virtual enum vc_type type() const ;
	virtual int is_nil() const ;
    virtual int is_decomposable() const ;
	virtual vc copy() const ;
	virtual vc get_special() const ;
	virtual const char *peek_str() const ;
	virtual long len() const;
	virtual vc_default *do_copy() const;
	virtual hashValueType hashValue() const ;
	
	virtual int open(const vc& name, vcfilemode m)  ;
	virtual int close()  ;
	virtual int seek(long pos, int whence)  ;
	virtual int read(void *buf, long& len)  ;
	virtual int write(const void *buf, long& len)  ;
	virtual int fgets(void *buf, long len);
    virtual int eof();
	
    // conversions, misc.
	virtual operator int() const ;
	virtual operator double() const ;
	virtual operator const char *() const ;
	virtual operator long() const ;
	virtual operator char() const ;
	virtual operator void *() const ;
    virtual operator long long() const;
    //virtual operator int64_t() const;

	// default relationals
	
	virtual void printOn(VcIO outputStream) ;

	virtual vc force_eval() const ;
	virtual vc eval() const ;

	virtual void stringrep(VcIO o) const ;
	
	virtual VC_ERR_CALLBACK set_err_callback(VC_ERR_CALLBACK);
	virtual vc socket_init(const vc& local_addr, int listen, int reuse, int syntax = 0) ;
	virtual vc socket_init(SOCKET os_handle, vc listenting, int syntax = 0) ;
	virtual vc socket_accept(vc& addr_info) ;
	virtual vc socket_connect(const vc& addr_info) ;
	virtual vc socket_close(int close_info) ;
	virtual vc socket_shutdown(int how);
	virtual int socket_poll(int what_for, long to_sec, long to_usec) ;
	virtual vc socket_send(vc item, const vc& send_info, const vc& to = vcnil) ;
	virtual vc socket_recv(vc& item, const vc& recv_info, vc& addr_info = vcbitbucket) ;
	virtual vc socket_send_raw(void *buf, long& len, long timeout, const vc& to = vcnil) ;
	virtual vc socket_recv_raw(void *buf, long& len, long timeout, vc& addr_info = vcbitbucket) ;
	virtual vc socket_error();
	virtual vc socket_error_desc();
	virtual void socket_set_error(vc v);
	virtual vc socket_set_option(vcsocketmode m, unsigned long = 0,
		unsigned long = 0, unsigned long = 0);
#ifdef _Windows
    virtual int socket_set_async(void *, unsigned int msg, long events);
#endif
	virtual vcsocketmode socket_get_mode();
	virtual vc socket_local_addr();
	virtual vc socket_peer_addr();
	virtual void socket_add_read_set();
	virtual void socket_add_write_set();
	virtual void socket_del_read_set();
	virtual void socket_del_write_set();
	virtual vc socket_recv_buf(vc& item, int to_get, const vc& recv_info, vc& addr_info);
	virtual vc socket_send_buf(vc item, const vc& send_info, const vc&);

    virtual vc socket_get_obj(int& avail, vc& addr_info);
    virtual vc socket_put_obj(vc obj, const vc& to_addr, int syntax);
    virtual int socket_get_write_q_size();
    virtual int socket_get_read_q_len();
	virtual long overflow(vcxstream&, char *buf, long len);
	virtual long underflow(vcxstream&, char *buf, long min, long max);
	virtual long devopen(vcxstream&, int style, int stat);
	virtual long devclose(vcxstream&, int style);
	virtual long xfer_out(vcxstream&);
	virtual long xfer_in(vcxstream&);
	 virtual int re_match(const vc& v, int len, int pos = 0) const ;
	 virtual int re_match(const vc& v) const ;
	 virtual int re_search(const vc& v, int len, int& matchlen, int pos = 0) const ;
	 virtual int re_match_info(int& strt, int& len, int nth = 0) const ;
	 
	virtual vc funmeta() const ;

#define decl_arith(type) \
	virtual vc type##_add(const vc&) const ; \
	virtual vc type##_sub(const vc&) const ;  \
	virtual vc type##_mul(const vc&) const ;  \
	virtual vc type##_div(const vc&) const ;  \
	virtual vc type##_mod(const vc&) const ;  
#define decl_rel(type) \
	virtual int type##_lt(const vc&) const ; \
	virtual int type##_le(const vc&) const ;  \
	virtual int type##_gt(const vc&) const ;  \
	virtual int type##_ge(const vc&) const ;   \
	virtual int type##_eq(const vc&) const ;   \
	virtual int type##_ne(const vc&) const ; 


decl_arith(int)
decl_arith(double)
decl_rel(int)
decl_rel(double)
decl_arith(vec)
decl_rel(str)

#undef decl_arith
#undef decl_rel
	// functors 
	virtual vc operator()(void) const ;
	//virtual vc operator()(void *p) const ;
	virtual vc operator()(VCArglist *al) const ;

	virtual vc operator()(vc v0) const ;
	virtual vc operator()(vc v0, vc v1) const ;
	virtual vc operator()(vc v0, vc v1, vc v2) const ;
	virtual vc operator()(vc v0, vc v1, vc v2, vc v3) const ;

	// relationals
	virtual int operator <(const vc &v) const ;
	virtual int operator <=(const vc &v) const ;
	virtual int operator >(const vc &v) const ;
	virtual int operator >=(const vc &v) const ;
	virtual int operator ==(const vc &v) const ;
	virtual int operator !=(const vc &v) const ;

	// arithmetic ops
	virtual vc operator+(const vc &v) const ;
	virtual vc operator-(const vc &v) const ;
	virtual vc operator*(const vc &v) const ;
	virtual vc operator/(const vc &v) const ;
	virtual vc operator%(const vc &v) const ;

	// decomposable operations
	virtual int is_empty() const ;
    virtual int num_elems() const ;
	virtual vc& operator[](const vc& v) ;
	virtual vc& operator[](int i) ;
	virtual vc& operator[](long i) ;
	virtual const vc& operator[](const vc& v) const;
	virtual const vc& operator[](int) const;
	virtual const vc& operator[](long) const;
    virtual int contains(const vc& v) const;
    virtual int find(const vc& v, vc& out) ;
	virtual void add(const vc& v) ;
	virtual void add_kv(const vc& k, const vc& v) ;
	virtual int del(const vc& v) ;
	virtual void append(const vc& v) ;
    virtual vc remove_last() ;
	virtual void prepend(const vc& v) ;
    virtual vc remove_first() ;
	virtual void remove(const vc& strt, const vc& len) ;
	virtual void remove(const vc& strt, long len = 1) ;
	virtual void remove(long strt, const vc& len) ;
	virtual void remove(long strt, long len = 1) ;
	virtual void insert(const vc& item, const vc& idx) ;
	virtual void insert(const vc& item, long idx) ;

	virtual void foreach(const vc& v, const vc& expr) const ;
	virtual void foreach(const vc& v, VC_FOREACH_CALLBACK) const ;
    virtual void foreach(const vc& v, VC_FOREACH_CALLBACK2) const ;
	
	virtual int func_eq(const vc& v) const  ;
	virtual int set_eq(const vc& v) const ;

	// internal control
	virtual int visited();
	virtual void set_visited();

	virtual void operator<<(vc);

    virtual vc translate(VcIO o) const;

#ifdef VCDBG
	int break_tag;
	virtual void set_break_on(int);
	virtual void clear_break_on(int);
	virtual int break_on(int) const;
#endif
	virtual vc set_device(vc);
	virtual vc remove_first2(vc&);

};


#endif
