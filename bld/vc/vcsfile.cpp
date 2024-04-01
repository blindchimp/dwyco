
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "vcsfile.h"
#include "vcmap.h"
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcsfile.cpp 1.48 1998/08/01 04:47:34 dwight Exp $";


vc_stdio_file::vc_stdio_file()
{
	handle = 0;
}

vc_stdio_file::~vc_stdio_file()
{
	if(handle != 0)
		fclose(handle);
}

int
vc_stdio_file::eof()
{
    if(handle == 0)
    {
        VCFILE_RAISEABORT("A:UNOPENED_FILE", vcnil, vcnil, 0);
        return 0;
    }
    return feof(handle);
}

int
vc_stdio_file::open(const vc& nm, vcfilemode mode)
{
	char lmode[10];
	memset(lmode, 0, sizeof(lmode));
	int seek = 0;

	int m = mode;
	switch(m & ~VCFILE_BINARY)
	{
	case VCFILE_READ:
		lmode[0] = 'r';
		break;

	case VCFILE_WRITE:
		lmode[0] = 'w';
		break;

	case VCFILE_APPEND:
		lmode[0] = 'a';
		lmode[1] = '+';
		//seek = 1;
		break;

	default:
		VCFILE_RAISEABORT("A:BAD_FILE_MODE", vcnil, vcnil, 0);
	}
	if(m & VCFILE_BINARY)
		strcat(lmode, "b");
	else
		strcat(lmode, "t");
	
	 
	handle = fopen((const char *)nm, lmode);
	if(handle == 0)
    {
		VCFILE_RAISEABORT("A:OPEN_FAILED", nm, vcnil, 0);
	}
	if(seek)
	{
		if(fseek(handle, 0L, SEEK_END) != 0)
		{
			VCFILE_RAISEABORT("A:SEEK_FAILED", nm, vcnil, 0);
		}
	}
	filename = nm;
    return 1;
}

int
vc_stdio_file::close()
{
	if(handle == 0)
    {
		VCFILE_RAISE("W:CLOSING_UNOPENED_FILE", vcnil, vcnil, 0);
		return 0;
	}
	if(fclose(handle) != 0)
	{
		VCFILE_RAISE("W:CLOSE_FAILED", vcnil, vcnil, 0);
		handle = 0;
		return 0;
	}
	handle = 0;
	return 1;
}

int
vc_stdio_file::seek(long pos, int whence)
{

	if(handle == 0)
	{
		VCFILE_RAISEABORT("A:SEEKING_ON_UNOPENED_FILE", vcnil, vcnil, 0);
	}
	if(fseek(handle, pos, whence) == -1)
    {
		VCFILE_RAISEABORT("A:SEEK_OPERATION_FAILED", filename, vcnil, 0);
	}
    return 1;
}


int
vc_stdio_file::fgets(void *buf, long len)
{
	if(handle == 0)
	{
		VCFILE_RAISEABORT("A:ATTEMPT_TO_READ_UNOPENED_FILE", vcnil, vcnil, 0);
	}

	if(feof(handle))
		return 0;

	char *ret;

	ret = ::fgets((char *)buf, len, handle);

	if(ret == 0)
    {
    	if(!feof(handle))
			VCFILE_RAISEABORT("A:READ_OPERATION_FAILED", filename, vcnil, 0);
		return 0;
	}

    return 1;

}
int
vc_stdio_file::read(void *buf, long& len)
{
	if(handle == 0)
	{
		VCFILE_RAISEABORT("A:ATTEMPT_TO_READ_UNOPENED_FILE", vcnil, vcnil, 0);
	}

	if(feof(handle))
		return 0;

	long ret;
    // testing
    len = 1;
	ret = fread(buf, 1, len, handle);

	if(ret != len && !feof(handle))
    {
		VCFILE_RAISEABORT("A:READ_OPERATION_FAILED", filename, vcnil, 0);
	}

	len = ret;
    return 1;

}


int
vc_stdio_file::write(const void *buf, long& len)
{
	if(handle == 0)
	{
		VCFILE_RAISEABORT("A:ATTEMPT_TO_WRITE_UNOPENED_FILE", vcnil, vcnil, 0);
	}

	long ret;

	ret = fwrite(buf, 1, len, handle);

	if(ret != len)
    {
		VCFILE_RAISEABORT("A:WRITE_OPERATION_FAILED", filename, vcnil, 0);
	}

	len = ret;
    return 1;

}

void
vc_stdio_file::operator<<(vc v)
{
	if(handle == 0)
	{
		VCFILE_RAISEABORT2("A:ATTEMPT_TO_WRITE_UNOPENED_FILE", vcnil, vcnil);
	}
	
	//fflush(handle);
	VcIOHack strm(handle);
	v.print_top(strm);
	if(ferror(handle))
	{
		// doesn't appear to be a way of deciding
		// what actually happened...
		VCFILE_RAISEABORT2("A:OUTPUT_OPERATION_FAILED", vcnil, vcnil);
	}
	// note: don't close and hope that stream dtor doesn't
	// close the fd.
}
