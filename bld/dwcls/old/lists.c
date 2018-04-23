/******************************************************************************
* Generic linked LIST routines
* 
* Define CAREFUL at compile time if you want basic sanity checking.
******************************************************************************/

#ifndef lint
static char RCSid[] = "$Header: g:/dwight/repo/dwcls/rcs/lists.c 1.8 1994/10/23 09:39:00 dwight Stable095 $";
#endif

#include "standard.h"
#include "lists.h"

struct LISTELEM
{
    POINTER data;				/* points to data */
    struct LISTELEM *next;
    struct LISTELEM *prev;
};
 
 
struct LIST
{
    int			count;
    LISTELEM	current;
    LISTELEM	listheader;
};


/******************************************************************************
* list_alloc: return an empty list
*
* Allocate a new LIST and return an opaque handle to the caller.
******************************************************************************/

LIST
list_alloc()
{
	LIST l;

	if ((l = (LIST) malloc(sizeof(*l))) == NULL)
	{
		exc_raise("A:LIST.OUT_OF_MEMORY", "list_alloc", NULL);
		/*NOTREACHED*/
	}
		
	l->count = 0;
	l->listheader = (LISTELEM) malloc(sizeof(*l->listheader));
	if(l->listheader == NULL)
	{
		exc_raise("A:LIST.OUT_OF_MEMORY", "list_alloc", NULL);
		/*NOTREACHED*/
	}
	l->listheader->data = NULL;
	l->listheader->prev = l->listheader->next = l->listheader;
	l->current = l->listheader;

	return l;
}

/******************************************************************************
* list_dealloc: toast a list.
*
* Deallocate a LIST - only deallocates the structures built by other
* routines in this package.  The data field is untouched.
* 
******************************************************************************/

void
list_dealloc(l)
LIST            l;
{
	LISTELEM        elem,
	                elem2;

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_dealloc", NULL);
#endif

	list_rewind(l);
	elem = l->listheader->next;
	while (l->count-- > 0)
	{

#ifdef CAREFUL
		if (elem == NULL)
			exc_raise("A:LIST.NULL_ELEM", "list_dealloc", NULL);
#endif

		elem2 = elem->next;
		free((POINTER )elem);
		elem = elem2;
	}
	free((POINTER )(l->listheader));
	free((POINTER )l);
}

/******************************************************************************
* list_append: put an element at the end of a list without changing current.
*
* Append the given data to the end of LIST l.
* The current element pointer is left unchanged.
******************************************************************************/

void
list_append(l, data)
LIST            l;
POINTER data;
{
	LISTELEM        e;

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_append", NULL);
#endif

	e = (LISTELEM) malloc(sizeof(*e));
	if(e == NULL)
	{
		exc_raise("A:LIST.OUT_OF_MEMORY", "list_append", NULL);
		/*NOTREACHED*/
	}
	e->data = data;
	e->next = l->listheader;
	e->prev = l->listheader->prev;
	l->listheader->prev->next = e;
	l->listheader->prev = e;
	++l->count;
}

/******************************************************************************
* list_prepend: put an element at the beginning of a list w/o changing current.
*
* Prepend the given data to the beginning of LIST l.
* The current element pointer is left unchanged.
******************************************************************************/

void
list_prepend(l, data)
LIST            l;
POINTER data;
{
	LISTELEM        e;

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_prepend", NULL);
#endif

	e = (LISTELEM) malloc(sizeof(*e));
	if(e == NULL)
	{
		exc_raise("A:LIST.OUT_OF_MEMORY", "list_prepend", NULL);
		/*NOTREACHED*/
	}
	e->data = data;
	e->prev = l->listheader;
	e->next = l->listheader->next;
	l->listheader->next->prev = e;
	l->listheader->next = e;
	++l->count;
}

/******************************************************************************
* list_readdata: return data at current pos and increment pos.
*
* Read the data associated with the current LIST element and
* advance the pointer to the next element in the LIST.
* Returns LIST_ERROR if there are no more items in the LIST
*******************************************************************************/

POINTER 
list_readdata(l)
LIST            l;
{
	POINTER data;

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_readdata", NULL);
#endif

	if (l->current == l->listheader)
		return LIST_ERROR;
	data = l->current->data;
	list_forward(l);
	return data;
}

/******************************************************************************
* list_readdata_back: return data at current and decrement pos.
*
* Read the data associated with the current LIST element and
* move the pointer to the previous element in the LIST.
* Returns LIST_ERROR if there are no more items in the LIST
* 
******************************************************************************/

POINTER 
list_readdata_back(l)
LIST            l;
{
	POINTER data;

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_readdata_back", NULL);
#endif

	if (l->current == l->listheader)
		return LIST_ERROR;
	data = l->current->data;
	list_backward(l);
	return data;
}

