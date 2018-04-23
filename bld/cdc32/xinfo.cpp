
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <stdio.h>
#include "xinfo.h"
#include "vccomp.h"
#include "vcxstrm.h"
#include "fnmod.h"
#include "ta.h"
#include "vccrypt2.h"
#ifdef LINUX
#include <unistd.h>
#endif

int Dont_remove_bad_files = 1;

static int
backout(vc *)
{
    return 1;
}

int
save_info(vc info, const char *filename, int nomodify)
{
    DwString newfn;
    if(!nomodify)
    {
        if(!filename_modify(filename, newfn))
            return 0;
    }
    else
        newfn = filename;

    int rem = 0;
    vc_composite::new_dfs();
    vc f(VC_FILE);
    f.set_err_callback(backout);
    vc v = newfn.c_str();
    if(f.open(v, (vcfilemode)(VCFILE_WRITE|VCFILE_BINARY)))
    {
        vcxstream vcx(f);
        if(!vcx.open(vcxstream::WRITEABLE))
        {
            TRACK_ADD_nosave(SI_open, 1);
            goto err;
        }
        if(info.xfer_out(vcx) < 0)
        {
            TRACK_ADD_nosave(SI_xfer_out, 1);
            rem = 1;
            goto err;
        }
        if(!vcx.close(vcxstream::FLUSH))
        {
            TRACK_ADD_nosave(SI_flush, 1);
            rem = 1;
            goto err;
        }
        if(!f.close())
        {
            TRACK_ADD_nosave(SI_close, 1);
            rem = 1;
            goto err2;
        }
        return 1;
    }
    else
    {
        TRACK_ADD_nosave(SI_open_failed, 1);
        TRACK_ADD_VC_nosave(v, 1);
    }
err:
    f.close();
err2:
    // note: avoid possible partially written files
    // note: if the *open* fails, we don't remove, since it
    // might just be that the file is read-only, but otherwise ok.
    if(rem)
        unlink(newfn.c_str());
    return 0;
}

int
load_info(vc& info, const char *filename, int nomodify)
{
    DwString newfn;
    if(!nomodify)
    {
        if(!filename_modify(filename, newfn))
            return 0;
    }
    else
        newfn = filename;
    vc f(VC_FILE);
    int err = 0;
    f.set_err_callback(backout);
    vc v = newfn.c_str();
    if(!f.open(v, (vcfilemode)(VCFILE_READ|VCFILE_BINARY)))
        return 0;
    vcxstream vcx(f);
    if(!vcx.open(vcxstream::READABLE))
        goto err;
    if((err = info.xfer_in(vcx)) < 0)
        goto err;
    if(!vcx.close())
        goto err;
    if(!f.close())
        goto err2;
    return 1;
err:
    f.close();
err2:
    if(err == EXIN_PARSE && !Dont_remove_bad_files)
        unlink(newfn.c_str());
    return 0;
}

#define XKEY "\xaa\x1d\xf7\x42\x6f\xa0\x7a\x8f\x3c\x88\xc6\xd2\xac\xb7\xa5\xc0"
static char Key[] = XKEY;
static vc enc_ctx;
static void
init_xkey()
{
    enc_ctx = vclh_encdec_open();
    vclh_encdec_init_key_ctx(enc_ctx, vc(VC_BSTRING, Key, sizeof(Key) - 1), 0);
}

int
save_info_e(vc info, const char *filename, int nomodify)
{
    if(enc_ctx.is_nil())
        init_xkey();

    vc prfe = vclh_encdec_xfer_enc_ctx(enc_ctx, info);
    if(prfe.is_nil())
        return 0;

    if(!save_info(prfe, filename, nomodify))
        return 0;
    return 1;
}

int
load_info_e(vc& info, const char *filename, int nomodify)
{
    if(enc_ctx.is_nil())
        init_xkey();

    vc c;
    // note: prf_cache_name added any prefix already
    if(!load_info(c, filename, nomodify))
    {
        return 0;
    }

    vc prf;
    if(encdec_xfer_dec_ctx(enc_ctx, c, prf).is_nil())
    {
        return 0;
    }
    info = prf;
    return 1;
}
