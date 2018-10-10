
#include "simplesql.h"
//#include "vc.h"
#include "dlli.h"
#include "qauth.h"
#include "pkcache.h"
#include "vccrypt2.h"
#include "ser.h"
#include "mcc.h"
#include "msend.h"
#include "dhgsetup.h"

extern DwVec<ValidPtr> CompositionDeleteQ;

using namespace dwyco;

struct skid_sql : public SimpleSql
{
    skid_sql() : SimpleSql("skid.sql") {}

    void init_schema() {
        start_transaction();
        sql_simple("create table if not exists keys ("
                   "uid text collate nocase, "
                   "alt_name text collate nocase, "
                   "pubkey blob,"
                   "privkey blob,"
                   "time integer"
                   ")");
        sql_simple("create index if not exists keys_uid on keys(uid)");
        sql_simple("create index if not exists keys_alt_name on keys(alt_name)");
        sql_simple("create table if not exists pstate ("
                   "initiator_uid text collate nocase,"
                   "responding_uid text collate nocase, "
                   "alt_name text collate nocase, "
                   "state integer,"
                   "serial text collate nocase,"
                   "nonce_1 text collate nocase,"
                   "nonce_2 text collate nocase,"
                   "time integer"
                   ")");
        commit_transaction();

    }


};

static struct skid_sql *SKID;

int
init_gj()
{
    SKID = new struct skid_sql;
    SKID->init();
    return 1;
}

int
exit_gj()
{
    SKID->exit();
    delete SKID;
    SKID = 0;
    return 1;
}

// protocol states

/*
 * G is the group you want to join
 * P is the password for G
 * B is who wants to join
 * A is some member of G and can provide G's private key
 * Ek(m) is m encrypted with k = H(P) using GCM
 *
 * if any message fails to decrypt the protocol terminates with failure
 *
B->A: create m = (rB, G) and send Ek(m) to G using only G's public key
A->B: A decrypts m using k, m = Ek(G, rA, rB, B)
B->A: B decrypts m using k, if rB != m's rB fail. if G or B don't match, fail. m = Ek(G, rB, rA, A)
A receives and decrypts m, checks all items match. if so, it sends G's private key to B in
a message that is encrypted using B's public key.

*/

static
vc
xfer_enc(vc v, vc password)
{
    vc k = vclh_sha3_256(password);
    k = vc(VC_BSTRING, (const char *)k, 16);
    vc enc_ctx = vclh_encdec_open();
    vclh_encdec_init_key_ctx(enc_ctx, k, 0);
    vc p = vclh_encdec_xfer_enc_ctx(enc_ctx, v);
    if(p.is_nil())
        return vcnil;
    return serialize(p);
}

static
vc
xfer_dec(vc vs, vc password)
{
    vc v;
    if(!deserialize(vs, v))
        return vcnil;

    vc k = vclh_sha3_256(password);
    k = vc(VC_BSTRING, (const char *)k, 16);
    vc enc_ctx = vclh_encdec_open();
    vclh_encdec_init_key_ctx(enc_ctx, k, 0);
    vc ret;
    vc p = vclh_encdec_xfer_dec_ctx(enc_ctx, v, ret);
    if(p.is_nil())
        return vcnil;
    return ret;
}

static
int
post_req(int compid, vc vuid)
{
    ValidPtr p = cookie_to_ptr(compid);

    if(!p.is_valid())
    {
        GRTLOG("post_req: bad id %d", compid, 0);
        return 0;
    }
    TMsgCompose *m = (TMsgCompose *)(void *)p;
    if(!m->send_button_enabled)
    {
        GRTLOG("post_req: send not enabled %d, most likely accessing media (like playing or recording it)", compid, 0);
        return 0;
    }

    vc v(VC_VECTOR);
    v.append(vuid);

    m->rid_list = v;
    m->no_forward = 1;
    m->dont_save_sent = 1;
    m->send_buttonClick();
    if(!send_via_server(m->qfn))
    {
        GRTLOG("post_req: send startup failed", 0, 0);
        return 0;
    }
    // the file sender objects own the files now, don't let the composer delete them
    m->composer = 0;

    if(CompositionDeleteQ.index(m->vp) == -1)
        CompositionDeleteQ.append(m->vp);

    return 1;
}

