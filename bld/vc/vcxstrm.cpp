
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include <climits>
#include "dwvec.h"
#include "dwvecp.h"
#include <string.h>
#include "vcxstrm.h"
#include <new>


void dwrtlog(const char *, char *, int, int, int, int, int, int);
void dwrtlog_vc(char *, int, vc);

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcxstrm.cpp 1.52 1998/12/09 05:12:37 dwight Exp $";


long vcxstream::Max_element_len = LONG_MAX;
long vcxstream::Max_elements = LONG_MAX;
long vcxstream::Max_depth = LONG_MAX;
long vcxstream::Max_memory = LONG_MAX;

// ok, this global tally thing isn't going to work like i expected.
// there were a bunch of things i forgot to consider like if there are multiple
// streams and some of them are "retry" because the device blocked, how do i
// adjust the amount of memory that has been used by that stream for those
// open-retry-close situations. also, who gets "charged" for a memory allocation
// needs to be thought about a little more concretely. new api's are needed so clients
// can set the max memory limits for each stream (or come up with a scheme where we
// just have one global limit, in order to protect against problems at a coarse level.)
// as it is, this will only work ok for simple things like "xfer" where there is one
// blocking deserialization going at any time.
unsigned long vcxstream::Memory_tally = 0;

void* operator new(std::size_t sz)
{
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success

    if (void *ptr = std::malloc(sz))
    {
        vcxstream::Memory_tally += sz;
        return ptr;
    }
    throw std::bad_alloc{}; // required by [new.delete.single]/3
}

static
int
elog10(unsigned int N)
{
    int estimate = 0;
    while (N > 9)
    {
        estimate += 1;
        N /= 10;
    }
    return estimate;
}

void
vcxstream::set_max_memory(int mxm)
{
    int lmxm = elog10(mxm);
    max_memory = mxm;
    max_count_digits = lmxm + 1;
}

#if 0
long VCX_max_element_len = 1024 * 1024;
long VCX_max_elements = 1024;
long VCX_max_depth = 7;
#endif

#define VCX_ILOG 128

int
vcxstream::check_status(enum status s)
{
	if(stat != s)
		return 0;
	return 1;
}

#if 0
vcxstream::vcxstream(char *ubuf, long l, enum style sty) : log(VCX_ILOG)
{
	dtype = NONE;
	uflow = 0;
	oflow = 0;
	manager = 0;
	mgr = 0;
	buf = ubuf;
	cur = buf;
	len = l;
	stat = NOT_OPEN;
	styl = sty;
    chit_table = 0;
	own_buf = 0;
	read_only = 0;
}
#endif

vcxstream::vcxstream(const char *ubuf, long l, enum style sty) : log(VCX_ILOG)
{
	dtype = NONE;
	uflow = 0;
	oflow = 0;
	manager = 0;
	mgr = 0;
	buf = (char *)ubuf;
	cur = buf;
	len = l;
	stat = NOT_OPEN;
	styl = sty;
    chit_table = 0;
	own_buf = 0;
	read_only = 0;

    max_elements = Max_elements;
    max_depth = Max_depth;
    max_element_len = Max_element_len;
    max_memory = Max_memory;
    memory_tally = 0;
    max_count_digits = INT_MAX;
}

vcxstream::vcxstream(VCXUNDERFUN uf, VCXOVERFUN of, vc_default *obj, char *ubuf, long l, enum style sty) : log(VCX_ILOG)
{
	dtype = INDEPENDENT;
	uflow = uf;
	oflow = of;
	manager = obj;
	buf = ubuf;
	cur = buf;
	len = l;
	stat = NOT_OPEN;
    chit_table = 0;
	own_buf = 0;
	styl = sty;
	read_only = 0;

    max_elements = Max_elements;
    max_depth = Max_depth;
    max_element_len = Max_element_len;
    max_memory = Max_memory;
    memory_tally = 0;
    max_count_digits = INT_MAX;
}

