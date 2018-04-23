#ifndef DWTREE_H
#define DWTREE_H
#include "dwiter.h"
#include "dwassoc.h"
#include "dwstack.h"

// note: not done, but useable

// WARNING: bc4.52 doesn't like this class when inlining is
// turned on. optimization seems to be ok, but you must out-of-line
// inlines in order for deletes to work right.

// WARNING: gcc 3.4+ gets extreme heartburn from this class,
// mainly because of the struct node definition inside the
// template... all of the new name lookup stuff just makes it
// go bonkers.

template<class R, class D> class DwTreeIter;
template<class R, class D> class DwTreeIterPre;
template<class R, class D> class DwTree;

typedef void (*VOIDFUN)(void *);
typedef void (*VOIDFUN2)(void *, void *);

template<class R, class D>
class DwTree
{
friend class DwTreeIter<R,D>;
friend class DwTreeIterPre<R,D>;
friend struct node;
private:
#ifdef BC5
public:
#endif
	struct node;
	typedef void (*DWTREE_NODEFUN)(node *);
	typedef void (*DWTREE_NODEFUN2)(DwTree<R,D> *, node *);
	struct node
	{
		node(R const &cd, D const &k, node *par = 0)
			: clnt_data(cd), key(k) { l = 0; r = 0; parent = par;}
		node(node *li, node *ri, R const &cd, D const &k)
			: clnt_data(cd), key(k) {l = li; r = ri;}
		node *l;
		node *r;
		node *parent;
		R clnt_data;
		D key;

		node *find(D const &k, int& found){
			DwTree<R,D>::node **dummy;
			DwTree<R,D>::node *dum2;
			return find(k, found, dummy, dum2);
		}
		node *find(D const &k, int& found, node **& link_info){
			DwTree<R,D>::node *dum2;
			return find(k, found, link_info, dum2);
		}
		node *find(D const &k, int& found, node **& clink, node *&par) {
				if(k == key)
				{
					found = 1;
					return this;
				}
				if(k < key)
				{
					if(l != 0)
					{
						DwTree<R,D>::node *par = this;
						return l->find(k, found, clink, par);
					}
					else
					{
						clink = &l;
						found = 0;
					}
				}
				else
				{
					if(r != 0)
					{
						DwTree<R,D>::node *par = this;
						return r->find(k, found, clink, par);
					}
					else
					{
						clink = &r;
						found = 0;
					}
				}
			return this;
		}


		node *delmin(node *&n) {
			if(l == 0)
			{
				// this is smallest, unlink in parent and return it.
				n = r;
				if(r)
				{
					r->parent = parent;
				}
				return this;
			}
			return l->delmin(l);
		}

		node *del(D const &k, node *&par) {
			// patterned after aho, ullman, hopcroft 5.1
			if(k < key)
			{
				if(l)
					return l->del(k, l);
			}
			else if(!(k == key))
			{
				if(r)
					return r->del(k, r);
			}
			else // found it
			{
				if(l == 0 && r == 0)
				{
					par = 0;
					return this;
				}
				else if(l == 0)
				{
					par = r;
					r->parent = parent;
					return this;
				}
				else if(r == 0)
				{
					par = l;
					l->parent = parent;
					return this;
				}
				else
				{
					DwTree<R, D>::node *min = r->delmin(r);
					key = min->key;
					clnt_data = min->clnt_data;
					//delete min; // no need to delete
					return min;
				}
			}
			
			return 0;
		}

		void post_trav(DWTREE_NODEFUN f) {
			if(l) l->post_trav(f);
			if(r) r->post_trav(f);
			(*f)(this);
		}
		void post_trav(DwTree<R,D> *t, DWTREE_NODEFUN2 f) {
			if(l) l->post_trav(t, f);
			if(r) r->post_trav(t, f);
			(*f)(t, this);
		}
	};
#ifdef BC5
private:
#endif

	node *root;
	long count;
	R def;

#ifdef __GNUG__
	void postorder(VOIDFUN) const;
	void postorder(DwTree<R,D> *, VOIDFUN2) const;
	static void toastnode(void *);
	static void copy_node(DwTree<R,D> *, void *);
#else
	void postorder(DWTREE_NODEFUN) const;
	void postorder(DwTree<R,D> *, DWTREE_NODEFUN2) const;
	static void toastnode(node *);
	static void copy_node(DwTree<R,D> *, node *);
#endif

public:

	DwTree(R const &def);
	virtual ~DwTree();

	DwTree(const DwTree &t);
	DwTree& operator=(const DwTree &t);
	int operator==(DwTree const &t) const;