static
void
terminate(vc initiator, vc responder)
{

    SKID->sql_simple("delete from pstate where initiating_uid = $1 and responding_uid = $2",
                     initiator, responder);

}

// this initiates the protocol, but the request is more like
// "i want to join the group that target_uid is in", rather than
// i want to join group G. might want to fix this eventually.
// there is a chance that we would send the message to
// someone that could decrypt the message, but not actually have
// the group key. removing the p2p key, using only the group key
// and forcing encryption would eliminate this i think since the
// recipient wouldn't be able to decrypt without the group private key.
//
// CALLED IN INITIATOR
int
start_gj(vc target_uid, vc password)
{
    vc nonce = to_hex(get_entropy());

    // note: in order to get the group pk we need to have access to
    // at least one uid that is in the group, which needs to be fixed
    // eventually.
    vc pk;
    vc alt_pk;
    vc alt_name;

    if(!get_pk2(target_uid, pk, alt_pk, alt_name))
    {
        return 0;
    }

    vc m(VC_VECTOR);
    m[0] = nonce;
    m[1] = alt_name;

    vc mk = xfer_enc(m, password);

    int comp_id = dwyco_make_special_zap_composition(DWYCO_SPECIAL_TYPE_JOIN1, 0, (const char *)mk, mk.len());
    if(comp_id == -1)
        return 0;
    if(!post_req(comp_id, target_uid))
    {
        dwyco_delete_zap_composition(comp_id);
        return 0;
    }

    SKID->sql_simple("insert into pstate (alt_name, nonce_1, initiating_uid, state, time) "
                     "values($1, $2, $3, 1, strftime('%s', 'now'))",
                     alt_name,
                     nonce,
                     to_hex(My_UID)
                     );
    return 1;
}

// CALLED IN INITIATOR
int
recv_gj2(vc from, vc msg, vc password)
{
    try
    {

        vc m = xfer_dec(msg, password);
        vc hfrom = to_hex(from);

        if(m.is_nil())
        {
            throw -1;
        }

        vc alt_name = m[0];
        vc nonce2 = m[1];
        vc nonce = m[2];
        vc our_uid = m[3];

        // first checks
        if(to_hex(My_UID) != our_uid)
        {
            throw -1;
        }

        vc res = SKID->sql_simple("select * from pstate where "
                                  "initiating_uid = $1 and nonce_1 = $2 and "
                                  "alt_name = $3 and pstate = 1",
                                  our_uid, nonce, alt_name);
        if(res.num_elems() == 0)
        {
            throw -1;
        }

        vc mr(VC_VECTOR);
        mr[0] = alt_name;
        mr[1] = nonce;
        mr[2] = nonce2;
        mr[3] = hfrom;

        vc mrs = xfer_enc(mr, password);
        int comp_id = dwyco_make_special_zap_composition(DWYCO_SPECIAL_TYPE_JOIN3, 0, (const char *)mrs, mrs.len());
        if(comp_id == -1)
            throw -1;

        if(!post_req(comp_id, from))
        {
            dwyco_delete_zap_composition(comp_id);
            throw -1;
        }
        try
        {
            SKID->start_transaction();
            VCArglist a;
            a.append("insert into pstate (responding_uid = $1,  nonce_2 = $2, time = strftime('%s', 'now'), pstate = 3) "
                     "where initiating_uid = $3 and nonce_1 = $4 and alt_name = $5 and pstate = 1");
            a.append(hfrom);
            a.append(nonce2);
            a.append(to_hex(My_UID));
            a.append(nonce);
            a.append(alt_name);
            SKID->query(&a);
            SKID->commit_transaction();
            return 1;
        }
        catch(...)
        {
            SKID->rollback_transaction();
            terminate(to_hex(My_UID), to_hex(from));
            return 0;
        }
    }
    catch (...)
    {
        terminate(to_hex(My_UID), to_hex(from));
        //SKID->rollback_transaction();
    }

    return 0;
}

