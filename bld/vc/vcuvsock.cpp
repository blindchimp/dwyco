
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifndef DWYCO_NO_UVSOCK
#include <stdlib.h>
#include "vcuvsock.h"
#include "dwstr.h"
#include "vcxstrm.h"


uv_loop_t *vc_uvsocket::uvs_loop;
DwTreeKaz<vc_uvsocket *, long> *vc_uvsocket::Ready_q_p;

#ifndef _WIN32
static char *
ultoa(unsigned long l, char *out, int radix)
{
    sprintf(out, "%ld", l);
    return out;
}
#endif


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

static
vc
sockaddr_to_vc(struct sockaddr *sapi, int len)
{
    char dst[2048];
    char tmp_str[2048];
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
    uv_err_t err = uv_last_error(vc_uvsocket::uvs_loop);
    addr += uv_strerror(err);
    return addr.c_str();

}

int
vc_uvsocket::init_uvs_loop()
{
    uvs_loop = uv_loop_new();
	Ready_q_p = new DwTreeKaz<vc_uvsocket *, long>(0);
    return 1;
}

int
vc_uvsocket::run_loop_once()
{
    int ret;

    ret = uv_run(uvs_loop, UV_RUN_NOWAIT);

    return ret;
}

vc_uvsocket::vc_uvsocket() :
    vp(this),
    getq(this),
    putq(this),
    readx(this, 0, 0, vcxstream::CONTINUOUS_READAHEAD)
{
	listening = 0;
	tcp_handle = 0;
    tmpbuf = 0;
    tmplen = 0;
    syntax = 0;
    state = 0;
}

// note: 0 = xfer rep, 1 = length encoded, 2 = raw?, 3 = ogg?
int
vc_uvsocket::set_syntax(int s)
{
    // note: need some checking or flushing probably if you switch midstream.
    syntax = s;
    return 1;
}

static
void
close_cb(uv_handle_t *h)
{
    DwVP vp = DwVP::cookie_to_ptr((long)h->data);
	if(vp.is_valid())
	{
        //vc_uvsocket *vcu = (vc_uvsocket *)(void *)vp;
		// hmmm... what to do about the put and get q's?
	}
    else
    {
        // the dtor for uvsocket invalidates the object, but we
        // still need to delete the handle
    }
    delete (uv_tcp_t *)h;
}

// if we are destroying one of these things,
// we need to close everything down, and make sure
// the callbacks end up properly nulled out (make sure
// "data" members are checked properly
vc_uvsocket::~vc_uvsocket()
{
	if(tcp_handle)
        uv_close((uv_handle_t *)tcp_handle, close_cb);
	tcp_handle = 0;
    Ready_q.del(vp.cookie);
	vp.invalidate();
}

void
vc_uvsocket::add_error(vc v)
{
    v[1] = vcnil;
    uv_err_t err = uv_last_error(vc_uvsocket::uvs_loop);
    v[2] = err.code;
    v[3] = uv_strerror(err);
}

long
vc_uvsocket::underflow(vcxstream&, char *buf, long min, long max)
{
    if(tmplen < min)
        return -1;
    long n = max < tmplen ? max : tmplen;
    memcpy(buf, tmpbuf, n);
    tmpbuf += n;
    tmplen -= n;
    return n;
}

