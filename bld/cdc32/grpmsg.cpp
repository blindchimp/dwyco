
#include "simplesql.h"

using namespace dwyco;

struct skid_sql : public SimpleSql
{
    skid_sql() : SimpleSql("skid.sql") {}

    void init_schema() {
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
                   "responding_uid text collate nocase, "
                   "alt_name text collate nocase, "
                   "state integer"
                   "time integer"
                   ")");


    }


};

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


