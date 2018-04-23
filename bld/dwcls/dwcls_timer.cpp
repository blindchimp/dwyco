/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/**
 * \file
 * Timer library implementation.
 * \author
 * Adam Dunkels <adam@sics.se>
 */

/**
 * \addtogroup timer
 * @{
 */

#include "dwcls_timer.h"
#ifdef LINUX
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#endif
#ifdef _Windows
#include <windows.h>
#endif
#include "dwtree2.h"
#include "dwvecp.h"
#ifdef DWCLS_TIMER_DBG
#include <stdio.h>
#include <string.h>
#endif

namespace dwyco {

static
clock_time_t
clock_time()
{
#ifdef _Windows
	return timeGetTime();
#elif defined(LINUX)
#ifdef MACOSX
	struct timeval tv;
	gettimeofday(&tv, 0);
	dwtime_t d = ((dwtime_t)tv.tv_sec * 1000 + tv.tv_usec / 1000);
	return d;
#else
	struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
	dwtime_t d = ((dwtime_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
	return d;
#endif
#else
#error fix timer routines for this os
#endif
}

static DwTreeKaz<DwVecP<struct timer> *, clock_time_t> *Timers;

// setting tm to 0 causes the timer to be removed from the index
static void
update_exp_time(struct timer *t, clock_time_t tm, clock_time_t interval)
{
    if(!Timers)
    {
        Timers = new DwTreeKaz<DwVecP<struct timer> *, clock_time_t>(0);
    }
    clock_time_t old_t = t->start + t->interval;
    DwVecP<struct timer> *tv = 0;
    if(Timers->find(old_t, tv))
    {
        long i = tv->index(t);
        if(i != -1)
            tv->del(i);
        if(tv->num_elems() == 0)
        {
            Timers->del(old_t);
            delete tv;
            tv = 0;
        }
    }
    if(tm == 0 && interval == 0)
        return;
    clock_time_t ntm = tm + interval;
    if(!Timers->find(ntm, tv))
    {
        tv = new DwVecP<struct timer>;
        Timers->add(ntm, tv);
    }
    tv->append(t);
    t->start = tm;
    t->interval = interval;
}

// this just gives a hint of what timer is the next
// unexpired timer among all the timers that are running.
// if there are timers that are already expired, we
// un-index them here. note that this can cause a problem
// if you have timers that are initialized in auto-reload
// and are recurring, but that you are not polling
// and ack-ing the expiration. unfortunately, we
// have played fast and loose with timers in a lot
// of places, and in some cases they are not polled
// depending on the state of the channel, etc.
// so... this should only be used as a hint in
// situations where you may get other events to wake you
// up (like network connections.) using this to try and
// estimate when to wake up for synthetic events, like
// trying to time up video playback or something, is
// likely to cause problems because if a timer expires
// because you went a little past its due time without
// polling it, it is removed here and no indication is
// given that you should hurry up and poll it.
clock_time_t
timer_next_expire()
{
    if(!Timers)
    {
        Timers = new DwTreeKaz<DwVecP<struct timer> *, clock_time_t>(0);
    }
    clock_time_t mt = 0;
    clock_time_t now = clock_time();
    while(1)
    {
        DwVecP<struct timer> *v = Timers->findmin(&mt);
        if(!v)
            return 0;
#ifdef DWCLS_TIMER_DBG
        fprintf(stderr, "dtmr min\n");
        for(int i = 0; i < v->num_elems(); ++i)
            (*v)[i]->print();
#endif
        if(mt < now)
        {
#ifdef DWCLS_TIMER_DBG
            fprintf(stderr, "delmin\n");
#endif
            Timers->delmin();
            delete v;
            continue;
        }
        else break;
    }
    return mt;
}

#ifdef DWCLS_TIMER_DBG
void
timer::print()
{
    fprintf(stderr, "dtmr %s %ld %ld\n", lid, start, interval);
}

#endif
timer::timer(const char *id)
{
    start = 0;
    interval = 0;
#ifdef DWCLS_TIMER_DBG
    if(id)
        strcpy(lid, id);
    else
        lid[0] = 0;
#endif
}

timer::~timer()
{
    update_exp_time(this, 0, 0);
}

void
timer::stop()
{
    update_exp_time(this, 0, 0);
}

/*---------------------------------------------------------------------------*/
/**
 * Set a timer.
 *
 * This function is used to set a timer for a time sometime in the
 * future. The function timer_expired() will evaluate to true after
 * the timer has expired.
 *
 * \param t A pointer to the timer
 * \param interval The interval before the timer expires.
 *
 */
void
timer_set(struct timer *t, clock_time_t interval)
{
  //t->interval = interval;
  //t->start = clock_time();
  update_exp_time(t, clock_time(), interval);
}
/*---------------------------------------------------------------------------*/
/**
 * Reset the timer with the same interval.
 *
 * This function resets the timer with the same interval that was
 * given to the timer_set() function. The start point of the interval
 * is the exact time that the timer last expired. Therefore, this
 * function will cause the timer to be stable over time, unlike the
 * timer_restart() function.
 *
 * \note Must not be executed before timer expired
 *
 * \param t A pointer to the timer.
 * \sa timer_restart()
 */
void
timer_reset(struct timer *t)
{
    // problem here if there is a large gap in time, an auto-reload
    // timer will seem to fire continuously as this catches up.
    // since it doesn't make a lot of sense to set the start
    // to sometime way in the past, just check for it being
    // more than an interval or so, and just restart the timer.
    // this is a problem in situations where timer state is kept around
    // while a process is stopped for a long time.
    clock_time_t now = clock_time();
    if((now - t->start) / t->interval > 1)
        timer_restart(t);
    else
    {
        //t->start += t->interval;
        update_exp_time(t, t->start + t->interval, t->interval);
    }
}
/*---------------------------------------------------------------------------*/
/**
 * Restart the timer from the current point in time
 *
 * This function restarts a timer with the same interval that was
 * given to the timer_set() function. The timer will start at the
 * current time.
 *
 * \note A periodic timer will drift if this function is used to reset
 * it. For preioric timers, use the timer_reset() function instead.
 *
 * \param t A pointer to the timer.
 *
 * \sa timer_reset()
 */
void
timer_restart(struct timer *t)
{
  //t->start = clock_time();
    update_exp_time(t, clock_time(), t->interval);
}
/*---------------------------------------------------------------------------*/
/**
 * Check if a timer has expired.
 *
 * This function tests if a timer has expired and returns true or
 * false depending on its status.
 *
 * \param t A pointer to the timer
 *
 * \return Non-zero if the timer has expired, zero otherwise.
 *
 */
int
timer_expired(struct timer *t)
{
  /* Note: Can not return diff >= t->interval so we add 1 to diff and return
     t->interval < diff - required to avoid an internal error in mspgcc. */
  clock_time_t diff = (clock_time() - t->start) + 1;
  return t->interval < diff;

}
/*---------------------------------------------------------------------------*/
/**
 * The time until the timer expires
 *
 * This function returns the time until the timer expires.
 *
 * \param t A pointer to the timer
 *
 * \return The time until the timer expires
 *
 */
clock_time_t
timer_remaining(struct timer *t)
{
  return t->start + t->interval - clock_time();
}
/*---------------------------------------------------------------------------*/
}

/** @} */
