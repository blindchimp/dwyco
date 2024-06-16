
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vcdecom.h"
#include "vcmap.h"
#include "vcxstrm.h"
#include "vcenco.h"
#include "vcint.h"
#include "vcdbl.h"
#ifdef _Windows
#include <windows.h>
#endif
#ifdef DWYCO_VC_THREADED
#include <mutex>
std::mutex SN_mutex;
#endif

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcdecom.cpp 1.50 1998/12/09 05:12:20 dwight Exp $";

long vc_decomposable::SN;	// serial number for id's of decomposables

static vc bogus;
vc_decomposable::vc_decomposable()
{
    quoted = 1;
#ifdef DWYCO_VC_THREADED
    SN_mutex.lock();
#endif
	serial_number = SN++;
#ifdef DWYCO_VC_THREADED
    SN_mutex.unlock();
#endif
	iterators = 0;
}

void
vc_decomposable::bomb_op() const
{
	USER_BOMB2("can't do arithmetic on sets");
}

#define bomb(op, msg) \
vc \
vc_decomposable::operator op (const vc& ) const \
{ \
	USER_BOMB("unimplemented aggregate op: " msg, vcone); \
}

bomb(+, "addition")
bomb(-, "subtraction")
bomb(*, "multiplication")
bomb(/, "division")
bomb(%, "modulo")

#undef bomb

void
vc_list_set::bomb_rem_list()
{
	USER_BOMB2("can't remove from list");
}

void
vc_bag::bomb_rem() 
{
	USER_BOMB2("can't remove from bag");
}

void
vc_set::bomb_rem()
{
	USER_BOMB2("can't remove from set");
}

void
vc_map::bomb_rem_map()
{
	USER_BOMB2("can't use remove op on map");
}

void
vc_tree::bomb_rem_tree()
{
	USER_BOMB2("can't use remove op on tree");
}

vc&
vc_vector::operator[](const vc& v)
{
	if(v.type() != VC_INT)
		USER_BOMB("attempt to index vector with non-integer", bogus);
	return vec[v];
}

vc&
vc_vector::operator[](int i)
{
	return vec[i];
}

vc&
vc_vector::operator[](long i)
{
	return vec[i];
}

const vc&
vc_vector::operator[](const vc& v) const
{
	if(v.type() != VC_INT)
		USER_BOMB("attempt to index vector with non-integer", bogus);
	return vec[v];
}

const vc&
vc_vector::operator[](int i) const
{
	return vec[i];
}

const vc&
vc_vector::operator[](long i) const
{
	return vec[i];
}

int
vc_vector::member_select(const vc& member, vc& out, int , vc_object *)
{
	if(member.type() == VC_INT)
	{
		out = vec[(long)member];
		return 1;
	}
	USER_BOMB("unimplemented member select on vector (only integers supported)", 0);
}

int
vc_vector::contains(const vc& v) const
{
	return vec.index(v) != -1;
}

int
vc_vector::find(const vc& v, vc& out)
{
	long i = vec.index(v);
	if(i == -1)
		return 0;
	out = vec[i];
	return 1;
}

// UGH FIX THIS
// this probably should be "set"-like add, instead of
// append (since if append is what you want, then you should
// use that operator). BUT this has been like this for a long
// time and i would have to ferret out all the places it was
// used before changing it.
void
vc_vector::add(const vc& v)
{
	vec.append(v);
}

int
vc_vector::del(const vc& v)
{
	long i = vec.index(v);
	if(i == -1)
		return 0;
	if(iterators)
	{
		user_warning("vector del while foreach active");
	}
	vec.del(i);
	return 1;
}
	
void
vc_vector::append(const vc& v)
{
	if(iterators)
	{
		user_warning("vector append while foreach active");
	}
	vec.append(v);
}

vc
vc_vector::remove_last()
{
	long cnt = vec.num_elems();
	if(cnt == 0)
		USER_BOMB("can't remove last from empty vector", vcnil);
	vc tmp = vec[cnt - 1];
	if(iterators)
	{
		user_warning("vector remove-last while foreach active");
	}
	vec.del(cnt - 1);
	return tmp;
}

void
vc_vector::prepend(const vc& v)
{
	if(iterators)
	{
		user_warning("vector prepend while foreach active");
	}
	vec.insert(v, 0);
}

vc
vc_vector::remove_first()
{
	if(vec.num_elems() == 0)
		USER_BOMB("can't remove first from empty vector", vcnil);
	vc tmp = vec[0];
	if(iterators)
	{
		user_warning("vector remove-first while foreach active");
	}
	vec.del(0);
	return tmp;
}
	