vcxstream::vcxstream(vc_default *obj, char *ubuf, long l, enum style sty) : log(VCX_ILOG)
{
	dtype = VCPTR;
	uflow = 0;
	oflow = 0;
	manager = obj;
	mgr = 0;
	buf = ubuf;
	cur = buf;
	len = l;
	stat = NOT_OPEN;
	styl = sty;
    chit_table = 0;
	own_buf = 0;
	read_only = 0;

    max_elements = Max_elements;
    max_depth = Max_depth;
    max_element_len = Max_element_len;
    max_memory = Max_memory;
    memory_tally = 0;
    max_count_digits = INT_MAX;
}

vcxstream::vcxstream(vc obj, char *ubuf, long l, enum style sty) : log(VCX_ILOG)
{
	dtype = VC;
	uflow = 0;
	oflow = 0;
	manager = 0;
	mgr = obj;
	buf = ubuf;
	cur = buf;
	len = l;
	stat = NOT_OPEN;
	styl = sty;
    chit_table = 0;
	own_buf = 0;
	read_only = 0;

    max_elements = Max_elements;
    max_depth = Max_depth;
    max_element_len = Max_element_len;
    max_memory = Max_memory;
    memory_tally = 0;
    max_count_digits = INT_MAX;
}

vcxstream::~vcxstream()
{
	if(!check_status(CLOSED))
		close(DISCARD);
	if(own_buf)
		delete [] buf;
    chit_destroy_table();
	if(chit_table != 0)
		delete chit_table;
}

enum vcxstream::status
vcxstream::get_status()
{
	return stat;
}

int
vcxstream::open(enum status s, enum e_iostyle ios)
{
	if(!(check_status(NOT_OPEN) || check_status(CLOSED) ||
		check_status(RETRYING)))
		return 0;
	if(read_only && s != READABLE)
		return 0;
	if(check_status(RETRYING))
	{
		if(s == READABLE)
		{
			stat = READABLE;
			return 1;
		}
		else if(s == WRITEABLE)
		{
			stat = WRITEABLE;
			return 1;
		}
		else
			return 0;
	}
	if(s != READABLE && s != WRITEABLE)
		return 0;

	if(buf == 0)
    {
		buf = new char[len];
		own_buf = 1;
	}
	stat = s;
	iostyle = ios;
	cur = buf;
	eob = buf + len;
	eod = buf;
	if(do_devopen(s, ios) == 0)
		return 0;
	if(styl == FIXED && s == READABLE)
	{
    	long nread;
		if((nread = do_underflow(buf, 0, len)) < 0)
			return 0;
		eod = buf + nread;
	}
    return 1;
}

int
vcxstream::close(how_close how)
{
    if(!check_status(READABLE) && !check_status(WRITEABLE))
        return 0;
	// regardless of what we're doing, eliminate the
	// chit table. this means that open-close pairs
	// are what determines how graph visits are computed
    chit_destroy_table();
    //delete chit_table;
    //chit_table = 0;
	
	if(how != DISCARD && iostyle == ATOMIC && check_status(READABLE))
	{
		if(how == RETRY)
			put_back(log.ref_str(), log.length());
		log.reset();
		if(how == RETRY)
		{
			stat = RETRYING;
			return 1;
		}
	}
	else if(how == RETRY && check_status(WRITEABLE))
	{
		stat = RETRYING;
		return 1;
	}
	int ok = 1;
	if(how == FLUSH && cur != buf)
	{
		if(!flush())
			ok = 0;
    }
	stat = CLOSED;
	if(do_devclose(how) == 0)
		return 0;
	return ok;
}

// calling retry resets the stream so that all the
// logged input is re-entered into the stream, so it will
// be returned by the next in_want. in addition, it resets
// the chit table. this is used if you are partway through
// a deserialization operation, and have to stop for some reason
// (most likely the input device returns wouldblock.)
//
int
vcxstream::retry()
{
    if(!(stat == READABLE || stat == WRITEABLE))
        oopanic("can't retry in that state");
    if(iostyle != ATOMIC)
        oopanic("can't retry on non-atomic streams");
    if(stat == READABLE)
    {
        put_back(log.ref_str(), log.length());
        log.reset();
    }
    chit_destroy_table();
    //delete chit_table;
    //chit_table = 0;
    //stat = (stat == READABLE ? READ_RETRY : WRITE_RETRY);
    return 1;
}

