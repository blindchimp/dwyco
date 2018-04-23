
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#define VCENC
#include <new.h>
#ifndef _Windows
#ifndef UNIX
#ifdef __BORLANDC__
#include <dos.h>
#include <alloc.h>
#endif
#endif
#endif
#if defined(__GNUG__) && defined(MSDOS)
#include <fcntl.h>
extern int _fmode;
#endif
#include <sys/stat.h>
#include <io.h>
#include <stdlib.h>

class Allocator;
#include "vc.h"
#include "lhboot.h"
#undef VCDBG
#undef new
#include "vclex.h"
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/lhboot.cpp 1.6 1998/12/14 05:11:06 dwight Exp $";

#ifdef VCDBG
#include "vcdbg.h"
#endif

Allocator *Default_alloc;
int endoworld;

extern "C" void malloc_shutdown(void);
extern "C" void malloc_debug(long);
static void
out_of_mem()
{
	// this is catastrophic
	creat("nomem", S_IWRITE);
	_exit(1);
#if 0
	cout << "Out of memory\n";
	exit(1);
#endif
}

int
lhboot(int argc, char *argv[], char *which_loadfile)
{
#if defined(__GNUG__) && defined(__MSDOS__)
	// for losing stream package "fix"
	_fmode = O_BINARY;
#endif

#if !defined(_Windows) && !defined(UNIX) && !defined(__GNUG__)
	cout << "Far mem: " << farcoreleft() << "\n";
#endif
#if __BCPLUSPLUS__ < 0x540
	set_new_handler(out_of_mem);
#else
	std::set_new_handler(out_of_mem);
#endif	

	FILE *i = fopen(argv[1], "r");
	if(i == 0)
	{
		VcError << "can't open " << argv[1] << "\n";
		return 0;
	}

#if defined(__MSDOS__) || defined(_Windows)
	char *load_file = "c:\\lhlib\\load.lh";
#else
	char *home = getenv("HOME");
	static char homestr[512];
	char *load_file;
	if(home == 0)
		load_file = "load.lh";
	else
	{
		sprintf(homestr, "%s/lhlib/load.lh", home);
		load_file = homestr;
	}
#endif
	// try to get the basic stuff in first
#ifdef VCDBG
	vc("__lh_start_debugger").global_bind("t");
#endif
	if(which_loadfile != 0)
		load_file = which_loadfile;
	FILE *lf = fopen(load_file, "r");
	if(lf != 0)
	{
		VcLexerStdio& lfstrm = *new VcLexerStdio(lf, VcError);
		lfstrm.set_input_description(load_file);
		vc *lfvc = new vc(lfstrm);
		lfvc->force_eval();
		fclose(lf);
		delete &lfstrm;
		delete lfvc;
	}
	else
	{
		VcError << "warning: can't find load-file\n";
	}
	vc args(VC_VECTOR);
	for(int j = 2; j < argc; ++j)
	{
		args.append(vc(argv[j]));
	}
	vc("__argv").global_bind(args);
	int c;
	if((c = fgetc(i)) != '#')
		ungetc(c, i);
	else
		while(!feof(i) && fgetc(i) != '\n')
			;
	VcLexerStdio& strm = *new VcLexerStdio(i, VcError);
	strm.set_input_description(argv[1]);

	vc *s = new vc(strm);
	delete &strm;
	fclose(i);
#ifdef VCDBG
	vc("__lh_dbg_user_expr").global_bind(*s);
	vc *dbg = new vc(VC_CVAR, "__lh_debug(|LH debug, type \"help\" for help.| toplev nil nil)");
	dbg->force_eval();
	delete dbg;
	delete s;
#else
	(void)s->force_eval();
	delete s;
#endif
	//vc::exit();
	return 1;
}

