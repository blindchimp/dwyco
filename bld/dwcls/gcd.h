
/*
 * $Header: g:/dwight/repo/dwcls/rcs/gcd.h 1.11 1997/06/01 04:40:07 dwight Stable095 $
 */
#ifndef GCD_H
#define GCD_H

long gcd(long, long);

#ifndef _builtin_h
inline long lcm(long a, long b) {return a / gcd(a, b) * b; }
#endif
#endif