int
vcxstream::open2(enum status s, enum e_iostyle ios)
{
    if(!(check_status(NOT_OPEN) || check_status(CLOSED) ||
         check_status(READ_RETRY) || check_status(WRITE_RETRY)))
        return 0;
    if(read_only && s != READABLE)
        return 0;
    if(s == READABLE && stat == READ_RETRY)
    {
        stat = READABLE;
        return 1;
    }
    else if(s == WRITEABLE && stat == WRITE_RETRY)
    {
        stat = WRITEABLE;
        return 1;
    }
    else if(stat == READ_RETRY || stat == WRITE_RETRY)
    {
        return 0;
    }

    if(s != READABLE && s != WRITEABLE)
        return 0;

    if(buf == 0)
    {
        buf = new char[len];
        own_buf = 1;
    }
    stat = s;
    iostyle = ios;
    cur = buf;
    eob = buf + len;
    eod = buf;
    if(do_devopen(s, ios) == 0)
        return 0;
    if(styl == FIXED && s == READABLE)
    {
        long nread;
        if((nread = do_underflow(buf, 0, len)) < 0)
            return 0;
        eod = buf + nread;
    }
    return 1;
}


int
vcxstream::close2(how_close how)
{
    int ret = 1;
    switch(stat)
    {
    case READABLE:
        switch(how)
        {
        case DISCARD:
            log.reset();
            cur = eod;
            break;
        case FLUSH:
            log.reset();
            cur = eod;
            break;
        case CONTINUE:
            log.reset();
            chit_destroy_table();
            //stat = CLOSED;
            return 1;
            break;
        default:
            oopanic("bad close2");
        }
        break;
    case WRITEABLE:
        switch(how)
        {
        case DISCARD:
            cur = buf;
            break;
        case FLUSH:
            if(!flush())
                ret = 0;
            break;
        default:
            oopanic("bad close2-2");
        }
        break;

    case RETRYING:
        oopanic("intermixed close styles");
        break;
    case READ_RETRY:
    case WRITE_RETRY:
        if(how != DISCARD)
            oopanic("close called twice (and not from dtor)");
        // the only way we should get here is from the dtor, so
        // it will clean up everything
        return 1;
        break;

    default:
        return 0;
    }
    // regardless of what we're doing, eliminate the
    // chit table. this means that open-close pairs
    // are what determines how graph cycles are computed
    chit_destroy_table();
    //delete chit_table;
    //chit_table = 0;

    stat = CLOSED;
    if(do_devclose(how) == 0)
        return 0;
    return ret;
}

char *
vcxstream::out_want(long bytes)
{
	if(!check_status(WRITEABLE))
    	return 0;
//dwrtlog("outwant %x %d", __FILE__, __LINE__, (int)this, bytes, 0, 0, 0);
	if(cur + bytes > eob)
	{
		if(styl == FIXED)
			return 0;
		if(iostyle == ATOMIC)
		{
			// increase size of buffer to accommodate new
			// request, and avoid io operation.
			if(!own_buf)
				return 0;
			int newbuflen = len + (bytes - (eob - cur));
			char *newbuf = new char [newbuflen];
			memmove(newbuf, buf, cur - buf);
			cur = newbuf + (cur - buf);
			eob = newbuf + newbuflen;
			len = newbuflen;
			delete [] buf;
			buf = newbuf;
			eod = buf;
		}
		else
		{
			long to_send = cur - buf;
			long sent;
			if((sent = do_overflow(buf, to_send)) < to_send)
			{
				if(sent < 0)
					return 0;
				oopanic("overflow returned partial send, probably because you are trying to do nonatomic serialization on a non-blocking socket");
				/*NOTREACHED*/
			}
			cur = buf;
		}
	}
	if(bytes > len)
	{
		if(!own_buf || styl == FIXED)
        	return 0;
		delete [] buf;
		buf = cur = eod = new char [bytes];
		eob = buf + bytes;
        len = bytes;
	}
	char *ret = cur;
	cur += bytes;
	return ret;
}

