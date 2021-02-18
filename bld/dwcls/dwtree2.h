
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWTREE2_H
#define DWTREE2_H
#include "dwiter.h"
#include "dwassoc.h"
#define DICT_IMPLEMENTATION
#include "dict.h"

// note: this is an encapsulation of the kazlib "dict" adt

template<class R, class D> class DwTreeKazIter;
template<class R, class D> class DwTreeKaz;

template<class R, class D>
class DwTreeKaz
{
    friend class DwTreeKazIter<R,D>;
private:

    R def;
    dict_t *dict;

public:

    DwTreeKaz(R const &def);
    DwTreeKaz(const DwTreeKaz &t);
    DwTreeKaz& operator=(const DwTreeKaz &t);
    virtual ~DwTreeKaz();


    int operator==(DwTreeKaz const &t) const;

    R get(D const&);
    int exists(D const&);
    int contains(D const&);
    int find(D const&, R& out);
    int find(const D&, R&out, DwTreeKazIter<R,D>& i);
    int find_upper_bound(const D&, R& out, DwTreeKazIter<R,D> *i = 0);
    int find_lower_bound(const D&, R& out, DwTreeKazIter<R,D> *i = 0);
    R& operator[](D const&);
    void add(D const &, R const &);
    void replace(D const &, R const &);
    int del(D const&);
    void clear();

    int num_elems() const {
        return dict_count(dict);
    }

    // warning: if the dict is empty, these return
    // default value.
    R delmin(D* = 0);
    R findmin(D* = 0);
    R findmax(D* = 0);

    DwAssocImp<R,D> get_by_iter(DwIter<DwTreeKaz<R,D>, DwAssocImp<R,D> > *) const;
};


#define theader template<class R, class D>
#define tcls DwTreeKaz<R,D>

template<class D>
int
dwtreekaz_compare(const void* t1, const void* t2)
{
    const D* d1 = (const D*) t1;
    const D* d2 = (const D*) t2;
    if(*d1 < *d2)
        return -1;
    if(*d1 == *d2)
        return 0;
    return 1;
}

template<class R, class D>
void
dwtreekaz_delete_node(dnode_t *n, void *)
{
    delete (R*)dnode_get(n);
    delete (D*)dnode_getkey(n);
    delete n;
}

dnode_t * dwtreekaz_alloc_node(void *);


theader
tcls::DwTreeKaz(R const & rdeflt)
{
    dict_comp_t cc = dwtreekaz_compare<D>;
    //dict = dict_create(1 << 31, cc);
    dict = new dict_t;
    dict_init(dict, 1 << 31, cc);
    dnode_free_t ft = dwtreekaz_delete_node<R,D>;
    dict_set_allocator(dict, dwtreekaz_alloc_node, ft, this);
    def = rdeflt;
}

theader
tcls::~DwTreeKaz()
{
    dict_free_nodes(dict);
    //dict_destroy(dict);
    delete dict;
    dict = 0;
}

theader
void
tcls::clear()
{
    dict_free_nodes(dict);
}

template<class R, class D>
void
copy_kaz_node(dict_t *d, dnode_t *n, void *ctx)
{
    dict_t *dnew = (dict_t *)ctx;
    R* r = new R(*(R*)dnode_get(n));
    D* dd = new D(*(D*)dnode_getkey(n));
    dict_alloc_insert(dnew, dd, r);
}

theader
DwTreeKaz<R,D>&
tcls::operator=(const DwTreeKaz<R,D>& t)
{
    if(this == &t)
        return *this;
    dict_free_nodes(dict);
    delete dict;
    //dict_comp_t cc = dwtreekaz_compare<D>;
    dict = new dict_t;
    dict_init_like(dict, t.dict);
    dnode_process_t dp = copy_kaz_node<R,D>;
    dict_process(t.dict, dict, dp);
    return *this;
}

theader
tcls::DwTreeKaz(const DwTreeKaz<R,D>& t)
    : def(t.def)
{
    //dict_comp_t cc = dwtreekaz_compare<D>;
    dict = new dict_t;
    dict_init_like(dict, t.dict);
    dnode_process_t dp = copy_kaz_node<R,D>;
    dict_process(t.dict, dict, dp);
}

theader
DwAssocImp<R,D>
tcls::get_by_iter(DwIter<DwTreeKaz<R,D>, DwAssocImp<R,D> > *a) const
{
    DwTreeKazIter<R,D> *ti = (DwTreeKazIter<R,D> *)a;

    return DwAssocImp<R,D>(*(R*)dnode_get(ti->curnode), *(D*)dnode_getkey(ti->curnode));
}


theader
R
tcls::get(D const &k)
{
    dnode_t *n = dict_lookup(dict, (const void *)&k);
    if(n)
        return *(R*)dnode_get(n);
    return def;
}

theader
int
tcls::exists(D const &k)
{
    dnode_t *n = dict_lookup(dict, (const void *)&k);
    return !!n;
}

