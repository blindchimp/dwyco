
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
#ifdef LINUX
#include <sys/socket.h>
#endif
#ifdef USE_WINSOCK
#include <WinSock2.h>
#include <WS2tcpip.h>
#define SHUT_RDWR SD_BOTH
static
int
close(SOCKET sock)
{
    return ::closesocket(sock);
}
#endif

DwTreeKaz<vc_tsocket *, long> *vc_tsocket::Ready_q_p;

#ifndef USE_WINSOCK
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

vc
vc_tsocket::sockaddr_to_vc(struct sockaddr *sapi, int len)
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
    if(!readx.open2(vcxstream::READABLE, vcxstream::MULTIPLE))
        return -1;
    while(1)
    {
        vc item;
        long len;

        if((len = item.xfer_in(readx)) < 0)
        {
            recv_mutex.lock();
            if(len == EXIN_DEV)
            {
                vc v(VC_VECTOR);
                v[0] = "eof";
                v[1] = vc("t");
                getq.append(v);
                v = vcnil;
            }
            else if(len == EXIN_PARSE)
            {
                vc v(VC_VECTOR);
                v[0] = "error";
                v[1] = vcnil;
                getq.append(v);
                v = vcnil;
            }
            recv_mutex.unlock();
            // terminate thread
            readx.close2(vcxstream::FLUSH);
            return len;
        }
        if(!readx.close2(vcxstream::CONTINUE))
            return -1;
        vc v(VC_VECTOR);
        v[0] = "data";
        // note: queasy about using "vctrue" here since
        // we aren't doing the rc using thread safety.
        // we have this problem with vcnil all over the place
        // too.
        v[1] = vc("t");
        v[2] = item;
        v[3] = len;
        recv_mutex.lock();
        getq.append(v);
        item = vcnil;
        v = vcnil;
        recv_mutex.unlock();
    }
}