char *
vcxstream::in_want(long bytes)
{
//dwrtlog("in want %x %d ", __FILE__, __LINE__, (int)this, bytes,  0, 0, 0);
//VcError << "in want " << bytes << " ";
	if(!check_status(READABLE))
		return 0;
	if(cur + bytes > eob)
	{
		if(styl == FIXED)
			return 0;
		// keep the remainder of the buffer
		// and then cat the new stuff onto it
		// so the whole mess appears contiguous
		long to_keep = eod - cur;
		char *addr_to_keep = cur;
		long to_get = bytes - to_keep;
		if(bytes <= len) // we can use current buffer
		{
			if(to_keep > 0)
				memmove(buf, cur, to_keep);
		}
		else // need a bigger buffer
		{
			if(!own_buf)	// can't do anything if it ain't ours
				return 0;  	// maybe we could have a callback or something
							// for this.
			char *obuf = buf;
			buf = new char [bytes];
			len = bytes;
			memmove(buf, addr_to_keep, to_keep);
			delete [] obuf;
			eob = buf + len;
		}
		//VcError << "new buf len " << len << " keeping " << to_keep << "\n";
		eod = buf + to_keep;
		cur = buf;
		long nread;
		if(to_get > 0)
		{
			if((nread = do_underflow(eod,
				to_get,
				styl == CONTINUOUS_READAHEAD ? 
					dwmax(to_get, len - to_keep) : to_get)) < 0)
				return 0;
			eod += nread;
			if(eod - cur < bytes)
			{
				return 0;
			}
		}
	}
	else if(cur + bytes > eod)
	{
		if(styl == FIXED)
			return 0;
		long bytes_needed = bytes - (eod - cur);
		long nread;
		if((nread = do_underflow(eod, bytes_needed, 
			styl == CONTINUOUS_READAHEAD ? eob - eod : bytes_needed)) < 0)
			return 0;
		eod += nread;
		if(eod - cur < bytes)
			return 0;
	}

	if(iostyle == ATOMIC)
	{
		// log the read results in case an
		// abort comes down.
		log.append(cur, bytes);
	}
	char *ret = cur;
	cur += bytes;

#if 0
	//VcError << "returning len " << bytes << " " ;
vc s(VC_BSTRING, ret, bytes);
s.printOn(VcError);
VcError << "\r\n";
#endif
	return ret;
}

void
vcxstream::put_back(const char *str, long pblen)
{
//VcError << "putting back " << pblen << "\n";
	if(!check_status(READABLE))
        oopanic("put back on non-readable stream");
	// attempt to stuff some stuff back into the
	// beginning of the buffer.
	if(pblen <= (cur - buf))
	{
		// it'll fit on the top of the current buffer
//VcError << "puting back on top " << cur << "\n";
		cur -= pblen;
		memmove(cur, str, pblen);
		return;
	}
	// not enough space at the top, allocate a new
	// buffer and copy everything over to the new
	// space.
	if(!own_buf)
        oopanic("can't enlarge buffer");
	int newlen = len + pblen;
	char *newbuf = new char [newlen];
//VcError << "putback new buffer len " << newlen << "\n";
	memmove(newbuf, str, pblen);
	memmove(newbuf + pblen, cur, eod - cur);
	eod = newbuf + (pblen + (eod - cur));
	cur = newbuf;
	eob = newbuf + newlen;
	delete [] buf;
	buf = newbuf;
	len = newlen;
}

int
vcxstream::flush()
{
	switch(stat)
	{
    case WRITEABLE:
		if(buf != cur)
		{
            if(do_overflow(buf, cur - buf) < cur - buf)
				return 0;
        	cur = buf;
		}
		break;

	case READABLE:
		cur = eod;
		break;
	default:
		break;

    }
	return 1;
}

