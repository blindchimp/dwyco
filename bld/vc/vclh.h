
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

vc dosetdynamic(vc);
vc dofunmeta(vc);
vc do_exploded_funcall(vc, vc);
vc dotype(vc);
vc dotype2(vc);
vc dotype3(vc);
vc doreturn(vc);
vc doloop(vc, vc, vc, vc);
vc dowhile(vc, vc);
vc dobreak(vc);
vc domod(vc, vc);
vc dodiv(vc, vc);
vc domul(vc, vc);
vc doadd(vc, vc);
vc doadd(vc, vc);
vc dosub(vc, vc);
vc dosub(vc, vc);
vc incrfun(vc);
vc decrfun(vc);
vc incrfun(vc);
vc decrfun(vc);
vc bindfun(vc, vc);
vc lbindfun(vc, vc);
vc gbindfun(vc, vc);
vc gremovefun(vc);
vc lremovefun(vc);
vc removefun(vc);
vc boundfun(vc);
vc lboundfun(vc);
vc gboundfun(vc);
vc gfindfun(vc);
vc lfindfun(vc);
vc oremovefun(vc);
vc obindfun(vc, vc);
vc oboundfun(vc);
vc ofindfun(vc);
vc eqfun(vc, vc);
vc nefun(vc, vc);
vc ltfun(vc, vc);
vc lefun(vc, vc);
vc gtfun(vc, vc);
vc gefun(vc, vc);
vc notfun(vc);
vc bnotfun(vc);
vc borfun(vc, vc);
vc bandfun(vc, vc);
vc bxorfun(vc, vc);
vc blsrfun(vc, vc);
vc blslfun(vc, vc);
vc basrfun(vc, vc);
vc baslfun(vc, vc);
vc doforeach(vc, vc, vc);
vc doappend(vc, vc);
vc doprepend(vc, vc);
vc doaddset(vc, vc);
vc doget(vc, vc);
vc doremset(vc, vc);
vc doremset2(vc, vc);
vc doremfirst(vc);
vc doremfirst2(vc);
vc doremlast(vc);
vc docontains(vc, vc);
vc doempty(vc);
vc donumelems(vc);
vc doputindex(vc, vc, vc);
vc dogetindex(vc, vc);
vc doputkv(vc, vc, vc);
vc doexcdhandle(vc, vc);
vc dosethandlerret(vc);
vc doexcbackout(vc);
vc doprint(vc);
vc dofprint(vc, vc);
vc doreadline(vc);
vc doreadline2(vc);
vc doreadatoms(vc);
vc docontents_of(vc);
vc doflush();
vc dofgets(vc);
vc dofputs(vc, vc);
vc dofread(vc, vc);
vc dogensym();
vc dostringrep(vc);
vc docopy(vc);
vc evalfun(vc);
vc vclh_strlen(vc);
vc vclh_substr(vc, vc, vc);
vc vclh_tolower(vc);
vc vclh_strspn(vc, vc);
vc vclh_strcspn(vc, vc);
vc vclh_strstr(vc, vc);
vc doregex(vc);
vc doopenfile(vc, vc);
vc doclosefile(vc);
vc doseekfile(vc, vc, vc);
vc dodump();
vc doexit(vc);
vc vclh_fmt(vc, vc);
vc vclh_atol(vc);
vc vclh_to_hex(vc);
vc vclh_from_hex(vc);
vc doputfile(vc, vc);
vc dogetfile(vc, vc);
vc vclh_serialize(vc);
vc vclh_deserialize(vc, vc);
vc vclh_uri_encode(vc);
vc vclh_uri_decode(vc);
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
vc doexchandle(VCArglist *);
vc doexchandle2(VCArglist *);
vc dotry(VCArglist *);
vc doexcraise(VCArglist *);
vc dovoid(VCArglist *);
vc doprog(VCArglist *);
vc doresplit(VCArglist *);
vc dorematch(VCArglist *);
vc dobacktrace(VCArglist *);
#endif
