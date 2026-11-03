#if defined(ANDROID) || defined(_Windows) || defined(EMSCRIPTEN)
#define DWYCO_NO_UVSOCK
#endif
#define DWYCO_NO_VCREGEX
#ifndef NO_VCEVAL
#define NO_VCEVAL
#endif
#undef LHOBJ
