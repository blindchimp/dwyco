
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/chatchro.cc 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#include "vc.h"
#include "chatchro.h"
#include "gvchild.h"
#include "owlchrot.h"

ChatCharOwl::ChatCharOwl()
{
    box = 0;
    curs0 = 0;
    curs1 = 0;
}

ChatCharOwl::ChatCharOwl(ChatBox *c) : box(c)
{
    curs0 = 0;
    curs1 = 0;
}

int
ChatCharOwl::output(vc log, int id)
{
    ChatBox *cb = cb_by_id(id);
    if(!cb)
        return 0;

    OwlCharOutput *t = (OwlCharOutput*)cb->output;

    // interpret log to put the chars into
    // the edit of the chat box

    int i;
    // windows: seems that inserts work fine
    // in read-only mode, but deletions do not.
    t->SetReadOnly(0);
    // before processing the log, put the cursor back
    // to where it was before, in case the user
    // has been jacking with it.
    t->SetSelection(curs0, curs1);
    for(i = 0; i < log.num_elems(); ++i)
    {
        const char *s = log[i][0];
        char a[2];
        uint pos1, pos2;
        a[1] = 0;
        int len;
        const char *buf;
        if(strcmp(s, "k") == 0)
        {
            a[0] = (int)log[i][1];

            switch(a[0])
            {
            case '\r':
            case '\n':
                t->Insert("\r\n");
                t->SetSelection(curs0 + 2, curs0 + 2);
                break;
            case '\b':

                t->GetSelection(pos1, pos2);
                len = t->GetTextLen();
                if(len >= 2)
                {
                    buf = t->LockBuffer();
                    if(buf[pos1 - 2] == '\r' && buf[pos1 - 1] == '\n')
                    {
                        t->UnlockBuffer(buf);
                        t->DeleteSubText(pos1 - 2, pos1);
                        t->SetSelection(pos1 - 2, pos1 - 2);
                        break;
                    }
                    t->UnlockBuffer(buf);
                }
                if(pos1 >= 1)
                {
                    t->DeleteSubText(pos1 - 1, pos1);
                    t->SetSelection(pos1 - 1, pos1 - 1);
                }
                break;

            default:
                t->Insert(a);
                t->SetSelection(curs0 + 1, curs0 + 1);
            }

            //t->key_press((int)log[i][1], (int)log[i][2], 1);
            //t->HandleMessage(WM_KEYDOWN, (int)log[i][1], (int)log[i][3]);
        }
        else if(strcmp(s, "m") == 0)
        {
            int chr = (int)log[i][1];
            t->SetSelection(chr, chr);
        }
        else if(strcmp(s, "s") == 0)
        {
            t->SetSelection((int)log[i][1], (int)log[i][2]);
        }
        else if(strcmp(s, "c") == 0)
        {
            t->Clear();
            t->SetSelection(0, 0);
        }
        else if(strcmp(s, "p") == 0)
        {
            t->Insert((const char *)log[i][1]);
            int tmp = log[i][1].len();
            t->SetSelection(curs0 + tmp, curs0 + tmp);
        }
        else if(strcmp(s, "d") == 0)
        {
            t->GetSelection(pos1, pos2);
            if(pos1 != pos2)
            {
                t->DeleteSubText(pos1, pos2);
            }
            else
            {
                len = t->GetTextLen();
                if(pos1 + 1 < len)
                {
                    buf = t->LockBuffer();
                    if(buf[pos1] == '\r' && buf[pos1 + 1] == '\n')
                    {
                        t->UnlockBuffer(buf);
                        t->DeleteSubText(pos1, pos1 + 2);
                        break;
                    }
                    t->UnlockBuffer(buf);
                }
                if(pos1 < len)
                {
                    t->DeleteSubText(pos1, pos1 + 1);
                }
            }
        }
        // after every char, get the current cursor setup.
        t->GetSelection(curs0, curs1);
    }
    t->GetSelection(curs0, curs1);
    t->SetReadOnly(1);
    return 1;
}
