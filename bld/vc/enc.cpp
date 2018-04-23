
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
/******************************************************************************
* enc.c: do simple encrypting to keep plaintext stuff to a minimum
*
******************************************************************************/

#include "enc.h"

namespace ns_vc {

static const char key[] = "fooBarMumbleGrunch";
static const int keylen = sizeof(key) - 1;

/******************************************************************************
* munge: do simple XOR encrypting in place.
*
* To decrypt, simply pass the encrypted string back to munge.
******************************************************************************/

Enc::Enc()
{
	keychar = 0;
}

void 
Enc::munge(char *buf, int count)
{
	int i;

	for(i = 0; i < count; ++i)
	{
		char tmp = buf[i];
		tmp ^= (key[keychar++ % keylen]) + 'A';
		buf[i] = tmp;
	}
}

void
Enc::mungeback(char *buf, int count)
{
	int i;

	for(i = 0; i < count; ++i)
	{
		char tmp = buf[count - i - 1];
		tmp ^= (key[--keychar % keylen]) + 'A';
		buf[count - i - 1] = tmp;
	}
}

}
