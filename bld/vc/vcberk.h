
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/vc/rcs/vcberk.h 1.18 1997/10/05 17:27:06 dwight Stable $
 */
#ifndef VCBERK_H
#define VCBERK_H

extern "C" {
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
};

typedef int SOCKET;
#define SOCKET_ERROR (-1)
#define INVALID_SOCKET (-1)
#ifndef INADDR_NONE
#define INADDR_NONE (-1)
#endif
#endif
