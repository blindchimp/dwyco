// $Header: g:/dwight/repo/cdc32/rcs/gnumal.c 1.16 1999/01/10 16:09:54 dwight Checkpoint $
#undef TRAPMALLOC
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)malloc.c	5.9 (Berkeley) 6/1/90";
#endif /* LIBC_SCCS and not lint */

/*
 * malloc.c (Caltech) 2/21/82
 * Chris Kingsley, kingsley@cit-20.
 *
 * This is a very fast storage allocator.  It allocates blocks of a small 
 * number of different sizes, and keeps free lists of each size.  Blocks that
 * don't exactly fit are passed up to the next larger size.  In this 
 * implementation, the available sizes are 2^n-4 (or 2^n-10) bytes long.
 * This is designed for use in a virtual memory environment.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef __WIN32__
#include <windows.h>
/* note: this is bc45 stuff, locking stuff that
 * was used in the bc rtl memory manager, just
 * ripping it off to use here.
 */
#include "_thread.h"
#else
void
_lock_heap() {}
void
_unlock_heap() {}
#endif

#undef NULL
#define	NULL 0
#undef RCHECK
#undef DEBUG
#if 0
#define log(f, a, b, c) printf(f "\n", a, b, c);
#endif
#define log(f, a, b, c) 

/*
 * The overhead on a block is at least 4 bytes.  When free, this space
 * contains a pointer to the next free block, and the bottom two bits must
 * be zero.  When in use, the first byte is set to MAGIC, and the second
 * byte is the size index.  The remaining bytes are for alignment.
 * If range checking is enabled then a second word holds the size of the
 * requested block, less 1, rounded up to a multiple of sizeof(RMAGIC).
 * The order of elements is critical: ov_magic must overlay the low order
 * bits of ov_next, and ov_magic can not be a valid ov_next bit pattern.
 */
#ifdef __WIN32__
typedef unsigned char u_char;
typedef unsigned char *caddr_t;
typedef unsigned int u_int;
#endif
union	overhead {
	union	overhead *ov_next;	/* when free */
	struct {
		u_char	ovu_magic;	/* magic number */
		u_char	ovu_index;	/* bucket # */
#ifdef RCHECK
		u_short	ovu_rmagic;	/* range magic number */
		u_int	ovu_size;	/* actual block size */
#endif
	} ovu;
#define	ov_magic	ovu.ovu_magic
#define	ov_index	ovu.ovu_index
#define	ov_rmagic	ovu.ovu_rmagic
#define	ov_size		ovu.ovu_size
};

#define	MAGIC		0xef		/* magic # on accounting info */
#define RMAGIC		0x5555		/* magic # on range info */

#ifdef RCHECK
#define	RSLOP		sizeof (u_short)
#else
#define	RSLOP		0
#endif

/*
 * for each page allocated, a little info so we can
 * tell if the page can be reallocated. the main reason for
 * this is that the free used in this allocator will tend to
 * create free lists which, when used by malloc, create widely
 * spaced allocations and no chance for coalescing pages.
 */
struct blockhead
{
	struct balloced {
		short count;
		short resv;
		union overhead *first_free;
	} bal;
	struct blockhead *next;
	struct blockhead *prev;
#define bh_count bal.count
#define bh_first_free bal.first_free
};

static struct blockhead *page_top;
static struct blockhead *page_bottom;

/*
 * nextf[i] is the pointer to the next free block of size 2^(i+3).  The
 * smallest allocatable block is 8 bytes.  The overhead information
 * precedes the data area returned to the user.
 */
#define	NBUCKETS 30
static	union overhead *nextf[NBUCKETS];
static	struct blockhead pagelists[NBUCKETS];
static	struct blockhead *freepages;

static	int pagesz;			/* page size */
static	int pagebucket;			/* page size bucket */
static	int pagemask;

#ifdef MSTATS
/*
 * nmalloc[i] is the difference between the number of mallocs and frees
 * for a given block size.
 */
