
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// call types, used for UI-specific call handling
// the list of calls is the list of active calls originated
// from remote users.
//

#include "ct.h"
#include "dlli.h"
#include "dwquerybymember.h"
#include "callsm.h"

static QList<dwyco_call_type> Calls;

void
call_add(const dwyco_call_type& c)
{
    // if the call already exists, just update it
    // with the new call object
    int n = Calls.count();
    for(int i = 0; i < n; ++i)
    {
        if(Calls[i].chan_id == c.chan_id)
        {
            Calls.removeAt(i);
            break;
        }
    }
    Calls.append(c);
}


// remove a call without calling destroy
// used where a call is terminated by the dll
//
void
call_del(int chan_id)
{
    int n = Calls.count();
    for(int i = 0; i < n; ++i)
    {
        if(Calls[i].chan_id == chan_id)
        {
            Calls.removeAt(i);
            break;
        }
    }
}

void
call_del_by_uid(const QByteArray& uid)
{
    int n = Calls.count();
    for(int i = 0; i < n; ++i)
    {
        if(Calls[i].uid == uid)
        {
            Calls.removeAt(i);
            break;
        }
    }
}

int
call_exists(int chan_id)
{
    int n = Calls.count();
    for(int i = 0; i < n; ++i)
    {
        if(Calls[i].chan_id == chan_id)
        {
            return 1;
        }
    }
    return 0;
}

int
call_find(int chan_id, dwyco_call_type& call_out)
{
    int n = Calls.count();
    for(int i = 0; i < n; ++i)
    {
        if(Calls[i].chan_id == chan_id)
        {
            call_out = Calls[i];
            return 1;
        }
    }
    return 0;
}

int
call_destroy_by_uid(const QByteArray& uid)
{
    int n = Calls.count();
    int nk = 0;
    for(int i = 0; i < n; ++i)
    {
        if(Calls[i].uid == uid)
        {
            dwyco_destroy_channel(Calls[i].chan_id);
            Calls.removeAt(i);
            --n;
            --i;
            ++nk;
        }
    }
    return nk;
}

int
call_exists_by_uid(const QByteArray& uid)
{
    int n = Calls.count();
    for(int i = 0; i < n; ++i)
    {
        if(Calls[i].uid == uid)
        {
            return 1;
        }
    }
    return 0;
}

int
call_destroy_by_chan_id(int chan_id)
{
    int n = Calls.count();
    int nk = 0;
    for(int i = 0; i < n; ++i)
    {
        if(Calls[i].chan_id == chan_id)
        {
            dwyco_destroy_channel(Calls[i].chan_id);
            Calls.removeAt(i);
            --n;
            --i;
            ++nk;
        }
    }
    if(nk > 1)
        cdcxpanic("corrupted call list");
    return nk;
}

int
call_destroy_by_type(const char *type, int dir)
{
    int n = Calls.count();
    int nk = 0;
    for(int i = 0; i < n; ++i)
    {
        if(Calls[i].call_type == type)
        {
            if(dir == DWYCO_CT_ANY ||
                    dir == Calls[i].dir)
            {
                dwyco_destroy_channel(Calls[i].chan_id);
                Calls.removeAt(i);
                --n;
                --i;
                ++nk;
            }
        }
    }
    return nk;
}

int
call_destroy_by_mask(const char *type, int dir,
                     int sending_video, int receiving_video,
                     int sending_audio, int receiving_audio,
                     int pubchat, int privchat)
{
    int n = Calls.count();
    int nk = 0;
    for(int i = 0; i < n; ++i)
    {
        if(type == 0 || Calls[i].call_type == type)
        {
            if(dir == DWYCO_CT_ANY ||
                    dir == Calls[i].dir)
            {
                int sv, rv, sa, ra, pubc, privc;

                //note: this isn't obvious here, but the channel ids
                // we have here are the result of network calls, which
                // implies if a stream exists it is sending or receiving
                // its data to the network (and not, for example, to
                // a file in a local record operation. even tho technically
                // streams exists for local records to disk and to
                // remote servers.)
                if(!dwyco_channel_streams(Calls[i].chan_id,
                                          &sv, &rv, &sa, &ra, &pubc, &privc))
                    continue;
                if(sending_video != -1 && sending_video != sv)
                    continue;
                if(receiving_video != -1 && receiving_video != rv)
                    continue;
                if(sending_audio != -1 && sending_audio != sa)
                    continue;
                if(receiving_audio != -1 && receiving_audio != ra)
                    continue;
                if(pubchat != -1 && pubchat != pubc)
                    continue;
                if(privchat != -1 && privchat != privc)
                    continue;

                dwyco_destroy_channel(Calls[i].chan_id);
                Calls.removeAt(i);
                --n;
                --i;
                ++nk;
            }
        }
        return nk;
    }
    return nk;

}

int
is_video_streaming_out()
{
    int n = Calls.count();
    int sv, rv, sa, ra, pubc, privc;
    for(int i = 0; i < n; ++i)
    {

        //note: this isn't obvious here, but the channel ids
        // we have here are the result of network calls, which
        // implies if a stream exists it is sending or receiving
        // its data to the network (and not, for example, to
        // a file in a local record operation. even tho technically
        // streams exists for local records to disk and to
        // remote servers.)
        if(!dwyco_channel_streams(Calls[i].chan_id,
                                  &sv, &rv, &sa, &ra, &pubc, &privc))
            continue;
        if(sv)
            return 1;


    }

    QList<int> chans = simple_call::Simple_calls.project(&simple_call::chan_id);
    n = chans.count();
    for(int i = 0; i < n; ++i)
    {
        if(!dwyco_channel_streams(chans[i],
                                  &sv, &rv, &sa, &ra, &pubc, &privc))
            continue;
        if(sv)
            return 1;

    }
    return 0;

}

int
is_audio_streaming_out()
{
    int n = Calls.count();

    for(int i = 0; i < n; ++i)
    {

        int sv, rv, sa, ra, pubc, privc;

        //note: this isn't obvious here, but the channel ids
        // we have here are the result of network calls, which
        // implies if a stream exists it is sending or receiving
        // its data to the network (and not, for example, to
        // a file in a local record operation. even tho technically
        // streams exists for local records to disk and to
        // remote servers.)
        if(!dwyco_channel_streams(Calls[i].chan_id,
                                  &sv, &rv, &sa, &ra, &pubc, &privc))
            continue;
        if(sa)
            return 1;
    }
    return 0;
}

int
call_exists_by_type(const char *type, int dir)
{
    int n = Calls.count();
    for(int i = 0; i < n; ++i)
    {
        if(Calls[i].call_type == type)
        {
            if(dir == DWYCO_CT_ANY)
                return 1;
            if(dir == Calls[i].dir)
                return 1;
        }
    }
    return 0;
}

int
call_exists_by_type_to_uid(const char *type, const QByteArray& uid, int dir)
{
    int n = Calls.count();
    for(int i = 0; i < n; ++i)
    {
        if(Calls[i].call_type == type && Calls[i].uid == uid)
        {
            if(dir == DWYCO_CT_ANY)
                return 1;
            if(dir == Calls[i].dir)
                return 1;
        }
    }
    return 0;
}

int
call_destroy_all()
{
    int n = Calls.count();
    for(int i = 0; i < n; ++i)
    {
        dwyco_destroy_channel(Calls[i].chan_id);
    }
    Calls.clear();
    return n;
}