/******************************************************************************
* list_getdata: return data at current pos.
*
* Return data of current element without changing position in LIST
******************************************************************************/

POINTER 
list_getdata(l)
LIST            l;
{

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_getdata", NULL);
#endif

	if (l->current == l->listheader)
		return LIST_ERROR;
	return l->current->data;
}

/******************************************************************************
* list_rewind: set current pos to beginning of list.
*
* Set the current element pointer to the first element in the LIST.
******************************************************************************/

void
list_rewind(l)
LIST            l;
{

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_rewind", NULL);
#endif

	l->current = l->listheader->next;
}

/******************************************************************************
* list_fastforward: set current pos to end of list.
*
* Set the current element pointer to the last element in the LIST.
******************************************************************************/

void
list_fastforward(l)
LIST            l;
{

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_fastforward", NULL);
#endif

	l->current = l->listheader->prev;
}

/******************************************************************************
* list_forward: move current pos forward one.
*
* Move the current element pointer forward one element.
******************************************************************************/

void
list_forward(l)
LIST            l;
{

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_forward", NULL);
#endif

	l->current = l->current->next;
}

/******************************************************************************
* list_backward: move current pos back one.
*
* Move the current element pointer backward one element.
******************************************************************************/

void
list_backward(l)
LIST            l;
{

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_backward", NULL);
#endif

	l->current = l->current->prev;
}

/******************************************************************************
* list_search: linear search through list
*
* Linear search though LIST l for key. Compare is a pointer to a function
* that will return non-zero if key matches the data field and zero
* otherwise.  Returns the data of the element that matched.  Returns
* LIST_ERROR if key is not found in the LIST.  The current element pointer
* is left at the matched element.
******************************************************************************/

POINTER 
list_search(l, key, compare)
LIST            l;
POINTER key;
int             (*compare) ();
{
	POINTER d;

	list_rewind(l);
	while ((d = list_readdata(l)) != LIST_ERROR)
	{
		if ((*compare) (key, d))
		{
			list_backward(l);
			return d;
		}
	}
	return LIST_ERROR;
}

/******************************************************************************
* list_numelems: return the number of elements in a list.
******************************************************************************/

int
list_numelems(l)
LIST            l;
{

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_numelems", NULL);
#endif

	return l->count;
}

/******************************************************************************
* list_insert: put a new element into list before current pos.
*
* Insert data before current LIST element.  Doesn't change current pointer.
* If current pointer is off the LIST, this function reverts to
* a list_append.
* 
******************************************************************************/

void
list_insert(l, data)
LIST            l;
POINTER data;
{
	LISTELEM        e;

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_insert", NULL);
#endif
	
	e = (LISTELEM) malloc(sizeof(*e));
	if(e == NULL)
	{
		exc_raise("A:LIST.OUT_OF_MEMORY", "list_insert", NULL);
		/*NOTREACHED*/
	}
	e->next = l->current;
	e->prev = l->current->prev;
	e->data = data;
	l->current->prev->next = e;
	l->current->prev = e;

	++l->count;
}

/******************************************************************************
* list_delete: toast element at current pos.
*
* Delete the current element from the LIST. Shifts everything after deleted
* element forward, leaving current pointer on next element.  If last item is
* deleted, the current pointer is left on end-of-LIST.  Raises an exception
* if the current element pointer is off the end of the LIST.
******************************************************************************/

void
list_delete(l)
LIST            l;
{
	LISTELEM temp;
	
#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_delete", NULL);
#endif

	if (l->current == l->listheader)
	{
		exc_raise("A:LIST.DELETING_END_OF_LIST", "list_delete", NULL);
		/*NOTREACHED*/
	}
	l->current->prev->next = l->current->next;
	l->current->next->prev = l->current->prev;
	temp = l->current;
	l->current = l->current->next;

	free((POINTER )temp);
	--l->count;
}

/******************************************************************************
* list_sortadd: add an element to a list keeping list in sorted order.
*
* Add an element to LIST l at a position indicated by func.
* The function should be a boolean function of the form:
* 	func(e, d)
* 	<type of whatever you've been storing in the LIST> e,d;
* The function should return TRUE if d should come before e in the LIST.
* The current element pointer is undefined.
*******************************************************************************/

void
list_sortadd(l, d, func)
LIST            l;
POINTER d;
int             (*func) ();

{
	POINTER e;

	if (list_numelems(l) == 0)
	{
		list_append(l, d);
		return;
	}
	list_rewind(l);
	while ((e = list_readdata(l)) != LIST_ERROR)
	{
		/* func should return nonzero if d is to be inserted
		 * before e in the LIST
		 */
		if ((*func) (e, d))
		{
			list_backward(l);
			list_insert(l, d);
			return;
		}
	}
	list_append(l, d);
}

