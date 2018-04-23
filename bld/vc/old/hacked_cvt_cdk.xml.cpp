
static vc
cvt__char(char foo)
{
	return foo;
}

static vc
cvt__int(int foo)
{
	return foo;
}

static vc
cvt__long_int(long int foo)
{
	return foo;
}

static vc
cvt__long_unsigned_int(long unsigned int foo)
{
	return (long)foo;
}

static vc
cvt_p_SAlphalist_(SAlphalist* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SButtonBox_(SButtonBox* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SCalendar_(SCalendar* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SDialogBox_(SDialogBox* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SEntry_(SEntry* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SFileSelector_(SFileSelector* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SGraph_(SGraph* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SHistogram_(SHistogram* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SItemList_(SItemList* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SLabel_(SLabel* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SMarquee_(SMarquee* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SMatrix_(SMatrix* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SMentry_(SMentry* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SMenu_(SMenu* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SRadio_(SRadio* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SScale_(SScale* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SScreen_(SScreen* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SScroll_(SScroll* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SSelection_(SSelection* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SSlider_(SSlider* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SSwindow_(SSwindow* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_STemplate_(STemplate* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_SViewer_(SViewer* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p__IO_FILE_(_IO_FILE* foo)
{
	return vc(VC_INT_DTOR, 0, (long)foo);
}

static vc
cvt_p_char(char* foo)
{
	if(foo == 0)
		return vcnil;
	return foo;
}

static vc
cvt_p_int(int* foo)
{
	::abort();
}

static vc
cvt_p_long_unsigned_int(long unsigned int* foo)
{
	::abort();
}

static vc
cvt_pp_char(char** foo)
{
	::abort();
}

static vc
cvt_pp_long_unsigned_int(long unsigned int** foo)
{
	::abort();
}
static
BINDFN
cvt_vc_BINDFN(vc a)
{
	::abort();
}
static
ENTRYCB
cvt_vc_ENTRYCB(vc a)
{
	::abort();
}
static
MATRIXCB
cvt_vc_MATRIXCB(vc a)
{
	::abort();
}
static
MENTRYCB
cvt_vc_MENTRYCB(vc a)
{
	::abort();
}
static
TEMPLATECB
cvt_vc_TEMPLATECB(vc a)
{
	::abort();
}
static
char
cvt_vc__char(vc a)
{
	return a;
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
unsigned int
cvt_vc__unsigned_int(vc a)
{
	return (int)a;
}
static
SAlphalist*
cvt_vc_p_SAlphalist_(vc a)
{
	return (SAlphalist *)(void *)a;
}
static
SButtonBox*
cvt_vc_p_SButtonBox_(vc a)
{
	return (SButtonBox *)(void *)a;
}
static
SCalendar*
cvt_vc_p_SCalendar_(vc a)
{
	return (SCalendar *)(void *)a;
}
static
SDialogBox*
cvt_vc_p_SDialogBox_(vc a)
{
	return (SDialogBox *)(void *)a;
}
static
SEntry*
cvt_vc_p_SEntry_(vc a)
{
	return (SEntry *)(void *)a;
}
static
SFileSelector*
cvt_vc_p_SFileSelector_(vc a)
{
	return (SFileSelector *)(void *)a;
}
static
SGraph*
cvt_vc_p_SGraph_(vc a)
{
	return (SGraph *)(void *)a;
}
static
SHistogram*
cvt_vc_p_SHistogram_(vc a)
{
	return (SHistogram *)(void *)a;
}
static
SItemList*
cvt_vc_p_SItemList_(vc a)
{
	return (SItemList *)(void *)a;
}
static
SLabel*
cvt_vc_p_SLabel_(vc a)
{
	return (SLabel *)(void *)a;
}
static
SMarquee*
cvt_vc_p_SMarquee_(vc a)
{
	return (SMarquee *)(void *)a;
}
static
SMatrix*
cvt_vc_p_SMatrix_(vc a)
{
	return (SMatrix *)(void *)a;
}
static
SMentry*
cvt_vc_p_SMentry_(vc a)
{
	return (SMentry *)(void *)a;
}
static
SMenu*
cvt_vc_p_SMenu_(vc a)
{
	return (SMenu *)(void *)a;
}
static
SRadio*
cvt_vc_p_SRadio_(vc a)
{
	return (SRadio *)(void *)a;
}
static
SScale*
cvt_vc_p_SScale_(vc a)
{
	return (SScale *)(void *)a;
}
static
SScreen*
cvt_vc_p_SScreen_(vc a)
{
	return (SScreen *)(void *)a;
}
static
SScroll*
cvt_vc_p_SScroll_(vc a)
{
	return (SScroll *)(void *)a;
}
static
SSelection*
cvt_vc_p_SSelection_(vc a)
{
	return (SSelection *)(void *)a;
}
static
SSlider*
cvt_vc_p_SSlider_(vc a)
{
	return (SSlider *)(void *)a;
}
static
SSwindow*
cvt_vc_p_SSwindow_(vc a)
{
	return (SSwindow *)(void *)a;
}
static
STemplate*
cvt_vc_p_STemplate_(vc a)
{
	return (STemplate *)(void *)a;
}
static
SViewer*
cvt_vc_p_SViewer_(vc a)
{
	return (SViewer *)(void *)a;
}
static
_IO_FILE*
cvt_vc_p__IO_FILE_(vc a)
{
	::abort();
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
	if(a.is_nil())
		return 0;
	return (char *)(const char *)a;
}
static
int*
cvt_vc_p_int(vc a)
{
	::abort();
}

// in this lib, everywhere you see this type it is a
// string of chtype chars, so we map that to a vector
// of ints.
static
long unsigned int*
cvt_vc_p_long_unsigned_int(vc a)
{
	if(a.is_nil())
		return 0;
	int n = a.num_elems();
	unsigned long *v = (unsigned long *)malloc(n * sizeof(*v));
	for(int i = 0; i < n; ++i)
	{
		v[i] = (long)a[i];
	}
	push_ptr(v);
	return v;
}

static
void*
cvt_vc_p_void(vc a)
{
	return a;
}

static
SScreen**
cvt_vc_pp_SScreen_(vc a)
{
	::abort();
}

// WARNING: assumes the input vector is not mucked with during the
// wrapped function call (so the strings are NOT copied out.)
static
char**
cvt_vc_pp_char(vc a)
{
	int n = a.num_elems();
	char **v = (char **)malloc(n * sizeof(char *));
	for(int i = 0; i < n; ++i)
	{
		v[i] = (char *)(const char *)a[i];
	}
	push_ptr(v);
	return v;
}
