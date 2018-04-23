
/*
 * $Header: g:/dwight/repo/dwcls/rcs/chall.h 1.10 1997/06/01 04:39:45 dwight Stable095 $
 */
#ifndef CHALL_H
#define CHALL_H

#include "allret.h"
#include "ialloc.h"

//
// This class template allows you to create an allocator that is a list
// of suballocators embedded in a containing allocator. This allocator
// can be destroyed in-toto, even if the sub-allocators do not support
// in-toto destruction.
//
// By specifying ChainAllocImp<FooImp>(Allocator *container)
// you get an allocator whose components are FooImp's, each of which
// is contained in container.
//

template<class A>
class ChainAllocImp : public Allocator
{
private:
	struct list_node
	{
		list_node(Allocator *a) {allocator = a; next = 0;}
		list_node(Allocator *a, list_node *n) {allocator = a; next = n;}
        ~list_node() { delete allocator; }
		Allocator *allocator;
		struct list_node *next;
	};
	list_node *head;

public:
	ChainAllocImp() { head = new list_node(new A);}
	ChainAllocImp(Allocator *a) : Allocator(a) { head = new list_node(new A(a));}
	ChainAllocImp(Allocator *a, u_long arg) : Allocator(a) {head = new list_node(new A(a, arg));}
	~ChainAllocImp();

	ALLRET& alloc(u_long byte_count);
	void free(ALLRET& p) { p.from->free(p); }
	void *map_to_phys(const ALLRET&) {return 0;}
};

template<class A>
ALLRET&
ChainAllocImp<A>::alloc(u_long bytes)
{
	ALLRET& tmp = head->allocator->alloc(bytes);
	if(tmp.no_space())
	{   //note: bogus copy constructor call, need new function here...
		head = new list_node (head->allocator->make_one_like(), head);
		return head->allocator->alloc(bytes);
	}
	return tmp;
}

template<class A>
ChainAllocImp<A>::~ChainAllocImp()
{
	list_node *tmp;

	for(list_node *l = head; l != 0; l = tmp)
	{
    	tmp = l->next;
		delete l;
    }
}

#endif
