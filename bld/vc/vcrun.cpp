
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <new>
#ifdef _MSC_VER
#define _Windows
#include <direct.h>
#endif

#include "vc.h"
#include "vcio.h"

#undef new
#include "vclex.h"
//static char Rcsid[] = "$Header: /e/linux/home/dwight/repo/vc/rcs/vcrun.cpp 1.55 1999/03/17 14:57:04 dwight Exp $";

#ifdef VCDBG
#include "vcdbg.h"
#endif

#if defined(LINUX) || defined(MACOSX)
#include <signal.h>
#include <unistd.h>
#endif

extern "C" void malloc_shutdown(void);
extern "C" void malloc_debug(long);
static void
out_of_mem()
{
	VcError << "Out of memory\n";
	::abort();
}

int
main(int argc, char *argv[])
{
#ifdef USE_RCT
void init_rct();
init_rct();
#endif

#ifdef UNIX
	//extern reg_syntax_t re_syntax_options;
	//re_syntax_options |= RE_DOT_NEWLINE;

	signal(SIGPIPE, SIG_IGN);
#endif
#ifdef LINUX
	siginterrupt(SIGALRM, 1);
#endif

    std::set_new_handler(out_of_mem);
	int dec = 0;
	if(argc >= 2 && argv[1][0] == '-' && argv[1][1] == 0)
	{
		--argc;
		++argv;
		dec = 1;
	} 
	const char *load_name = "load.lh";
    int translate = 0;
	while(argc >= 3)
	{
		if(strcmp(argv[1], "-w") == 0)
		{
			if(freopen(argv[2], "w", stderr) == 0)
			{
				VcError << "can't redirect stderr to " << argv[2] << "\n";
				exit(1);
			}
		}
		else if(strcmp(argv[1], "-c") == 0)
		{
			if(chdir(argv[2]) == -1)
			{
				VcError << "can't chdir to " << argv[2] << "\n";
				exit(1);
			}
		}
		else if(strcmp(argv[1], "-l") == 0)
		{
			load_name = argv[2];
		}
        else if(strcmp(argv[1], "-t") == 0)
            translate = atoi(argv[2]);
        else
			break;
		argc -= 2;
		argv += 2;
	} 

	FILE *i;
	i = fopen(argv[1], dec ? "rb" : "r");
	if(i == 0)
	{
		VcError << "can't open " << argv[1] << "\n";
		exit(1);
	}

	vc::setup_logs();
	vc::init();

#if defined(_Windows)
    DwString load_file("c:\\lhlib\\load.lh");
#else
	char *home = getenv("HOME");
    DwString homestr;
    DwString load_file;
	if(home == 0)
		load_file = load_name;
	else
	{
        homestr += home;
        homestr += "/lhlib/";
        homestr += load_name;
		load_file = homestr;
	}
#endif
	// try to get the basic stuff in first

	vc("__lh_enc").global_bind(dec ? vctrue : vcnil);
    FILE *lf = fopen(load_file.c_str(), dec ? "rb" : "r");
	if(lf != 0)
	{
		VcLexerStdio *lfstrm = dec ? new VcLexerStdioEncrypted(lf, VcError) : new VcLexerStdio(lf, VcError);
        lfstrm->set_input_description(load_file.c_str());
		vc *lfvc = new vc(*lfstrm);
		lfvc->force_eval();
		fclose(lf);
		delete lfstrm;
		delete lfvc;
	}
	else
	{
		// quiet ourselves...
		// if it is important, we'll get an error when we try to 
		// call it. emitting this with xinetd causes a problem
		// because xinetd maps stderr onto the socket it gives
		// the app, and that makes it hard to
		// transfer structured data.
		//VcError << "warning: can't find load-file\n";
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
	VcLexerStdio *strm = (dec ? new VcLexerStdioEncrypted(i, VcError) : new VcLexerStdio(i, VcError));
	strm->set_input_description(argv[1]);

	vc *s = new vc(*strm);
	delete strm;
	fclose(i);

    if(!translate)
        (void)s->force_eval();
    else
    {
        vc top = s->translate(VcOutput);
        VcOutput << "vc top() { return " << top << "();}\n";
    }
	delete s;
	vc::exit();
	vc::shutdown_logs();
	return 0;
}

[[noreturn]]
void
oopanic(const char *s)
{
	printf("panic: %s\n", s);
	fflush(stdout);
	exit(1);
}

