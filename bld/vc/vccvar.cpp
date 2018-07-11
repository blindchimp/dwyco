
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifndef NO_VCEVAL
#include "vcdeflt.h"
#include "vccomp.h"
#include "vccvar.h"
#include "vcmap.h"
#include "vcfuncal.h"
#include "vcmemsel.h"
#include "dwdate.h"
#include "vcio.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vccvar.cpp 1.52 1998/10/26 02:48:42 dwight Exp $";

void dbg_print_date();
#ifdef VCDBG
#include "vcdbg.h"

class VcEvalDbgNode : public VcDebugNode
{
public:
	VcEvalDbgNode(const vc_cvar *);

	const vc_cvar *node;
	int expr_num;
	const char *var_name;

	virtual void printOn(VcIO) ;
};

VcEvalDbgNode::VcEvalDbgNode(const vc_cvar *v)
{
    linenum = v->begin_scoord.linenum;
    filename = v->begin_scoord.filename;
	node = v;
	expr_num = -1;
	var_name = "";
}

void
VcEvalDbgNode::printOn(VcIO os) 
{
	VcDebugNode::printOn(os);
	os << "Evaluating expression that began near line " <<
		node->begin_scoord.filename << ":" << node->begin_scoord.linenum << "\n";
	os << "Expr num = " << expr_num << ", Accumulated result = \"" << var_name << "\"\n";
	os << "\n";

}

#endif


//
// variable evaluation and creation
//
#include "vclex.h"
#define EOS VcLexer::EOS
#define LTICK VcLexer::LTICK
#define RTICK VcLexer::RTICK
#define RBRACE VcLexer::RBRACE
#define LBRACE VcLexer::LBRACE
#define LPAREN VcLexer::LPAREN
#define RPAREN VcLexer::RPAREN
#define LBRACKET VcLexer::LBRACKET
#define RBRACKET VcLexer::RBRACKET
#define ATOM VcLexer::ATOM
#define UPARROW VcLexer::UPARROW
#define LSBRACK VcLexer::LSBRACK
#define RSBRACK VcLexer::RSBRACK

int vc_cvar::error;

#ifdef CACHE_LOOKUPS

unsigned long vc_cvar::Lookup_cache_counter;
void
vc_cvar::flush_lookup_cache()
{
	if(++Lookup_cache_counter == 0)
		oopanic("lookup cache wraparound");
}

#endif

void
vc_cvar::raise_compile_error()
{
	vc exc("E:LH.COMPILE_ERROR");
	VCArglist a;
    a.append(exc);
    a.append(vc(lexer->input_description()));
	Vcmap->excraise(exc, &a);
}

vc_cvar::vc_cvar(const char *expr, int decrypt)
{
#ifdef PERFHACKS
	nopf = 0;
	use_cached_atom = 0;
#endif
#ifdef CACHE_LOOKUPS
	range = 0;
	time_cached = 0;
#endif
	// note: have to do this cuz lexer does decryption in
	// place. also, gives us a chance to wipe the
	// results of the decryption.
	int len;
	if(decrypt < 0)
	{
		len = -decrypt;
		decrypt = 1;
	}
	else
	{
		len = decrypt;
		decrypt = 0;
	}
	char *e1 = new char[len + 1];
	memcpy(e1, expr, len);
	e1[len] = 0;
	if(decrypt)
		lexer = new VcLexerStringEncrypted(e1, len, VcError);
	else
		lexer = new VcLexerString(e1, len, VcError);
	// this may be a call via "load-file", in which
	// case, try to set the filename up properly.
	vc fn = Vcmap->global_find(vc("__lh_loading_file"));
	if(!fn.is_nil())
	{
		lexer->set_input_description(fn);
	}
	tok = lexer->next_token(tokval, toklen, atom_type);
	begin_scoord.init(lexer);
	error = 0;
	varlist(&vc_list);
	memset(e1, 0, len);
	delete [] e1;
	is_root = 1;
	//dont_map = 1;
	quoted = 1;
	if(tok != EOS)
		syntax_err("trailing garbage after variable expression");
	exp_error = error;
	end_scoord.init(lexer);
	if(exp_error)
		raise_compile_error();
	delete lexer;
}

