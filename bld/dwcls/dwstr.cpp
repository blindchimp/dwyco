
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#include "dwstr.h"
#ifdef LINUX
#include <unistd.h>
#else
#include <io.h>
#endif

#ifdef DWYCO_VC_CONV
#include "vc.h"
DwString::DwString(const vc& v)
    : DwVec<char>(v.len() + 1, !DWVEC_FIXED, !DWVEC_AUTO_EXPAND, 64, 0, 1)
{
    if(v.type() != VC_STRING)
        oopanic("bad vc dwstring conversion");
    memcpy(&(*this)[0], (const char *)v, v.len());
    (*this)[v.len()] = 0;
}
DwString&
DwString::operator+=(const vc& s)
{
    int l = s.len();
    if(l == 0)
        return *this;
    int old_count = count;
    set_size(count + l);
    char *eos = &(*this)[old_count - 1];
    memcpy(eos, (const char *)s, l);
    (*this)[count - 1] = 0;
    return *this;
}

DwString::operator vc() const
{
    return vc(VC_BSTRING, c_str(), length());
}

#endif

DwString&
DwString::tr(char from, char to)
{
    for(int i = 0; i < count - 1; ++i)
    {
        if(values[i] == from)
            values[i] = to;
    }
    return (*this);
}

int
DwString::find_first_of(const char *set) const
{
    DwString tmp(set);
    tmp.del(tmp.num_elems() - 1);

    for(int i = 0; i < count - 1; ++i)
    {
        if(tmp.index(values[i]) != -1)
            return i;
    }
    return npos;
}

int
DwString::find_first_not_of(const char *set) const
{
    DwString tmp(set);
    tmp.del(tmp.num_elems() - 1);

    for(int i = 0; i < count - 1; ++i)
    {
        if(tmp.index(values[i]) == -1)
            return i;
    }
    return npos;
}

int
DwString::find_last_of(const char *set) const
{
    DwString tmp(set);
    // trim off the null
    tmp.del(tmp.num_elems() - 1);

    for(int i = count - 2; i >= 0; --i)
    {
        if(tmp.index(values[i]) != -1)
            return i;
    }
    return npos;
}

int
DwString::find_last_not_of(const char *set) const
{
    DwString tmp(set);
    tmp.del(tmp.num_elems() - 1);

    for(int i = count - 2; i >= 0; --i)
    {
        if(tmp.index(values[i]) == -1)
            return i;
    }
    return npos;
}

int
DwString::find(const char *s, int start) const
{
    int l = strlen(s);
    if(l > count - 1 - start)
        return npos;
    if(l == 0)
        return npos;
    for(int i = start; i < count - 1 - (l - 1); ++i)
    {
        if(memcmp(&values[i], s, l) == 0)
            return i;
    }
    return npos;
}

int
DwString::find(const DwString& s, int start) const
{
    int l = s.length();
    if(l > count - 1 - start)
        return npos;
    if(l == 0)
        return npos;
    for(int i = start; i < count - 1 - (l - 1); ++i)
    {
        if(memcmp(&values[i], s.c_str(), l) == 0)
            return i;
    }
    return npos;
}

int
DwString::rfind(const char *s) const
{
    int l = strlen(s);
    if(l > count - 1)
        return npos;
    if(l == 0)
        return npos;
    for(int i = count - 2 - (l - 1); i >= 0; --i)
    {
        if(memcmp(&values[i], s, l) == 0)
            return i;
    }
    return npos;
}

int
DwString::contains(const char *s) const
{
    return find(s) != npos;
}

int
DwString::contains(const DwString& s) const
{
    return find(s) != npos;
}

void
DwString::replace(int idx, int len, const DwString& s)
{
    remove(idx, len);
    spread(idx, s.length());
    memcpy(&(*this)[idx], &s[0], s.length());
    (*this)[count - 1] = 0;
}

void
DwString::insert(int idx, const DwString& s)
{
    spread(idx, s.length());
    memcpy(&(*this)[idx], &s[0], s.length());
    (*this)[count - 1] = 0;
}

int
DwString::compare(const DwString& s2) const
{
    int len = length() < s2.length() ? length() : s2.length();
    int c = memcmp(&(*this)[0], &s2[0], len);
    if(c != 0)
        return c;
    if(length() < s2.length())
        return -1;
    else if(length() > s2.length())
        return 1;
    return 0;
}

