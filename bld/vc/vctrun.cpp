
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
using namespace std;
#include <new>

class Allocator;
#include "vc.h"
#include "vcio.h"
#include "vctrt.h"

#ifdef LINUX
#include <unistd.h>
#include "signal.h"
#endif

static void
out_of_mem()
{
	VcError << "Out of memory\n";
	::abort();
}

int
main(int argc, char *argv[])
{
#ifdef LINUX
    signal(SIGPIPE, SIG_IGN);
#endif
#ifdef LINUX
	siginterrupt(SIGALRM, 1);
#endif

	set_new_handler(out_of_mem);
	int dec = 0;
	if(argc >= 2 && argv[1][0] == '-' && argv[1][1] == 0)
	{
		--argc;
		++argv;
		dec = 1;
	} 
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
		else
			break;
		argc -= 2;
		argv += 2;
	} 

	vc::setup_logs();
	vc::init();

	vc args(VC_VECTOR);
	// note: for interpreter, first arg is file-to-interpret.
	// in this case, we have already compiled everything and
	// that is not on the arg list.
	for(int j = 1; j < argc; ++j)
	{
		args.append(vc(argv[j]));
	}
	vc("__argv").global_bind(args);
	vc top();
    vc entry = top();
    vc (*p)() = (vc (*)())(long)entry;
    try
    {
        (*p)();
    }
    catch(VcErr e)
    {
        VcError << "runtime error: " << e.err << " (" << e.specific << ")\n";
    }
    catch(...)
    {
        VcError << "runtime error: unknown error caught\n";
    }

	vc::exit();
	vc::shutdown_logs();
	return 0;
}

void
oopanic(const char *s)
{
	printf("panic: %s\n", s);
	fflush(stdout);
	exit(1);
}

