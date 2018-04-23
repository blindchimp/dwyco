
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCSFILE_H
#define VCSFILE_H
// $Header: g:/dwight/repo/vc/rcs/vcsfile.h 1.47 1997/11/27 03:06:24 dwight Stable $

#include <stdio.h>
#include "vcfile.h"

class vc_stdio_file : public vc_file
{
private:
	FILE *handle;

public:
	vc_stdio_file();
	~vc_stdio_file();

	virtual int open(const vc& name, vcfilemode);
	virtual int close();
	virtual int seek(long pos, int whence);
	virtual int read(void *buf, long& len);
	virtual int write(const void *buf, long& len);
	virtual int fgets(void *buf, long len);
    virtual int eof();
    virtual void operator<<(vc);
};


#endif
