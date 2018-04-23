
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// this little helper program creates a buildtime file from the current
// time (which is included in main.cpp usually).
// it also touches main.cpp to force makefiles to rebuild and link, in case
// the buildtime.h dependency isn't setup properly.)
//
#include <time.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	const char *ofn = "buildtime.h";
	if(argc == 3)
		ofn = argv[2];
    FILE *f = fopen(ofn, "w");
    if(!f)
    {
        return 1;
    }
    fprintf(f, "#define BUILDTIME \"%ld\"\n", time(0));
    fclose(f);
    const char *fn;
    if(argc >= 2)
	    fn = argv[1];
    else
	    fn = "main.cpp";
    f = fopen(fn, "r+b");
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
