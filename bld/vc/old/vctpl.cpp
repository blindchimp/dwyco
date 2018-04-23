
/*
 * $Header: g:/dwight/repo/vc/rcs/vctpl.cpp 1.31 1998/12/18 23:57:19 dwight Exp $
 */
#ifdef VCCFG_FILE
#include "vccfg.h"
#endif
#ifdef VCSOCK
class vc_winsock;
unsigned long hash(vc_winsock*);
#endif
// template implementation stuff for gcc
#ifdef __GNUG__
#define GNUTPL_IMPL
#pragma implementation "dwamap.h"
#pragma implementation "dwassoc.h"
#pragma implementation "dwbagr.h"
#pragma implementation "dwhash.h"
#pragma implementation "dwiter.h"
#pragma implementation "dwlista.h"
#pragma implementation "dwmapr.h"
#pragma implementation "dwmaps.h"
#pragma implementation "dwqmap3.h"
#pragma implementation "dwsetr.h"
#pragma implementation "dwvec.h"
#pragma implementation "dwvecp.h"
#pragma implementation "useful.h"
#pragma implementation "dwbag.h"
#pragma implementation "dwset.h"


#include "dwamap.h"
#include "dwassoc.h"
#include "dwbagr.h"
#include "dwhash.h"
#include "dwiter.h"
#include "dwlista.h"
#include "dwmapr.h"
#include "dwmaps.h"
#include "dwqmap3.h"
#include "dwsetr.h"
#include "dwvec.h"
#include "dwvecp.h"
#include "useful.h"
#include "dwset.h"
#include "dwbag.h"

#pragma implementation "dwgrows.h"
#pragma implementation "dwioh.h"
#pragma implementation "vc.h"
#pragma implementation "vcatomic.h"
#pragma implementation "vccfun.h"
#pragma implementation "vccomp.h"
#pragma implementation "vcctx.h"
#pragma implementation "vccvar.h"
#pragma implementation "vcdbg.h"
#pragma implementation "vcdbl.h"
#pragma implementation "vcdecom.h"
#pragma implementation "vcdeflt.h"
#pragma implementation "vcenco.h"
#pragma implementation "vcexcctx.h"
#pragma implementation "vcfac.h"
#pragma implementation "vcfext.h"
#pragma implementation "vcfile.h"
#pragma implementation "vcfunc.h"
#pragma implementation "vcfuncal.h"
#pragma implementation "vcfunctx.h"
#pragma implementation "vcfundef.h"
#pragma implementation "vcint.h"
#pragma implementation "vclex.h"
#pragma implementation "vclhfac.h"
#pragma implementation "vclhnet.h"
#pragma implementation "vclhsys.h"
#pragma implementation "vcmap.h"
#pragma implementation "vcmath.h"
#pragma implementation "vcmemfun.h"
#pragma implementation "vcmemsel.h"
#pragma implementation "vcnil.h"
#pragma implementation "vcobj.h"
#pragma implementation "vcobjfun.h"
#pragma implementation "vcregex.h"
#pragma implementation "vcsc.h"
#pragma implementation "vcsfile.h"
#pragma implementation "vcsock.h"
#pragma implementation "vcwsock.h"
#pragma implementation "vcsrc.h"
#pragma implementation "vcstr.h"
#pragma implementation "vcxstrm.h"

#include "dwgrows.h"
#include "dwioh.h"
#include "vc.h"
#include "vcatomic.h"
#include "vccfun.h"
#include "vccomp.h"
#include "vcctx.h"
#include "vccvar.h"
#include "vcdbg.h"
#include "vcdbl.h"
#include "vcdecom.h"
#include "vcdeflt.h"
#include "vcenco.h"
#include "vcexcctx.h"
#include "vcfac.h"
#include "vcfext.h"
#include "vcfile.h"
#include "vcfunc.h"
#include "vcfuncal.h"
#include "vcfunctx.h"
#include "vcfundef.h"
#include "vcint.h"
#include "vclex.h"
#include "vclhfac.h"
#include "vclhnet.h"
#include "vclhsys.h"
#include "vcmap.h"
#include "vcmath.h"
#include "vcmemfun.h"
#include "vcmemsel.h"
#include "vcnil.h"
#include "vcobj.h"
#if 0
#include "vcobjfun.h"
#endif
#include "vcregex.h"
#include "vcsc.h"
#include "vcsfile.h"
#include "vcsock.h"
#include "vcwsock.h"
#include "vcsrc.h"
#include "vcstr.h"
#include "vcxstrm.h"

