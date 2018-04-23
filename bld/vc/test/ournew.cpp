#ifdef LEAK
#define CPPLEAK
#endif
#include <stdlib.h>
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/ournew.cpp 1.46 1997/10/05 17:27:06 dwight Stable $";
#ifdef USE_MOBJ
#include "ialloc.h"
#endif
#ifdef CPPLEAK
#include "useful.h"
#include <stdio.h>
#include <string.h>
#endif


#include <malloc.h>
#ifndef CPPLEAK
extern "C" {

#include "..\dbmall\malloc.h"
};
#endif


#ifdef USE_MOBJ
extern Allocator *Default_alloc;
#endif

int beginning_of_world = 1;
static int Firstnew;
#define MDB_LEV (0xfc7f)
#define MDB_LEV 0x00417

#ifdef CPPLEAK
static FILE *leak_strm;
#endif
void
energize_me()
{
#ifdef CPPLEAK
if(beginning_of_world) return;
if(!Firstnew)
{
	if((leak_strm = fopen("leak.trc", "wb")) == 0)
	{
		::abort();
    }
   Firstnew = 1;
}
#endif

}
char *mystery_filename = "mystery";
char *__nl_file = mystery_filename;
int __nl_line = 0;

#ifdef CPPLEAK
#include "cppleak.h"
#ifdef _Windows
#include <toolhelp.h>
#else
#ifndef LINUX
#include <dos.h>
#endif
#endif


static void
write_leak_rec(int type, void *p, long s, const char *file, long line)
{
	static long op = 0;
	leakrec r;
	memset(&r, 0, sizeof(r));
	r.type = type;
	r.ptr = p;
	r.size = s;
#ifndef LINUX
#ifdef _Windows
	TIMERINFO t;
	t.dwSize = sizeof(t);
	TimerCount(&t);
	r.real_time = t.dwmsSinceStart;
#else
	struct time t;
	gettime(&t);
	r.real_time = t.ti_min * 60 * 100 + t.ti_sec * 100 + t.ti_hund;
#endif
#endif
	r.opnum = op++;	
	const char *f = strrchr(file, '\\');
	if(f == 0)
		f = file;
	else
    	++f;
	strncpy(r.file, f, FN_SIZE - 1);
	r.line = line;
    fwrite(&r, sizeof(r), 1, leak_strm);
}
#endif


#ifndef LINUX
void far *
operator new(unsigned long s)
{
energize_me();
	if(s == 0)
		s = 1;
	if(beginning_of_world)
	{
		__nl_file = mystery_filename;
		__nl_line = 0;
		return farmalloc(s);
	}
#ifdef USE_MOBJ
	return (void*)Default_alloc->alloc(s);
#else
#ifndef CPPLEAK
	void *p = _malloc_leap(__nl_file, __nl_line, s);
#else
#ifdef LINUX
	void *p = malloc(s);
#else
	void *p = farmalloc(s);
#endif
#endif
	if(p == 0)
		::abort();
#ifdef CPPLEAK
	write_leak_rec(LEAK_NEW, p, s, __nl_file, __nl_line);
#endif
	__nl_file = mystery_filename;
	__nl_line = 0;

	return p;
#endif
}
#endif


void *
operator new(size_t s)
{
energize_me();
	if(s == 0)
		s = 1;
	if(beginning_of_world)
	{
		__nl_file = mystery_filename;
		__nl_line = 0;
#ifdef LINUX
		return (void *)malloc(s);
#else
		return farmalloc(s);
#endif
	}
#ifdef USE_MOBJ
	return (void*)Default_alloc->alloc(s);
#else
#ifndef CPPLEAK
	void *p = _malloc_leap(__nl_file, __nl_line, s);
#else
#ifdef LINUX
	void *p = malloc(s);
#else
	void *p = farmalloc(s);
#endif
#endif
	if(p == 0)
		::abort();
#ifdef CPPLEAK
	write_leak_rec(LEAK_NEW, p, s, __nl_file, __nl_line);
#endif
	__nl_file = mystery_filename;
	__nl_line = 0;

	return p;
#endif
}


void
operator delete(void *p)
{
energize_me();
	if(p == 0)
		return;
	if(beginning_of_world)
	{
		__nl_file = mystery_filename;
		__nl_line = 0;
#ifdef LINUX
		free(p);
#else
		farfree(p);
#endif
		return;
	}
#ifdef USE_MOBJ
	Default_alloc->free(p);
#else
#ifdef CPPLEAK
	write_leak_rec(LEAK_DELETE, p, 0, __nl_file, __nl_line);
	__nl_file = mystery_filename;
	__nl_line = 0;
#endif
#ifndef CPPLEAK
	_free_leap("?delete", 3, (char *)p);
#else
#ifdef LINUX
	free(p);
#else
	farfree(p);
#endif
#endif
#endif
}

#if 0
// note: this didn't work because bc3.1 doesn't do
// new (place) T[] and delete [] properly (doesn't
// get the count right, so delete goes off into space.)

void *
operator new(size_t s, const char *file, int line)
{
energize_me();
	if(s == 0)
		s = 1;
	if(beginning_of_world)
#ifndef CPPLEAK
		return _malloc_leap(file, line, s);
#else
		return farmalloc(s);
#endif
#ifdef USE_MOBJ
	return (void*)Default_alloc->alloc(s);
#else
#ifndef CPPLEAK
	void *p = _malloc_leap(file, line, s);
#else
	void *p = farmalloc(s);
	write_leak_rec(LEAK_NEW, p, s, file, line);
#endif
	if(p == 0)
    	::abort();
	return p;

#endif
}

void *
operator new(unsigned long s, const char *file, int line)
{
energize_me();
	if(s == 0)
		s = 1;
	if(beginning_of_world)
		return farmalloc(s);

#ifdef USE_MOBJ
	return (void*)Default_alloc->alloc(s);
#else
#ifndef CPPLEAK
	void *p = _malloc_leap(file, line, s);
#else
	void *p = farmalloc(s);
	write_leak_rec(LEAK_NEW, p, s, file, line);
#endif
	if(p == 0)
    	::abort();
	return p;

#endif
}
#endif

#ifndef CPPLEAK
#undef free
#undef malloc
/*
 * trap calls that go direct to malloc
 */
void
free(char * pnt)
{
	_free_leap("dirfree", 1, pnt);
}

void *
malloc(unsigned int size)
{
energize_me();

return	_malloc_leap("dirmalloc", 2, size);
}

#endif
