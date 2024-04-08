
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
/*
 * Filename: vcswerr.cpp
 *
 * Description:
 *  String Table with Windows Socket errors as integer identifiers and
 *  Error value and text description in text.
 *
 * Edit History:
 *  21-Jan-94	dwight@its.bldrdoc.gov changed to use normal C rather than
 *				resource junk.
 *  18-Feb-93  <MIKE@sx.gar.no> created and sent to WinSockAPI mailing list 
 *				 for public distribution.
 *  25-Apr-93  <rcq@ftp.com>    WSAINITIALIZED respelt as WSAINITIALISED,
 *				 and added header with comments.
 */
#ifdef USE_WINSOCK
#include <WinSock2.h>
#else
#include <errno.h>
#define WSABASEERR 0
#endif
#include <stdio.h>
#include <string.h>
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcwserr.cpp 1.46 1997/10/05 17:27:06 dwight Stable $";

static struct vcwserrvals
{
	int	errnum;
	const char *str;
} Vcwserrvals [] =
{
{  WSABASEERR,         "[0] No Error"},
#ifndef USE_WINSOCK
{  EINTR,           "[10004] Interrupted system call"},
{  EBADF,           "[10009] Bad file number"},
{  EACCES,          "[10013] Permission denied"},
{  EFAULT,          "[10014] Bad address"},
{  EINVAL,          "[10022] Invalid argument"},
{  EMFILE,          "[10024] Too many open files"},
#else
{  WSAEINTR,           "[10004] Interrupted system call"},
{  WSAEBADF,           "[10009] Bad file number"},
{  WSAEACCES,          "[10013] Permission denied"},
{  WSAEFAULT,          "[10014] Bad address"},
{  WSAEINVAL,          "[10022] Invalid argument"},
{  WSAEMFILE,          "[10024] Too many open files"},
#endif
#ifndef USE_WINSOCK
{  EWOULDBLOCK,     "[10035] Operation would block"},
{  EINPROGRESS,     "[10036] Operation now in progress"},
{  EALREADY,        "[10037] Operation already in progress"},
{  ENOTSOCK,        "[10038] Socket operation on non-socket"},
{  EDESTADDRREQ,    "[10039] Destination address required"},
{  EMSGSIZE,        "[10040] Message too long"},
{  EPROTOTYPE,      "[10041] Protocol wrong type for socket"},
{  ENOPROTOOPT,     "[10042] Bad protocol option"},
{  EPROTONOSUPPORT, "[10043] Protocol not supported"},
{  ESOCKTNOSUPPORT, "[10044] Socket type not supported"},
{  EOPNOTSUPP,      "[10045] Operation not supported on socket"},
{  EPFNOSUPPORT,    "[10046] Protocol family not supported"},
{  EAFNOSUPPORT,    "[10047] Address family not supported by protocol family"},
{  EADDRINUSE,      "[10048] Address already in use"},
{  EADDRNOTAVAIL,   "[10049] Can't assign requested address"},
{  ENETDOWN,        "[10050] Network is down"},
{  ENETUNREACH,     "[10051] Network is unreachable"},
{  ENETRESET,       "[10052] Net dropped connection or reset"},
{  ECONNABORTED,    "[10053] Software caused connection abort"},
{  ECONNRESET,      "[10054] Connection reset by peer"},
{  ENOBUFS,         "[10055] No buffer space available"},
{  EISCONN,         "[10056] Socket is already connected"},
{  ENOTCONN,        "[10057] Socket is not connected"},
{  ESHUTDOWN,       "[10058] Can't send after socket shutdown"},
{  ETOOMANYREFS,    "[10059] Too many references, can't splice"},
{  ETIMEDOUT,       "[10060] Connection timed out"},
{  ECONNREFUSED,    "[10061] Connection refused"},
{  ELOOP,           "[10062] Too many levels of symbolic links"},
{  ENAMETOOLONG,    "[10063] File name too long"},
{  EHOSTDOWN,       "[10064] Host is down"},
{  EHOSTUNREACH,    "[10065] No Route to Host"},
{  ENOTEMPTY,       "[10066] Directory not empty"},
{  EUSERS,          "[10068] Too many users"},
{  EDQUOT,          "[10069] Disc Quota Exceeded"},
{  ESTALE,          "[10070] Stale NFS file handle"},
{  EREMOTE,         "[10071] Too many levels of remote in path"},
#endif
#ifdef USE_WINSOCK
{  WSAEWOULDBLOCK,     "[10035] Operation would block"},
{  WSAEINPROGRESS,     "[10036] Operation now in progress"},
{  WSAEALREADY,        "[10037] Operation already in progress"},
{  WSAENOTSOCK,        "[10038] Socket operation on non-socket"},
{  WSAEDESTADDRREQ,    "[10039] Destination address required"},
{  WSAEMSGSIZE,        "[10040] Message too long"},
{  WSAEPROTOTYPE,      "[10041] Protocol wrong type for socket"},
{  WSAENOPROTOOPT,     "[10042] Bad protocol option"},
{  WSAEPROTONOSUPPORT, "[10043] Protocol not supported"},
{  WSAESOCKTNOSUPPORT, "[10044] Socket type not supported"},
{  WSAEOPNOTSUPP,      "[10045] Operation not supported on socket"},
{  WSAEPFNOSUPPORT,    "[10046] Protocol family not supported"},
{  WSAEAFNOSUPPORT,    "[10047] Address family not supported by protocol family"},
{  WSAEADDRINUSE,      "[10048] Address already in use"},
{  WSAEADDRNOTAVAIL,   "[10049] Can't assign requested address"},
{  WSAENETDOWN,        "[10050] Network is down"},
{  WSAENETUNREACH,     "[10051] Network is unreachable"},
{  WSAENETRESET,       "[10052] Net dropped connection or reset"},
{  WSAECONNABORTED,    "[10053] Software caused connection abort"},
{  WSAECONNRESET,      "[10054] Connection reset by peer"},
{  WSAENOBUFS,         "[10055] No buffer space available"},
{  WSAEISCONN,         "[10056] Socket is already connected"},
{  WSAENOTCONN,        "[10057] Socket is not connected"},
{  WSAESHUTDOWN,       "[10058] Can't send after socket shutdown"},
{  WSAETOOMANYREFS,    "[10059] Too many references, can't splice"},
{  WSAETIMEDOUT,       "[10060] Connection timed out"},
{  WSAECONNREFUSED,    "[10061] Connection refused"},
{  WSAELOOP,           "[10062] Too many levels of symbolic links"},
{  WSAENAMETOOLONG,    "[10063] File name too long"},
{  WSAEHOSTDOWN,       "[10064] Host is down"},
{  WSAEHOSTUNREACH,    "[10065] No Route to Host"},
{  WSAENOTEMPTY,       "[10066] Directory not empty"},
#if 0
{  WSAEPROCLIM,        "[10067] Too many processes"},
#endif
{  WSAEUSERS,          "[10068] Too many users"},
{  WSAEDQUOT,          "[10069] Disc Quota Exceeded"},
{  WSAESTALE,          "[10070] Stale NFS file handle"},
{  WSAEREMOTE,         "[10071] Too many levels of remote in path"},
#endif
#ifdef USE_WINSOCK
{  WSASYSNOTREADY,     "[10091] Network SubSystem is unavailable"},
{  WSAVERNOTSUPPORTED, "[10092] WINSOCK DLL Version out of range"},
{  WSANOTINITIALISED,  "[10093] Successful WSASTARTUP not yet performed"},
{  WSAHOST_NOT_FOUND,  "[11001] Host not found"},
{  WSATRY_AGAIN,       "[11002] Non-Authoritative Host not found"},
{  WSANO_RECOVERY,     "[11003] Non-Recoverable errors: FORMERR, REFUSED, NOTIMP"},
{  WSANO_DATA,         "[11004] Valid name, no data record of requested type"}
#endif
};

static int Nerrs = sizeof(Vcwserrvals) / sizeof(Vcwserrvals[0]);

const char *
vc_wsget_errstr(int err)
{
	if(err == 0)
    	err = WSABASEERR;
	// note: errors are not contiguous, so we search...
	for(int i = 0; i < Nerrs; ++i)
	{
		if(err == Vcwserrvals[i].errnum)
			return Vcwserrvals[i].str;
	}
    static char a[500];
    snprintf(a, sizeof(a), "[%d] %s Say what?", err, strerror(err));
	return a;
}