void
vc_vector::remove(const vc& strt, const vc& len)
{
	if(iterators)
	{
		user_warning("vector remove while foreach active");
	}
	vec.del(strt, len);
}

void
vc_vector::remove(const vc& strt, long len)
{
	if(iterators)
	{
		user_warning("vector remove while foreach active");
	}
	vec.del(strt, len);
}

void
vc_vector::remove(long strt, const vc& len)
{
	if(iterators)
	{
		user_warning("vector remove while foreach active");
	}
	vec.del(strt, len);
}

void
vc_vector::remove(long strt, long len)
{
	if(iterators)
	{
		user_warning("vector remove while foreach active");
	}
	vec.del(strt, len);
}


void
vc_vector::insert(const vc& item, const vc& idx)
{
	if(iterators)
	{
		user_warning("vector insert while foreach active");
	}
	vec.insert(item, idx);
}

void
vc_vector::insert(const vc& item, long idx)
{
	if(iterators)
	{
		user_warning("vector insert while foreach active");
	}
	vec.insert(item, idx);
}

void
vc_vector::printOn(VcIO os) 
{
		os << "vector(";
        int cnt = vec.num_elems();
		for(long i = 0; i < cnt; ++i)
		{
			vec[i].printOn(os);
			os << " ";
		}
		os << ")\n";
}

void
vc_vector::stringrep(VcIO o) const
{
	o << "vector(";
    int cnt = vec.num_elems();
	for(long i = 0; i < cnt; ++i)
	{
		vec[i].stringrep(o);
		o << " ";
	}
	o << ")";
}

void
vc_vector::foreach(const vc& v, const vc& expr) const
{
	Vcmap->open_loop();
	VcVecIter i(&vec);
	++iterators;
	for(;!i.eol();i.forward())
	{
		Vcmap->local_add(v, i.get());
		expr.eval();
		if(Vcmap->unwind_in_progress())
			break;
	}
	--iterators;
	Vcmap->close_loop();
}

void
vc_vector::foreach(const vc& v, VC_FOREACH_CALLBACK cb) const
{
	++iterators;
	VcVecIter i(&vec);
	for(;!i.eol();i.forward())
	{
		(*cb)(i.get());
	}
	--iterators;
}

void
vc_vector::foreach(const vc& v, VC_FOREACH_CALLBACK2 cb) const
{
    ++iterators;
    VcVecIter i(&vec);
    for(;!i.eol();i.forward())
    {
        (*cb)(v, i.get());
    }
    --iterators;
}

#if 0
		vec[i] = v_vec[i] op (const vc&)v_scal; \

		vec[i] = (v_vec[i]).operator op (v_scal); \
// note: braindamaged use of explicit tmps is
// to satisfy cfront's stupid conversion problems.
//
#endif
#define scal(name, op) \
void \
vc_vector::scal1_##name(const vc_vector& v_vec, const vc_int& v_scal) \
{ \
	long num = vec.num_elems(); \
	vc vci; \
	vci.attach(&(vc_int&)v_scal); \
	for(long i = 0; i < num; ++i) \
	{\
		const vc& v = v_vec[i]; \
		vec[i] = v op vci; \
		CHECK_DBG_BO_LOOP; \
	} \
} \
void \
vc_vector::scal1_##name(const vc_vector& v_vec, const vc_double& v_scal) \
{ \
	long num = vec.num_elems(); \
	vc vcd; \
	vcd.attach(&(vc_double&)v_scal); \
	for(long i = 0; i < num; ++i) \
	{\
		const vc& v = v_vec[i]; \
		vec[i] = v op vcd; \
		CHECK_DBG_BO_LOOP; \
	} \
} \
void \
vc_vector::scal2_int_##name(const vc& v_scal, const vc_vector& v_vec) \
{ \
	long num = vec.num_elems(); \
	for(long i = 0; i < num; ++i) \
	{\
		const vc& v = v_vec[i]; \
		vec[i] = (const vc_int&)v_scal op v; \
		CHECK_DBG_BO_LOOP; \
	} \
} \
void \
vc_vector::scal2_double_##name(const vc& v_scal, const vc_vector& v_vec) \
{ \
	long num = vec.num_elems(); \
	for(long i = 0; i < num; ++i) \
	{\
		const vc& v = v_vec[i]; \
		vec[i] = (const vc_double&)v_scal op v; \
		CHECK_DBG_BO_LOOP; \
	} \
} \
vc \
vc_vector::operator op(const vc& v) const \
{ \
	return v.vec_##name(*this); \
} \
\
vc \
vc_vector::vec_##name(const vc& v)  const \
{ \
	long len = dwmin(((const vc_vector&)v).vec.num_elems(), vec.num_elems()); \
	vc retval; \
	vc_vector *v1 = new vc_vector(len); \
\
	for(long i = 0; i < len; ++i) \
	{ \
		(*v1)[i] = ((const vc_vector&)v)[i] op (*this)[i]; \
		CHECK_DBG_BO_LOOP; \
	} \
	retval.redefine(v1); \
	return retval; \
} \
\
vc \
vc_vector::int_##name(const vc& v) const \
{\
	vc_vector *v1 = new vc_vector(vec.num_elems());\
	v1->scal2_int_##name(v, *this);\
	vc retval;   \
	retval.redefine(v1); \
	return retval; \
} \
vc \
vc_vector::double_##name(const vc& v) const \
{\
	vc_vector *v1 = new vc_vector(vec.num_elems());\
	v1->scal2_double_##name(v, *this);\
	vc retval;   \
	retval.redefine(v1); \
	return retval; \
}