vc_cvar::vc_cvar(VcLexer *l)
{
#ifdef PERFHACKS
	nopf = 0;
	use_cached_atom = 0;
#endif
#ifdef CACHE_LOOKUPS
	range = 0;
	time_cached = 0;
#endif
	lexer = l;
	tok = lexer->next_token(tokval, toklen, atom_type);
	begin_scoord.init(lexer);
	error = 0;
	varlist(&vc_list);
	is_root = 1;
	//dont_map = 1;
	quoted = 1;
	if(tok != EOS)
		syntax_err("trailing garbage after variable expression");
	exp_error = error;
	end_scoord.init(lexer);
	if(exp_error)
		raise_compile_error();
}


vc_cvar::~vc_cvar()
{
}

#ifdef PERFHACKS
// this hack short-cuts a lot of things for the common
// case where there is a simple variable lookup 
//
int
vc_cvar::performance_hack(vc& atom) const
{
	if(vc_list.num_elems() != 1)
	{
		((vc_cvar *)this)->nopf = 1;
		return 0;
	}
	vc tmp = vc_list.get_first();
	if(tmp.type() != VC_STRING)
	{
		((vc_cvar *)this)->nopf = 1;
		return 0;
	}
	((vc_cvar *)this)->cached_atom = tmp;
	((vc_cvar *)this)->use_cached_atom = 1;
	if(dont_map)
	{
		atom = tmp;
		return 1;
	}
#ifdef CACHE_LOOKUPS
	if(range && time_cached >= Lookup_cache_counter)
		atom = *range;
	else
	{
		atom = Vcmap->get2(tmp, range);
		time_cached = Lookup_cache_counter;
	}
#else
	atom = Vcmap->get(tmp);
#endif
	return 1;
}

#endif

void
vc_cvar::eval(vc& res) const
{
    res = eval();
}

vc
vc_cvar::eval() const
{
	if(exp_error)
		USER_BOMB("attempt to eval erroneous expression", vcnil);


#ifdef PERFHACKS
	if(!nopf)
	{
		if(use_cached_atom)
		{
			if(dont_map)
				return cached_atom;
#ifdef CACHE_LOOKUPS
			if(range && time_cached >= Lookup_cache_counter)
				return *range;
			else
			{
				time_cached = Lookup_cache_counter;
				return Vcmap->get2(cached_atom, range);
			}
#else
			return Vcmap->get(cached_atom);
#endif
		}
		vc atom;
		if(performance_hack(atom))
			return atom;
	}
#endif

	int expr_num = 0;
	char *var_name = 0;
	long curlen = 0;
	long bufsize = 128;
	if(!quoted)
	{
		var_name = new char[128];
		var_name[0] = '\0';
	}

#ifdef VCDBG
	VcEvalDbgNode dbg(this);
    if(Eval_break)
      {
        if(drop_to_dbg("eval break", "evalbreak"))
		{
			delete [] var_name;
          return vcnil;
		 }
      }
        
#endif

	VCListIter i(&vc_list);
	vc atom;
	dwlista_foreach_iter(i, atom, vc_list)
	{
		vc val;
#ifdef VCDBG
		dbg.expr_num = expr_num;
		dbg.var_name = var_name;
#endif
			val = atom.eval();
			// special case: we're unwinding due to a return/break/exc
			// action. The various return values are stashed in the
            // function context, so we abandon it here.
			if(Vcmap->unwind_in_progress())
			{
				if(Vcmap->dbg_backout_in_progress())
					dbg_print(expr_num, var_name);
				delete [] var_name;
				return vcnil;
			}	
			atom = val;

		if(!quoted)
		{
			const char *str = val.peek_str();
			int len = val.len();
			if(len + curlen >= bufsize - 1)
			{
				bufsize += dwmax(512, len + 1);
				char *new_var_name = new char[bufsize];
                memcpy(new_var_name, var_name, curlen);
				delete [] var_name;
				var_name = new_var_name;
			}
            memcpy(var_name + curlen, str, len);
			curlen += len;
            // null terminate for compatibility
            var_name[curlen] = '\0';
		}
		++expr_num;
	}
	vc vn;
	if(!quoted)
	{
		// special case: the var name "" (zero length name) always maps
		// to nil.
		if(curlen != 0 || dont_map /*var_name[0] != '\0'*/)
			vn = vc(VC_BSTRING, var_name, curlen);
	}


	if(dont_map) // note: quoted -> no mapping too
	{
		delete [] var_name;
		if(quoted)
			return atom; // last expr in the expr list
		return vn;
    }
	else
	{
		delete [] var_name;
		return Vcmap->get(vn);
	}
}


