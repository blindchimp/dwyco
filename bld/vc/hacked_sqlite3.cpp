
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "vc.h"
#include "dwvecp.h"
#include "vcmap.h"

static DwVecP<void> Arg_stk;
static DwVec<vc> *outbind_bool;
static DwVec<vc> *outbind_int;
static DwVec<vc> *outbind_pvoid;
static DwVec<vc> *outbind_long;
static DwVec<vc> *outbind_str;
static DwVec<vc> *outbind_double;
typedef DwVec<vc> vvc;
#define Mem sqlite3_value

static
int
start_wrapper()
{
	return Arg_stk.num_elems();
}

static
void
end_wrapper(int i)
{
	int j = Arg_stk.num_elems();
	for(int k = j - 1; k >= i; --k)
		free(Arg_stk[k]);
	Arg_stk.set_size(i);
}

static void
push_ptr(void *p)
{
	Arg_stk.append(p);
}

#define START_WRAP int i = start_wrapper(); \
DwVec<vc> loutbind_bool; \
DwVec<vc> loutbind_int; \
DwVec<vc> loutbind_pvoid; \
DwVec<vc> loutbind_long; \
DwVec<vc> loutbind_str; \
DwVec<vc> loutbind_double; \
vvc *soutbind_bool = outbind_bool; \
outbind_bool = &loutbind_bool; \
vvc *soutbind_int = outbind_int; \
outbind_int = &loutbind_int; \
vvc *soutbind_pvoid = outbind_pvoid; \
outbind_pvoid = &loutbind_pvoid; \
vvc *soutbind_long = outbind_long; \
outbind_long = &loutbind_long; \
vvc *soutbind_str = outbind_str; \
outbind_str = &loutbind_str; \
vvc *soutbind_double = outbind_double; \
outbind_double = &loutbind_double;