#if 0
// note: no need to differentiate by type
// at the moment, so all scalar calls go through
// one routine.
	vc_vector *v1 = new vc_vector(vec.num_elems());\
	v1->scal2_##name(v, *this);\
	vc retval;   \
	retval.redefine(v1); \
	return retval;
}
#endif

scal(add, +)
scal(sub, -)
scal(mul, *)
scal(div, /)
scal(mod, %)

#undef scal

int
vc_decomposable::encode_numelems(vcxstream& vcx)
{
	char buf[40];
	long nelems = num_elems();
	int len = encode_long(buf, nelems);
    char *cp;
	if((cp = vcx.out_want(len)) == 0)
		return -1;
	memcpy(cp, buf, len);
	return len;
}


long
vc_decomposable::xfer_out(vcxstream& vcx)
{
	long total = encode_numelems(vcx);
	if(total < 0)
		return -1;
	long etotal = encode_elems(vcx);
	if(etotal < 0)
		return -1;
    return total + etotal;
}

int
vc_decomposable::decode_numelems(vcxstream& vcx, long& elems_out)
{
	char *cp;

	if((cp = vcx.in_want(ENCODED_LEN_LEN)) == 0)
		return EXIN_DEV;
	int len = decode_len(cp);
    if(len == -1 || len == 0 || len > vcx.max_count_digits)
		return EXIN_PARSE;
    // don't t think this makes sense any more
    //if(len > vcx.max_element_len)
    //    return EXIN_PARSE;
	if((cp = vcx.in_want(len)) == 0)
		return EXIN_DEV;
	long l = decode_long(cp, len);
	if(l == -1)
		return EXIN_PARSE;
    if(l > vcx.max_elements)
    {
        user_warning("xfer_in hit max_elements");
        return EXIN_PARSE;
    }
	elems_out = l;
    return len + ENCODED_LEN_LEN;
}

long
vc_decomposable::xfer_in(vcxstream& vcx)
{
	long nelems;
	int total = decode_numelems(vcx, nelems);
	if(total < 0)
		return total;
	if(nelems < 0)
		return EXIN_PARSE;
	long etotal = decode_elems(vcx, nelems);
	if(etotal < 0)
		return etotal;
    return total + etotal;
}

long
vc_vector::encode_elems(vcxstream& vcx)
{
	long total = 0;
	long nelems = vec.num_elems();
	for(long i = 0; i < nelems; ++i)
	{
    	long esize;
		if((esize = vec[i].xfer_out(vcx)) < 0)
			return -1;
		else
			total += esize;
	}
	return total;
}

long
vc_vector::decode_elems(vcxstream& vcx, long nelems)
{
	vec.set_size(nelems);
	long total = 0;
	for(long i = 0; i < nelems; ++i)
	{
		long esize;
		if((esize = vec[i].xfer_in(vcx)) < 0)
			return esize;
		else
			total += esize;
	}
	return total;
}

// mapping operators

vc&
vc_map::operator[](const vc& v)
{
	oopanic("unimp map op");
//	return map[v];
	return bogus;
}

vc&
vc_map::operator[](int i)
{
	oopanic("unimp map op");
//	return map[vc(i)];
	return bogus;
}

