
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifdef LHOBJ
#include "vcobj.h"
#include "vcfac.h"
#include "vcmemfun.h"
#include "vcmap.h"
#include "vcdbg.h"
#include "vcio.h"

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcobj.cpp 1.45 1997/10/05 17:27:06 dwight Stable $";

#ifdef VCDBG
class VcObjectDbgNode : public VcDebugNode
{
public:
	VcObjectDbgNode(vc_object *);

	enum state {LOCAL, ISA, DELEGATE, FORWARD};
	
	void printOn(VcIO os) ;
	void make_toplev() {toplev = 1;}
	void set_try(const vc& v) {trying = v;}
	void set_state(enum state e) {find_state = e;}

	vc_object *obj;
	vc trying;
	vc sel;
	enum state find_state;
	int toplev;
	
};

VcObjectDbgNode::VcObjectDbgNode(vc_object *v)
{
	obj = v;
	toplev = 0;
	find_state = LOCAL;
}

void
VcObjectDbgNode::printOn(VcIO os)
{
	if(toplev)
	{
		os << "---free-standing ";
	}
	switch(find_state)
	{
	case FORWARD:
		os << "encapsulated ";
		break;
	case DELEGATE:
		os << "integrated ";
		break;
	case ISA:
		os << "base ";
		break;
	case LOCAL:
		break;
	default:
		os << "<<erroneous dbg state>> ";
	}
	os << "object generated from factory \"";
    vc noconst = obj->factory_name;
    noconst.printOn(os);
	os << "\"\n";
	os << "Attempting to ";
	switch(find_state)
	{
	case FORWARD:
		os << "forward ";
		break;
	case DELEGATE:
		os << "delegate ";
		break;
	case ISA:
		os << "find in base objects the ";
		break;
	case LOCAL:
        os << "find ";
        break;
	default:
		os << "<<erroneous dbg state>>";
	}
	os << "selector \"";
	sel.printOn(os);
    if(find_state == FORWARD || find_state == DELEGATE)
	{
		os << "\" to member \"";
		trying.printOn(os);
		os << "\"";
	}
    else
    {
        os << "\"";
    }
	os << "\n";
}
#endif

vc_object::vc_object(const vc& fac_name, VC_FACTORY_MAP *mems) :
    factory_name(fac_name),
    members(mems)
{
	mutant = 0;
}


vc_object::~vc_object()
{
	delete members;
}

// note: infinite loop if recursive base structure is created
// and key can't be found. needs to check the list of
// forwarded objects as well.
//
int
vc_object::find(const vc& key, vc& out)
{
	return member_select(key, out, 1, 0);
}

int
vc_object::obj_bind(const vc& mem, const vc& val, int top)
{
	if(mem.type() != VC_STRING)
	{
		USER_BOMB("object member name must be string", 0);
	}
		
	if(members->contains(mem))
	{
		members->replace(mem, val);
		return 1;
	}

	int found = 0;
	// not local, check in the base class object recursively
	if(!base_obj.is_nil())
		found = base_obj.get_obj()->obj_bind(mem, val, 0);
	if(top && !found)
	{
		// mutant!
		mutant = 1;
		members->add(mem, val);
	}
	return found;
}

vc_object *
vc_object::get_obj() const
{
	return (vc_object *)this;
}

// member selection is used when the syntax foo[bar] is being
// processed. 
int
vc_object::member_select(const vc& mem, vc& mem_out, int toplev, vc_object *topobj)
{
	vc memval;
#ifdef VCDBG
	VcObjectDbgNode dbg(this);
	dbg.sel = mem;
#endif

	if(toplev)
	{
		topobj = this;
#ifdef VCDBG
		dbg.make_toplev();
#endif
	}
	
	if(mem.type() != VC_STRING)
	{
		USER_BOMB("object member name must be string", 0);
	}
		
	if(members->find(mem, memval))
	{
		if(memval.type() == VC_FUNC)
		{
			// create a member function object and return it
			// so proper member function processing can occur.
			vc v;
			vc_memberfun *mf = new vc_memberfun(topobj, memval);
			v.redefine(mf);
			mem_out = v;
			return 1;
		}
#ifdef VCDBG
		if(break_on(BREAK_ACCESS))
		{
			VcIOHackStr *o = new VcIOHackStr();
			*o << "break accessing member \"" << (const char *)mem << "\"";
			*o << '\0';
			char *s = o->str();
			delete o;
			if(drop_to_dbg(s, "breakaccess"))
			{
				delete [] s;
				return 0;
			}
			delete [] s;
		}
#endif
		mem_out = memval;
		return 1;
	}

#ifdef VCDBG
	dbg.set_state(VcObjectDbgNode::ISA);
#endif

	// not local, check in the base class object recursively
	if(!base_obj.is_nil())
	{
		if(base_obj.member_select(mem, mem_out, 0, topobj))
			return 1;
	}

#ifdef LH_OBJ_FORWARDS
	
#ifdef VCDBG
	dbg.set_state(VcObjectDbgNode::DELEGATE);
#endif

	// not in this object, see if it is delegated.
	VC_FORWARD_LIST_ITER i(&delegates);
	for(; !i.eol(); i.forward())
	{
		vc v = i.get();
#ifdef VCDBG
		dbg.set_try(v);
#endif
		if(!members->find(v, memval))
		{
			USER_BOMB("can't find forwarded member", 0);
			/*NOTREACHED*/
		}
		if(memval.type() != VC_OBJECT)
		{
			USER_BOMB("can't delegate to non-object (yet.)", 0);
			/*NOTREACHED*/
		}
		// note: delegating uses current context for evaluation, giving
		// a sort of multiple-inheritance flavor...
		if(memval.member_select(mem, mem_out, 0, topobj))
			return 1;
	}
	
#ifdef VCDBG
	dbg.set_state(VcObjectDbgNode::FORWARD);
#endif
	// not in this object, see if it is forwarded.
	VC_FORWARD_LIST_ITER i2(&forwards);
	for(; !i2.eol(); i2.forward())
	{
		vc v = i2.get();
#ifdef VCDBG
		dbg.set_try(v);
#endif
		if(!members->find(v, memval))
		{
			USER_BOMB("can't find forwarded member", 0);
			/*NOTREACHED*/
		}
		if(memval.type() != VC_OBJECT)
		{
			USER_BOMB("can't forward to non-object (yet.)", 0);
			/*NOTREACHED*/
		}
		// note: forwarding starts a new distinct context for object contexts
		if(memval.member_select(mem, mem_out, 1, 0))
			return 1;
	}
#endif

	return 0;
}

void
vc_object::printOn(VcIO os)
{
	VC_FACTORY_MAP_ITER i(members);
	os << "object(factory: ";
    vc noconst = factory_name;
    noconst.printOn(os);
	if(mutant)
		os << " (mutant)";
	os << "\n";
	for(;!i.eol(); i.forward())
	{
		VC_FACTORY_MAP_ASSOC a = i.get();
		vc memname = a.get_key();
		// punt on this (avoid recursion) and leave
		// base to last.
		if(memname == vc_factory_def::Thisname ||
			memname == vc_factory_def::Basename)
			continue;

		// this is temporary, since printing is used most often for
		// debugging, having all the member functions printed out
		// is a pain
		if(a.get_value().type() != VC_FUNC)
		{
			memname.printOn(os);
			os << ": " ;
			a.get_value().printOn(os);
			os << "\n";
		}
	}
	os << "base: ";
	base_obj.printOn(os);
	os << ")";
}
#endif