// CALLED IN INITIATOR
int
install_group_key(vc from, vc msg, vc password)
{
    vc m = xfer_dec(msg, password);
    vc hfrom = to_hex(from);

    if(m.is_nil())
        return 0;

    vc alt_name = m[0];
    vc grp_key = m[1];
    int ret = DH_alternate::insert_new_key(alt_name, grp_key);
    terminate(to_hex(My_UID), to_hex(from));
    return ret;
}

// receive first request from join candidate

// CALLED IN RESPONDER
int
recv_gj1(vc from, vc msg, vc password)
{
    try
    {

        vc m = xfer_dec(msg, password);
        vc hfrom = to_hex(from);

        if(m.is_nil())
            return 0;

        vc nonce = m[0];
        vc alt_name = m[1];

        SKID->start_transaction();

        SKID->sql_simple("delete from pstate where initiating_uid = $1", hfrom);

        vc nonce2 = to_hex(get_entropy());

        vc mr(VC_VECTOR);
        mr[0] = alt_name;
        mr[1] = nonce2;
        mr[2] = nonce;
        mr[3] = hfrom;

        vc mrs = xfer_enc(mr, password);
        int comp_id = dwyco_make_special_zap_composition(DWYCO_SPECIAL_TYPE_JOIN2, 0, (const char *)mrs, mrs.len());
        if(comp_id == -1)
            throw -1;

        if(!post_req(comp_id, from))
        {
            dwyco_delete_zap_composition(comp_id);
            throw -1;
        }
        VCArglist a;
        a.append("insert into pstate (initiating_uid, responding_uid, nonce_1, nonce_2, alt_name, time, pstate) "
                         "values($1, $2, $3, $4, $5, strftime('%s', 'now'), 2)");
        a.append(hfrom);
        a.append(to_hex(My_UID));
        a.append(nonce);
        a.append(nonce2);
        a.append(alt_name);
        SKID->query(&a);
        SKID->commit_transaction();
        return 1;
    }
    catch (...)
    {
        SKID->rollback_transaction();
    }

    return 0;
}

// final part of SKID, if everything matches up, send them
// the private group key we have

// CALLED IN RESPONDER
int
recv_gj3(vc from, vc msg, vc password)
{
    try
    {

        vc m = xfer_dec(msg, password);
        vc hfrom = to_hex(from);

        if(m.is_nil())
        {
            throw -1;
        }

        vc alt_name = m[0];
        vc nonce = m[1];
        vc nonce2 = m[2];
        vc our_uid = m[3];

        // first checks
        if(to_hex(My_UID) != our_uid)
        {
            throw -1;
        }

        VCArglist a;
        a.append("select * from pstate where "
                 "initiating_uid = $1 and responding_uid = $2 and nonce_1 = $3 and "
                 "nonce_2 = $4 and alt_name = $5 and pstate = 2");
        a.append(hfrom);
        a.append(our_uid);
        a.append(nonce);
        a.append(nonce2);
        a.append(alt_name);

        vc res = SKID->query(&a);

        if(res.num_elems() == 0)
        {
            throw -1;
        }
        // note: for now, we are only in one group, and it should be the
        // same as the requester wanted. this ought to be fixed later
        // if we allow multiple groups for some reason
        if(Current_alternate->alt_name() != alt_name)
        {
            oopanic("protocol error");
            throw -1;
        }

        vc mr(VC_VECTOR);
        mr[0] = alt_name;
        mr[1] = Current_alternate->my_static();

        vc mrs = xfer_enc(mr, password);
        int comp_id = dwyco_make_special_zap_composition(DWYCO_SPECIAL_TYPE_JOIN4, 0, (const char *)mrs, mrs.len());
        if(comp_id == -1)
            throw -1;

        if(!post_req(comp_id, from))
        {
            dwyco_delete_zap_composition(comp_id);
            throw -1;
        }
    }
    catch (...)
    {

    }

    terminate(to_hex(from), to_hex(My_UID));

    return 0;
}



