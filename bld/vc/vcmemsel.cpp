
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "vcmap.h"
#include "vcmemsel.h"
#include "vcio.h"
#include "dwstr.h"
#ifndef NO_VCEVAL

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcmemsel.cpp 1.46 1997/11/27 03:06:30 dwight Stable $";

#ifdef VCDBG
class VcMemSelectDbgNode : public VcDebugNode
{
public:
    VcMemSelectDbgNode(const vc_memselect *);

    void printOn(VcIO os);
    void printOnBrief(VcIO os);
    int has_brief();

    const char *state;
};

VcMemSelectDbgNode::VcMemSelectDbgNode(const vc_memselect *m)
{
    linenum = m->start.linenum;
    filename = m->start.filename;
    state = "<<error>>";
}

void
VcMemSelectDbgNode::printOn(VcIO os)
{
    os << "---object member selection near line " << filename.c_str() << ":" <<
        linenum << " (" << state << ")";
    os << "\n";
}

// this outta be put into the base class...
int
VcMemSelectDbgNode::has_brief()
{
    int r = filename.rfind("/");
    if(r == DwString::npos)
        r = filename.rfind("\\");
	vc file;
    if(r != DwString::npos)
        file = vc(filename.c_str() + r + 1);
	else
        file = vc(filename.c_str());
	
	static vc debug("lhdbg.lh");
	static vc userinp("__lh_user_input");
	if(file == debug || file == userinp)
		return 0;
	return 1;
}

void
VcMemSelectDbgNode::printOnBrief(VcIO os)
{
    os << filename.c_str() << ":" << linenum << ", ";
    os << "memsel (" << state << ")";
    os << "\n";
}
#endif

vc_memselect::vc_memselect(const vc& expr, const vc& selectr,
                           const vc_cvar_src_coord& s, const vc_cvar_src_coord &e)
	: obj_expr(expr), selector(selectr), start(s), end(e)
{

}

vc_memselect::~vc_memselect()
{
}

vc
vc_memselect::eval() const
{

	// note: this hack has the same flavor as the funcall
	// hack of the same name. since it is a convenience to allow
	// foo[bar] to mean <foo>[bar] we'll allow that here.
	// there is one slight difference with the funcall hack.
	// since there is a meaning for foo[bar], namely, get the
	// "bar" meta-data for foo, we try the meta-data lookup if
	// the first lookup fails. this essentially makes all user-defined
	// members hide meta-data lookup members. this might change...
	
#ifdef VCDBG
    VcMemSelectDbgNode dbg(this);
    dbg.state = "finding object";
#endif
    
	vc obj;
	if(obj_expr.type() == VC_STRING)
	{
		obj = Vcmap->get(obj_expr);
        if(obj.is_nil())
        {
            DwString err("can't find object \"%1\"");
            err.arg(obj_expr.peek_str());
            USER_BOMB(err.c_str(), vcnil);
        }
	}
	else
	{
		obj = obj_expr.eval();
		CHECK_ANY_BO(vcnil);
	}


#ifdef VCDBG
    if(obj.break_on(BREAK_ACCESS))
    {
        // break on access to the object before the
        // access really goes. this gives you a chance
        // to look into things before the member selection
        // is invoked
        if(drop_to_dbg("break accessing object", "breakobjectaccess"))
        {
            return vcnil;
        }
    }
    dbg.state = "finding selector";
#endif
	
	vc sel = selector.eval();
	CHECK_ANY_BO(vcnil);
	vc out;
	if(obj.member_select(sel, out, 1, 0))
		return out;
	// maybe try the meta-data call here...
    DwString err("can't find member \"%1\" in object");
    err.arg((const char *)sel);
    USER_BOMB(err.c_str(), vcnil);
	return vcnil;
}

void
vc_memselect::stringrep(VcIO os) const
{
	obj_expr.stringrep(os);
	os << '[';
	selector.stringrep(os);
	os << ']';
}

void
vc_memselect::printOn(VcIO os)
{
	obj_expr.printOn(os);
	os << "[";
	selector.printOn(os);
	os << "]";
}
#endif
