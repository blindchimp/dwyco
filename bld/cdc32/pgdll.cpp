
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "pgdll.h"
#include "dlli.h"
#include "sysattr.h"

extern DwycoChatCtxCallback dwyco_pg_callback;
extern DwycoChatCtxCallback2 dwyco_pg_callback2;

void internal_list_release(DWYCO_LIST l);
DWYCO_LIST dwyco_list_from_vc(vc vec);

vc Current_user_lobbies;
vc Current_gods;

ProfileGrid::ProfileGrid() : vp(this)
{
    // clear all attrs for this server
    init_sysattr();
    Current_user_lobbies = vc(VC_TREE);
    Current_gods = vc(VC_TREE);
    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_NEW, vp.cookie, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

ProfileGrid::~ProfileGrid()
{
    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_DEL, vp.cookie, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    vp.invalidate();
    // all info about lobbies is invalid now.
    // how about sys_attrs? technically yes...
    // ca 11/2010, there is a problem with doing this.
    // the client really needs some extra info around, even
    // if it is out of date. we'll have to revisit this whole idea
    // to find a good solution.
    //init_sysattr();
    //Current_user_lobbies = vc(VC_TREE);

}

void
ProfileGrid::add_user(vc name, vc uid)
{
    if(!vp.is_valid())
        ::abort();
    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_ADD_USER, vp.cookie, uid, uid.len(), name, name.len(), 0, 0, 0, 0, 0);

}

void
ProfileGrid::remove_user(vc uid)
{
    if(!vp.is_valid())
        ::abort();
    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_DEL_USER, vp.cookie, uid, uid.len(), 0, 0, 0, 0, 0, 0, 0);
}

void
ProfileGrid::update_ah(vc uid)
{
    if(!vp.is_valid())
        ::abort();
    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_UPDATE_AH, vp.cookie, uid, uid.len(), 0, 0, 0, 0, 0, 0, 0);

}

void
ProfileGrid::start_update()
{
    if(!vp.is_valid())
        ::abort();
    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_START_UPDATE, vp.cookie, 0, 0, 0, 0, 0, 0, 0, 0, 0);

}

void
ProfileGrid::end_update()
{
    if(!vp.is_valid())
        ::abort();
    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_END_UPDATE, vp.cookie, 0, 0, 0, 0, 0, 0, 0, 0, 0);

}

void
ProfileGrid::addq(vc q, vc uid, vc pos)
{
    if(!vp.is_valid())
        ::abort();
    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_Q_ADD, vp.cookie, uid, uid.len(), 0, 0, 0, 0, 0, q, 0);
}

void
ProfileGrid::delq(vc q, vc uid)
{
    if(!vp.is_valid())
        ::abort();
    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_Q_DEL, vp.cookie, uid, uid.len(), 0, 0, 0, 0, 0, q, 0);
}

void ProfileGrid::data(vc q, vc uid, vc data)
{
    if(!vp.is_valid())
        ::abort();
    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_Q_DATA, vp.cookie, uid, uid.len(), data, data.len(), 0, 0, 0, q, 0);
}

void
ProfileGrid::grant(vc q, vc uid, vc tm)
{
    if(!vp.is_valid())
        ::abort();
    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_Q_GRANT, vp.cookie, uid, uid.len(), 0, 0, 0, 0, 0, q, tm);
}

static int
vc_type_to_dwyco_type(vc v)
{
    switch(v.type())
    {
    case VC_NIL:
        return DWYCO_TYPE_NIL;
    case VC_VECTOR:
        return DWYCO_TYPE_VECTOR;
    case VC_INT:
        return DWYCO_TYPE_INT;
    case VC_STRING:
    case VC_BSTRING:
        return DWYCO_TYPE_STRING;
    default:
        return DWYCO_TYPE_NIL;
    }
}

