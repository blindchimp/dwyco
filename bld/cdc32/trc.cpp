
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef DWYCO_TRACE
//
// these are little stubs used when tracing entry to the
// dll
//
//
#include "dlli.h"
#include "vc.h"
#include "dwrtlog.h"

static int Reentered;
static VcIOHackStr *Bufp;
#define Buf (Bufp ? *Bufp : (init_tracer(), *Bufp))
//static VcIOHackStr Buf;


#include <ctype.h>
#include <stdio.h>

void
invalidate_cb_ctx(int)
{
}

// note: this hackery is to make sure the lifetime of the
// buffer object lasts until the process dies... since some
// of the api calls (like the memory free calls) might happen
// at post-exit time during destructor cleanup.
// not perfect, but it is only used during debugging
void
init_tracer()
{
    Bufp = new VcIOHackStr;
}

static void flushbuf()
{
    if(Buf.pcount() == 0)
        return;
    char *a = new char[Buf.pcount() + 1];
    memcpy(a, Buf.ref_str(), Buf.pcount());
    a[Buf.pcount()] = 0;
    GRTLOG("%s", a, 0);
    delete [] a;
    Buf.reset();
}

static
void
hexdump(const char *ptr, int buflen) {
    const unsigned char *buf = (unsigned char *)ptr;
    int i, j;
    for (i=0; i<buflen; i+=16) {
        Buf.set_format("%06x") << i;
        Buf.set_format(0) << ": ";
        for (j=0; j<16; j++)
            if (i+j < buflen)
            {
                //printf("%02x ", buf[i+j]);
                Buf.set_format("%02x") << buf[i + j];
                Buf.set_format(0);
            }
            else
                Buf << "   ";
        //printf("   ");
        Buf << " ";
        //printf(" ");
        for (j=0; j<16; j++)
            if (i+j < buflen)
            {
                //printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
                if(isprint(buf[i + j]))
                    Buf << buf[i + j];
                else
                    Buf << ".";
            }
        Buf << "\n";
        //printf("\n");
    }
}

static void
printfunname(const char *fn)
{
    if(Reentered)
        flushbuf();
    for(int i = 0; i < Reentered; ++i)
        Buf << "*";
    ++Reentered;
    Buf << "called " << fn << "(";
}

static void
printcbfunname(const char *fn)
{
    if(Reentered)
        flushbuf();
    for(int i = 0; i < Reentered; ++i)
        Buf << "*";
    ++Reentered;
    Buf << "invoked " << fn << "(";
}

template<class T>
static void
printarg(const char *atype, const char *aname, T val)
{
    Buf << aname << "= " << val << " ";
}

template<class T>
static void
printargout(const char *atype, const char *aname, T val)
{
    if(val)
        Buf << aname << "<= " << *val << " ";
    else
        Buf << aname << "<= " << "(nil)" << " ";
}

template<class T, class L>
static void
printargout(const char *atype, const char *aname, T val,
            const char *altype, const char *alname, L len)
{
    if(val == 0 || len == 0)
    {
        Buf << "(" << aname << "," << alname << "=" << "(nil)" << ")" << "<= " << "(nil)" << " ";
        return;
    }

    if(strstr(aname, "uid") != 0 || strstr(aname, "user_id") != 0)
    {
        // note: hack, we don't have the len, but for now we know
        // it is always 10, ok for debugging anyways
        vc to_hex(vc);
        vc h = to_hex(vc(VC_BSTRING, *val, *len));
        Buf << "(" << aname << "," << alname << "<=" << *len << ")" << "<= " << (const char *)h << " ";
    }
    else
    {
        // check for non-printables, if there are none, just barf it out
        int hex = 0;
        for(int i = 0; i < *len; ++i)
        {
            if(!isprint((*val)[i] & 0xff))
            {
                hex = 1;
                break;
            }
        }
        if(hex)
        {
            Buf << "(" << aname << "," << alname << "<=" << *len << ")" << "<= " << "\n";
            hexdump(*val, *len);
        }
        else
            Buf << "(" << aname << "," << alname << "<=" << *len << ")" << "<= " << *val << " ";
    }
}

static void
printargout(const char *atype, const char *aname, const char **val,
            const char *altype, const char *alname, int len)
{
    if(val == 0)
    {
        Buf << "(" << aname << "," << alname << "=" << len << ")" << "= " << "(nil)" << " ";
        return;
    }
    if(strstr(aname, "uid") != 0 || strstr(aname, "user_id") != 0)
    {
        // note: hack, we don't have the len, but for now we know
        // it is always 10, ok for debugging anyways
        vc to_hex(vc);
        vc h = to_hex(vc(VC_BSTRING, *val, len));
        Buf << "(" << aname << "," << alname << "<=" << len << ")" << "<= " << (const char *)h << " ";
    }
    else
    {
        // check for non-printables, if there are none, just barf it out
        int hex = 0;
        for(int i = 0; i < len; ++i)
        {
            if(!isprint((*val)[i] & 0xff))
            {
                hex = 1;
                break;
            }
        }
        if(hex)
        {
            Buf << "(" << aname << "," << alname << "<=" << len << ")" << "<= " << "\n";
            hexdump(*val, len);
        }
        else
            Buf << "(" << aname << "," << alname << "<=" << len << ")" << "<= " << *val << " ";
    }
}

