#include "vccfg.h"
#ifndef DWYCO_NO_VCREGEX
#include <regex.h>
#include <string.h>
#include "reobj.h"

// dwight note: don't use new and delete because
// they are not guaranteed to be compatible
// with malloc/free, which are used in the
// underlying stuff
Regex::~Regex()
{
    regfree(&compiled);
    delete [] matches;
}

#define NREGS 100

Regex::Regex(const char* t)
{
    ctor_error = regcomp(&compiled, t, 0);
    // my understanding from the docs are that single
    // subexpressions could generate multiple entries
    // in a result match array, so knowing the number of
    // subexprs is kinda useless. just take a guess and say
    // we'll keep 100 subexpressions tops
    matches = new regmatch_t[NREGS];
    memset(matches, 0, sizeof(matches[0]) * NREGS);
    nmatches = NREGS;
}

int
Regex::error() const
{
	return ctor_error != 0;
}

int Regex::match_info(int& start, int& length, int nth) const
{
    if (nth < 0 || nth >= NREGS)
        return 0;
    else
    {
        start = matches[nth].rm_so;
        length = matches[nth].rm_eo - start;
        return start >= 0 && length >= 0;
    }
}

// hmmm, don't think this works right anymore
int Regex::search(const char* s, int len, int& matchlen, int startpos) const
{
    int matchpos, pos, range;
    if (startpos >= 0)
    {
        pos = startpos;
        range = len - startpos;
    }
    else
    {
        pos = len + startpos;
        range = -pos;
    }
    char *substr = new char[range + 1];
    memset(substr, 0, range + 1);
    memcpy(substr, s + pos, range);
    int err = regexec(&compiled, substr, nmatches, matches, 0);
    if(err == REG_NOMATCH)
    {
        matchlen = 0;
        delete [] substr;
        return -1;
    }
    else if(err == REG_ESPACE)
    {
        matchlen = 0;
        delete [] substr;
        return -2;
    }
    matchlen = matches[0].rm_eo - matches[0].rm_so;
    delete [] substr;
    return matches[0].rm_so;
}

int Regex::match(const char*s, int len, int p) const
{
    if (p < 0)
    {
        p += len;
        if (p > len)
            return -1;

    }
    else if (p > len)
        return -1;

    int err = regexec(&compiled, s + p, nmatches, matches, 0);
    if(err == REG_NOMATCH)
        return -1;
    if(err == REG_ESPACE)
        return -2;
    return matches[0].rm_eo - matches[0].rm_so;
}

#ifdef TEST
#include <stdio.h>
int
main(int, char **)
{
    for(int i = 0; i < 100; ++i)
    {
        Regex foo(".*a.*");
    }

    Regex bar("abc");
    printf("%d\n", bar.match("abc", 3));
    Regex baz(".*abc.*");
    printf("%d\n", baz.match("dillabchole", 11));
}
#endif

#endif
