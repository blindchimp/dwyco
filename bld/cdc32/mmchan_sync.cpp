#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mmchan.h"
#include "netvid.h"

#include "qmsg.h"
#include "snds.h"
#include "dwrtlog.h"
#include "fnmod.h"
#include "se.h"
#include "dwscoped.h"
#include "qauth.h"

using namespace dwyco;

#ifdef LINUX
#include "miscemu.h"
#endif
#include "sepstr.h"

static
vc
file_to_string(DwString fn)
{
    int fd = open(fn.c_str(), O_RDONLY);
    if(fd == -1)
        return vcnil;
    DwString res;
    while(1)
    {
        char buf[32768];
        ssize_t num = read(fd, buf, sizeof(buf));
        if(num == -1)
        {
            close(fd);
            return vcnil;
        }
        if(num == 0)
            break;
        res += DwString(buf, num);
    }
    close(fd);
    return vc(VC_BSTRING, res.c_str(), res.length());
}

static
int
string_to_file(vc str, DwString fn)
{
    int fd = open(fn.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if(fd == -1)
        return 0;
    if(write(fd, (const char *)str, str.len()) != str.len())
    {
        close(fd);
        return 0;
    }
    close(fd);
    return 1;
}

vc
MMChannel::package_index()
{

    // to make things easy for testing, we'll just package up the
    // files directly. probably not the best idea, but ok for now.
    vc cmd(VC_VECTOR);
    cmd[0] = "idx";
    cmd[1] = file_to_string(newfn("mi.sql"));
    cmd[2] = file_to_string(newfn("fav.sql"));

    return cmd;


    return vcnil;
}

vc
MMChannel::package_next_cmd()
{
    if(sync_sendq.num_elems() == 0)
        return vcnil;
    vc cmd(VC_VECTOR);
    cmd[0] = "pull";
    cmd[1] = sync_sendq.get_first();
    sync_sendq.remove_first();
    return cmd;

    return vcnil;
}

void
MMChannel::unpack_index(vc cmd)
{
    if(cmd[0] != vc("idx"))
        return;
    DwString mifn("mi%1.sql");
    DwString favfn("fav%1.sql");

    vc huid = to_hex(remote_uid());
    mifn.arg((const char *)huid);
    favfn.arg((const char *)huid);
    mifn = newfn(mifn);
    favfn = newfn(favfn);
    string_to_file(cmd[1], mifn);
    string_to_file(cmd[2], favfn);
}

void
MMChannel::process_pull(vc cmd)
{
    if(cmd[0] != vc("pull"))
        return;
    vc mid = cmd[1];
    // load the msg and attachment, and send it back as a "pull-resp"

}

void
MMChannel::process_pull_resp(vc)
{

}

void
MMChannel::send_pull(vc mid)
{
    sync_sendq.append(mid);
}

int
MMChannel::process_outgoing_sync()
{
    if(mms_sync_state == MMSS_ERR)
        return 0;
    if(!tube->can_write_data(msync_chan))
    {
        return 0;
    }

    int err;

    vc vcx;
    if(mms_sync_state == SEND_INIT)
    {
        vcx = package_index();
    }
    else if(mms_sync_state == MMSS_TRYAGAIN)
    {
        vcx = vcnil;
    }
    else if(mms_sync_state == NORMAL_SEND)
    {
        vcx = package_next_cmd();
        if(vcx.is_nil())
            return 0;
    }
    else
    {
        oopanic("bad mm state");
    }

    if((err = tube->send_data(vcx, msync_chan, VIDEO_CHANNEL)) == SSERR)
    {
        drop_subchannel(msync_chan);
        msync_chan = -1;
        mms_sync_state = MMSS_ERR;
        return 0;
    }
    else if(err == SSTRYAGAIN)
    {
        mms_sync_state = MMSS_TRYAGAIN;
        return 0;
    }

    mms_sync_state = NORMAL_SEND;

    sproto *s = simple_protos[msync_chan];
    s->timeout.load(VIDEO_IDLE_TIMEOUT);
    s->timeout.start();

    GRTLOG("msync output ok", 0, 0);

    return 1;
}

int
MMChannel::process_incoming_sync()
{
    int ret;

    vc rvc;
    int len;
    if((ret = tube->recv_data(rvc, msync_chan)) >= 0)
    {
        if(rvc.type() == VC_VECTOR)
        {
            // drop pings
            if(rvc[0] == vc("!"))
                return 1;
            if(mmr_sync_state == RECV_INIT)
            {
                unpack_index(rvc);
                mmr_sync_state = NORMAL_RECV;
            }
            else if(mmr_sync_state == NORMAL_RECV)
            {
                if(rvc[0] == vc("pull"))
                    process_pull(rvc);
                else if(rvc[0] == vc("pull-resp"))
                    process_pull_resp(rvc);
            }
            return 1;
        }
        else
            ret = SSERR;
    }

    if(ret == SSERR)
    {
        drop_subchannel(msync_chan);
        msync_chan = -1;
    }
    return 0;
}