void
DwString::to_lower()
{
    int n = length();
    for(int i = 0; i < n; ++i)
    {
        char c = values[i];
        if(c >= 'A' && c <= 'Z')
            values[i] = (c - 'A') + 'a';
    }
}

int
DwString::read_line(int fd)
{
    erase(0);
    char c;
    int i;
    while((i = read(fd, &c, 1)) == 1)
    {
        if(c == '\n')
            break;
        *this += c;
    }
    if(i == 0 && length() > 0)
        return 1;
    return i;
}

unsigned long
DwString::hashValue() const
{
    unsigned long hv = 0;
    long *s = (long *)&values[0];
    int len = length();
    int n = len / sizeof(*s);
    int left = len % sizeof(*s);
    int i;
    for(i = 0; i < n; ++i)
        hv += *s++;
    char *s2 = (char *)s;
    for(i = 0; i < left; ++i)
        hv += *s2++;
    hv *= 0x15a4e35L;
    return hv;
}

int
DwString::srep(const char *to_find, const char *repl, int all)
{
    return srep(DwString(to_find), DwString(repl), all);
}

int
DwString::srep(const DwString& to_find, const DwString& repl, int all)
{
    int did_one = 0;
    DwString orig(*this);
    int last_i = 0;
    int n = 0;
    int delta = repl.length() - to_find.length();
    do
    {
        int i = orig.find(to_find, last_i);
        if(i == npos)
            break;
        last_i = i + to_find.length();
        replace(i  + (n * delta), to_find.length(), repl);
        did_one = 1;
        ++n;
    } while(all);
    return did_one;
}

DwString&
DwString::arg(const DwString& a1, const DwString& a2, const DwString& a3,
              const DwString& a4)
{
    srep("%1", a1, 1);
    srep("%2", a2, 1);
    srep("%3", a3, 1);
    srep("%4", a4, 1);
    return(*this);
}

#include <stdio.h>

DwString
DwString::fromInt(int i)
{
    char a[100];
    if(snprintf(a, sizeof(a), "%d", i) >= sizeof(a))
        oopanic("truncated int");
    return(a);
}

DwString
DwString::fromLong(long i)
{
    char a[100];
    if(snprintf(a, sizeof(a), "%ld", i) >= sizeof(a))
        oopanic("truncated long");
    return(a);
}

DwString
DwString::fromUint(unsigned int i)
{
    char a[100];
    if(snprintf(a, sizeof(a), "%u", i) >= sizeof(a))
        oopanic("truncated uint");
    return(a);
}

static
int
hexd(char a)
{
    if(a >= '0' && a <= '9')
        return a - '0';
    if(a >= 'a' && a <= 'f')
        return a - 'a' + 10;
    if(a >= 'A' && a <= 'F')
        return a - 'A' + 10;
    return 0;
}

static
void
tohexd(char a, char *out)
{
    int d = a & 0xf;
    if(d >= 0 && d <= 9)
        d += '0';
    else if(d >= 10 && d <= 15)
        d = 'a' + (d - 10);
    out[1] = d;

    d = (a >> 4) & 0xf;
    if(d >= 0 && d <= 9)
        d += '0';
    else if(d >= 10 && d <= 15)
        d = 'a' + (d - 10);
    out[0] = d;
}

DwString
DwString::from_hex(const DwString& s)
{
    const char *a = &s[0];
    int len = s.length();
    DwString d;
    //d.set_size(len / 2);
    for(int i = 0; i < len / 2; ++i)
    {
        d += (hexd(a[i * 2]) << 4) | hexd(a[i * 2 + 1]);
    }
    return d;
}

DwString
DwString::to_hex(const DwString& s)
{
    const char *a = &s[0];
    int len = s.length();
    DwString d;
    d.set_size(len * 2 + 1);
    for(int i = 0; i < len; ++i)
    {
        tohexd(a[i], &d[2 * i]);
        //sprintf(&d[2 * i], "%02x", a[i] & 0xff);
    }
    d[2 * len] = 0;
    return d;
}



#undef TESTSTR
#ifdef TESTSTR
#include <stdio.h>
#ifndef LINUX
#include <io.h>
#endif
#include <fcntl.h>
#include <unistd.h>

