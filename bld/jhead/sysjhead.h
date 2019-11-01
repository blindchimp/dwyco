#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

//--------------------------------------------------------------------------

#ifdef _WIN32
    #include <sys/utime.h>

    // Make the Microsoft Visual c 10 deprecate warnings go away.
    // The _CRT_SECURE_NO_DEPRECATE doesn't do the trick like it should.
    #define unlink _unlink
    #define chmod _chmod
    #define access _access
    #define mktemp _mktemp
#else
    #include <utime.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <errno.h>
    #include <limits.h>
#endif
