#ifndef DWSTR_H
#define DWSTR_H
// real simple string class
// crude, but effective. assumes good ol' 8 bit ascii strings

#include "dwvec.h"
#include <string.h>
#if defined(_Windows) && defined(__BORLANDC__)
#include <mem.h>
#endif

class DwString : private DwVec<char>
{
public:
	DwString();
	DwString(const char *);
	DwString(const char *, int start, int len);
	DwString(const char *, int len);
	enum {npos = -1};

	const char *c_str() const;
	
	DwString& operator+=(const DwString&);
	DwString& operator+=(const char *);
	DwString& operator+=(char);
    int operator==(const DwString& s) const {
        return DwVec<char>::operator==(s);
    }
    int operator!=(const DwString& s) const {
        return !((*this) == s);
    }
	int operator<(const DwString& rt) const;
    // warning, eq uses strcmp, so it won't work right unless
	// the DwString is 0-free
	// operator== does work right tho
	int eq(const char *) const;
	int contains(const char *) const;
	int contains(const DwString&) const;
    int find(const char *, int start = 0) const;
    int find(const DwString&, int start = 0) const;
	int rfind(const char *) const;
	void erase(int start, int count = -1);
	void remove(int start, int count = -1);
	int length() const;
	int find_first_of(const char *) const;
	int find_first_not_of(const char *) const;
	int find_last_of(const char *) const;
	int find_last_not_of(const char *) const;
	char at(int) const;
	void replace(int idx, int len, const DwString&);
	int compare(const DwString&) const;
	void insert(int idx, const DwString&);
	void to_lower();
	int read_line(int fd);
	void *operator new(size_t, void *p) {return p;}
	void *operator new(size_t, DwString*p) {return p;}
	void *operator new(size_t a) {return ::operator new(a);}
	unsigned long hashValue() const;
	int srep(const char *find, const char *repl, int all = 0);
	int srep(const DwString& find, const DwString& repl, int all = 0);
    // warning: arg performs the substitutions in place and with multiple passes, so if
    // %1 contains a "%2" in it, that will get replaced if the second arg is
    // specified.
    DwString& arg(const DwString& a1, const DwString& = DwString(), const DwString& = DwString(),
                  const DwString& = DwString());
};

inline
char
DwString::at(int i) const
{
	return (*this)[i];
}

inline
const char *
DwString::c_str() const
{
	return &(*this)[0];
}

inline
int
DwString::length() const
{
	// note: length less the terminating 0, which
	// we always put on.
	return num_elems() - 1;
}

inline
DwString::DwString()
    : DwVec<char>(1, !DWVEC_FIXED, !DWVEC_AUTO_EXPAND, 64, 0, 1)
{
	(*this)[0] = 0;
}

inline
DwString::DwString(const char *s)
    : DwVec<char>(strlen(s) + 1, !DWVEC_FIXED, !DWVEC_AUTO_EXPAND, 64, 0, 1)
{
	strcpy(&(*this)[0], s);
}

inline
DwString::DwString(const char *s, int start, int len)
    : DwVec<char>(len + 1, !DWVEC_FIXED, !DWVEC_AUTO_EXPAND, 64, 0, 1)
{
	memcpy(&(*this)[0], &s[start], len);
	(*this)[len] = 0;
}

inline
DwString::DwString(const char *s, int len)
    : DwVec<char>(len + 1, !DWVEC_FIXED, !DWVEC_AUTO_EXPAND, 64, 0, 1)
{
	memcpy(&(*this)[0], &s[0], len);
	(*this)[len] = 0;
}

inline
DwString&
DwString::operator+=(const char *s)
{
	int l = strlen(s);
	int old_count = count;
	set_size(count + l);
	char *eos = &(*this)[old_count - 1];
	strcpy(eos, s);
	return *this;
}

inline
DwString&
DwString::operator+=(char s)
{
	int old_count = count;
	set_size(count + 1);
	char *eos = &(*this)[old_count - 1];
	*eos = s;
	(*this)[count - 1] = 0;
	return *this;
}

inline
DwString&
DwString::operator+=(const DwString& s)
{
	int l = s.length();
	int old_count = count;
	set_size(count + l);
	char *eos = &(*this)[old_count - 1];
	memcpy(eos, &s[0], l);
	(*this)[count - 1] = 0;
	return *this;
}

inline
int
DwString::eq(const char *s) const
{
	if(strcmp(c_str(), s) == 0)
		return 1;
	return 0;
}

inline
void
DwString::erase(int start, int n)
{
	if(n == npos)
		n = count - start - 1;
	else if(start + n > count - 1)
		oopanic("bad erase");
	del(start, n);
}

inline
void
DwString::remove(int start, int n)
{
	erase(start, n);
}

inline
int
DwString::operator<(const DwString& rt) const
{
	int i = compare(rt);
	if(i < 0)
		return 1;
	return 0;
}

#endif
