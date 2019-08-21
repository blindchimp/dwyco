#undef LOCAL_TEST
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <sys/resource.h>
#include "vc.h"
#include "vcxstrm.h"
#include "vcudh.h"
#include "vcsock.h"
#include "dwstr.h"
#include "vccrypt2.h"

vc Filename;
vc Filesize;
vc Key_material;
vc Session_key;
vc ECtx;
vc DCtx;
vc Client_UID;
vc Client_MID;
vc vclh_to_hex(vc);
vc vclh_file_size(vc);

int Req_enc;

vc Peer;
int File;
int Send;
int Loose_file; // set to one to allow looser filename checking
int Plain_file; // 1 means file is not an audio/video attachment
int Allow_start; // 1 means use slight variation that allows requester to start downloads at a given offset
int Allow_restart; // 1 means we will tell the sender we have part of the file already
off_t Start_offset;
off_t Total_recv;
off_t Session_recv;
int Recv_to_null;

#define MAXFILESIZE (100 * 1024 * 1024)

static int
backout(vc *)
{
    return VC_SOCKET_BACKOUT;
}

void
dolog(const char *s)
{
    DwString out;
    char a[4096];
    char b[4096];
    struct tm *t;
    time_t tt = time(0);
    t = gmtime(&tt);
    strftime(b, sizeof(b), "%Y/%m/%d %T - ", t);
    static int fd = open("xfer.log", O_CREAT|O_APPEND|O_WRONLY, 0666);
    sprintf(a, "(%d) ", getpid());

    out = b;
    out += a;
    if(Client_UID.type() == VC_STRING && Client_UID.len() == 10)
    {
        out += (const char *)vclh_to_hex(Client_UID);
        out += " ";
    }
    else
        out += Client_UID.is_nil() ? "nil " : "bogus ";

    if(Client_MID.type() == VC_STRING && Client_MID.len() == 10)
    {
        out += (const char *)vclh_to_hex(Client_MID);
        out += " ";
    }
    else
        out += Client_UID.is_nil() ? "nil " : "bogus ";

    DwString fn;
    if(Filename.len() > 100)
        fn = DwString((const char *)Filename, 0, 100);
    else
        fn = (const char *)Filename;
    out += fn;
    out += " ";
    out += s;
    out += "\n";

    (void)write(fd, out.c_str(), out.length());
    //close(fd);
}

int
load_info(vc& info, const char *filename)
{
    DwString newfn;

    newfn = filename;
    vc f(VC_FILE);
    f.set_err_callback(backout);
    vc v = newfn.c_str();
    if(!f.open(v, (vcfilemode)(VCFILE_READ|VCFILE_BINARY)))
        return 0;
    vcxstream vcx(f);
    if(!vcx.open(vcxstream::READABLE))
        goto err;
    if(info.xfer_in(vcx) < 0)
        goto err;
    if(!vcx.close())
        goto err;
    if(!f.close())
        goto err2;
    return 1;
err:
    f.close();
err2:
    return 0;
}


vc Server_keys;
#include "dwyco_dh_dif.cpp"

void
setup_session_key()
{
    if(Key_material.is_nil())
        return;
    udh_init(vcnil);

    vcxstream vcx(dwyco_dh_dif, sizeof(dwyco_dh_dif));
    if(!vcx.open(vcxstream::READABLE))
        return;
    if(Server_keys.xfer_in(vcx) < 0)
        return;
    vcx.close();

    Session_key = dh_store_and_forward_get_key(Key_material, Server_keys);
    ECtx = vclh_encdec_open();
    DCtx = vclh_encdec_open();
    vclh_encdec_init_key_ctx(ECtx, Session_key, 0);
    vclh_encdec_init_key_ctx(DCtx, Session_key, 0);
}