static	u_int nmalloc[NBUCKETS];
#endif

static void morecore(int bucket);
static int findbucket(union overhead *freep, int srchlen);

#if defined(DEBUG) || defined(RCHECK)
#define	ASSERT(p)   if (!(p)) botch(#p)
static
botch(s)
	char *s;
{
	fprintf(stderr, "\r\nassertion botched: %s\r\n", s);
 	(void) fflush(stderr);		/* just in case user buffered it */
	*((int *)-1) = 0; /* force fault; abort will probably fail if malloc's dead */
	abort();
}
#else
#define	ASSERT(p)
#endif

#ifdef __WIN32__
static int getpagesize(void);

static int
getpagesize()
{
	SYSTEM_INFO s;
	GetSystemInfo(&s);
	return s.dwPageSize;
}

static void *
sbrk(int n)
{
	void *ret = VirtualAlloc(0, n, MEM_COMMIT, PAGE_READWRITE);
	if(ret == 0)
		return (void *)-1;
	return ret;
}

static void bzero(char *v, int n)
{
	memset(v, 0, n);
}

static void bcopy(char *s, char *d, int n)
{
	memmove(d, s, n);
}

static lock_t _heap_lock;
static void _RTLENTRY _init_heap()
{
#pragma startup _init_heap 2
	_create_lock(&_heap_lock,"creating heap lock");
}

static void _lock_heap(void)
{
    _lock(_heap_lock,"locking heap");
}

static void _unlock_heap(void)
{
    _unlock(_heap_lock,"unlocking heap");
}


#endif

void *
#ifdef TRAPMALLOC
real_malloc(size_t nbytes)
#else
malloc(nbytes)
size_t nbytes;
#endif

{
  	register union overhead *op;
  	register int bucket, n;
	register unsigned amt;
	struct blockhead *bh;
	_lock_heap();

	/*
	 * First time malloc is called, setup page size and
	 * align break pointer so all data will be page aligned.
	 */
	if (pagesz == 0) {
		for(n = 0; n < NBUCKETS; ++n)
		{
			pagelists[n].prev = &pagelists[n];
			pagelists[n].next = &pagelists[n];
		}
		pagesz = n = getpagesize();
		pagemask = ~(n - 1);
#ifndef __WIN32__
		op = (union overhead *)sbrk(0);
  		/*n = n - sizeof (*op) - ((int)op & (n - 1));*/
		/* sbrk now will return page aligned chunks, the
		 * above thing had sbrk returning chunks 4 bytes
		 * less than aligned, and then presumably the
		 * pointers might be page aligned, but that's not
		 * what we want anymore
		 */
  		n = n - ((int)op & (n - 1));
		if (n < 0)
			n += pagesz;
  		if (n) {
  			if (sbrk(n) == (char *)-1)
				return (NULL);
		}
#endif
		bucket = 0;
		amt = 8;
		while (pagesz > amt) {
			amt <<= 1;
			bucket++;
		}
		pagebucket = bucket;
	}
	/*
	 * Convert amount of memory requested into closest block size
	 * stored in hash buckets which satisfies request.
	 * Account for space used per block for accounting.
	 */
	if (nbytes <= (n = pagesz - sizeof (*op) - RSLOP)) {
#ifndef RCHECK
		amt = 8;	/* size of first bucket */
		bucket = 0;
#else
		amt = 16;	/* size of first bucket */
		bucket = 1;
#endif
		n = -(sizeof (*op) + RSLOP);
	} else {
		amt = pagesz;
		bucket = pagebucket;
	}
	while (nbytes > amt + n) {
		amt <<= 1;
		if (amt == 0)
		{
			_unlock_heap();
			return (NULL);
		}
		bucket++;
	}
	/*
	 * If nothing in hash bucket right now,
	 * request more memory from the system.
	 */
  	if ((op = nextf[bucket]) == NULL) {
log("malloc buck %d null", bucket, 0, 0);
		if(bucket < pagebucket)
		{
		struct blockhead *b = pagelists[bucket].next;
		while(b != &pagelists[bucket])
		{
			if(b->bh_first_free)
			{
				nextf[bucket] = b->bh_first_free;
				op = nextf[bucket];
log("malloc using bh %x", b, 0, 0);
				goto out;
			}
			b = b->next;
		}
		}
  		morecore(bucket);
  		if ((op = nextf[bucket]) == NULL)
		{
			_unlock_heap();
  			return (NULL);
		}
	}
out:
	bh = (struct blockhead *)(((unsigned int)op) & pagemask);
	ASSERT(bh->bh_count >= 0);
	++bh->bh_count;
log("malloc bh %x count %d", bh, bh->bh_count, 0);
	if(((unsigned int)op->ov_next & pagemask) == (unsigned int)bh)
		bh->bh_first_free = op->ov_next;
	else
		bh->bh_first_free = 0;
	/* remove from linked list */
  	nextf[bucket] = op->ov_next;
	op->ov_magic = MAGIC;
	op->ov_index = bucket;
#ifdef MSTATS
  	nmalloc[bucket]++;
#endif
#ifdef RCHECK
	/*
	 * Record allocated size of block and
	 * bound space with magic numbers.
	 */
	op->ov_size = (nbytes + RSLOP - 1) & ~(RSLOP - 1);
	op->ov_rmagic = RMAGIC;
  	*(u_short *)((caddr_t)(op + 1) + op->ov_size) = RMAGIC;
	memset((char *)(op + 1), 0xaa, nbytes);
#endif
	_unlock_heap();
log("malloc ret %x", op + 1, 0, 0);
  	return ((char *)(op + 1));
}