// use this function for doing non-blocking
// io writes to the device. a call may return
// "done", in which case the io is all finished.
// a return of "retry" means to call flush again
// to push out some more info.
// this is only really intended to be used
// for writable atomic streams, with a device that
// can return partial write counts (like a socket in
// non-blocking mode). it hasn't been tested
// in other cases. do not interleave calls of nbflush
// with other stream operations. call flush until it
// returns done, and then close the stream.
//
int
vcxstream::flushnb()
{
	switch(stat)
	{
    case WRITEABLE:
		// eod is normally only used in read streams, but
		// in this case we hijack it to use it as a pointer
		// to the current bytes that need to be written out.
		//dwrtlog("flushnb %x eod %x cur %x to_send %d", __FILE__, __LINE__, (int)this, (int)eod, (int)cur, (int)(cur - eod),  0);
		if(eod != cur)
		{
			long to_send = cur - eod;
			long sent;
			if((sent = do_overflow(eod, to_send)) < to_send)
			{
				if(sent < 0)
					return NB_ERROR;
				eod += sent;
				return NB_RETRY;
			}
        	cur = buf;
			eod = buf;
		}
		break;

	case READABLE:
		cur = eod;
		break;
	default:
		break;

    }
	return NB_DONE;
}

vc_default *
vcxstream::chit_get(VCXCHIT c)
{
    if(!allow_self_ref)
        return 0;
	if(chit_table == 0)
		return 0;

	if(c < 0 || c >= chit_table->num_elems())
		return 0;
	return (*chit_table)[c];
}

void
vcxstream::chit_append(vc_default *v)
{
    if(!allow_self_ref)
        return;
	if(chit_table == 0)
		chit_table = new ChitTable;
	chit_table->append(v);
    // note: as a result of fuzzing, these references from the
    // chit table to the vc_default objects need to be tracked
    // by hand, otherwise they end up getting deleted too soon.
    // possibly it might make sense to make the chit table
    // just hold vc objects instead of vc_defaults, and then the
    // problem  might be solved without this manual reference counting.
    // but i'm not sure that will work without some thought since
    // we use this while we are constructing partially initialized objects...
    // have to think.
    ++v->ref_count;
}

VCXCHIT
vcxstream::chit_find(vc_default *v)
{
	VCXCHIT c = -1;

    if(!allow_self_ref)
    {
        oopanic("attempt to serialize self referential vc");
    }
    // this really is a panic, because chit_find is only called
    // during serialization and if we cant find a visited
    // item, there is something really wrong.
	if(chit_table == 0 || (c = chit_table->index(v)) < 0)
        oopanic("cant find chit");
	return c;
}

void
vcxstream::chit_new_table()
{
	delete chit_table;
	chit_table = 0;
}

void
vcxstream::chit_destroy_table()
{
    if(!chit_table)
        return;
    int n = chit_table->num_elems();
    for(int i = 0; i < n; ++i)
    {
        vc_default *v = (*chit_table)[i];
        --v->ref_count;
        if(v->ref_count <= 0)
            delete v;
    }
    chit_new_table();
}

void
vcxstream::commit()
{
	log.reset();
}


long
vcxstream::do_overflow(char *buf, long len)
{
	switch(dtype)
	{
	case NONE:
		return 0;
	case INDEPENDENT:
		return (*oflow)(*this, buf, len);
	case VCPTR:
		return manager->overflow(*this, buf, len);
	case VC:
		return mgr.overflow(*this, buf, len);
	default:
		::abort();
	}
	return -1;
}

long
vcxstream::do_underflow(char *buf, long min, long max)
{
	switch(dtype)
	{
	case NONE:
		return len; // fake it out so it looks like we got an entire buffer
	case INDEPENDENT:
		return (*uflow)(*this, buf, min, max);
	case VCPTR:
		return manager->underflow(*this, buf, min, max);
	case VC:
		return mgr.underflow(*this, buf, min, max);
	default:
		::abort();
	}
	return -1;
}

