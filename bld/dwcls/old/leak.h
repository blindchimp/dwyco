
/*
 * $Header: g:/dwight/repo/dwcls/rcs/leak.h 1.10 1997/06/01 04:40:12 dwight Stable095 $
 */
#ifndef LEAK_H
#define LEAK_H

#include "allret.h"
#include "ialloc.h"
#include "dwlist.h"
#include "exmap.h"
#include "iostream.h"

//
// Use this class to track allocations/frees in the containing
// allocator. This allocator keeps a table of allocations, and
// whenever an operation comes in, it records it in the table.
// Allocations results in an entry being put in the table,
// frees remove entries from the table (note: bogus frees may
// cause a core dump before anything can be done about them.)
//
// To get a report about the state of allocations, call the print_report
// function.
//

class LeakImp : public Allocator
{
private:
	ExistMap<ALLRET> ptrmap;
	DwList<ALLRET> bad_frees;
	int mapparm;


public:
	LeakImp(int mapsize = 53);
	LeakImp(Allocator *a, int mapsize = 53);
	~LeakImp();

	virtual ALLRET& alloc(u_long byte_count);
	virtual void free(ALLRET&);
	virtual void *map_to_phys(const ALLRET&);
	virtual unsigned char *start_addr();
	virtual Allocator *make_one_like() const;

    void print_report(const ostream& = cout);
};

class LEAK_POINTER : public ALLRET
{

friend class LeakImp;

private:
	ALLRET& user_ptr;
	static ALLRET_alloc<LEAK_POINTER> *pool;
	
public:
	LEAK_POINTER(Allocator *a, ALLRET& p) : ALLRET(a), user_ptr(p) {}
	LEAK_POINTER()  : user_ptr(*this) {}
	~LEAK_POINTER() { delete &user_ptr; }
	void *operator new(size_t) {return pool->alloc();}
	void operator delete(void *p) {pool->free((LEAK_POINTER *)p);}
	virtual operator void *() const {return (void *)user_ptr;}

	static void init_pool(int nhandles = 100);
	int no_space() const { return user_ptr.no_space(); }
};



#endif
