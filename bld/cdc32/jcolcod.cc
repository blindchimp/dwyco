
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef _Windows
#include <winsock2.h>
#endif
#ifdef LINUX
#include <arpa/inet.h>
#endif
#include "jcolcod.h"
#include "jtcode.h"
#include "chroma.h"

JPEGCoderColor::JPEGCoderColor()
{
    JPEGTCoder *jt = new JPEGTCoder;
    luma = jt;
    cr = new JPEGTCoderChroma(jt);
    cb = new JPEGTCoderChroma(jt);
}

JPEGCoderColor::~JPEGCoderColor()
{
    delete luma;
    delete cr;
    delete cb;
}

//
// this is a little funky...
// instead of incurring an extra set of
// copy operations by calling final package
// on the chroma codecs, we call the luma
// packager and then do the chroma packaging
// here, cuz it is so simple (there is no
// chroma control info, it is all derived
// from the luma codec)
//
DWBYTE *
JPEGCoderColor::final_package(int& len_out)
{
    JPEGTCoder *jluma = (JPEGTCoder *)luma;
    JPEGTCoderChroma *jcr = (JPEGTCoderChroma *)cr;
    JPEGTCoderChroma *jcb = (JPEGTCoderChroma *)cb;

    int l_len;
    DWBYTE *lb = jluma->final_package(l_len);
    // since we are building a chroma packet,
    // go ahead and fill in the offset that
    // is normally 0 for jluma only packets.
    if(sampling != SAMP400)
    {
        *(unsigned short *)(lb + CHROMA_OFFSET) = htons(l_len);

        // put in sampling style
        *(unsigned char *)lb |= ((sampling == SAMP411 || sampling == SAMPYUV9) << 4);
    }
    else
    {
        // jluma only
        *(unsigned short *)(lb + CHROMA_OFFSET) = 0;
        len_out = l_len;
        return lb;
    }

    //
    // get the chroma lens
    //
    int cr_len = (DWBYTE *)jcr->frame_buf - (DWBYTE *)jcr->coded_frame;
    int cb_len = (DWBYTE *)jcb->frame_buf - (DWBYTE *)jcb->coded_frame;
//printf("y %d jcb %d jcr %d\n", l_len, cb_len, cr_len);

    //
    // get a whole new buffer and assemble the whole thing
    // first short of chroma buffer is offset to jcr info
    //

    int len = l_len + cr_len + cb_len + CHROMA_HEADER_SZ;
    DWBYTE *buf = new DWBYTE[len];
    DWBYTE *nb = buf;
    bcopy(lb, nb, l_len);
    delete [] lb;
    nb += l_len;
    *(unsigned short *)nb = htons(l_len + CHROMA_HEADER_SZ + cb_len);
    nb += sizeof(unsigned short);
    bcopy(jcb->coded_frame, nb, cb_len);
    nb += cb_len;
    bcopy(jcr->coded_frame, nb, cr_len);

    len_out = len;
    return buf;
}