int
dllify(vc v, const char*& str_out, int& len_out)
{
    if(v.type() == VC_INT)
    {
        str_out = v.peek_str();
        len_out = strlen(v.peek_str());
    }
    else if(v.type() == VC_VECTOR)
    {
        // note: used to assume that we never got anything
        // more than a simple vector. we have to make it into
        // a 1 row matrix so the dll can get at it right.
        // now: sometimes we get a vector of vectors, so
        // take that into account.
        if(v[0].type() == VC_VECTOR)
        {
            str_out = (const char *)dwyco_list_from_vc(v);
            len_out = v.num_elems();
            // bogus, assume dwyco_type doesn't have a -1
            return -1;
        }
        else
        {
            vc v2(VC_VECTOR);
            v2[0] = v;
            str_out = (const char *)dwyco_list_from_vc(v2);
            len_out = v.num_elems();
        }
    }
    else
    {
        str_out = (const char *)v;
        len_out = v.len();
    }

    return vc_type_to_dwyco_type(v);
}

void
ProfileGrid::sys_attr(vc uid, vc name, vc val)
{
    if(!vp.is_valid())
        ::abort();
    sysattr_put(name, val);
    const char *out_str;
    int len;
    int type = dllify(val, out_str, len);
    if(type == -1)
    {
        if(dwyco_pg_callback2)
            (*dwyco_pg_callback2)(DWYCO_CHAT_CTX_SYS_ATTR, vp.cookie,
                                  uid, uid.len(),
                                  name, name.len(),
                                  (DWYCO_LIST)out_str,
                                  0, 0);
    }
    else
    {
        if(dwyco_pg_callback)
            (*dwyco_pg_callback)(DWYCO_CHAT_CTX_SYS_ATTR, vp.cookie,
                                 uid, uid.len(),
                                 name, name.len(),
                                 type, out_str, len,
                                 0, 0);
    }
    if(type == DWYCO_TYPE_VECTOR || type == -1)
        internal_list_release((DWYCO_LIST)out_str);
}

void
ProfileGrid::update_attr(vc uid, vc name, vc val)
{
    if(!vp.is_valid())
        ::abort();
    const char *out_str;
    int len;
    int type = dllify(val, out_str, len);

    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_UPDATE_ATTR, vp.cookie,
                             uid, uid.len(),
                             name, name.len(),
                             type, out_str, len,
                             0, 0);

    if(type == DWYCO_TYPE_VECTOR || type == -1)
        internal_list_release((DWYCO_LIST)out_str);

}

void
ProfileGrid::add_user_lobby(vc info)
{
    if(!vp.is_valid())
        ::abort();
    const char *out_str;
    int len;
    int type = dllify(info, out_str, len);
    Current_user_lobbies.add_kv(info[SL_ULOBBY_ID], info);
    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_ADD_LOBBY, vp.cookie,
                             0, 0,
                             0, 0,
                             type, out_str, len,
                             0, 0);
    internal_list_release((void *)out_str);

}

void
ProfileGrid::del_user_lobby(vc id)
{
    if(!vp.is_valid())
        ::abort();
    const char *out_str;
    int len;
    int type = dllify(id, out_str, len);

    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_DEL_LOBBY, vp.cookie,
                             0, 0,
                             0, 0,
                             type, out_str, len,
                             0, 0);

    Current_user_lobbies.del(id);
}

void
ProfileGrid::god_online(vc uid, vc info)
{
    if(!vp.is_valid())
        ::abort();
    const char *out_str;
    int len;
    int type = dllify(info, out_str, len);

    Current_gods.add_kv(uid, info);
    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_ADD_GOD, vp.cookie,
                             uid, uid.len(),
                             0, 0,
                             type, out_str, len,
                             0, 0);
    internal_list_release((void*)out_str);

}

void
ProfileGrid::god_offline(vc uid)
{
    if(!vp.is_valid())
        ::abort();


    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_DEL_GOD, vp.cookie,
                             uid, uid.len(),
                             0, 0,
                             0, 0, 0,
                             0, 0);
    Current_gods.del(uid);
}

void
ProfileGrid::simple_data(vc qid, vc display_name, vc uid, vc data)
{
    if(!vp.is_valid())
        ::abort();
    const char *out_str;
    int len;
    int type = dllify(data, out_str, len);
    if(dwyco_pg_callback)
        (*dwyco_pg_callback)(DWYCO_CHAT_CTX_RECV_DATA, vp.cookie,
                             uid, uid.len(),
                             display_name, display_name.len(),
                             type, out_str, len,
                             qid, 0);
    internal_list_release((DWYCO_LIST)out_str);

}