#ifdef ANSI_INSTANTIATE
template class DwAMap<vc,vc>;
template class DwAMapIter<vc,vc>;
template class DwIter<DwMaps<vc, vc>, DwAssocImp<vc, vc> >;
template class DwMapR<vc,vc>;
template class DwMapRIter<vc,vc>;
template class DwBagR<vc>;
template class DwBagR<DwAssocImp<vc, vc> >;
//template class DwBagRIter<DwBagR<DwAssocImp<vc, vc> >, DwAssocImp<vc, vc> >;
template class DwBagRIter<vc>;
template class DwIter<DwBagR<vc>, vc>;
template class DwBag<vc>;
template class DwBagIter<vc>;
template class DwIter<DwBag<vc>, vc>;
template class DwSet<vc>;
template class DwSetIter<vc>;
template class DwIter<DwSet<vc>, vc>;
template class DwAssocImp<vc,vc>;
template class DwVec<vc>;
template class DwVecIter<vc>;
template class DwIter<DwVec<vc>, vc>;
#ifdef OLD_TREE
template class DwTree<vc,vc>;
template class DwTreeIter<vc, vc>;
template class DwTreeIterPre<vc, vc>;
template class DwIter<DwTree<vc, vc>, DwAssocImp<vc, vc> > ;
#else
template class DwTreeKaz<vc,vc>;
template class DwTreeKazIter<vc, vc>;
template class DwIter<DwTreeKaz<vc, vc>, DwAssocImp<vc, vc> > ;
#endif
#ifdef VCDBG
template class DwVecP<VcDebugNode>;
#endif
template class DwVecP<excctx>;
template class DwVecP<functx>;
template class DwVecP<excfun>;
template class DwVec<char>;
template class DwIter<DwListA<TableElem>, TableElem>;
template class DwBagRIter<DwAssocImp<vc, vc> >;
template class DwIter<DwBagR<DwAssocImp<vc,vc> >, DwAssocImp<vc,vc> >;
template class DwVec<void *>;
template class DwIter<DwVec<void *>, void *>;
template class DwIter<DwListA<vc_composite *>, vc_composite *>;
template class DwIter<DwListA<vc_funcall *>, vc_funcall *>;
template class DwIter<DwVec<DwListA<TableElem> >, DwListA<TableElem> >;
template class DwIter<DwVec<char>, char>;
template class DwMapsIter<vc,vc>;
template class DwVecP<char>;
template class DwVec<int>;
#ifdef VCSOCK
template class DwBag<vc_winsock *>;
template class DwSet<vc_winsock *>;
template class DwBagIter<vc_winsock *>;
template class DwSetIter<vc_winsock *>;
template class DwIter<DwBag<vc_winsock *>, vc_winsock *>;
template class DwIter<DwSet<vc_winsock *>, vc_winsock *>;
#endif
#if 0
#ifdef VCHOMOV
template class DwVec<long>;
template class DwVec<double>;
#endif
#endif
template class DwListA<DwIOHandler *>;
template class DwListAIter<DwIOHandler *>;
template class DwIter<DwListA<DwIOHandler *>, DwIOHandler *>;
template class DwListA<vc_composite *>;
template class DwListA<vc>;
template class DwListA<vc_funcall *>;
template class DwListAIter<vc>;
template class DwIter<DwListA<vc>, vc>;
#ifdef VCSOCK
template class DwVecP<vc_winsock>;
template class DwVecP<vc_default>;
#endif

#ifdef LHPROF
unsigned long hash(vc_func * const &);
template class DwSetIter<vc_func *>;
template class DwBagIter<vc_func *>;
template class DwBag<vc_func *>;
template class DwSet<vc_func *>;
template class DwIter<DwBag<vc_func *>, vc_func *>;
#endif

#if 0
template int max(int, int);
template long max(long, long);
#endif

#else
typedef DwAMap<vc,vc> t1;
typedef DwAMapIter<vc,vc> t1a;
typedef DwAssocImp<vc,vc> t2;
typedef DwVec<vc> t3;
#ifdef VCDBG
typedef DwVecP<VcDebugNode> t4;
#endif
typedef DwVecP<excctx> t5;
typedef DwVecP<functx> t6;
typedef DwVec<char> t7;
#if 0
#ifdef VCHOMOV
typedef DwVec<long> t7a;
typedef DwVec<double> t7b;
#endif
#endif
typedef DwListA<DwIOHandler *> t8;
typedef DwListA<vc_composite *> t9;
typedef DwListA<vc> t10;
typedef DwBag<vc> t11;
typedef DwSet<vc> t12;
#if 0
static
dummy()
{
	max(1, 1);
	max(1L, 1L);
}
#endif
#endif

#endif
