
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "vcsock.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcsock.cpp 1.47 1997/10/05 17:27:06 dwight Stable $";

vc_socket::vc_socket()
{
	emode = EXCEPTIONS;
	smode = VC_BLOCKING;
	err_callback = 0;
	exc_level = 'U';
	last_wsa_error = 0;
    status = 0;
    retries = 0;
    max_retries = 0;
}

int
vc_socket::do_error_processing()
{
	if(err_callback != 0)
		return (*err_callback)(this);
	else
		return VC_SOCKET_RESUME;
}


