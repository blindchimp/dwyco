
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
/*
 * $Header: g:/dwight/repo/vc/rcs/bonehead.h 1.1 1998/06/14 07:54:49 dwight Exp $
 */
#ifndef BONEHEAD_H
#define BONEHEAD_H

class Str;
unsigned long hash_string(const Str&);

class Str
{
public:
	char *s;
	Str() {s = 0;}
	operator char *() const {return s;}
	operator const char *() const {return s;}
	Str(char *s) {this->s = s;}
	int operator==(const Str& s2) const {return strcmp(s, s2.s) == 0;}
	unsigned long hashValue() const {return hash_string(*this);}
};

class Int
{
public:
	int i;
	Int() {i = 0;}
	operator int() const {return i;}
	Int(int i) {this->i = i;}
	int operator==(const Int& i2) const {return i == i2.i;}
	unsigned long hashValue() const {return i;}
};

class VP
{
public:
	void *p;
	VP() {p = 0;}
	operator void *() const {return p;}
	VP(void *p) {this->p = p;}
	int operator==(const VP& i2) const {return p == i2.p;}
	unsigned long hashValue() const {return (unsigned long)p;}
};

#if 0
// compiler craps out on this... sigh
template<class T, int ival>
class IntInit 
{
	T i;
	T() {i = ival;}
	operator int() const {return i;}
	T(int i) {this->i = i;}
	int operator==(const T& i2) const {return i == i2.i;}
	unsigned long hashValue() const {return i;}
};
#endif
#endif