int
vc_uvsocket::parse_buffer(int once)
{
    vc obj;
    vcxstream& vcx = readx;

    tmpbuf = tailstr.c_str();
    tmplen = tailstr.length();
    //long used = tailstr.length();
    try
    {
        if(syntax == 0)
        {
            while(1)
            {
                long len;
                len = obj.xfer_in(vcx);
                if(len == EXIN_PARSE)
                {
                    throw -2;
                }
                if(len == EXIN_DEV)
                {
                    vcx.retry();
                    break;
                }
                else
                {
                    vc v(VC_VECTOR);
                    v[0] = "data";
                    v[1] = vctrue;
                    v[2] = obj;
                    v[3] = len;
                    getq.append(v);
                    //used -= len;
                    vcx.commit();
                    vcx.chit_destroy_table();
                    if(once)
                        break;
                }
            }
        }
        else if(syntax == 1)
        {
            oopanic("doesnt work");
            //                char *b;
            //                if(state == 0)
            //                {

            //                    if((b = vcx.in_want(sizeof(int32_t))) == 0)
            //                    {
            //                        vcx.retry();
            //                        break;
            //                    }
            //					vcx.commit();
            //                    len_to_read = (int32_t)ntohl(*(int32_t *)b);
            //                    used -= sizeof(int32_t);
            //                    if(len_to_read < 0)
            //                    {
            //                        throw -2;
            //                    }
            //                    // this is just a kludge sanity check
            //                    if(len_to_read > 10 * 1024 * 1024)
            //                    {
            //                        throw -2;
            //                    }
            //                    state = 1;
            //                }
            //                else if(state == 1)
            //                {
            //                    if((b = vcx.in_want(len_to_read)) == 0)
            //                    {
            //                        vcx.retry();
            //                        break;
            //                    }
            //					vcx.commit();
            //                    used -= len_to_read;
            //                    vc v(VC_VECTOR);
            //                    v[0] = "data";
            //                    v[1] = vctrue;
            //                    v[2] = vc(VC_BSTRING, b, len_to_read);
            //                    getq.append(v);
            //                    state = 0;
            //                    len_to_read = -1;
            //                    if(once)
            //                        break;
            //                }
            //                else
            //                    oopanic("bogus state");

        }
        else
            oopanic("bad syntax");


    }
    catch(int i)
    {
        // horrific error, socket should be abandoned in this
        // case because of some syntax error in the stream.
        // we leave it up to the client what to do, however.
        vc v(VC_VECTOR);
        v[0] = "error";
        v[1] = vcnil;
        v[2] = "syntax error";
        getq.append(v);
        vcx.close2(vcxstream::DISCARD);
        uv_read_stop((uv_stream_t *)tcp_handle);
        return 0;
    }

    // note: it is ok if tmplen is 0
    if(tmplen == 0) // everything was consumed
        tailstr = "";
    else
        tailstr = DwString(tmpbuf, 0, tmplen);
	return 1;
}

static
uv_buf_t
alloc_cb(uv_handle_t *handle, size_t sugg_size)
{
    uv_buf_t buf;
    buf.base = (char *)malloc(sugg_size);
    buf.len = sugg_size;
    return buf;
}

static
void
read_cb(uv_stream_t *stream, ssize_t nread, uv_buf_t buf)
{
    if(nread == 0)
    {
        free(buf.base);
        return;
    }

    DwVP p = DwVP::cookie_to_ptr((long)stream->data);
    if(!p.is_valid())
        return;
    vc_uvsocket *vcu = (vc_uvsocket *)(void *)p;

    vc obj;
    vcxstream& vcx = vcu->readx;

    if(nread == -1)
    {
        if(uv_last_error(vcu->uvs_loop).code == UV_EOF)
        {
            if(vcu->tailstr.length() == 0)
            {
                // normal end, all data consumed successfully
                // and all objects q-d. just give a normal eof indicator
                vc v(VC_VECTOR);
                v[0] = "eof";
                v[1] = vctrue;
                vcu->getq.append(v);
                vcx.close2(vcxstream::DISCARD);
                uv_read_stop((uv_stream_t *)vcu->tcp_handle);
                free(buf.base);
                return;
            }
            else
            {
                // some stuff left in the buffer we can't parse.
                // make it a syntax error
                vc v(VC_VECTOR);
                v[0] = "error";
                v[1] = vcnil;
                v[2] = "syntax error";
                vcu->getq.append(v);
            }
        }
        else
        {
            vc v(VC_VECTOR);
            v[0] = "error";
            vcu->add_error(v);
            vcu->getq.append(v);
        }
        // either the stream is in a weird state, or there is some
        // residual data that hasn't been able to be consumed, or there
        // was some other error with the device, give an error in all
        // cases.
        vcx.close2(vcxstream::DISCARD);
        uv_read_stop((uv_stream_t *)vcu->tcp_handle);
        free(buf.base);
        return;
    }
    vcu->tailstr += DwString(buf.base, 0, nread);
    free(buf.base);

    vcu->parse_buffer(0);
}

