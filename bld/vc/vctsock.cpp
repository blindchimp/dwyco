
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifndef DWYCO_NO_TSOCK
#include <stdlib.h>
#include "vctsock.h"
#include "dwstr.h"
#include "vcxstrm.h"
#include <sys/socket.h>

#ifdef USE_BERKSOCK
#include "vcberk.h"

static int
WSAGetLastError()
{
    return errno;
}

static int
WSASetLastError(int e)
{
    errno = e;
    return 0;
}

#endif

struct scoped_sockaddr
{
    struct sockaddr *value;
    explicit scoped_sockaddr(struct sockaddr *&v) {value = v; v = 0;}
    ~scoped_sockaddr() {if(value) free(value);}
    operator struct sockaddr *() {return value;}
};


#ifndef _WIN32
static char *
ultoa(unsigned long l, char *out, int radix)
{
    sprintf(out, "%ld", l);
    return out;
}
#endif
static
int
vc_to_sockaddr(const vc& v, struct sockaddr *& sapr, int& len)
{
    unsigned short in_port = 0;

    DwString inp((const char *)v);

    int colpos = inp.find(":");
    DwString ip;
    if(colpos != DwString::npos)
    {
        ip = inp;
        ip.remove(colpos);
        DwString sport = inp;
        sport.erase(0, colpos + 1);
        if(!sport.eq("any"))
            in_port = htons(atoi(sport.c_str()));
    }
    else
        ip = inp;

    struct in_addr in_addr;
    in_addr.s_addr = INADDR_ANY;

    if(!ip.eq("any"))
    {
        int res = inet_pton(AF_INET, ip.c_str(), &in_addr);
        // bogus, doesn't work right for broadcast address
        // need to use updated version of inet_addr
        if(res == 0)
        {
            return 0;
        }
    }

    struct sockaddr_in *sap = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));

    memset(sap, 0, sizeof(*sap));

    sap->sin_family = AF_INET;
    sap->sin_addr = in_addr;
    sap->sin_port = in_port;
    sapr = (struct sockaddr *)sap;
    len = sizeof(*sap);
    return 1;
}

static
vc
sockaddr_to_vc(struct sockaddr *sapi, int len)
{
    char tmp_str[128];

    struct sockaddr_in *sap = (struct sockaddr_in *)sapi;

    memset(tmp_str, 0, sizeof(tmp_str));
    char *str = inet_ntoa(sap->sin_addr);
    if(str == 0)
        return vcnil;
    DwString in_addrstr(str);

    if(sap->sin_port != 0)
    {
        in_addrstr += ':';
        in_addrstr += DwString::fromUint(ntohs(sap->sin_port));
    }

    vc ret(in_addrstr.c_str());
    return ret;
}

#if 0
static
void
vc_to_sockaddr(const vc& addr, sockaddr_in& out)
{
    DwString ip = (const char *)addr;
    int port = 0;

    int idx = ip.find(":");
    if(idx != DwString::npos)
    {
        DwString ps = ip;
        ps.erase(0, idx + 1);
        if(ps.eq("any"))
            port = 0;
        else
            port = atoi(ps.c_str());
        ip.erase(idx);
    }
    if(ip.eq("any"))
        ip = "0.0.0.0";
    out = uv_ip4_addr(ip.c_str(), port);
}

vc
vc_tsocket::sockaddr_to_vc(struct sockaddr *sapi, int len)
{
    char dst[512];
    char tmp_str[512];
    DwString addr;
    struct sockaddr_in *sap = (struct sockaddr_in *)sapi;
    if(uv_ip4_name(sap, dst, sizeof(dst)) == UV_OK)
    {
        addr = dst;
        ultoa(ntohs(sap->sin_port), tmp_str, 10);
        addr += ":";
        addr += tmp_str;
        return vc(addr.c_str());
    }
    uv_err_t err = uv_last_error(vc_tsocket::uvs_loop);
    addr += uv_strerror(err);
    return addr.c_str();

}
#endif

static
void
add_error(vc v, int e)
{
    v[1] = vcnil;
    v[2] = e;
    v[3] = vc_wsget_errstr(e);
}

