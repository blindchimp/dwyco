
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "vc.h"
#include "W_cvt_sqlite3.cpp"
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
static vc
wrap_sqlite3_result_value(VCArglist *a)
{
START_WRAP
	sqlite3_result_value(cvt_vc_p_sqlite3_context_((*a)[0]),
cvt_vc_p_Mem_((*a)[1]));
WRAP_END
return(vcnil);

}
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
static vc
wrap_sqlite3_column_numeric_type(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_column_numeric_type(cvt_vc_p_sqlite3_stmt_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
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
static vc
wrap_sqlite3_clear_bindings(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_clear_bindings(cvt_vc_p_sqlite3_stmt_((*a)[0])));
WRAP_END
return ret;

}
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
static vc
wrap_sqlite3_free(VCArglist *a)
{
START_WRAP
	sqlite3_free(cvt_vc_p_char((*a)[0]));
WRAP_END
return(vcnil);

}
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
static vc
wrap_sqlite3_free_table(VCArglist *a)
{
START_WRAP
	sqlite3_free_table(cvt_vc_pp_char((*a)[0]));
WRAP_END
return(vcnil);

}
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
static vc
wrap_sqlite3_busy_timeout(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_busy_timeout(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
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
static vc
wrap_sqlite3_exec(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_exec(cvt_vc_p_sqlite3_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_sqlite3_callback((*a)[2]),
cvt_vc_p_void((*a)[3]),
cvt_vc_pp_char((*a)[4])));
WRAP_END
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
wrap_sqlite3_libversion_number(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(sqlite3_libversion_number());
WRAP_END
return ret;

}
static vc
wrap_sqlite3_libversion(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_char(sqlite3_libversion());
WRAP_END
return ret;

}
ignoring function __builtin_expect 
ignoring function __builtin_prefetch 
ignoring function __builtin_return 
ignoring function __builtin_return_address 
ignoring function __builtin_frame_address 
ignoring function __builtin_nansl 
ignoring function __builtin_nansf 
ignoring function __builtin_nans 
ignoring function __builtin_infl 
ignoring function __builtin_inff 
ignoring function __builtin_inf 

static void
makefun(const char *name, const vc& fun)
{
    vc(name).bind(fun);
}
void
wrapper_init_sqlite3 ()
{

makefun("wrap_sqlite3_errmsg", vc(wrap_sqlite3_errmsg, "wrap_sqlite3_errmsg", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_text", vc(wrap_sqlite3_bind_text, "wrap_sqlite3_bind_text", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_busy_handler", vc(wrap_sqlite3_busy_handler, "wrap_sqlite3_busy_handler", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_blob", vc(wrap_sqlite3_column_blob, "wrap_sqlite3_column_blob", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_aggregate_context", vc(wrap_sqlite3_aggregate_context, "wrap_sqlite3_aggregate_context", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_count", vc(wrap_sqlite3_column_count, "wrap_sqlite3_column_count", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_decltype", vc(wrap_sqlite3_column_decltype, "wrap_sqlite3_column_decltype", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_step", vc(wrap_sqlite3_step, "wrap_sqlite3_step", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_finalize", vc(wrap_sqlite3_finalize, "wrap_sqlite3_finalize", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_text16be", vc(wrap_sqlite3_result_text16be, "wrap_sqlite3_result_text16be", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_enable_shared_cache", vc(wrap_sqlite3_enable_shared_cache, "wrap_sqlite3_enable_shared_cache", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_exec", vc(wrap_sqlite3_exec, "wrap_sqlite3_exec", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_bytes", vc(wrap_sqlite3_column_bytes, "wrap_sqlite3_column_bytes", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_double", vc(wrap_sqlite3_value_double, "wrap_sqlite3_value_double", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_text16", vc(wrap_sqlite3_result_text16, "wrap_sqlite3_result_text16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_free", vc(wrap_sqlite3_free, "wrap_sqlite3_free", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_set_authorizer", vc(wrap_sqlite3_set_authorizer, "wrap_sqlite3_set_authorizer", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_text", vc(wrap_sqlite3_column_text, "wrap_sqlite3_column_text", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_create_function16", vc(wrap_sqlite3_create_function16, "wrap_sqlite3_create_function16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_create_collation16", vc(wrap_sqlite3_create_collation16, "wrap_sqlite3_create_collation16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_collation_needed16", vc(wrap_sqlite3_collation_needed16, "wrap_sqlite3_collation_needed16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_reset", vc(wrap_sqlite3_reset, "wrap_sqlite3_reset", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_text16le", vc(wrap_sqlite3_value_text16le, "wrap_sqlite3_value_text16le", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_close", vc(wrap_sqlite3_close, "wrap_sqlite3_close", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_snprintf", vc(wrap_sqlite3_snprintf, "wrap_sqlite3_snprintf", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_double", vc(wrap_sqlite3_bind_double, "wrap_sqlite3_bind_double", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_value", vc(wrap_sqlite3_bind_value, "wrap_sqlite3_bind_value", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_blob", vc(wrap_sqlite3_value_blob, "wrap_sqlite3_value_blob", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_int64", vc(wrap_sqlite3_result_int64, "wrap_sqlite3_result_int64", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_changes", vc(wrap_sqlite3_changes, "wrap_sqlite3_changes", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_complete", vc(wrap_sqlite3_complete, "wrap_sqlite3_complete", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_parameter_count", vc(wrap_sqlite3_bind_parameter_count, "wrap_sqlite3_bind_parameter_count", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_double", vc(wrap_sqlite3_column_double, "wrap_sqlite3_column_double", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_bytes", vc(wrap_sqlite3_value_bytes, "wrap_sqlite3_value_bytes", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_blob", vc(wrap_sqlite3_result_blob, "wrap_sqlite3_result_blob", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_table_column_metadata", vc(wrap_sqlite3_table_column_metadata, "wrap_sqlite3_table_column_metadata", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_mprintf", vc(wrap_sqlite3_mprintf, "wrap_sqlite3_mprintf", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_name", vc(wrap_sqlite3_column_name, "wrap_sqlite3_column_name", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_user_data", vc(wrap_sqlite3_user_data, "wrap_sqlite3_user_data", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_interrupt", vc(wrap_sqlite3_interrupt, "wrap_sqlite3_interrupt", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_open", vc(wrap_sqlite3_open, "wrap_sqlite3_open", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_null", vc(wrap_sqlite3_bind_null, "wrap_sqlite3_bind_null", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_name16", vc(wrap_sqlite3_column_name16, "wrap_sqlite3_column_name16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_errmsg16", vc(wrap_sqlite3_errmsg16, "wrap_sqlite3_errmsg16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_prepare16", vc(wrap_sqlite3_prepare16, "wrap_sqlite3_prepare16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_clear_bindings", vc(wrap_sqlite3_clear_bindings, "wrap_sqlite3_clear_bindings", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_database_name16", vc(wrap_sqlite3_column_database_name16, "wrap_sqlite3_column_database_name16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_table_name16", vc(wrap_sqlite3_column_table_name16, "wrap_sqlite3_column_table_name16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_text", vc(wrap_sqlite3_result_text, "wrap_sqlite3_result_text", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_transfer_bindings", vc(wrap_sqlite3_transfer_bindings, "wrap_sqlite3_transfer_bindings", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_update_hook", vc(wrap_sqlite3_update_hook, "wrap_sqlite3_update_hook", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_soft_heap_limit", vc(wrap_sqlite3_soft_heap_limit, "wrap_sqlite3_soft_heap_limit", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_trace", vc(wrap_sqlite3_trace, "wrap_sqlite3_trace", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_origin_name", vc(wrap_sqlite3_column_origin_name, "wrap_sqlite3_column_origin_name", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_data_count", vc(wrap_sqlite3_data_count, "wrap_sqlite3_data_count", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_bytes16", vc(wrap_sqlite3_value_bytes16, "wrap_sqlite3_value_bytes16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_int", vc(wrap_sqlite3_value_int, "wrap_sqlite3_value_int", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_numeric_type", vc(wrap_sqlite3_value_numeric_type, "wrap_sqlite3_value_numeric_type", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_release_memory", vc(wrap_sqlite3_release_memory, "wrap_sqlite3_release_memory", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_prepare", vc(wrap_sqlite3_prepare, "wrap_sqlite3_prepare", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_libversion", vc(wrap_sqlite3_libversion, "wrap_sqlite3_libversion", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_get_table", vc(wrap_sqlite3_get_table, "wrap_sqlite3_get_table", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_free_table", vc(wrap_sqlite3_free_table, "wrap_sqlite3_free_table", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_int64", vc(wrap_sqlite3_value_int64, "wrap_sqlite3_value_int64", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_double", vc(wrap_sqlite3_result_double, "wrap_sqlite3_result_double", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_rollback_hook", vc(wrap_sqlite3_rollback_hook, "wrap_sqlite3_rollback_hook", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_vmprintf", vc(wrap_sqlite3_vmprintf, "wrap_sqlite3_vmprintf", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_blob", vc(wrap_sqlite3_bind_blob, "wrap_sqlite3_bind_blob", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_create_collation", vc(wrap_sqlite3_create_collation, "wrap_sqlite3_create_collation", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_collation_needed", vc(wrap_sqlite3_collation_needed, "wrap_sqlite3_collation_needed", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_total_changes", vc(wrap_sqlite3_total_changes, "wrap_sqlite3_total_changes", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_busy_timeout", vc(wrap_sqlite3_busy_timeout, "wrap_sqlite3_busy_timeout", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_int64", vc(wrap_sqlite3_bind_int64, "wrap_sqlite3_bind_int64", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_text16be", vc(wrap_sqlite3_value_text16be, "wrap_sqlite3_value_text16be", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_last_insert_rowid", vc(wrap_sqlite3_last_insert_rowid, "wrap_sqlite3_last_insert_rowid", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_int", vc(wrap_sqlite3_bind_int, "wrap_sqlite3_bind_int", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_parameter_name", vc(wrap_sqlite3_bind_parameter_name, "wrap_sqlite3_bind_parameter_name", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_bytes16", vc(wrap_sqlite3_column_bytes16, "wrap_sqlite3_column_bytes16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_aggregate_count", vc(wrap_sqlite3_aggregate_count, "wrap_sqlite3_aggregate_count", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_text16", vc(wrap_sqlite3_value_text16, "wrap_sqlite3_value_text16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_error16", vc(wrap_sqlite3_result_error16, "wrap_sqlite3_result_error16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_sleep", vc(wrap_sqlite3_sleep, "wrap_sqlite3_sleep", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_expired", vc(wrap_sqlite3_expired, "wrap_sqlite3_expired", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_get_autocommit", vc(wrap_sqlite3_get_autocommit, "wrap_sqlite3_get_autocommit", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_progress_handler", vc(wrap_sqlite3_progress_handler, "wrap_sqlite3_progress_handler", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_text16", vc(wrap_sqlite3_bind_text16, "wrap_sqlite3_bind_text16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_type", vc(wrap_sqlite3_value_type, "wrap_sqlite3_value_type", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_type", vc(wrap_sqlite3_column_type, "wrap_sqlite3_column_type", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_thread_cleanup", vc(wrap_sqlite3_thread_cleanup, "wrap_sqlite3_thread_cleanup", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_bind_parameter_index", vc(wrap_sqlite3_bind_parameter_index, "wrap_sqlite3_bind_parameter_index", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_set_auxdata", vc(wrap_sqlite3_set_auxdata, "wrap_sqlite3_set_auxdata", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_text16le", vc(wrap_sqlite3_result_text16le, "wrap_sqlite3_result_text16le", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_libversion_number", vc(wrap_sqlite3_libversion_number, "wrap_sqlite3_libversion_number", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_int", vc(wrap_sqlite3_column_int, "wrap_sqlite3_column_int", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_null", vc(wrap_sqlite3_result_null, "wrap_sqlite3_result_null", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_errcode", vc(wrap_sqlite3_errcode, "wrap_sqlite3_errcode", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_table_name", vc(wrap_sqlite3_column_table_name, "wrap_sqlite3_column_table_name", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_value_text", vc(wrap_sqlite3_value_text, "wrap_sqlite3_value_text", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_get_auxdata", vc(wrap_sqlite3_get_auxdata, "wrap_sqlite3_get_auxdata", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_value", vc(wrap_sqlite3_result_value, "wrap_sqlite3_result_value", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_rekey", vc(wrap_sqlite3_rekey, "wrap_sqlite3_rekey", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_complete16", vc(wrap_sqlite3_complete16, "wrap_sqlite3_complete16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_create_function", vc(wrap_sqlite3_create_function, "wrap_sqlite3_create_function", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_key", vc(wrap_sqlite3_key, "wrap_sqlite3_key", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_profile", vc(wrap_sqlite3_profile, "wrap_sqlite3_profile", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_commit_hook", vc(wrap_sqlite3_commit_hook, "wrap_sqlite3_commit_hook", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_database_name", vc(wrap_sqlite3_column_database_name, "wrap_sqlite3_column_database_name", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_origin_name16", vc(wrap_sqlite3_column_origin_name16, "wrap_sqlite3_column_origin_name16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_text16", vc(wrap_sqlite3_column_text16, "wrap_sqlite3_column_text16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_db_handle", vc(wrap_sqlite3_db_handle, "wrap_sqlite3_db_handle", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_numeric_type", vc(wrap_sqlite3_column_numeric_type, "wrap_sqlite3_column_numeric_type", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_global_recover", vc(wrap_sqlite3_global_recover, "wrap_sqlite3_global_recover", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_open16", vc(wrap_sqlite3_open16, "wrap_sqlite3_open16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_int", vc(wrap_sqlite3_result_int, "wrap_sqlite3_result_int", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_decltype16", vc(wrap_sqlite3_column_decltype16, "wrap_sqlite3_column_decltype16", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_column_int64", vc(wrap_sqlite3_column_int64, "wrap_sqlite3_column_int64", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_sqlite3_result_error", vc(wrap_sqlite3_result_error, "wrap_sqlite3_result_error", VC_FUNC_BUILTIN_LEAF));
}
