
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//---------------------------------------------------------------------------

#include "vc.h"
#include "mmchan.h"
#include "qdirth.h"
#include "qauth.h"
#include "cryptlib.h"
#include "sha.h"
using namespace CryptoPP;
#include "filters.h"
#include "files.h"
#include "vccrypt2.h"
#include "vclhsys.h"
#include "dwstr.h"
#include "autoup.h"
#include "fnmod.h"
#if defined(MACOSX) || defined(LINUX)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#include "qmsg.h"

#define EXENAME "setup.exe"
#ifdef _Windows
#define DOWNLOADER_NAME "xferdl.exe"
#else
#define DOWNLOADER_NAME "xferdl"
#endif

#include "sepstr.h"

TAutoUpdate::TAutoUpdate() : vp(this)
{
    user_status_callback = 0;
    user_scb_arg1 = 0;
    user_eo_xfer = 0;
    xfer_channel = 0;
    compulsory = 0;
    done_background = 0;
}


static
int
run_patch(vc file)
{
    //DeleteFile(newfn(EXENAME).c_str());
    if(!move_replace(newfn(file), newfn(EXENAME)))
        return 0;
    // be sure to add directory to avoid
    // funky searches.
    DwString a((const char *) EXENAME);
    DwString b = "." DIRSEPSTR;
    b += a;
    b = newfn(b);
    VCArglist al;
#ifdef LINUX
    // create a shared fd that will close when
    // we exit.
    int fds[2];
    if(pipe(fds) == -1)
        return 0;
    // close it for the eventual child process. we'll
    // be the only reference to the write end.
    // when we close, the read end should unblock with eof
    fcntl(fds[1], F_SETFD, FD_CLOEXEC);
    al.append(fds[0]);
    al.append(b.c_str());
    char s[100];
    sprintf(s, "%d", fds[0]);
    al.append(s);
    chmod(b.c_str(), 0700);
#else
    al.append(0);
    al.append(b.c_str());
#endif
    // note: the autoupdate process really assumes you are
    // in the CWD where it needs to apply the patch. this
    // might be a problem if the various prefixes in fnmod
    // are set to a bunch of different places.
    vc pid;
#ifdef ANDROID
    oopanic("autoupdate not supported yet");
#else
    pid = vclh_process_create(&al);
#endif
    if(pid.is_nil())
        return 0;

    return 1;
}

//
// simple auto update stuff
// just download a patch exe
// from a special port on the
// server and then verify the
// signature on it, and run it to
// do the update.
//

static void
eo_xfer(MMChannel *mc, vc m, void *, ValidPtr vp)
{
    if(!vp.is_valid())
        return;
    TAutoUpdate *q = (TAutoUpdate *)(void *)vp;
    if(!mc->xfer_failed)
    {
        if(!q)
            return;
        q->xfer_channel = 0;
        // check the signature on the file

        vclh_dsa_pub_init(newfn("dsadwyco.pub").c_str());

        SHA hash;
        BufferedTransformation *bt = new HashFilter(hash);
        FileSource *file = new FileSource(newfn((const char *)mc->remote_filename).c_str(), TRUE, bt);
        int n = bt->MaxRetrievable();
        vc val;
        if(n != 20)
            val = vc("");
        else
        {
            byte a[20];
            bt->Get(a, sizeof(a));
            vc v(VC_BSTRING, (const char *)a, sizeof(a));
            val = v;
        }
        vc valid = vclh_dsa_verify(val, q->patch[6]);
        delete file;
        if(valid.is_nil())
        {
            DeleteFile(newfn((const char *)mc->remote_filename).c_str());
            if(q->user_eo_xfer)
                (*q->user_eo_xfer)(DWYCO_AUTOUPDATE_SIGNATURE_FAILED);
            return;
        }
    }
    else
    {
        DeleteFile(newfn((const char *)mc->remote_filename).c_str());
    }

    if(q)
    {
        q->xfer_channel = 0;
        if(mc->xfer_failed)
        {
            if(q->user_eo_xfer)
                (*q->user_eo_xfer)(DWYCO_AUTOUPDATE_DOWNLOAD_FAILED);
            return;
        }
    }
    if(!run_patch(mc->remote_filename))
    {
        if(q)
        {
            if(q->user_eo_xfer)
                (*q->user_eo_xfer)(DWYCO_AUTOUPDATE_RUN_FAILED);
            return;
        }
    }
    if(q)
    {
        if(q->user_eo_xfer)
            (*q->user_eo_xfer)(DWYCO_AUTOUPDATE_IN_PROGRESS);
    }
}

static void
set_status(MMChannel *mc, vc msg, void *, ValidPtr vp)
{
    if(!vp.is_valid())
        return;

    TAutoUpdate *q = (TAutoUpdate *)(void *)vp;
    if(q && q->user_status_callback)
    {
        int e = mc->expected_size;
        if(e == 0)
            e = 1;
        int p = (int)(((double)mc->total_got * 100) / e);
        (*q->user_status_callback)(mc->myid, msg, p, q->user_scb_arg1);
    }
}