int
checkfn(vc s)
{
    if(Loose_file)
    {
        const char *a = (const char *)s;
        int n = s.len();
        // eliminates . and ..
        if(n <= 2 || n > 40)
            return 0;
        // only allow a-zA-Z0-9.-_
        for(int i = 0; i < n; ++i)
        {
            if((a[i] >= 'a' && a[i] <= 'z') ||
                    (a[i] >= 'A' && a[i] <= 'Z'))
                continue;
            if(a[i] >= '0' && a[i] <= '9')
                continue;
            if(a[i] == '.' || a[i] == '-' || a[i] == '_')
                continue;
            return 0;
        }
        return 1;
    }
    if(s.len() != 44 && s.len() != 24)
        return 0;
    const char *a = (const char *)s;
    int n = s.len() - 4;
    int i;
    for(i = 0; i < n; ++i)
    {
        if(a[i] >= '0' && a[i] <= '9')
            continue;
        if(a[i] >= 'a' && a[i] <= 'f')
            continue;
        break;
    }
    if(i != n)
        return 0;
    if(a[n] == '.' || a[n+1] == 'd' || a[n+2] == 'y' || a[n+3] == 'c')
        return 1;
    if(a[n] == '.' || a[n+1] == 'f' || a[n+2] == 'l' || a[n+3] == 'e')
    {
        Plain_file = 1;
        return 1;
    }
    return 0;
}

int
get_write_locked_fd(vc filename)
{
    int flags;
    struct flock fl;
    int retries;

    for(retries = 0; retries < 5; ++retries)
    {
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;

        if(Allow_restart)
            flags = O_CREAT|O_EXCL|O_WRONLY;
        else
            flags = O_CREAT|O_EXCL|O_WRONLY|O_TRUNC;

        int fd = open((const char *)filename, flags, 0666);
        if(fd == -1)
        {
            fd = open((const char *)filename, O_WRONLY);
            if(fd != -1)
            {
                fl.l_type = F_WRLCK;
                fl.l_whence = SEEK_SET;
                fl.l_start = 0;
                fl.l_len = 0;
                if(fcntl(fd, F_SETLK, &fl) == -1)
                {
                    // someone else has it, probably in the
                    // process of timing out, so we try to
                    // blow him away before we proceed so the
                    // file doesn't get gronked later when he
                    // does exit.
                    fl.l_type = F_WRLCK;
                    fl.l_whence = SEEK_SET;
                    fl.l_start = 0;
                    fl.l_len = 0;
                    if(fcntl(fd, F_GETLK, &fl) == -1)
                        return -1;
                    if(fl.l_type != F_UNLCK)
                    {
                        if(fl.l_pid > 0)
                        {
                            dolog("kill to unlock");
                            kill(fl.l_pid, SIGKILL);
                            sleep(1);
                            close(fd);
                            continue;
                        }
                        else
                            return -1;
                    }
                    else
                    {
                        // well it was unlocked, give it a retry
                        dolog("odd found unlocked");
                        close(fd);
                        continue;
                    }
                }
                else
                {
                    // we got the lock
                    if(Allow_restart)
                    {
                        off_t off = lseek(fd, 0, SEEK_END);
                        char a[1024];
                        sprintf(a, "locked recv restart %ld", off);
                        dolog(a);
                    }
                    else if(ftruncate(fd, 0) == 0)
                        dolog("locked and truncated");
                    else
                    {
                        close(fd);
                        dolog("truncate failed");
                        return -1;
                    }
                    return fd;
                }
            }
            else
            {
                // something weird, we can't open it for
                // exclusive write, but it doesn't exist when
                // we open it a second time, means someone else
                // zapped it in the mean time, retry
                // or wrong perms or something...
                dolog("zapped");
                continue;
            }
        }
        else
        {
            // put the lock on it
            fl.l_type = F_WRLCK;
            fl.l_whence = SEEK_SET;
            fl.l_start = 0;
            fl.l_len = 0;
            if(fcntl(fd, F_SETLK, &fl) == -1)
            {
                dolog("can't get lock");
                return -1;
            }
            dolog("got lock");
            return fd;
        }
    }
    return -1;
}

int
get_read_locked_file(vc filename)
{
    struct flock fl;

    int fd = open((const char *)filename, O_RDONLY);
    if(fd != -1)
    {
        fl.l_type = F_RDLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;
        if(fcntl(fd, F_SETLK, &fl) == -1)
        {
            return -1;
        }
        return fd;
    }
    return -1;
}


