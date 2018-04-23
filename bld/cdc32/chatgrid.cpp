
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "pgdll.h"

ProfileGrid *TheChatGrid;

void
show_chat_grid()
{
    if(!TheChatGrid)
    {
        TheChatGrid = new ProfileGrid;
    }
}

void
hide_chat_grid()
{
    if(!TheChatGrid)
        return;
    delete TheChatGrid;
    TheChatGrid = 0;
}
