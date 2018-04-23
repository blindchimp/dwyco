
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef USE_RCT
//
// simple ref count gc, doesn't do
// cycles (don't have any by design)
// just a test to see if this is worth
// the trouble.
//
// Sun Dec 18 11:06:24 MST 2011
// just as a little fun aside, i tried to get this to
// work. it does achieve some overlap in the processing
// of the reference counting and the mutation, but
// there is just enough overhead to make it not as good as just
// improving the rc stuff directly (and there is a major problem
// mentioned below.)
// in addition, the size of the process grows pretty large
// as would be expected with any deferred delete situation. so
// from that perspective, it suffers from the same problem as the other
// gc things i have explored.
//
// i also tried, but eventually gave up on, trying to do this
// in a "lock free" mode, by trying to allow the buffer
// pointers to be manipulated in both threads with minimal
// barriers. i couldn't see this working without inserting
// barriers in the ref counting stuff, which is an instant lose.

// assume these are aligned on a large
// boundary so we can mask for end of
// buffer.
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "vc.h"
#include "vcdeflt.h"
#include "vcnil.h"
#include "rct.h"

static void **RcInc[2];
static void **RcDec[2];
static void **RcDelq[2];

void **Inc_bp;
void **Dec_bp;
void **Del_bp;

static pthread_t Rc_th;
static pthread_cond_t rc_signal;
static pthread_mutex_t rc_signal_mutex;
static int Rc_space;
static int last_space;

void
init_rct()
{
	unsigned long c = (unsigned long)sbrk(0);

	int newc = (c + RCBUFSIZE) & ~(RCBUFSIZE - 1);

	(void **)sbrk(newc - c);
	for(int i = 0; i < 2; ++i)
	{
		RcInc[i] = (void **)sbrk(RCBUFSIZE);
		RcDec[i] = (void **)sbrk(RCBUFSIZE);
		RcDelq[i] = (void **)sbrk(RCBUFSIZE);
	}

	Inc_bp = RcInc[0];
	Dec_bp = RcDec[0];
	Del_bp = RcDelq[0];
	pthread_cond_init(&rc_signal, 0);
	pthread_mutex_init(&rc_signal_mutex, 0);

	void *rcgc_loop(void *);
	if(pthread_create(&Rc_th, 0, rcgc_loop, 0) != 0)
		::abort();

}


void
process_inc(void **bp)
{
	void **a = RcInc[last_space];
	while(a < bp)
	{
		((vc_default *)(*a++))->ref_count++;
	}
}


void
process_dec(void **bp)
{
	static int here;
	if(here)
		::abort();
	here = 1;
	void **a = RcDec[last_space];
	Del_bp = RcDelq[last_space];
	while(a < bp)
	{
		vc_default *v = (vc_default *)*a++;
		if(v == vc_nil::vcnilrep) continue;
		if(--v->ref_count == 0)
		{
			// we don't want a bunch of
			// recursive dec's coming in
			// here, so we q the delete
			// so we can do the actual
			// deletes after we have 
			// cleared the dec q.
			// note: no need to check here, since
			// the del q is as large as the
			// dec q.
			*Del_bp++ = (void *)v;
		}
	}
	a = RcDelq[last_space];
	// note: this isn't right because if there are
	// other vc objects in the object being deleted, this
	// will cause their dtors to fire, which means the Dec_bp
	// pointer could be getting manipulated by both threads
	// at the same time. adding barriers or sync stuff into the
	// ctors/dtors would negate the performance gains we are looking
	// for. only solution that comes to mind is to not do this here
	// and just send the q back to the other thread for processing.
	// but that will cause some performance loss too. so basically
	// this technique is a bit of a fail.
	while(a < Del_bp)
		delete (vc_default *)(*a++);
	here = 0;
}

static void **old_dec_bp;
static void **old_inc_bp;

void
rcgc2()
{
	// note:
	// need to stop if rc thread is still processing other space

	// swap space so mutator is writing to other space
	pthread_mutex_lock(&rc_signal_mutex);

	last_space = Rc_space;
	int nspace = !Rc_space;
	
	old_dec_bp = Dec_bp;
	old_inc_bp = Inc_bp;

	Dec_bp = RcDec[nspace];
	Inc_bp = RcInc[nspace];

	// signal thread to start processing
	Rc_space = !Rc_space;
	pthread_mutex_unlock(&rc_signal_mutex);
	pthread_cond_signal(&rc_signal);
}

void *
rcgc_loop(void *)
{
	while(1)
	{
		pthread_mutex_lock(&rc_signal_mutex);
		if(pthread_cond_wait(&rc_signal, &rc_signal_mutex) != 0)
			::abort();
		if(last_space == Rc_space)
			continue;
		pthread_mutex_unlock(&rc_signal_mutex);

		process_inc(old_inc_bp);
		process_dec(old_dec_bp);
	}
}

#endif

#if 0
main(int, char **)
{
	init_rct();
printf("%x", RCMASK);
fflush(stdout);
	int i;
	while(1) 
	{
		RCQINC(i);
		++i;
	}
}
#endif
