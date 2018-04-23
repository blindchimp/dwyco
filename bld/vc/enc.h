
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
/******************************************************************************
* file: enc.h
* prototypes for enc.c (simple encryption)
*
******************************************************************************/

#ifndef ENC_H
#define ENC_H

namespace ns_vc {

/*
 * $Header: g:/dwight/repo/vc/rcs/enc.h 1.3 1997/10/05 17:27:06 dwight Stable $
 */


class Enc
{
public:
	Enc();
	void munge(char *buf, int count);
	void mungeback(char *buf, int count);
private:
	int keychar;

};
}


#endif
