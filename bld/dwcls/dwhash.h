
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWHASH_H
#define DWHASH_H

template<class T>
unsigned long
hash(const T& val)
{
    return val.hashValue();
}

unsigned long hash(int a);
unsigned long hash(long a);
unsigned long hash(double d);
unsigned long hash(void *d);

#endif