vc&
vc_map::operator[](long i)
{
	oopanic("unimp map op");
//	return map[vc(i)];
	return bogus;
}

int
vc_map::contains(const vc& v) const
{
	return map.contains(v);
}

int
vc_map::find(const vc& v, vc& out)
{
	return map.find(v, out);
}

void
vc_map::add(const vc& k)
{
	if(iterators)
	{
		user_warning("map add while foreach active");
	}
	map.replace(k, vcnil);
}

void
vc_map::add_kv(const vc& k, const vc& v)
{
	if(iterators)
	{
		user_warning("map add_kv while foreach active");
	}
	map.replace(k, v);
}

int
vc_map::del(const vc& v)
{
	if(iterators)
	{
		user_warning("map del while foreach active");
	}
	return map.del(v);
}
	
void
vc_map::append(const vc& v)
{
	add(v);
}

vc
vc_map::remove_last()
{
	USER_BOMB("can't remove last from mapping", vcnil);
}

void
vc_map::prepend(const vc& v)
{
	add(v);
}

vc
vc_map::remove_first()
{
	USER_BOMB("can't remove first from mapping", vcnil);
}
	
void
vc_map::remove(const vc& , const vc& )
{
	bomb_rem_map();
}

void
vc_map::remove(const vc& , long )
{
	bomb_rem_map();
}

void
vc_map::remove(long , const vc& )
{
	bomb_rem_map();
}

void
vc_map::remove(long , long )
{
	bomb_rem_map();
}

void
vc_map::insert(const vc& , const vc& )
{
	USER_BOMB2("can't insert into map");
	//map[idx] = item;
}

void
vc_map::insert(const vc& , long )
{
	USER_BOMB2("can't insert into map");
	//map[vc(idx)] = item;
}

void
vc_map::stringrep(VcIO o) const
{

	o << "map(";
	VcMapIter i(&map);
	vc assoc(VC_VECTOR);
	for(; !i.eol(); i.forward())
	{
		// break out Dwassoc into a vector
		DwAssocImp<vc,vc> a = i.get();
		assoc[0] = a.get_key();
        assoc[1] = a.get_value();
		assoc.stringrep(o);
		o << " ";
	}
	o << ")";
}


void
vc_map::foreach(const vc& v, const vc& expr) const
{
	Vcmap->open_loop();
	VcMapIter i(&map);
	++iterators;
	for(; !i.eol(); i.forward())
	{
		// break out Dwassoc into a vector
		DwAssocImp<vc,vc> a = i.get();
		vc assoc(VC_VECTOR);
		Vcmap->local_add(v, assoc);
		assoc[0] = a.get_key();
        assoc[1] = a.get_value();
		expr.eval();
		if(Vcmap->unwind_in_progress())
			break;
	}
	--iterators;
	Vcmap->close_loop();
}

void
vc_map::foreach(const vc& v, VC_FOREACH_CALLBACK cb) const
{
	VcMapIter i(&map);
	++iterators;
	for(; !i.eol(); i.forward())
	{
		// break out Dwassoc into a vector
		DwAssocImp<vc,vc> a = i.get();
		vc assoc(VC_VECTOR);
		assoc[0] = a.get_key();
		assoc[1] = a.get_value();
		(*cb)(assoc);
	}
	--iterators;
}

void
vc_map::foreach(const vc& v, VC_FOREACH_CALLBACK2 cb) const
{
    VcMapIter i(&map);
    ++iterators;
    for(; !i.eol(); i.forward())
    {
        // break out Dwassoc into a vector
        DwAssocImp<vc,vc> a = i.get();
        vc assoc(VC_VECTOR);
        assoc[0] = a.get_key();
        assoc[1] = a.get_value();
        (*cb)(v, assoc);
    }
    --iterators;
}

long
vc_map::encode_elems(vcxstream& vcx)
{
	VcMapIter i(&map);
	long total = 0;
	for(; !i.eol(); i.forward())
	{
		long s1, s2;
		// break out Dwassoc into a vector
		DwAssocImp<vc,vc> a = i.get();
		if((s1 = a.get_key().xfer_out(vcx)) < 0 || 
			(s2 = a.get_value().xfer_out(vcx)) < 0)
				return -1;
        total += s1 + s2;
	}
    return total;
 	
}

