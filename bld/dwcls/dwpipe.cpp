
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/

#define thdr template<class I, class O>
#define tcls DwPipeline<I, O>

static void
ec_pthread_mutex_lock(pthread_mutex_t *m)
{
    if(pthread_mutex_lock(m) != 0)
        ::oopanic("mutexL");
    return;
}
static void
ec_pthread_mutex_unlock(pthread_mutex_t *m)
{
    if(pthread_mutex_unlock(m) != 0)
        ::oopanic("mutexU");
    return;
}

thdr
void *
tcls::pipe_thread(void *pipeline)
{
    tcls *p = (tcls *)pipeline;
    p->thread_loop();
    return 0;
}


thdr
tcls::DwPipeline(int ord, int nthreads) :
    threads(nthreads, DWVEC_FIXED, !DWVEC_AUTO_EXPAND),
    outq(DwVec<O *>())
{
    serial = 0;
    just_finished = 0;
    next_to_get = 0;
    die = 0;
    num_working = 0;
    ordered = ord;
}

// warning: this will almost certainly cause a
// lock up or a crash, so don't delete pipelines if
// you can help it.
thdr
tcls::~DwPipeline()
{
    die = 1;
    pthread_cond_broadcast(&q_cond);
    pthread_cond_broadcast(&outq_cond);
#ifndef ANDROID
    for(int i = 0; i < threads.num_elems(); ++i)
    {
        pthread_cancel(threads[i]);
    }
#endif
    for(int i = 0; i < threads.num_elems(); ++i)
    {
        pthread_join(threads[i], 0);
        pthread_cond_broadcast(&q_cond);
        pthread_cond_broadcast(&outq_cond);
    }
    pthread_cond_destroy(&q_cond);
    pthread_cond_destroy(&outq_cond);
    pthread_mutex_destroy(&q_mutex);
    // really need to do some clean up of the
    // partial results too...
}

thdr
int
tcls::init()
{
    pthread_mutexattr_t m;
    pthread_mutexattr_init(&m);
#ifdef MACOSX
    pthread_mutexattr_settype(&m, PTHREAD_MUTEX_ERRORCHECK);
#else
    pthread_mutexattr_settype(&m, PTHREAD_MUTEX_ERRORCHECK_NP);
#endif

    pthread_mutex_init(&q_mutex, &m);
    pthread_cond_init(&q_cond, 0);
    pthread_cond_init(&outq_cond, 0);
    for(int i = 0; i < threads.num_elems(); ++i)
        if(pthread_create(&threads[i], 0, pipe_thread, this) != 0)
            oopanic("can't create pipeline thread");

    return 1;
}

thdr
void
tcls::thread_loop()
{
#if 0
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000000;
#endif

    ec_pthread_mutex_lock(&q_mutex);
    while(1)
    {
        if(die)
            break;
        while(inq.num_elems() > 0)
        {
            if(die)
                goto out;
            ++num_working;
            process_next_item();
            --num_working;
        }
        //ec_pthread_mutex_unlock(&q_mutex);
        if(die)
            break;
#if 0
        ts.tv_sec = 0;
        ts.tv_nsec = 1000000;
#endif

        //pthread_cond_timedwait(&q_cond, &q_mutex, &ts);
        pthread_cond_wait(&q_cond, &q_mutex);
    }
out:
    ;
    ec_pthread_mutex_unlock(&q_mutex);
    return;
}

thdr
void
tcls::process_next_item()
{
    // note: assumes we have the q mutex
    ordered_q_item<I> item = inq.get_first();
    inq.remove_first();
    ec_pthread_mutex_unlock(&q_mutex);
    DwVec<O *> results(ops.num_elems(), DWVEC_FIXED, !DWVEC_AUTO_EXPAND);
    for(int i = 0; i < ops.num_elems(); ++i)
    {
        DwPipelineOp<I,O> *o = ops[i];
        if(!o->execute(item.item, results[i]))
        {
            oopanic("thread execute failed (fixme)");
        }
    }
    ec_pthread_mutex_lock(&q_mutex);
    just_finished = item.serial;
    outq.add(item.serial, results);
    //ec_pthread_mutex_unlock(&q_mutex);
    pthread_cond_signal(&outq_cond);
}

thdr
int
tcls::put(I *ni)
{
    ec_pthread_mutex_lock(&q_mutex);
    ordered_q_item<I> it;
    it.item = ni;
    it.serial = serial++;
    inq.append(it);
    pthread_cond_signal(&q_cond);
    ec_pthread_mutex_unlock(&q_mutex);

    return 1;
}

