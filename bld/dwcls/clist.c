
/*###TLIBID### "%n %v %f Last checkin: %w Locked by: %l"*/
static char TLIBID[]="LISTS.C 4 9-Jan-92,15:06:24 Last checkin: DWIGHT Locked by: ***_NOBODY_***";

/*$Log: clist.c $
 * Revision 1.8  1994/10/23  09:36:45  dwight
 * nc
 * 
 * Revision 1.7  1994/09/24  07:54:42  dwight
 * Checkpoint
 * 
 * Revision 1.6  1994/09/23  15:20:44  dwight
 * Checkpoint
 * 
 * Revision 1.5  1994/09/21  14:56:13  dwight
 * Checkpoint
 * 
 * Revision 1.4  1994/09/21  02:35:12  dwight
 * Checkpoint
 * 
 * Revision 1.3  1994/09/20  16:15:29  dwight
 * Checkpoint
 * 
 * Revision 1.2  1994/09/17  03:45:00  dwight
 * compiles with cfront
 * 
 * Revision 1.1  1994/07/17  03:09:54  dwight
 * Initial revision
 * 
 * Revision 1.1  1992/12/06  18:11:48  dwight
 * Initial revision
 *
 * Revision 1.3  88/01/28  13:38:21  dwight
 * checkpoint
 * 
 * Revision 1.2  87/03/19  12:40:41  dwight
 * fixed, reformatted
 * 
 * Revision 1.1  87/02/27  08:38:08  dwight
 * Initial revision
 * 
 * Revision 1.2  85/10/09  20:41:08  dwight
 * added list_dealloc for getting rid of structures malloc'ed by
 * list routines.
 * 
 * Revision 1.1  85/06/15  10:26:31  dwight
 * Initial revision
 * */
static char rcsid[] = "$Header: g:/dwight/repo/dwcls/rcs/clist.c 1.8 1994/10/23 09:36:45 dwight Stable095 $";

/* general linked list routines */
#include <alloc.h>
#include <stdlib.h>
#include "c_lists.h"

/******************************************************************************
* Allocate a new list and return an opaque handle to the caller.
******************************************************************************/

CList
list_alloc(void)

{
	CList            l;

	if ((l = (CList) malloc(sizeof(*l))) == NULL)
		return NULL;
		
	l->count = 0;
	l->listheader = (ListElem) malloc(sizeof(*l->listheader));
	l->listheader->clnt_data = NULL;
	l->listheader->prev = l->listheader->next = l->listheader;
	l->current = l->listheader;

	return l;
}

/******************************************************************************
* Deallocate a list - only deallocates the structures built by other
* routines in this package.  The clnt_data field is untouched.
* 
******************************************************************************/

void
list_dealloc(
CList            l
)
 
{
	ListElem        elem,
	                elem2;

	if (l == NULL)
		abort();
	list_rewind(l);
	elem = l->listheader->next;
	while (l->count-- > 0)
	{
		if (elem == NULL)
			abort();
		elem2 = elem->next;
		free((char *)elem);
		elem = elem2;
	}
	free((char *)(l->listheader));
	free((char *)l);
}

/******************************************************************************
* Append the given clnt_data to the end of list l.
* The current element pointer is left unchanged.
******************************************************************************/

void
list_append(
CList            l,
void           *clnt_data
)
 
{
	ListElem        e;

	if (l == NULL)
		abort();

	e = (ListElem) malloc(sizeof(*e));
	e->clnt_data = clnt_data;
	e->next = l->listheader;
	e->prev = l->listheader->prev;
	l->listheader->prev->next = e;
	l->listheader->prev = e;
	++l->count;
}

/******************************************************************************
* Get the clnt_data associated with the current list element and
* advance the pointer to the next element in the list.
* Returns LIST_ERROR if there are no more items in the list
*******************************************************************************/

void           *
list_getelem(
CList            l
)
 
{
	void           *clnt_data;

	if (l == NULL)
		abort();
	if (l->current == l->listheader)
		return LIST_ERROR;
	clnt_data = l->current->clnt_data;
	list_forward(l);
	return clnt_data;
}

/******************************************************************************
* Get the clnt_data associated with the current list element and
* move the pointer to the previous element in the list.
* Returns LIST_ERROR if there are no more items in the list
* 
******************************************************************************/

void           *
list_getelem_back(
CList            l
)
 
{
	void           *clnt_data;

	if (l == NULL)
		abort();
	if (l->current == l->listheader)
		return LIST_ERROR;
	clnt_data = l->current->clnt_data;
	list_backward(l);
	return clnt_data;
}

/******************************************************************************
* Return clnt_data of current element without changing position in list
******************************************************************************/

void           *
list_readelem(
CList            l
)
 