long
vc_map::decode_elems(vcxstream& vcx, long nelems)
{
	long total = 0;
	vc key;
	vc value;
    int ts;
    if(nelems == 0)
        ts = 7;
    else if(nelems > 500)
        ts = 499;
    else
        ts = nelems;

    map.set_size(ts);

	for(long i = 0; i < nelems; ++i)
	{
		long s1 = key.xfer_in(vcx);
		if(s1 < 0)
			return s1;
		long s2 = value.xfer_in(vcx);
		if(s2 < 0)
			return s2;
		add_kv(key, value);
		total += s1 + s2;
	}
    return total;

}

void
vc_map::printOn(VcIO outputStream) 
{
    outputStream << "map(\n";
    DwAssocImp<vc,vc> a;
    VcMapIter i(&map);

    for(;!i.eol();i.forward())
    {
        a = i.get();
        outputStream << "vector(";
        a.get_key().printOn(outputStream);
        outputStream << " ";
        a.get_value().printOn(outputStream);
        outputStream << ")\n";
    }
    outputStream << ")\n";
}

// list operators

vc&
vc_list_set::operator[](const vc& v)
{
	USER_BOMB("can't index a list", bogus);
}

vc&
vc_list_set::operator[](int i)
{
	USER_BOMB("can't index a list", bogus);
}

vc&
vc_list_set::operator[](long i)
{
	USER_BOMB("can't index a list", bogus);
}

int
vc_list_set::contains(const vc& v) const
{
	return list.exists(v);
}

int
vc_list_set::find(const vc& v, vc& out)
{
	return list.exists(v, out);
}

// MAYBE FIX THIS
// SEE COMMENT ABOVE FOR VECTOR "add"
void
vc_list_set::add(const vc& v)
{
	if(iterators)
	{
		user_warning("list add while foreach active");
	}
	list.append(v);
}

int
vc_list_set::del(const vc& v)
{
	if(list.exists(v))
    {
		if(iterators)
		{
			user_warning("list remove while foreach active");
		}
		list.remove();
		return 1;
	}
    return 0;
}
	
void
vc_list_set::append(const vc& v)
{
	if(iterators)
	{
		user_warning("list append while foreach active");
	}
	list.append(v);
}

vc
vc_list_set::remove_last()
{
	vc tmp = list.get_last();
	if(iterators)
	{
		user_warning("list remove-last while foreach active");
	}
	list.remove_last();
    return tmp;
}

void
vc_list_set::prepend(const vc& v)
{
	if(iterators)
	{
		user_warning("list prepend while foreach active");
	}
	list.prepend(v);
}

vc
vc_list_set::remove_first()
{
	vc tmp = list.get_first();
	if(iterators)
	{
		user_warning("list remove-first while foreach active");
	}
	list.remove_first();
    return tmp;
}
	
void
vc_list_set::remove(const vc& strt, const vc& len)
{
	bomb_rem_list();
}

void
vc_list_set::remove(const vc& strt, long len)
{
	bomb_rem_list();
}

void
vc_list_set::remove(long strt, const vc& len)
{
	bomb_rem_list();
}

void
vc_list_set::remove(long strt, long len)
{
	bomb_rem_list();
}


void
vc_list_set::insert(const vc& item, const vc& idx)
{
	USER_BOMB2("can't insert into list");
}

void
vc_list_set::insert(const vc& item, long idx)
{
	USER_BOMB2("can't insert into list");
}

void
vc_list_set::printOn(VcIO os) 
{
		os << "list(";
		DwListAIter<vc> i(&list);
        vc e;
		dwlista_foreach_iter(i, e, list)
		{
			e.printOn(os);
			os << " ";
		}
		os << ")\n";
}

void
vc_list_set::stringrep(VcIO o) const
{

	o << "list(";
	DwListAIter<vc> i(&list);
    vc e;
	dwlista_foreach_iter(i, e, list)
	{
		e.stringrep(o);
		o << " ";
	}
	o << ")";
}


void
vc_list_set::foreach(const vc& v, const vc& expr) const
{
	Vcmap->open_loop();
	DwListAIter<vc> i(&list);
	vc e;
	++iterators;
	dwlista_foreach_iter(i, e, list)
	{
		Vcmap->local_add(v, e);
		expr.eval();
		if(Vcmap->unwind_in_progress())
			break;
	}
	--iterators;
	Vcmap->close_loop();
}

void
vc_list_set::foreach(const vc& v, VC_FOREACH_CALLBACK cb) const
{
	DwListAIter<vc> i(&list);
	vc e;
	++iterators;
	dwlista_foreach_iter(i, e, list)
	{
		(*cb)(e);
	}
	--iterators;
}

