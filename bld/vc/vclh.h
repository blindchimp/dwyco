
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCLH_H
#define VCLH_H
// note: these all used to be statics, but since we have
// decided to use this as a runtime, they need to be
// globals for now.

vc dosetdynamic(const vc&);
vc dofunmeta(const vc&);
vc do_exploded_funcall(const vc&, const vc&);
vc dotype(const vc&);
vc dotype2(VCArglist *a);
vc dotype3(const vc&);
vc doreturn(const vc&);
vc doloop(const vc&, const vc&, vc, const vc&);
vc dowhile(const vc&, const vc&);
vc dobreak(const vc&);
vc domod(const vc&, const vc&);
vc dodiv(const vc&, const vc&);
vc domul(const vc&, const vc&);
vc doadd(const vc&, const vc&);
vc doadd(const vc&, const vc&);
vc dosub(const vc&, const vc&);
vc dosub(const vc&, const vc&);
vc incrfun(const vc&);
vc decrfun(const vc&);
vc incrfun(const vc&);
vc decrfun(const vc&);
vc bindfun(const vc&, const vc&);
vc lbindfun(const vc&, const vc&);
vc gbindfun(const vc&, const vc&);
vc gremovefun(const vc&);
vc lremovefun(const vc&);
vc removefun(const vc&);
vc boundfun(const vc&);
vc lboundfun(const vc&);
vc gboundfun(const vc&);
vc gfindfun(const vc&);
vc lfindfun(const vc&);
vc oremovefun(const vc&);
vc obindfun(const vc&, const vc&);
vc oboundfun(const vc&);
vc ofindfun(const vc&);
vc eqfun(const vc&, const vc&);
vc nefun(const vc&, const vc&);
vc ltfun(const vc&, const vc&);
vc lefun(const vc&, const vc&);
vc gtfun(const vc&, const vc&);
vc gefun(const vc&, const vc&);
vc notfun(const vc&);
vc bnotfun(const vc&);
vc borfun(const vc&, const vc&);
vc bandfun(const vc&, const vc&);
vc bxorfun(const vc&, const vc&);
vc blsrfun(const vc&, const vc&);
vc blslfun(const vc&, const vc&);
vc basrfun(const vc&, const vc&);
vc baslfun(const vc&, const vc&);
vc doforeach(const vc&, const vc&, const vc&);
vc doappend(VCArglist *a);
vc doprepend(VCArglist *a);
vc doaddset(VCArglist *a);
vc doget(VCArglist *a);
vc doremset(VCArglist *a);
vc doremset2(VCArglist *a);
vc doremfirst(VCArglist *a);
vc doremfirst2(VCArglist *a);
vc doremlast(VCArglist *a);
vc docontains(const vc&, const vc&);
vc doempty(const vc&);
vc donumelems(const vc&);
vc doputindex(VCArglist *a);
vc dogetindex(VCArglist *a);
vc doputkv(VCArglist *a);
vc doexcdhandle(VCArglist *a);
vc doexcbackout(VCArglist *a);
vc doprint(VCArglist *a);
vc dofprint(VCArglist *a);
vc doreadline(VCArglist *a);
vc doreadline2(VCArglist *a);
vc doreadatoms(VCArglist *a);
vc docontents_of(VCArglist *a);
vc doflush();
vc dofgets(VCArglist *a);
vc dofputs(VCArglist *a);
vc dofread(VCArglist *a);
vc dogensym();
vc dostringrep(const vc&);
vc docopy(const vc&);
vc evalfun(const vc&);
vc vclh_strlen(const vc&);
vc vclh_substr(VCArglist *a);
vc vclh_tolower(const vc&);
vc vclh_strspn(const vc&, const vc&);
vc vclh_strcspn(const vc&, const vc&);
vc vclh_strstr(const vc&, const vc&);
vc doregex(const vc&);
vc doopenfile(const vc&, const vc&);
vc doclosefile(VCArglist *a);
vc doseekfile(VCArglist *a);
vc dodump();
vc doexit(const vc&);
vc vclh_fmt(const vc&, const vc&);
vc vclh_atol(const vc&);
vc vclh_to_hex(const vc&);
vc vclh_from_hex(const vc&);
vc doputfile(VCArglist *a);
vc dogetfile(VCArglist *a);
vc vclh_serialize(VCArglist *a);
vc vclh_deserialize(VCArglist *a);
vc vclh_uri_encode(const vc&);
vc vclh_uri_decode(const vc&);
vc dump_flat_profile();

vc dofundef(VCArglist *);
vc dolfundef(VCArglist *);
vc dogfundef(VCArglist *);
vc dospecial_fundef(VCArglist *);
vc dolambda(VCArglist *);
vc doif(VCArglist *);
vc docond(VCArglist *);
vc doswitch(VCArglist *);
vc docand(VCArglist *);
vc docor(VCArglist *);
vc orfun(VCArglist *);
vc andfun(VCArglist *);
vc xorfun(VCArglist *);
vc dolist(VCArglist *);
vc dovector(VCArglist *);
vc dovectorsize(VCArglist *);
vc dovector_from_strings(VCArglist *);
vc doset(VCArglist *);
vc doset_size(VCArglist *);
vc dobag(VCArglist *);
vc dobag_size(VCArglist *);
vc domap(VCArglist *);
vc create_tree(VCArglist *);
vc dostring(VCArglist *);
vc dotry(VCArglist *);
vc doexcraise(VCArglist *);
vc dovoid(VCArglist *);
vc doprog(VCArglist *);
vc doresplit(VCArglist *);
vc dorematch(VCArglist *);
vc dobacktrace(VCArglist *);
#endif
