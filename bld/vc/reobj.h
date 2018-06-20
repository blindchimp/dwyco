#ifdef VCCFG_FILE
#include "vccfg.h"
#endif
#ifndef DWYCO_NO_VCREGEX
#ifndef _Regex_h
#define _Regex_h 1
#include <regex.h>

class Regex
{
private:

    Regex(const Regex&) {}  // no X(X&)
    void               operator = (const Regex&) {} // no assignment
    int ctor_error;

protected:
    regex_t compiled;
    regmatch_t *matches;
    size_t nmatches;

public:
    Regex(const char* t);

    ~Regex();

    int                match(const char* s, int len, int pos = 0) const;
    int                search(const char* s, int len,
                              int& matchlen, int startpos = 0) const;
    int                match_info(int& start, int& length, int nth = 0) const;
    int error() const;

};

#endif
#endif
