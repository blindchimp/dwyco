
/*
 * $Header: g:/dwight/repo/cdc32/rcs/vccfg.h 1.14 1999/01/10 16:10:54 dwight Checkpoint $
 */

#if defined(ANDROID) || defined(DWYCO_IOS) || defined(_Windows) || defined(EMSCRIPTEN)
#define DWYCO_NO_UVSOCK
#endif

#define DWYCO_NO_VCREGEX
// remove LH eval loop
#define NO_VCEVAL
#undef LHOBJ
