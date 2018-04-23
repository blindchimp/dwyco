
/*
 * $Header: g:/dwight/repo/dwcls/rcs/mswin.h 1.10 1997/06/01 04:40:16 dwight Stable095 $
 */
//
// This allocator uses MS-Windows Global memory allocation routines
// to get memory. There should only be one of these objects created, and
// used as a base or default memory allocator. This object cannot be
// embedded in other memory objects (this is the root of the memory
// allocation tree for windows programs.)
//
//

#ifndef MSWIN_H
#define MSWIN_H
#include "ialloc.h"
#include "allret.h"
#include <windows.h>
#include "aalloc.h"

class MSWindowsGlobalImp : public Allocator
{
private:
	//CList handle_list;

public:
	MSWindowsGlobalImp();
	~MSWindowsGlobalImp();
	virtual ALLRET& alloc(u_long byte_count);
	virtual void free(ALLRET&);
	virtual void *map_to_phys(const ALLRET&) { return 0;}
};

class MSWIN_POINTER : public ALLRET
{
friend class MSWindowsGlobalImp;

private:
	HGLOBAL h;
	void *ptr;

	static ALLRET_alloc<MSWIN_POINTER> *pool;

public:
	static void init_pool(int nhandles = 100);

	MSWIN_POINTER();
	MSWIN_POINTER(HGLOBAL gh);
	MSWIN_POINTER(const MSWIN_POINTER&);
	~MSWIN_POINTER();
	MSWIN_POINTER& operator=(const MSWIN_POINTER&);
	void *operator new(size_t);
    void operator delete(void *);
	operator void *() const;
	int no_space() const;

};

#endif