int
vc_tsocket::recv_loop()
{
    while(1)
    {
        vc item;
        long len;
        if(!readx.open2(vcxstream::READABLE, vcxstream::MULTIPLE))
            return -1;
        if((len = item.xfer_in(readx)) < 0)
        {
            // terminate thread
            readx.close(vcxstream::FLUSH);
            return len;
        }
        if(!readx.close2(vcxstream::FLUSH))
            return -1;
        recv_mutex.lock();
        getq.append(item);
        item = vcnil;
        recv_mutex.unlock();
    }
}



int
vc_tsocket::send_loop()
{
    std::unique_lock<std::mutex> ul(send_mutex);
    while(1)
    {
        send_mutex.lock();
        putq_wait.wait(ul, [=](){return putq.num_elems() > 0;});
        cbuf b = putq.get_first();
        putq.remove_first();
        send_mutex.unlock();
        auto n = send(sock, b.buf, b.len, 0);
        if(n == -1)
            return -1;
        if(n < b.len)
        {
            oopanic("what? blocking socket partial send");
        }
        b.done();
    }
}

int
vc_tsocket::accept_loop()
{
    while(1)
    {
        SOCKET s;
        sockaddr sa;
        socklen_t slen = 0;
        try
        {
            if((s = accept(sock, &sa, &slen)) == SOCKET_ERROR)
            {
                throw -1;
            }
            auto ts = new vc_tsocket;
            ts->sock = s;
            ts->cached_peer = sockaddr_to_vc(&sa, slen);
            ts->syntax = syntax;
            vc nsock;
            nsock.redefine(ts);
            vc v(VC_VECTOR);
            v[0] = "listen-accept";
            v[1] = vctrue;
            v[2] = nsock;

            // build and install the first couple of messages on
            // the new socket, and start the data transfer threads
            vc v2(VC_VECTOR);
            v2[0] = "accept";
            v2[1] = vctrue;
            ts->getq.append(v2);

            vc vcc(VC_VECTOR);
            vcc[0] = "connect";
            vcc[1] = vctrue;
            ts->getq.append(vcc);

            std::thread rl(&vc_tsocket::recv_loop, ts);
            std::thread sl(&vc_tsocket::send_loop, ts);
            recv_mutex.lock();
            getq.append(v);
            recv_mutex.unlock();

        }
        catch(...)
        {
            int err = WSAGetLastError();
            vc v(VC_VECTOR);
            v[0] = "listen-accept";
            add_error(v, err);
            recv_mutex.lock();
            getq.append(v);
            recv_mutex.unlock();
            if(sock != INVALID_SOCKET)
                ::close(sock);
            sock = INVALID_SOCKET;
            return -1;
        }
    }
}

vc_tsocket::vc_tsocket() :
    vp(this),
    getq(this),
    //putq(this),
    readx(this, 0, 0, vcxstream::CONTINUOUS_READAHEAD)
{
    listening = 0;
    syntax = 0;
    state = 0;
    sock = INVALID_SOCKET;
}

// note: 0 = xfer rep, 1 = length encoded, 2 = raw?, 3 = ogg?
int
vc_tsocket::set_syntax(int s)
{
    // note: need some checking or flushing probably if you switch midstream.
    syntax = s;
    return 1;
}


// if we are destroying one of these things,
// we need to close everything down, and make sure
// the callbacks end up properly nulled out (make sure
// "data" members are checked properly
vc_tsocket::~vc_tsocket()
{
    if(sock != INVALID_SOCKET)
    {
        ::shutdown(sock, SHUT_RDWR);
        ::close(sock);
    }
    sock = INVALID_SOCKET;
    //Ready_q.del(vp.cookie);
    vp.invalidate();
}

long
vc_tsocket::underflow(vcxstream&, char *buf, long min, long max)
{
    long got = 0;
    auto to_get = max;
    while(to_get > 0)
    {
        auto n = recv(sock, buf, to_get, 0);
        if(n == -1)
        {
            // even if we got some, this means some kind of
            // unexpected thing happened, so we bail on the
            // data instead of returning a partial amount.
            return -1;
        }
        if(n == 0)
        {
            if(got < min)
                return -1;
            else
                break;
        }
        got += n;
        buf += n;
        to_get -= n;
    }
    return got;
}