static void
printargout(const char *atype, const char *aname, char **val,
            const char *altype, const char *alname, int len)
{
    if(val == 0)
    {
        Buf << "(" << aname << "," << alname << "=" << len << ")" << "= " << "(nil)" << " ";
        return;
    }
    if(strstr(aname, "uid") != 0 || strstr(aname, "user_id") != 0)
    {
        // note: hack, we don't have the len, but for now we know
        // it is always 10, ok for debugging anyways
        vc to_hex(vc);
        vc h = to_hex(vc(VC_BSTRING, *val, len));
        Buf << "(" << aname << "," << alname << "<=" << len << ")" << "<= " << (const char *)h << " ";
    }
    else
    {
        // check for non-printables, if there are none, just barf it out
        int hex = 0;
        for(int i = 0; i < len; ++i)
        {
            if(!isprint((*val)[i] & 0xff))
            {
                hex = 1;
                break;
            }
        }
        if(hex)
        {
            Buf << "(" << aname << "," << alname << "<=" << len << ")" << "<= " << "\n";
            hexdump(*val, len);
        }
        else
            Buf << "(" << aname << "," << alname << "<=" << len << ")" << "<= " << *val << " ";
    }
}

static void
printargout(const char *atype, const char *aname, const char **val,
            const char *altype, const char *alname, int *len)
{
    if(val == 0 || *val == 0)
    {
        Buf << "(" << aname << "," << alname << "=" << len << ")" << "= " << "(nil)" << " ";
        return;
    }
    if(strstr(aname, "uid") != 0 || strstr(aname, "user_id") != 0)
    {
        // note: hack, we don't have the len, but for now we know
        // it is always 10, ok for debugging anyways
        vc to_hex(vc);
        vc h = to_hex(vc(VC_BSTRING, *val, *len));
        Buf << "(" << aname << "," << alname << "<=" << len << ")" << "<= " << (const char *)h << " ";
    }
    else
    {
        // check for non-printables, if there are none, just barf it out
        int hex = 0;
        for(int i = 0; i < *len; ++i)
        {
            if(!isprint((*val)[i] & 0xff))
            {
                hex = 1;
                break;
            }
        }
        if(hex)
        {
            Buf << "(" << aname << "," << alname << "<=" << *len << ")" << "<= " << "\n";
            hexdump(*val, *len);
        }
        else
            Buf << "(" << aname << "," << alname << "<=" << *len << ")" << "<= " << *val << " ";
    }
}

static void
printargout(const char *atype, const char *aname, char **val,
            const char *altype, const char *alname, int *len)
{
    if(val == 0 || *val == 0)
    {
        Buf << "(" << aname << "," << alname << "=" << len << ")" << "= " << "(nil)" << " ";
        return;
    }
    if(strstr(aname, "uid") != 0 || strstr(aname, "user_id") != 0)
    {
        // note: hack, we don't have the len, but for now we know
        // it is always 10, ok for debugging anyways
        vc to_hex(vc);
        vc h = to_hex(vc(VC_BSTRING, *val, *len));
        Buf << "(" << aname << "," << alname << "<=" << len << ")" << "<= " << (const char *)h << " ";
    }
    else
    {
        // check for non-printables, if there are none, just barf it out
        int hex = 0;
        for(int i = 0; i < *len; ++i)
        {
            if(!isprint((*val)[i] & 0xff))
            {
                hex = 1;
                break;
            }
        }
        if(hex)
        {
            Buf << "(" << aname << "," << alname << "<=" << *len << ")" << "<= " << "\n";
            hexdump(*val, *len);
        }
        else
            Buf << "(" << aname << "," << alname << "<=" << *len << ")" << "<= " << *val << " ";
    }
}

template<class T, class L>
static void
printarg(const char *atype, const char *aname, T val,
         const char *altype, const char *alname, L len)
{
    if(strstr(aname, "uid") != 0 || strstr(aname, "user_id") != 0)
    {
        if(val == 0)
        {
            Buf << "(" << aname << "," << alname << "=" << len << ") = (nil) ";
        }
        else
        {
            vc to_hex(vc);
            vc h = to_hex(vc(VC_BSTRING, val, len));
            Buf << "(" << aname << "," << alname << "=" << len << ")" << "= " << (const char *)h << " ";
        }
    }
    else
    {
        // check for non-printables, if there are none, just barf it out
        int hex = 0;
        for(int i = 0; i < len; ++i)
        {
            if(!isprint(val[i] & 0xff))
            {
                hex = 1;
                break;
            }
        }
        if(hex)
        {
            Buf << "(" << aname << "," << alname << "=" << len << ")" << "= " << "\n";
            hexdump(val, len);
        }
        else
            Buf << "(" << aname << "," << alname << "=" << len << ")" << "= " << val << " ";
    }
}