int TAutoUpdate::fetch_update(DwycoStatusCallback scb, void *scb_arg1,
                              DwycoAutoUpdateDownloadCallback cb)
{
    // vector(desc type file version cur-hash new-hash vector(ip port))
    // quick check to see if we have actually got something
    // from the server that looks like a patch vector.
    // avoid crashing. note: this needs a better interface so
    // this function is disabled unless we have actually talked to the
    // server about autoupdates.
    if(patch.type() != VC_VECTOR ||
            patch[10].type() != VC_VECTOR)
        return 0;
    MMChannel *mc;
    vc patch_name = patch[2];
    user_status_callback = scb;
    user_scb_arg1 = scb_arg1;
    user_eo_xfer = cb;

    if(!(mc = fetch_attachment(patch_name, eo_xfer, vcnil, 0, vp,
                               set_status, 0, vp, patch[10][0], patch[10][1])))
    {
        return 0;
    }
    xfer_channel = mc;
    return 1;
}

// note: this function leaks a little bit, but it is only
// called once or twice, so no big deal
static int
check_staged_update(DwString fn, vc sig)
{
    FILE *f = fopen(fn.c_str(), "rb");
    if(!f)
        return 0;

    SHA1 sha;
    byte *hash = new byte[sha.DigestSize()];
    while(!feof(f))
    {
        char buf[2048];
        size_t n;
        if((n = fread(buf, 1, sizeof(buf), f)) != sizeof(buf))
        {
            if(ferror(f))
            {
                fclose(f);
                DeleteFile(fn.c_str());
                return 0;
            }
        }
        sha.Update((byte *)buf, n);
    }
    fclose(f);
    sha.Final(hash);
    DwString chk(fn);
    chk += ".chk";
    FILE *f2 = fopen(chk.c_str(), "rb");
    if(!f2)
    {
        // if there is no .chk file, that might just mean that
        // xferdl hasn't finished transferring it yet, so we just
        // return, maybe allowing a partial transfer to complete.
        return 0;
    }
    byte *hash2 = new byte[sha.DigestSize()];
    if(fread(hash2, 1, sha.DigestSize(), f2) != sha.DigestSize())
    {
        fclose(f2);
        DeleteFile(chk.c_str());
        DeleteFile(fn.c_str());
        return 0;
    }
    fclose(f2);
    if(memcmp(hash, hash2, sha.DigestSize()) != 0)
    {
        DeleteFile(chk.c_str());
        DeleteFile(fn.c_str());
        return 0;
    }

    vclh_dsa_pub_init(newfn("dsadwyco.pub").c_str());
    vc valid = vclh_dsa_verify(vc(VC_BSTRING, (const char *)hash, sha.DigestSize()), sig);
    if(valid.is_nil())
    {
        DeleteFile(chk.c_str());
        DeleteFile(fn.c_str());
        return 0;
    }

#if 0
    DwString sigfn(fn);
    sigfn += ".sig";
    FILE *f3 = fopen(sigfn.c_str(), "wb");
    if(!f3)
        return 0;
    if(fwrite((const char *)sig, 1, sig.len(), f3) != sig.len())
    {
        fclose(f3);
        return 0;
    }
    if(fclose(f3) != 0)
        return 0;
#endif

    return 1;

}

int
TAutoUpdate::run()
{
    DwString file = (const char *)patch[2];
    if(!check_staged_update(file, patch[6]))
        return 0;
    return run_patch(file.c_str());

}

int TAutoUpdate::fetch_update_background()
{
    if(done_background)
        return 1;

    // vector(desc type file version cur-hash new-hash sig nil nil nil vector(ip port))
    // quick check to see if we have actually got something
    // from the server that looks like a patch vector.
    // avoid crashing. note: this needs a better interface so
    // this function is disabled unless we have actually talked to the
    // server about autoupdates.

    // this is an update that is being downloaded in the background
    // and the download may continue after the main process has finished.
    // the next time we start up, we check for the existence of the
    // update and run it then.
    // so, there is no progress indicator or anything for the user to worry
    // about here.

    // note: not a good idea to mix this with the other style of update.
    // should probably just disable one or the other after one of these fetch
    // calls.

    if(patch.type() != VC_VECTOR ||
            patch[10].type() != VC_VECTOR)
        return 0;

    vc patch_name = patch[2];
    if(check_staged_update((const char *)patch_name, patch[6]))
        return 2;

    DwString a((const char *) DOWNLOADER_NAME);
    DwString b = "." DIRSEPSTR;
    b += a;
    b = newfn(b);

    DwString srv((const char *)patch[10][0]);
    srv += ":";
    srv += patch[10][1].peek_str();

    VCArglist al;
    // voodoo, first arg is fd of socket to keep open, and -2
    // means close all sockets. -1 means close no sockets.
    // sheesh
    al.append(-2);
    al.append(b.c_str());
    al.append(srv.c_str());
    al.append(patch_name);
    al.append(to_hex(patch[6]));

    // note: the autoupdate process really assumes you are
    // in the CWD where it needs to apply the patch. this
    // might be a problem if the various prefixes in fnmod
    // are set to a bunch of different places.
    vc pid;
#ifdef ANDROID
    oopanic("autoupdate not supported yet");
#else
    pid = vclh_process_create(&al);
#endif
    if(pid.is_nil())
        return 0;
    done_background = 1;
    return 1;
}
//---------------------------------------------------------------------------



void TAutoUpdate::abort_fetch()
{

    if(xfer_channel != 0)
    {
        xfer_channel->schedule_destroy(MMChannel::HARD);
        xfer_channel = 0;
    }
}