/******************************************************************************
* list_getpos: return current element pointer.
*
* Return the current element pointer.  Can be given to list_setpos
* to reset the current pointer.  The validity of the value returned from
* this functions is not guaranteed after deletes are performed.
* 
******************************************************************************/

LISTPOS
list_getpos(l)
LIST            l;
{

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_getpos", NULL);
#endif

	return ((LISTPOS)l->current);
}

/******************************************************************************
* list_setpos: set the current element pointer.
*
* Takes a value returned from list_getpos and resets the current element
* pointer to that value.
* 
******************************************************************************/

void
list_setpos(l, p)
LIST            l;
LISTPOS p;
{

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_setpos", NULL);
#endif

	l->current = (LISTELEM)p;
}

/******************************************************************************
* list_setelem: replace data at current element pointer.
*
* List_setelem updates the data field of the element at the current
* element pointer.  Returns LIST_ERROR if the current element is end-of-LIST.
* Returns the data sent in otherwise.
* 
******************************************************************************/

POINTER 
list_setelem(l, d)
LIST l;
POINTER d;
{
#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_setelem", NULL);
#endif

	if (l->current == l->listheader)
		return LIST_ERROR;
	l->current->data=d;
	return d;
}


/******************************************************************************
* list_unlink: remove an element from a list, returning element to caller. 
*
* Unlink the current element from the LIST. Shifts everything after deleted
* element forward, leaving current pointer on next element.  If last item is
* deleted, the current pointer is left on end-of-LIST.  Returns LIST_ERROR
* if current element pointer is off the end of the LIST and the unlinked
* LISTELEM otherwise.  Note that the LISTELEM may have been obtained through
* the heap, however it is the callers responsibilty to dispose of the
* element in the proper way.  It is expected that the LISTELEM will be the
* argument of a subsequent relink call.
* 
******************************************************************************/

LISTELEM
list_unlink(l)
LIST            l;
{
	LISTELEM temp;
	
#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_unlink", NULL);
#endif

	if (l->current == l->listheader)
		return (LISTELEM)LIST_ERROR;
	l->current->prev->next = l->current->next;
	l->current->next->prev = l->current->prev;
	temp = l->current;
	l->current = l->current->next;

	--l->count;
	return(temp);
}

/******************************************************************************
* list_relink: put an unlinked element back into a list.
*
* Relink LISTELEM before current LIST element.  Doesn't change current pointer.
* If current pointer is off the LIST, this function reverts to
* a list_append.
* 
******************************************************************************/

void
list_relink(l, e)
LIST l;
LISTELEM e;
{

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_relink", NULL);
#endif
	
	e->next = l->current;
	e->prev = l->current->prev;
	l->current->prev->next = e;
	l->current->prev = e;

	++l->count;
}

/******************************************************************************
* list_append_relink: add an unlinked element back to the end of a list.
*
* Relink the given LISTELEM onto the end of LIST l.
* The current element pointer is left unchanged.
******************************************************************************/

void
list_append_relink(l, e)
LIST l;
LISTELEM e;
{

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_append_relink", NULL);
#endif

	e->next = l->listheader;
	e->prev = l->listheader->prev;
	l->listheader->prev->next = e;
	l->listheader->prev = e;
	++l->count;
}

/******************************************************************************
* list_prepend_relink: add an unlinked element back to the beginning of a list.
*
* Prepend the given LISTELEM to the beginning of LIST l.
* The current element pointer is left unchanged.
******************************************************************************/

void
list_prepend_relink(l, e)
LIST l;
LISTELEM e;
{

#ifdef CAREFUL
	if (l == NULL)
		exc_raise("A:LIST.NULL_LIST", "list_prepend_relink", NULL);
#endif

	e->prev = l->listheader;
	e->next = l->listheader->next;
	l->listheader->next->prev = e;
	l->listheader->next = e;
	++l->count;
}

/******************************************************************************
* list_print: same as list_apply, only here for compatibility.
*
* Linear scan though LIST l calling the print function on each data field.
* printfun is a pointer to a function that will print the data field.  The
* current element pointer is left at end-of-list.
* 
******************************************************************************/

void
list_print(l, printfun)
LIST            l;
int             (*printfun)();
{
	POINTER d;

	list_rewind(l);
	while ((d = list_readdata(l)) != LIST_ERROR)
	{
		 (*printfun)(d);
	}
}

/******************************************************************************
* list_apply: apply function to all elements of a list.
*
* Linear scan though LIST l calling the function on each data field.
* fun is a pointer to a function.  The
* current element pointer is left at end-of-list.
* 
******************************************************************************/

void
list_apply(l, fun)
LIST            l;
int             (*fun)();
{
	POINTER d;

	list_rewind(l);
	while ((d = list_readdata(l)) != LIST_ERROR)
	{
		 (*fun)(d);
	}
}
