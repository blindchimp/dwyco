
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// callbacks are handled like this:
//
// when LH calls a static callback set function, like
// set_directory_downloaded_callback(fun-name)
// the fun-name is a string
// in the convert function, we return a constant C function
// that will get installed in the dll and be called by it.
// this C function will get the callback, and the C function
// will use the stored value of the string to lookup a function
// and eval that function in LH.
//
// for functions that get callbacks that are associated with
// specific non-static contexts (like calls and zap sends)
// the callback arg package is explicitly saved, and an entry
// in a map is made that maps the context to the arg package.
// we then usurp the void * usually associated with such callbacks
// to use as a context identifier.
// again, we return a contant C function for the callback to the wrapper.
// this C function, when it gets the callback, uses the void * arg
// to find the user arg package, and then evals the users function
// with the right args.
// note: there is no way to know when an arg package is done
// being used, since some callbacks can be called repeatedly
// (like the status callback). should figure out a way of re-using
// arg packages, which will reduce the number that end up hanging around.
// alternately, we could piecemeal figure out when a context is done and
// delete the arg packages then.
//
// "out" parameters are handled in the convert function by
// allocating memory with malloc and returning a pointer to this
// memory. it is this pointer that is sent into the wrapped call, and
// into which the "out" result is presumably put.
// calls in LH send in strings as the "out" parameter, and
// this is used as a variable that is local-binded to the
// "out" parameter being returned by the function. the convert function
// appends a (name, ptr) pair to a global vector that is set up
// in the calling wrapper. when the wrapped function call returns
// the wrapper function traverses the vector and performs the
// lbinds, and then disposes of the temporaries.
//
// in-out parameters cannot be handled very easily in LH, and so in those
// cases, the LH function has a slightly different signature, usually
// an "in" parameter, and a separate "out" parameter.

#include "dwmapr.h"

static long unsigned int hash(const long& l)
{
	return l * 0x31415926;
}
static long AP_id;
static vc Arg_packages;
// an entry is made in this vector each time
// a callback ctx is created that needs to be killed
// when the callback ctx is finished.
struct idpair
{
	long ctx;
	long arg_package;
	int operator==(const idpair& i1) const {if(ctx == i1.ctx) return 1; return 0;}
};
static DwVec<struct idpair> Ctx_kill;
template class DwVec<struct idpair>;

static vc
cvt__double(double foo)
{
	return foo;
}

static vc
cvt__int(int foo)
{
	return foo;
}

static vc
cvt__long_long_int(long long int foo)
{
// problem in 32bits-ville, have to keep an eye on this
oopanic("long long cvt");
	return (long)foo;
}

static vc
cvt_p_char(char* foo)
{
	return foo;
}