void
oopanic(const char *s)
{
    printf("\"%s\"\n", s);
    ::abort();
}

#ifndef LINUX
void
get_addr()
{
    // KLUGE FROM HELL
    system("ipconfig >foo.out");
    int fd;
    if((fd = open("foo.out", O_RDONLY|O_TEXT)) == -1)
        return;
    DwString line;
    while(1)
    {
        if(!line.read_line(fd))
            break;
        if(line.contains("IP Address"))
        {
            int c = line.find(":");
            if(c == DwString::npos)
                continue;
            line.erase(0, c + 1);
            // at this point, if it contains anything other
            // than numbers, "." and whitespace, we punt, it might
            // be some ipv6 or other whacky thing we can't deal with
            // right now.
            if(line.find_first_not_of(".0123456789 \t\r\n") != DwString::npos)
                continue;
            // strip off all the white space
            int w;
            while((w = line.find_first_of(" \t\r\n")) != DwString::npos)
            {
                line.erase(w, 1);
            }
            printf("%s %d\n", line.c_str(), line.length());

        }
    }
    close(fd);
}
#endif

main(int, char **)
{
    DwString a;
    printf("\"%s\"\n", a.c_str());
    a += "wow";
    printf("\"%s\"\n", a.c_str());
    DwString b("bee");
    a += b;
    printf("\"%s\"\n", a.c_str());
    a += a;
    printf("\"%s\"\n", a.c_str());
    a.erase(0, 3);
    printf("\"%s\"\n", a.c_str());
    a.erase(a.length() - 3, 3);
    printf("\"%s\"\n", a.c_str());

    printf("%d\n", a.find_first_of("e"));
    printf("%d\n", a.find_first_of("oe"));
    printf("%d\n", a.find_first_of("w"));
    printf("%d\n", a.find_first_of("ow"));
    printf("%d\n", a.find_first_of("z"));

    printf("%d\n", a.find_first_not_of("e"));
    printf("%d\n", a.find_first_not_of("oe"));
    printf("%d\n", a.find_first_not_of("w"));
    printf("%d\n", a.find_first_not_of("ow"));
    printf("%d\n", a.find_first_not_of("z"));
    printf("%d\n", a.find_first_not_of("be"));

    printf("%d\n", a.find("be"));
    printf("%d\n", a.find("wow"));
    printf("%d\n", a.find("ow"));
    printf("%d\n", a.find("beewow"));
    printf("%d\n", a.find("beewowa"));
    printf("%d\n", a.find(""));

    a += "a";
    printf("%d\n", a.find("a"));

    DwString x = a;
    printf("\"%s\"\n", x.c_str());
    a += "baz";
    printf("\"%s\"\n", a.c_str());
    printf("\"%s\"\n", x.c_str());
    x = a;
    printf("\"%s\"\n", a.c_str());
    printf("\"%s\"\n", x.c_str());

    a.erase(0);
    printf("\"%s\"\n", a.c_str());
    a.erase(0);
    printf("\"%s\"\n", a.c_str());

    a = "wawawa replace me owowow";
    a.srep("replace me", "oof");
    printf("\"%s\"\n%d\n", a.c_str(), a.length());
    a.srep("wa", "ba", 1);
    printf("\"%s\"\n%d\n", a.c_str(), a.length());
    a.srep("ow", "mumble", 1);
    printf("\"%s\"\n%d\n", a.c_str(), a.length());
    a.srep("mumble", "tee", 1);
    printf("\"%s\"\n%d\n", a.c_str(), a.length());
    a.srep("te", "kab", 1);
    printf("\"%s\"\n%d\n", a.c_str(), a.length());
    a.srep("abe", "", 1);
    printf("\"%s\"\n%d\n", a.c_str(), a.length());
    a = "aabcbc";
    a.srep("abc", "foo", 1);
    printf("\"%s\"\n%d\n", a.c_str(), a.length());

    a = "%1 foo %2 bar %3 baz %4 %4 %1 %2 %3 %4";
    a.arg("mumble", "two", "three", "four");
    printf("\"%s\"\n%d\n", a.c_str(), a.length());

#ifndef LINUX
    get_addr();
#endif
#if 0
    a.erase(0, 1);
    printf("\"%s\"\n", a.c_str());
#endif
}
#endif