void *
calloc(size,nelem) /* added by DJ Delorie */
     size_t size;
     size_t nelem;
{
#ifdef TRAPMALLOC
  char *rv = real_malloc(size*nelem);
#else
  char *rv = malloc(size*nelem);
#endif
  if (rv)
    bzero(rv, size*nelem);
  return rv;
}

/*
 * Allocate more memory to the indicated bucket.
 */
static void
morecore(bucket)
	int bucket;
{
  	register union overhead *op;
	register int sz;		/* size of desired block */
  	int amt;			/* amount to allocate */
  	int nblks;			/* how many blocks we get */
	struct blockhead *bh;

	/*
	 * sbrk_size <= 0 only for big, FLUFFY, requests (about
	 * 2^30 bytes on a VAX, I think) or for a negative arg.
	 */
	sz = 1 << (bucket + 3);
#ifdef DEBUG
	ASSERT(sz > 0);
#else
	if (sz <= 0)
		return;
#endif
	if (sz < pagesz) {
		amt = pagesz;
  		nblks = (amt - sizeof(struct blockhead)) / sz;
		if(freepages)
		{
			bh = freepages;
			freepages = freepages->next;
log("morecore sz %d using free bh %x", sz, bh, 0);
			goto nobrk;
		}
	} else {
		amt = sz + pagesz;
		nblks = 1;
	}
	bh = (struct blockhead *)sbrk(amt);
log("morecore bh %x", bh, 0, 0);
	/* no more room! */
  	if ((int)bh == -1)
  		return;
nobrk:
	bh->bh_count = 0;
	op = (union overhead *)((caddr_t)bh + sizeof(*bh));
	bh->bh_first_free = op;
	bh->prev = &pagelists[bucket];
	bh->next = pagelists[bucket].next;
	pagelists[bucket].next->prev = bh;
	pagelists[bucket].next = bh;
	/*
	 * Add new memory allocated to that on
	 * free list for this hash bucket.
	 */
  	nextf[bucket] = op;
  	while (--nblks > 0) {
		op->ov_next = (union overhead *)((caddr_t)op + sz);
		op = (union overhead *)((caddr_t)op + sz);
  	}
	op->ov_next = 0;
}

