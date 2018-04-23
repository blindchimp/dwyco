
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifndef NO_VCEVAL
#include "vcmap.h"
#include "vccvar.h"
#include "vcstr.h"
#include "vcint.h"
#include "vcio.h"
#include "vcfuncal.h"
#include "vcfundef.h"
#include "vccfun.h"
#include "dwstr.h"
#include "vcdbl.h"
#include "vclh.h"
#include "vcnil.h"
#include "vcmemsel.h"

// note: this translation obscures the relationships
// developed in the parse tree, so things like stringrep will not work
// without some more work. if you needed stringrep in the translated unit, one
// way to do that might be to just generate the stringrep for the entire
// unit that is being translated, and put that into the resulting
// translation. then, when the unit is executed (or maybe later when
// a stringrep is requested) send the stringrep to the parser/lexer to
// regenerate the parse tree. alternately, one might be able to define
// a xfer-rep for cvars (not implemented now), which, when xferr-ed in,
// would result in a parse tree.
// another method might be to generate a table or something with a representation
// of the parse tree (in c) in it. but that sounds like another representation to
// deal with, for little gain (stringrep isn't used all that much.)
//
// note2: everywhere in this translation where you see things like
// "foo(%1(), %2())" are a problem because the ordering of the
// evaluation of args in C is not defined. if we want to provide
// ordered evaluation in LH everywhere, we need to put the args
// into tmps and then do the call. not sure if i might want to
// just go ahead and make unordered evaluation the default, with
// order defined by particular ops, or not...

static vc
gensym()
{
    static unsigned long i;
    char buf[128];

    ++i;
    sprintf(buf, "gensym%lu", i);
    return vc(buf);
}

DwString
buf_to_c_string(const char *buf, int len)
{
    // return something that will get buffer past c lexer intact
    DwString ret((const char *)vclh_to_hex(vc(VC_BSTRING, buf, len)));
    for(int i = 0; i < len; ++i)
    {
        ret.insert(i * 4, "\\x");
    }
    ret.insert(0, "\"");
    ret += "\"";
    return ret;
}

template<class T>
DwString
num_to_str(T n)
{
    VcIOHackStr a;
    a << n;
    return DwString(a.ref_str(), 0, a.pcount());
}

DwString
buf_to_c_vc(const char *buf, int len)
{
    DwString a = buf_to_c_string(buf, len);
    DwString tmpl("vc(VC_BSTRING, %1, %2)");
    tmpl.arg(a, num_to_str(len));
    return tmpl;
}

DwString
vc_to_c_vc(vc v)
{
    if(v.type() != VC_STRING)
        oopanic("not impl");
    return buf_to_c_vc((const char *)v, v.len());
}

// the simplest thing is just to provide a function to return
// a copy of the string. but this needs to be fast, so we
// hack up some stuff to make the string static and faux-eval
// it. might also want to just emit something that is a vc but
// has the ref counting disabled, since we'll never delete it here

vc
vc_string::translate(VcIO o) const
{
    vc sym = gensym();

    DwString tmpl(
                "// %2 \n"
                //"static vc slow_%sym() {return %1;}\n"
                "static const vc s_%sym = %1;\n"
                "#define %sym() s_%sym\n"
                );
    tmpl.srep("%sym", (const char *)sym, 1);
    tmpl.arg(vc_to_c_vc(str), str);
    o << tmpl.c_str();
    return sym;
}

vc
vc_nil::translate(VcIO o) const
{
    vc sym = gensym();
    DwString tmpl("#define %1() vcnil\n");
    tmpl.arg((const char *)sym);
    o << tmpl.c_str();
    return sym;
}

vc
vc_int::translate(VcIO o) const
{
    vc sym = gensym();
    //o << "static vc " << sym << "() { return " << i << "; }\n";
    //o << "static vc slow_" << sym << "() { return " << i << ";}\n";
    o << "static const vc s_" << sym << "(" << i << ");\n";
    o << "#define " << sym << "() s_" << sym << "\n";
    return sym;
}

vc
vc_double::translate(VcIO o) const
{
    vc sym = gensym();
    //o << "static vc " << sym << "() { return " << d << "; }\n";
    //o << "static vc slow_" << sym << "() { return " << d << ";}\n";
    o << "static const vc s_" << sym << "(" << d << ");\n";
    o << "#define " << sym << "() s_" << sym << "\n";
    return sym;
}