void
vc_cvar::next_tok()
{

	tok = lexer->next_token(tokval, toklen, atom_type);

}

void
vc_cvar::syntax_err(const char *msg)
{
	vc_cvar_src_coord s;
	s.init(lexer);
	syntax_err(msg, s, s);
}

void
vc_cvar::syntax_err(const char *msg, vc_cvar_src_coord start, vc_cvar_src_coord end)
{
	VcIO os = lexer->get_err_strm();
	os << start.filename << ": syntax error in expression that began near line " <<
		start.linenum << "\n";
	os << msg << " at line " << end.linenum << "\n";
	error = 1;
}

vc
vc_cvar::make_atom()
{
	enum vc_type vct = VC_INT;
	switch(atom_type)
	{
	case VcLexer::INTEGER:
		vct = VC_INT;
		break;
	case VcLexer::FLOAT:
		vct = VC_DOUBLE;
		break;
	case VcLexer::STRING:
		vct = VC_BSTRING;
		break;
	default:
		oopanic("bad atom_type");
		/*NOTREACHED*/
	}
	vc cvar;
	if(!(toklen == 3 && strncmp(tokval, "nil", 3) == 0))
		cvar = vc(vct, tokval, toklen);
	return cvar;
}

vc
vc_cvar::pvar()
{
	vc cvar;
	int quot = 0;
	int inhibit_map = 0;

	switch(tok)
	{
	case ATOM:
		cvar = make_atom();
		next_tok();
		break;

	case LTICK:
		// note: quoting inhibits mapping as well
		quot = 1;
		/*FALL THROUGH*/

	case LBRACE:
		inhibit_map = 1;
		/*FALL THROUGH*/

	case LBRACKET:
		{
        vc_cvar_src_coord begin_scoord;
		next_tok();
		begin_scoord.init(lexer);
		vc_cvar *v = new vc_cvar;
		varlist(&v->vc_list);
		vc_cvar_src_coord end_scoord;
		end_scoord.init(lexer);

		v->quoted = quot;
		v->dont_map = inhibit_map;
		v->begin_scoord = begin_scoord;
		v->end_scoord = end_scoord;
		cvar.redefine(v);
		if(!quot)
		{
			if(!inhibit_map && tok != RBRACKET)
				syntax_err("expecting '>' after variable list", begin_scoord, end_scoord);
			if(inhibit_map && tok != RBRACE)
				syntax_err("expecting '}' after variable list", begin_scoord, end_scoord);
		}
		if(quot && tok != RTICK)
			syntax_err("expecting ' after variable list", begin_scoord, end_scoord);
		next_tok();
		  break;
		}

	default:
		syntax_err("real bad syntax error in variable expression");

	}
	return vprime(cvar);
}