static
void
listen_cb(uv_stream_t *req, int status)
{
    DwVP p = DwVP::cookie_to_ptr((long)req->data);
    if(!p.is_valid())
    {
        // hmmm... listen socket object doesn't exist anymore
        // but we are still get cb's, maybe something more
        // drastic is needed here...
        return;
    }
    vc_uvsocket *vcu = (vc_uvsocket *)(void *)p;
    vc v(VC_VECTOR);
    v[0] = "listen-accept";
    if(status != 0)
    {
        vcu->add_error(v);
        vcu->getq.append(v);
        // don't delete the req, but if we get an error from the listener,
        // can we really continue to get more valid connections?
        return;
    }
    uv_tcp_t *client;
    client = new uv_tcp_t;
    if(uv_tcp_init(vcu->uvs_loop, client) == -1 ||
            uv_accept((uv_stream_t *)req, (uv_stream_t *)client) == -1)
    {
        delete client;
        vcu->add_error(v);
        vcu->getq.append(v);
        return;
    }

    vc_uvsocket *nv = new vc_uvsocket;
    nv->tcp_handle = client;
    nv->tcp_handle->data = (void *)nv->vp.cookie;
	nv->cached_peer = nv->socket_peer_addr();
	// inherit syntax from listening socket, even tho a listening
	// socket never really parses anything.
	nv->syntax = vcu->syntax;
    vc nvc;
    nvc.redefine(nv);
    // this is the return value on the listening socket
    v[1] = vctrue;
    v[2] = nvc; // new client socket

    // now, if there are problems getting initial setup on new client
    // socket, put an error in the getq immediately for that socket
    vc vcc(VC_VECTOR);
    vcc[0] = "connect";
    vcc[1] = vctrue;
    if(uv_read_start((uv_stream_t *)nv->tcp_handle, alloc_cb, read_cb) == -1)
    {
        nv->add_error(vcc);
    }
    else if(!nv->readx.open2(vcxstream::READABLE, vcxstream::ATOMIC))
    {
        // note: this isn't right. there *could* be a libuv error
        // pending here, but most errors are just programming logic
        // errors that should be a panic, but we don't have a method
        // for that in the vcxstream at the moment
        nv->add_error(vcc);
    }
    else
    {
        // note: we put an accept on the generated socket as well, in case
        // the application doesn't really give a rats ass about what the
        // listen socket is generating
        vc v2(VC_VECTOR);
        v2[0] = "accept";
        v2[1] = vctrue;
        nv->getq.append(v2);
    }
    nv->getq.append(vcc);

    vcu->getq.append(v);

    // note: this is different from other libuv callbacks...
    // the req is reused when new accepts come in, so don't delete it.
    // delete req;

}