vc
vc_cvar::translate(VcIO o) const
{
    if(exp_error)
        USER_BOMB("attempt to eval erroneous expression", vcnil);


    VCListIter i(&vc_list);
    vc atom;
    vc vars(VC_VECTOR);
    int expr_num = 0;
    // here is where we match a really common case: <str>
    // and emit better code for it.
    if(!quoted && !dont_map && vc_list.num_elems() == 1)
    {
        vc val = vc_list.get_first();
        if(val.type() == VC_STRING)
        {
            DwString tmpl("static vc %2() {static vc var = %1;\nreturn Vcmap->get(var);\n}\n");
            vc ourname = gensym();
            tmpl.arg(vc_to_c_vc(val), (const char *)ourname);
            o << tmpl.c_str();
            return ourname;
        }
    }
    dwlista_foreach_iter(i, atom, vc_list)
    {
        vc val;

        val = atom.translate(o);
        // what we get back is a "function name" that if we
        // emit a call to that function, the result will be
        // an evaluation of the function.

        vars[expr_num++] = val;
    }
    // we now have all the function names used to implement
    // the component vars, now emit our implementation
    vc ourname = gensym();


    if(quoted)
    {
        // NOTE: have to think about how to translate "last
        // expr in list is value of list" on evaluation.
		// NOTE: for quoted items, we emit two things:
		// a "here is how you refer to me" thing, and a
		// "here is how you run me" thing, since the whole
		// point of quoting is to defer execution.
        o << "static vc real_" << ourname << "() {";
        if(vars.num_elems() == 0)
        {
            o << "return vcnil;\n}";
        }
        else
        {
            for(int i = 0; i < vars.num_elems(); ++i)
            {
                if(i == vars.num_elems() - 1)
                    o << "vc ret = ";
                o << vars[i] << "();\n";
            }
            o << "return ret;}";
        }
		o << "static vc " << ourname << "() { \n";
		o << " return (long)" << "real_" << ourname << ";}\n";
        return ourname;
    }
    o << "static vc " << ourname << "() { \n";
    o << "DwString val;\n";
    for(int i = 0; i < vars.num_elems(); ++i)
    {
        o << "val += " << vars[i] << "().peek_str();\n";
    }
    if(!dont_map)
    {
        //special case: the var name "" (zero length name) always maps
        // to nil.
        o << "if(val.length() == 0) return vcnil;\n";
        o << "return Vcmap->get(val.c_str());\n";
    }
    else
    {
        o << "return val.c_str();\n";
    }
    o << "}\n";
    return ourname;
}

vc
vc_memselect::translate(VcIO o) const
{
    vc ourname = gensym();
    DwString sourname = (const char *)ourname;
    DwString tmpl;
    tmpl += "static vc %1() {\n";
    tmpl.arg(sourname);
    if(obj_expr.type() == VC_STRING)
    {
        tmpl += "static vc var = %1;\nvc obj = Vcmap->get(var);\n";
        tmpl.arg(vc_to_c_vc(obj_expr));
    }
    else
    {
        tmpl += "vc obj = %1();\n";
        vc oname = obj_expr.translate(o);
        tmpl.arg((const char *)oname);
    }

    vc sname = selector.translate(o);
    tmpl += "vc sel = %1();\nvc out;\nif(obj.member_select(sel, out, 1, 0)) return out;\n";
    tmpl += "throw VcErr(\"can't find member in object\", sel);\n}\n";
    tmpl.arg((const char *)sname);
    o << tmpl.c_str();
    return ourname;
}