static void
printarg(const char *atype, const char *aname, const char *val)
{
    if(strstr(aname, "uid") != 0 || strstr(aname, "user_id") != 0)
    {
        // note: hack, we don't have the len, but for now we know
        // it is always 10, ok for debugging anyways
        vc to_hex(vc);
        vc h = to_hex(vc(VC_BSTRING, val, 10));
        Buf << aname << "= " << (const char *)h << " ";
    }
    else
        Buf << aname << "= " << val << " ";
}

template<class T>
static void
printretval(T val)
{
    Buf << ") = " << val;
    char *a = new char[Buf.pcount() + 1];
    memcpy(a, Buf.ref_str(), Buf.pcount());
    a[Buf.pcount()] = 0;
    GRTLOG("%s", a, 0);
    delete [] a;
    Buf.reset();
    --Reentered;
}

static void
printret()
{
    Buf << ")";
    char *a = new char[Buf.pcount() + 1];
    memcpy(a, Buf.ref_str(), Buf.pcount());
    a[Buf.pcount()] = 0;
    GRTLOG("%s", a, 0);
    delete [] a;
    Buf.reset();
    --Reentered;
}

template<class T>
static void
printcbretval(T val)
{
    Buf << ") = " << val;
    char *a = new char[Buf.pcount() + 1];
    memcpy(a, Buf.ref_str(), Buf.pcount());
    a[Buf.pcount()] = 0;
    GRTLOG("%s", a, 0);
    delete [] a;
    Buf.reset();
    --Reentered;
}

static void
printcbret()
{
    Buf << ")";
    char *a = new char[Buf.pcount() + 1];
    memcpy(a, Buf.ref_str(), Buf.pcount());
    a[Buf.pcount()] = 0;
    GRTLOG("%s", a, 0);
    delete [] a;
    Buf.reset();
    --Reentered;
}

#if 0
// callback tracing
// for static callbacks:
// in the static callback setting function, we insert a macro
// that swaps our "arg printing/return printing" wrapper
// function for the users functions, and stores the user function
// in a global variable.
// we emit a unique function that wraps the callback, but knows
// the type and calls pair of type-based printers
/*
emit:
static cb-type user_foo_callback;
wrap_foo_callback(cb-args)
{
	printargs...
	ret = (*user_foo_callback)(cb-args);
	printret
}
HOOK in "set_foo_callback":
user_foo_callback = cb;
cb = wrap_foo_callback;
*/
// for callbacks that come with a context and a known lifetime
// the hook has to be installed by hand at the site where the
// callback is entered into the dll
// the function swap would allow a function to be called
// that could find the context for the callback and
// get the users (function, user_arg) pair set properly
// while printing the info from the dll before doing the "user" callback
// dummy "set" functions are employed to allow dbgt.lh to generate
// the printing functions
DWYCOEXPORT
void
dwyco_dummy_set_ctx_call_disposition_callback(DwycoCallDispositionCallback cb)
{
}

// sometimes, a set of callbacks are bound together with args (like the id
// on the call_disposition + status_callbacks, or the message_send_callback and the status callback, etc.)
// this is a design flaw i think
// i'm not sure i want to fix it at this point, since it would involve
// merging the status info with the primary callbacks and change
// the api for all those callbacks. have to think about it.
//
// some ctx callbacks are called once (destroy callbacks) so we can
// chuck the context info immediately
// others are called an indeterminate number of times. we'll have to
// recycle the context info some other way (maybe insert some stuff
// into the dll to signal an explicit end of context use.)


// stuff for hooking and printing callback stuff
struct cb_ctx
{
    void *user_cb;
    void *user_arg;
};

static
void
our_DwycoChannelDestroyCallback(int id, void *user_arg)
{
    printfunname("DwycoChannelDestroyCallback");
    printarg("int", "id", id);
    cb_ctx *cc = (cb_ctx *)(user_arg);
    printarg("void *", "user_arg", cc->user_arg);
    (*(DwycoChannelDestroyCallback)cc->user_cb)(id, cc->user_arg);
    printret();
    delete cc;
}

void
hook_ctx_callback(void*& user_cb, void *&user_arg, void *wrapper_cb)
{
    cb_ctx *cc = new cb_ctx;
    cc->user_cb = (void *)user_cb;
    cc->user_arg = user_arg;

    user_cb = wrapper_cb;
    user_arg = cc;

}

#define HOOK(cb_type, cb_name, argname) \
static cb_type user_##cb_name; \
void \
hook_##cb_type(cb_type& argname) \
{ \
	user_##cb_name = argname; \
	argname = wrapped_##cb_type; \
}
#endif


#include "trc_wrappers.cpp"
#endif
