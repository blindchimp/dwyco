
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWPIPE_H
#define DWPIPE_H
#ifndef __WIN32__
#include "dwlista.h"
#include "dwvec.h"
#include "dwvecp.h"
#include "dwtree2.h"
#include <stdint.h>
#include <pthread.h>
#include <time.h>

// this class defines a pipeline where you feed the pipeline a chunk
// of memory, and the pipeline assumes the chunks can be operated
// on independently. the output of the pipeline is determined by the
// operations that are given as arguments when the pipeline is
// created. the outputs are returned in the same order as the chunks
// that are entered into the pipeline if you set the ordered flag on pipeline
// creation.
// NOTE: there is no guarantee of the order of processing if you have
// more than 1 thread.
// HOWEVER, if you specify one thread with ordering, you get a special case that is
// useful: a thread that can do processing in the background and notifies you
// when the result is ready. this is useful for cases where you don't need to
// overlap processing of items, but just need to avoid the latency of doing the operation
// in-line (like avoiding blocking a UI thread on an underpowered device, for example.)
namespace dwyco {
template<class I, class O>
struct DwPipelineOp
{
    virtual int execute(I *, O* &) = 0;
};

template<class I>
struct ordered_q_item
{
    ordered_q_item() {
        serial = -1;
        item = 0;
    }
    int operator==(const ordered_q_item& oqi) const {
        return serial == oqi.serial;
    }
    int64_t serial;
    I *item;
};

template<class I, class O>
class DwPipeline
{
public:
    DwPipeline(int ordered = 1, int nthreads = 8);
    ~DwPipeline();

    int init();

    int add_operation(DwPipelineOp<I, O> *);

    int put(I *);
    DwVec<O *> get();
    O *get2(); // block until we get it
    int get(DwVec<O *>&); // polling version (may block on q-locks tho)
    int get(O* &); // polling, but simpler return
    // note: should provide a "get" that is unordered
    int num_results();
    int num_inq();
    int idle();
    int next_result_ready();
    void thread_loop();

private:
    DwVecP<DwPipelineOp<I, O> > ops;
    DwListA<ordered_q_item<I> > inq;
    DwTreeKaz<DwVec<O *>, int64_t> outq;

    pthread_mutex_t q_mutex;
    pthread_cond_t q_cond;
    pthread_cond_t outq_cond;
    DwVec<pthread_t> threads;
    // each request that is entered in the q gets a
    // sequential serial number. "get" is guaranteed to
    // return the results in serial number order even if
    // the items are finished processing out of order.
    int64_t serial;
    int64_t just_finished;
    int64_t next_to_get;

    int ordered;
    int num_working;
    int die;

    void process_next_item();
};

#include "dwpipe.cpp"
}
#endif
#endif // DWPIPE_H