vc
vc_funcall::translate(VcIO o) const
{
    VCArglist enames;
    for(int i = 0; i < arglist.num_elems(); ++i)
    {
        enames[i] = arglist[i].translate(o);
    }
    vc fname = func.translate(o);
    vc ourname = gensym();

    VcIOHackStr our_impl;
    //our_impl << "vc " << ourname << "() { \n";

    if(func.type() == VC_STRING)
    {
        vc f = Vcmap->get(func);
        vc_cfunc *fd = dynamic_cast<vc_cfunc *>(f.nonono());
        // kluge: for now, functions that start with "wrap_" are forced
        // thru indirect calls
        DwString fn((const char *)func, 0, func.len());
        int force_ind = 0;
        if(fn.length() >= 5)
        {
            fn.erase(5);
            if(fn == DwString("wrap_"))
                force_ind = 1;
        }
        if(fd && !force_ind)
        {
            fd->emitlink(&enames, ourname, our_impl);
        }
        else
        {
            // either it isn't bound to a function, or it is a
            // different type of function (like a user defined function)
            our_impl << "static vc " << ourname << "() { \n";
            our_impl << "vc ret;";
            our_impl << "VCArglist a;\n";
            for(int i = 0; i < arglist.num_elems(); ++i)
            {
                our_impl << "a.append(" << enames[i] << "());\n";
            }
            our_impl << "vc fobj = " << fname << "();\n";
            our_impl << "ret = fobj(&a);\n";

        }

    }
    else
    {
        our_impl << "static vc " << ourname << "() { \n";
        our_impl << "vc ret;";
        our_impl << "VCArglist a;\n";
        for(int i = 0; i < arglist.num_elems(); ++i)
        {
            our_impl << "a.append(" << enames[i] << "());\n";
        }
        our_impl << "vc fobj = " << fname << "();\n";
        our_impl << "ret = fobj(&a);\n";


    }
    our_impl << "return ret;\n}\n";

    o.append(our_impl.ref_str(), our_impl.pcount());

    return ourname;
}

vc
vc_cfunc::emitlink(VCArglist *a, vc ourname, VcIO o) const
{
    if(transfunc == 0)
    {
        // generate a translation that just calls straight into the normal
        // runtime used in the interpreter
        // note: this argument setup is a convenience thing that really
        // needs to be changed into one form (ie, the varadic style.)
        // the problem is, if we use separate arguments to c functions, we have a lot
        // less control over what happens to those arguments in terms of
        // what ctors/dtors get called and when, and even what order the args
        // are evaluated in (even the interpreter suffers from this currently).
        // this has has some performance implications, as well as just various
        // compiler dependencies and whatnot.
        // downside of using the varadic style is, if we want to use the runtime functions
        // directly in another application for some reason, we are stuck
        // writing the goofy vcarglist setup. thing is, i don't think i have
        // done that too much, maybe just with the crypto stuff.
        o << "static vc " << ourname << "() { \n";
        if(varadic)
        {
            o << "VCArglist a;\n";
            for(int i = 0; i < a->num_elems(); ++i)
            {
                o << "a.append(" << (*a)[i] << "());\n";
            }
            o << "vc ret = " << impl_name << "(&a);";
            return vcnil;
        }
        else
        {
            DwString args;
            for(int i = 0; i < nargs; ++i)
            {
                args += DwString("%1()").arg((const char *)(*a)[i]);
                if(i != nargs - 1)
                    args += ",";
            }
            DwString tmpl("vc ret = %1(%2);");
            tmpl.arg(impl_name, args);
            o << tmpl.c_str();

        }
        return vcnil;

    }

    // call the function that will generate code for the function/construct
    VcIOHackStr o2;
    vc v = (*transfunc)(a, o2);
    // kluge, need a better way of doing this
    // some translations require us to emit a couple of
    // functions one after another, and since c doesn't really allow
    // nested function defs, we have to think of a better way of
    // emitting this.
    if(v.is_nil())
    {
        o << "static vc " << ourname << "() { \n";
        o.append(o2.ref_str(), o2.pcount());
        return vcnil;
    }
    o << v[0];
    DwString b = (const char *)v[1];
    b.arg((const char *)ourname);
    o << b.c_str();
    return vcnil;

}

vc
trans_domul(VCArglist *a, VcIO o)
{
    DwString tmpl("vc ret = %1() * %2();");
    tmpl.arg((const char *)(*a)[0], (const char *)(*a)[1]);
    o << tmpl.c_str();
    return vcnil;
}

vc
trans_doadd(VCArglist *a, VcIO o)
{
    DwString tmpl("vc ret = %1() + %2();");
    tmpl.arg((const char *)(*a)[0], (const char *)(*a)[1]);
    o << tmpl.c_str();
    return vcnil;
}

vc
trans_dosub(VCArglist *a, VcIO o)
{
    DwString tmpl("vc ret = %1() - %2();");
    tmpl.arg((const char *)(*a)[0], (const char *)(*a)[1]);
    o << tmpl.c_str();
    return vcnil;
}

vc
trans_dodiv(VCArglist *a, VcIO o)
{
    DwString tmpl("vc ret = %1() / %2();");
    tmpl.arg((const char *)(*a)[0], (const char *)(*a)[1]);
    o << tmpl.c_str();
    return vcnil;
}

