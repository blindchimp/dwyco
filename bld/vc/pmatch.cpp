
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/pmatch.cpp 1.45 1996/11/17 05:57:56 dwight Stable $";
/* 
 * match string against Shell (glob) pattern
 */

int pmatch(const char *pat, const char *str)
{
    int pc; /* pattern character */
    int sc; /* string character */
    int ok; /* flag used for brackets */

	for (;;) 
	{
		pc = *pat++;

		/*
		 * if end of string, return TRUE if at end of pattern, FALSE
		 *    if not at an asterisk
		 */
		if ((sc = *str++) == '\0') 
		{
			if (pc == '\0')
				return 1;
			if (pc != '*')
				return 0;
		}
		/*
		 * if asterisk, recurse trying to match the rest of the
		 * pattern with the rest of the string
		 */
		if (pc == '*') 
		{
			if (*pat != '\0')
				for (--str; !pmatch(pat, str);)
					if (*str++ == '\0')
						return 0;
			return 1;
		}
		/*
		 * if question mark, go to next character in pattern
		 */
		if (pc == '?')
			continue;
		/*
		 * if brackets, check string character against characters (or ranges
		 * of characters) specified in the brackets
		 */
		if (pc == '[') 
		{
			for (ok = 0;;) 
			{
				if ((pc = *pat++) == '\0')
					return 0;
				if (pc == ']') 
				{
					if (ok)
						break;
					return 0;
				}
				if (pc == '-') 
				{
					if ((pc = *pat++) == '\0')
						return 0;
					if (sc <= (pc &= 0177) && sc >= (pat[-3] & 0177))
						ok = 1;
				}
				else
					if (sc == (pc &= 0177))
						ok = 1;
			} 
			continue;
		}

	if (pc != '?' && sc != (pc &= 0177))
		return 0;
	}
}