thdr
DwVec<O *>
tcls::get()
{
    ec_pthread_mutex_lock(&q_mutex);
    while(1)
    {
        DwVec<O *> res;
        if(ordered)
        {
            if(outq.find(next_to_get, res))
            {
                outq.del(next_to_get);
                ++next_to_get;
                ec_pthread_mutex_unlock(&q_mutex);
                return res;
            }
            else
            {
                //ec_pthread_mutex_unlock(&q_mutex);
                pthread_cond_wait(&outq_cond, &q_mutex);
                if(die)
                {
                    return DwVec<O *>();
                }
            }
        }
        else
        {
            if(outq.num_elems() > 0)
            {
                res = outq.delmin();
                ec_pthread_mutex_unlock(&q_mutex);
                return res;
            }
            else
            {
                //ec_pthread_mutex_unlock(&q_mutex);
                pthread_cond_wait(&outq_cond, &q_mutex);
                if(die)
                {
                    return DwVec<O *>();
                }
            }
        }

    }
}

thdr
int
tcls::get(DwVec<O *>& res)
{
    ec_pthread_mutex_lock(&q_mutex);
    if(ordered)
    {
        if(outq.find(next_to_get, res))
        {
            outq.del(next_to_get);
            ++next_to_get;
            ec_pthread_mutex_unlock(&q_mutex);
            return 1;
        }
    }
    else
    {
        if(outq.num_elems() > 0)
        {
            res = outq.delmin();
            ec_pthread_mutex_unlock(&q_mutex);
            return 1;
        }
    }
    ec_pthread_mutex_unlock(&q_mutex);
    return 0;

}

thdr
int
tcls::get(O* &out)
{
    DwVec<O *> res;
    int ret = get(res);
    if(ret)
        out = res[0];
    return ret;
}

thdr
O*
tcls::get2()
{
    DwVec<O *> res = get();
    return res[0];
}

thdr
int
tcls::idle()
{
    ec_pthread_mutex_lock(&q_mutex);
    if(num_working > 0 || inq.num_elems() > 0 || outq.num_elems() > 0)
    {
        ec_pthread_mutex_unlock(&q_mutex);
        return 0;
    }
    ec_pthread_mutex_unlock(&q_mutex);
    return 1;
}

thdr
int
tcls::next_result_ready()
{
    ec_pthread_mutex_lock(&q_mutex);
    int ret = 0;
    if(ordered)
    {
        DwVec<O *> res;
        if(outq.find(next_to_get, res))
        {
            ret = 1;
        }
    }
    else
    {
        ret = (outq.num_elems() > 0);
    }
    ec_pthread_mutex_unlock(&q_mutex);
    return ret;
}

thdr
int
tcls::num_results()
{
    ec_pthread_mutex_lock(&q_mutex);
    int cnt = outq.num_elems();
    ec_pthread_mutex_unlock(&q_mutex);
    return cnt;
}

thdr
int
tcls::num_inq()
{
    ec_pthread_mutex_lock(&q_mutex);
    int cnt = inq.num_elems();
    ec_pthread_mutex_unlock(&q_mutex);
    return cnt;
}

thdr
int
tcls::add_operation(DwPipelineOp<I,O> *o)
{
    ops.append(o);
    return 1;
}

#undef thdr
#undef tcls

#undef TEST
#ifdef TEST
// g++ -g -I../../ins/dbg/inc dwpipeline.cpp ../../ins/dbg/lib/dwcls.a ../../ins/dbg/lib/kazlib.a -lpthread
#include <stdio.h>
#include <unistd.h>

void
oopanic(const char *m)
{
    ::abort();
}

struct test_op : public DwPipelineOp<int, int>
{
    virtual int execute(int *in, int *& out) {
        out = new int;
        *out = *in + 1000000;
        for(int i = 0; i < rand() % 20000; ++i)
            ;
        delete in;
        return 1;
    }
};

int
main(int, char **)
{
    DwPipeline<int, int> p;

    p.init();
    p.add_operation(new test_op);

    for(int i = 1; i < 5000000; ++i)
    {
        int *n = new int;
        *n = i;
        p.put(n);
    }
    sleep(60);
    for(int i = 1; i < 5000000; ++i)
    {
        int *o = p.get2();
        printf("%d\n", *o);

        delete o;
    }
}

#endif