int
send_vc(vc sock, vc v)
{
    vcxstream vcx(sock);
    if(!vcx.open(vcxstream::WRITEABLE))
        return 0;
    vc_composite::new_dfs();
    if(!ECtx.is_nil())
    {
        v = vclh_encdec_xfer_enc_ctx(ECtx, v);
        if(v.is_nil())
            return 0;
    }
    if(v.xfer_out(vcx) < 0)
        return 0;
    vcx.close();
    return 1;
}

int
send_request(vc sock, vc filename, int filesize)
{
    vc v(VC_VECTOR);
    v.append("sfile");
    vc v2(VC_VECTOR);
    v2.append(filename);
    v2.append(filesize);
    v.append(v2);

    return send_vc(sock, v);
}

int
recv_vc(vc sock, vc& out)
{
    vc req;
    vcxstream vcx(sock);
    int len;

    if(!vcx.open(vcxstream::READABLE))
        return 0;
    if((len = req.xfer_in(vcx)) < 0)
        return 0;
    //dolog("xferin ok");
    if(!DCtx.is_nil())
    {
        if(encdec_xfer_dec_ctx(DCtx, req, out).is_nil())
            return 0;
    }
    else
        out = req;
    return 1;
}

int
recv_crypto(vc sock)
{
    vc sf_material;
    if(!recv_vc(sock, sf_material))
        return 0;
    Key_material = sf_material;
    setup_session_key();
    return 1;
}

int
recv_get_request(vc sock)
{
    vc req;

    if(!recv_vc(sock, req))
        return 0;
    if(Allow_start)
    {
        if(req.type() != VC_STRING)
        {
            if(req.type() != VC_VECTOR)
                return 0;
            if(req[0].type() != VC_STRING)
                return 0;
            if(req[1].type() != VC_INT)
                return 0;
            Start_offset = (long)req[1];
            Client_UID = req[2];
            Client_MID = req[3];

            req = req[0];
        }
    }
    else
    {
        if(req.type() != VC_STRING)
            return 0;
    }
    if(!checkfn(req))
    {
        dolog("bogus filename");
        return 0;
    }
    Filename = req;
    // it is better to get the lock as soon as we know the filename
    // so that we can avoid problems where file sizes and stuff would
    // change if there is another writer running around
    File = get_read_locked_file(Filename);
    if(File == -1)
        return 0;

    Filesize = vclh_file_size(req);
    if(Filesize.is_nil())
        return 0;
    long fs = (long)Filesize;
    // note: there is a problem here where if a file changes
    // size between fetches and becomes smaller (like an autoupdate
    // fetch where the size of the file gets smaller), we end up
    // seeking past the end of the file and then going crazy.
    // but, we don't want to error out necessarily either because that
    // may just trigger more retries. best bet is to just return and "end"
    // indicator normally and assume the signature will fail, causing the
    // client to reset from the beginning
    if(fs < 0 || fs > MAXFILESIZE ||
            Start_offset < 0)
        return 0;
    char a[4096];
    if(Start_offset > fs)
    {
        sprintf(a, "using size %d instead of %ld", (int)Filesize, Start_offset);
        dolog(a);
        Start_offset = fs;
    }

    sprintf(a, "size %d off %ld", (int)Filesize, Start_offset);
    dolog(a);
    return 1;
}

int
send_ok(vc sock)
{
    return send_vc(sock, "fileok");
}

int
send_reject(vc sock)
{
    return send_vc(sock, "zreject");
}

int
send_frok(vc sock)
{
    return send_vc(sock, "frok");
}

int
send_restart(vc sock, vc loc)
{
    vc v(VC_VECTOR);
    v[0] = "restart";
    v[1] = loc;
    return send_vc(sock, v);
}

int
recv_response(vc sock)
{
    vc resp;

    if(!recv_vc(sock, resp))
        return 0;
    if(resp.type() == VC_VECTOR)
    {
        if(Send)
            return 0;
        // receive a file

        if(resp[0] == vc("file"))
        {
            if(!checkfn(resp[1][0]))
            {
                dolog("bogus filename2");
                return 0;
            }
            Filename = resp[1][0];
            Filesize = resp[1][1];
            // 2 is session_id, not sure why i put that in
            Client_UID = resp[3];
            Client_MID = resp[4];
            int fd;
            if((fd = get_write_locked_fd(Filename)) == -1)
                return 0;
            File = fd;

            return 1;
        }
        return 0;
    }

    if(resp.type() == VC_STRING)
    {
        if(!Send)
            return 0;
        // send a file
        if(resp == vc("fileok"))
        {
            // the read lock should already have been gotten by now
            if(File == -1)
                return 0;
            if(lseek(File, Start_offset, SEEK_SET) == -1)
                return 0;
            return 1;
        }
        return 0;
    }
    return 0;
}