long
vcxstream::do_devopen(int style, int stat)
{
	switch(dtype)
	{
	case NONE:
		return 1;
	case INDEPENDENT:
		return 1;
	case VCPTR:
		return manager->devopen(*this, style, stat);
	case VC:
		return mgr.devopen(*this, style, stat);
	default:
		::abort();
	}
	return 0;
}

long
vcxstream::do_devclose(int style)
{
	switch(dtype)
	{
	case NONE:
		return 1;
	case INDEPENDENT:
		return 0;
	case VCPTR:
		return manager->devclose(*this, style);
	case VC:
		return mgr.devclose(*this, style);
	default:
		::abort();
	}
	return -1;
}

int
vcxstream::has_input()
{
	if(!check_status(READABLE))
		return 0;
	return cur != eod;
}

#undef TEST_VCXSTRM
#ifdef TEST_VCXSTRM
static char Buf[500];
static char *bp;
static int failpoint;

class Allocator;
Allocator *Default_alloc;

void
oopanic(char *s)
{
	printf("panic: %s\n", s);
	fflush(stdout);
#ifdef VCDBG
	drop_to_dbg("unrecoverable implementation bug", "bomb");
#endif
	exit(1);
}

static long
underflow(vcxstream& vcx, char *buf, long min, long max)
{
printf("underflow\n");
if(bp - Buf >= failpoint)
{
	printf("fail\n");
	return -1;
}
if(sizeof(Buf) - (bp - Buf) < min)
{
	printf("fail2\n");
	return -1;
}
memcpy(buf, bp, min);
bp += min;
printf("return %d \"%.*s\"\n", min, min, bp - min);
return min;
}

static long
overflow(vcxstream& vcx, char *buf, long howmuch)
{
printf("overflow\n");
	if(sizeof(Buf) - (bp - Buf) < howmuch)
	{
		printf("fail\n");
		return -1;
	}
	printf("output \"%.*s\"\n", howmuch, buf);
	memcpy(bp, buf, howmuch);
	bp += howmuch;
	return howmuch;
}
#include "vccomp.h"

static char rubbish[4096];

main(int, char **)
{
	int i;
	char *cp;
	
	cout.sync_with_stdio();
	bp = Buf;
	failpoint = sizeof(Buf) + 1;
	vcxstream v(underflow, overflow, 0, 0, 5);

	v.open(vcxstream::READABLE);
	v.put_back("abc", 3);
	v.put_back("k", 1);
	v.put_back(rubbish, 4096);

	cp = v.in_want(4096);
	for(i = 0; i < 4096; ++i)
		if(cp[i] != 0)
			::abort();
	cp = v.in_want(1);
	if(*cp != 'k')
		::abort();
	cp = v.in_want(3);
	if(strcmp(cp, "abc") != 0)
		::abort();
	cp = v.in_want(1);
	if(*cp != 0)
		::abort();

	v.close(vcxstream::FLUSH);
	
	
	bp = Buf;
	vc_composite::new_dfs();
	v.open(vcxstream::WRITEABLE, vcxstream::ATOMIC);
	vc v2(VC_VECTOR);
	vc v3(VC_SET);
	v3.append(v2);
	v2.append(v3);
	v2.xfer_out(v);
	v.close();

	int j = strlen(Buf);

	for(i = 0; i < j + 1; ++i)
	{
		failpoint = i;
		bp = Buf;
		vcxstream& vr = *new vcxstream(underflow, overflow, 0);
		vc v3;
		vr.open(vcxstream::READABLE, vcxstream::ATOMIC);
		int len = v3.xfer_in(vr);
		printf("fp %d len %d\n", failpoint, len);
		v3.print_top(cout);
		printf("\n");
		vr.close(vcxstream::RETRY);
		vr.open(vcxstream::READABLE, vcxstream::ATOMIC);
		failpoint = j + 1;
		len = v3.xfer_in(vr);
		printf("fp %d len %d\n", failpoint, len);
		v3.print_top(cout);
		printf("\n");
		vr.close(vcxstream::FLUSH);
		delete &vr;
		
	}
}
#endif

