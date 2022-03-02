
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifndef DWYCO_NO_TSOCK
#ifndef VCTSOCK_H
#define VCTSOCK_H
#include <thread>
#include <mutex>
#include <condition_variable>

#include "vcnil.h"
#include "vccomp.h"
#include "dwvp.h"
#include "vcxstrm.h"
#include "dwstr.h"
#include "vcio.h"
#include "dwgrows.h"

class vc_tsocket : public vc_composite
{
private:
    DwVP vp;
    int sock;

    static vc sockaddr_to_vc(struct sockaddr *sapi, int len);

    int listening;
    // xstream we use to reconstruct objects as they come in
    vcxstream readx;

    // for use by length-encoding syntax
    int state;
    int32_t len_to_read;

    // note: used to be private inh, but that cause some problems
    // elsewhere. this isn't the best idea, just a hackaround so i
    // didn't have to do a bunch of forwarding by hand.
    struct list2 : public DwListA<vc>
    {
        vc_tsocket *container;
        list2(vc_tsocket *s) : container(s) {}

        void append(const vc& v) {
            if(num_elems() == 0)
            {
                vc_tsocket::Ready_q_p->add(container->vp.cookie, container);
            }
            DwListA<vc>::append(v);

        }

        int remove_first() {
            if(num_elems() == 1)
            {
                vc_tsocket::Ready_q_p->del(container->vp.cookie);
            }
            return DwListA<vc>::remove_first();
        }
        void clear() {
            if(num_elems() >= 1)
            {
                vc_tsocket::Ready_q_p->del(container->vp.cookie);
            }
            DwListA<vc>::clear();
        }
        //DwListA<vc>::num_elems;
        // DwListA<vc>::get_first;
    };

    static DwTreeKaz<vc_tsocket *, long> *Ready_q_p;

    std::mutex recv_mutex;
    std::mutex send_mutex;
    std::condition_variable putq_wait;

    struct cbuf
    {
        cbuf() {
            buf = 0;
            len = -1;
        }

        int operator==(const cbuf&) const {
            oopanic("no searches please");
        }

        char *buf;
        int len;

        void done() {
            delete [] buf;
            buf = 0;
        }

    };

    list2 getq;
    DwListA<cbuf> putq;
    int recv_loop();
    int send_loop();
    int accept_loop();
    int async_connect();

    // this is needed since errors in the UV socket like
    // "connection reset by peer" obliterate the peer information.
    vc cached_peer;
    int syntax;

public:
    vc_tsocket();
    ~vc_tsocket();

    int set_syntax(int);


    // note: it would be possible to have these different syntaxes
    // interleaved on one stream, but it would require either some kind
    // of "meta encapsulation", or some other declaration of when to
    // switch between the different syntaxes. the length-encoded
    // functions are mostly here for compatibility with older
    // versions of the networking stuff. so for now, you cannot
    // interleave different syntaxes, stick with either all object
    // or all length-encoded on a single stream.

    // these are for i/o with vc xfer rep syntax
    vc socket_get_obj(int& avail, vc& addr_info);
    vc socket_put_obj(vc obj, const vc& to_addr, int syntax);

    vc socket_init(const vc& local_addr, int listen, int reuse, int syntax = 0);
    vc socket_close(int close_info);
    // graceful write shutdown
    vc socket_shutdown(int how);
    vc socket_connect(const vc& addr);
    vc socket_peer_addr();
    vc socket_local_addr();
    int socket_get_write_q_size();
    int socket_get_read_q_len();

    long overflow(vcxstream&, char *buf, long len);
    long underflow(vcxstream&, char *buf, long min, long max);
    virtual vc_default *do_copy() const {
        oopanic("copy tsocket?");
        return 0;
    }
    enum vc_type type() const {
        return VC_SOCKET;
    }
    int is_quoted() const {
        return 1;
    }
    void printOn(VcIO os);
    void stringrep(VcIO os) const {
        os << "tsocket()";
    }
};

#endif
#endif