int
do_sendfile(vc sock)
{
    char buf[2048];
    int len;

    alarm(60);
    while((len = read(File, buf, 2048)) > 0)
    {
        if(!send_vc(sock, vc(VC_BSTRING, buf, len)))
        {
            dolog("send failed");
            exit(0);
        }
        alarm(60);
    }
    if(len == -1)
    {
        dolog("read error on file");
        exit(0);
    }
    if(!send_vc(sock, vcnil))
    {
        dolog("send failed");
        exit(0);
    }
    else
    {
        dolog("send done");
        return 1;
    }
    exit(0);
}

int
do_recvfile(vc sock)
{
    int len = -1;
    int done = 0;
    off_t total = Start_offset;
    Total_recv = total;

    vc rvc;
    while(recv_vc(sock, rvc))
    {
        if(rvc.is_nil())
        {
            if(total == (off_t)(long)Filesize)
                done = 1;
            break;
        }
        if(rvc.type() != VC_STRING)
        {
            break;
        }
        const char *buf = (const char *)rvc;
        len = rvc.len();
        if(len + total > (long)Filesize)
        {
            dolog("recv file bigger than filesize");
            char a[4096];
            sprintf(a, "recv fail %ld, %d, %ld", total, len, (long)Filesize);
            dolog(a);
            break;
        }
        if(!Recv_to_null && write(File, buf, len) != len)
        {
            dolog("disk write failed");
            unlink(Filename);
            exit(0);
        }
        total += len;
        Total_recv += len;
        Session_recv += len;
        alarm(60);
    }
    alarm(60);
    close(File);
    if(!done)
    {
        if(!Allow_restart)
        {
            unlink((const char *)Filename);
        }
        else
        {
            char a[4096];
            sprintf(a, "recv fail %ld, %d, %ld", total, len, (long)Filesize);
            dolog(a);
        }
        dolog("recv failed");
    }
    else
    {
        send_frok(sock);
        char a[4096];
        sprintf(a, "recv ok %ld", total);
        dolog(a);
    }
    exit(0);
}

vc lh_socket_from_os_handle(vc, vc, vc);


void
recv_main(int sock)
{
    vc vcsock = lh_socket_from_os_handle(sock, "tcp", vcnil);
    vcsock.set_err_callback(backout);
    if(Req_enc)
    {
        if(!recv_crypto(vcsock))
            exit(0);
    }
    if(!recv_response(vcsock))
        exit(0);
    if((long)Filesize > MAXFILESIZE)
    {
        // can't do this for now since it gives them the
        // option of retrying, but it will never work, so
        // we have to accept the file and send it to the
        // bit bucket
        Recv_to_null = 1;
        if(Allow_restart)
        {
            unlink(Filename);
            if(!send_reject(vcsock))
                exit(0);
            dolog("rejected too big");
            vc frok;
            if(!recv_vc(vcsock, frok) || frok != vc("rejectok"))
                dolog("no rejectok");
            else
                dolog("got rejectok");
            exit(0);
        }
    }
    if(!send_ok(vcsock))
        exit(0);
    do_recvfile(vcsock);
    exit(0);
}

void
send_main(int sock)
{
    vc vcsock = lh_socket_from_os_handle(sock, "tcp", vcnil);
    vcsock.set_err_callback(backout);
    if(Req_enc)
    {
        if(!recv_crypto(vcsock))
            exit(0);
    }
    if(!recv_get_request(vcsock))
        exit(0);
    if(!send_request(vcsock, Filename, Filesize))
        exit(0);
    if(!recv_response(vcsock))
        exit(0);
    do_sendfile(vcsock);
    vc frok;
    if(!recv_vc(vcsock, frok) || frok != vc("frok"))
        dolog("no frok");
    else
        dolog("got frok");
    exit(0);
}

