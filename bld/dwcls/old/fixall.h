
/*
 * $Header: g:/dwight/repo/dwcls/rcs/fixall.h 1.11 1997/06/01 04:40:05 dwight Stable095 $
 */
#ifndef FIXALL_H
#define FIXALL_H
#include <string.h>

#define dw_min(x, y) (((x) > (y)) ? (y) : (x))
#include "ialloc.h"
#include "allret.h"

#define FA_ALLOC 0x55
#define FA_FREE 0xaa

#define DEFAULT_NUM_NODES (100)
#define DEFAULT_FL_SIZE 20

//
// This allocator allocates fixed size chunks from a contiguous
// pool. A free list is kept to satify requests fast. When the free
// list is empty, the pool is scanned for unused blocks to refresh
// the free list. The size of the pool is fixed. The size of the allocation
// unit is fixed at compile time by specifying the number of bytes in the
// unit as the template argument.
//

template <u_long T>
class FixAlloc : public Allocator
{
private:
	ALLRET& node_space;
	ALLRET& free_list;
	u_long free_tos;
	u_long max_nodes;
	u_long free_list_len;

	void build_free_list();
    void do_init();

public:
	FixAlloc(u_long max_nodes = DEFAULT_NUM_NODES, u_long fl_size = DEFAULT_FL_SIZE);
	FixAlloc(Allocator *, u_long max_nodes = DEFAULT_NUM_NODES, u_long fl_size = DEFAULT_FL_SIZE);
	~FixAlloc();

	ALLRET& alloc(u_long bytes);
	void free(ALLRET&);
	void *map_to_phys(const ALLRET& p) {return (void *)p;}
    Allocator *make_one_like() const;
	  
};

template <u_long T>
FixAlloc<T>::FixAlloc(u_long nodes, u_long fl_len)
	: Allocator(Default_alloc),
	max_nodes(nodes),
	free_list_len(fl_len),
	free_list(container->alloc(sizeof(void *) * dw_min(fl_len, nodes))),
	node_space(container->alloc((T + 1) * nodes))
{
	
	free_list_len = dw_min(free_list_len, max_nodes);
	do_init();
}


template <u_long T>
FixAlloc<T>::FixAlloc(Allocator *a, u_long nodes, u_long fl_len)
	: Allocator(a),
	max_nodes(nodes),
	free_list_len(fl_len),
	free_list(container->alloc(sizeof(void *) * dw_min(nodes, fl_len))),
	node_space(container->alloc((T + 1) * nodes))
{


	free_list_len = dw_min(free_list_len, max_nodes);

	do_init();
}

template <u_long T>
void
FixAlloc<T>::do_init()
{
	memset(node_space, FA_FREE, (T + 1) * max_nodes);
	free_tos = free_list_len;

	void **fl = (void **)(void *)free_list;
	u_char *ns = (u_char *)(void *)node_space;
	for(int i = 0; i < free_list_len; ++i)
		fl[i] = &ns[i * (T + 1)];
}

template <u_long T>
FixAlloc<T>::~FixAlloc()
{
	container->free(free_list);
	container->free(node_space);
}

template <u_long T>
Allocator *
FixAlloc<T>::make_one_like() const
{
	return new FixAlloc<T>(container, max_nodes, free_list_len);
}

template <u_long T>
ALLRET&
FixAlloc<T>::alloc(u_long bytes)
{
	if(bytes > T)
		abort();
	if(free_tos == 0)
	{
		build_free_list();
		if(free_tos == 0)
			return *new POINTER(0);
	}
	u_char *ptr = (u_char *)((void **)(void *)free_list)[--free_tos];
	*ptr = FA_ALLOC;
	return *new POINTER(ptr + 1);
}

template <u_long T>
void
FixAlloc<T>::free(ALLRET& p)
{
	u_char *ptr = (u_char *)(void *)p;
	if(*--ptr != FA_ALLOC)
		abort();
	*ptr = FA_FREE;
	if(free_tos < free_list_len)
		((void **)(void *)free_list)[free_tos++] = ptr;
}

template <u_long T>
void
FixAlloc<T>::build_free_list()
{
	void **fl = (void **)(void *)free_list;
	u_char *ns = (u_char *)(void *)node_space;
	u_char *ens = ns + ((T + 1) * max_nodes);

	for(;ns < ens && free_tos < free_list_len; ns += (T + 1))
	{
		switch(*ns)
		{
		case FA_ALLOC:
			break;
		case FA_FREE:
			fl[free_tos++] = ns;
			break;
		default:
			abort();
		}
	}
}

#endif
