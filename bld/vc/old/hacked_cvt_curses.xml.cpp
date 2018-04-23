
static vc
cvt__bool(bool foo)
{
	if(foo) return vctrue;
	return vcnil;
}

static vc
cvt__char(char foo)
{
	return (int)foo;
}

static vc
cvt__int(int foo)
{
	return foo;
}

static vc
cvt__long_unsigned_int(long unsigned int foo)
{
	return (long)foo;
}

static vc
cvt_p__win_st_(_win_st* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

// appears all cases involve returning a string or NULL
static vc
cvt_p_char(char* foo)
{
	if(!foo)
		return vcnil;
	return foo;
}

static vc
cvt_p_screen_(screen* foo)
{
	if(!foo)
		return vcnil;
	return vc(VC_INT_DTOR, 0, (long) foo);
}

static vc
cvt_pq_char(const char* foo)
{
	if(!foo)
		return vcnil;
	return foo;
}

static
bool
cvt_vc__bool(vc a)
{
	return !a.is_nil();
}

static
int
cvt_vc__int(vc a)
{
	return a;
}

static
long unsigned int
cvt_vc__long_unsigned_int(vc a)
{
	return (long)a;
}

static
short int
cvt_vc__short_int(vc a)
{
	return (int)a;
}

static
unsigned int
cvt_vc__unsigned_int(vc a)
{
	return (int)a;
}

#if 0
typedef int (*pF__int__int)(int);

static
pF__int__int
cvt_vc_pF__int__int(vc a)
{
}

typedef int (*pF__int_p__win_st__x__int)(_win_st *, int);
static
pF__int_p__win_st__x__int
cvt_vc_pF__int_p__win_st__x__int(vc a)
{
}
#endif

// warning: only works if the MEVENT * is an IN-only param
// and there is exactly one of them.
static
MEVENT*
cvt_vc_p_MEVENT_(vc a)
{
	static MEVENT m;
	m.id = (int)a[0];
	m.x = a[1];
	m.y = a[2];
	m.z = a[3];
	m.bstate = (int)a[4];
	return &m;
}

static
_win_st*
cvt_vc_p__win_st_(vc a)
{
	return (_win_st *)(void *)a;
}
static
char*
cvt_vc_p_char(vc a)
{
}
static
int*
cvt_vc_p_int(vc a)
{
}
static
long unsigned int*
cvt_vc_p_long_unsigned_int(vc a)
{
}
static
screen*
cvt_vc_p_screen_(vc a)
{
	return (screen *)(void *)a;
}
static
short int*
cvt_vc_p_short_int(vc a)
{
}
static
void*
cvt_vc_p_void(vc a)
{
	if(a.is_nil())
		return 0;
	// there are no cases where this is used where
	// the arg shouldn't be 0
	::abort();
	return (void *)a;
}
static
const MEVENT*
cvt_vc_pq_MEVENT_(vc a)
{
}
static
const _win_st*
cvt_vc_pq__win_st_(vc a)
{
	return (const _win_st *)(void *)a;
}
static
const cchar_t*
cvt_vc_pq_cchar_t_(vc a)
{
}
static
const char*
cvt_vc_pq_char(vc a)
{
	return (const char *)a;
}

// WARNING: static return
static
const long unsigned int*
cvt_vc_pq_long_unsigned_int(vc a)
{
	static unsigned long b = (unsigned long)(long)a;
	return &b;
}

static
const void*
cvt_vc_pq_void(vc a)
{
	if(a.is_nil())
		return 0;
	// there are no cases where this is used where
	// the arg shouldn't be 0
	::abort();
	return (const void *)(void *)a;
}