vc
vc_cvar::vprime(vc cvar)
{
	if(tok != LPAREN && tok != LSBRACK)
		return cvar;

	int is_memselect = 0;
	if(tok == LSBRACK)
		is_memselect = 1;

	vc_cvar_src_coord start;
	next_tok();
	start.init(lexer);

	VCList tmp;
	vc selector;
	if(is_memselect)
		selector = pvar();
	else
		varlist(&tmp);

	if(!is_memselect && tok != RPAREN)
		syntax_err("expecting ')' after function arg list");
	if(is_memselect && tok != RSBRACK)
		syntax_err("expecting ']' after member selector");

	vc_cvar_src_coord end;
	next_tok();
	end.init(lexer);

	if(is_memselect)
	{
		vc_memselect *ms = new vc_memselect(cvar, selector, start, end);
		cvar.redefine(ms);
		return vprime(cvar);
	}
	else
	{
		vc_funcall *vca = new vc_funcall(cvar, tmp, start, end);
		cvar.redefine(vca);
		return vprime(cvar);
	}
}


#ifdef OLD_PARSE
void
vc_cvar::varlist(VCList *vlist)
{
	if(tok == ATOM || tok == LBRACKET || tok == LBRACE || tok == LTICK)
    {
		vc v = pvar();
		vlist->append(v);
		tail(vlist);
	} // else expand empty
}

void
vc_cvar::tail(VCList *vlist)
{
	if(tok == ATOM || tok == LBRACKET || tok == LBRACE || tok == LTICK)
	{
		varlist(vlist);
		return;
	}
    // expand empty
}
#else
void
vc_cvar::varlist(VCList *vlist)
{
	while(tok == ATOM || tok == LBRACKET || tok == LBRACE || tok == LTICK)
    {
		vc v = pvar();
		vlist->append(v);
	} // else expand empty
}

#endif

void
vc_cvar::stringrep(VcIO o) const
{
	vc atom;
	
	const char *tail = "";
	
	if(is_quoted())
	{
		o << "`";
		tail = "'";
	}
	else if(is_map_inhibited())
	{
		o << "{";
		tail = "}";
	}
	else if(!is_root)
	{
		o << "<";
		tail = ">";
	}
	VCListIter i(&vc_list);
	dwlista_foreach_iter(i, atom, vc_list)
	{
		atom.stringrep(o);
	}
	o << tail;
}

void
vc_cvar::dbg_print(int expr_num, const char *var_name) const
{
	dbg_print_date();
	VcError << "Eval expr that began near line " <<
		begin_scoord.filename << ":" << begin_scoord.linenum << "\n";
	dbg_print_date();
	VcError << "Expr num = " << expr_num << ", Accumulated result = \"" << var_name << "\"\n";
}

vc_cvar::vc_cvar()
{
	is_root = 0;
	exp_error = 0;
#ifdef CACHE_LOOKUPS
	range = 0;
	time_cached = 0;
#endif
#ifdef PERFHACKS
	nopf = 0;
	use_cached_atom = 0;
#endif
}
vc_cvar::vc_cvar(const vc_cvar& a)
	: vc_list(a.vc_list),
	is_root(a.is_root),
	exp_error(a.exp_error)
{
#ifdef CACHE_LOOKUPS
	range = a.range;
	time_cached = a.time_cached;
#endif
#ifdef PERFHACKS
	nopf = 0;
	use_cached_atom = 0;
#endif
}

void vc_cvar::bind(const vc& v) const {
	eval().bind(v);
}

void vc_cvar::local_bind(const vc& v) const {
	eval().local_bind(v);
}

void vc_cvar::global_bind(const vc& v) const {
	eval().global_bind(v);
}

void vc_cvar::bremove() const {
	eval().bremove();
}

void vc_cvar::local_bremove() const {
	eval().local_bremove();
}

void vc_cvar::global_bremove() const {
	eval().global_bremove();
}

vc_default *vc_cvar::do_copy() const {vc_cvar *v = new vc_cvar(*this); return v;}

vc_cvar::operator double() const {return (double)eval(); }
vc_cvar::operator int() const {return (int)eval();}
vc_cvar::operator long() const {return (long)eval();}
vc_cvar::operator const char *() const {return (const char *)eval();}