vc
vc_uvsocket::socket_init(const vc& local_addr, int listen, int reuse, int syntax)
{
	tcp_handle = new uv_tcp_t;
    tcp_handle->data = (void *)vp.cookie;
    try
    {
        if(uv_tcp_init(uvs_loop, tcp_handle) == -1)
            throw -1;

        vc laddr = local_addr;
        if(local_addr.is_nil())
            laddr = "any:any";
        struct sockaddr_in sa;
        vc_to_sockaddr(laddr, sa);
        if(uv_tcp_bind(tcp_handle, sa) == -1)
			throw -1;

		if(listen)
		{
            if(uv_listen((uv_stream_t *)tcp_handle, 128, listen_cb) == -1)
				throw -1;
			listening = 1;
		}
	}
	catch(int i)
	{
        vc v(VC_VECTOR);
        v[0] = "init";
        add_error(v);
        getq.append(v);
        uv_close((uv_handle_t *)tcp_handle, close_cb);
		tcp_handle = 0;
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
vc_uvsocket::socket_close(int close_info)
{
    if(tcp_handle)
        uv_close((uv_handle_t *)tcp_handle, close_cb);

    tcp_handle = 0;
    getq.clear();
    putq.clear();
    return vctrue;
}

static
void
shutdown_cb(uv_shutdown_t *req, int status)
{
    DwVP p = DwVP::cookie_to_ptr((long)req->data);
    if(!p.is_valid())
    {
        delete req;
        return;
    }
    vc_uvsocket *vcu = (vc_uvsocket *)(void *)p;
    vc v(VC_VECTOR);
    v[0] = "shutdown";
    v[1] = vctrue;
    if(status != UV_OK)
        vcu->add_error(v);
    vcu->getq.append(v);
    delete req;
}

vc
vc_uvsocket::socket_shutdown(int how)
{
	if(!tcp_handle)
		return vcnil;
    uv_shutdown_t *req = new uv_shutdown_t;
    req->data = (void *)vp.cookie;
    if(uv_shutdown(req, (uv_stream_t *)tcp_handle, shutdown_cb) != UV_OK)
    {
        vc v(VC_VECTOR);
        v[0] = "shutdown";
        add_error(v);
        getq.append(v);
        delete req;
        return vcnil;
    }
    return vctrue;
}

static
void
connect_cb(uv_connect_t *req, int status)
{
    DwVP p = DwVP::cookie_to_ptr((long)req->data);
	if(!p.is_valid())
    {
        delete req;
		return;
    }
	vc_uvsocket *vcu = (vc_uvsocket *)(void *)p;
	vc v(VC_VECTOR);
	v[0] = "connect";
    v[1] = vctrue;
    if(status != 0)
    {
        vcu->add_error(v);
    }
    else
    {
        if(uv_read_start((uv_stream_t *)vcu->tcp_handle, alloc_cb, read_cb) == -1)
        {
            vcu->add_error(v);
        }
        else if(!vcu->readx.open2(vcxstream::READABLE, vcxstream::ATOMIC))
        {
            // note: this isn't right. there *could* be a libuv error
            // pending here, but most errors are just programming logic
            // errors that should be a panic, but we don't have a method
            // for that in the vcxstream at the moment
            vcu->add_error(v);
        }
        else
        {
            // looks odd, but since we can start reading here, we add
            // an "accept" on the socket to mirror the "connect" you get
            // when you accept a socket (ie, the channels are full duplex).
            vc vca(VC_VECTOR);
            vca[0] = "accept";
            vca[1] = vctrue;
            // little hokey, i don't return a "new" socket here because
            // client-end sockets act slightly different than listen sockets.
            // maybe if it becomes a problem can do that here.
            vcu->getq.append(vca);
        }
    }
	vcu->getq.append(v);
	delete req;
}

vc
vc_uvsocket::socket_connect(const vc& addr)
{
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
vc_uvsocket::socket_get_obj(int& avail, vc& addr_info)
{
    if(getq.num_elems() == 0)
    {
        avail = 0;
        return vcnil;
    }
    vc ret = getq.get_first();
    getq.remove_first();
    avail = 1;
    return ret;

}

struct overflow_req
{
    long cookie;
    char *buf;
    uv_buf_t ubuf;
};

static void
write_cb(uv_write_t *req, int status)
{
    overflow_req *o = (overflow_req *)req->data;
    DwVP p = DwVP::cookie_to_ptr(o->cookie);
    if(!p.is_valid())
    {
        delete [] o->buf;
        delete o;
        delete req;
        return;
    }
    vc_uvsocket *vcu = (vc_uvsocket *)(void *)p;
    // note: not sure i want an ack on every write, so we just
    // put in errors (which will most likely result in a close
    // anyways.)
    if(status != 0)
    {
        vc v(VC_VECTOR);
        v[0] = "write";

        vcu->add_error(v);
        vcu->getq.append(v);
    }
    delete [] o->buf;
    delete o;
    delete req;

}



long
vc_uvsocket::overflow(vcxstream&, char *buf, long len)
{
    uv_write_t *req = new uv_write_t;
    overflow_req *o = new overflow_req;
    o->cookie = vp.cookie;
    o->buf = new char [len];

    req->data = o;

    memcpy(o->buf, buf, len);
    o->ubuf = uv_buf_init(o->buf, len);

    if(uv_write(req, (uv_stream_t *)tcp_handle, &o->ubuf, 1, write_cb) == -1)
    {
        delete [] o->buf;
        delete o;
        delete req;
        return -1;
    }
    return len;
}

// note: the syntax is a bit of a hack right now.
// changes in syntax on a stream would have to be sent
// out of band (or done apriori), since the get has no way of
// encoding the syntax right now (for compat reasons.)

vc
vc_uvsocket::socket_put_obj(vc obj, const vc& to_addr, int syntax)
{
	if(!tcp_handle)
		return vcnil;
    // serialize the object immediately and write the buffer out.
    // this avoids problems with attempting to serialize the
    // object while it is being updated by the caller.

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
vc_uvsocket::socket_get_write_q_size()
{
    if(tcp_handle)
        return (long)tcp_handle->write_queue_size;
    return 0;
}

int
vc_uvsocket::socket_get_read_q_len()
{
    return getq.num_elems();
}

vc
vc_uvsocket::socket_local_addr()
{
    if(tcp_handle == 0)
        return vcnil;
    struct sockaddr sa;
    int salen = sizeof(sa);
    if(uv_tcp_getsockname(tcp_handle, &sa, &salen) == UV_OK)
    {
        return sockaddr_to_vc(&sa, salen);

    }
    return vcnil;
}

vc
vc_uvsocket::socket_peer_addr()
{
    if(tcp_handle == 0)
        return vcnil;
    struct sockaddr sa;
    int salen = sizeof(sa);
    if(uv_tcp_getpeername(tcp_handle, &sa, &salen) == UV_OK)
    {
        return sockaddr_to_vc(&sa, salen);

    }
	return cached_peer;
}


void
vc_uvsocket::printOn(VcIO os)
{
    if(tcp_handle == 0)
    {
        os << "uvsocket(uninit)";
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

    os << "uvsocket(lo=";
	local.printOn(os);
	os << " peer=";
	peer.printOn(os);
	os << " cached= ";
	cached_peer.printOn(os);
	os << ")";

}


#endif