#define START_WRAP_misc int i = start_wrapper(); \
DwVec<vc> loutbind_int; \
DwVec<vc> loutbind_str; \
vvc *soutbind_int = outbind_int; \
outbind_int = &loutbind_int; \
vvc *soutbind_str = outbind_str; \
outbind_str = &loutbind_str;

	
// WARNING: this assumes that strings are 0 terminated
#define outbind(type, ctype) \
	{int n = outbind_##type->num_elems(); \
	for(int i = 0; i < n; ++i) \
	{\
		vc bv = (*outbind_##type)[i]; \
		bv[0].local_bind(*(ctype *)(void *)bv[1]); \
	}\
	outbind_##type = soutbind_##type; \
	}

#define WRAP_END  \
	outbind(bool, bool) \
	outbind(int, int) \
	outbind(pvoid, long) \
	outbind(long, long) \
	outbind(str, const char *) \
	outbind(double, double) \
	end_wrapper(i);

#define WRAP_END_misc  \
	vc bv = (*outbind_str)[0]; \
	vc bv_len = (*outbind_int)[0]; \
	int len = *(int *)(void *)bv_len[1]; \
	bv[0].local_bind(vc(VC_BSTRING, (const char *)(void *)bv[1], len)); \
	outbind_int = soutbind_int; \
	outbind_str = soutbind_str; \
	end_wrapper(i);

#define CTX_INTERPOSE(apid, fp_argnum, vp_argnum) \
void *scb_arg1 = cvt_vc_p_void((*a)[vp_argnum]); \
vc args(VC_VECTOR); \
args[0] = (*a)[fp_argnum]; \
args[1] = (long)scb_arg1; \
long ap_id = apid; \
Arg_packages.add_kv(ap_id, args);

#define CTX_KILL \
{\
struct idpair idp; \
idp.ctx = (long)ret; \
idp.arg_package = ap_id; \
Ctx_kill.append(idp);\
}

#define CTX_INTERPOSE2(apid, fp_argnum, vp_argnum) \
scb_arg1 = cvt_vc_p_void((*a)[vp_argnum]); \
args = vc(VC_VECTOR); \
args[0] = (*a)[fp_argnum]; \
args[1] = (long)scb_arg1; \
long ap_id2 = apid; \
Arg_packages.add_kv(ap_id2, args);

#define CTX_KILL2 \
{\
struct idpair idp; \
idp.ctx = (long)ret; \
idp.arg_package = ap_id2; \
Ctx_kill.append(idp);\
}


#include <sqlite3.h>
static vc
wrap_sqlite3_bulk_query(VCArglist *a)
{
	VCArglist &aa = *a;
	vc db = aa[0];
	sqlite3 *dbs = (sqlite3 *)(void *)db;
	vc sql = aa[1];
	vc err = aa[2];
	// everything after err is considered to be a binding to be
	// set into the sql
	sqlite3_stmt *st = 0;
	const char *tail = 0;
	int errcode;
	if((errcode = sqlite3_prepare_v2(dbs, sql, sql.len(), 
		&st, &tail)) != SQLITE_OK)
	{
		err.local_bind(sqlite3_errmsg(dbs));
		return vcnil;
	}
	if(tail && *tail != 0)
	{
        USER_BOMB("must be exactly 1 sql statement in query", vcnil);
	}
	// bind in vars
	if(a->num_elems() > 3)
	{
		for(int i = 3; i < a->num_elems(); ++i)
		{
			switch((*a)[i].type())
			{
			case VC_INT:
				if(sqlite3_bind_int(st, i - 2, (*a)[i]) != SQLITE_OK)
				{
					sqlite3_finalize(st);
                    USER_BOMB("sql bind error", vcnil);
				}
				break;
			case VC_STRING:
			case VC_BSTRING:
				if(sqlite3_bind_text(st, i - 2, (*a)[i], (*a)[i].len(), SQLITE_TRANSIENT) != SQLITE_OK)
				{
					sqlite3_finalize(st);
                    USER_BOMB("sql bind error", vcnil);
				}
				break;
			case VC_DOUBLE:
				if(sqlite3_bind_double(st, i - 2, (*a)[i]) != SQLITE_OK)
				{
					sqlite3_finalize(st);
                    USER_BOMB("sql bind error", vcnil);
				}
				break;
			case VC_NIL:
				if(sqlite3_bind_null(st, i - 2) != SQLITE_OK)
				{
					sqlite3_finalize(st);
                    USER_BOMB("sql bind error", vcnil);
				}
				break;
			case VC_VECTOR:
				// this is a kluge: we allow specifying a type
				// in the first element, and the value in the second
				// element. this is only used for blobs at the moment,
				// but it could easily be extended to allow this
				// routine to serialize and deserialize stuff for the
				// caller.
				if((*a)[i].num_elems() != 2 ||
					(*a)[i][0] != vc("blob"))
				{
                    USER_BOMB("sql bind error, explicit type vector must be (type value) pair (and only \"blob\" type is supported now.", vcnil);
				}
				if(sqlite3_bind_blob(st, i - 2, (const void *)(const char *)((*a)[i][1]), (*a)[i][1].len(), SQLITE_TRANSIENT) != SQLITE_OK)
				{
					sqlite3_finalize(st);
                    USER_BOMB("sql bind error", vcnil);
				}
				break;

			default:
                USER_BOMB("can't bind that in sql", vcnil);
			}
		}
	}
	vc res(VC_VECTOR);
	int cols = sqlite3_column_count(st);
	while(1)
	{
		int serr = sqlite3_step(st);
		switch(serr)
		{
		case SQLITE_DONE:
			goto out;
		case SQLITE_BUSY:
			// need some indication of what we should do here.
			// retry or abort and error out. maybe do some exception
			// stuff here to allow the user to abort if they want to.
			res = "busy";
			// note: because of bugs in sqlite, it appears that
			// this case requires any transaction to be rolled back
			// to avoid database corruption (wtf, that is pretty sad.)
			// anyway, as long as you are 3.4+ on sqlite, the
			// transaction will be rolled back for you. it doesn't
			// really say how that affects other operations. it is
			// probably best to just issue an explicit rollback sql
			// command just in case.
			break;
		case SQLITE_ROW:
			{
				vc resrow(VC_VECTOR);
				for(int i = 0; i < cols; ++i)
				{
					switch(sqlite3_column_type(st, i))
					{
					case SQLITE_INTEGER:
						resrow[i] = sqlite3_column_int(st, i);
						break;
					case SQLITE_FLOAT:
						resrow[i] = sqlite3_column_double(st, i);
						break;
					case SQLITE_BLOB:
						resrow[i] = vc(VC_BSTRING, (const char *)sqlite3_column_blob(st, i), sqlite3_column_bytes(st, i));
						break;
					case SQLITE_NULL:
						resrow[i] = vcnil;
						break;
					case SQLITE_TEXT:
						resrow[i] = vc(VC_BSTRING, (const char *)sqlite3_column_text(st, i), sqlite3_column_bytes(st, i));
						break;
					default:
						oopanic("bogus sqlite type");
					}
				}
				res.append(resrow);
			}
			break;
		default:
			sqlite3_finalize(st);
			err.local_bind(sqlite3_errmsg(dbs));
			return vcnil;
		}
	}
out: ;
	sqlite3_finalize(st);
	return res;
}


#include "hacked_cvt_sqlite3.cpp"

#if 0
	static vc
wrap_sqlite3_table_column_metadata(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_table_column_metadata(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc_pq_char((*a)[3]),
cvt_vc_ppq_char((*a)[4]),
cvt_vc_ppq_char((*a)[5]),
cvt_vc_p_int((*a)[6]),
cvt_vc_p_int((*a)[7]),
cvt_vc_p_int((*a)[8])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_thread_cleanup(VCArglist *a)
{
START_WRAP
	sqlite3_thread_cleanup();
WRAP_END
return(vcnil);

}
#endif
#if 0
static vc
wrap_sqlite3_soft_heap_limit(VCArglist *a)
{
START_WRAP
	sqlite3_soft_heap_limit(cvt_vc__int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_sqlite3_release_memory(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_release_memory(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}

static vc
wrap_sqlite3_enable_shared_cache(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_enable_shared_cache(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}

static vc
wrap_sqlite3_rollback_hook(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(sqlite3_rollback_hook(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pF__void_p_void((*a)[1]),
cvt_vc_p_void((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_update_hook(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(sqlite3_update_hook(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pF__void_p_void_x__int_x_pq_char_x_pq_char_x__long_long_int((*a)[1]),
cvt_vc_p_void((*a)[2])));
WRAP_END
return ret;

}
#endif
static vc
wrap_sqlite3_db_handle(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_sqlite3_(sqlite3_db_handle(cvt_vc_p_sqlite3_stmt_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_get_autocommit(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_get_autocommit(cvt_vc_p_sqlite3_((*a)[0])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_sqlite3_global_recover(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_global_recover());
WRAP_END
return ret;

}
static vc
wrap_sqlite3_transfer_bindings(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_transfer_bindings(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc_p_sqlite3_stmt_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_expired(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_expired(cvt_vc_p_sqlite3_stmt_((*a)[0])));
WRAP_END
return ret;

}
#endif
#if 0
static vc
wrap_sqlite3_sleep(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_sleep(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_rekey(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_rekey(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pq_void((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_key(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_key(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pq_void((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_collation_needed16(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_collation_needed16(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_p_void((*a)[1]),
cvt_vc_pF__void_p_void_x_p_sqlite3__x__int_x_pq_void((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_collation_needed(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_collation_needed(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_p_void((*a)[1]),
cvt_vc_pF__void_p_void_x_p_sqlite3__x__int_x_pq_char((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_create_collation16(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_create_collation16(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_void((*a)[3]),
cvt_vc_pF__int_p_void_x__int_x_pq_void_x__int_x_pq_void((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_create_collation(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_create_collation(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_void((*a)[3]),
cvt_vc_pF__int_p_void_x__int_x_pq_void_x__int_x_pq_void((*a)[4])));
WRAP_END
return ret;

}
#endif
static vc
wrap_sqlite3_result_value(VCArglist *a)
{
START_WRAP
	sqlite3_result_value(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc_p_Mem_((*a)[1]));
WRAP_END
return(vcnil);

}
#if 0
static vc
wrap_sqlite3_result_text16be(VCArglist *a)
{
START_WRAP
	sqlite3_result_text16be(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc_pq_void((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pF__void_p_void((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_sqlite3_result_text16le(VCArglist *a)
{
START_WRAP
	sqlite3_result_text16le(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc_pq_void((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pF__void_p_void((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_sqlite3_result_text16(VCArglist *a)
{
START_WRAP
	sqlite3_result_text16(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc_pq_void((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pF__void_p_void((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_sqlite3_result_text(VCArglist *a)
{
START_WRAP
	sqlite3_result_text(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pF__void_p_void((*a)[3]));
WRAP_END
return(vcnil);

}
#endif
static vc
wrap_sqlite3_result_null(VCArglist *a)
{
START_WRAP
	sqlite3_result_null(cvt_vc_p_sqlite3_context_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_sqlite3_result_int64(VCArglist *a)
{
START_WRAP
	sqlite3_result_int64(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc__long_long_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_sqlite3_result_int(VCArglist *a)
{
START_WRAP
	sqlite3_result_int(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_sqlite3_result_error16(VCArglist *a)
{
START_WRAP
	sqlite3_result_error16(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc_pq_void((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_sqlite3_result_error(VCArglist *a)
{
START_WRAP
	sqlite3_result_error(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_sqlite3_result_double(VCArglist *a)
{
START_WRAP
	sqlite3_result_double(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc__double((*a)[1]));
WRAP_END
return(vcnil);

}
#if 0
static vc
wrap_sqlite3_result_blob(VCArglist *a)
{
START_WRAP
	sqlite3_result_blob(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc_pq_void((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pF__void_p_void((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_sqlite3_set_auxdata(VCArglist *a)
{
START_WRAP
	sqlite3_set_auxdata(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_void((*a)[2]),
cvt_vc_pF__void_p_void((*a)[3]));
WRAP_END
return(vcnil);

}
#endif
static vc
wrap_sqlite3_get_auxdata(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(sqlite3_get_auxdata(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_user_data(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(sqlite3_user_data(cvt_vc_p_sqlite3_context_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_aggregate_context(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(sqlite3_aggregate_context(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_value_numeric_type(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_value_numeric_type(cvt_vc_p_Mem_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_value_type(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_value_type(cvt_vc_p_Mem_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_value_text16be(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_void(sqlite3_value_text16be(cvt_vc_p_Mem_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_value_text16le(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_void(sqlite3_value_text16le(cvt_vc_p_Mem_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_value_text16(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_void(sqlite3_value_text16(cvt_vc_p_Mem_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_value_text(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_unsigned_char(sqlite3_value_text(cvt_vc_p_Mem_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_value_int64(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_long_int(sqlite3_value_int64(cvt_vc_p_Mem_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_value_int(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_value_int(cvt_vc_p_Mem_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_value_double(VCArglist *a)
{
START_WRAP
	vc ret = cvt__double(sqlite3_value_double(cvt_vc_p_Mem_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_value_bytes16(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_value_bytes16(cvt_vc_p_Mem_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_value_bytes(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_value_bytes(cvt_vc_p_Mem_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_value_blob(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_void(sqlite3_value_blob(cvt_vc_p_Mem_((*a)[0])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_sqlite3_aggregate_count(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_aggregate_count(cvt_vc_p_sqlite3_context_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_create_function16(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_create_function16(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pq_void((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_p_void((*a)[4]),
cvt_vc_pF__void_p_sqlite3_context__x__int_x_pp_Mem_((*a)[5]),
cvt_vc_pF__void_p_sqlite3_context__x__int_x_pp_Mem_((*a)[6]),
cvt_vc_pF__void_p_sqlite3_context_((*a)[7])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_create_function(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_create_function(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_p_void((*a)[4]),
cvt_vc_pF__void_p_sqlite3_context__x__int_x_pp_Mem_((*a)[5]),
cvt_vc_pF__void_p_sqlite3_context__x__int_x_pp_Mem_((*a)[6]),
cvt_vc_pF__void_p_sqlite3_context_((*a)[7])));
WRAP_END
return ret;

}
#endif
static vc
wrap_sqlite3_reset(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_reset(cvt_vc_p_sqlite3_stmt_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_finalize(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_finalize(cvt_vc_p_sqlite3_stmt_((*a)[0])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_sqlite3_column_numeric_type(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_column_numeric_type(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
#endif
static vc
wrap_sqlite3_column_type(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_column_type(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_text16(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_void(sqlite3_column_text16(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_text(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_unsigned_char(sqlite3_column_text(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_int64(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_long_int(sqlite3_column_int64(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_int(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_column_int(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_double(VCArglist *a)
{
START_WRAP
	vc ret = cvt__double(sqlite3_column_double(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_bytes16(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_column_bytes16(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_bytes(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_column_bytes(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_blob(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_void(sqlite3_column_blob(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_data_count(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_data_count(cvt_vc_p_sqlite3_stmt_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_step(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_step(cvt_vc_p_sqlite3_stmt_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_decltype16(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_void(sqlite3_column_decltype16(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_decltype(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_char(sqlite3_column_decltype(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_sqlite3_column_origin_name16(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_void(sqlite3_column_origin_name16(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_origin_name(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_char(sqlite3_column_origin_name(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_table_name16(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_void(sqlite3_column_table_name16(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_table_name(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_char(sqlite3_column_table_name(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_database_name16(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_void(sqlite3_column_database_name16(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_database_name(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_char(sqlite3_column_database_name(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
#endif
static vc
wrap_sqlite3_column_name16(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_void(sqlite3_column_name16(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_name(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_char(sqlite3_column_name(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_column_count(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_column_count(cvt_vc_p_sqlite3_stmt_((*a)[0])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_sqlite3_clear_bindings(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_clear_bindings(cvt_vc_p_sqlite3_stmt_((*a)[0])));
WRAP_END
return ret;

}
#endif
static vc
wrap_sqlite3_bind_parameter_index(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_bind_parameter_index(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc_pq_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_bind_parameter_name(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_char(sqlite3_bind_parameter_name(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_bind_parameter_count(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_bind_parameter_count(cvt_vc_p_sqlite3_stmt_((*a)[0])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_sqlite3_bind_value(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_bind_value(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_Mem_((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_bind_text16(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_bind_text16(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_void((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_pF__void_p_void((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_bind_text(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_bind_text(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_pF__void_p_void((*a)[4])));
WRAP_END
return ret;

}
#endif
static vc
wrap_sqlite3_bind_null(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_bind_null(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_bind_int64(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_bind_int64(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__long_long_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_bind_int(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_bind_int(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_bind_double(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_bind_double(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__double((*a)[2])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_sqlite3_bind_blob(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_bind_blob(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_void((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_pF__void_p_void((*a)[4])));
WRAP_END
return ret;

}
#endif
static vc
wrap_sqlite3_prepare16(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_prepare16(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pq_void((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pp_sqlite3_stmt_((*a)[3]),
cvt_vc_ppq_void((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_prepare(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_prepare(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pp_sqlite3_stmt_((*a)[3]),
cvt_vc_ppq_char((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_errmsg16(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_void(sqlite3_errmsg16(cvt_vc_p_sqlite3_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_errmsg(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_char(sqlite3_errmsg(cvt_vc_p_sqlite3_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_errcode(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_errcode(cvt_vc_p_sqlite3_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_open16(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_open16(cvt_vc_pq_void((*a)[0]),
cvt_vc_pp_sqlite3_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_open(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_open(cvt_vc_pq_char((*a)[0]),
cvt_vc_pp_sqlite3_((*a)[1])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_sqlite3_commit_hook(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(sqlite3_commit_hook(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pF__int_p_void((*a)[1]),
cvt_vc_p_void((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_progress_handler(VCArglist *a)
{
START_WRAP
	sqlite3_progress_handler(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pF__int_p_void((*a)[2]),
cvt_vc_p_void((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_sqlite3_profile(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(sqlite3_profile(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pF__void_p_void_x_pq_char_x__long_long_unsigned_int((*a)[1]),
cvt_vc_p_void((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_trace(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_void(sqlite3_trace(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pF__void_p_void_x_pq_char((*a)[1]),
cvt_vc_p_void((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_set_authorizer(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_set_authorizer(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pF__int_p_void_x__int_x_pq_char_x_pq_char_x_pq_char_x_pq_char((*a)[1]),
cvt_vc_p_void((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_snprintf(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(sqlite3_snprintf(cvt_vc__int((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc_pq_char((*a)[2]),
));
WRAP_END
return ret;

}
#endif
static vc
wrap_sqlite3_free(VCArglist *a)
{
START_WRAP
	sqlite3_free(cvt_vc_p_char((*a)[0]));
WRAP_END
return(vcnil);

}
#if 0
static vc
wrap_sqlite3_vmprintf(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(sqlite3_vmprintf(cvt_vc_pq_char((*a)[0]),
cvt_vc_p_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_mprintf(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(sqlite3_mprintf(cvt_vc_pq_char((*a)[0]),
));
WRAP_END
return ret;

}
#endif
static vc
wrap_sqlite3_free_table(VCArglist *a)
{
START_WRAP
	sqlite3_free_table(cvt_vc_pp_char((*a)[0]));
WRAP_END
return(vcnil);

}

#if 0
static vc
wrap_sqlite3_get_table(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_get_table(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_ppp_char((*a)[2]),
cvt_vc_p_int((*a)[3]),
cvt_vc_p_int((*a)[4]),
cvt_vc_pp_char((*a)[5])));
WRAP_END
return ret;

}
#endif

static vc
wrap_sqlite3_busy_timeout(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_busy_timeout(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_sqlite3_busy_handler(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_busy_handler(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pF__int_p_void_x__int((*a)[1]),
cvt_vc_p_void((*a)[2])));
WRAP_END
return ret;

}
#endif
static vc
wrap_sqlite3_complete16(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_complete16(cvt_vc_pq_void((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_complete(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_complete(cvt_vc_pq_char((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_interrupt(VCArglist *a)
{
START_WRAP
	sqlite3_interrupt(cvt_vc_p_sqlite3_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_sqlite3_total_changes(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_total_changes(cvt_vc_p_sqlite3_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_changes(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_changes(cvt_vc_p_sqlite3_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_last_insert_rowid(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_long_int(sqlite3_last_insert_rowid(cvt_vc_p_sqlite3_((*a)[0])));
WRAP_END
return ret;

}
// see notes in cvt regarding assumptions about this function and
// its callback
static vc
wrap_sqlite3_exec(VCArglist *a)
{
CTX_INTERPOSE(AP_id++, 2, 3)
Column_type_cache.set_size(0);
Column_name_cache = vc(VC_VECTOR);
char *error = 0;
START_WRAP
	vc ret = cvt__int(sqlite3_exec(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_sqlite3_callback((*a)[2]),
	(void *)ap_id, //cvt_vc_p_void((*a)[3]),
	&error
	));
// do the error string by hand
//cvt_vc_pp_char((*a)[4])));
	if(error)
	{
		// lbind to the arg string
		(*a)[4].local_bind(error);
		free(error);
	}
	else
	{
		(*a)[4].local_bind(vcnil);
	}
WRAP_END
// we assume this callback is done, so we just kill the
// ctx here
	Arg_packages.del(ap_id);
return ret;

}
static vc
wrap_sqlite3_close(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_close(cvt_vc_p_sqlite3_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_sqlite3_libversion_number(VCArglist *)
{
START_WRAP
	vc ret = cvt__int(sqlite3_libversion_number());
WRAP_END
return ret;

}
static vc
wrap_sqlite3_libversion(VCArglist *)
{
START_WRAP
	vc ret = cvt_pq_char(sqlite3_libversion());
WRAP_END
return ret;

}

static void
makefun(const char *name, const vc& fun)
{
    vc(name).bind(fun);
}

#define VC(fun, nicename, attr) vc(fun, nicename, #fun, attr)
#define VC2(fun, nicename, attr, trans) vc(fun, nicename, #fun, attr, trans)

void
wrapper_init_sqlite3 ()
{
Arg_packages = vc(VC_MAP);

makefun("sql_query", VC(wrap_sqlite3_bulk_query, "sql_query", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sql_query", VC(wrap_sqlite3_bulk_query, "wrap_sql_query", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_errmsg", VC(wrap_sqlite3_errmsg, "wrap_sqlite3_errmsg", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_bind_text", VC(wrap_sqlite3_bind_text, "wrap_sqlite3_bind_text", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_busy_handler", VC(wrap_sqlite3_busy_handler, "wrap_sqlite3_busy_handler", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_blob", VC(wrap_sqlite3_column_blob, "wrap_sqlite3_column_blob", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_aggregate_context", VC(wrap_sqlite3_aggregate_context, "wrap_sqlite3_aggregate_context", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_count", VC(wrap_sqlite3_column_count, "wrap_sqlite3_column_count", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_decltype", VC(wrap_sqlite3_column_decltype, "wrap_sqlite3_column_decltype", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_step", VC(wrap_sqlite3_step, "wrap_sqlite3_step", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_finalize", VC(wrap_sqlite3_finalize, "wrap_sqlite3_finalize", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_result_text16be", VC(wrap_sqlite3_result_text16be, "wrap_sqlite3_result_text16be", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_enable_shared_cache", VC(wrap_sqlite3_enable_shared_cache, "wrap_sqlite3_enable_shared_cache", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_exec", VC(wrap_sqlite3_exec, "wrap_sqlite3_exec", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_bytes", VC(wrap_sqlite3_column_bytes, "wrap_sqlite3_column_bytes", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_double", VC(wrap_sqlite3_value_double, "wrap_sqlite3_value_double", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_result_text16", VC(wrap_sqlite3_result_text16, "wrap_sqlite3_result_text16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_free", VC(wrap_sqlite3_free, "wrap_sqlite3_free", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_set_authorizer", VC(wrap_sqlite3_set_authorizer, "wrap_sqlite3_set_authorizer", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_text", VC(wrap_sqlite3_column_text, "wrap_sqlite3_column_text", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_create_function16", VC(wrap_sqlite3_create_function16, "wrap_sqlite3_create_function16", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_create_collation16", VC(wrap_sqlite3_create_collation16, "wrap_sqlite3_create_collation16", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_collation_needed16", VC(wrap_sqlite3_collation_needed16, "wrap_sqlite3_collation_needed16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_reset", VC(wrap_sqlite3_reset, "wrap_sqlite3_reset", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_text16le", VC(wrap_sqlite3_value_text16le, "wrap_sqlite3_value_text16le", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_close", VC(wrap_sqlite3_close, "wrap_sqlite3_close", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_snprintf", VC(wrap_sqlite3_snprintf, "wrap_sqlite3_snprintf", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_double", VC(wrap_sqlite3_bind_double, "wrap_sqlite3_bind_double", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_bind_value", VC(wrap_sqlite3_bind_value, "wrap_sqlite3_bind_value", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_blob", VC(wrap_sqlite3_value_blob, "wrap_sqlite3_value_blob", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_int64", VC(wrap_sqlite3_result_int64, "wrap_sqlite3_result_int64", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_changes", VC(wrap_sqlite3_changes, "wrap_sqlite3_changes", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_complete", VC(wrap_sqlite3_complete, "wrap_sqlite3_complete", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_parameter_count", VC(wrap_sqlite3_bind_parameter_count, "wrap_sqlite3_bind_parameter_count", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_double", VC(wrap_sqlite3_column_double, "wrap_sqlite3_column_double", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_bytes", VC(wrap_sqlite3_value_bytes, "wrap_sqlite3_value_bytes", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_result_blob", VC(wrap_sqlite3_result_blob, "wrap_sqlite3_result_blob", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_table_column_metadata", VC(wrap_sqlite3_table_column_metadata, "wrap_sqlite3_table_column_metadata", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_mprintf", VC(wrap_sqlite3_mprintf, "wrap_sqlite3_mprintf", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_name", VC(wrap_sqlite3_column_name, "wrap_sqlite3_column_name", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_user_data", VC(wrap_sqlite3_user_data, "wrap_sqlite3_user_data", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_interrupt", VC(wrap_sqlite3_interrupt, "wrap_sqlite3_interrupt", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_open", VC(wrap_sqlite3_open, "wrap_sqlite3_open", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_null", VC(wrap_sqlite3_bind_null, "wrap_sqlite3_bind_null", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_name16", VC(wrap_sqlite3_column_name16, "wrap_sqlite3_column_name16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_errmsg16", VC(wrap_sqlite3_errmsg16, "wrap_sqlite3_errmsg16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_prepare16", VC(wrap_sqlite3_prepare16, "wrap_sqlite3_prepare16", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_clear_bindings", VC(wrap_sqlite3_clear_bindings, "wrap_sqlite3_clear_bindings", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_column_database_name16", VC(wrap_sqlite3_column_database_name16, "wrap_sqlite3_column_database_name16", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_column_table_name16", VC(wrap_sqlite3_column_table_name16, "wrap_sqlite3_column_table_name16", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_result_text", VC(wrap_sqlite3_result_text, "wrap_sqlite3_result_text", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_transfer_bindings", VC(wrap_sqlite3_transfer_bindings, "wrap_sqlite3_transfer_bindings", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_update_hook", VC(wrap_sqlite3_update_hook, "wrap_sqlite3_update_hook", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_soft_heap_limit", VC(wrap_sqlite3_soft_heap_limit, "wrap_sqlite3_soft_heap_limit", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_trace", VC(wrap_sqlite3_trace, "wrap_sqlite3_trace", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_column_origin_name", VC(wrap_sqlite3_column_origin_name, "wrap_sqlite3_column_origin_name", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_data_count", VC(wrap_sqlite3_data_count, "wrap_sqlite3_data_count", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_bytes16", VC(wrap_sqlite3_value_bytes16, "wrap_sqlite3_value_bytes16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_int", VC(wrap_sqlite3_value_int, "wrap_sqlite3_value_int", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_numeric_type", VC(wrap_sqlite3_value_numeric_type, "wrap_sqlite3_value_numeric_type", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_release_memory", VC(wrap_sqlite3_release_memory, "wrap_sqlite3_release_memory", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_prepare", VC(wrap_sqlite3_prepare, "wrap_sqlite3_prepare", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_libversion", VC(wrap_sqlite3_libversion, "wrap_sqlite3_libversion", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_get_table", VC(wrap_sqlite3_get_table, "wrap_sqlite3_get_table", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_free_table", VC(wrap_sqlite3_free_table, "wrap_sqlite3_free_table", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_int64", VC(wrap_sqlite3_value_int64, "wrap_sqlite3_value_int64", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_double", VC(wrap_sqlite3_result_double, "wrap_sqlite3_result_double", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_rollback_hook", VC(wrap_sqlite3_rollback_hook, "wrap_sqlite3_rollback_hook", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_vmprintf", VC(wrap_sqlite3_vmprintf, "wrap_sqlite3_vmprintf", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_bind_blob", VC(wrap_sqlite3_bind_blob, "wrap_sqlite3_bind_blob", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_create_collation", VC(wrap_sqlite3_create_collation, "wrap_sqlite3_create_collation", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_collation_needed", VC(wrap_sqlite3_collation_needed, "wrap_sqlite3_collation_needed", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_total_changes", VC(wrap_sqlite3_total_changes, "wrap_sqlite3_total_changes", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_busy_timeout", VC(wrap_sqlite3_busy_timeout, "wrap_sqlite3_busy_timeout", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_int64", VC(wrap_sqlite3_bind_int64, "wrap_sqlite3_bind_int64", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_text16be", VC(wrap_sqlite3_value_text16be, "wrap_sqlite3_value_text16be", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_last_insert_rowid", VC(wrap_sqlite3_last_insert_rowid, "wrap_sqlite3_last_insert_rowid", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_int", VC(wrap_sqlite3_bind_int, "wrap_sqlite3_bind_int", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_parameter_name", VC(wrap_sqlite3_bind_parameter_name, "wrap_sqlite3_bind_parameter_name", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_bytes16", VC(wrap_sqlite3_column_bytes16, "wrap_sqlite3_column_bytes16", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_aggregate_count", VC(wrap_sqlite3_aggregate_count, "wrap_sqlite3_aggregate_count", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_text16", VC(wrap_sqlite3_value_text16, "wrap_sqlite3_value_text16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_error16", VC(wrap_sqlite3_result_error16, "wrap_sqlite3_result_error16", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_sleep", VC(wrap_sqlite3_sleep, "wrap_sqlite3_sleep", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_expired", VC(wrap_sqlite3_expired, "wrap_sqlite3_expired", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_get_autocommit", VC(wrap_sqlite3_get_autocommit, "wrap_sqlite3_get_autocommit", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_progress_handler", VC(wrap_sqlite3_progress_handler, "wrap_sqlite3_progress_handler", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_bind_text16", VC(wrap_sqlite3_bind_text16, "wrap_sqlite3_bind_text16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_type", VC(wrap_sqlite3_value_type, "wrap_sqlite3_value_type", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_type", VC(wrap_sqlite3_column_type, "wrap_sqlite3_column_type", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_thread_cleanup", VC(wrap_sqlite3_thread_cleanup, "wrap_sqlite3_thread_cleanup", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_parameter_index", VC(wrap_sqlite3_bind_parameter_index, "wrap_sqlite3_bind_parameter_index", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_set_auxdata", VC(wrap_sqlite3_set_auxdata, "wrap_sqlite3_set_auxdata", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_result_text16le", VC(wrap_sqlite3_result_text16le, "wrap_sqlite3_result_text16le", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_libversion_number", VC(wrap_sqlite3_libversion_number, "wrap_sqlite3_libversion_number", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_int", VC(wrap_sqlite3_column_int, "wrap_sqlite3_column_int", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_null", VC(wrap_sqlite3_result_null, "wrap_sqlite3_result_null", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_errcode", VC(wrap_sqlite3_errcode, "wrap_sqlite3_errcode", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_column_table_name", VC(wrap_sqlite3_column_table_name, "wrap_sqlite3_column_table_name", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_text", VC(wrap_sqlite3_value_text, "wrap_sqlite3_value_text", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_get_auxdata", VC(wrap_sqlite3_get_auxdata, "wrap_sqlite3_get_auxdata", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_value", VC(wrap_sqlite3_result_value, "wrap_sqlite3_result_value", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_rekey", VC(wrap_sqlite3_rekey, "wrap_sqlite3_rekey", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_complete16", VC(wrap_sqlite3_complete16, "wrap_sqlite3_complete16", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_create_function", VC(wrap_sqlite3_create_function, "wrap_sqlite3_create_function", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_key", VC(wrap_sqlite3_key, "wrap_sqlite3_key", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_profile", VC(wrap_sqlite3_profile, "wrap_sqlite3_profile", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_commit_hook", VC(wrap_sqlite3_commit_hook, "wrap_sqlite3_commit_hook", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_column_database_name", VC(wrap_sqlite3_column_database_name, "wrap_sqlite3_column_database_name", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_column_origin_name16", VC(wrap_sqlite3_column_origin_name16, "wrap_sqlite3_column_origin_name16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_text16", VC(wrap_sqlite3_column_text16, "wrap_sqlite3_column_text16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_db_handle", VC(wrap_sqlite3_db_handle, "wrap_sqlite3_db_handle", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_column_numeric_type", VC(wrap_sqlite3_column_numeric_type, "wrap_sqlite3_column_numeric_type", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_sqlite3_global_recover", VC(wrap_sqlite3_global_recover, "wrap_sqlite3_global_recover", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_open16", VC(wrap_sqlite3_open16, "wrap_sqlite3_open16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_int", VC(wrap_sqlite3_result_int, "wrap_sqlite3_result_int", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_decltype16", VC(wrap_sqlite3_column_decltype16, "wrap_sqlite3_column_decltype16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_int64", VC(wrap_sqlite3_column_int64, "wrap_sqlite3_column_int64", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_error", VC(wrap_sqlite3_result_error, "wrap_sqlite3_result_error", VC_FUNC_BUILTIN_LEAF));
}
