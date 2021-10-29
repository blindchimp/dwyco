#ifndef PROFILEDB_H
#define PROFILEDB_H
#include "vc.h"

namespace dwyco {
void init_prfdb();
void exit_prfdb();

int get_pk(vc uid, vc& sfpk_out);
int get_pk2(vc uid, vc& sfpk_out, vc& alt_sfpk_out, vc &alt_name);
int put_pk(vc uid, vc prf, vc sig);
int put_pk2(vc uid, vc sfpk, vc sig, vc alt_pk, vc server_sig, vc gname);
//void clean_pk_cache(int days_old, int max_left);

void pk_force_check(vc uid);
void pk_invalidate(vc uid);
void pk_set_session_cache(vc uid);
int pk_session_cached(vc uid);

#define PKC_STATIC_PUBLIC 0
#define PKC_DWYCO_SIGNATURE 1
#define PKC_ALT_STATIC_PUBLIC 2
#define PKC_ALT_SERVER_SIG 3
#define PKC_ALT_GNAME 4

int load_profile(vc uid, vc& prf_out);
int save_profile(vc uid, vc prf);
//void clean_profile_cache(int days_old, int max_left);
int prf_already_cached(vc uid);
void prf_force_check(vc uid);
void prf_invalidate(vc uid);
void prf_set_cached(vc uid);

#define PRF_PACK 0
#define PRF_MEDIA 1
#define PRF_CHKSUM 2
#define PRF_REVIEWED 3
#define PRF_REGULAR 4
#define PRF_IMAGE 5

}

#endif // PROFILEDB_H
