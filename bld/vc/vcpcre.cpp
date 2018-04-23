
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef VCPCRE
#include "vc.h"
#include <pcre.h>
// note: this is a quicky hack to support the
// peculiar regex for in the trivia database

static
void
vclh_pcre_free(long d)
{
	void *p = (void *)d;
	pcre_free(p);
}

static
vc
vclh_pcre_compile(vc re)
{
	pcre *p = 0;
	const char *error;
	int erroffset;

	if(!(p = pcre_compile(re, PCRE_CASELESS | PCRE_NO_AUTO_CAPTURE | PCRE_UNGREEDY, &error, &erroffset, 0)))
	{
		VcError << "pcre compile error: " << error << "\"" << (const char *)re << "\"\n";
		return vcnil;
	}
	vc ret(VC_INT_DTOR, (const char *)vclh_pcre_free, (long)p);
	return ret;
}

static
vc
vclh_pcre_exec(vc vcpcre, vc str)
{
	pcre *p = (pcre *)(void *)vcpcre;
	int ovector[1];
	int ovecsize = 0;
	if(pcre_exec(p, 0, str, str.len(), 0, 0, ovector, ovecsize) >= 0)
	{
		return vctrue;
	}
	return vcnil;
}

static void
makefun(const char *name, const vc& fun)
{
	vc(name).bind(fun);
}

void
init_vcpcre()
{
	makefun("pcre-compile", vc(vclh_pcre_compile, "pcre-compile", VC_FUNC_BUILTIN_LEAF));
	makefun("pcre-exec", vc(vclh_pcre_exec, "pcre-exec", VC_FUNC_BUILTIN_LEAF));

}
#endif
