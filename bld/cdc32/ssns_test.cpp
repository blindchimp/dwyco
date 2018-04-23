
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// File:    test.cpp
// Author:  Brian Allen Vanderburg II
// Purpose: Some tests for SSNS
// -----------------------------------------------------------------------

#include <iostream>

#include "ssns.h"

class TestObject : public ssns::trackable
{
public:
    void Action0()
    {
        std::cout << "In TestObject::Action0\n";
    }
};

void Global0()
{
    std::cout << "In Global0\n";
}

void Global1c(const char* msg)
{
    std::cout << "In Global1c: " << msg << std::endl;
}

void Global1c2(const char* msg)
{
    std::cout << "In Global1c2: " << msg << std::endl;
}


int main()
{
    ssns::connection con;
    ssns::signal1<const char*> sig1c;

    sig1c.connect_ptrfun(&Global1c);
    con = sig1c.connect_ptrfun(&Global1c2);

    sig1c("The aliens are coming.");

    con.disconnect();

    sig1c("Wow");

    {
        ssns::signal1<const char*> sig1c2;
        con = sig1c2.connect_ptrfun(&Global1c2);
        sig1c2("Neat");
    }

    con.disconnect();

}



