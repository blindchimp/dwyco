
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/qpol.cc 1.6 1997/11/25 20:41:03 dwight Stable095 $
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "qpol.h"
#include "dwlog.h"
#include "fnmod.h"
#include "doinit.h"
using namespace dwyco;

// note: these structures are rather rickety, not
// a good idea to muck with them after they are
// built. this really should be done via LH
// or some other more robust means...

PGROUP Policy;

void
qpol_init()
{
// note: there is an optimizer bug that causes the linux build
// to crash if the poli file exists. since we don't use it anymore,
// just ignore this.
    return;
    int i = 0;

    FILE *f = fopen(newfn("poli").c_str(), "rt");
    if(f == 0)
    {
        Log_make_entry("no policies?");
        return;
    }

    enum state {POLNAME, QNAME};
    enum state st = POLNAME;
    DwString pname;
    for(;;)
    {
        char s[255];
        // polname
        // qname...
        // end
        if(fscanf(f, "%s", s) != 1)
            break;
        QPolicy *qpp;
        int do_end = 0;
        if(s[0] == ';')
        {
            while(!feof(f) && fgetc(f) != '\n')
                /*eat*/;
            continue;
        }
        if(strcmp(s, "end") == 0)
            do_end = 1;
        switch(st)
        {
        case POLNAME:
            if(do_end)
            {
                Log_make_entry("expected policy name, got end");
                return;
            }
#if defined(__CONSOLE__) || defined(__GNUG__)
            printf("policy: %s\n", s);
#endif
            pname = s;
            qpp = new QPolicy;
            st = QNAME;
            ++i;
            break;

        case QNAME:
            if(do_end)
            {
                Policy.add(pname, qpp);
                st = POLNAME;
            }
            else
                qpp->names.append(DwString(s));
            break;
        default:
            oopanic("bad pol");
        }

    }
    fclose(f);
    char s[100];
    sprintf(s, "%d policies", i);
    Log_make_entry(s);
}