void
vc_list_set::foreach(const vc& v, VC_FOREACH_CALLBACK2 cb) const
{
    DwListAIter<vc> i(&list);
    vc e;
    ++iterators;
    dwlista_foreach_iter(i, e, list)
    {
        (*cb)(v, e);
    }
    --iterators;
}


long
vc_list_set::encode_elems(vcxstream& vcx)
{
	DwListAIter<vc> i(&list);
	vc e;
    long total = 0;
	dwlista_foreach_iter(i, e, list)
	{
		long s1 = e.xfer_out(vcx);
		if(s1 < 0)
			return -1;
		total += s1;
	}
    return total;
}

long
vc_list_set::decode_elems(vcxstream& vcx, long nelems)
{
	long total = 0;
	vc v;
	for(long i = 0; i < nelems; ++i)
	{
		long s1 = v.xfer_in(vcx);
		if(s1 < 0)
			return s1;
		append(v);
		total += s1;
	}

	return total;
}


// bag operators

vc&
vc_bag::operator[](const vc& v)
{
	USER_BOMB("can't index a bag", bogus);
}

vc&
vc_bag::operator[](int i)
{
	USER_BOMB("can't index a bag", bogus);
}

vc&
vc_bag::operator[](long i)
{
	USER_BOMB("can't index a bag", bogus);
}

int
vc_bag::contains(const vc& v) const
{
	return set.contains(v);
}

int
vc_bag::find(const vc& v, vc& out)
{
	if(set.contains(v))
	{
		out = v;
		return 1;
	}
    return 0;
}

void
vc_bag::add(const vc& v)
{
	if(iterators)
	{
		user_warning("bag add while foreach active");
	}
	set.add(v);
}

int
vc_bag::del(const vc& v)
{
	if(iterators)
	{
		user_warning("bag del while foreach active");
	}
	return set.del(v);
}
	
void
vc_bag::append(const vc& v)
{
	if(iterators)
	{
		user_warning("bag add while foreach active");
	}
	set.add(v);
}

vc
vc_bag::remove_last()
{
	bomb_rem();
	return vcnil;
}

void
vc_bag::prepend(const vc& v)
{
	if(iterators)
	{
		user_warning("bag add while foreach active");
	}
	set.add(v);
}

vc
vc_bag::remove_first()
{
	bomb_rem();
	return vcnil;
}
	
void
vc_bag::remove(const vc& , const vc& )
{
	bomb_rem();
}

void
vc_bag::remove(const vc& , long )
{
	bomb_rem();
}

void
vc_bag::remove(long , const vc& )
{
	bomb_rem();
}

void
vc_bag::remove(long , long )
{
	bomb_rem();
}


void
vc_bag::insert(const vc& , const vc& )
{
	USER_BOMB2("can't insert into bag");
}

void
vc_bag::insert(const vc& , long )
{
	USER_BOMB2("can't insert into bag");
}

void
vc_bag::printOn(VcIO os) 
{
		os << "bag(";
		VcBagIter i(&set);
		for(;!i.eol();i.forward())
		{
			i.get().printOn(os);
			os << " ";
		}
		os << ")\n";
}

void
vc_bag::stringrep(VcIO o) const
{
		
		o << "bag(";
		VcBagIter i(&set);
		for(;!i.eol();i.forward())
		{
			i.get().stringrep(o);
			o << " ";
		}
		o << ")";
}

void
vc_bag::foreach(const vc& v, const vc& expr) const
{
	Vcmap->open_loop();
	VcBagIter i(&set);
	++iterators;
	for(;!i.eol();i.forward())
	{
		Vcmap->local_add(v, i.get());
		expr.eval();
		if(Vcmap->unwind_in_progress())
			break;
	}
	--iterators;
	Vcmap->close_loop();
}

void
vc_bag::foreach(const vc& v, VC_FOREACH_CALLBACK cb) const
{
	VcBagIter i(&set);
	++iterators;
	for(;!i.eol();i.forward())
	{
		(*cb)(i.get());
	}
	--iterators;
}

void
vc_bag::foreach(const vc& v, VC_FOREACH_CALLBACK2 cb) const
{
    VcBagIter i(&set);
    ++iterators;
    for(;!i.eol();i.forward())
    {
        (*cb)(v, i.get());
    }
    --iterators;
}