	const R& peek_ref(D const&);
	const R* peek_ptr(D const&);
	R get(D const&);
	int exists(D const&);
	int find(D const&, R& out);
	R& operator[](D const&);
	void add(D const &, R const &);
	void replace(D const &, R const &);
	int del(D const&);
	void clear();
	void rewind();
	int peek_get(R const*&, D const*&);
	void next();
	int num_elems() const {return count;}
	R delmin(D* = 0);

	DwAssocImp<R,D> get_by_iter(DwIter<DwTree<R,D>, DwAssocImp<R,D> > *) const;
};


#define theader template<class R, class D>
#define tcls DwTree<R,D>

theader
tcls::DwTree(R const & rdeflt)
{
	root = 0;
	count = 0;
	def = rdeflt;
}

theader
tcls::~DwTree()
{
#ifdef __GNUG__
	postorder((VOIDFUN)toastnode);
#else
	postorder(toastnode);
#endif
}

theader
void
tcls::clear()
{
#ifdef __GNUG__
	postorder((VOIDFUN)toastnode);
#else
	postorder(toastnode);
#endif
	root = 0;
	count = 0;
}

theader
DwTree<R,D>&
tcls::operator=(const DwTree<R,D>& t)
{
	if(this == &t)
		return *this;
#ifdef __GNUG__
	postorder((VOIDFUN)toastnode);
#else
	postorder(toastnode);
#endif
	root = 0;
	count = 0;
#ifdef __GNUG__
	t.postorder(this, (VOIDFUN2)copy_node);
#else
	t.postorder(this, copy_node);
#endif
	return *this;
}

theader
tcls::DwTree(const DwTree<R,D>& t)
	: def(t.def)
{
	root = 0;
	count = 0;
	
#ifdef __GNUG__
	t.postorder(this, (VOIDFUN2)copy_node);
#else
	t.postorder(this, copy_node);
#endif
}

theader
void
tcls::copy_node(DwTree<R,D> *to_tree,
#ifdef __GNUG__
	void *t)
#else
 tcls::node *from_node)
#endif
{
#ifdef __GNUG__
	node *from_node = (node *)t;
#endif
	to_tree->add(from_node->key, from_node->clnt_data);
}

theader
DwAssocImp<R,D>
tcls::get_by_iter(DwIter<DwTree<R,D>, DwAssocImp<R,D> > *a) const
{
	DwTreeIter<R,D> *ti = (DwTreeIter<R,D> *)a;

	return DwAssocImp<R,D>(ti->curnode->clnt_data, ti->curnode->key);
}

theader
void
#ifdef __GNUG__
tcls::postorder(VOIDFUN f) const
#else
tcls::postorder(tcls::DWTREE_NODEFUN f) const
#endif
{
	if(root == 0)
		return;
	root->post_trav((DWTREE_NODEFUN)f);
}

theader
void
#ifdef __GNUG__
tcls::postorder(DwTree<R,D> *t, VOIDFUN2 f) const
#else
tcls::postorder(DwTree<R,D> *t, tcls::DWTREE_NODEFUN2 f) const
#endif
{
	if(root == 0)
		return;
	root->post_trav(t, (DWTREE_NODEFUN2)f);
}

theader
void
#ifdef __GNUG__
tcls::toastnode(void *n)
#else
tcls::toastnode(node *n)
#endif
{
	delete (node *)n;
}

theader
R
tcls::get(D const &k)
{
	if(root == 0)
		return def;
	int found;
	node *n = root->find(k, found);
	if(found)
		return n->clnt_data;
	return def;
}

theader
int
tcls::exists(D const &k)
{
	if(root == 0)
		return 0;
	int found;
	root->find(k, found);
	return found;
}

theader
int
tcls::find(D const &k, R& out)
{
	if(root == 0)
		return 0;
	int found;
	node *n = root->find(k, found);
	if(found)
		out = n->clnt_data;
	return found;
}

theader
void
tcls::add(D const& key, R const& cd)
{
	if(root == 0)
	{
		root = new node(cd, key);
		count = 1;
		return;
	}
	node **link_info;
	int found;
	node *n = root->find(key, found, link_info);
	if(found)
		return;
	++count;
	*link_info = new node(cd, key, n);
}

theader
void
tcls::replace(D const& key, R const& cd)
{
	if(root == 0)
	{
		root = new node(cd, key);
		count = 1;
		return;
	}
	node **link_info;
	int found = 0;
	node *n = root->find(key, found, link_info);
	if(!found)
	{
		++count;
		*link_info = new node(cd, key, n);
		return;
	}
	n->clnt_data = cd;
}

