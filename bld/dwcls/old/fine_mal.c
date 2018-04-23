/******************************************************************************
* fine_malloc.c: fine grained allocator
******************************************************************************/

#ifndef lint
static char *RCSid = "$Header: g:/dwight/repo/dwcls/rcs/fine_mal.c 1.8 1994/10/23 09:37:39 dwight Stable095 $";
#endif

#ifdef __cplusplus
#include "pchdr.h"
#pragma hdrstop

#include "rtglob.h"
#endif
#undef NOT_REALLY
#undef FM_DEBUG
#define KEEPLIST
#undef malloc
#undef realloc
#undef free

#ifdef _Windows
#define OPS_WIN
#endif

#include <alloc.h>
#include <mem.h>
#ifdef OPS_WIN
#include <windows.h>
#endif
#ifdef KEEPLIST
#include "c_lists.h"
#endif

int Fm_blocksize = 2048;
#define BLOCK_SIZE (Fm_blocksize)
#define WE_ALLOCED (0xfacadeL)
#define WE_FREED (0xdefacedL)
#define OUR_GOOP (sizeof(struct fm_blk *) + sizeof(long))
#ifdef ALIGN
#define ROUNDUP(a) ((a) + ((-((a)&7))&7))
#else
#define ROUNDUP(a) (a)
#endif
#ifndef NULL
#define NULL (0)
#endif

struct fm_blk
{
	long size;
	long align;
	long num_allocs;
	char  huge *alloc_ptr;
	int space_left;
	char  huge *mem;
#ifdef OPS_WIN
	HGLOBAL blk_handle;
#endif
#ifdef KEEPLIST
	void *backptr;
#endif
};
#ifdef KEEPLIST
static CList Blk_list;
void dumplist(void);
static void dump_a_bit(char huge *, int);
#define NLONG 5
#endif

static struct fm_blk  huge *Cur_blk = NULL;
static struct fm_blk  huge *new_blk(void);

static long Blocks_out;

void *
f_malloc(unsigned int size)
{
	long actual_size;
	char  huge *ap;

#ifdef KEEPLIST
	if(Blk_list == NULL)
		Blk_list = list_alloc();
#endif

	actual_size = ROUNDUP(size + OUR_GOOP);

#ifdef NOT_REALLY
	ap = (char  huge *)malloc(actual_size);
	*(long  huge *)(ap + 4) = WE_ALLOCED;
	return ap + OUR_GOOP;
#endif

	if(actual_size > BLOCK_SIZE)
		return malloc(size);

	if(Cur_blk == NULL || actual_size > Cur_blk->space_left)
		Cur_blk = new_blk();
	ap = Cur_blk->alloc_ptr;
	*(struct fm_blk  huge **)ap = Cur_blk;
	*(long  huge *)(ap + 4) = WE_ALLOCED;
	Cur_blk->alloc_ptr += actual_size;
	Cur_blk->space_left -= actual_size;
	++Cur_blk->num_allocs;

	return ap + OUR_GOOP;
}

void
f_free(void  *ptr)
{
	char  huge *actual_ptr;
	struct fm_blk  huge *blk;

	actual_ptr = (char  huge *)ptr - OUR_GOOP;
#ifdef ALIGN
	if(((long)actual_ptr & 7) != 0 || *(long *)(actual_ptr + 4) != WE_ALLOCED)
#else
	if(*(long  huge *)(actual_ptr + 4) != WE_ALLOCED)
#endif
	{
		/* just assume we got it from malloc... */
		//free(ptr);
		//return;
		oopanic("bad free");
		return;
	}

#ifdef NOT_REALLY
	*(long  huge *)(actual_ptr + 4) = WE_FREED;
	free(actual_ptr);
	return;
#endif

	*(long  huge *)(actual_ptr + 4) = WE_FREED;
	blk = *(struct fm_blk  huge **)actual_ptr;
	if(--blk->num_allocs == 0)
	{
		if(blk == Cur_blk)
			Cur_blk = NULL;
#ifdef KEEPLIST
		list_setpos(Blk_list, blk->backptr);
		list_delete(Blk_list);
#endif
#ifdef OPS_WIN
		GlobalUnlock(blk->blk_handle);
		GlobalFree(blk->blk_handle);
#else
		free(blk->mem);
		free(blk);
#endif
		--Blocks_out;
#if 0
		if((Blocks_out % 10) == 0) cout << Blocks_out << "\n";
#endif
		return;
	}
}

void *
f_realloc(void *ptr, long old_size, long new_size)
{
	void *new_ptr;

	new_ptr = f_malloc(new_size);
	memcpy(new_ptr, ptr, old_size);
	f_free(ptr);
	return new_ptr;
}