vc
trans_lbindfun(VCArglist *a, VcIO o)
{
    DwString tmpl("vc ret; Vcmap->local_add(%1(), %2());");
    tmpl.arg((const char *)(*a)[0], (const char *)(*a)[1]);
    o << tmpl.c_str();
    return vcnil;
}

vc
trans_gbindfun(VCArglist *a, VcIO o)
{
    DwString tmpl("vc ret; Vcmap->global_add(%1(), %2());");
    tmpl.arg((const char *)(*a)[0], (const char *)(*a)[1]);
    o << tmpl.c_str();
    return vcnil;
}

vc
trans_doprog(VCArglist *a, VcIO o)
{
    o<< "vc ret;\n";
    for(int i = 0; i < a->num_elems(); ++i)
    {
        o << (*a)[i] << "();\n";
    }

    return vcnil;
}

vc
trans_doloop(VCArglist *a, VcIO o)
{
    DwString tmpl("vc ret;long l = %1();\nlong h = %2();\nvc var = %3();\n");
    tmpl.arg((const char *)(*a)[1], (const char *)(*a)[2], (const char *)(*a)[0]);

    DwString tmpl2("try { long i = l; for(; i <= h; ++i) { Vcmap->local_add(var, vc(i)); %1(); } }\n"
                   "catch(VcBreak& b) {--b.lev; if(b.lev > 0) throw; } return vcnil;\n");
    tmpl2.arg((const char *)(*a)[3]);

    o << tmpl.c_str() << tmpl2.c_str();
    return vcnil;

}

// note: obscure bug in interpreter too: the set expr is evaled before the var expr,
// which is out of order according to spec's left to right ordering.
vc
trans_doforeach(VCArglist *a, VcIO o)
{
    vc cbname = gensym();
    DwString fecb(
                "static void %1(vc var, vc item) {\n"
                "Vcmap->local_add(var, item);\n"
                "%2();\n"
                "}\n"
                );
    fecb.arg((const char *)cbname, (const char *)(*a)[2]);

    DwString tmpl(
                "vc ret;\n"
                "vc var = %1();\n"
                "vc set = %2();\n"
                "try {\n"
                "set.foreach(var, %3);\n"
                "} catch (VcBreak& b) \n"
                "{--b.lev; if(b.lev > 0) throw; }\n"
                );
    tmpl.arg((const char *)(*a)[0], (const char *)(*a)[1], (const char *)cbname);
    //o << fecb.c_str() << tmpl.c_str();
    tmpl.insert(0, "static vc %1(){\n");
    vc v(VC_VECTOR);
    v[0] = fecb.c_str();
    v[1] = tmpl.c_str();
    return v;
}

vc
trans_doif(VCArglist *a, VcIO o)
{
    // 1..3 exprs, it is a construct, so we can emit it right into the containing
    // context
    o << "vc ret;";
    o << "if(!";
    o << (*a)[0] << "().is_nil()) {\n";
    o << "ret = " << (*a)[1] << "();}\n";
    if(a->num_elems() == 3)
    {
        o << "else {\n";
        o << "ret = " << (*a)[2] << "();}\n";
    }

    return vcnil;


}

// implementation of user defined functions
// when a function context is created, the target
// code looks like this:
// try { ... rest of function ... } catch (Ret ret) { }
//
// exception handler:
// try {... stuff ...} catch (Exc exc) { }
// loop construct:
// try { ... stuff ... } catch (Break brk) { }

vc
trans_dowhile(VCArglist *a, VcIO o)
{
    o << "vc ret;";
    o << "try {\n";
    o << "vc cond = " << (*a)[0] << "();\n";
    o << "while(!cond.is_nil()) {\n";
    o << (*a)[1] << "();\n";
    o << "cond = " << (*a)[0] << "();\n";
    o << "}\n";
    o << "}\n";
    o << "catch(VcBreak& b) {\n";
    o << "--b.lev;\n";
    o << "if(b.lev > 0) { throw; }\n";
    o << "}\n";
    o << "return vcnil;";
    return vcnil;

}

vc
trans_dobreak(VCArglist *a, VcIO o)
{
    o << "vc ret;";
    o << "vc lev = " << (*a)[0] << "();\n";
    o << "throw VcBreak((int)lev);\n";
    o << "return vcnil;";
    return vcnil;
}

vc
trans_doreturn(VCArglist *a, VcIO o)
{
    o << "vc ret;";
    o << "ret = " << (*a)[0] << "();\n";
    o << "throw VcRet(ret);\n";
    o << "return vcnil;";
    return vcnil;
}

