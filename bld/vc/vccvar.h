
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCCVAR_H
#define VCCVAR_H
// $Header: g:/dwight/repo/vc/rcs/vccvar.h 1.47 1997/10/05 17:27:06 dwight Stable $
#include "vc.h"
#include "vccomp.h"
#include "dwlista.h"
#include "vcsrc.h"

#include "vclex.h"

/* grammar for lingua hacka exprs

 var -> atom |
	< var_list > |
	var ( var_list ) |
	var [ var ] |
	{ var_list } |
	` var_list '

	not used: var ^ var

var_list -> var_list var |
		var |
		empty


note: grammar is ambiguous for funcall
and member selection. there is no
precedence, and ops are evaluated in
a strict left to right sequence (left
associative.)

----------
hacked grammar used for r-d parsing:

var_list -> var tail | empty
tail -> var_list | empty

var -> { var_list } v' |
	< var_list > v' |
	` var_list ' v' |
	atom v'

v' -> ( var_list ) v' |
		[ var ] v' |
		empty

note: start symbol is "var_list", which is
reasonable since it would be pointless to define
var's that weren't evaluated. unfortunately, this
leads to a problem where the first set of <> are
implicit, and when converting back and forth to
stringrep results in more levels of <> showing up.
the extra <> level is removed by having a flag
that indicates the root of the cvar tree, and we
don't print the implicit <>. changing the start symbol
to "var", allows vars that
are essentially the same as their atomic counterparts...
not a big deal, but not pretty either. it also complicates
the implementation of cvar's, since then the root would have
to have another vc that was used specifically for holding the
root of the vc-tree.
*/                              

//                                 
// composite variable
//

typedef DwListA<vc> VCList;
typedef DwListAIter<vc> VCListIter;
	                             
class vc_cvar : public vc_composite
{
friend class VcEvalDbgNode;
friend long dovcinput(char *, long);
friend vc doreadatoms(vc);

private:
	vc_cvar();
	vc_cvar(const vc_cvar& a);

	VcLexer::Atom atom_type;
	VcLexer::Token tok;
	long toklen;
	const char *tokval;
	VcLexer *lexer;

	static int error;			// set to true on syntax error

	VCList vc_list;	// list sub-vc's (including atoms, vars, and composites)
    int is_root;	// non-zero means vc is root, used to avoid extra <> at stringrep time
    int exp_error;	// non-zero if error detected in parsing

	// source coordinates for the expression
	vc_cvar_src_coord begin_scoord;
	vc_cvar_src_coord end_scoord;
	
	// lexer functions
	void next_tok(void);

	// recursive decent parsing functions
	void varlist(VCList *);
	vc pvar(void);
#ifdef OLD_PARSE
	void tail(VCList *);
#endif
	vc vprime(vc);

	vc make_atom();

	void dbg_print(int expr_num, const char *var_name) const;
	void syntax_err(const char *msg);
	void syntax_err(const char *msg, vc_cvar_src_coord, vc_cvar_src_coord);
	void raise_compile_error();

#ifdef PERFHACKS
	int performance_hack(vc&) const;
	mutable int nopf;
	mutable vc cached_atom;
	mutable int use_cached_atom;
#endif
#ifdef CACHE_LOOKUPS
	mutable vc *range;
	mutable unsigned long time_cached;
	
	static unsigned long Lookup_cache_counter;

public:
	static void flush_lookup_cache();
private:
#endif

	void bind(const vc& v) const ;
	void local_bind(const vc& v) const ;
	void global_bind(const vc& v) const ;
	void bremove() const ;
	void local_bremove() const ;
	void global_bremove() const ;
	vc eval() const;
	virtual vc_default *do_copy() const ;

public:
	vc_cvar(const char *expr, int decrypt = 0);
	vc_cvar(VcLexer *);
	~vc_cvar();
	operator double() const ;
	operator int() const ;
	operator long() const ;
	operator const char *() const ;

	const char *peek_str() const ;
	virtual void stringrep(VcIO) const;
	
	virtual vc operator+(const vc &v) const ;
	virtual vc operator-(const vc &v) const ;
	virtual vc operator*(const vc &v) const ;
	virtual vc operator/(const vc &v) const ;
	virtual vc operator%(const vc &v) const ;

	vc int_add(const vc& v) const ;
	vc int_sub(const vc& v) const ;
	vc int_mul(const vc& v) const ;
	vc int_div(const vc& v) const ;
	vc int_mod(const vc& v) const ;

	vc double_add(const vc& v) const ;
	vc double_sub(const vc& v) const ;
	vc double_mul(const vc& v) const ;
	vc double_div(const vc& v) const ;
	vc double_mod(const vc& v) const ;

	int int_lt(const vc& v) const ;
	int int_le(const vc& v) const ;
	int int_gt(const vc& v) const ;
	int int_ge(const vc& v) const ;
	int int_eq(const vc& v) const ;
	int int_ne(const vc& v) const ;

	int double_lt(const vc& v) const ;
	int double_le(const vc& v) const ;
	int double_gt(const vc& v) const ;
	int double_ge(const vc& v) const ;
	int double_eq(const vc& v) const ;
	int double_ne(const vc& v) const ;

	//vc operator()(void *p) const ;
	vc operator()(VCArglist *a) const ;

	vc operator()() const ;
	vc operator()(vc v0) const ;
	vc operator()(vc v0, vc v1) const ;
	vc operator()(vc v0, vc v1, vc v2) const ;
	vc operator()(vc v0, vc v1, vc v2, vc v3) const ;
	
	int func_eq(const vc& v) const ;

	enum vc_type type() const ;
	virtual int is_nil() const ;

	virtual int operator <(const vc &v) const ;
	virtual int operator <=(const vc &v) const ;
	virtual int operator >(const vc &v) const ;
	virtual int operator >=(const vc &v) const ;
	virtual int operator ==(const vc &v) const ;
	virtual int operator !=(const vc &v) const ;

	//virtual VcIO operator<<(VcIO os) ;

	hashValueType hashValue() const ;
	void printOn(VcIO outputStream) ;
    vc translate(VcIO) const;

};
#endif