static struct fm_blk  huge *
new_blk(void)
{
	struct fm_blk  huge *blk;

	blk = (struct fm_blk huge *) malloc(sizeof(*blk));
	if(blk == NULL)
		oopanic("out of memory (newblk)");
#ifdef OPS_WIN
        blk->blk_handle = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, BLOCK_SIZE);
	blk->alloc_ptr = blk->mem = GlobalLock(blk->blk_handle);
	blk->space_left = GlobalSize(blk->blk_handle);
#else
	blk->alloc_ptr = blk->mem = (char huge *)malloc(BLOCK_SIZE);
	blk->space_left = BLOCK_SIZE;
#endif
	if(blk->mem == NULL)
		oopanic("out of memory (newblk2)");
	blk->num_allocs = 0;
#if defined(FM_DEBUG) || defined(KEEPLIST)
	memset(blk->mem, 0xaa, BLOCK_SIZE);
#endif
#ifdef KEEPLIST
	list_append(Blk_list, blk);
	list_fastforward(Blk_list);
	blk->backptr = list_getpos(Blk_list);
#endif
	++Blocks_out;
#if 0
	if((Blocks_out % 10) == 0) cout << Blocks_out << "\n";
#endif

	return blk;
}

#ifdef KEEPLIST
void
check_ptr(char huge *p)
{
	char huge *hp = p;
	struct fm_blk *f;

	foreach(struct fm_blk *, f, Blk_list)
	{
		if(hp >= (f->mem + OUR_GOOP) && hp < f->alloc_ptr)
		{
			// found block pointer is in, check to make sure
			// it's not obviously bogus
			char huge *ptr;
			for(ptr = hp; ptr > f->mem; --ptr)
			{
				if(*(long huge *)ptr == WE_ALLOCED)
					return; // probably ok
				if(*(long huge *)ptr == WE_FREED)
					oopanic("pointer references freed memory");
			}
			oopanic("can't find allocation tag before pointer");
		}
	}
	oopanic("pointer doesn't reference any currently allocated block");
}
			
		
void
dumplist(void)
{
	int i;
	struct fm_blk *f;
	long num_found;

	printf("Total Blocks still allocated: %ld\n", Blocks_out);
	foreach(struct fm_blk *, f, Blk_list)
	{
		printf("addr %08p, numallocs %ld, space_left %d\n",
			(char huge *)f->mem, f->num_allocs, f->space_left);
		num_found = 0;
		for(i = 0; i < BLOCK_SIZE - sizeof(long); ++i)
		{
			if(*(long huge *)(f->mem + i) == WE_ALLOCED)
			{
				++num_found;
				dump_a_bit(f->mem + i,
					((i + NLONG * sizeof(long) > BLOCK_SIZE)? ((BLOCK_SIZE - i) / sizeof(long)) : NLONG));
			}
		}
		if(num_found != f->num_allocs)
			printf("WARNING: allocation markers hosed (expected %ld, found %ld)!\n",
				f->num_allocs, num_found);
	}
    fflush(stdout);
}

static void
dump_a_bit(char huge *p, int nlongs)
{
	int i;

	printf("%08p: ", (char huge *)p);
	for(i = 0; i < nlongs; ++i)
	{
		printf("%08lx ", *((long huge *)p + i));
	}
	for(i = 0; i < nlongs * sizeof(long); ++i)
	{
		char c;
		c = *(p + i);
		if(isascii(c) && isprint(c))
			putchar(c);
		else
			putchar('.');
	}
	printf("\n");
}

#endif

#ifdef __cplusplus
#if 0
void *
operator new(size_t size)
{
	unsigned long s = size;
	return operator new(s);
}
#endif
void *
operator new(size_t size)
{
#if !defined(OPS_WIN) && defined(FM_DEBUG)
	if(heapcheck() < 0)
		oopanic("bogus heap");
	if(heapcheckfree(0xaa55) < 0)
		oopanic("bogus free blocks");
#endif
	if(size == 0)
		size = 1;
	void *ret = f_malloc(size);
	if(ret == 0)
		oopanic("out of memory");
//	printf("malloc %d = %lx\n", size, ret);
	return ret;
}

void
operator delete(void *ptr)
{
//	printf("delete %lx\n", ptr);
#if !defined(OPS_WIN) && defined(FM_DEBUG)
	if(heapcheck() < 0)
		oopanic("bogus heap (free)");
	if(heapcheckfree(0xaa55) < 0)
		oopanic("bogus free blocks(free)");
#endif
	if(ptr == 0)
		return;
	f_free(ptr);

#if !defined(OPS_WIN) && defined(FM_DEBUG)
	if(heapfillfree(0xaa55) < 0)
		oopanic("bogus fillfree");
#endif

}
#endif
