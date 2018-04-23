#ifndef CSTRING_H
#define CSTRING_H
#ifdef USE_SYSTEM_CSTRING
#include <cstring.h>
#else
#include "dwstr.h"
#define string DwString
#endif
#endif