// note: if you call init after calling connect, fireworks will ensue.
// this is assumed to run in the calling thread and there are no other
// threads running on this object
vc
vc_tsocket::socket_init(const vc& local_addr, int listen, int reuse, int syntax)
{
    try
    {
        sock = socket(PF_INET, SOCK_STREAM, 0);
        if(sock == SOCKET_ERROR)
            throw -1;

        vc laddr = local_addr;
        if(local_addr.is_nil())
            laddr = "any:any";
        struct sockaddr *_sa = 0;
        int len_addr;
        if(!vc_to_sockaddr(laddr, _sa, len_addr))
            throw -1;

        scoped_sockaddr sa(_sa);
        if(::bind(sock, sa, len_addr) == -1)
            throw -1;

        if(listen)
        {
            if(::listen(sock, 128) == -1)
                throw -1;
            listening = 1;
            std::thread al(&vc_tsocket::accept_loop, this);
        }
    }
    catch(...)
    {
        // NOTE: any allocations may tweak error return global, so we
        // do this before anything else, and hope we didnt get hosed.
        int err = WSAGetLastError();
        vc v(VC_VECTOR);
        v[0] = "init";
        add_error(v, err);
        getq.append(v);
        if(sock != INVALID_SOCKET)
            ::close(sock);
        sock = INVALID_SOCKET;
        return vcnil;
    }
    this->syntax = syntax;
    return vctrue;
}

// note: this is like a hard close, so anything in the
// output q is just eliminated by this call (which means you
// may have data loss unless your app protocol takes care of
// flushing explicitly. likewise with the input q.
vc
vc_tsocket::socket_close(int close_info)
{
    if(sock != INVALID_SOCKET)
    {
        ::shutdown(sock, SHUT_RDWR);
        ::close(sock);
        sock = INVALID_SOCKET;
    }
    // NOTE: the cleanup of these items should be handled in the
    // threads, blocking here waiting for the threads to end is not
    // a good idea.
    //getq.clear();
    //putq.clear();
    return vctrue;
}


vc
vc_tsocket::socket_shutdown(int how)
{
    // i'm not sure this is used anywhere, will have to check it out
    if(sock != INVALID_SOCKET)
    {
        ::shutdown(sock, SHUT_RDWR);
        ::close(sock);
        sock = INVALID_SOCKET;
    }
    return vctrue;
}


vc
vc_tsocket::socket_connect(const vc& addr)
{
    if(sock == INVALID_SOCKET)
        return vcnil;
    if(listening)
        return vcnil;


    if(!tcp_handle)
        return vcnil;
    struct sockaddr_in sa;
    vc_to_sockaddr(addr, sa);
    uv_connect_t *conn_handle = new uv_connect_t;
    if(uv_tcp_connect(conn_handle, tcp_handle, sa, connect_cb) == -1)
    {
        vc v(VC_VECTOR);
        v[0] = "connect";
        add_error(v);
        getq.append(v);
        delete conn_handle;
        return vcnil;
    }
    tcp_handle->data = (void *)vp.cookie;
    conn_handle->data = (void *)vp.cookie;
    // in this case, the cached_peer become a "provisional peer"
    cached_peer = addr;
    return vctrue;
}

vc
vc_tsocket::socket_get_obj(int& avail, vc& addr_info)
{
    recv_mutex.lock();
    if(getq.num_elems() == 0)
    {
        avail = 0;
        recv_mutex.unlock();
        return vcnil;
    }
    vc ret = getq.get_first();
    getq.remove_first();
    avail = 1;
    recv_mutex.unlock();
    return ret;
}


long
vc_tsocket::overflow(vcxstream&, char *buf, long len)
{
    send_mutex.lock();
    cbuf b;
    b.buf = new char[len];
    memcpy(b.buf, buf, len);
    b.len = len;
    putq.append(b);
    send_mutex.unlock();
    putq_wait.notify_one();
}