void
#ifdef TRAPMALLOC
real_free(void *cp)
#else
free(cp)
	void *cp;
#endif

{   
  	register int size;
	register union overhead *op;
	struct blockhead *bh;
	int s;

  	if (cp == NULL)
  		return;
	_lock_heap();
	op = (union overhead *)((caddr_t)cp - sizeof (union overhead));
#ifdef DEBUG
  	ASSERT(op->ov_magic == MAGIC);		/* make sure it was in use */
#else
	if (op->ov_magic != MAGIC)
	{
		_unlock_heap();
		return;				/* sanity */
	}
#endif
#ifdef RCHECK
  	ASSERT(op->ov_rmagic == RMAGIC);
	ASSERT(*(u_short *)((caddr_t)(op + 1) + op->ov_size) == RMAGIC);
	s = op->ov_size;
	memset(op + 1, 0x5a, s < 2 ? 0 : (s - 2));
#endif
  	size = op->ov_index;
  	ASSERT(size < NBUCKETS);
#if 0
	op->ov_next = nextf[size];	/* also clobbers ov_magic */
  	nextf[size] = op;
#endif
#ifdef MSTATS
  	nmalloc[size]--;
#endif
	/* instead of linking back into the first of
	 * the generic bucket, link it back into the
	 * page where it was allocated from
	 */
	bh = (struct blockhead *)(((unsigned int)op) & pagemask);
	--bh->bh_count;
log("free on bh %x cnt %d", bh, bh->bh_count, 0);
	ASSERT(bh->bh_count >= 0);
	if(bh->bh_count == 0)
	{
		if(size < pagebucket)
		{
log("free bh %x to freepages", bh, 0, 0);
			bh->prev->next = bh->next;
			bh->next->prev = bh->prev;
			bh->next = freepages;
			freepages = bh;
			nextf[size] = 0;
			op->ov_next = 0;
		}
		else
		{
			op->ov_next = nextf[size];	/* also clobbers ov_magic */
			nextf[size] = op;
		}
	}
	else
	{
		op->ov_next = bh->bh_first_free;
		bh->bh_first_free = op;
	}
	_unlock_heap();
}

/*
 * When a program attempts "storage compaction" as mentioned in the
 * old malloc man page, it realloc's an already freed block.  Usually
 * this is the last block it freed; occasionally it might be farther
 * back.  We have to search all the free lists for the block in order
 * to determine its bucket: 1st we make one pass thru the lists
 * checking only the first block in each; if that fails we search
 * ``realloc_srchlen'' blocks in each list for a match (the variable
 * is extern so the caller can modify it).  If that fails we just copy
 * however many bytes was given to realloc() and hope it's not huge.
 */
int realloc_srchlen = 4;	/* 4 should be plenty, -1 =>'s whole list */

#ifdef TRAPMALLOC
void *
real_realloc(void *cp, size_t nbytes)
#else
void *
realloc(cp, nbytes)
     void *cp; 
     size_t nbytes;
