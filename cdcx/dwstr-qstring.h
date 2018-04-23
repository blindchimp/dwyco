
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWSTR_QSTRING_H
#define DWSTR_QSTRING_H
#include <QByteArray>
#include <QHash>
#include <string.h>

void cdcxpanic(const char *);

struct DwOString : public QByteArray
{
    enum {npos = -1};

    DwOString() : QByteArray("") {}
    DwOString(const char *a) : QByteArray(a) {}
    DwOString(const char *a, int s, int len) : QByteArray(a + s, len) {}
    DwOString(const QByteArray& b) : QByteArray(b) {}

    const char *c_str() const {
        return this->constData();
    }
    int eq(const char *a) const {
        return (*this) == a;
    }
    int find(const char *a) const {
        return this->indexOf(a);
    }
    int find(const DwOString& a) const {
        return this->indexOf(a);
    }
    // note: not exact interface, replace does all occurences
    // but old interface only did first occurance by default
    int srep(const char *find, const char *repl) {
        this->replace(find, repl);
        return 1;
    }
    int srep_all(const char *find, const char *repl) {
        this->replace(find, repl);
        return 1;
    }
    int srep(const DwOString& find, const DwOString& repl) {
        this->replace(find, repl);
        return 1;
    }
    long hashValue() const {
        return qHash(*this);
    }
    int compare(const DwOString& s2) const
    {
        int len = length() < s2.length() ? length() : s2.length();
        int c = memcmp(this->constData(), s2.constData(), len);
        if(c != 0)
            return c;
        if(length() < s2.length())
            return -1;
        else if(length() > s2.length())
            return 1;
        return 0;
    }
    void remove(int pos) {
        this->truncate(pos);
    }
    void remove(int pos, int len) {
        QByteArray::remove(pos, len);
    }

    int find_first_of(const char *set) const {
        if(strlen(set) != 1)
            cdcxpanic("unimp");
        return this->indexOf(set);
    }

    int find_last_of(const char *set) const {
        if(strlen(set) != 1)
            cdcxpanic("unimp");
        return this->lastIndexOf(set);
    }

    int rfind(const char *s) const {
        return this->lastIndexOf(s);
    }

    void erase(int start, int n = npos) {
        if(n == npos)
            remove(start);
        else
            remove(start, n);
    }


    DwOString&
    arg(const DwOString& a1, const DwOString& a2 = DwOString(), const DwOString& a3 = DwOString(),
        const DwOString& a4 = DwOString())
    {
        srep_all("%1", a1);
        srep_all("%2", a2);
        srep_all("%3", a3);
        srep_all("%4", a4);
        return(*this);
    }

};

#endif
