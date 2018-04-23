
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "filetube.h"
#include "imgmisc.h"
#include "vc.h"
#include "mmchan.h"
#include "dwrtlog.h"
#include "theoracol.h"
#include "tpgmdec.h"

void
oopanic(const char *)
{
    ::abort();
}
void
oopanic(char *)
{
    ::abort();
}

int
create_preview(DwString vidfile, const char *filename, int len_filename)
{
    if(vidfile.length() == 0)
    {
        GRTLOG("create_preview_view: %d no media to probe", 0, 0);
        return 0;
    }
    if(!access(vidfile.c_str(), 0) == 0)
    {
        GRTLOG("create_preview_view: %d cant access media", 0, 0);
        return 0;
    }
    SlippyTube st(vidfile, "rb", FileTube::SOURCE);
    DWBYTE *vidbuf;
    int len_vidbuf;
    int dummy;
    int codec;
    CDCTheoraDecoderColor new_decoder;
    TPGMMSWDecoderColor old_decoder;
    DwDecoderColor *decoder = 0;
    void *vimg;
    int rows;
    int cols;
    DWBYTE *orig_vidbuf;
    // ok, this is a bit hokey.
    // we really need to be able to tell it to give us the next
    // usable key frame (which may not be the first one in the
    // file). for now, we just scan for it using the functions
    // we currently have.
    int frm;
    for(int frm = 1; frm < 20; ++frm)
    {
        st.quick_stats(-1, frm, -1, dummy, dummy, dummy, codec, &vidbuf, &len_vidbuf);
        if(vidbuf == 0)
        {
            GRTLOG("create_preview_view: %d no video in media", 0, 0);
            return 0;
        }
        if(MMChannel::codec_number_to_name(codec) == vc("theora"))
        {
            decoder = &new_decoder;
        }
        else if(MMChannel::codec_number_to_name(codec) == vc("dct"))
        {
            decoder = &old_decoder;
        }
        else
        {
            GRTLOG("create_preview_view: %d not theora/dct vid", 0, 0);
            delete [] vidbuf;
            return 0;
        }
        orig_vidbuf = vidbuf;
        decoder->decode_from_stream(vidbuf, len_vidbuf, vimg, cols, rows);
        if(vimg != 0)
        {
            break;
        }
    }
    if(frm == 20)
    {
        GRTLOG("create_preview_view: %d didn't find keyframe in first 20 frames", 0, 0);
        return 0;
    }
    // due to windows braindamage, this needs to be flipped and
    // red-blue swapped
    pixel **px = (pixel **)vimg;
    flip_in_place(px, cols, rows);
    for(int r = 0; r < rows; ++r)
    {
        for(int c = 0; c < cols; ++c)
        {
            pixel pp = px[r][c];
            PPM_ASSIGN(px[r][c], PPM_GETB(pp), PPM_GETG(pp), PPM_GETR(pp));
        }
    }

    FILE *f = fopen(DwString(filename, 0, len_filename).c_str(), "wb");
    if(!f)
    {
        GRTLOG("create_preview_view: %d can't open %s", 0, filename);
        delete [] orig_vidbuf;
        return 0;
    }
    // duh, no error checking
    ppm_writeppm(f, (pixel **)vimg, cols, rows, PPM_MAXMAXVAL, 0);
    fclose(f);
    delete [] orig_vidbuf;
    ppm_freearray(vimg, rows);
    return 1;
}

int
main(int, char **argv)
{
    void simple_init_codec(const char *);
    simple_init_codec("log.out");
    DwString out(argv[1]);
    out += ".ppm";
    if(create_preview(argv[1], out.c_str(), out.length()))
        exit(0);
    exit(1);
}