#endif
{   
  	register u_int onb;
	register int i;
	union overhead *op;
  	char *res;
	int was_alloced = 0;

  	if (cp == NULL)
#ifdef TRAPMALLOC
  		return (real_malloc(nbytes));
#else
  		return (malloc(nbytes));
#endif
	_lock_heap();
	op = (union overhead *)((caddr_t)cp - sizeof (union overhead));
	if (op->ov_magic == MAGIC) {
		was_alloced++;
		i = op->ov_index;
	} else {
		/*
		 * Already free, doing "compaction".
		 *
		 * Search for the old block of memory on the
		 * free list.  First, check the most common
		 * case (last element free'd), then (this failing)
		 * the last ``realloc_srchlen'' items free'd.
		 * If all lookups fail, then assume the size of
		 * the memory block being realloc'd is the
		 * largest possible (so that all "nbytes" of new
		 * memory are copied into).  Note that this could cause
		 * a memory fault if the old area was tiny, and the moon
		 * is gibbous.  However, that is very unlikely.
		 */
		if ((i = findbucket(op, 1)) < 0 &&
		    (i = findbucket(op, realloc_srchlen)) < 0)
			i = NBUCKETS;
	}
	onb = 1 << (i + 3);
	if (onb < pagesz)
		onb -= sizeof (*op) + RSLOP;
	else
		onb += pagesz - sizeof (*op) - RSLOP;
#if 0
	/* avoid the copy if same size block */
	if (was_alloced) {
		if (i) {
			i = 1 << (i + 2);
			if (i < pagesz)
				i -= sizeof (*op) + RSLOP;
			else
				i += pagesz - sizeof (*op) - RSLOP;
		}
		if (nbytes <= onb && nbytes > i) {
#ifdef RCHECK
			op->ov_size = (nbytes + RSLOP - 1) & ~(RSLOP - 1);
			*(u_short *)((caddr_t)(op + 1) + op->ov_size) = RMAGIC;
#endif
			_unlock_heap();
			return(cp);
		} else {
		// don't do free here since we put destructive stuff in
		// it for debugging.
		}
	}
#endif
#ifdef TRAPMALLOC
  	if ((res = real_malloc(nbytes)) == NULL)
#else
  	if ((res = malloc(nbytes)) == NULL)
#endif
	{
		_unlock_heap();
  		return (NULL);
	}
  	if (cp != res)		/* common optimization if "compacting" */
		bcopy(cp, res, (nbytes < onb) ? nbytes : onb);
#ifdef TRAPMALLOC
	real_free(cp);
#else
	free(cp);
#endif
	_unlock_heap();
  	return (res);
}

/*
 * Search ``srchlen'' elements of each free list for a block whose
 * header starts at ``freep''.  If srchlen is -1 search the whole list.
 * Return bucket number, or -1 if not found.
 */
static int
findbucket(freep, srchlen)
	union overhead *freep;
	int srchlen;
{
	register union overhead *p;
	register int i, j;

	for (i = 0; i < NBUCKETS; i++) {
		j = 0;
		for (p = nextf[i]; p && j != srchlen; p = p->ov_next) {
			if (p == freep)
				return (i);
			j++;
		}
	}
	return (-1);
}

#ifdef MSTATS
/*
 * mstats - print out statistics about malloc
 * 
 * Prints two lines of numbers, one showing the length of the free list
 * for each size category, the second showing the number of mallocs -
 * frees for each size category.
 */
mstats(s)
	char *s;
{
  	register int i, j;
  	register union overhead *p;
  	int totfree = 0,
  	totused = 0;

  	fprintf(stderr, "Memory allocation statistics %s\nfree:\t", s);
  	for (i = 0; i < NBUCKETS; i++) {
  		for (j = 0, p = nextf[i]; p; p = p->ov_next, j++)
  			;
  		fprintf(stderr, " %d", j);
  		totfree += j * (1 << (i + 3));
  	}
  	fprintf(stderr, "\nused:\t");
  	for (i = 0; i < NBUCKETS; i++) {
  		fprintf(stderr, " %d", nmalloc[i]);
  		totused += nmalloc[i] * (1 << (i + 3));
  	}
  	fprintf(stderr, "\n\tTotal in use: %d, total free: %d\n",
	    totused, totfree);
}
#endif

void
dump_free()
{
	int i, j;
	union overhead *p;
	FILE *f = fopen("free.out", "w");
  	for (i = 0; i < NBUCKETS; i++) {
  		for (j = 0, p = nextf[i]; p; p = p->ov_next, j++)
		{
			fprintf(f, "%d %x\n", 1 << (i + 3), (int)p/* / pagesz*/);
		}
  	}
	fclose(f);
}
