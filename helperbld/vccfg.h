
/*
 * $Header: g:/dwight/repo/cdc32/rcs/vccfg.h 1.14 1999/01/10 16:10:54 dwight Checkpoint $
 */
#define VCLH_COMPRESS
#define DWYCO_NO_UVSOCK
// note: we don't use regex in typical clients (no need for it,
// and even if there was, i think the license prohibits it.)
#define DWYCO_NO_VCREGEX
// remove LH eval loop and some hacks related to that
#define NO_VCEVAL
#define VCLH_COMPRESS
#undef LHOBJ