const char *vc_cvar::peek_str() const { oopanic("peek at cvar?"); return 0;}

vc vc_cvar::operator+(const vc &v) const {return eval() + v;}
vc vc_cvar::operator-(const vc &v) const {return eval() - v;}
vc vc_cvar::operator*(const vc &v) const {return eval() * v;}
vc vc_cvar::operator/(const vc &v) const {return eval() / v;}
vc vc_cvar::operator%(const vc &v) const {return eval() % v;}

vc vc_cvar::int_add(const vc& v) const {return eval().int_add(v);}
vc vc_cvar::int_sub(const vc& v) const {return eval().int_sub(v);}
vc vc_cvar::int_mul(const vc& v) const {return eval().int_mul(v);}
vc vc_cvar::int_div(const vc& v) const {return eval().int_div(v);}
vc vc_cvar::int_mod(const vc& v) const {return eval().int_mod(v);}

vc vc_cvar::double_add(const vc& v) const {return eval().double_add(v);}
vc vc_cvar::double_sub(const vc& v) const {return eval().double_sub(v);}
vc vc_cvar::double_mul(const vc& v) const {return eval().double_mul(v);}
vc vc_cvar::double_div(const vc& v) const {return eval().double_div(v);}
vc vc_cvar::double_mod(const vc& v) const {return eval().double_mod(v);}

int vc_cvar::int_lt(const vc& v) const {return eval().int_lt(v);}
int vc_cvar::int_le(const vc& v) const {return eval().int_le(v);}
int vc_cvar::int_gt(const vc& v) const {return eval().int_gt(v);}
int vc_cvar::int_ge(const vc& v) const {return eval().int_ge(v);}
int vc_cvar::int_eq(const vc& v) const {return eval().int_eq(v);}
int vc_cvar::int_ne(const vc& v) const {return eval().int_ne(v);}

int vc_cvar::double_lt(const vc& v) const {return eval().double_lt(v);}
int vc_cvar::double_le(const vc& v) const {return eval().double_le(v);}
int vc_cvar::double_gt(const vc& v) const {return eval().double_gt(v);}
int vc_cvar::double_ge(const vc& v) const {return eval().double_ge(v);}
int vc_cvar::double_eq(const vc& v) const {return eval().double_eq(v);}
int vc_cvar::double_ne(const vc& v) const {return eval().double_ne(v);}


vc vc_cvar::operator()(void *p) const {return eval()(p);}
vc vc_cvar::operator()(VCArglist *a) const {return eval()(a);}

vc vc_cvar::operator()() const {return eval()();}
vc vc_cvar::operator()(vc v0) const {return eval()(v0);}
vc vc_cvar::operator()(vc v0, vc v1) const {return eval()(v0, v1);}
vc vc_cvar::operator()(vc v0, vc v1, vc v2) const {return eval()(v0, v1, v2);}
vc vc_cvar::operator()(vc v0, vc v1, vc v2, vc v3) const {return eval()(v0, v1, v2, v3);}

int vc_cvar::func_eq(const vc& v) const {return eval().func_eq(v);}


enum vc_type vc_cvar::type() const { return VC_CVAR; }
int vc_cvar::is_nil() const { return eval().is_nil();}

int vc_cvar::operator <(const vc &v) const {return eval() < v;}
int vc_cvar::operator <=(const vc &v) const {return eval() <= v;}
int vc_cvar::operator >(const vc &v) const {return eval() > v;}
int vc_cvar::operator >=(const vc &v) const {return eval() >= v;}
int vc_cvar::operator ==(const vc &v) const {return eval() == v;}
int vc_cvar::operator !=(const vc &v) const {return eval() != v;}


// vc_cvar::VcIO operator<<(VcIO os) {os << d; return os;}

hashValueType vc_cvar::hashValue() const {return eval().hashValue();}
void vc_cvar::printOn(VcIO outputStream) {outputStream << "cvar";}

#endif