theader
R&
tcls::operator[](D const &k)
{
	int found;
	node **link_info;
	if(root != 0)
	{
		node *n = root->find(k, found, link_info);
		if(found)
			return n->clnt_data;
	}
	add(k, def);
	root->find(k, found, link_info);
	return (*link_info)->clnt_data;
}

theader
int
tcls::del(D const& k)
{
	if(root == 0)
		return 0;
	node *par;
	node *n;

	n = root->del(k, par);
	if(n)
	{
		--count;
		delete n;
		if(n == root)
			root = par;
		if(count == 0)
			root = 0;
		return 1;
	}
	return 0;
}

theader
R
tcls::delmin(D* key_out)
{
	if(count == 0)
		return def;
	node *n;
	if(root->l == 0 && root->r == 0)
	{
		n = root;
	}
	else if(root->l == 0)
	{
		n = root;
		root->r->parent = 0;
		root = root->r;
	}
	else
	{
		n = root->l->delmin(root->l);
	}
	R tmp = n->clnt_data;
	if(key_out)
		*key_out = n->key;
	delete n;
	--count;
	if(count == 0)
		root = 0;
	return tmp;
}

#undef theader
#undef tcls

#define theader template<class R, class D>
#define tcls DwTreeIter<R,D>

// this is so we can avoid a call to "init"
struct _foofoodummy {};

// note: might be useful to have subclasses of this that
// perform pre/postorder enumerations of the tree.
theader
class DwTreeIter : public DwIter<DwTree<R,D>, DwAssocImp<R,D> >
{
friend class DwTree<R,D>;
protected:
#if __BORLANDC__ >= 0x560
typedef DwTree<R,D>::node dwtiFoo;
#endif

	DwTree<R,D>::node *curnode;
#if __BORLANDC__ >= 0x560
       DwStackA<dwtiFoo *> stk;
#else
	DwStackA<DwTree<R,D>::node *> stk;
#endif
	int go_right;
	
public:
	DwTreeIter(const DwTree<R,D> *);
	DwTreeIter(const DwTree<R,D> *, _foofoodummy);
	//~DwTreeIter();
	void init();
	void rewind() {init(); }
	void fast_forward() {}
	void forward();
	void backward() {}
	int eol() {return  curnode == 0; }
};

theader
tcls::DwTreeIter(const DwTree<R,D> *t)
	: DwIter<DwTree<R,D>, DwAssocImp<R,D> > (t), stk(0)
{
	init();
}

theader
tcls::DwTreeIter(const DwTree<R,D> *t, _foofoodummy)
	: DwIter<DwTree<R,D>, DwAssocImp<R,D> > (t), stk(0)
{

}


theader
void
tcls::init()
{
	go_right = 0;
	curnode = to_iterate->root;
	if(curnode == 0)
		return;
	forward();
}

theader
void
tcls::forward()
{
	while(1)
	{
		if(!go_right && curnode != 0)
		{
			while(curnode->l)
			{
				stk.push(curnode);
				curnode = curnode->l;
			}
			go_right = 1;
			return; // curnode is leaf node to visit in inorder
		}
		else
		{
			if(curnode && curnode->r)
			{
				curnode = curnode->r;
				go_right = 0;
				continue; // see if we can go down to the left
			}

			if(stk.num_elems() == 0)
			{
				curnode = 0;
				return;
			}
			curnode = stk.pop();
			go_right = 1;
			return; // in-order non-leaf

		}
	}
}

#undef tcls
#define tcls DwTreeIterPre<R,D>
theader
class DwTreeIterPre : public DwTreeIter<R,D>
{
public:
	DwTreeIterPre(const DwTree<R,D> *a);

	void init();
	void forward();

};

static _foofoodummy foofoo;

theader
tcls::DwTreeIterPre(const DwTree<R,D> *a)
	 : DwTreeIter<R,D>(a, foofoo)
{
	init();
}

theader
void
tcls::init()
{
	go_right = 0;
	curnode = to_iterate->root;
}

theader
void
tcls::forward()
{
	if(curnode == 0)
		return;
	while(1)
	{
		if(!go_right && curnode->l)
		{
			stk.push(curnode);
			curnode = curnode->l;
			return;
		}
		else if(curnode->r)
		{
			curnode = curnode->r;
			go_right = 0;
			return;
		}
		else
		{
			if(stk.num_elems() == 0)
			{
				curnode = 0;
				return;
			}
			curnode = stk.pop();
			go_right = 1;
		}
	}
}

#undef theader
#undef tcls

#endif