vc
trans_docand(VCArglist *a, VcIO o)
{
    DwString tmpl("vc ret;\n");
    for(int i = 0; i < a->num_elems(); ++i)
    {
        DwString as("if(%1().is_nil()) return vcnil;");
        as.arg((const char *)(*a)[i]);
        tmpl += as;
    }
    tmpl += "return vctrue;\n";
    o << tmpl.c_str();
    return vcnil;
}

vc
trans_docor(VCArglist *a, VcIO o)
{
    DwString tmpl("vc ret;\n");
    for(int i = 0; i < a->num_elems(); ++i)
    {
        DwString as("if(!%1().is_nil()) return vctrue;");
        as.arg((const char *)(*a)[i]);
        tmpl += as;
    }
    tmpl += "return vcnil;\n";
    o << tmpl.c_str();
    return vcnil;
}

vc
trans_doswitch(VCArglist *a, VcIO o)
{
    DwString tmpl("vc ret;");
    tmpl += "vc match = %1();\n";
    tmpl.arg((const char *)(*a)[0]);
    int i = 1;
    for(; i < a->num_elems() - 1; i += 2)
    {
        DwString as("if(match == %1()) {\nret = %2(); return ret;}\n");
        as.arg((const char *)(*a)[i], (const char *)(*a)[i + 1]);
        tmpl += as;
    }
    DwString as("ret = %1();\n");
    as.arg((const char *)(*a)[i]);
    tmpl += as;
    o << tmpl.c_str();
    return vcnil;

}

vc
trans_docond(VCArglist *a, VcIO o)
{
    int has_default = 0;
    int nargs = a->num_elems();
    if(nargs % 2)
    {
        has_default = 1;
        --nargs;
    }
    DwString tmpl("vc ret;\n");
    for(int i = 0; i < nargs; i += 2)
    {
        DwString as("if(!%1().is_nil()) {\nret = %2(); return ret;}\n");
        as.arg((const char *)(*a)[i], (const char *)(*a)[i + 1]);
        tmpl += as;
    }
    if(has_default)
    {
        DwString as("ret = %1();");
        as.arg((const char *)(*a)[nargs]);
        tmpl += as;
    }
    o << tmpl.c_str();
    return vcnil;
}

//
// this is where we bind a string to a function implementation:
//
// case 1
// compile(fname args... expr) where expr is a non-string
// in this case, all the input to the function has been evaluated
// and we can just perform the binding and setup the fundef object.
//
// case 2:
// compile(fname args... str)
// this time we need to either call the interpreter to generate an expr
// we can feed to the interpreter loop (might be tricky), or take the
// resulting expr and feed it to "translate" to produce a chunk of code we
// can run through the toolchain and dynamically load. this isn't used much
// outside of "load-file", so maybe we can avoid this complexity if we
// produce some version of "load-file" that doesn't use this functionality
//
// there is a 3rd case that isn't directly dealt with here:
// what if someone compiles an expression like <<foo> <bar>>(fun a1 a2 `baz')
// where the initial expression evaluates to "compile". we could just
// disallow it, since it is very unlikely, or we would have to produce
// a runtime function that would do the binding between the string
// and the translated version of baz. we would need another type of function
// object like "vc_rt_fundef" that would do the function setup properly
// instead of trying to do a translation (which is what we are doing here...
// ie, we are noticing a specific syntactic form and generating a translation for
// that.)


vc
trans_compile(VCArglist *a, VcIO o)
{
    // note: the existing vc_fundef stuff won't really be usable here, so
    // we have a separate class for dealing with translated functions.

    DwString fnm("vc ret;vc fnm = %1();");
    fnm.arg((const char *)(*a)[0]);
    DwString arglist_tmpl("VCArglist na;");
    for(int i = 1; i < a->num_elems() - 1; ++i)
    {
        arglist_tmpl += DwString("na.append(%1());").arg((const char *)(*a)[i]);
    }

    DwString fdef_tmpl("vc fdef = %1();");
    fdef_tmpl.arg((const char *)(*a)[a->num_elems() - 1]);

    DwString tmpl(
    "vc_trans_fundef *f = new vc_trans_fundef(fnm, &na, fdef, VC_FUNC_NORMAL);"
                "vc v;"
                "v.redefine(f);"
                "fnm.bind(v);"
                ""
                );

    o << fnm.c_str() << arglist_tmpl.c_str() << fdef_tmpl.c_str() << tmpl.c_str();
    return vcnil;
}
#endif
