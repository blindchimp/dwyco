/******************************************************************************
* fmal.cpp: fine grained allocator
******************************************************************************/

#ifndef lint
static char *RCSid = "$Header: g:/dwight/repo/dwcls/rcs/fmal.cpp 1.10 1994/10/23 09:37:41 dwight Stable095 $";
#endif

#undef FM_DEBUG
#undef KEEPLIST
#include "fmal.h"


#include <string.h>
#ifdef KEEPLIST
#include "c_lists.h"
#endif

//#define DEFAULT_BLOCKSIZE (8192)
#define BLOCK_SIZE (FineMallocImp::fm_blocksize)
#define WE_ALLOCED (0xfacadeL)
#define WE_FREED (0xdefacedL)
#define OUR_GOOP (sizeof(cur_blk) + sizeof(long))
#ifdef ALIGN
#define ROUNDUP(a) ((a) + ((-((a)&7))&7))
#else
#define ROUNDUP(a) (a)
#endif
#undef NULL
#define NULL (0)

#define NLONG 5


FineMallocImp::FineMallocImp(u_long chunk_size) : Allocator(Default_alloc)
{
	fm_blocksize = chunk_size;
	cur_blk = NULL;
	blocks_out = 0;
}


FineMallocImp::FineMallocImp(Allocator *a, u_long chunk_size) : Allocator(a)
{
	fm_blocksize = chunk_size;
	cur_blk = NULL;
	blocks_out = 0;
}

FineMallocImp::~FineMallocImp()
{
}

ALLRET&
FineMallocImp::alloc(u_long size)
{
	long actual_size;
    char *ap;

#ifdef KEEPLIST
	if(Blk_list == NULL)
		Blk_list = list_alloc();
#endif

	actual_size = ROUNDUP(size + OUR_GOOP);

	if(actual_size > BLOCK_SIZE)
#ifndef NO_FREE_CHECK
		::abort();
#else
		return container->alloc(size);
#endif

	if(cur_blk == NULL || actual_size > ((struct fm_blk *)(void*)cur_blk)->space_left)
    {
		cur_blk = &(container->alloc(sizeof(fm_blk)));
		new (cur_blk) fm_blk(container, BLOCK_SIZE);
    }

    struct fm_blk *cb = (struct fm_blk *)(void *)(*cur_blk);
	ap = cb->alloc_ptr;
    *(ALLRET **)ap = cur_blk;
    *(long  *)(ap + sizeof(cur_blk)) = WE_ALLOCED;
	cb->alloc_ptr += actual_size;
	cb->space_left -= actual_size;
	++cb->num_allocs;

	return *new POINTER(ap + OUR_GOOP);
}

void
FineMallocImp::free(ALLRET& ptr)
{
    char  *actual_ptr;

    actual_ptr = (char  *)(void *)ptr - OUR_GOOP;
#ifdef ALIGN
	if(((long)actual_ptr & 7) != 0 ||
		*(long *)(actual_ptr + sizeof(cur_blk)) != WE_ALLOCED)
#else
    if(*(long  *)(actual_ptr + sizeof(cur_blk)) != WE_ALLOCED)
#endif
	{
		/* just assume we got it from malloc... */
		//free(ptr);
		//return;
#ifndef NO_FREE_CHECK
		::abort();
		//oopanic("bad free");
#endif
		return;
	}

    *(long  *)(actual_ptr + sizeof(cur_blk)) = WE_FREED;
	ALLRET *ablk = *(ALLRET **)actual_ptr;
    struct fm_blk *blk = (struct fm_blk *)(void *)(*ablk);
	if(--blk->num_allocs == 0)
	{
		if(ablk == cur_blk)
			cur_blk = NULL;
#ifdef KEEPLIST
		list_setpos(Blk_list, blk->backptr);
		list_delete(Blk_list);
#endif
		container->free(*blk->mem);
		container->free(*ablk);
		--blocks_out;
#if 0
		if((blocks_out % 10) == 0) cout << blocks_out << "\n";
#endif
	}
    delete &ptr;
}

void *
FineMallocImp::map_to_phys(const ALLRET& p)
{
	return (void *)p;
}


FineMallocImp::fm_blk::fm_blk(Allocator *a, u_long chunk_size)
{
	mem = &(a->alloc(chunk_size));
    alloc_ptr = (char *)(void *)(*mem);
	space_left = chunk_size;
	num_allocs = 0;

#if defined(FM_DEBUG) || defined(KEEPLIST)
	memset(mem, 0xaa, chunk_size);
#endif
#if 0
#ifdef KEEPLIST
	list_append(Blk_list, this);
	list_fastforward(Blk_list);
	blk->backptr = list_getpos(Blk_list);
#endif
	++blocks_out;
#if 0
	if((blocks_out % 10) == 0) cout << blocks_out << "\n";
#endif
#endif
}

void *
FineMallocImp::fm_blk::operator new(size_t s, ALLRET *a)
{
	return (void *)(*a);
}

#ifdef KEEPLIST
void
FineMallocImp::check_ptr(char  *p)
{
	char  *hp = p;
	struct fm_blk *f;

	foreach(struct fm_blk *, f, Blk_list)
	{
		if(hp >= (f->mem + OUR_GOOP) && hp < f->alloc_ptr)
		{
			// found block pointer is in, check to make sure
			// it's not obviously bogus
			char  *ptr;
			for(ptr = hp; ptr > f->mem; --ptr)
			{
				if(*(long  *)ptr == WE_ALLOCED)
					return; // probably ok
				if(*(long  *)ptr == WE_FREED)
					oopanic("pointer references freed memory");
			}
			oopanic("can't find allocation tag before pointer");
		}
	}
	oopanic("pointer doesn't reference any currently allocated block");
}
			
		
void
FineMallocImp::dump_list(void)
{
	int i;
	struct fm_blk *f;
	long num_found;

	printf("Total Blocks still allocated: %ld\n", blocks_out);
	foreach(struct fm_blk *, f, Blk_list)
	{
		printf("addr %08p, numallocs %ld, space_left %d\n",
			(char  *)f->mem, f->num_allocs, f->space_left);
		num_found = 0;
		for(i = 0; i < BLOCK_SIZE - sizeof(long); ++i)
		{
			if(*(long  *)(f->mem + i) == WE_ALLOCED)
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

void
FineMallocImp::dump_a_bit(char  *p, int nlongs)
{
	int i;

	printf("%08p: ", (char  *)p);
	for(i = 0; i < nlongs; ++i)
	{
		printf("%08lx ", *((long  *)p + i));
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

