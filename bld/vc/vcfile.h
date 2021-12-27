
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCFILE_H
#define VCFILE_H
// $Header: g:/dwight/repo/vc/rcs/vcfile.h 1.47 1997/11/27 03:06:04 dwight Stable $

#include "vccomp.h"
#include "vcio.h"

#define VCFILE_RAISE(exc, a1, a2, retval) do {if(do_error_processing(exc, a1, a2) == VC_FILE_BACKOUT) return retval;} while(0)
#define VCFILE_RAISEABORT(exc, a1, a2, retval) do {if(do_error_processing(exc, a1, a2) == VC_FILE_RESUME) oopanic("resume after file abort?"); return retval;} while(0)
#define VCFILE_RAISEABORT2(exc, a1, a2) do {if(do_error_processing(exc, a1, a2) == VC_FILE_RESUME) oopanic("resume after file abort?"); return ;} while(0)

#define VC_FILE_RESUME 0
#define VC_FILE_BACKOUT 1

class vc_file : public vc_composite
{
private:

protected:
    vcfilemode fmode;
    vc filename;

    virtual int do_error_processing(vc exc, vc a1, vc a2);


public:
	vcerrormode emode;
    vc errvc;
    vc errvc1;
    vc errvc2;
	
	vc_file() {emode = EXCEPTIONS;}
	vc_file(const vc& n) : filename(n) { emode = EXCEPTIONS; }

	virtual vc_default *do_copy() const {oopanic("copy file?"); return 0;}
	enum vc_type type() const {return VC_FILE;}
	void printOn(VcIO os) {os << "file("; filename.print(os); os << " " <<
		fmode << " " << emode << ")";}

	virtual int open(const vc& name, vcfilemode) = 0;
	virtual int close() = 0;
	virtual int seek(long pos, int whence) = 0;
	virtual int read(void *buf, long& len) = 0;
	virtual int write(const void *buf, long& len) = 0;
	virtual int fgets(void *buf, long len) = 0;
    virtual int eof() = 0;
	virtual long overflow(vcxstream&, char *, long);
	virtual long underflow(vcxstream&, char *, long, long);
};


#endif
