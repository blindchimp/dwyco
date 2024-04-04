
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
}

int
main(int, char **argv)
{
	vc v(VC_FILE);
	v.open(argv[1], VCFILE_READ|VCFILE_BINARY);
	vcxstream vcx(v);
    vcx.open(vcxstream::READABLE);

	vc o;
	o.xfer_in(vcx);
    o.print_top(VcOutput);
}

