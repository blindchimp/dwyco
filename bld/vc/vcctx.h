
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCCTX_H
#define VCCTX_H
// $Header: g:/dwight/repo/vc/rcs/vcctx.h 1.46 1997/10/05 17:27:06 dwight Stable $

#include "vc.h"
#include "dwvecp.h"
#include "vcfunctx.h"
class vc_object;

class vcctx
{
private:
	DwVecP<functx> maps;
	int ctx;
    functx *cur_ctx;

public:
	vcctx();
	~vcctx();
	
	int contains(const vc&);
	void add(const vc& k, const vc& val);
	vc get(const vc& key) const;
	vc get2(const vc& key, vc*& wp) const;
    void remove(const vc& key) const;
	int find(const vc& k, vc& out);

	void global_add(const vc& k, const vc& val) ;
	int global_contains(const vc& k) ;
	void global_remove(const vc& v) ;
	vc global_find(const vc& v);

	void local_add(const vc& k, const vc& val) ;
	int local_contains(const vc& v) ;
	void local_remove(const vc& k) ;
	vc local_find(const vc& v);

#ifdef LHOBJ
	void obj_bind(const vc& mem, const vc& val);
	int obj_contains(const vc& mem);
	void obj_remove(const vc& mem);
	vc obj_find(const vc& mem);
	void open_obj_ctx(vc_object *);
	void close_obj_ctx();
	void set_base_init(int);
	int is_base_init();
#endif
	
	void open_ctx(functx * = 0) ;
	void close_ctx();

	void dump(VcIO) const;
};

#endif
