
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef ZCOMP_H
#define ZCOMP_H

vc vclh_compression_open(const vc& level);
vc vclh_compression_close(VCArglist *a);
vc vclh_decompression_open();
vc vclh_decompression_close(VCArglist *a);
vc vclh_compress(const vc& ctx, const vc& str);
vc vclh_decompress(const vc& ctx, const vc& str);
vc vclh_decompress_xfer(vc ctx, vc v, vc& out);
vc vclh_compress_xfer(VCArglist *a);

vc lh_decompress_xfer(const vc& ctx, const vc& v, const vc& out);

#endif