long
vc_bag::encode_elems(vcxstream& vcx)
{
	VcBagIter i(&set);
    long total = 0;
	for(;!i.eol();i.forward())
	{
		long s1 = i.get().xfer_out(vcx);
		if(s1 < 0)
			return -1;
        total += s1;
	}
    return total;
}

long
vc_bag::decode_elems(vcxstream& vcx, long nelems)
{
	long total = 0;
	vc v;
    int ts;
    if(nelems == 0)
        ts = 3;
    else if(nelems > 500)
        ts = 499;
    else
        ts = nelems;

    set.set_size(ts);
	for(long i = 0; i < nelems; ++i)
	{
		long s1 = v.xfer_in(vcx);
		if(s1 < 0)
			return s1;
		add(v);
		total += s1;
	}

	return total;
}

// set operators

void
vc_set::printOn(VcIO os) 
{
		os << "set(";
		VcBagIter i(&set);
		for(;!i.eol();i.forward())
		{
			i.get().printOn(os);
			os << " ";
		}
		os << ")\n";
}

void
vc_set::stringrep(VcIO o) const
{
		
		o << "set(";
		VcBagIter i(&set);
		for(;!i.eol();i.forward())
		{
			i.get().stringrep(o);
			o << " ";
		}
		o << ")";
}

void
vc_set::prepend(const vc& v)
{
	if(!set.contains(v))
	{
		if(iterators)
		{
			user_warning("set add while foreach active");
		}
		set.add(v);
	}
}

void
vc_set::append(const vc& v)
{
	if(!set.contains(v))
	{
		if(iterators)
		{
			user_warning("set add while foreach active");
		}
		set.add(v);
	}
}

void
vc_set::add(const vc& v)
{
	if(!set.contains(v))
	{
		if(iterators)
		{
			user_warning("set add while foreach active");
		}
		set.add(v);
	}
}


// tree operators

vc&
vc_tree::operator[](const vc& v)
{
	return tree[v];
}

vc&
vc_tree::operator[](int i)
{
	oopanic("unimp tree op");
//	return map[vc(i)];
	return bogus;
}

vc&
vc_tree::operator[](long i)
{
	oopanic("unimp tree op");
//	return map[vc(i)];
	return bogus;
}

int
vc_tree::contains(const vc& v) const
{
	return tree.exists(v);
}

int
vc_tree::find(const vc& v, vc& out)
{
	return tree.find(v, out);
}

void
vc_tree::add(const vc& k)
{
	if(iterators)
	{
		user_warning("tree add while foreach active");
	}
	tree.replace(k, vcnil);
}

void
vc_tree::add_kv(const vc& k, const vc& v)
{
	if(iterators)
	{
		user_warning("tree add_kv while foreach active");
	}
	tree.replace(k, v);
}

int
vc_tree::del(const vc& v)
{
	if(iterators)
	{
		user_warning("tree del while foreach active");
	}
	return tree.del(v);
}
	
void
vc_tree::append(const vc& v)
{
	add(v);
}

vc
vc_tree::remove_last()
{
	USER_BOMB("can't remove last from tree", vcnil);
}

void
vc_tree::prepend(const vc& v)
{
	add(v);
}

vc
vc_tree::remove_first()
{
	if(iterators)
	{
		user_warning("tree remove_first while foreach active");
	}
	return tree.delmin();
}

vc
vc_tree::remove_first2(vc& v)
{
	if(iterators)
	{
		user_warning("tree remove_first2 while foreach active");
	}
	return tree.delmin(&v);
}
	
void
vc_tree::remove(const vc& , const vc& )
{
	bomb_rem_tree();
}

void
vc_tree::remove(const vc& , long )
{
	bomb_rem_tree();
}

void
vc_tree::remove(long , const vc& )
{
	bomb_rem_tree();
}

void
vc_tree::remove(long , long )
{
	bomb_rem_tree();
}

void
vc_tree::insert(const vc& , const vc& )
{
	USER_BOMB2("can't insert into tree");
	//tree[idx] = item;
}

void
vc_tree::insert(const vc& , long )
{
	USER_BOMB2("can't insert into tree");
	//tree[vc(idx)] = item;
}

void
vc_tree::stringrep(VcIO o) const
{

	o << "tree(";
	VcTreeIter i(&tree);
	vc assoc(VC_VECTOR);
	for(; !i.eol(); i.forward())
	{
		// break out Dwassoc into a vector
		DwAssocImp<vc,vc> a = i.get();
		assoc[0] = a.get_key();
        assoc[1] = a.get_value();
		assoc.stringrep(o);
		o << " ";
	}
	o << ")";
}

