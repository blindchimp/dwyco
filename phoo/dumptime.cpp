
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <time.h>
#include <stdio.h>

int
main(int, char **)
{
    FILE *f = fopen("buildtime.h", "w");
    if(!f)
        return 1;
    fprintf(f, "#define BUILDTIME \"%ld\"\n", time(0));
    fclose(f);
    f = fopen("main.cpp", "r+b");
    if(!f)
        return 1;
    int c;
    if((c = fgetc(f)) == EOF)
        return 1;
    rewind(f);
    if(fputc(c, f) == EOF)
        return 1;
    fclose(f);
    return 0;
}