int
vc_tsocket::send_loop()
{
    std::unique_lock<std::mutex> ul(send_mutex, std::defer_lock);
    while(1)
    {
        ul.lock();
        putq_wait.wait(ul, [=](){return foad == 1 || putq.num_elems() > 0;});
        if(foad)
        {
            ul.unlock();
            return 0;
        }
        cbuf b = putq.get_first();
        putq.remove_first();
        ul.unlock();
        // ok, so send on blocking socket *can* return less than n
        cbuf tmp = b;
        while(tmp.len > 0)
        {
            auto n = send(sock, tmp.buf, tmp.len, 0);
            if(n <= 0)
                return -1;
            tmp.buf += n;
            tmp.len -= n;
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
        socklen_t slen = sizeof(sa);
        try
        {
            if((s = ::accept(sock, &sa, &slen)) == SOCKET_ERROR)
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
            v[1] = vc("t");
            v[2] = nsock;

            // build and install the first couple of messages on
            // the new socket, and start the data transfer threads
            vc v2(VC_VECTOR);
            v2[0] = "accept";
            v2[1] = vc("t");
            ts->getq.append(v2);

            vc vcc(VC_VECTOR);
            vcc[0] = "connect";
            vcc[1] = vc("t");
            ts->getq.append(vcc);
            ts->thread_start_mutex.lock();
            ts->recv_thread = new std::thread(&vc_tsocket::recv_loop, ts);
            ts->send_thread = new std::thread(&vc_tsocket::send_loop, ts);
            ts->thread_start_mutex.unlock();
            recv_mutex.lock();
            getq.append(v);
            v = vcnil;
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
            v = vcnil;
            recv_mutex.unlock();
            //if(sock != INVALID_SOCKET)
            //    ::close(sock);
            //sock = INVALID_SOCKET;
            return -1;
        }
    }
}

int
vc_tsocket::async_connect()
{
    struct sockaddr  *_sa;
    int alen;
    vc_to_sockaddr(cached_peer, _sa, alen);
    scoped_sockaddr sa(_sa);
    if(::connect(sock, sa, alen) == -1)
    {
        recv_mutex.lock();
        vc v(VC_VECTOR);
        v[0] = "connect";
        add_error(v, WSAGetLastError());
        getq.append(v);
        v = vcnil;
        recv_mutex.unlock();
        return -1;
    }
    // build and install the first couple of messages on
    // the new socket, and start the data transfer threads
    recv_mutex.lock();
    // note: scope block to ensure dtors are called while locked
    {
    vc v2(VC_VECTOR);
    v2[0] = "accept";
    v2[1] = vc("t");
    getq.append(v2);

    vc vcc(VC_VECTOR);
    vcc[0] = "connect";
    vcc[1] = vc("t");
    getq.append(vcc);
    }
    recv_mutex.unlock();
    thread_start_mutex.lock();
    recv_thread = new std::thread(&vc_tsocket::recv_loop, this);
    send_thread = new std::thread(&vc_tsocket::send_loop, this);
    thread_start_mutex.unlock();
    return 1;
}

vc_tsocket::vc_tsocket() :
    vp(this),
    getq(this),
    readx(this, 0, 0, vcxstream::CONTINUOUS_READAHEAD),
    send_lock(send_mutex, std::defer_lock)
{
    listening = 0;
    syntax = 0;
    state = 0;
    sock = INVALID_SOCKET;
    accept_thread = nullptr;
    send_thread = nullptr;
    recv_thread = nullptr;
    connect_thread = nullptr;
    foad = 0;
}

// note: 0 = xfer rep, 1 = length encoded, 2 = raw?, 3 = ogg?
int
vc_tsocket::set_syntax(int s)
{
    // note: need some checking or flushing probably if you switch midstream.
    syntax = s;
    return 1;
}

vc_tsocket::~vc_tsocket()
{
    foad = 1;
    if(sock != INVALID_SOCKET)
    {
        // note: don't close the socket until all
        // the threads are done, that way we don't
        // have problems with the socket being
        // reallocated by another thread.
        if(::shutdown(sock, SHUT_RDWR) == -1)
        {
#ifdef MACOSX
            // there is likely just a straight up bug in macos.
            // all other platforms (haven't checked ios)
            // the shutdown will cause the accept to
            // return an error even if it is blocked. the docs on macos say the
            // shutdown must be called on a "connected" socket
            // but the accept docs say "accept fails if the socket
            // is not willing to accept connections" as well as
            // "connection is aborted" which i guess means accept
            // got a wake up, but it can't finish putting the connection
            // together internally. anyway, this is different from the
            // other unix-like platforms. instead of trying to do signals
            // and polling, condition variable side-channels, or
            // other mickey-mouse work-arounds, we'll just
            // accept the slight chance of file-descriptor confusion.
            // if the shutdown doesn't succeed, the join below on the
            // accepter-thread ends up deadlocking the program in most cases.
            //
#endif
            // note: this is necessary for sockets that might be
            // in connect/read/write if the shutdown fails for some reason.
            // or already shutdown internally or whatever. shutdown seems to
            // fail in odd cases.
            ::close(sock);
            sock = INVALID_SOCKET;

        }
    }
    //sock = INVALID_SOCKET;
    // this is probably a bad idea to potentially block here,
    // but for testing right now, we'll do it.
    putq_wait.notify_all();
    if(accept_thread)
        accept_thread->join();
    if(connect_thread)
        connect_thread->join();
    delete accept_thread;
    delete connect_thread;

    thread_start_mutex.lock();
    if(send_thread)
        send_thread->join();
    if(recv_thread)
        recv_thread->join();
    delete send_thread;
    delete recv_thread;
    thread_start_mutex.unlock();

    cbuf c;
    dwlista_foreach(c, putq)
    {
        c.done();
    }
    //Ready_q.del(vp.cookie);
    if(sock != INVALID_SOCKET)
    {
        ::close(sock);
        sock = INVALID_SOCKET;
    }
    vp.invalidate();
}

long
vc_tsocket::underflow(vcxstream&, char *buf, long min, long max)
{
    long got = 0;
    auto to_get = max;
    while(got < min)
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
            accept_thread = new std::thread(&vc_tsocket::accept_loop, this);
            //al.detach();
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
        //::close(sock);
        //sock = INVALID_SOCKET;
    }
    return vctrue;
}


vc
vc_tsocket::socket_shutdown(int how)
{
    // i'm not sure this is used anywhere, will have to check it out
    if(sock != INVALID_SOCKET)
    {
        ::shutdown(sock, SHUT_RDWR);
        //::close(sock);
        //sock = INVALID_SOCKET;
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
    // in this case, the cached_peer become a "provisional peer"
    cached_peer = addr;
    connect_thread = new std::thread(&vc_tsocket::async_connect, this);
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
    send_lock.lock();
    cbuf b;
    b.buf = new char[len];
    memcpy(b.buf, buf, len);
    b.len = len;
    putq.append(b);
    send_lock.unlock();
    putq_wait.notify_one();
    return len;
}

// note: the syntax is a bit of a hack right now.
// changes in syntax on a stream would have to be sent
// out of band (or done apriori), since the get has no way of
// encoding the syntax right now (for compat reasons.)

vc
vc_tsocket::socket_put_obj(vc obj, const vc& to_addr, int syntax)
{
    if(sock == INVALID_SOCKET)
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
    send_lock.lock();
    const cbuf *cb;
    int len = 0;
    dwlista_foreach_peek(cb, putq)
    {
        len += cb->len;
    }
    send_lock.unlock();
    return len;
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


    os << "tsocket(lo=";
    socket_local_addr().printOn(os);
    os << " peer=";
    socket_peer_addr().printOn(os);
    os << " cached= ";
    cached_peer.printOn(os);
    os << ")";

}


#endif
