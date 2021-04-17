
#include "simplesql.h"
#include "vc.h"
#include "dlli.h"
#include "qauth.h"
#include "profiledb.h"
#include "vccrypt2.h"
#include "ser.h"
#include "mcc.h"
#include "msend.h"
#include "dhgsetup.h"
#include "dwrtlog.h"
#include "grpmsg.h"
#include "se.h"

extern DwVec<ValidPtr> CompositionDeleteQ;

using namespace dwyco;

namespace dwyco {
ssns::signal1<vc> Join_signal;

struct skid_sql : public SimpleSql
{
    skid_sql() : SimpleSql("skid.sql") {}

    void init_schema(const DwString&) {
        start_transaction();
        sql_simple("drop table if exists keys");
        //sql_simple("create index if not exists keys_uid on keys(uid)");
        //sql_simple("create index if not exists keys_alt_name on keys(alt_name)");
        sql_simple("create table if not exists pstate ("
                   "initiating_uid text collate nocase not null,"
                   "responding_uid text collate nocase, "
                   "alt_name text collate nocase not null, "
                   "state integer not null,"
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
    if(SKID)
        return 1;
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

void
clear_gj()
{
    SKID->start_transaction();
    SKID->sql_simple("delete from pstate");
    SKID->commit_transaction();
    se_emit_group_status_change();
}

vc
get_status_gj()
{
    vc res = SKID->sql_simple("select alt_name, state, time from pstate order by time desc limit 1");
    if(res.num_elems() == 0)
        return vcnil;
    return res[0];
}

// the basic idea here is that we don't know who has the
// private key for the group. so we send a message to the
// group, and the first one that responds and completes the
// protocol, we use that private key, and ignore the rest of
// the responses. we *could* do something along the lines of
// letting all the responders finish the protocol and compare
// the results, possibly getting some better confidence in
// the key (since all the final keys should be the same),
// but i haven't bothered with that here.

/*
 * G is the group you want to join
 * P is the password for G (a pre-shared secret)
 * B is who wants to join
 * A is some member of G and can provide G's private key
 * Ek(m) is m encrypted with k = H(P) using GCM
 *
 * if any message fails to decrypt it is ignored.
 * if a message arrives that fails to match the protocol state, it is ignored.
 * the protocol only "terminates" on success, or after a timeout (error).
 * (the reason i'm making this distinction is that because of the underlying
 * transport, messages can occasionally be delivered more than once, and i don't
 * want to terminate the protocol based on that. i'm not sure this is a good idea
 * but it does allow more sequences of messages to result in the protocol
 * succeeding.
 *
B->A: create m1 = (rB, G) and send Ek(m) to G using only G's public key
A->B: A decrypts m1 using k, m2 = Ek(G, rA, rB, B)
B->A: B decrypts m2 using k, if rB != m1's rB fail. if G or B don't match, fail. m3 = Ek(G, rB, rA, A)
A receives and decrypts m3, checks all items match. if so, it sends G's private key to B in
a message that is encrypted using B's p2p public key.

*/

static
vc
xfer_enc(vc v, vc password)
{
    if(password.type() != VC_STRING)
        return vcnil;
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
    if(password.type() != VC_STRING)
        return vcnil;
    vc k = vclh_sha3_256(password);
    k = vc(VC_BSTRING, (const char *)k, 16);
    vc enc_ctx = vclh_encdec_open();
    vclh_encdec_init_key_ctx(enc_ctx, k, 0);
    vc ret;
    vc p = encdec_xfer_dec_ctx(enc_ctx, v, ret);
    if(p.is_nil())
        return vcnil;
    return ret;
}

static
int
post_req(int compid, vc vuid, DwString& pers_id, int no_group)
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
    // NOTE: we inhibit the default PK encryption here in order to
    // improve the chances that someone will be able to get the join
    // message and respond to the requests. the problem is that we may not
    // have a group key, especially early on before a client has a chance
    // to accumulate keys from other clients.
    // we might end up sending a message to the server encrypted with the
    // uid's pk, but with no group pk available, no other group member would
    // be able to decrypt the message. by just sending it with only the
    // password encryption, all the current group members can decrypt it.
    // note that message itself is encrypted with the password, not as
    // good as the pk stuff, but still reasonable. the underlying skid protocol
    // is supposedly resistant to attack even if it is done "in the clear", so
    // we are probably ok even if someone else can decrypt the messages.
    if(!send_via_server(m->qfn, 1, no_group, 1))
    {
        GRTLOG("post_req: send startup failed", 0, 0);
        return 0;
    }
    pers_id = m->qfn;
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

    SKID->sql_simple("delete from pstate where initiating_uid = ?1 and responding_uid = ?2",
                     initiator, responder);
    se_emit_group_status_change();

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
// CALLED IN INITIATOR, first state
int
start_gj(vc target_uid, vc gname, vc password)
{
    DwString pers_id;

    vc nonce = to_hex(get_entropy());

    // note: in order to get the group pk we need to have access to
    // at least one uid that is in the group, which needs to be fixed
    // eventually.
    vc pk;
    vc alt_pk;
    vc alt_name = gname;

#if 0
    if(!get_pk2(target_uid, pk, alt_pk, alt_name))
    {
        return 0;
    }
#endif

    vc m(VC_VECTOR);
    m[0] = nonce;
    m[1] = alt_name;

    vc mk = xfer_enc(m, password);

    int comp_id = dwyco_make_special_zap_composition(DWYCO_SPECIAL_TYPE_JOIN1, (const char *)mk, mk.len());
    if(comp_id == -1)
        return 0;

    // send to group
    if(!post_req(comp_id, target_uid, pers_id, 0))
    {
        dwyco_delete_zap_composition(comp_id);
        return 0;
    }
    try
    {

        SKID->sql_simple("insert into pstate (alt_name, nonce_1, initiating_uid, state, time) "
                         "values(?1, ?2, ?3, 1, strftime('%s', 'now'))",
                         alt_name,
                         nonce,
                         to_hex(My_UID)
                         );
        se_emit_group_status_change();
        return 1;
    }
    catch(...)
    {
        dwyco_delete_zap_composition(comp_id);
        dwyco_kill_message(pers_id.c_str(), pers_id.length());
    }
    return 0;
}

// CALLED IN INITIATOR
// note: the initial send was a broadcast, so we end up with
// N replies. this function also essentially is broadcasting to
// each of the responders (that is what the server does by default, even
// if you are just sending to a single uid.)
// so we end up in N*N messages being processed, which is a waste, especially
// since most of the messages will be dropped because they have the wrong
// nonces in them. we need a "store" that doesn't perform the group mapping,
// essentially making it "point to point" instead of the broadcast.
//
int
recv_gj2(vc from, vc msg, vc password)
{
    DwString pers_id;
    vc hfrom = to_hex(from);
    int rollback = 0;
    try
    {

        vc m = xfer_dec(msg, password);

        if(m.is_nil())
        {
            return 0;
        }

        vc alt_name = m[0];
        vc nonce2 = m[1];
        vc nonce = m[2];
        vc our_uid = m[3];

        // first checks
        // if it wasn't for us directly, or we somehow get it
        // looped to ourselves, ignore it.
        if(to_hex(My_UID) != our_uid || from == My_UID)
        {
            return 0;
        }

        SKID->start_transaction();
        rollback = 1;
        {
        vc res = SKID->sql_simple("select 1 from pstate where "
                                  "initiating_uid = ?1 and nonce_1 = ?2 and "
                                  "alt_name = ?3 and state = 1",
                                  our_uid, nonce, alt_name);
        if(res.num_elems() == 0)
        {
            throw -1;
        }
        }
        // there is at least a chance they are in the group, because they
        // got the password and nonce right, and it isn't a problem if
        // they turn out not to be in the group... if they don't have the key
        // no sync channel is set up anyway.
        Group_uids.add(from);

        vc mr(VC_VECTOR);
        mr[0] = alt_name;
        mr[1] = nonce;
        mr[2] = nonce2;
        mr[3] = hfrom;

        vc mrs = xfer_enc(mr, password);
        int comp_id = dwyco_make_special_zap_composition(DWYCO_SPECIAL_TYPE_JOIN3, (const char *)mrs, mrs.len());
        if(comp_id == -1)
            throw -1;

        // respond to just the sender (not group)
        if(!post_req(comp_id, from, pers_id, 1))
        {
            dwyco_delete_zap_composition(comp_id);
            throw -1;
        }

        SKID->sql_simple("update pstate set responding_uid = ?1,  nonce_2 = ?2, time = strftime('%s', 'now'), state = 3 "
                         "where initiating_uid = ?3 and nonce_1 = ?4 and alt_name = ?5 and state = 1",
                         hfrom,
                         nonce2,
                         to_hex(My_UID),
                         nonce,
                         alt_name);

        SKID->commit_transaction();
        se_emit_group_status_change();
        return 1;
    }
    catch (...)
    {
        if(rollback)
            SKID->rollback_transaction();
        se_emit_group_status_change();
        // don't terminate, in cast there are duplicate messages, we can just ignore
        // the follow ons.
        //terminate(to_hex(My_UID), hfrom);
        dwyco_kill_message(pers_id.c_str(), pers_id.length());

    }

    return 0;
}

// CALLED IN INITIATOR, final state
int
install_group_key(vc from, vc msg, vc password)
{
    if(from == My_UID)
        return 0;
    vc m = xfer_dec(msg, password);

    if(m.is_nil())
        return 0;
    vc our_uid = to_hex(My_UID);
    vc hfrom = to_hex(from);
    vc alt_name = m[0];
    vc grp_key = m[1];
    vc nonce1 = m[2];
    vc nonce2 = m[3];

    vc res = SKID->sql_simple("select 1 from pstate where "
                              "initiating_uid = ?1 and nonce_1 = ?2 and nonce_2 = ?3 and "
                                "responding_uid = ?4 and "
                              "alt_name = ?5 and state = 3",
                              our_uid, nonce1, nonce2, hfrom, alt_name);
    if(res.num_elems() == 0)
    {
        se_emit_group_status_change();
        terminate(our_uid, hfrom);
        return 0;
    }
    // note: some basic checks should be dont to make sure the
    // private key matches the public key. we got here presumably
    // after we got the public key from the server (since we
    // didn't generate ourselves). the server should have signed it.
    // if the private key doesn't match for some reason, it probably
    // isn't a security problem,  nothing else will work properly.
    int ret = DH_alternate::insert_private_key(alt_name, grp_key);
    // once it completes, just kill everything. this is not what you
    // want to do if you are expecting to run this protocol on multiple
    // groups at the same time, but we don't want to do that anyway.
    clear_gj();
    Group_uids.add(from);
    se_emit_join(alt_name, 1);
    se_emit_group_status_change();
    Join_signal.emit(alt_name);
    return ret;
}

// receive first request from join candidate

// CALLED IN RESPONDER
int
recv_gj1(vc from, vc msg, vc password)
{
    DwString pers_id;
    vc hfrom = to_hex(from);
    if(from == My_UID)
        return 0;
    try
    {
        vc m = xfer_dec(msg, password);

        if(m.is_nil())
            return 0;

        vc nonce = m[0];
        vc alt_name = m[1];

        SKID->start_transaction();

        // yea, this is a problem... if we process duplicate messages
        // only the *last* one we get will be retained if we delete the previous state.
        // in that case, if the initiator sees multiple join2 messages, it only
        // responds to the *first* one it got (throwing away duplicates)
        //SKID->sql_simple("delete from pstate where initiating_uid = ?1", hfrom);
        // if we process multiple duplicate messages, only one of them will
        // cause the initiator's state machine to advance, and the rest will be
        // ignored.

        vc nonce2 = to_hex(get_entropy());

        vc mr(VC_VECTOR);
        mr[0] = alt_name;
        mr[1] = nonce2;
        mr[2] = nonce;
        mr[3] = hfrom;

        Group_uids.add(from);

        vc mrs = xfer_enc(mr, password);
        int comp_id = dwyco_make_special_zap_composition(DWYCO_SPECIAL_TYPE_JOIN2, (const char *)mrs, mrs.len());
        if(comp_id == -1)
            throw -1;
        // respond to just the initiator (not any group they might accidently be in)
        if(!post_req(comp_id, from, pers_id, 1))
        {
            dwyco_delete_zap_composition(comp_id);
            throw -1;
        }
        SKID->sql_simple("insert into pstate (initiating_uid, responding_uid, nonce_1, nonce_2, alt_name, time, state) "
                         "values(?1, ?2, ?3, ?4, ?5, strftime('%s', 'now'), 2)",
                         hfrom,
                         to_hex(My_UID),
                         nonce,
                         nonce2,
                         alt_name);
        SKID->commit_transaction();
        return 1;
    }
    catch (...)
    {
        SKID->rollback_transaction();
        dwyco_kill_message(pers_id.c_str(), pers_id.length());
    }

    return 0;
}

// final part of SKID, if everything matches up, send them
// the private group key we have.
// note: since this is the last state, it makes sense to terminate
// the protocol if there is an error, ignoring duplicate messages
// results in the same actions (ie, the first one that succeeds wins.)

// CALLED IN RESPONDER
int
recv_gj3(vc from, vc msg, vc password)
{
    DwString pers_id;
    vc hfrom = to_hex(from);
    int ret = 0;

    if(from == My_UID)
        return 0;
    try
    {
        // if we're not currently in a group, don't even try to send a key back
        if(!Current_alternate)
            throw -1;
        vc m = xfer_dec(msg, password);

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

        {
            vc res = SKID->sql_simple("select 1 from pstate where "
                                      "initiating_uid = ?1 and responding_uid = ?2 and nonce_1 = ?3 and "
                                      "nonce_2 = ?4 and alt_name = ?5 and state = 2",
                                      hfrom,
                                      our_uid,
                                      nonce,
                                      nonce2,
                                      alt_name);

            if(res.num_elems() == 0)
            {
                throw -1;
            }
        }

        // note: for now, we are only in one group, and it should be the
        // same as the requester wanted. this ought to be fixed later
        // if we allow multiple groups for some reason
        if(Current_alternate->alt_name() != alt_name)
        {
            //oopanic("protocol error");
            throw -1;
        }
        Group_uids.add(from);
        vc mr(VC_VECTOR);
        mr[0] = alt_name;
        mr[1] = Current_alternate->my_static();
        mr[2] = nonce;
        mr[3] = nonce2;

        vc mrs = xfer_enc(mr, password);
        int comp_id = dwyco_make_special_zap_composition(DWYCO_SPECIAL_TYPE_JOIN4, (const char *)mrs, mrs.len());
        if(comp_id == -1)
            throw -1;
        // respond to just the initiator (not any group they might accidently be in)
        if(!post_req(comp_id, from, pers_id, 1))
        {
            dwyco_delete_zap_composition(comp_id);
            throw -1;
        }
        ret = 1;
    }
    catch (...)
    {

    }

    terminate(hfrom, to_hex(My_UID));

    return ret;
}

}



