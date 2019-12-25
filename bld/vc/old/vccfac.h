
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCCFAC_H
#define VCCFAC_H

// this class encapsulates c++ objects as
// lh objects. in other words, you can do:
// lbind(a CPPObj())
// and a will refer to an object create from
// a cpp class.
// then if it is
// class CPPObj : public CPPBase {
// int foo(char *, int);
// };
// for example, in LH you can do:
// a[foo](a1 a2)
// and get a call to the CPPObj
// member func "foo" with the right args.
//
// if CPPBase has funcions, then calling
// in LH a[basefun](a1) will get the right
// functions automatically.
//
// hmmmm, on further thought, i think trying 
// to merge the two object systems to be
// a bad idea, since they are too different.
// i think it would be better to use a
// function-based approach to encapsulating
// c++ objects, and then if needed, in LH
// encapsulate those functions.
// for example:
// lbind(a create-obj(CPPType ctora1 ctora2 ...))
// memcall(<a> memfun-name a1 a2 ...)
// memget(<a> mem-name )
// memset(<a> mem-name v1)
// etc.
//
/*
internally, the encapsulation would look like this:

lh_create_obj(VCArglist *a)
{
	typename = a[0];

	find typename in table, gives pointer
	to function that looks like this:

	vc typename_foo(VCArglist *a)
	{
	
		foo *f = new foo(
		(type_from_objdump1)a[0],
		(type_from_objdump2)a[1], etc.
		);
		// this function has to be generated
		//fully.
		// store typeid with it, so we can
		// can't it back later
		return vc(VC_INT_DTOR_TYPE, f, typeid(f));
	}

	
}

lh_call_memfun(VCArglist *a)
{
	ptr, type pair is a[0]

	type_info& i = a[0]'s typeinfo
	typename = i.name();

	find typename in table, gives map of
	members.
}

// in order to call this, we would need to have
// a table that mapped strings type_memname to
// a pointer to this function.
// arguments would be handled as in regular calls.
// overloading is a problem (have to think about it.)
// if, during building the table for typename, we
// "bring down" all the members of its base, and
// map them typename_memname to function pointer for
// lh_wrap_memcall_basename_memname, then inheritance
// is taken care of.
lh_wrap_memcall_<typename>_<memname>(VCArglist *a)
{
	o = (typename *)(void *)a[0];
	o-><memname>(
		cvt a[0]
		cvt a[1]
		...
	);
}


*/

#endif
