
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef ZCOMP_H
#define ZCOMP_H

vc vclh_compression_open(vc level);
vc vclh_compression_close(vc ctx);
vc vclh_decompression_open();
vc vclh_decompression_close(vc ctx);
vc vclh_compress(vc ctx, vc str);
vc vclh_decompress(vc ctx, vc str);
vc vclh_decompress_xfer(vc ctx, vc v, vc& out);
vc vclh_compress_xfer(vc ctx, vc v);

vc lh_decompress_xfer(vc ctx, vc v, vc out);

#endif