theader
int
tcls::contains(D const &k)
{
    dnode_t *n = dict_lookup(dict, (const void *)&k);
    return !!n;
}

theader
int
tcls::find(D const &k, R& out)
{
    dnode_t *n = dict_lookup(dict, (const void *)&k);
    if(n)
    {
        out = *(R*)dnode_get(n);
        return 1;
    }
    return 0;
}

theader
int
tcls::find(D const &k, R& out, DwTreeKazIter<R,D>& i)
{
    dnode_t *n = dict_lookup(dict, (const void *)&k);
    if(n)
    {
        out = *(R*)dnode_get(n);
        i.curnode = n;
        return 1;
    }
    return 0;
}

theader
int
tcls::find_lower_bound(D const &k, R& out, DwTreeKazIter<R,D> *i)
{
    dnode_t *n = dict_lower_bound(dict, (const void *)&k);
    if(n)
    {
        out = *(R*)dnode_get(n);
        if(i)
            i->curnode = n;
        return 1;
    }
    return 0;
}

theader
int
tcls::find_upper_bound(D const &k, R& out, DwTreeKazIter<R,D> *i)
{
    dnode_t *n = dict_upper_bound(dict, (const void *)&k);
    if(n)
    {
        out = *(R*)dnode_get(n);
        if(i)
            i->curnode = n;
        return 1;
    }
    return 0;
}

theader
void
tcls::add(D const& key, R const& cd)
{
    dnode_t *n = dict_lookup(dict, (const void *)&key);
    if(n)
        return;
    R *clnt = new R(cd);
    D *k = new D(key);
    dict_alloc_insert(dict, (const void *)k, (void *)clnt);
}

theader
void
tcls::replace(D const& key, R const& cd)
{
    dnode_t *n = dict_lookup(dict, (const void *)&key);
    if(n)
    {
        R* clnt = (R*)dnode_get(n);
        delete clnt;
        clnt = new R(cd);
        dnode_put(n, clnt);
        return;
    }
    R *clnt = new R(cd);
    D *k = new D(key);
    dict_alloc_insert(dict, (const void *)k, (void *)clnt);
}

theader
R&
tcls::operator[](D const &k)
{
    dnode_t *n = dict_lookup(dict, (const void *)&k);
    if(n)
    {
        R* clnt = (R*)dnode_get(n);
        return *clnt;
    }
    add(k, def);
    n = dict_lookup(dict, (const void *)&k);
    R* clnt = (R*)dnode_get(n);
    return *clnt;
}

theader
int
tcls::del(D const& k)
{
    //NOTE: must define alloc/dealloc to free the pointers
    // in the node
    dnode_t *n = dict_lookup(dict, (const void *)&k);
    if(n)
    {
        dict_delete_free(dict, n);
        return 1;
    }
    return 0;
}

theader
R
tcls::delmin(D* key_out)
{
    dnode_t *n = dict_first(dict);
    if(n)
    {
        if(key_out)
            *key_out = *(D*)dnode_getkey(n);
        R tmp = *(R*)dnode_get(n);
        dict_delete_free(dict, n);
        return tmp;
    }
    return def;
}

theader
R
tcls::findmin(D* key_out)
{
    dnode_t *n = dict_first(dict);
    if(n)
    {
        if(key_out)
            *key_out = *(D*)dnode_getkey(n);
        R tmp = *(R*)dnode_get(n);
        return tmp;
    }
    return def;
}

theader
R
tcls::findmax(D* key_out)
{
    dnode_t *n = dict_last(dict);
    if(n)
    {
        if(key_out)
            *key_out = *(D*)dnode_getkey(n);
        R tmp = *(R*)dnode_get(n);
        return tmp;
    }
    return def;
}

#undef theader
#undef tcls

#define theader template<class R, class D>
#define tcls DwTreeKazIter<R,D>

// note: might be useful to have subclasses of this that
// perform pre/postorder enumerations of the tree.
theader
class DwTreeKazIter : public DwIter<DwTreeKaz<R,D>, DwAssocImp<R,D> >
{
    friend class DwTreeKaz<R,D>;
protected:

    dnode_t *curnode;

public:
    DwTreeKazIter(const DwTreeKaz<R,D> *);
    ~DwTreeKazIter() {}
    void init();
    void rewind() {
        init();
    }
    void fast_forward() {
        curnode = dict_last(this->to_iterate->dict);
    }
    void forward();
    void backward();
    int eol() {
        return  curnode == 0;
    }
};

theader
tcls::DwTreeKazIter(const DwTreeKaz<R,D> *t)
    : DwIter<DwTreeKaz<R,D>, DwAssocImp<R,D> > (t)
{
    init();
}

theader
void
tcls::init()
{
    curnode = dict_first(this->to_iterate->dict);
}

theader
void
tcls::forward()
{
    curnode = dict_next(this->to_iterate->dict, curnode);
}

theader
void
tcls::backward()
{
    curnode = dict_prev(this->to_iterate->dict, curnode);
}


#undef theader
#undef tcls

#endif