void
vc_tree::printOn(VcIO outputStream) 
{
		outputStream << "tree(\n";
		DwAssocImp<vc,vc> a;
		VcTreeIter i(&tree);

        for(;!i.eol();i.forward())
		{
			a = i.get();
			outputStream << "vector(";
			a.get_key().printOn(outputStream);
			outputStream << " ";
			a.get_value().printOn(outputStream);
			outputStream << ")\n";
		}
		outputStream << ")\n";
}


void
vc_tree::foreach(const vc& v, const vc& expr) const
{
	Vcmap->open_loop();
	VcTreeIter i(&tree);
	++iterators;
	for(; !i.eol(); i.forward())
	{
		// break out Dwassoc into a vector
		DwAssocImp<vc,vc> a = i.get();
		vc assoc(VC_VECTOR);
		Vcmap->local_add(v, assoc);
		assoc[0] = a.get_key();
        assoc[1] = a.get_value();
		expr.eval();
		if(Vcmap->unwind_in_progress())
			break;
	}
	--iterators;
	Vcmap->close_loop();
}

void
vc_tree::foreach(const vc& v, VC_FOREACH_CALLBACK cb) const
{
	VcTreeIter i(&tree);
	++iterators;
	for(; !i.eol(); i.forward())
	{
		// break out Dwassoc into a vector
		DwAssocImp<vc,vc> a = i.get();
		vc assoc(VC_VECTOR);
		assoc[0] = a.get_key();
		assoc[1] = a.get_value();
		(*cb)(assoc);
	}
	--iterators;
}

void
vc_tree::foreach(const vc& v, VC_FOREACH_CALLBACK2 cb) const
{
    VcTreeIter i(&tree);
    ++iterators;
    for(; !i.eol(); i.forward())
    {
        // break out Dwassoc into a vector
        DwAssocImp<vc,vc> a = i.get();
        vc assoc(VC_VECTOR);
        assoc[0] = a.get_key();
        assoc[1] = a.get_value();
        (*cb)(v, assoc);
    }
    --iterators;
}

long
vc_tree::encode_elems(vcxstream& vcx)
{
	// we do it pre-order to avoid too much
	// imbalance in the tree when it is decoded.
	// (normal iterator does in-order traversal
	VcTreeIterPre i(&tree);
	long total = 0;
	for(; !i.eol(); i.forward())
	{
		long s1, s2;
		// break out Dwassoc into a vector
		DwAssocImp<vc,vc> a = i.get();
		if((s1 = a.get_key().xfer_out(vcx)) < 0 || 
			(s2 = a.get_value().xfer_out(vcx)) < 0)
				return -1;
        total += s1 + s2;
	}
    return total;
 	
}

long
vc_tree::decode_elems(vcxstream& vcx, long nelems)
{
	long total = 0;
	vc key;
	vc value;
	for(long i = 0; i < nelems; ++i)
	{
		long s1 = key.xfer_in(vcx);
		if(s1 < 0)
			return s1;
		long s2 = value.xfer_in(vcx);
		if(s2 < 0)
			return s2;
		add_kv(key, value);
		total += s1 + s2;
	}
    return total;

}

// misc conversions that are useful

vc
vc::set_to_vector(vc set)
{
    vc v(VC_VECTOR);
    vc_bag *vs = (vc_bag *)set.nonono();
    VcBagIter i(&vs->set);

    for(; !i.eol(); i.forward())
    {
        vc uid = i.get();
        v.append(uid);
    }
    return v;
}

vc
vc::map_to_vector(vc map)
{
    vc v(VC_VECTOR);
    vc_map *vs = (vc_map *)map.nonono();
    VcMapIter i(&vs->map);

    for(; !i.eol(); i.forward())
    {
        vc vec(VC_VECTOR);
        DwAssocImp<vc, vc> a = i.get();
        vec[0] = a.peek_key();
        vec[1] = a.peek_value();
        v.append(vec);
    }
    return v;
}

vc
vc::tree_to_vector(vc map)
{
    vc v(VC_VECTOR);
    vc_tree *vs = (vc_tree *)map.nonono();
    VcTreeIter i(&vs->tree);

    for(; !i.eol(); i.forward())
    {
        vc vec(VC_VECTOR);
        DwAssocImp<vc, vc> a = i.get();
        vec[0] = a.peek_key();
        vec[1] = a.peek_value();
        v.append(vec);
    }
    return v;
}

