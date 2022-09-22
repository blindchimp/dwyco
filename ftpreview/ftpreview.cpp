
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "dlli.h"
#include <stdlib.h>
#include <stdio.h>
#include "dwstr.h"

namespace dwyco {
void simple_init_codec(const char *);
}

int
main(int argc, char **argv)
{
    if(argc != 2)
	exit(1);
    dwyco::simple_init_codec("log.out");
    DwString out(argv[1]);
    out += ".ppm";
    int viewid = dwyco_make_zap_view_file_raw(argv[1]);
    int has_video;
    
    if(!dwyco_zap_quick_stats_view(viewid, &has_video, 0, 0))
        exit(1);
    if(!has_video)
    {
        fprintf(stderr, "no video\n");
        exit(1);
    }
    if(!dwyco_zap_create_preview(viewid, out.c_str(), out.length()))
    {
        fprintf(stderr, "can't create preview");
        exit(1);
    }
    exit(0);
}