void
oopanic(const char *s)
{
    dolog(s);
    exit(0);
}

void
got_alarm(int)
{
    dolog("alarm");
    if(!Send)
    {
        if(Allow_restart)
        {
            char a[4096];
            sprintf(a, "recv fail %ld, %ld, %ld", Total_recv, Session_recv, (long)Filesize);
            dolog(a);
        }
        else
        {
            dolog("unlink");
            unlink((const char *)Filename);
        }
    }
    _exit(0);
}

int
main(int argc, char **argv)
{
    // assume we're started by inetd
    signal(SIGALRM, got_alarm);
    alarm(60);
    if(!(argc == 3 || argc == 4 || argc == 5))
        exit(1);

    if(chdir(argv[1]) != 0)
        exit(1);
    //dolog("starting");
    //dolog(argv[1]);
    //dolog(argv[2]);
    vc::non_lh_init();

#if 1
    struct rlimit r;
    r.rlim_cur = r.rlim_max = 20 * 1024 * 1024;
    setrlimit(RLIMIT_DATA, &r);
    setrlimit(RLIMIT_STACK, &r);
    setrlimit(RLIMIT_AS, &r);
    r.rlim_cur = r.rlim_max = 30;
    setrlimit(RLIMIT_NOFILE, &r);
    r.rlim_cur = r.rlim_max = 0;
    setrlimit(RLIMIT_CORE, &r);
    r.rlim_cur = r.rlim_max = 60;
    setrlimit(RLIMIT_CPU, &r);
#if 0
// surprise! if the xfer.log file gets too big, the process dies
// when it tries to log. need a better way of protecting ourselves
// against this problem.
    r.rlim_cur = r.rlim_max = MAXFILESIZE + (1024 * 1024);
    setrlimit(RLIMIT_FSIZE, &r);
#endif
#endif
    //close(2);
    //open("/tmp/st", O_CREAT|O_WRONLY|O_TRUNC, 0666);
#define TEST
    int s = 0;
#ifdef TEST
    int port = atoi(argv[3]);
    s = socket(PF_INET, SOCK_STREAM, 0);
    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(port);
    bind(s, (struct sockaddr *)&sa, sizeof(sa));
    listen(s, 5);
    socklen_t slen;
    s = accept(s, (struct sockaddr *)&sa, &slen);
#endif
    struct sockaddr_in in;
    socklen_t len = sizeof(in);
    if(getpeername(s, (struct sockaddr *)&in, &len) < 0)
        Peer = "unknown";
    else
    {
        Peer = inet_ntoa(in.sin_addr);
    }
    if(strcmp(argv[2], "send") == 0)
    {
        Send = 1;
        send_main(s);
    }
    if(strcmp(argv[2], "send2") == 0)
    {
        Send = 1;
        Allow_start = 1;
        send_main(s);
    }
    if(strcmp(argv[2], "send3") == 0)
    {
        Send = 1;
        Allow_start = 1;
        Req_enc = 1;
        send_main(s);
    }
    else if(strcmp(argv[2], "psend") == 0)
    {
        Send = 1;
        Loose_file = 1;
        send_main(s);
    }
    else if(strcmp(argv[2], "psend2") == 0)
    {
        // allows giving a start point for a download
        Send = 1;
        Loose_file = 1;
        Allow_start = 1;
        send_main(s);
    }
    else if(strcmp(argv[2], "psend3") == 0)
    {
        // allows giving a start point for a download
        Send = 1;
        Loose_file = 1;
        Allow_start = 1;
        Req_enc = 1;
        send_main(s);
    }
    else if(strcmp(argv[2], "recv") == 0)
    {
        recv_main(s);
    }
    else if(strcmp(argv[2], "recv3") == 0)
    {
        Req_enc = 1;
        recv_main(s);
    }
    else if(strcmp(argv[2], "recv4") == 0)
    {
        Req_enc = 1;
        Allow_restart = 1;
        recv_main(s);
    }
#if 0
    else if(strcmp(argv[2], "precv") == 0)
    {
        Loose_file = 1;
        recv_main(s);
    }
#endif
    exit(0);
}