static vc
cvt_p_sqlite3_(sqlite3* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_void(void* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_pq_char(const char* foo)
{
	return foo;
}

static vc
cvt_pq_unsigned_char(const unsigned char* foo)
{
	oopanic("cvt_pq_unsigned_char");
	return vcnil;
}

// it appears in some cases sqlite3 returns a pointer to a blob
// and then requires you to make another call to get the length of
// it, which kinda pooches our changes to make a single vc object out
// of it. maybe we don't need it.
static vc
cvt_pq_void(const void* foo)
{
	oopanic("figure this out");
	return vcnil;
}
static
double
cvt_vc__double(vc a)
{
	return a;
}
static
int
cvt_vc__int(vc a)
{
	return a;
}
static
long long int
cvt_vc__long_long_int(vc a)
{
oopanic("long long cvt");
	return (long)a;
}
#if 0
static
pF__int_p_void
cvt_vc_pF__int_p_void(vc a)
{
}
static
pF__int_p_void_x__int
cvt_vc_pF__int_p_void_x__int(vc a)
{
}
static
pF__int_p_void_x__int_x_pq_char_x_pq_char_x_pq_char_x_pq_char
cvt_vc_pF__int_p_void_x__int_x_pq_char_x_pq_char_x_pq_char_x_pq_char(vc a)
{
}
static
pF__int_p_void_x__int_x_pq_void_x__int_x_pq_void
cvt_vc_pF__int_p_void_x__int_x_pq_void_x__int_x_pq_void(vc a)
{
}
static
pF__void_p_sqlite3_context_
cvt_vc_pF__void_p_sqlite3_context_(vc a)
{
}
static
pF__void_p_sqlite3_context__x__int_x_pp_Mem_
cvt_vc_pF__void_p_sqlite3_context__x__int_x_pp_Mem_(vc a)
{
}
static
pF__void_p_void
cvt_vc_pF__void_p_void(vc a)
{
}
static
pF__void_p_void_x__int_x_pq_char_x_pq_char_x__long_long_int
cvt_vc_pF__void_p_void_x__int_x_pq_char_x_pq_char_x__long_long_int(vc a)
{
}
static
pF__void_p_void_x_p_sqlite3__x__int_x_pq_char
cvt_vc_pF__void_p_void_x_p_sqlite3__x__int_x_pq_char(vc a)
{
}
static
pF__void_p_void_x_p_sqlite3__x__int_x_pq_void
cvt_vc_pF__void_p_void_x_p_sqlite3__x__int_x_pq_void(vc a)
{
}
static
pF__void_p_void_x_pq_char
cvt_vc_pF__void_p_void_x_pq_char(vc a)
{
}
static
pF__void_p_void_x_pq_char_x__long_long_unsigned_int
cvt_vc_pF__void_p_void_x_pq_char_x__long_long_unsigned_int(vc a)
{
}
#endif
static
Mem*
cvt_vc_p_Mem_(vc a)
{
	return (Mem *)(void *)a;
}
// note: only used in sqlite3_free
static
char*
cvt_vc_p_char(vc a)
{
	return (char *)(void *)a;
}
// only used in get_table
static
int*
cvt_vc_p_int(vc a)
{
}

static
sqlite3*
cvt_vc_p_sqlite3_(vc a)
{
	return (sqlite3 *)(void *)a;
}
static
sqlite3_context*
cvt_vc_p_sqlite3_context_(vc a)
{
	return (sqlite3_context *)(void *)a;
}
static
sqlite3_stmt*
cvt_vc_p_sqlite3_stmt_(vc a)
{
	return (sqlite3_stmt *)(void *)a;
}
static
void*
cvt_vc_p_void(vc a)
{
	if(a.is_nil())
		return 0;
	return a;
}
// this is used as an out in exec,
static
char**
cvt_vc_pp_char(vc a)
{
}
// out param
static
sqlite3**
cvt_vc_pp_sqlite3_(vc a)
{
	if(a.is_nil())
		return 0;
	sqlite3 **v = (sqlite3 **)malloc(sizeof(sqlite3 *));
	vc bv(VC_VECTOR);
	bv[0] = a;
	bv[1] = (long)v;
	outbind_pvoid->append(bv);
	push_ptr(v);
	return v;
}

static
sqlite3_stmt**
cvt_vc_pp_sqlite3_stmt_(vc a)
{
}
static
char***
cvt_vc_ppp_char(vc a)
{
}
static
const char**
cvt_vc_ppq_char(vc a)
{
}
static
const void**
cvt_vc_ppq_void(vc a)
{
}
static
const Mem*
cvt_vc_pq_Mem_(vc a)
{
}
static
const char*
cvt_vc_pq_char(vc a)
{
	return a;
}
static
const void*
cvt_vc_pq_void(vc a)
{
}

static DwVec<int> Column_type_cache(0, !DWVEC_FIXED, !DWVEC_AUTO_EXPAND);
static vc Column_name_cache;

static
int
sqlite3_callback_bounce(void *user_arg, int argc, char **argv, char **columnNames)
{
	// we stole the user_arg to use in finding the arg package to
	// send to the user.
	vc args;
	if(!Arg_packages.find((long)user_arg, args))
		oopanic("unregistered call disp callback");
	vc fn = args[0]; // LH function name to call (speced by user)
	// this callback is kinda complicated, for now we are going to 
	// assume that pragma show_datatypes is set.
	// it appears from the documentation that sqlite3_exec does not
	// return until the results are all computed, which means we
	// don't need a dynamic set of callback args, tho i'll do it
	// anyway just in case. (one could imagine a case where you
	// have multiple threads and/or exec returned immediately and
	// the callback was called as the results became available.)
	vc res(VC_VECTOR);
	// accumulate results into a vector
#if 1
// can't do this if sqlite doesn't have show_datatypes
	if(Column_type_cache.num_elems() == 0)
	{
		Column_type_cache.set_size(argc);
		for(int i = 0; i < argc; ++i)
		{
			if(strcasecmp(columnNames[argc + i], "text") == 0)
				Column_type_cache[i] = 0;
			else if(strcasecmp(columnNames[argc + i], "integer") == 0)
				Column_type_cache[i] = 1;
			else if(strcasecmp(columnNames[argc + i], "float") == 0)
				Column_type_cache[i] = 2;
			else
				Column_type_cache[i] = 0;
		}
	}
#endif
	if(Column_name_cache.num_elems() == 0)
	{
		for(int i = 0; i < argc; ++i)
			Column_name_cache[i] = columnNames[i];
	}
	for(int i = 0; i < argc; ++i)
	{
		if(argv[i] == 0)
			res[i] = vcnil;
		else
		{
#if 1
			switch(Column_type_cache[i])
			{
			case 0:
				res[i] = argv[i];
				break;
			case 1:
				res[i] = atol(argv[i]);
				break;
			case 2:
				res[i] = atof(argv[i]);
				break;
			default:
				res[i] = vcnil;
			}
#else
			res[i] = argv[i];
#endif
		}
	}
	VCArglist nargs;
	nargs[0] = args[1];
	nargs[1] = res;
	nargs[2] = Column_name_cache;
	vc ret = fn(&nargs);
	if(ret.is_nil())
		return 0;
	return (int)ret;
}

static
sqlite3_callback
cvt_vc_sqlite3_callback(vc a)
{
	if(a.is_nil())
		return 0;
	return sqlite3_callback_bounce;
}