// note: the syntax is a bit of a hack right now.
// changes in syntax on a stream would have to be sent
// out of band (or done apriori), since the get has no way of
// encoding the syntax right now (for compat reasons.)

vc
vc_tsocket::socket_put_obj(vc obj, const vc& to_addr, int syntax)
{
    if(!tcp_handle)
        return vcnil;
    // serialize the object immediately and write the buffer out.
    // this avoids problems with attempting to serialize the
    // object while it is being updated by the caller.
    // NOTE! serialization is NOT thread safe, and will never be
    // for this setup. it is best to just serialize into bytes
    // that are shuffled out by another thread.

    vcxstream ox(this, 0, 0, vcxstream::CONTINUOUS);
    // note: we could make this MULTIPLE if you were sending really
    // large items, but for now, we'll keep it simple.
    if(!ox.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
    {
        return vcnil;
    }
    long len = 0;
    if(syntax == 0)
    {
        vc_composite::new_dfs();
        if((len = obj.xfer_out(ox)) == -1)
        {
            return vcnil;
        }
    }
    else if(syntax == 1)
    {
        char *b;
        if((b = ox.out_want(sizeof(int32_t))) == 0)
        {
            return vcnil;
        }
        *(int32_t *)b = htonl(obj.len());
        if((b = ox.out_want(obj.len())) == 0)
        {
            return vcnil;
        }
        memcpy(b, (const char *)obj, obj.len());
        len = sizeof(int32_t) + obj.len();
    }
    else
    {
        oopanic("bogus syntax");
    }
    if(!ox.close(vcxstream::FLUSH))
        return vcnil;
    return vc(len);
}

int
vc_tsocket::socket_get_write_q_size()
{
    // NOTE: add up all the bytes in the write q
    return 0;
}

int
vc_tsocket::socket_get_read_q_len()
{
    recv_mutex.lock();
    int i = getq.num_elems();
    recv_mutex.unlock();
    return i;
}

static
struct sockaddr *
get_sockaddr()
{
    struct sockaddr_in *sa;
    sa = (struct sockaddr_in *)malloc(sizeof(*sa));
    return (struct sockaddr *)sa;
}

static
int
get_sockaddr_len()
{
    return sizeof(struct sockaddr_in);
}

vc
vc_tsocket::socket_local_addr()
{
    struct sockaddr *_sa = get_sockaddr();
    scoped_sockaddr sa(_sa);
#ifdef UNIX
    socklen_t len = get_sockaddr_len();
#else
    int len = get_sockaddr_len();
#endif
    if(getsockname(sock, sa, &len) == SOCKET_ERROR)
    {
        return vcnil;
    }
    vc local_addr = sockaddr_to_vc(sa, get_sockaddr_len());
    return local_addr;
}

vc
vc_tsocket::socket_peer_addr()
{
    if(sock == INVALID_SOCKET)
        return vcnil;
    struct sockaddr *_sa = get_sockaddr();
    scoped_sockaddr sa(_sa);
#ifdef UNIX
    socklen_t len = get_sockaddr_len();
#else
    int len = get_sockaddr_len();
#endif
    if(getpeername(sock, sa, &len) == SOCKET_ERROR)
    {
        return cached_peer;
    }
    cached_peer = sockaddr_to_vc(sa, get_sockaddr_len());
    return cached_peer;
}


void
vc_tsocket::printOn(VcIO os)
{
    if(sock == INVALID_SOCKET)
    {
        os << "tsocket(uninit)";
        return;
    }
    struct sockaddr sa;
    int salen = sizeof(sa);

    vc local;
    vc peer;
    if(uv_tcp_getsockname(tcp_handle, &sa, &salen) == UV_OK)
    {
        local = sockaddr_to_vc(&sa, salen);
    }

    salen = sizeof(sa);
    if(uv_tcp_getpeername(tcp_handle, &sa, &salen) == UV_OK)
    {
        peer = sockaddr_to_vc(&sa, salen);
    }

    os << "tsocket(lo=";
    local.printOn(os);
    os << " peer=";
    peer.printOn(os);
    os << " cached= ";
    cached_peer.printOn(os);
    os << ")";

}


#endif
