
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "vcio.h"
#define TMPBUFSTR 128000


VcIOHack::VcIOHack(FILE *s) : sio(s)
{
	format = 0;
}

void
VcIOHack::flush()
{
	fflush(sio);
}

VcIOHack&
VcIOHack::set_format(const char *a)
{
	// horrific, but ok for now
	format = a;
	return *this;
}

VcIOHack&
VcIOHack::operator<<(const char *c)
{
	fprintf(sio, format ? format : "%s", c);
	return *this;
}

VcIOHack&
VcIOHack::operator<<(void *c)
{
	fprintf(sio, format ? format : "%p", c);
	return *this;
}

VcIOHack&
VcIOHack::operator<<(char c)
{
	fprintf(sio, format ? format : "%c", c);
	return *this;
}

VcIOHack&
VcIOHack::operator<<(unsigned char c)
{
	fprintf(sio, format ? format : "%c", c);
	return *this;
}

VcIOHack&
VcIOHack::operator<<(int c)
{
	fprintf(sio, format ? format : "%d", c);
	return *this;
}

VcIOHack&
VcIOHack::operator<<(unsigned int c)
{
	fprintf(sio, format ? format : "%u", c);
	return *this;
}

VcIOHack&
VcIOHack::operator<<(long c)
{
	fprintf(sio, format ? format : "%ld", c);
	return *this;
}

VcIOHack&
VcIOHack::operator<<(long long c)
{
    fprintf(sio, format ? format : "%lld", c);
    return *this;
}


VcIOHack&
VcIOHack::operator<<(unsigned long c)
{
	fprintf(sio, format ? format : "%lu", c);
	return *this;
}

VcIOHack&
VcIOHack::operator<<(double c)
{
	fprintf(sio, format ? format : "%f", c);
	return *this;
}

VcIOHack&
VcIOHack::operator<<(vc c)
{
    VcIOHackStr tmp;
    c.print_top(tmp);
    tmp << '\0';
    fprintf(sio, format ? format : "%s", tmp.ref_str());
    return *this;
}

VcIOHack&
VcIOHack::append(const char *buf, int len)
{
    fwrite(buf, len, 1, sio);
    return *this;
}


VcIOHackStr::VcIOHackStr(int sz) : s(sz), VcIOHack(0)
{
}

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

VcIOHack&
VcIOHackStr::operator<<(const char *c)
{
	char a[TMPBUFSTR];
    unsigned int i;
	i = snprintf(a, sizeof(a) - 1, format ? format : "%s", c);
	if(i >= sizeof(a) - 1)
		oopanic("snprintf pooched");
	s.append(a, i);
	return *this;
}

VcIOHack&
VcIOHackStr::operator<<(void *c)
{
	char a[TMPBUFSTR];
    unsigned int i;
	i = snprintf(a, sizeof(a) - 1, format ? format : "%p", c);
	if(i >= sizeof(a) - 1)
		oopanic("snprintf pooched");
	s.append(a, i);
	return *this;
}

VcIOHack&
VcIOHackStr::operator<<(char c)
{
	char a[TMPBUFSTR];
    unsigned int i;
	i = snprintf(a, sizeof(a) - 1, format ? format : "%c", c);
	if(i >= sizeof(a) - 1)
		oopanic("snprintf pooched");
	s.append(a, i);
	return *this;
}

VcIOHack&
VcIOHackStr::operator<<(unsigned char c)
{
	char a[TMPBUFSTR];
    unsigned int i;
	i = snprintf(a, sizeof(a) - 1, format ? format : "%c", c);
	if(i >= sizeof(a) - 1)
		oopanic("snprintf pooched");
	s.append(a, i);
	return *this;
}

VcIOHack&
VcIOHackStr::operator<<(int c)
{
	char a[TMPBUFSTR];
    unsigned int i;
	i = snprintf(a, sizeof(a) - 1, format ? format : "%d", c);
	if(i >= sizeof(a) - 1)
		oopanic("snprintf pooched");
	s.append(a, i);
	return *this;
}

VcIOHack&
VcIOHackStr::operator<<(unsigned int c)
{
	char a[TMPBUFSTR];
    unsigned int i;
	i = snprintf(a, sizeof(a) - 1, format ? format : "%u", c);
	if(i >= sizeof(a) - 1)
		oopanic("snprintf pooched");
	s.append(a, i);
	return *this;
}

VcIOHack&
VcIOHackStr::operator<<(long c)
{
	char a[TMPBUFSTR];
    unsigned int i;
	i = snprintf(a, sizeof(a) - 1, format ? format : "%ld", c);
	if(i >= sizeof(a) - 1)
		oopanic("snprintf pooched");
	s.append(a, i);
	return *this;
}

VcIOHack&
VcIOHackStr::operator<<(long long c)
{
    char a[TMPBUFSTR];
    unsigned int i;
    i = snprintf(a, sizeof(a) - 1, format ? format : "%lld", c);
    if(i >= sizeof(a) - 1)
        oopanic("snprintf pooched");
    s.append(a, i);
    return *this;
}

VcIOHack&
VcIOHackStr::operator<<(unsigned long c)
{
	char a[TMPBUFSTR];
    unsigned int i;
	i = snprintf(a, sizeof(a) - 1, format ? format : "%lu", c);
	if(i >= sizeof(a) - 1)
		oopanic("snprintf pooched");
	s.append(a, i);
	return *this;
}

VcIOHack&
VcIOHackStr::operator<<(double c)
{
	char a[TMPBUFSTR];
    unsigned int i;
	i = snprintf(a, sizeof(a) - 1, format ? format : "%f", c);
	if(i >= sizeof(a) - 1)
		oopanic("snprintf pooched");
	s.append(a, i);
	return *this;
}

char *
VcIOHackStr::str()
{
	char *a = new char[s.length()];
	s.copy_out(a, s.length());
	return a;
}

const char *
VcIOHackStr::ref_str()
{
	return s.ref_str();
}

int
VcIOHackStr::pcount()
{
	return s.length();
}

void
VcIOHackStr::reset()
{
	s.reset();
}

VcIOHack&
VcIOHackStr::operator<<(vc c)
{
    VcIOHackStr tmp;
    c.print_top(tmp);
    tmp << '\0';
    char a[TMPBUFSTR];
    unsigned int i;
    i = snprintf(a, sizeof(a) - 1, format ? format : "%s", tmp.ref_str());
    if(i >= sizeof(a) - 1)
        oopanic("snprintf pooched");
    s.append(a, i);
    return *this;
}

VcIOHack&
VcIOHackStr::append(const char *buf, int len)
{
    s.append(buf, len);
    return *this;
}

#ifdef TESTVCIOHACK
main(int, char **)
{
	VcIOHack v(stdout);

	v << "foo\n";
	v << 1;
	v << 1.0;

	v << "baz " << "bar " << 1;

}
#endif
