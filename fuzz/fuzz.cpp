
#include "vc.h"
#include "vcxstrm.h"
#include <cstdlib>
#include <stdio.h>

class DwAllocator;
DwAllocator *Default_alloc;
int Continue_on_alarm;

void
oopanic(const char *s)
{
    ::abort();
	// note: this isn't a "crash" from afl perspective,
	// since we detected it rather than segv or something.
	// note that a server might still crash, and this is
	// worth trying to fix, if a stream comes in with
	// sets as "keys" or something, since that makes no
	// sense.
    exit(0);
}

int
main(int, char **argv)
{
	vc::set_xferin_constraints(20000, -1, -1, -1);
	vc v(VC_FILE);
	v.open(argv[1], VCFILE_READ|VCFILE_BINARY);
	vcxstream vcx(v);
    vcx.open(vcxstream::READABLE);
    {
	vc o;
    if(o.xfer_in(vcx) < 0)
    {
        // if the deserialize fails, o should be nil
        if(!o.is_nil())
            ::abort();
    }
    }
    //o.print_top(VcOutput);
    return 0;
}