{
	if (l == NULL)
		abort();
	if (l->current == l->listheader)
		return LIST_ERROR;
	return l->current->clnt_data;
}

/******************************************************************************
* Set the current element pointer to the first element in the list.
******************************************************************************/
void
list_rewind(
CList            l
)
 
{
	if (l == NULL)
		abort();
	l->current = l->listheader->next;
}

/******************************************************************************
* Set the current element pointer to the last element in the list.
******************************************************************************/

void
list_fastforward(
CList            l
)
 
{
	if (l == NULL)
		abort();
	l->current = l->listheader->prev;
}

/******************************************************************************
* Move the current element pointer forward one element.
******************************************************************************/

void
list_forward(
CList            l
)
 
{
	if (l == NULL)
		abort();
	l->current = l->current->next;
}

/******************************************************************************
* Move the current element pointer backward one element.
******************************************************************************/

void
list_backward(
CList            l
)
 
{
	if (l == NULL)
		abort();
	l->current = l->current->prev;
}

/******************************************************************************
* Linear search though list l for key. Compare is a pointer to a function
* that will return non-zero if key matches the clnt_data field and zero
* otherwise.  Returns the clnt_data of the element that matched.  Returns
* LIST_ERROR if key is not found in the list.  The current element pointer
* is left at the matched element.
******************************************************************************/

void           *
list_search(
CList            l,
void           *key,
LIST_COMP_FUN_P compare
)
 
{
	void           *d;

	list_rewind(l);
	while ((d = list_getelem(l)) != LIST_ERROR)
	{
		if ((*compare) (key, d))
		{
			list_backward(l);
			return d;
		}
	}
	return LIST_ERROR;
}

int
list_numelems(
CList            l
)
 
{
	if (l == NULL)
		abort();
	return l->count;
}

/******************************************************************************
* Insert clnt_data before current list element.  Doesn't change current pointer.
* If current pointer is off the list, this function reverts to
* a list_append.
* 
******************************************************************************/

void
list_insert(
CList            l,
void           *clnt_data
)
 
{
	ListElem        e;

	if (l == NULL)
		abort();
	
	e = (ListElem) malloc(sizeof(*e));
	e->next = l->current;
	e->prev = l->current->prev;
	e->clnt_data = clnt_data;
	l->current->prev->next = e;
	l->current->prev = e;
	++l->count;
}

/******************************************************************************
* Delete the current element from the list. Shifts everything after deleted
* element forward, leaving current pointer on next element.  If last item is
* deleted, the current pointer is left on end-of-list.  Returns FALSE if
* current element pointer is off the end of the list and TRUE otherwise.
* 
******************************************************************************/

bool_t
list_delete(
CList            l
)
 
{
	ListElem temp;
	
	if (l == NULL)
		abort();
	if (l->current == l->listheader)
		return FALSE;
	l->current->prev->next = l->current->next;
	l->current->next->prev = l->current->prev;
	temp = l->current;
	l->current = l->current->next;

	free((void *)temp);
	--l->count;
	return(TRUE);
}

/******************************************************************************
* Add an element to list l at a position indicated by func.
* The function should be a boolean function of the form:
* 	func(e, d)
* 	<type of whatever you've been storing in the list> e,d;
* The function should return TRUE if d should come before e in the list.
* The current element pointer is undefined.
*******************************************************************************/

void
list_sortadd(
CList            l,
void           *d,
LIST_COMP_FUN_P func
)
 
{
	void *e;

	if (list_numelems(l) == 0)
	{
		list_append(l, d);
		return;
	}
	list_rewind(l);
	while ((e = list_getelem(l)) != LIST_ERROR)
	{
		/* func should return nonzero if d is to be inserted
		 * before e in the list
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
* Return the current element pointer.  Can be given to list_setpos
* to reset the current pointer.  The validity of the value returned from
* this functions is not guaranteed after deletes are performed.
* 
******************************************************************************/

void *
list_getpos(
CList            l
)
 
{
	if (l == NULL)
		abort();
	return ((void *)l->current);
}

/******************************************************************************
* Takes a value returned from list_getpos and resets the current element
* pointer to that value.
* 
******************************************************************************/

void
list_setpos(
CList l,
void *p
)
 
{
	if (l == NULL)
		abort();

	l->current = (ListElem)p;
}

/******************************************************************************
* List_setelem updates the clnt_data field of the element at the current
* element pointer.  Returns LIST_ERROR if the current element is end-of-list.
* Returns the clnt_data sent in otherwise.
* 
******************************************************************************/

void *
list_setelem(
CList l,
void *d
)
 
{
	if (l == NULL)
		abort();
	if (l->current == l->listheader)
		return LIST_ERROR;
	l->current->clnt_data=d;
	return d;
}
