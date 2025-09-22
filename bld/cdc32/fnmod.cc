
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// this changes file names based on
// some rules. this is used to store
// the various files used by the
// dll in different places (in the past
// it was always assumed to be in ".")
//
// the input filename is always assumed to
// have no absolute path information on it
// and it is relative to "." (the cwd)

#include "dwstr.h"
#include "sepstr.h"
#ifndef STANDALONE
#include "dwrtlog.h"
#include "qmsg.h"
#endif

namespace dwyco {


static char *System_prefix;
static char *User_prefix;
static char *Tmp_prefix;

// tired of getting burned by the FNMOD define, this is used to do
// it dynamically, so old software sees what it is used to (ie, no
// filename mapping.)
static int Do_fnmod;

#include "gp.cpp"

void
set_fn_prefixes(
    const char *sys_pfx,
    const char *user_pfx,
    const char *tmp_pfx
)
{

    if(sys_pfx)
    {
        if(System_prefix)
            delete [] System_prefix;
        System_prefix = new char [strlen(sys_pfx) + 1];
        strcpy(System_prefix, sys_pfx);
    }
    if(user_pfx)
    {
        if(user_pfx && User_prefix &&
                strcmp(user_pfx, User_prefix) != 0)
        {
#ifndef STANDALONE
            // new place to find user list, clear out our existing
            // cached info
            //UID_infos = vc(VC_MAP);
            MsgFolders = vc(VC_TREE);
#endif
        }
        if(User_prefix)
            delete [] User_prefix;
        User_prefix = new char [strlen(user_pfx) + 1];
        strcpy(User_prefix, user_pfx);
    }
    if(tmp_pfx)
    {
        if(Tmp_prefix)
            delete [] Tmp_prefix;
        Tmp_prefix = new char [strlen(tmp_pfx) + 1];
        strcpy(Tmp_prefix, tmp_pfx);
    }
    // XXX be sure to kill all the cached values in the hash table
    for(int i = MIN_HASH_VALUE; i <= MAX_HASH_VALUE; ++i)
    {
        delete wordlist[i].cached_val;
        wordlist[i].cached_val = 0;
    }
    if(System_prefix || User_prefix || Tmp_prefix)
        Do_fnmod = 1;
    else
        Do_fnmod = 0;
}

void
get_fn_prefixes(DwString& sys, DwString& user, DwString& tmp)
{
    if(System_prefix)
        sys = System_prefix;
    if(User_prefix)
        user = User_prefix;
    if(Tmp_prefix)
        tmp = Tmp_prefix;
}

static
int
check_pfx(const DwString& pfx, const DwString& fn)
{
    if(pfx.length() > fn.length())
        return 0;
    DwString as(fn.c_str(), 0, pfx.length());
    if(pfx == as)
    {
        GRTLOG("already suf %s", fn.c_str(), 0);
        return 1;
    }
    return 0;
}

int
filename_modify(const DwString& fn, DwString& fn_out)
{
    if(!Do_fnmod)
    {
        fn_out = fn;
        return 1;
    }

    if(fn.length() == 0)
        oopanic("mapping empty file");
    // check for exact matches first
    // that way we avoid
    // some problems with substring
    // matches (ie, we want to match
    // xyz.dif to a different prefix than
    // *.dif).
    struct fn_rules *fnr = Perfect_Hash::in_word_set(fn.c_str(), fn.length());
    if(fnr)
    {
        if(fnr->cacheable && !fnr->cached_val)
        {
            if(*fnr->pfx)
                fnr->cached_val = new DwString(*fnr->pfx);
            else
                fnr->cached_val = new DwString;
            *fnr->cached_val += fn;
        }
        if(fnr->cacheable)
        {
            fn_out = *fnr->cached_val;
            GRTLOG("exact %s -> %s", fn.c_str(), fn_out.c_str());
            return 1;
        }
        // hmmm, exact match but not cached, something is wrong
        DwString a("\"%1\" exact not cached");
        a.arg(fn);
        oopanic(a.c_str());

    }

    // ok, this was a bonehead move not having them all 3 chars...
    // just special case it for now
    if(fn.length() >= 2 && fn.at(fn.length() - 1) == 'q' && fn.at(fn.length() - 2) == '.')
    {
        fnr = Perfect_Hash::in_word_set(".q", 2);
        if(!fnr)
            oopanic("something wrong with hash tables");
        if(*fnr->pfx)
        {
            if(check_pfx(*fnr->pfx, fn))
            {
                fn_out = fn;
                return 1;
            }
            fn_out = *fnr->pfx;
            fn_out += fn;
        }
        else
        {
            fn_out = fn;
        }
        GRTLOG("suffix %s -> %s", fn.c_str(), fn_out.c_str());
        return 1;
    }

    if(fn.length() >= 4)
    {
        DwString suf(fn.c_str(), fn.length() - 4, 4);
        fnr = Perfect_Hash::in_word_set(suf.c_str(), suf.length());
        if(!fnr)
            oopanic("can't map suffix");
        if(*fnr->pfx)
        {
            if(check_pfx(*fnr->pfx, fn))
            {
                fn_out = fn;
                return 1;
            }
            fn_out = *fnr->pfx;
            fn_out += fn;
        }
        else
        {
            fn_out = fn;
        }
        GRTLOG("suffix %s -> %s", fn.c_str(), fn_out.c_str());
        return 1;
    }
    DwString msg = fn;
    msg += " didn't match anything";
    oopanic(msg.c_str());
    fn_out = fn;
    return 0;
}

DwString
newfn(const DwString& fn)
{
    DwString nfn;
    if(!filename_modify(fn, nfn))
        oopanic("bogus filename");
    return nfn;
}

#if 0
DwString
newfn(const char *fn)
{

    if(!Do_fnmod)
        return fn;

    DwString nfn;
    DwString a(fn);
    if(!filename_modify(a, nfn))
        oopanic("bogus filename2");
    return nfn;
}
#endif

// for cases where we want to force the location of
// the file
DwString
newfn_userpfx(const char *fn)
{
    DwString a(fn);
    if(!Do_fnmod)
        return a;

    if(!User_prefix)
        return a;
    DwString p(User_prefix);
    p += a;
    return p;

}

DwString
prepend_pfx(const char *subdir, const char *fn)
{
    DwString a(subdir);
    a = newfn(a);
    a += DIRSEPSTR;
    a += fn;
    return a;
}

// misc filename modification things

DwString
dwbasename(const char *name)
{
    DwString f(name);
    auto b = f.rfind(DIRSEPSTR);
    if(b == DwString::npos)
        return f;
    f.erase(0, b + 1);
    return f;
}

int
is_msg_fn(vc fn)
{
    if(fn.len() != 24)
        return 0;
    DwString a((const char *)fn);
    a.to_lower();
    if(a.find_first_not_of("abcdef0123456789") != 20)
        return 0;
    if(a.rfind(".snt") != 20 && a.rfind(".bod") != 20)
        return 0;
    return 1;
}

int
is_msg_fn(const DwString& fn)
{
    if(fn.length() != 24)
        return 0;
    DwString a = fn;
    a.to_lower();
    if(a.find_first_not_of("abcdef0123456789") != 20)
        return 0;
    if(a.rfind(".snt") != 20 && a.rfind(".bod") != 20)
        return 0;
    return 1;
}

int
is_user_dir(const DwString& fn)
{
    if(fn.length() != 24)
        return 0;
    DwString a = fn;
    a.to_lower();
    if(a.find_first_not_of("abcdef0123456789") != 20)
        return 0;
    if(a.rfind(".usr") != 20)
        return 0;
    return 1;
}

int
is_attachment(const vc& fn)
{
    if(fn.len() != 24)
        return 0;
    DwString a((const char *)fn);
    a.to_lower();
    if(a.find_first_not_of("abcdef0123456789") != 20)
        return 0;
    if(a.rfind(".dyc") != 20 && a.rfind(".jpg") != 20 && a.rfind(".fle") != 20)
        return 0;
    return 1;
}

int
is_attachment(const DwString& fn)
{
    if(fn.length() != 24)
        return 0;
    DwString a = fn;
    a.to_lower();
    if(a.find_first_not_of("abcdef0123456789") != 20)
        return 0;
    if(a.rfind(".dyc") != 20 && a.rfind(".jpg") != 20 && a.rfind(".fle") != 20)
        return 0;
    return 1;
}

DwString
fn_extension(const DwString& fn)
{
    int c = fn.rfind(".");
    if(c == DwString::npos)
        return "";
    DwString ret = fn;
    ret.remove(0, c);
    return ret;
}

DwString
fn_extension(vc fn)
{
    DwString a((const char *)fn);
    return fn_extension(a);
}

DwString
fn_base_wo_extension(const DwString& fn)
{
    DwString base = dwbasename(fn.c_str());
    int c = base.rfind(".");
    if(c == DwString::npos)
        return base;
    base.remove(c);
    return base;
}

}
