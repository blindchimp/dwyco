
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "vc.h"
#include "W_cvt_libuv.xml.cpp"
static vc
wrap_uv_run_once(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_run_once(cvt_vc_p_uv_loop_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_handle_size(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(uv_handle_size(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_udp_recv_stop(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_udp_recv_stop(cvt_vc_p_uv_udp_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_shutdown(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_shutdown(cvt_vc_p_uv_shutdown_s_((*a)[0]),
cvt_vc_p_uv_stream_s_((*a)[1]),
cvt_vc_pF__void_p_uv_shutdown_s__x__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_strlcpy(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(uv_strlcpy(cvt_vc_p_char((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_sem_trywait(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_sem_trywait(cvt_vc_p_sem_t_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_rwlock_destroy(VCArglist *a)
{
START_WRAP
	uv_rwlock_destroy(cvt_vc_p_pthread_rwlock_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_buf_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__uv_buf_t_(uv_buf_init(cvt_vc_p_char((*a)[0]),
cvt_vc__unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_check_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_check_init(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_check_s_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_tcp_keepalive(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_tcp_keepalive(cvt_vc_p_uv_tcp_s_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__unsigned_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_rwlock_trywrlock(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_rwlock_trywrlock(cvt_vc_p_pthread_rwlock_t_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_free_cpu_info(VCArglist *a)
{
START_WRAP
	uv_free_cpu_info(cvt_vc_p_uv_cpu_info_s_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_getaddrinfo(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_getaddrinfo(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_getaddrinfo_s_((*a)[1]),
cvt_vc_pF__void_p_uv_getaddrinfo_s__x__int_x_p_addrinfo_((*a)[2]),
cvt_vc_pq_char((*a)[3]),
cvt_vc_pq_char((*a)[4]),
cvt_vc_pq_addrinfo_((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_uv_check_start(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_check_start(cvt_vc_p_uv_check_s_((*a)[0]),
cvt_vc_pF__void_p_uv_check_s__x__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_pipe_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_pipe_init(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_pipe_s_((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_chmod(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_chmod(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_uv_tty_get_winsize(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_tty_get_winsize(cvt_vc_p_uv_tty_s_((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc_p_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_process_kill(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_process_kill(cvt_vc_p_uv_process_s_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_freeaddrinfo(VCArglist *a)
{
START_WRAP
	uv_freeaddrinfo(cvt_vc_p_addrinfo_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_last_error(VCArglist *a)
{
START_WRAP
	vc ret = cvt__uv_err_s_(uv_last_error(cvt_vc_p_uv_loop_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_tty_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_tty_init(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_tty_s_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_close(VCArglist *a)
{
START_WRAP
	uv_close(cvt_vc_p_uv_handle_s_((*a)[0]),
cvt_vc_pF__void_p_uv_handle_s_((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_dlerror(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_char(uv_dlerror(cvt_vc_p_uv_lib_t_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_udp_set_ttl(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_udp_set_ttl(cvt_vc_p_uv_udp_s_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_udp_set_multicast_ttl(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_udp_set_multicast_ttl(cvt_vc_p_uv_udp_s_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_readlink(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_readlink(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_poll_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_poll_init(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_poll_s_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_mutex_trylock(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_mutex_trylock(cvt_vc_p_pthread_mutex_t_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_req_cleanup(VCArglist *a)
{
START_WRAP
	uv_fs_req_cleanup(cvt_vc_p_uv_fs_s_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_hrtime(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(uv_hrtime());
WRAP_END
return ret;

}
static vc
wrap_uv_once(VCArglist *a)
{
START_WRAP
	uv_once(cvt_vc_p_int((*a)[0]),
cvt_vc_pF__void_((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_inet_pton(VCArglist *a)
{
START_WRAP
	vc ret = cvt__uv_err_s_(uv_inet_pton(cvt_vc__int((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_p_void((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_rwlock_wrlock(VCArglist *a)
{
START_WRAP
	uv_rwlock_wrlock(cvt_vc_p_pthread_rwlock_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_resident_set_memory(VCArglist *a)
{
START_WRAP
	vc ret = cvt__uv_err_s_(uv_resident_set_memory(cvt_vc_p_long_unsigned_int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_chdir(VCArglist *a)
{
START_WRAP
	vc ret = cvt__uv_err_s_(uv_chdir(cvt_vc_pq_char((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_cond_destroy(VCArglist *a)
{
START_WRAP
	uv_cond_destroy(cvt_vc_p_pthread_cond_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_mutex_destroy(VCArglist *a)
{
START_WRAP
	uv_mutex_destroy(cvt_vc_p_pthread_mutex_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_dlsym(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_dlsym(cvt_vc_p_uv_lib_t_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_pp_void((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_mutex_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_mutex_init(cvt_vc_p_pthread_mutex_t_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_cond_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_cond_init(cvt_vc_p_pthread_cond_t_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_readdir(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_readdir(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_uv_tcp_nodelay(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_tcp_nodelay(cvt_vc_p_uv_tcp_s_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_rmdir(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_rmdir(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_udp_getsockname(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_udp_getsockname(cvt_vc_p_uv_udp_s_((*a)[0]),
cvt_vc_p_sockaddr_((*a)[1]),
cvt_vc_p_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_fchown(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_fchown(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_uv_free_interface_addresses(VCArglist *a)
{
START_WRAP
	uv_free_interface_addresses(cvt_vc_p_uv_interface_address_s_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_timer_get_repeat(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_int(uv_timer_get_repeat(cvt_vc_p_uv_timer_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_tcp_connect(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_tcp_connect(cvt_vc_p_uv_connect_s_((*a)[0]),
cvt_vc_p_uv_tcp_s_((*a)[1]),
cvt_vc__sockaddr_in_((*a)[2]),
cvt_vc_pF__void_p_uv_connect_s__x__int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_get_process_title(VCArglist *a)
{
START_WRAP
	vc ret = cvt__uv_err_s_(uv_get_process_title(cvt_vc_p_char((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_check_stop(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_check_stop(cvt_vc_p_uv_check_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_thread_self(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(uv_thread_self());
WRAP_END
return ret;

}
static vc
wrap_uv_loop_new(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_uv_loop_s_(uv_loop_new());
WRAP_END
return ret;

}
static vc
wrap_uv_timer_again(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_timer_again(cvt_vc_p_uv_timer_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_read_start(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_read_start(cvt_vc_p_uv_stream_s_((*a)[0]),
cvt_vc_pF__uv_buf_t__p_uv_handle_s__x__long_unsigned_int((*a)[1]),
cvt_vc_pF__void_p_uv_stream_s__x__long_int_x__uv_buf_t_((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_dlclose(VCArglist *a)
{
START_WRAP
	uv_dlclose(cvt_vc_p_uv_lib_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_tcp_getpeername(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_tcp_getpeername(cvt_vc_p_uv_tcp_s_((*a)[0]),
cvt_vc_p_sockaddr_((*a)[1]),
cvt_vc_p_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_dlopen(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_dlopen(cvt_vc_pq_char((*a)[0]),
cvt_vc_p_uv_lib_t_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_tcp_bind6(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_tcp_bind6(cvt_vc_p_uv_tcp_s_((*a)[0]),
cvt_vc__sockaddr_in6_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_fstat(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_fstat(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_udp_send(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_udp_send(cvt_vc_p_uv_udp_send_s_((*a)[0]),
cvt_vc_p_uv_udp_s_((*a)[1]),
cvt_vc_p_uv_buf_t_((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__sockaddr_in_((*a)[4]),
cvt_vc_pF__void_p_uv_udp_send_s__x__int((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_uv_pipe_bind(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_pipe_bind(cvt_vc_p_uv_pipe_s_((*a)[0]),
cvt_vc_pq_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_disable_stdio_inheritance(VCArglist *a)
{
START_WRAP
	uv_disable_stdio_inheritance();
WRAP_END
return(vcnil);

}
static vc
wrap_uv_read_stop(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_read_stop(cvt_vc_p_uv_stream_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_sem_wait(VCArglist *a)
{
START_WRAP
	uv_sem_wait(cvt_vc_p_sem_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_udp_set_multicast_loop(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_udp_set_multicast_loop(cvt_vc_p_uv_udp_s_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_thread_join(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_thread_join(cvt_vc_p_long_unsigned_int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_poll_init_socket(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_poll_init_socket(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_poll_s_((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_poll_stop(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_poll_stop(cvt_vc_p_uv_fs_poll_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_setup_args(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pp_char(uv_setup_args(cvt_vc__int((*a)[0]),
cvt_vc_pp_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_kill(VCArglist *a)
{
START_WRAP
	vc ret = cvt__uv_err_s_(uv_kill(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_exepath(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_exepath(cvt_vc_p_char((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_write(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_write(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_void((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]),
cvt_vc__long_int((*a)[5]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[6])));
WRAP_END
return ret;

}
static vc
wrap_uv_udp_bind(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_udp_bind(cvt_vc_p_uv_udp_s_((*a)[0]),
cvt_vc__sockaddr_in_((*a)[1]),
cvt_vc__unsigned_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_sem_destroy(VCArglist *a)
{
START_WRAP
	uv_sem_destroy(cvt_vc_p_sem_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_is_writable(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_is_writable(cvt_vc_pq_uv_stream_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_cond_broadcast(VCArglist *a)
{
START_WRAP
	uv_cond_broadcast(cvt_vc_p_pthread_cond_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_udp_set_membership(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_udp_set_membership(cvt_vc_p_uv_udp_s_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_inet_ntop(VCArglist *a)
{
START_WRAP
	vc ret = cvt__uv_err_s_(uv_inet_ntop(cvt_vc__int((*a)[0]),
cvt_vc_pq_void((*a)[1]),
cvt_vc_p_char((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_async_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_async_init(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_async_s_((*a)[1]),
cvt_vc_pF__void_p_uv_async_s__x__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_strerror(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_char(uv_strerror(cvt_vc__uv_err_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_get_total_memory(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(uv_get_total_memory());
WRAP_END
return ret;

}
static vc
wrap_uv_fs_rename(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_rename(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc_pq_char((*a)[3]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_unlink(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_unlink(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_futime(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_futime(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__double((*a)[3]),
cvt_vc__double((*a)[4]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_uv_tty_reset_mode(VCArglist *a)
{
START_WRAP
	uv_tty_reset_mode();
WRAP_END
return(vcnil);

}
static vc
wrap_uv_fs_sendfile(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_sendfile(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__long_int((*a)[4]),
cvt_vc__long_unsigned_int((*a)[5]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[6])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_open(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_open(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_symlink(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_symlink(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc_pq_char((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_uv_ip6_addr(VCArglist *a)
{
START_WRAP
	vc ret = cvt__sockaddr_in6_(uv_ip6_addr(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_uptime(VCArglist *a)
{
START_WRAP
	vc ret = cvt__uv_err_s_(uv_uptime(cvt_vc_p_double((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_rwlock_rdlock(VCArglist *a)
{
START_WRAP
	uv_rwlock_rdlock(cvt_vc_p_pthread_rwlock_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_rwlock_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_rwlock_init(cvt_vc_p_pthread_rwlock_t_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_read(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_read(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_void((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]),
cvt_vc__long_int((*a)[5]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[6])));
WRAP_END
return ret;

}
static vc
wrap_uv_sem_post(VCArglist *a)
{
START_WRAP
	uv_sem_post(cvt_vc_p_sem_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_udp_set_broadcast(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_udp_set_broadcast(cvt_vc_p_uv_udp_s_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_async_send(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_async_send(cvt_vc_p_uv_async_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_link(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_link(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc_pq_char((*a)[3]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_uv_timer_set_repeat(VCArglist *a)
{
START_WRAP
	uv_timer_set_repeat(cvt_vc_p_uv_timer_s_((*a)[0]),
cvt_vc__long_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_fs_mkdir(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_mkdir(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_uv_req_size(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(uv_req_size(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_utime(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_utime(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__double((*a)[3]),
cvt_vc__double((*a)[4]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_uv_is_active(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_is_active(cvt_vc_pq_uv_handle_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_pipe_connect(VCArglist *a)
{
START_WRAP
	uv_pipe_connect(cvt_vc_p_uv_connect_s_((*a)[0]),
cvt_vc_p_uv_pipe_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc_pF__void_p_uv_connect_s__x__int((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_timer_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_timer_init(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_timer_s_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_rwlock_tryrdlock(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_rwlock_tryrdlock(cvt_vc_p_pthread_rwlock_t_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_update_time(VCArglist *a)
{
START_WRAP
	uv_update_time(cvt_vc_p_uv_loop_s_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_err_name(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pq_char(uv_err_name(cvt_vc__uv_err_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_cond_signal(VCArglist *a)
{
START_WRAP
	uv_cond_signal(cvt_vc_p_pthread_cond_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_fs_chown(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_chown(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_uv_is_closing(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_is_closing(cvt_vc_pq_uv_handle_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_timer_start(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_timer_start(cvt_vc_p_uv_timer_s_((*a)[0]),
cvt_vc_pF__void_p_uv_timer_s__x__int((*a)[1]),
cvt_vc__long_int((*a)[2]),
cvt_vc__long_int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_cond_wait(VCArglist *a)
{
START_WRAP
	uv_cond_wait(cvt_vc_p_pthread_cond_t_((*a)[0]),
cvt_vc_p_pthread_mutex_t_((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_write2(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_write2(cvt_vc_p_uv_write_s_((*a)[0]),
cvt_vc_p_uv_stream_s_((*a)[1]),
cvt_vc_p_uv_buf_t_((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_p_uv_stream_s_((*a)[4]),
cvt_vc_pF__void_p_uv_write_s__x__int((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_fchmod(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_fchmod(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_uv_read2_start(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_read2_start(cvt_vc_p_uv_stream_s_((*a)[0]),
cvt_vc_pF__uv_buf_t__p_uv_handle_s__x__long_unsigned_int((*a)[1]),
cvt_vc_pF__void_p_uv_pipe_s__x__long_int_x__uv_buf_t__x__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_prepare_start(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_prepare_start(cvt_vc_p_uv_prepare_s_((*a)[0]),
cvt_vc_pF__void_p_uv_prepare_s__x__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_event_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_event_init(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_event_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc_pF__void_p_uv_fs_event_s__x_pq_char_x__int_x__int((*a)[3]),
cvt_vc__int((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_uv_poll_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_poll_init(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_poll_s_((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_tcp_simultaneous_accepts(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_tcp_simultaneous_accepts(cvt_vc_p_uv_tcp_s_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_udp_bind6(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_udp_bind6(cvt_vc_p_uv_udp_s_((*a)[0]),
cvt_vc__sockaddr_in6_((*a)[1]),
cvt_vc__unsigned_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_ip4_name(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_ip4_name(cvt_vc_p_sockaddr_in_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_write(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_write(cvt_vc_p_uv_write_s_((*a)[0]),
cvt_vc_p_uv_stream_s_((*a)[1]),
cvt_vc_p_uv_buf_t_((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_pF__void_p_uv_write_s__x__int((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_uv_rwlock_rdunlock(VCArglist *a)
{
START_WRAP
	uv_rwlock_rdunlock(cvt_vc_p_pthread_rwlock_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_udp_send6(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_udp_send6(cvt_vc_p_uv_udp_send_s_((*a)[0]),
cvt_vc_p_uv_udp_s_((*a)[1]),
cvt_vc_p_uv_buf_t_((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__sockaddr_in6_((*a)[4]),
cvt_vc_pF__void_p_uv_udp_send_s__x__int((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_uv_poll_stop(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_poll_stop(cvt_vc_p_uv_poll_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_timer_stop(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_timer_stop(cvt_vc_p_uv_timer_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_accept(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_accept(cvt_vc_p_uv_stream_s_((*a)[0]),
cvt_vc_p_uv_stream_s_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_cond_timedwait(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_cond_timedwait(cvt_vc_p_pthread_cond_t_((*a)[0]),
cvt_vc_p_pthread_mutex_t_((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_guess_handle(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_guess_handle(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_prepare_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_prepare_init(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_prepare_s_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_pipe_open(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_pipe_open(cvt_vc_p_uv_pipe_s_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_fsync(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_fsync(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_close(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_close(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_poll_start(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_poll_start(cvt_vc_p_uv_fs_poll_s_((*a)[0]),
cvt_vc_pF__void_p_uv_fs_poll_s__x__int_x_pq_stat__x_pq_stat_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__unsigned_int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_poll_start(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_poll_start(cvt_vc_p_uv_poll_s_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pF__void_p_uv_poll_s__x__int_x__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_now(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_int(uv_now(cvt_vc_p_uv_loop_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_sem_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_sem_init(cvt_vc_p_sem_t_((*a)[0]),
cvt_vc__unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_cwd(VCArglist *a)
{
START_WRAP
	vc ret = cvt__uv_err_s_(uv_cwd(cvt_vc_p_char((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_signal_start(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_signal_start(cvt_vc_p_uv_signal_s_((*a)[0]),
cvt_vc_pF__void_p_uv_signal_s__x__int((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_udp_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_udp_init(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_udp_s_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_mutex_lock(VCArglist *a)
{
START_WRAP
	uv_mutex_lock(cvt_vc_p_pthread_mutex_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_tcp_connect6(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_tcp_connect6(cvt_vc_p_uv_connect_s_((*a)[0]),
cvt_vc_p_uv_tcp_s_((*a)[1]),
cvt_vc__sockaddr_in6_((*a)[2]),
cvt_vc_pF__void_p_uv_connect_s__x__int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_idle_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_idle_init(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_idle_s_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_signal_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_signal_init(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_signal_s_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_ref(VCArglist *a)
{
START_WRAP
	uv_ref(cvt_vc_p_uv_handle_s_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_listen(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_listen(cvt_vc_p_uv_stream_s_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pF__void_p_uv_stream_s__x__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_unref(VCArglist *a)
{
START_WRAP
	uv_unref(cvt_vc_p_uv_handle_s_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_fs_ftruncate(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_ftruncate(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__long_int((*a)[3]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_uv_mutex_unlock(VCArglist *a)
{
START_WRAP
	uv_mutex_unlock(cvt_vc_p_pthread_mutex_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_cpu_info(VCArglist *a)
{
START_WRAP
	vc ret = cvt__uv_err_s_(uv_cpu_info(cvt_vc_pp_uv_cpu_info_s_((*a)[0]),
cvt_vc_p_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_ip4_addr(VCArglist *a)
{
START_WRAP
	vc ret = cvt__sockaddr_in_(uv_ip4_addr(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_default_loop(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_uv_loop_s_(uv_default_loop());
WRAP_END
return ret;

}
static vc
wrap_uv_rwlock_wrunlock(VCArglist *a)
{
START_WRAP
	uv_rwlock_wrunlock(cvt_vc_p_pthread_rwlock_t_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_thread_create(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_thread_create(cvt_vc_p_long_unsigned_int((*a)[0]),
cvt_vc_pF__void_p_void((*a)[1]),
cvt_vc_p_void((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_udp_open(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_udp_open(cvt_vc_p_uv_udp_s_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_run(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_run(cvt_vc_p_uv_loop_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_prepare_stop(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_prepare_stop(cvt_vc_p_uv_prepare_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_set_process_title(VCArglist *a)
{
START_WRAP
	vc ret = cvt__uv_err_s_(uv_set_process_title(cvt_vc_pq_char((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_spawn(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_spawn(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_process_s_((*a)[1]),
cvt_vc__uv_process_options_s_((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_tcp_init(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_tcp_init(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_tcp_s_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_ip6_name(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_ip6_name(cvt_vc_p_sockaddr_in6_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_tcp_bind(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_tcp_bind(cvt_vc_p_uv_tcp_s_((*a)[0]),
cvt_vc__sockaddr_in_((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_lstat(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_lstat(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_udp_recv_start(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_udp_recv_start(cvt_vc_p_uv_udp_s_((*a)[0]),
cvt_vc_pF__uv_buf_t__p_uv_handle_s__x__long_unsigned_int((*a)[1]),
cvt_vc_pF__void_p_uv_udp_s__x__long_int_x__uv_buf_t__x_p_sockaddr__x__unsigned_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_fs_fdatasync(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_fdatasync(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_is_readable(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_is_readable(cvt_vc_pq_uv_stream_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_tty_set_mode(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_tty_set_mode(cvt_vc_p_uv_tty_s_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_tcp_open(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_tcp_open(cvt_vc_p_uv_tcp_s_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_idle_start(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_idle_start(cvt_vc_p_uv_idle_s_((*a)[0]),
cvt_vc_pF__void_p_uv_idle_s__x__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_interface_addresses(VCArglist *a)
{
START_WRAP
	vc ret = cvt__uv_err_s_(uv_interface_addresses(cvt_vc_pp_uv_interface_address_s_((*a)[0]),
cvt_vc_p_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_uv_queue_work(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_queue_work(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_work_s_((*a)[1]),
cvt_vc_pF__void_p_uv_work_s_((*a)[2]),
cvt_vc_pF__void_p_uv_work_s_((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_idle_stop(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_idle_stop(cvt_vc_p_uv_idle_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_strlcat(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(uv_strlcat(cvt_vc_p_char((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_pipe_pending_instances(VCArglist *a)
{
START_WRAP
	uv_pipe_pending_instances(cvt_vc_p_uv_pipe_s_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_fs_stat(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_fs_stat(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_p_uv_fs_s_((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc_pF__void_p_uv_fs_s_((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_uv_loop_delete(VCArglist *a)
{
START_WRAP
	uv_loop_delete(cvt_vc_p_uv_loop_s_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_tcp_getsockname(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_tcp_getsockname(cvt_vc_p_uv_tcp_s_((*a)[0]),
cvt_vc_p_sockaddr_((*a)[1]),
cvt_vc_p_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_uv_loadavg(VCArglist *a)
{
START_WRAP
	uv_loadavg(cvt_vc_p_double((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_signal_stop(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(uv_signal_stop(cvt_vc_p_uv_signal_s_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_uv_walk(VCArglist *a)
{
START_WRAP
	uv_walk(cvt_vc_p_uv_loop_s_((*a)[0]),
cvt_vc_pF__void_p_uv_handle_s__x_p_void((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_uv_get_free_memory(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(uv_get_free_memory());
WRAP_END
return ret;

}

static void
makefun(const char *name, const vc& fun)
{
    vc(name).bind(fun);
}
void
wrapper_init_foo_xml ()
{

makefun("wrap_uv_accept", vc(wrap_uv_accept, "wrap_uv_accept", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_async_init", vc(wrap_uv_async_init, "wrap_uv_async_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_async_send", vc(wrap_uv_async_send, "wrap_uv_async_send", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_buf_init", vc(wrap_uv_buf_init, "wrap_uv_buf_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_chdir", vc(wrap_uv_chdir, "wrap_uv_chdir", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_check_init", vc(wrap_uv_check_init, "wrap_uv_check_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_check_start", vc(wrap_uv_check_start, "wrap_uv_check_start", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_check_stop", vc(wrap_uv_check_stop, "wrap_uv_check_stop", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_close", vc(wrap_uv_close, "wrap_uv_close", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_cond_broadcast", vc(wrap_uv_cond_broadcast, "wrap_uv_cond_broadcast", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_cond_destroy", vc(wrap_uv_cond_destroy, "wrap_uv_cond_destroy", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_cond_init", vc(wrap_uv_cond_init, "wrap_uv_cond_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_cond_signal", vc(wrap_uv_cond_signal, "wrap_uv_cond_signal", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_cond_timedwait", vc(wrap_uv_cond_timedwait, "wrap_uv_cond_timedwait", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_cond_wait", vc(wrap_uv_cond_wait, "wrap_uv_cond_wait", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_cpu_info", vc(wrap_uv_cpu_info, "wrap_uv_cpu_info", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_cwd", vc(wrap_uv_cwd, "wrap_uv_cwd", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_default_loop", vc(wrap_uv_default_loop, "wrap_uv_default_loop", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_disable_stdio_inheritance", vc(wrap_uv_disable_stdio_inheritance, "wrap_uv_disable_stdio_inheritance", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_dlclose", vc(wrap_uv_dlclose, "wrap_uv_dlclose", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_dlerror", vc(wrap_uv_dlerror, "wrap_uv_dlerror", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_dlopen", vc(wrap_uv_dlopen, "wrap_uv_dlopen", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_dlsym", vc(wrap_uv_dlsym, "wrap_uv_dlsym", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_err_name", vc(wrap_uv_err_name, "wrap_uv_err_name", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_exepath", vc(wrap_uv_exepath, "wrap_uv_exepath", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_free_cpu_info", vc(wrap_uv_free_cpu_info, "wrap_uv_free_cpu_info", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_free_interface_addresses", vc(wrap_uv_free_interface_addresses, "wrap_uv_free_interface_addresses", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_freeaddrinfo", vc(wrap_uv_freeaddrinfo, "wrap_uv_freeaddrinfo", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_chmod", vc(wrap_uv_fs_chmod, "wrap_uv_fs_chmod", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_chown", vc(wrap_uv_fs_chown, "wrap_uv_fs_chown", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_close", vc(wrap_uv_fs_close, "wrap_uv_fs_close", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_event_init", vc(wrap_uv_fs_event_init, "wrap_uv_fs_event_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_fchmod", vc(wrap_uv_fs_fchmod, "wrap_uv_fs_fchmod", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_fchown", vc(wrap_uv_fs_fchown, "wrap_uv_fs_fchown", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_fdatasync", vc(wrap_uv_fs_fdatasync, "wrap_uv_fs_fdatasync", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_fstat", vc(wrap_uv_fs_fstat, "wrap_uv_fs_fstat", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_fsync", vc(wrap_uv_fs_fsync, "wrap_uv_fs_fsync", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_ftruncate", vc(wrap_uv_fs_ftruncate, "wrap_uv_fs_ftruncate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_futime", vc(wrap_uv_fs_futime, "wrap_uv_fs_futime", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_link", vc(wrap_uv_fs_link, "wrap_uv_fs_link", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_lstat", vc(wrap_uv_fs_lstat, "wrap_uv_fs_lstat", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_mkdir", vc(wrap_uv_fs_mkdir, "wrap_uv_fs_mkdir", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_open", vc(wrap_uv_fs_open, "wrap_uv_fs_open", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_poll_init", vc(wrap_uv_fs_poll_init, "wrap_uv_fs_poll_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_poll_start", vc(wrap_uv_fs_poll_start, "wrap_uv_fs_poll_start", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_poll_stop", vc(wrap_uv_fs_poll_stop, "wrap_uv_fs_poll_stop", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_read", vc(wrap_uv_fs_read, "wrap_uv_fs_read", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_readdir", vc(wrap_uv_fs_readdir, "wrap_uv_fs_readdir", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_readlink", vc(wrap_uv_fs_readlink, "wrap_uv_fs_readlink", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_rename", vc(wrap_uv_fs_rename, "wrap_uv_fs_rename", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_req_cleanup", vc(wrap_uv_fs_req_cleanup, "wrap_uv_fs_req_cleanup", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_rmdir", vc(wrap_uv_fs_rmdir, "wrap_uv_fs_rmdir", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_sendfile", vc(wrap_uv_fs_sendfile, "wrap_uv_fs_sendfile", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_stat", vc(wrap_uv_fs_stat, "wrap_uv_fs_stat", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_symlink", vc(wrap_uv_fs_symlink, "wrap_uv_fs_symlink", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_unlink", vc(wrap_uv_fs_unlink, "wrap_uv_fs_unlink", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_utime", vc(wrap_uv_fs_utime, "wrap_uv_fs_utime", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_fs_write", vc(wrap_uv_fs_write, "wrap_uv_fs_write", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_get_free_memory", vc(wrap_uv_get_free_memory, "wrap_uv_get_free_memory", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_get_process_title", vc(wrap_uv_get_process_title, "wrap_uv_get_process_title", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_get_total_memory", vc(wrap_uv_get_total_memory, "wrap_uv_get_total_memory", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_getaddrinfo", vc(wrap_uv_getaddrinfo, "wrap_uv_getaddrinfo", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_guess_handle", vc(wrap_uv_guess_handle, "wrap_uv_guess_handle", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_handle_size", vc(wrap_uv_handle_size, "wrap_uv_handle_size", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_hrtime", vc(wrap_uv_hrtime, "wrap_uv_hrtime", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_idle_init", vc(wrap_uv_idle_init, "wrap_uv_idle_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_idle_start", vc(wrap_uv_idle_start, "wrap_uv_idle_start", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_idle_stop", vc(wrap_uv_idle_stop, "wrap_uv_idle_stop", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_inet_ntop", vc(wrap_uv_inet_ntop, "wrap_uv_inet_ntop", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_inet_pton", vc(wrap_uv_inet_pton, "wrap_uv_inet_pton", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_interface_addresses", vc(wrap_uv_interface_addresses, "wrap_uv_interface_addresses", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_ip4_addr", vc(wrap_uv_ip4_addr, "wrap_uv_ip4_addr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_ip4_name", vc(wrap_uv_ip4_name, "wrap_uv_ip4_name", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_ip6_addr", vc(wrap_uv_ip6_addr, "wrap_uv_ip6_addr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_ip6_name", vc(wrap_uv_ip6_name, "wrap_uv_ip6_name", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_is_active", vc(wrap_uv_is_active, "wrap_uv_is_active", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_is_closing", vc(wrap_uv_is_closing, "wrap_uv_is_closing", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_is_readable", vc(wrap_uv_is_readable, "wrap_uv_is_readable", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_is_writable", vc(wrap_uv_is_writable, "wrap_uv_is_writable", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_kill", vc(wrap_uv_kill, "wrap_uv_kill", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_last_error", vc(wrap_uv_last_error, "wrap_uv_last_error", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_listen", vc(wrap_uv_listen, "wrap_uv_listen", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_loadavg", vc(wrap_uv_loadavg, "wrap_uv_loadavg", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_loop_delete", vc(wrap_uv_loop_delete, "wrap_uv_loop_delete", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_loop_new", vc(wrap_uv_loop_new, "wrap_uv_loop_new", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_mutex_destroy", vc(wrap_uv_mutex_destroy, "wrap_uv_mutex_destroy", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_mutex_init", vc(wrap_uv_mutex_init, "wrap_uv_mutex_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_mutex_lock", vc(wrap_uv_mutex_lock, "wrap_uv_mutex_lock", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_mutex_trylock", vc(wrap_uv_mutex_trylock, "wrap_uv_mutex_trylock", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_mutex_unlock", vc(wrap_uv_mutex_unlock, "wrap_uv_mutex_unlock", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_now", vc(wrap_uv_now, "wrap_uv_now", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_once", vc(wrap_uv_once, "wrap_uv_once", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_pipe_bind", vc(wrap_uv_pipe_bind, "wrap_uv_pipe_bind", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_pipe_connect", vc(wrap_uv_pipe_connect, "wrap_uv_pipe_connect", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_pipe_init", vc(wrap_uv_pipe_init, "wrap_uv_pipe_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_pipe_open", vc(wrap_uv_pipe_open, "wrap_uv_pipe_open", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_pipe_pending_instances", vc(wrap_uv_pipe_pending_instances, "wrap_uv_pipe_pending_instances", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_poll_init", vc(wrap_uv_poll_init, "wrap_uv_poll_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_poll_init_socket", vc(wrap_uv_poll_init_socket, "wrap_uv_poll_init_socket", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_poll_start", vc(wrap_uv_poll_start, "wrap_uv_poll_start", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_poll_stop", vc(wrap_uv_poll_stop, "wrap_uv_poll_stop", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_prepare_init", vc(wrap_uv_prepare_init, "wrap_uv_prepare_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_prepare_start", vc(wrap_uv_prepare_start, "wrap_uv_prepare_start", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_prepare_stop", vc(wrap_uv_prepare_stop, "wrap_uv_prepare_stop", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_process_kill", vc(wrap_uv_process_kill, "wrap_uv_process_kill", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_queue_work", vc(wrap_uv_queue_work, "wrap_uv_queue_work", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_read2_start", vc(wrap_uv_read2_start, "wrap_uv_read2_start", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_read_start", vc(wrap_uv_read_start, "wrap_uv_read_start", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_read_stop", vc(wrap_uv_read_stop, "wrap_uv_read_stop", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_ref", vc(wrap_uv_ref, "wrap_uv_ref", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_req_size", vc(wrap_uv_req_size, "wrap_uv_req_size", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_resident_set_memory", vc(wrap_uv_resident_set_memory, "wrap_uv_resident_set_memory", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_run", vc(wrap_uv_run, "wrap_uv_run", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_run_once", vc(wrap_uv_run_once, "wrap_uv_run_once", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_rwlock_destroy", vc(wrap_uv_rwlock_destroy, "wrap_uv_rwlock_destroy", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_rwlock_init", vc(wrap_uv_rwlock_init, "wrap_uv_rwlock_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_rwlock_rdlock", vc(wrap_uv_rwlock_rdlock, "wrap_uv_rwlock_rdlock", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_rwlock_rdunlock", vc(wrap_uv_rwlock_rdunlock, "wrap_uv_rwlock_rdunlock", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_rwlock_tryrdlock", vc(wrap_uv_rwlock_tryrdlock, "wrap_uv_rwlock_tryrdlock", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_rwlock_trywrlock", vc(wrap_uv_rwlock_trywrlock, "wrap_uv_rwlock_trywrlock", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_rwlock_wrlock", vc(wrap_uv_rwlock_wrlock, "wrap_uv_rwlock_wrlock", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_rwlock_wrunlock", vc(wrap_uv_rwlock_wrunlock, "wrap_uv_rwlock_wrunlock", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_sem_destroy", vc(wrap_uv_sem_destroy, "wrap_uv_sem_destroy", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_sem_init", vc(wrap_uv_sem_init, "wrap_uv_sem_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_sem_post", vc(wrap_uv_sem_post, "wrap_uv_sem_post", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_sem_trywait", vc(wrap_uv_sem_trywait, "wrap_uv_sem_trywait", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_sem_wait", vc(wrap_uv_sem_wait, "wrap_uv_sem_wait", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_set_process_title", vc(wrap_uv_set_process_title, "wrap_uv_set_process_title", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_setup_args", vc(wrap_uv_setup_args, "wrap_uv_setup_args", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_shutdown", vc(wrap_uv_shutdown, "wrap_uv_shutdown", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_signal_init", vc(wrap_uv_signal_init, "wrap_uv_signal_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_signal_start", vc(wrap_uv_signal_start, "wrap_uv_signal_start", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_signal_stop", vc(wrap_uv_signal_stop, "wrap_uv_signal_stop", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_spawn", vc(wrap_uv_spawn, "wrap_uv_spawn", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_strerror", vc(wrap_uv_strerror, "wrap_uv_strerror", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_strlcat", vc(wrap_uv_strlcat, "wrap_uv_strlcat", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_strlcpy", vc(wrap_uv_strlcpy, "wrap_uv_strlcpy", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tcp_bind", vc(wrap_uv_tcp_bind, "wrap_uv_tcp_bind", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tcp_bind6", vc(wrap_uv_tcp_bind6, "wrap_uv_tcp_bind6", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tcp_connect", vc(wrap_uv_tcp_connect, "wrap_uv_tcp_connect", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tcp_connect6", vc(wrap_uv_tcp_connect6, "wrap_uv_tcp_connect6", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tcp_getpeername", vc(wrap_uv_tcp_getpeername, "wrap_uv_tcp_getpeername", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tcp_getsockname", vc(wrap_uv_tcp_getsockname, "wrap_uv_tcp_getsockname", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tcp_init", vc(wrap_uv_tcp_init, "wrap_uv_tcp_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tcp_keepalive", vc(wrap_uv_tcp_keepalive, "wrap_uv_tcp_keepalive", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tcp_nodelay", vc(wrap_uv_tcp_nodelay, "wrap_uv_tcp_nodelay", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tcp_open", vc(wrap_uv_tcp_open, "wrap_uv_tcp_open", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tcp_simultaneous_accepts", vc(wrap_uv_tcp_simultaneous_accepts, "wrap_uv_tcp_simultaneous_accepts", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_thread_create", vc(wrap_uv_thread_create, "wrap_uv_thread_create", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_thread_join", vc(wrap_uv_thread_join, "wrap_uv_thread_join", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_thread_self", vc(wrap_uv_thread_self, "wrap_uv_thread_self", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_timer_again", vc(wrap_uv_timer_again, "wrap_uv_timer_again", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_timer_get_repeat", vc(wrap_uv_timer_get_repeat, "wrap_uv_timer_get_repeat", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_timer_init", vc(wrap_uv_timer_init, "wrap_uv_timer_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_timer_set_repeat", vc(wrap_uv_timer_set_repeat, "wrap_uv_timer_set_repeat", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_timer_start", vc(wrap_uv_timer_start, "wrap_uv_timer_start", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_timer_stop", vc(wrap_uv_timer_stop, "wrap_uv_timer_stop", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tty_get_winsize", vc(wrap_uv_tty_get_winsize, "wrap_uv_tty_get_winsize", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tty_init", vc(wrap_uv_tty_init, "wrap_uv_tty_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tty_reset_mode", vc(wrap_uv_tty_reset_mode, "wrap_uv_tty_reset_mode", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_tty_set_mode", vc(wrap_uv_tty_set_mode, "wrap_uv_tty_set_mode", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_udp_bind", vc(wrap_uv_udp_bind, "wrap_uv_udp_bind", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_udp_bind6", vc(wrap_uv_udp_bind6, "wrap_uv_udp_bind6", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_udp_getsockname", vc(wrap_uv_udp_getsockname, "wrap_uv_udp_getsockname", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_udp_init", vc(wrap_uv_udp_init, "wrap_uv_udp_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_udp_open", vc(wrap_uv_udp_open, "wrap_uv_udp_open", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_udp_recv_start", vc(wrap_uv_udp_recv_start, "wrap_uv_udp_recv_start", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_udp_recv_stop", vc(wrap_uv_udp_recv_stop, "wrap_uv_udp_recv_stop", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_udp_send", vc(wrap_uv_udp_send, "wrap_uv_udp_send", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_udp_send6", vc(wrap_uv_udp_send6, "wrap_uv_udp_send6", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_udp_set_broadcast", vc(wrap_uv_udp_set_broadcast, "wrap_uv_udp_set_broadcast", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_udp_set_membership", vc(wrap_uv_udp_set_membership, "wrap_uv_udp_set_membership", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_udp_set_multicast_loop", vc(wrap_uv_udp_set_multicast_loop, "wrap_uv_udp_set_multicast_loop", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_udp_set_multicast_ttl", vc(wrap_uv_udp_set_multicast_ttl, "wrap_uv_udp_set_multicast_ttl", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_udp_set_ttl", vc(wrap_uv_udp_set_ttl, "wrap_uv_udp_set_ttl", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_unref", vc(wrap_uv_unref, "wrap_uv_unref", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_update_time", vc(wrap_uv_update_time, "wrap_uv_update_time", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_uptime", vc(wrap_uv_uptime, "wrap_uv_uptime", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_walk", vc(wrap_uv_walk, "wrap_uv_walk", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_write", vc(wrap_uv_write, "wrap_uv_write", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_uv_write2", vc(wrap_uv_write2, "wrap_uv_write2", VC_FUNC_BUILTIN_LEAF));
}
