
/*
 * $Header: g:/dwight/repo/dwcls/rcs/msptr.cpp 1.11 1997/06/01 04:40:15 dwight Stable095 $
 */
#include "mswin.h"
#include "aalloc.h"

ALLRET_alloc<MSWIN_POINTER> *MSWIN_POINTER::pool;

void
MSWIN_POINTER::init_pool(int nhandles)
{
	if(pool == 0)
		pool = new ALLRET_alloc<MSWIN_POINTER>(nhandles);
}

void *
MSWIN_POINTER::operator new(size_t t)
{
	return pool->alloc();
}

void
MSWIN_POINTER::operator delete(void *p)
{
	pool->free((MSWIN_POINTER *)p);
}


MSWIN_POINTER::MSWIN_POINTER() : ALLRET(0)
{
	h = NULL;
	ptr = NULL;
}

MSWIN_POINTER::MSWIN_POINTER(HGLOBAL gh) : ALLRET(0)
{
	h = gh;
    ptr = GlobalLock(h);
}

MSWIN_POINTER::MSWIN_POINTER(const MSWIN_POINTER& m) : ALLRET(0)
{
	h = m.h;
	ptr = GlobalLock(h);
}

MSWIN_POINTER::~MSWIN_POINTER()
{
	GlobalUnlock(h);
}

MSWIN_POINTER&
MSWIN_POINTER::operator=(const MSWIN_POINTER& m)
{
	GlobalUnlock(h);
	h = m.h;
	ptr = GlobalLock(h);
	return *this;
}


MSWIN_POINTER::operator void *() const
{
	// Lock it so the pointer doesn't become invalid,
	// but... there is no way to unlock it for now...
	// this is just for testing
    GlobalLock(h);
	return ptr;
}

int
MSWIN_POINTER::no_space() const
{
	return h == 0;
}
