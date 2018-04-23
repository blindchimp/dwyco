
#include "vc.h"
#include "dwvecp.h"

static DwVecP<void> Arg_stk;
template class DwVecP<void>;
static
int
start_wrapper()
{
	return Arg_stk.num_elems();
}

static
void
end_wrapper(int i)
{
	int j = Arg_stk.num_elems();
	for(int k = j - 1; k >= i; --k)
		free(Arg_stk[k]);
	Arg_stk.set_size(i);
}

static void
push_ptr(void *p)
{
	Arg_stk.append(p);
}

#define START_WRAP int i = start_wrapper();
#define WRAP_END end_wrapper(i);

#include <cdk.h>
#include "hacked_cvt_cdk.xml.cpp"

	static vc
wrap_drawShadow(VCArglist *a)
{
START_WRAP
	drawShadow(cvt_vc_p__win_st_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_attrbox(VCArglist *a)
{
START_WRAP
	attrbox(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]),
cvt_vc__long_unsigned_int((*a)[5]),
cvt_vc__long_unsigned_int((*a)[6]),
cvt_vc__long_unsigned_int((*a)[7]));
WRAP_END
return(vcnil);

}
static vc
wrap_writeChtypeAttrib(VCArglist *a)
{
START_WRAP
	writeChtypeAttrib(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_long_unsigned_int((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc__int((*a)[7]));
WRAP_END
return(vcnil);

}
static vc
wrap_writeChtype(VCArglist *a)
{
START_WRAP
	writeChtype(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_long_unsigned_int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6]));
WRAP_END
return(vcnil);

}
static vc
wrap_writeCharAttrib(VCArglist *a)
{
START_WRAP
	writeCharAttrib(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc__int((*a)[7]));
WRAP_END
return(vcnil);

}
static vc
wrap_writeChar(VCArglist *a)
{
START_WRAP
	writeChar(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawLine(VCArglist *a)
{
START_WRAP
	drawLine(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__long_unsigned_int((*a)[5]));
WRAP_END
return(vcnil);

}
static vc
wrap_boxWindow(VCArglist *a)
{
START_WRAP
	boxWindow(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKViewer(VCArglist *a)
{
START_WRAP
	destroyCDKViewer(cvt_vc_p_SViewer_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKViewer(VCArglist *a)
{
START_WRAP
	positionCDKViewer(cvt_vc_p_SViewer_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKViewer(VCArglist *a)
{
START_WRAP
	moveCDKViewer(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKViewer(VCArglist *a)
{
START_WRAP
	eraseCDKViewer(cvt_vc_p_SViewer_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKViewer(VCArglist *a)
{
START_WRAP
	drawCDKViewer(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKViewerBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKViewerBackgroundColor(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKViewerBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKViewerBoxAttribute(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKViewerHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKViewerHorizontalChar(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKViewerVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKViewerVerticalChar(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKViewerLRChar(VCArglist *a)
{
START_WRAP
	setCDKViewerLRChar(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKViewerLLChar(VCArglist *a)
{
START_WRAP
	setCDKViewerLLChar(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKViewerURChar(VCArglist *a)
{
START_WRAP
	setCDKViewerURChar(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKViewerULChar(VCArglist *a)
{
START_WRAP
	setCDKViewerULChar(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKViewerBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKViewerBox(cvt_vc_p_SViewer_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKViewerBox(VCArglist *a)
{
START_WRAP
	setCDKViewerBox(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKViewerInfoLine(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKViewerInfoLine(cvt_vc_p_SViewer_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKViewerInfoLine(VCArglist *a)
{
START_WRAP
	setCDKViewerInfoLine(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKViewerHighlight(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKViewerHighlight(cvt_vc_p_SViewer_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKViewerHighlight(VCArglist *a)
{
START_WRAP
	setCDKViewerHighlight(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKViewerTitle(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pp_long_unsigned_int(getCDKViewerTitle(cvt_vc_p_SViewer_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKViewerTitle(VCArglist *a)
{
START_WRAP
	setCDKViewerTitle(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKViewerInfo(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pp_long_unsigned_int(getCDKViewerInfo(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc_p_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_setCDKViewerInfo(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(setCDKViewerInfo(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_setCDKViewer(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(setCDKViewer(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc_pp_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc__int((*a)[7])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKViewer(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(activateCDKViewer(cvt_vc_p_SViewer_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKViewer(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SViewer_(newCDKViewer(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc_pp_char((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc__long_unsigned_int((*a)[7]),
cvt_vc__int((*a)[8]),
cvt_vc__int((*a)[9])));
WRAP_END
return ret;

}
static vc
wrap_setCDKTemplatePostProcess(VCArglist *a)
{
START_WRAP
	setCDKTemplatePostProcess(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKTemplatePreProcess(VCArglist *a)
{
START_WRAP
	setCDKTemplatePreProcess(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_unmixCDKTemplate(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(unmixCDKTemplate(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc_p_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_mixCDKTemplate(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(mixCDKTemplate(cvt_vc_p_STemplate_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKTemplateCB(VCArglist *a)
{
START_WRAP
	setCDKTemplateCB(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc_TEMPLATECB((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKTemplate(VCArglist *a)
{
START_WRAP
	destroyCDKTemplate(cvt_vc_p_STemplate_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKTemplate(VCArglist *a)
{
START_WRAP
	positionCDKTemplate(cvt_vc_p_STemplate_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKTemplate(VCArglist *a)
{
START_WRAP
	moveCDKTemplate(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_cleanCDKTemplate(VCArglist *a)
{
START_WRAP
	cleanCDKTemplate(cvt_vc_p_STemplate_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKTemplate(VCArglist *a)
{
START_WRAP
	eraseCDKTemplate(cvt_vc_p_STemplate_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKTemplate(VCArglist *a)
{
START_WRAP
	drawCDKTemplate(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKTemplateBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKTemplateBackgroundColor(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKTemplateBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKTemplateBoxAttribute(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKTemplateHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKTemplateHorizontalChar(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKTemplateVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKTemplateVerticalChar(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKTemplateLRChar(VCArglist *a)
{
START_WRAP
	setCDKTemplateLRChar(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKTemplateLLChar(VCArglist *a)
{
START_WRAP
	setCDKTemplateLLChar(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKTemplateURChar(VCArglist *a)
{
START_WRAP
	setCDKTemplateURChar(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKTemplateULChar(VCArglist *a)
{
START_WRAP
	setCDKTemplateULChar(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKTemplateBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKTemplateBox(cvt_vc_p_STemplate_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKTemplateBox(VCArglist *a)
{
START_WRAP
	setCDKTemplateBox(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKTemplateMin(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKTemplateMin(cvt_vc_p_STemplate_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKTemplateMin(VCArglist *a)
{
START_WRAP
	setCDKTemplateMin(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKTemplateValue(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(getCDKTemplateValue(cvt_vc_p_STemplate_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKTemplateValue(VCArglist *a)
{
START_WRAP
	setCDKTemplateValue(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKTemplate(VCArglist *a)
{
START_WRAP
	setCDKTemplate(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKTemplate(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(injectCDKTemplate(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKTemplate(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(activateCDKTemplate(cvt_vc_p_STemplate_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKTemplate(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_STemplate_(newCDKTemplate(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc_p_char((*a)[4]),
cvt_vc_p_char((*a)[5]),
cvt_vc_p_char((*a)[6]),
cvt_vc__int((*a)[7]),
cvt_vc__int((*a)[8])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSwindowPostProcess(VCArglist *a)
{
START_WRAP
	setCDKSwindowPostProcess(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSwindowPreProcess(VCArglist *a)
{
START_WRAP
	setCDKSwindowPreProcess(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKSwindow(VCArglist *a)
{
START_WRAP
	destroyCDKSwindow(cvt_vc_p_SSwindow_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKSwindow(VCArglist *a)
{
START_WRAP
	positionCDKSwindow(cvt_vc_p_SSwindow_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKSwindow(VCArglist *a)
{
START_WRAP
	moveCDKSwindow(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_trimCDKSwindow(VCArglist *a)
{
START_WRAP
	trimCDKSwindow(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_addCDKSwindow(VCArglist *a)
{
START_WRAP
	addCDKSwindow(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_cleanCDKSwindow(VCArglist *a)
{
START_WRAP
	cleanCDKSwindow(cvt_vc_p_SSwindow_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKSwindow(VCArglist *a)
{
START_WRAP
	eraseCDKSwindow(cvt_vc_p_SSwindow_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKSwindow(VCArglist *a)
{
START_WRAP
	drawCDKSwindow(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSwindowBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKSwindowBackgroundColor(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSwindowBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKSwindowBoxAttribute(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSwindowHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKSwindowHorizontalChar(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSwindowVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKSwindowVerticalChar(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSwindowLRChar(VCArglist *a)
{
START_WRAP
	setCDKSwindowLRChar(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSwindowLLChar(VCArglist *a)
{
START_WRAP
	setCDKSwindowLLChar(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSwindowURChar(VCArglist *a)
{
START_WRAP
	setCDKSwindowURChar(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSwindowULChar(VCArglist *a)
{
START_WRAP
	setCDKSwindowULChar(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKSwindowBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKSwindowBox(cvt_vc_p_SSwindow_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSwindowBox(VCArglist *a)
{
START_WRAP
	setCDKSwindowBox(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKSwindowContents(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pp_long_unsigned_int(getCDKSwindowContents(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc_p_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSwindowContents(VCArglist *a)
{
START_WRAP
	setCDKSwindowContents(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSwindow(VCArglist *a)
{
START_WRAP
	setCDKSwindow(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_loadCDKSwindowInformation(VCArglist *a)
{
START_WRAP
	loadCDKSwindowInformation(cvt_vc_p_SSwindow_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_saveCDKSwindowInformation(VCArglist *a)
{
START_WRAP
	saveCDKSwindowInformation(cvt_vc_p_SSwindow_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_jumpToLineCDKSwindow(VCArglist *a)
{
START_WRAP
	jumpToLineCDKSwindow(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_dumpCDKSwindow(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(dumpCDKSwindow(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc_p_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_execCDKSwindow(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(execCDKSwindow(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_injectCDKSwindow(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(injectCDKSwindow(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKSwindow(VCArglist *a)
{
START_WRAP
	activateCDKSwindow(cvt_vc_p_SSwindow_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_newCDKSwindow(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SSwindow_(newCDKSwindow(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc_p_char((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc__int((*a)[7]),
cvt_vc__int((*a)[8])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSliderPostProcess(VCArglist *a)
{
START_WRAP
	setCDKSliderPostProcess(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSliderPreProcess(VCArglist *a)
{
START_WRAP
	setCDKSliderPreProcess(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKSlider(VCArglist *a)
{
START_WRAP
	destroyCDKSlider(cvt_vc_p_SSlider_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKSlider(VCArglist *a)
{
START_WRAP
	positionCDKSlider(cvt_vc_p_SSlider_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKSlider(VCArglist *a)
{
START_WRAP
	moveCDKSlider(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKSlider(VCArglist *a)
{
START_WRAP
	eraseCDKSlider(cvt_vc_p_SSlider_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKSlider(VCArglist *a)
{
START_WRAP
	drawCDKSlider(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSliderBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKSliderBackgroundColor(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSliderBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKSliderBoxAttribute(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSliderHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKSliderHorizontalChar(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSliderVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKSliderVerticalChar(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSliderLRChar(VCArglist *a)
{
START_WRAP
	setCDKSliderLRChar(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSliderLLChar(VCArglist *a)
{
START_WRAP
	setCDKSliderLLChar(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSliderURChar(VCArglist *a)
{
START_WRAP
	setCDKSliderURChar(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSliderULChar(VCArglist *a)
{
START_WRAP
	setCDKSliderULChar(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKSliderBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKSliderBox(cvt_vc_p_SSlider_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSliderBox(VCArglist *a)
{
START_WRAP
	setCDKSliderBox(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKSliderValue(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKSliderValue(cvt_vc_p_SSlider_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_getCDKSliderHighValue(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKSliderHighValue(cvt_vc_p_SSlider_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_getCDKSliderLowValue(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKSliderLowValue(cvt_vc_p_SSlider_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSliderValue(VCArglist *a)
{
START_WRAP
	setCDKSliderValue(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSliderLowHigh(VCArglist *a)
{
START_WRAP
	setCDKSliderLowHigh(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSlider(VCArglist *a)
{
START_WRAP
	setCDKSlider(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKSlider(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(injectCDKSlider(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKSlider(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(activateCDKSlider(cvt_vc_p_SSlider_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKSlider(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SSlider_(newCDKSlider(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc_p_char((*a)[4]),
cvt_vc__long_unsigned_int((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc__int((*a)[7]),
cvt_vc__int((*a)[8]),
cvt_vc__int((*a)[9]),
cvt_vc__int((*a)[10]),
cvt_vc__int((*a)[11]),
cvt_vc__int((*a)[12]),
cvt_vc__int((*a)[13])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSelectionPostProcess(VCArglist *a)
{
START_WRAP
	setCDKSelectionPostProcess(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSelectionPreProcess(VCArglist *a)
{
START_WRAP
	setCDKSelectionPreProcess(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSelectionBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKSelectionBackgroundColor(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSelectionBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKSelectionBoxAttribute(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSelectionHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKSelectionHorizontalChar(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSelectionVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKSelectionVerticalChar(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSelectionLRChar(VCArglist *a)
{
START_WRAP
	setCDKSelectionLRChar(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSelectionLLChar(VCArglist *a)
{
START_WRAP
	setCDKSelectionLLChar(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSelectionURChar(VCArglist *a)
{
START_WRAP
	setCDKSelectionURChar(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSelectionULChar(VCArglist *a)
{
START_WRAP
	setCDKSelectionULChar(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKSelectionBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKSelectionBox(cvt_vc_p_SSelection_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSelectionBox(VCArglist *a)
{
START_WRAP
	setCDKSelectionBox(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKSelectionMode(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKSelectionMode(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSelectionMode(VCArglist *a)
{
START_WRAP
	setCDKSelectionMode(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
#if 0
static vc
wrap_getCDKSelectionModes(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_int(getCDKSelectionModes(cvt_vc_p_SSelection_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSelectionModes(VCArglist *a)
{
START_WRAP
	setCDKSelectionModes(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc_p_int((*a)[1]));
WRAP_END
return(vcnil);

}
#endif
static vc
wrap_getCDKSelectionChoice(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKSelectionChoice(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSelectionChoice(VCArglist *a)
{
START_WRAP
	setCDKSelectionChoice(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
#if 0
static vc
wrap_getCDKSelectionChoices(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_int(getCDKSelectionChoices(cvt_vc_p_SSelection_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSelectionChoices(VCArglist *a)
{
START_WRAP
	setCDKSelectionChoices(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc_p_int((*a)[1]));
WRAP_END
return(vcnil);

}
#endif
static vc
wrap_getCDKSelectionHighlight(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKSelectionHighlight(cvt_vc_p_SSelection_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSelectionHighlight(VCArglist *a)
{
START_WRAP
	setCDKSelectionHighlight(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKSelectionTitle(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(getCDKSelectionTitle(cvt_vc_p_SSelection_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSelectionTitle(VCArglist *a)
{
START_WRAP
	setCDKSelectionTitle(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKSelectionItems(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKSelectionItems(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc_pp_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_setCDKSelectionItems(VCArglist *a)
{
START_WRAP
	setCDKSelectionItems(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKSelection(VCArglist *a)
{
START_WRAP
	setCDKSelection(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]),
cvt_vc_p_int((*a)[2]),
cvt_vc__int((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKSelection(VCArglist *a)
{
START_WRAP
	destroyCDKSelection(cvt_vc_p_SSelection_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKSelection(VCArglist *a)
{
START_WRAP
	positionCDKSelection(cvt_vc_p_SSelection_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKSelection(VCArglist *a)
{
START_WRAP
	moveCDKSelection(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKSelection(VCArglist *a)
{
START_WRAP
	eraseCDKSelection(cvt_vc_p_SSelection_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKSelection(VCArglist *a)
{
START_WRAP
	drawCDKSelection(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKSelection(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(injectCDKSelection(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKSelection(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(activateCDKSelection(cvt_vc_p_SSelection_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKSelection(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SSelection_(newCDKSelection(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc_p_char((*a)[6]),
cvt_vc_pp_char((*a)[7]),
cvt_vc__int((*a)[8]),
cvt_vc_pp_char((*a)[9]),
cvt_vc__int((*a)[10]),
cvt_vc__long_unsigned_int((*a)[11]),
cvt_vc__int((*a)[12]),
cvt_vc__int((*a)[13])));
WRAP_END
return ret;

}
static vc
wrap_setCDKScalePostProcess(VCArglist *a)
{
START_WRAP
	setCDKScalePostProcess(cvt_vc_p_SScale_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScalePreProcess(VCArglist *a)
{
START_WRAP
	setCDKScalePreProcess(cvt_vc_p_SScale_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKScale(VCArglist *a)
{
START_WRAP
	destroyCDKScale(cvt_vc_p_SScale_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKScale(VCArglist *a)
{
START_WRAP
	positionCDKScale(cvt_vc_p_SScale_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKScale(VCArglist *a)
{
START_WRAP
	moveCDKScale(cvt_vc_p_SScale_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKScale(VCArglist *a)
{
START_WRAP
	eraseCDKScale(cvt_vc_p_SScale_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKScale(VCArglist *a)
{
START_WRAP
	drawCDKScale(cvt_vc_p_SScale_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScaleBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKScaleBackgroundColor(cvt_vc_p_SScale_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScaleBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKScaleBoxAttribute(cvt_vc_p_SScale_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScaleHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKScaleHorizontalChar(cvt_vc_p_SScale_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScaleVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKScaleVerticalChar(cvt_vc_p_SScale_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScaleLRChar(VCArglist *a)
{
START_WRAP
	setCDKScaleLRChar(cvt_vc_p_SScale_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScaleLLChar(VCArglist *a)
{
START_WRAP
	setCDKScaleLLChar(cvt_vc_p_SScale_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScaleURChar(VCArglist *a)
{
START_WRAP
	setCDKScaleURChar(cvt_vc_p_SScale_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScaleULChar(VCArglist *a)
{
START_WRAP
	setCDKScaleULChar(cvt_vc_p_SScale_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKScaleBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKScaleBox(cvt_vc_p_SScale_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKScaleBox(VCArglist *a)
{
START_WRAP
	setCDKScaleBox(cvt_vc_p_SScale_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKScaleValue(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKScaleValue(cvt_vc_p_SScale_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKScaleValue(VCArglist *a)
{
START_WRAP
	setCDKScaleValue(cvt_vc_p_SScale_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKScaleHighValue(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKScaleHighValue(cvt_vc_p_SScale_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_getCDKScaleLowValue(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKScaleLowValue(cvt_vc_p_SScale_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKScaleLowHigh(VCArglist *a)
{
START_WRAP
	setCDKScaleLowHigh(cvt_vc_p_SScale_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScale(VCArglist *a)
{
START_WRAP
	setCDKScale(cvt_vc_p_SScale_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKScale(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(injectCDKScale(cvt_vc_p_SScale_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKScale(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(activateCDKScale(cvt_vc_p_SScale_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKScale(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SScale_(newCDKScale(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc_p_char((*a)[4]),
cvt_vc__long_unsigned_int((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc__int((*a)[7]),
cvt_vc__int((*a)[8]),
cvt_vc__int((*a)[9]),
cvt_vc__int((*a)[10]),
cvt_vc__int((*a)[11]),
cvt_vc__int((*a)[12]),
cvt_vc__int((*a)[13])));
WRAP_END
return ret;

}
static vc
wrap_setCDKRadioPostProcess(VCArglist *a)
{
START_WRAP
	setCDKRadioPostProcess(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKRadioPreProcess(VCArglist *a)
{
START_WRAP
	setCDKRadioPreProcess(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKRadio(VCArglist *a)
{
START_WRAP
	destroyCDKRadio(cvt_vc_p_SRadio_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKRadio(VCArglist *a)
{
START_WRAP
	positionCDKRadio(cvt_vc_p_SRadio_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKRadio(VCArglist *a)
{
START_WRAP
	moveCDKRadio(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKRadio(VCArglist *a)
{
START_WRAP
	eraseCDKRadio(cvt_vc_p_SRadio_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKRadio(VCArglist *a)
{
START_WRAP
	drawCDKRadio(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKRadioBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKRadioBackgroundColor(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKRadioBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKRadioBoxAttribute(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKRadioHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKRadioHorizontalChar(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKRadioVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKRadioVerticalChar(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKRadioLRChar(VCArglist *a)
{
START_WRAP
	setCDKRadioLRChar(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKRadioLLChar(VCArglist *a)
{
START_WRAP
	setCDKRadioLLChar(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKRadioURChar(VCArglist *a)
{
START_WRAP
	setCDKRadioURChar(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKRadioULChar(VCArglist *a)
{
START_WRAP
	setCDKRadioULChar(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKRadioBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKRadioBox(cvt_vc_p_SRadio_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKRadioBox(VCArglist *a)
{
START_WRAP
	setCDKRadioBox(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKRadioRightBrace(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKRadioRightBrace(cvt_vc_p_SRadio_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKRadioRightBrace(VCArglist *a)
{
START_WRAP
	setCDKRadioRightBrace(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKRadioLeftBrace(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKRadioLeftBrace(cvt_vc_p_SRadio_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKRadioLeftBrace(VCArglist *a)
{
START_WRAP
	setCDKRadioLeftBrace(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKRadioChoiceCharacter(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKRadioChoiceCharacter(cvt_vc_p_SRadio_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKRadioChoiceCharacter(VCArglist *a)
{
START_WRAP
	setCDKRadioChoiceCharacter(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKRadioHighlight(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKRadioHighlight(cvt_vc_p_SRadio_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKRadioHighlight(VCArglist *a)
{
START_WRAP
	setCDKRadioHighlight(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKRadioItems(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKRadioItems(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc_pp_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_setCDKRadioItems(VCArglist *a)
{
START_WRAP
	setCDKRadioItems(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKRadio(VCArglist *a)
{
START_WRAP
	setCDKRadio(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2]),
cvt_vc__int((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKRadio(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(injectCDKRadio(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKRadio(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(activateCDKRadio(cvt_vc_p_SRadio_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKRadio(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SRadio_(newCDKRadio(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc_p_char((*a)[6]),
cvt_vc_pp_char((*a)[7]),
cvt_vc__int((*a)[8]),
cvt_vc__long_unsigned_int((*a)[9]),
cvt_vc__int((*a)[10]),
cvt_vc__long_unsigned_int((*a)[11]),
cvt_vc__int((*a)[12]),
cvt_vc__int((*a)[13])));
WRAP_END
return ret;

}
static vc
wrap_setCDKMenuPostProcess(VCArglist *a)
{
START_WRAP
	setCDKMenuPostProcess(cvt_vc_p_SMenu_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMenuPreProcess(VCArglist *a)
{
START_WRAP
	setCDKMenuPreProcess(cvt_vc_p_SMenu_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKMenu(VCArglist *a)
{
START_WRAP
	destroyCDKMenu(cvt_vc_p_SMenu_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMenuBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKMenuBackgroundColor(cvt_vc_p_SMenu_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKMenuSubwin(VCArglist *a)
{
START_WRAP
	eraseCDKMenuSubwin(cvt_vc_p_SMenu_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKMenu(VCArglist *a)
{
START_WRAP
	eraseCDKMenu(cvt_vc_p_SMenu_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKMenuSubwin(VCArglist *a)
{
START_WRAP
	drawCDKMenuSubwin(cvt_vc_p_SMenu_((*a)[0]));
WRAP_END
return(vcnil);

}
#if 0
static vc
wrap_drawCDKMenuTitles(VCArglist *a)
{
START_WRAP
	drawCDKMenuTitles(cvt_vc_p_SMenu_((*a)[0]));
WRAP_END
return(vcnil);

}
#endif
static vc
wrap_drawCDKMenu(VCArglist *a)
{
START_WRAP
	drawCDKMenu(cvt_vc_p_SMenu_((*a)[0]), cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKMenuSubTitleHighlight(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKMenuSubTitleHighlight(cvt_vc_p_SMenu_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKMenuSubTitleHighlight(VCArglist *a)
{
START_WRAP
	setCDKMenuSubTitleHighlight(cvt_vc_p_SMenu_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKMenuTitleHighlight(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKMenuTitleHighlight(cvt_vc_p_SMenu_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKMenuTitleHighlight(VCArglist *a)
{
START_WRAP
	setCDKMenuTitleHighlight(cvt_vc_p_SMenu_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKMenuCurrentItem(VCArglist *a)
{
START_WRAP
	getCDKMenuCurrentItem(cvt_vc_p_SMenu_((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc_p_int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMenuCurrentItem(VCArglist *a)
{
START_WRAP
	setCDKMenuCurrentItem(cvt_vc_p_SMenu_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMenu(VCArglist *a)
{
START_WRAP
	setCDKMenu(cvt_vc_p_SMenu_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKMenu(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(injectCDKMenu(cvt_vc_p_SMenu_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKMenu(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(activateCDKMenu(cvt_vc_p_SMenu_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_newCDKMenu(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SMenu_(newCDKMenu(cvt_vc_p_SScreen_((*a)[0]),
#endif
static vc
wrap_setCDKMentryPostProcess(VCArglist *a)
{
START_WRAP
	setCDKMentryPostProcess(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMentryPreProcess(VCArglist *a)
{
START_WRAP
	setCDKMentryPreProcess(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMentryCB(VCArglist *a)
{
START_WRAP
	setCDKMentryCB(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc_MENTRYCB((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKMentryField(VCArglist *a)
{
START_WRAP
	drawCDKMentryField(cvt_vc_p_SMentry_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKMentry(VCArglist *a)
{
START_WRAP
	destroyCDKMentry(cvt_vc_p_SMentry_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKMentry(VCArglist *a)
{
START_WRAP
	positionCDKMentry(cvt_vc_p_SMentry_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKMentry(VCArglist *a)
{
START_WRAP
	moveCDKMentry(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_cleanCDKMentry(VCArglist *a)
{
START_WRAP
	cleanCDKMentry(cvt_vc_p_SMentry_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKMentry(VCArglist *a)
{
START_WRAP
	eraseCDKMentry(cvt_vc_p_SMentry_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKMentry(VCArglist *a)
{
START_WRAP
	drawCDKMentry(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMentryBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKMentryBackgroundColor(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMentryBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKMentryBoxAttribute(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMentryHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKMentryHorizontalChar(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMentryVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKMentryVerticalChar(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMentryLRChar(VCArglist *a)
{
START_WRAP
	setCDKMentryLRChar(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMentryLLChar(VCArglist *a)
{
START_WRAP
	setCDKMentryLLChar(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMentryURChar(VCArglist *a)
{
START_WRAP
	setCDKMentryURChar(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMentryULChar(VCArglist *a)
{
START_WRAP
	setCDKMentryULChar(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKMentryBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKMentryBox(cvt_vc_p_SMentry_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKMentryBox(VCArglist *a)
{
START_WRAP
	setCDKMentryBox(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKMentryHiddenChar(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKMentryHiddenChar(cvt_vc_p_SMentry_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKMentryHiddenChar(VCArglist *a)
{
START_WRAP
	setCDKMentryHiddenChar(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKMentryFillerChar(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKMentryFillerChar(cvt_vc_p_SMentry_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKMentryFillerChar(VCArglist *a)
{
START_WRAP
	setCDKMentryFillerChar(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKMentryMin(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKMentryMin(cvt_vc_p_SMentry_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKMentryMin(VCArglist *a)
{
START_WRAP
	setCDKMentryMin(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKMentryValue(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(getCDKMentryValue(cvt_vc_p_SMentry_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKMentryValue(VCArglist *a)
{
START_WRAP
	setCDKMentryValue(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMentry(VCArglist *a)
{
START_WRAP
	setCDKMentry(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKMentry(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(injectCDKMentry(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKMentry(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(activateCDKMentry(cvt_vc_p_SMentry_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKMentry(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SMentry_(newCDKMentry(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc_p_char((*a)[4]),
cvt_vc__long_unsigned_int((*a)[5]),
cvt_vc__long_unsigned_int((*a)[6]),
cvt_vc__int((*a)[7]),
cvt_vc__int((*a)[8]),
cvt_vc__int((*a)[9]),
cvt_vc__int((*a)[10]),
cvt_vc__int((*a)[11]),
cvt_vc__int((*a)[12]),
cvt_vc__int((*a)[13])));
WRAP_END
return ret;

}
static vc
wrap_setCDKMatrixPostProcess(VCArglist *a)
{
START_WRAP
	setCDKMatrixPostProcess(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMatrixPreProcess(VCArglist *a)
{
START_WRAP
	setCDKMatrixPreProcess(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_jumpToCell(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(jumpToCell(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_destroyCDKMatrix(VCArglist *a)
{
START_WRAP
	destroyCDKMatrix(cvt_vc_p_SMatrix_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKMatrix(VCArglist *a)
{
START_WRAP
	positionCDKMatrix(cvt_vc_p_SMatrix_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKMatrix(VCArglist *a)
{
START_WRAP
	moveCDKMatrix(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveToCDKMatrixCell(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(moveToCDKMatrixCell(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_setCDKMatrixCB(VCArglist *a)
{
START_WRAP
	setCDKMatrixCB(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc_MATRIXCB((*a)[1]));
WRAP_END
return(vcnil);

}
#endif
static vc
wrap_cleanCDKMatrix(VCArglist *a)
{
START_WRAP
	cleanCDKMatrix(cvt_vc_p_SMatrix_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKMatrix(VCArglist *a)
{
START_WRAP
	eraseCDKMatrix(cvt_vc_p_SMatrix_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKMatrix(VCArglist *a)
{
START_WRAP
	drawCDKMatrix(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMatrixBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKMatrixBackgroundColor(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMatrixBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKMatrixBoxAttribute(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMatrixHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKMatrixHorizontalChar(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMatrixVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKMatrixVerticalChar(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMatrixLRChar(VCArglist *a)
{
START_WRAP
	setCDKMatrixLRChar(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMatrixLLChar(VCArglist *a)
{
START_WRAP
	setCDKMatrixLLChar(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMatrixURChar(VCArglist *a)
{
START_WRAP
	setCDKMatrixURChar(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMatrixULChar(VCArglist *a)
{
START_WRAP
	setCDKMatrixULChar(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKMatrixRow(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKMatrixRow(cvt_vc_p_SMatrix_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_getCDKMatrixCol(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKMatrixCol(cvt_vc_p_SMatrix_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_getCDKMatrixCell(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(getCDKMatrixCell(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_setCDKMatrixCell(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(setCDKMatrixCell(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_setCDKMatrix(VCArglist *a)
{
START_WRAP
	setCDKMatrix(cvt_vc_p_SMatrix_((*a)[0]),
#endif
static vc
wrap_injectCDKMatrix(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(injectCDKMatrix(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKMatrix(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(activateCDKMatrix(cvt_vc_p_SMatrix_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKMatrix(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SMatrix_(newCDKMatrix(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc_p_char((*a)[7]),
cvt_vc_pp_char((*a)[8]),
cvt_vc_pp_char((*a)[9]),
cvt_vc_p_int((*a)[10]),
cvt_vc_p_int((*a)[11]),
cvt_vc__int((*a)[12]),
cvt_vc__int((*a)[13]),
cvt_vc__long_unsigned_int((*a)[14]),
cvt_vc__int((*a)[15]),
cvt_vc__int((*a)[16]),
cvt_vc__int((*a)[17]),
cvt_vc__int((*a)[18])));
WRAP_END
return ret;

}
static vc
wrap_setCDKMarqueeBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKMarqueeBackgroundColor(cvt_vc_p_SMarquee_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMarqueeBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKMarqueeBoxAttribute(cvt_vc_p_SMarquee_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMarqueeHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKMarqueeHorizontalChar(cvt_vc_p_SMarquee_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMarqueeVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKMarqueeVerticalChar(cvt_vc_p_SMarquee_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMarqueeLRChar(VCArglist *a)
{
START_WRAP
	setCDKMarqueeLRChar(cvt_vc_p_SMarquee_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMarqueeLLChar(VCArglist *a)
{
START_WRAP
	setCDKMarqueeLLChar(cvt_vc_p_SMarquee_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMarqueeURChar(VCArglist *a)
{
START_WRAP
	setCDKMarqueeURChar(cvt_vc_p_SMarquee_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKMarqueeULChar(VCArglist *a)
{
START_WRAP
	setCDKMarqueeULChar(cvt_vc_p_SMarquee_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKMarquee(VCArglist *a)
{
START_WRAP
	destroyCDKMarquee(cvt_vc_p_SMarquee_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKMarquee(VCArglist *a)
{
START_WRAP
	positionCDKMarquee(cvt_vc_p_SMarquee_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKMarquee(VCArglist *a)
{
START_WRAP
	moveCDKMarquee(cvt_vc_p_SMarquee_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKMarquee(VCArglist *a)
{
START_WRAP
	eraseCDKMarquee(cvt_vc_p_SMarquee_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKMarquee(VCArglist *a)
{
START_WRAP
	drawCDKMarquee(cvt_vc_p_SMarquee_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_deactivateCDKMarquee(VCArglist *a)
{
START_WRAP
	deactivateCDKMarquee(cvt_vc_p_SMarquee_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_activateCDKMarquee(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(activateCDKMarquee(cvt_vc_p_SMarquee_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_newCDKMarquee(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SMarquee_(newCDKMarquee(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_setCDKItemlistPostProcess(VCArglist *a)
{
START_WRAP
	setCDKItemlistPostProcess(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKItemlistPreProcess(VCArglist *a)
{
START_WRAP
	setCDKItemlistPreProcess(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKItemlist(VCArglist *a)
{
START_WRAP
	destroyCDKItemlist(cvt_vc_p_SItemList_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKItemlist(VCArglist *a)
{
START_WRAP
	positionCDKItemlist(cvt_vc_p_SItemList_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKItemlist(VCArglist *a)
{
START_WRAP
	moveCDKItemlist(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKItemlist(VCArglist *a)
{
START_WRAP
	eraseCDKItemlist(cvt_vc_p_SItemList_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKItemlistField(VCArglist *a)
{
START_WRAP
	drawCDKItemlistField(cvt_vc_p_SItemList_((*a)[0]), cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKItemlist(VCArglist *a)
{
START_WRAP
	drawCDKItemlist(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKItemlistBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKItemlistBackgroundColor(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKItemlistBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKItemlistBoxAttribute(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKItemlistHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKItemlistHorizontalChar(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKItemlistVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKItemlistVerticalChar(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKItemlistLRChar(VCArglist *a)
{
START_WRAP
	setCDKItemlistLRChar(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKItemlistLLChar(VCArglist *a)
{
START_WRAP
	setCDKItemlistLLChar(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKItemlistURChar(VCArglist *a)
{
START_WRAP
	setCDKItemlistURChar(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKItemlistULChar(VCArglist *a)
{
START_WRAP
	setCDKItemlistULChar(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKItemlistBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKItemlistBox(cvt_vc_p_SItemList_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKItemlistBox(VCArglist *a)
{
START_WRAP
	setCDKItemlistBox(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKItemlistCurrentItem(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKItemlistCurrentItem(cvt_vc_p_SItemList_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKItemlistCurrentItem(VCArglist *a)
{
START_WRAP
	setCDKItemlistCurrentItem(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKItemlistDefaultItem(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKItemlistDefaultItem(cvt_vc_p_SItemList_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKItemlistDefaultItem(VCArglist *a)
{
START_WRAP
	setCDKItemlistDefaultItem(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKItemlistValues(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pp_long_unsigned_int(getCDKItemlistValues(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc_p_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_setCDKItemlistValues(VCArglist *a)
{
START_WRAP
	setCDKItemlistValues(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKItemlist(VCArglist *a)
{
START_WRAP
	setCDKItemlist(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKItemlist(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(injectCDKItemlist(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKItemlist(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(activateCDKItemlist(cvt_vc_p_SItemList_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKItemlist(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SItemList_(newCDKItemlist(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc_p_char((*a)[4]),
cvt_vc_pp_char((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc__int((*a)[7]),
cvt_vc__int((*a)[8]),
cvt_vc__int((*a)[9])));
WRAP_END
return ret;

}
static vc
wrap_destroyCDKHistogram(VCArglist *a)
{
START_WRAP
	destroyCDKHistogram(cvt_vc_p_SHistogram_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKHistogram(VCArglist *a)
{
START_WRAP
	positionCDKHistogram(cvt_vc_p_SHistogram_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKHistogram(VCArglist *a)
{
START_WRAP
	moveCDKHistogram(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKHistogram(VCArglist *a)
{
START_WRAP
	eraseCDKHistogram(cvt_vc_p_SHistogram_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKHistogram(VCArglist *a)
{
START_WRAP
	drawCDKHistogram(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKHistogramBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKHistogramBackgroundColor(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKHistogramBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKHistogramBoxAttribute(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKHistogramHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKHistogramHorizontalChar(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKHistogramVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKHistogramVerticalChar(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKHistogramLRChar(VCArglist *a)
{
START_WRAP
	setCDKHistogramLRChar(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKHistogramLLChar(VCArglist *a)
{
START_WRAP
	setCDKHistogramLLChar(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKHistogramURChar(VCArglist *a)
{
START_WRAP
	setCDKHistogramURChar(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKHistogramULChar(VCArglist *a)
{
START_WRAP
	setCDKHistogramULChar(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKHistogramBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKHistogramBox(cvt_vc_p_SHistogram_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKHistogramBox(VCArglist *a)
{
START_WRAP
	setCDKHistogramBox(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKHistogramStatsAttr(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKHistogramStatsAttr(cvt_vc_p_SHistogram_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKHistogramStatsAttr(VCArglist *a)
{
START_WRAP
	setCDKHistogramStatsAttr(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKHistogramStatsPos(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKHistogramStatsPos(cvt_vc_p_SHistogram_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKHistogramStatsPos(VCArglist *a)
{
START_WRAP
	setCDKHistogramStatsPos(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKHistogramFillerChar(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKHistogramFillerChar(cvt_vc_p_SHistogram_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKHistogramFillerChar(VCArglist *a)
{
START_WRAP
	setCDKHistogramFillerChar(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKHistogramViewType(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKHistogramViewType(cvt_vc_p_SHistogram_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKHistogramDisplayType(VCArglist *a)
{
START_WRAP
	setCDKHistogramDisplayType(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKHistogramHighValue(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKHistogramHighValue(cvt_vc_p_SHistogram_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_getCDKHistogramLowValue(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKHistogramLowValue(cvt_vc_p_SHistogram_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_getCDKHistogramValue(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKHistogramValue(cvt_vc_p_SHistogram_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKHistogramValue(VCArglist *a)
{
START_WRAP
	setCDKHistogramValue(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKHistogram(VCArglist *a)
{
START_WRAP
	setCDKHistogram(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc__long_unsigned_int((*a)[7]),
cvt_vc__int((*a)[8]));
WRAP_END
return(vcnil);

}
static vc
wrap_activateCDKHistogram(VCArglist *a)
{
START_WRAP
	activateCDKHistogram(cvt_vc_p_SHistogram_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_newCDKHistogram(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SHistogram_(newCDKHistogram(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc_p_char((*a)[6]),
cvt_vc__int((*a)[7]),
cvt_vc__int((*a)[8])));
WRAP_END
return ret;

}
static vc
wrap_destroyCDKGraph(VCArglist *a)
{
START_WRAP
	destroyCDKGraph(cvt_vc_p_SGraph_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKGraph(VCArglist *a)
{
START_WRAP
	positionCDKGraph(cvt_vc_p_SGraph_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKGraph(VCArglist *a)
{
START_WRAP
	moveCDKGraph(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKGraph(VCArglist *a)
{
START_WRAP
	eraseCDKGraph(cvt_vc_p_SGraph_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKGraph(VCArglist *a)
{
START_WRAP
	drawCDKGraph(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKGraphBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKGraphBackgroundColor(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKGraphBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKGraphBoxAttribute(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKGraphHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKGraphHorizontalChar(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKGraphVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKGraphVerticalChar(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKGraphLRChar(VCArglist *a)
{
START_WRAP
	setCDKGraphLRChar(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKGraphLLChar(VCArglist *a)
{
START_WRAP
	setCDKGraphLLChar(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKGraphURChar(VCArglist *a)
{
START_WRAP
	setCDKGraphURChar(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKGraphULChar(VCArglist *a)
{
START_WRAP
	setCDKGraphULChar(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKGraphBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKGraphBox(cvt_vc_p_SGraph_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKGraphBox(VCArglist *a)
{
START_WRAP
	setCDKGraphBox(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKGraphDisplayType(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKGraphDisplayType(cvt_vc_p_SGraph_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKGraphDisplayType(VCArglist *a)
{
START_WRAP
	setCDKGraphDisplayType(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKGraphCharacter(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKGraphCharacter(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_getCDKGraphCharacters(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_long_unsigned_int(getCDKGraphCharacters(cvt_vc_p_SGraph_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKGraphCharacter(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(setCDKGraphCharacter(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_char((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_setCDKGraphCharacters(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(setCDKGraphCharacters(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc_p_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_getCDKGraphValue(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKGraphValue(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__int((*a)[1])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_getCDKGraphValues(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_int(getCDKGraphValues(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc_p_int((*a)[1])));
WRAP_END
return ret;

}
#endif
static vc
wrap_setCDKGraphValue(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(setCDKGraphValue(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_setCDKGraphValues(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(setCDKGraphValues(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
#endif
static vc
wrap_setCDKGraph(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(setCDKGraph(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKGraph(VCArglist *a)
{
START_WRAP
	activateCDKGraph(cvt_vc_p_SGraph_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_newCDKGraph(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SGraph_(newCDKGraph(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc_p_char((*a)[5]),
cvt_vc_p_char((*a)[6]),
cvt_vc_p_char((*a)[7])));
WRAP_END
return ret;

}
static vc
wrap_deleteFileCB(VCArglist *a)
{
START_WRAP
	deleteFileCB(cvt_vc__int((*a)[0]),
cvt_vc_p_void((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKFselect(VCArglist *a)
{
START_WRAP
	destroyCDKFselect(cvt_vc_p_SFileSelector_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKFselect(VCArglist *a)
{
START_WRAP
	positionCDKFselect(cvt_vc_p_SFileSelector_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKFselect(VCArglist *a)
{
START_WRAP
	moveCDKFselect(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKFselect(VCArglist *a)
{
START_WRAP
	eraseCDKFselect(cvt_vc_p_SFileSelector_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKFselect(VCArglist *a)
{
START_WRAP
	drawCDKFselect(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKFselectBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKFselectBackgroundColor(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKFselectBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKFselectBoxAttribute(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKFselectHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKFselectHorizontalChar(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKFselectVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKFselectVerticalChar(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKFselectLRChar(VCArglist *a)
{
START_WRAP
	setCDKFselectLRChar(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKFselectLLChar(VCArglist *a)
{
START_WRAP
	setCDKFselectLLChar(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKFselectURChar(VCArglist *a)
{
START_WRAP
	setCDKFselectURChar(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKFselectULChar(VCArglist *a)
{
START_WRAP
	setCDKFselectULChar(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKFselectDirContents(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pp_char(getCDKFselectDirContents(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc_p_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_setCDKFselectDirContents(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(setCDKFselectDirContents(cvt_vc_p_SFileSelector_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_getCDKFselectBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKFselectBox(cvt_vc_p_SFileSelector_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKFselectBox(VCArglist *a)
{
START_WRAP
	setCDKFselectBox(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKFselectSocketAttribute(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(getCDKFselectSocketAttribute(cvt_vc_p_SFileSelector_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_getCDKFselectFileAttribute(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(getCDKFselectFileAttribute(cvt_vc_p_SFileSelector_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_getCDKFselectLinkAttribute(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(getCDKFselectLinkAttribute(cvt_vc_p_SFileSelector_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_getCDKFselectDirAttribute(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(getCDKFselectDirAttribute(cvt_vc_p_SFileSelector_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKFselectSocketAttribute(VCArglist *a)
{
START_WRAP
	setCDKFselectSocketAttribute(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKFselectFileAttribute(VCArglist *a)
{
START_WRAP
	setCDKFselectFileAttribute(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKFselectLinkAttribute(VCArglist *a)
{
START_WRAP
	setCDKFselectLinkAttribute(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKFselectDirAttribute(VCArglist *a)
{
START_WRAP
	setCDKFselectDirAttribute(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKFselectHighlight(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKFselectHighlight(cvt_vc_p_SFileSelector_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKFselectHighlight(VCArglist *a)
{
START_WRAP
	setCDKFselectHighlight(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKFselectFillerChar(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKFselectFillerChar(cvt_vc_p_SFileSelector_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKFselectFillerChar(VCArglist *a)
{
START_WRAP
	setCDKFselectFillerChar(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKFselectDirectory(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(getCDKFselectDirectory(cvt_vc_p_SFileSelector_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKFselectDirectory(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(setCDKFselectDirectory(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc_p_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_setCDKFselect(VCArglist *a)
{
START_WRAP
	setCDKFselect(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]),
cvt_vc_p_char((*a)[5]),
cvt_vc_p_char((*a)[6]),
cvt_vc_p_char((*a)[7]),
cvt_vc_p_char((*a)[8]),
cvt_vc__int((*a)[9]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKFselect(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(injectCDKFselect(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKFselect(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(activateCDKFselect(cvt_vc_p_SFileSelector_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKFselect(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SFileSelector_(newCDKFselect(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc_p_char((*a)[5]),
cvt_vc_p_char((*a)[6]),
cvt_vc__long_unsigned_int((*a)[7]),
cvt_vc__long_unsigned_int((*a)[8]),
cvt_vc__long_unsigned_int((*a)[9]),
cvt_vc_p_char((*a)[10]),
cvt_vc_p_char((*a)[11]),
cvt_vc_p_char((*a)[12]),
cvt_vc_p_char((*a)[13]),
cvt_vc__int((*a)[14]),
cvt_vc__int((*a)[15])));
WRAP_END
return ret;

}
static vc
wrap_positionCDKLabel(VCArglist *a)
{
START_WRAP
	positionCDKLabel(cvt_vc_p_SLabel_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKLabel(VCArglist *a)
{
START_WRAP
	moveCDKLabel(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_waitCDKLabel(VCArglist *a)
{
START_WRAP
	vc ret = cvt__char(waitCDKLabel(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc__char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_destroyCDKLabel(VCArglist *a)
{
START_WRAP
	destroyCDKLabel(cvt_vc_p_SLabel_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKLabel(VCArglist *a)
{
START_WRAP
	eraseCDKLabel(cvt_vc_p_SLabel_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKLabelBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKLabelBackgroundColor(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKLabelBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKLabelBoxAttribute(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKLabelHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKLabelHorizontalChar(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKLabelVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKLabelVerticalChar(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKLabelLRChar(VCArglist *a)
{
START_WRAP
	setCDKLabelLRChar(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKLabelLLChar(VCArglist *a)
{
START_WRAP
	setCDKLabelLLChar(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKLabelURChar(VCArglist *a)
{
START_WRAP
	setCDKLabelURChar(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKLabelULChar(VCArglist *a)
{
START_WRAP
	setCDKLabelULChar(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKLabel(VCArglist *a)
{
START_WRAP
	drawCDKLabel(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKLabelBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKLabelBox(cvt_vc_p_SLabel_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKLabelBox(VCArglist *a)
{
START_WRAP
	setCDKLabelBox(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKLabelMessage(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pp_long_unsigned_int(getCDKLabelMessage(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc_p_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_setCDKLabelMessage(VCArglist *a)
{
START_WRAP
	setCDKLabelMessage(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKLabel(VCArglist *a)
{
START_WRAP
	setCDKLabel(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_activateCDKLabel(VCArglist *a)
{
START_WRAP
	activateCDKLabel(cvt_vc_p_SLabel_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_newCDKLabel(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SLabel_(newCDKLabel(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pp_char((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6])));
WRAP_END
return ret;

}
static vc
wrap_setCDKDialogPostProcess(VCArglist *a)
{
START_WRAP
	setCDKDialogPostProcess(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKDialogPreProcess(VCArglist *a)
{
START_WRAP
	setCDKDialogPreProcess(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKDialogButtons(VCArglist *a)
{
START_WRAP
	drawCDKDialogButtons(cvt_vc_p_SDialogBox_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKDialog(VCArglist *a)
{
START_WRAP
	destroyCDKDialog(cvt_vc_p_SDialogBox_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKDialog(VCArglist *a)
{
START_WRAP
	positionCDKDialog(cvt_vc_p_SDialogBox_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKDialog(VCArglist *a)
{
START_WRAP
	moveCDKDialog(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKDialog(VCArglist *a)
{
START_WRAP
	eraseCDKDialog(cvt_vc_p_SDialogBox_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKDialog(VCArglist *a)
{
START_WRAP
	drawCDKDialog(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
#if 0
static vc
wrap_drawCDKDialogButton(VCArglist *a)
{
START_WRAP
	drawCDKDialogButton(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]));
WRAP_END
return(vcnil);

}
#endif
static vc
wrap_setCDKDialogBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKDialogBackgroundColor(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKDialogBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKDialogBoxAttribute(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKDialogHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKDialogHorizontalChar(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKDialogVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKDialogVerticalChar(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKDialogLRChar(VCArglist *a)
{
START_WRAP
	setCDKDialogLRChar(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKDialogLLChar(VCArglist *a)
{
START_WRAP
	setCDKDialogLLChar(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKDialogURChar(VCArglist *a)
{
START_WRAP
	setCDKDialogURChar(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKDialogULChar(VCArglist *a)
{
START_WRAP
	setCDKDialogULChar(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKDialogBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKDialogBox(cvt_vc_p_SDialogBox_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKDialogBox(VCArglist *a)
{
START_WRAP
	setCDKDialogBox(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKDialogSeparator(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKDialogSeparator(cvt_vc_p_SDialogBox_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKDialogSeparator(VCArglist *a)
{
START_WRAP
	setCDKDialogSeparator(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKDialogHighlight(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKDialogHighlight(cvt_vc_p_SDialogBox_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKDialogHighlight(VCArglist *a)
{
START_WRAP
	setCDKDialogHighlight(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKDialog(VCArglist *a)
{
START_WRAP
	setCDKDialog(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKDialog(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(injectCDKDialog(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKDialog(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(activateCDKDialog(cvt_vc_p_SDialogBox_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKDialog(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SDialogBox_(newCDKDialog(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pp_char((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc_pp_char((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc__long_unsigned_int((*a)[7]),
cvt_vc__int((*a)[8]),
cvt_vc__int((*a)[9]),
cvt_vc__int((*a)[10])));
WRAP_END
return ret;

}
static vc
wrap_setCDKCalendarPostProcess(VCArglist *a)
{
START_WRAP
	setCDKCalendarPostProcess(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKCalendarPreProcess(VCArglist *a)
{
START_WRAP
	setCDKCalendarPreProcess(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKCalendar(VCArglist *a)
{
START_WRAP
	destroyCDKCalendar(cvt_vc_p_SCalendar_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKCalendar(VCArglist *a)
{
START_WRAP
	positionCDKCalendar(cvt_vc_p_SCalendar_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKCalendar(VCArglist *a)
{
START_WRAP
	moveCDKCalendar(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKCalendar(VCArglist *a)
{
START_WRAP
	eraseCDKCalendar(cvt_vc_p_SCalendar_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKCalendar(VCArglist *a)
{
START_WRAP
	drawCDKCalendar(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_removeCDKCalendarMarker(VCArglist *a)
{
START_WRAP
	removeCDKCalendarMarker(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKCalendarMarker(VCArglist *a)
{
START_WRAP
	setCDKCalendarMarker(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKCalendarBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKCalendarBackgroundColor(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKCalendarBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKCalendarBoxAttribute(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKCalendarHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKCalendarHorizontalChar(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKCalendarVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKCalendarVerticalChar(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKCalendarLRChar(VCArglist *a)
{
START_WRAP
	setCDKCalendarLRChar(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKCalendarLLChar(VCArglist *a)
{
START_WRAP
	setCDKCalendarLLChar(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKCalendarURChar(VCArglist *a)
{
START_WRAP
	setCDKCalendarURChar(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKCalendarULChar(VCArglist *a)
{
START_WRAP
	setCDKCalendarULChar(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKCalendarBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKCalendarBox(cvt_vc_p_SCalendar_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKCalendarBox(VCArglist *a)
{
START_WRAP
	setCDKCalendarBox(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKCalendarHighlight(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKCalendarHighlight(cvt_vc_p_SCalendar_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKCalendarHighlight(VCArglist *a)
{
START_WRAP
	setCDKCalendarHighlight(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKCalendarYearAttribute(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKCalendarYearAttribute(cvt_vc_p_SCalendar_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKCalendarYearAttribute(VCArglist *a)
{
START_WRAP
	setCDKCalendarYearAttribute(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKCalendarMonthAttribute(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKCalendarMonthAttribute(cvt_vc_p_SCalendar_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKCalendarMonthAttribute(VCArglist *a)
{
START_WRAP
	setCDKCalendarMonthAttribute(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKCalendarDayAttribute(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKCalendarDayAttribute(cvt_vc_p_SCalendar_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKCalendarDayAttribute(VCArglist *a)
{
START_WRAP
	setCDKCalendarDayAttribute(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKCalendarDate(VCArglist *a)
{
START_WRAP
	getCDKCalendarDate(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc_p_int((*a)[2]),
cvt_vc_p_int((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKCalendarDate(VCArglist *a)
{
START_WRAP
	setCDKCalendarDate(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKCalendar(VCArglist *a)
{
START_WRAP
	setCDKCalendar(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]),
cvt_vc__long_unsigned_int((*a)[5]),
cvt_vc__long_unsigned_int((*a)[6]),
cvt_vc__long_unsigned_int((*a)[7]),
cvt_vc__int((*a)[8]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKCalendar(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_int(injectCDKCalendar(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKCalendar(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_int(activateCDKCalendar(cvt_vc_p_SCalendar_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKCalendar(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SCalendar_(newCDKCalendar(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc__long_unsigned_int((*a)[7]),
cvt_vc__long_unsigned_int((*a)[8]),
cvt_vc__long_unsigned_int((*a)[9]),
cvt_vc__long_unsigned_int((*a)[10]),
cvt_vc__int((*a)[11]),
cvt_vc__int((*a)[12])));
WRAP_END
return ret;

}
static vc
wrap_setCDKButtonboxPostProcess(VCArglist *a)
{
START_WRAP
	setCDKButtonboxPostProcess(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKButtonboxPreProcess(VCArglist *a)
{
START_WRAP
	setCDKButtonboxPreProcess(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
#if 0
static vc
wrap_redrawCDKButtonboxButtonboxs(VCArglist *a)
{
START_WRAP
	redrawCDKButtonboxButtonboxs(cvt_vc_p_SButtonBox_((*a)[0]));
WRAP_END
return(vcnil);

}
#endif
static vc
wrap_destroyCDKButtonbox(VCArglist *a)
{
START_WRAP
	destroyCDKButtonbox(cvt_vc_p_SButtonBox_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKButtonbox(VCArglist *a)
{
START_WRAP
	positionCDKButtonbox(cvt_vc_p_SButtonBox_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKButtonbox(VCArglist *a)
{
START_WRAP
	moveCDKButtonbox(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKButtonbox(VCArglist *a)
{
START_WRAP
	eraseCDKButtonbox(cvt_vc_p_SButtonBox_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKButtonboxButtons(VCArglist *a)
{
START_WRAP
	drawCDKButtonboxButtons(cvt_vc_p_SButtonBox_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKButtonbox(VCArglist *a)
{
START_WRAP
	drawCDKButtonbox(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
#if 0
static vc
wrap_drawCDKButtonboxButton(VCArglist *a)
{
START_WRAP
	drawCDKButtonboxButton(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3]));
WRAP_END
return(vcnil);

}
#endif
static vc
wrap_setCDKButtonboxBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKButtonboxBackgroundColor(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKButtonboxBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKButtonboxBoxAttribute(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKButtonboxHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKButtonboxHorizontalChar(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKButtonboxVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKButtonboxVerticalChar(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKButtonboxLRChar(VCArglist *a)
{
START_WRAP
	setCDKButtonboxLRChar(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKButtonboxLLChar(VCArglist *a)
{
START_WRAP
	setCDKButtonboxLLChar(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKButtonboxURChar(VCArglist *a)
{
START_WRAP
	setCDKButtonboxURChar(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKButtonboxULChar(VCArglist *a)
{
START_WRAP
	setCDKButtonboxULChar(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKButtonboxBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKButtonboxBox(cvt_vc_p_SButtonBox_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKButtonboxBox(VCArglist *a)
{
START_WRAP
	setCDKButtonboxBox(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKButtonboxHighlight(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKButtonboxHighlight(cvt_vc_p_SButtonBox_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKButtonboxHighlight(VCArglist *a)
{
START_WRAP
	setCDKButtonboxHighlight(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKButtonboxButtonCount(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKButtonboxButtonCount(cvt_vc_p_SButtonBox_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_getCDKButtonboxCurrentButton(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKButtonboxCurrentButton(cvt_vc_p_SButtonBox_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKButtonboxCurrentButton(VCArglist *a)
{
START_WRAP
	setCDKButtonboxCurrentButton(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKButtonbox(VCArglist *a)
{
START_WRAP
	setCDKButtonbox(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKButtonbox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(injectCDKButtonbox(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKButtonbox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(activateCDKButtonbox(cvt_vc_p_SButtonBox_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKButtonbox(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SButtonBox_(newCDKButtonbox(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc_p_char((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc__int((*a)[7]),
cvt_vc_pp_char((*a)[8]),
cvt_vc__int((*a)[9]),
cvt_vc__long_unsigned_int((*a)[10]),
cvt_vc__int((*a)[11]),
cvt_vc__int((*a)[12])));
WRAP_END
return ret;

}
static vc
wrap_setCDKAlphalistPostProcess(VCArglist *a)
{
START_WRAP
	setCDKAlphalistPostProcess(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKAlphalistPreProcess(VCArglist *a)
{
START_WRAP
	setCDKAlphalistPreProcess(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKAlphalist(VCArglist *a)
{
START_WRAP
	destroyCDKAlphalist(cvt_vc_p_SAlphalist_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKAlphalist(VCArglist *a)
{
START_WRAP
	positionCDKAlphalist(cvt_vc_p_SAlphalist_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKAlphalist(VCArglist *a)
{
START_WRAP
	moveCDKAlphalist(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKAlphalist(VCArglist *a)
{
START_WRAP
	eraseCDKAlphalist(cvt_vc_p_SAlphalist_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKAlphalist(VCArglist *a)
{
START_WRAP
	drawCDKAlphalist(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKAlphalistBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKAlphalistBackgroundColor(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKAlphalistBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKAlphalistBoxAttribute(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKAlphalistHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKAlphalistHorizontalChar(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKAlphalistVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKAlphalistVerticalChar(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKAlphalistLRChar(VCArglist *a)
{
START_WRAP
	setCDKAlphalistLRChar(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKAlphalistLLChar(VCArglist *a)
{
START_WRAP
	setCDKAlphalistLLChar(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKAlphalistURChar(VCArglist *a)
{
START_WRAP
	setCDKAlphalistURChar(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKAlphalistULChar(VCArglist *a)
{
START_WRAP
	setCDKAlphalistULChar(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKAlphalistBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKAlphalistBox(cvt_vc_p_SAlphalist_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKAlphalistBox(VCArglist *a)
{
START_WRAP
	setCDKAlphalistBox(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKAlphalistHighlight(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKAlphalistHighlight(cvt_vc_p_SAlphalist_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKAlphalistHighlight(VCArglist *a)
{
START_WRAP
	setCDKAlphalistHighlight(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKAlphalistFillerChar(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKAlphalistFillerChar(cvt_vc_p_SAlphalist_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKAlphalistFillerChar(VCArglist *a)
{
START_WRAP
	setCDKAlphalistFillerChar(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKAlphalistContents(VCArglist *a)
{
START_WRAP
	vc ret = cvt_pp_char(getCDKAlphalistContents(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc_p_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_setCDKAlphalistContents(VCArglist *a)
{
START_WRAP
	setCDKAlphalistContents(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKAlphalist(VCArglist *a)
{
START_WRAP
	setCDKAlphalist(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]),
cvt_vc__int((*a)[5]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKAlphalist(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(injectCDKAlphalist(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKAlphalist(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(activateCDKAlphalist(cvt_vc_p_SAlphalist_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKAlphalist(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SAlphalist_(newCDKAlphalist(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc_p_char((*a)[5]),
cvt_vc_p_char((*a)[6]),
cvt_vc_pp_char((*a)[7]),
cvt_vc__int((*a)[8]),
cvt_vc__long_unsigned_int((*a)[9]),
cvt_vc__long_unsigned_int((*a)[10]),
cvt_vc__int((*a)[11]),
cvt_vc__int((*a)[12])));
WRAP_END
return ret;

}
static vc
wrap_setCDKScrollPostProcess(VCArglist *a)
{
START_WRAP
	setCDKScrollPostProcess(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScrollPreProcess(VCArglist *a)
{
START_WRAP
	setCDKScrollPreProcess(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKScroll(VCArglist *a)
{
START_WRAP
	destroyCDKScroll(cvt_vc_p_SScroll_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKScroll(VCArglist *a)
{
START_WRAP
	positionCDKScroll(cvt_vc_p_SScroll_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKScroll(VCArglist *a)
{
START_WRAP
	moveCDKScroll(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKScroll(VCArglist *a)
{
START_WRAP
	eraseCDKScroll(cvt_vc_p_SScroll_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKScroll(VCArglist *a)
{
START_WRAP
	drawCDKScroll(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_deleteCDKScrollItem(VCArglist *a)
{
START_WRAP
	deleteCDKScrollItem(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_addCDKScrollItem(VCArglist *a)
{
START_WRAP
	addCDKScrollItem(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScrollBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKScrollBackgroundColor(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScrollBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKScrollBoxAttribute(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScrollHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKScrollHorizontalChar(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScrollVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKScrollVerticalChar(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScrollLRChar(VCArglist *a)
{
START_WRAP
	setCDKScrollLRChar(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScrollLLChar(VCArglist *a)
{
START_WRAP
	setCDKScrollLLChar(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScrollURChar(VCArglist *a)
{
START_WRAP
	setCDKScrollURChar(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScrollULChar(VCArglist *a)
{
START_WRAP
	setCDKScrollULChar(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKScrollBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKScrollBox(cvt_vc_p_SScroll_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKScrollBox(VCArglist *a)
{
START_WRAP
	setCDKScrollBox(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKScrollHighlight(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKScrollHighlight(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_setCDKScrollHighlight(VCArglist *a)
{
START_WRAP
	setCDKScrollHighlight(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKScrollItems(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKScrollItems(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc_pp_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_setCDKScrollItems(VCArglist *a)
{
START_WRAP
	setCDKScrollItems(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScrollPosition(VCArglist *a)
{
START_WRAP
	setCDKScrollPosition(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKScroll(VCArglist *a)
{
START_WRAP
	setCDKScroll(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]),
cvt_vc__int((*a)[5]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKScroll(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(injectCDKScroll(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKScroll(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(activateCDKScroll(cvt_vc_p_SScroll_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKScroll(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SScroll_(newCDKScroll(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc_p_char((*a)[6]),
cvt_vc_pp_char((*a)[7]),
cvt_vc__int((*a)[8]),
cvt_vc__int((*a)[9]),
cvt_vc__long_unsigned_int((*a)[10]),
cvt_vc__int((*a)[11]),
cvt_vc__int((*a)[12])));
WRAP_END
return ret;

}
static vc
wrap_setCDKEntryPostProcess(VCArglist *a)
{
START_WRAP
	setCDKEntryPostProcess(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKEntryPreProcess(VCArglist *a)
{
START_WRAP
	setCDKEntryPreProcess(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc_BINDFN((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKEntryCB(VCArglist *a)
{
START_WRAP
	setCDKEntryCB(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc_ENTRYCB((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKEntry(VCArglist *a)
{
START_WRAP
	destroyCDKEntry(cvt_vc_p_SEntry_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_positionCDKEntry(VCArglist *a)
{
START_WRAP
	positionCDKEntry(cvt_vc_p_SEntry_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_moveCDKEntry(VCArglist *a)
{
START_WRAP
	moveCDKEntry(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_cleanCDKEntry(VCArglist *a)
{
START_WRAP
	cleanCDKEntry(cvt_vc_p_SEntry_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKEntry(VCArglist *a)
{
START_WRAP
	eraseCDKEntry(cvt_vc_p_SEntry_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKEntry(VCArglist *a)
{
START_WRAP
	drawCDKEntry(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKEntryBackgroundColor(VCArglist *a)
{
START_WRAP
	setCDKEntryBackgroundColor(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKEntryBoxAttribute(VCArglist *a)
{
START_WRAP
	setCDKEntryBoxAttribute(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKEntryHorizontalChar(VCArglist *a)
{
START_WRAP
	setCDKEntryHorizontalChar(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKEntryVerticalChar(VCArglist *a)
{
START_WRAP
	setCDKEntryVerticalChar(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKEntryLRChar(VCArglist *a)
{
START_WRAP
	setCDKEntryLRChar(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKEntryLLChar(VCArglist *a)
{
START_WRAP
	setCDKEntryLLChar(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKEntryURChar(VCArglist *a)
{
START_WRAP
	setCDKEntryURChar(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKEntryULChar(VCArglist *a)
{
START_WRAP
	setCDKEntryULChar(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKEntryBox(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKEntryBox(cvt_vc_p_SEntry_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKEntryBox(VCArglist *a)
{
START_WRAP
	setCDKEntryBox(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKEntryHiddenChar(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKEntryHiddenChar(cvt_vc_p_SEntry_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKEntryHiddenChar(VCArglist *a)
{
START_WRAP
	setCDKEntryHiddenChar(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKEntryFillerChar(VCArglist *a)
{
START_WRAP
	vc ret = cvt__long_unsigned_int(getCDKEntryFillerChar(cvt_vc_p_SEntry_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKEntryFillerChar(VCArglist *a)
{
START_WRAP
	setCDKEntryFillerChar(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKEntryMin(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKEntryMin(cvt_vc_p_SEntry_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKEntryMin(VCArglist *a)
{
START_WRAP
	setCDKEntryMin(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKEntryMax(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getCDKEntryMax(cvt_vc_p_SEntry_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKEntryMax(VCArglist *a)
{
START_WRAP
	setCDKEntryMax(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__int((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_getCDKEntryValue(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(getCDKEntryValue(cvt_vc_p_SEntry_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_setCDKEntryValue(VCArglist *a)
{
START_WRAP
	setCDKEntryValue(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_setCDKEntry(VCArglist *a)
{
START_WRAP
	setCDKEntry(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_injectCDKEntry(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(injectCDKEntry(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_activateCDKEntry(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(activateCDKEntry(cvt_vc_p_SEntry_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_newCDKEntry(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SEntry_(newCDKEntry(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc_p_char((*a)[4]),
cvt_vc__long_unsigned_int((*a)[5]),
cvt_vc__long_unsigned_int((*a)[6]),
cvt_vc__int((*a)[7]),
cvt_vc__int((*a)[8]),
cvt_vc__int((*a)[9]),
cvt_vc__int((*a)[10]),
cvt_vc__int((*a)[11]),
cvt_vc__int((*a)[12])));
WRAP_END
return ret;

}
static vc
wrap_deleteCursesWindow(VCArglist *a)
{
START_WRAP
	deleteCursesWindow(cvt_vc_p__win_st_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCursesWindow(VCArglist *a)
{
START_WRAP
	eraseCursesWindow(cvt_vc_p__win_st_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_setWidgetDimension(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(setWidgetDimension(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_checkForLink(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(checkForLink(cvt_vc_p_char((*a)[0]),
cvt_vc_p_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_mode2Char(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(mode2Char(cvt_vc_p_char((*a)[0]),
cvt_vc__unsigned_int((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_chlen(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(chlen(cvt_vc_p_long_unsigned_int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_copyChar(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(copyChar(cvt_vc_p_char((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_copyChtype(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_long_unsigned_int(copyChtype(cvt_vc_p_long_unsigned_int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_char2DisplayType(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(char2DisplayType(cvt_vc_p_char((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_char2Chtype(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_long_unsigned_int(char2Chtype(cvt_vc_p_char((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc_p_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_chtype2Char(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(chtype2Char(cvt_vc_p_long_unsigned_int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_cleanChtype(VCArglist *a)
{
START_WRAP
	cleanChtype(cvt_vc_p_long_unsigned_int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_cleanChar(VCArglist *a)
{
START_WRAP
	cleanChar(cvt_vc_p_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__char((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_freeChtype(VCArglist *a)
{
START_WRAP
	freeChtype(cvt_vc_p_long_unsigned_int((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_freeChar(VCArglist *a)
{
START_WRAP
	freeChar(cvt_vc_p_char((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_dirName(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(dirName(cvt_vc_p_char((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_baseName(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(baseName(cvt_vc_p_char((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_searchList(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(searchList(cvt_vc_pp_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_char((*a)[2])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_getDirectoryContents(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getDirectoryContents(cvt_vc_p_char((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3])));
WRAP_END
return ret;

}
#endif
static vc
wrap_intlen(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(intlen(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_splitString(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(splitString(cvt_vc_p_char((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__char((*a)[2])));
WRAP_END
return ret;

}
#endif
static vc
wrap_stripWhiteSpace(VCArglist *a)
{
START_WRAP
	stripWhiteSpace(cvt_vc__int((*a)[0]),
cvt_vc_p_char((*a)[1]));
WRAP_END
return(vcnil);

}
#if 0
static vc
wrap_readFile(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(readFile(cvt_vc_p_char((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_swapIndex(VCArglist *a)
{
START_WRAP
	swapIndex(cvt_vc_pp_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_quickSort(VCArglist *a)
{
START_WRAP
	quickSort(cvt_vc_pp_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
#endif
static vc
wrap_viewInfo(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(viewInfo(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc_pp_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_pp_char((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6])));
WRAP_END
return ret;

}
static vc
wrap_selectFile(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(selectFile(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc_p_char((*a)[1])));
WRAP_END
return ret;

}
static vc
wrap_viewFile(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(viewFile(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc_p_char((*a)[2]),
cvt_vc_pp_char((*a)[3]),
cvt_vc__int((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_getString(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(getString(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc_p_char((*a)[2]),
cvt_vc_p_char((*a)[3])));
WRAP_END
return ret;

}
static vc
wrap_getListIndex(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(getListIndex(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc_pp_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_popupDialog(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(popupDialog(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pp_char((*a)[3]),
cvt_vc__int((*a)[4])));
WRAP_END
return ret;

}
static vc
wrap_popupLabel(VCArglist *a)
{
START_WRAP
	popupLabel(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc_pp_char((*a)[1]),
cvt_vc__int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_justifyString(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(justifyString(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
#if 0
static vc
wrap_substring(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_char(substring(cvt_vc_p_char((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));
WRAP_END
return ret;

}
#endif
static vc
wrap_alignxy(VCArglist *a)
{
START_WRAP
	alignxy(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc_p_int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_Beep(VCArglist *a)
{
START_WRAP
	Beep();
WRAP_END
return(vcnil);

}
static vc
wrap_cleanCDKObjectBindings(VCArglist *a)
{
START_WRAP
	cleanCDKObjectBindings(cvt_vc__int((*a)[0]),
cvt_vc_p_void((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_checkCDKObjectBind(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(checkCDKObjectBind(cvt_vc__int((*a)[0]),
cvt_vc_p_void((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2])));
WRAP_END
return ret;

}
static vc
wrap_unbindCDKObject(VCArglist *a)
{
START_WRAP
	unbindCDKObject(cvt_vc__int((*a)[0]),
cvt_vc_p_void((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2]));
WRAP_END
return(vcnil);

}
static vc
wrap_bindCDKObject(VCArglist *a)
{
START_WRAP
	bindCDKObject(cvt_vc__int((*a)[0]),
cvt_vc_p_void((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2]),
cvt_vc_BINDFN((*a)[3]),
cvt_vc_p_void((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_initCDKColor(VCArglist *a)
{
START_WRAP
	initCDKColor();
WRAP_END
return(vcnil);

}
static vc
wrap_endCDK(VCArglist *a)
{
START_WRAP
	endCDK();
WRAP_END
return(vcnil);

}
static vc
wrap_destroyCDKScreen(VCArglist *a)
{
START_WRAP
	destroyCDKScreen(cvt_vc_p_SScreen_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_eraseCDKScreen(VCArglist *a)
{
START_WRAP
	eraseCDKScreen(cvt_vc_p_SScreen_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_drawCDKScreen(VCArglist *a)
{
START_WRAP
	drawCDKScreen(cvt_vc_p_SScreen_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_refreshCDKScreen(VCArglist *a)
{
START_WRAP
	refreshCDKScreen(cvt_vc_p_SScreen_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_lowerCDKObject(VCArglist *a)
{
START_WRAP
	lowerCDKObject(cvt_vc__int((*a)[0]),
cvt_vc_p_void((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_raiseCDKObject(VCArglist *a)
{
START_WRAP
	raiseCDKObject(cvt_vc__int((*a)[0]),
cvt_vc_p_void((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_unregisterCDKObject(VCArglist *a)
{
START_WRAP
	unregisterCDKObject(cvt_vc__int((*a)[0]),
cvt_vc_p_void((*a)[1]));
WRAP_END
return(vcnil);

}
static vc
wrap_registerCDKObject(VCArglist *a)
{
START_WRAP
	registerCDKObject(cvt_vc_p_SScreen_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_void((*a)[2]));
WRAP_END
return(vcnil);

}
#if 0
static vc
wrap_setDefaultCDKScreen(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SScreen_(setDefaultCDKScreen(cvt_vc__int((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_lowerCDKScreen(VCArglist *a)
{
START_WRAP
	lowerCDKScreen(cvt_vc_pp_SScreen_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_raiseCDKScreen(VCArglist *a)
{
START_WRAP
	raiseCDKScreen(cvt_vc_pp_SScreen_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_unregisterCDKScreen(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(unregisterCDKScreen(cvt_vc_pp_SScreen_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_registerCDKScreen(VCArglist *a)
{
START_WRAP
	vc ret = cvt__int(registerCDKScreen(cvt_vc_pp_SScreen_((*a)[0])));
WRAP_END
return ret;

}
#endif
static vc
wrap_initCDKScreen(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p_SScreen_(initCDKScreen(cvt_vc_p__win_st_((*a)[0])));
WRAP_END
return ret;

}
static vc
wrap_stopCDKDebug(VCArglist *a)
{
START_WRAP
	stopCDKDebug(cvt_vc_p__IO_FILE_((*a)[0]));
WRAP_END
return(vcnil);

}
static vc
wrap_writeCDKDebugMessage(VCArglist *a)
{
START_WRAP
	writeCDKDebugMessage(cvt_vc_p__IO_FILE_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc_p_char((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc_p_char((*a)[4]));
WRAP_END
return(vcnil);

}
static vc
wrap_startCDKDebug(VCArglist *a)
{
START_WRAP
	vc ret = cvt_p__IO_FILE_(startCDKDebug(cvt_vc_p_char((*a)[0])));
WRAP_END
return ret;

}

static void
makefun(const char *name, const vc& fun)
{
    vc(name).bind(fun);
}
void
wrapper_init_cdk_xml ()
{
initscr();
vc("wrap_stdscr").global_bind(vc(VC_INT_DTOR, 0, (long)stdscr));

//makefun("wrap_setDefaultCDKScreen", vc(wrap_setDefaultCDKScreen, "wrap_setDefaultCDKScreen", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_registerCDKObject", vc(wrap_registerCDKObject, "wrap_registerCDKObject", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_baseName", vc(wrap_baseName, "wrap_baseName", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryValue", vc(wrap_setCDKEntryValue, "wrap_setCDKEntryValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKEntryFillerChar", vc(wrap_getCDKEntryFillerChar, "wrap_getCDKEntryFillerChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKEntryHiddenChar", vc(wrap_getCDKEntryHiddenChar, "wrap_getCDKEntryHiddenChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScrollULChar", vc(wrap_setCDKScrollULChar, "wrap_setCDKScrollULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalistBox", vc(wrap_setCDKAlphalistBox, "wrap_setCDKAlphalistBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKButtonboxBoxAttribute", vc(wrap_setCDKButtonboxBoxAttribute, "wrap_setCDKButtonboxBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarPostProcess", vc(wrap_setCDKCalendarPostProcess, "wrap_setCDKCalendarPostProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKDialogSeparator", vc(wrap_setCDKDialogSeparator, "wrap_setCDKDialogSeparator", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_getCDKGraphValues", vc(wrap_getCDKGraphValues, "wrap_getCDKGraphValues", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKGraphBoxAttribute", vc(wrap_setCDKGraphBoxAttribute, "wrap_setCDKGraphBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKHistogram", vc(wrap_activateCDKHistogram, "wrap_activateCDKHistogram", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKHistogramStatsAttr", vc(wrap_getCDKHistogramStatsAttr, "wrap_getCDKHistogramStatsAttr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMarqueeLLChar", vc(wrap_setCDKMarqueeLLChar, "wrap_setCDKMarqueeLLChar", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_setCDKMatrix", vc(wrap_setCDKMatrix, "wrap_setCDKMatrix", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMatrixVerticalChar", vc(wrap_setCDKMatrixVerticalChar, "wrap_setCDKMatrixVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMatrixBoxAttribute", vc(wrap_setCDKMatrixBoxAttribute, "wrap_setCDKMatrixBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKMentry", vc(wrap_destroyCDKMentry, "wrap_destroyCDKMentry", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_drawCDKMenuTitles", vc(wrap_drawCDKMenuTitles, "wrap_drawCDKMenuTitles", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKMenuSubwin", vc(wrap_drawCDKMenuSubwin, "wrap_drawCDKMenuSubwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScaleLowHigh", vc(wrap_setCDKScaleLowHigh, "wrap_setCDKScaleLowHigh", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKSelectionTitle", vc(wrap_getCDKSelectionTitle, "wrap_getCDKSelectionTitle", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSliderLRChar", vc(wrap_setCDKSliderLRChar, "wrap_setCDKSliderLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKSwindow", vc(wrap_injectCDKSwindow, "wrap_injectCDKSwindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_loadCDKSwindowInformation", vc(wrap_loadCDKSwindowInformation, "wrap_loadCDKSwindowInformation", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplateBackgroundColor", vc(wrap_setCDKTemplateBackgroundColor, "wrap_setCDKTemplateBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKTemplate", vc(wrap_destroyCDKTemplate, "wrap_destroyCDKTemplate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKViewer", vc(wrap_positionCDKViewer, "wrap_positionCDKViewer", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawLine", vc(wrap_drawLine, "wrap_drawLine", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_getDirectoryContents", vc(wrap_getDirectoryContents, "wrap_getDirectoryContents", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_copyChar", vc(wrap_copyChar, "wrap_copyChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryMax", vc(wrap_setCDKEntryMax, "wrap_setCDKEntryMax", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKAlphalist", vc(wrap_moveCDKAlphalist, "wrap_moveCDKAlphalist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarPreProcess", vc(wrap_setCDKCalendarPreProcess, "wrap_setCDKCalendarPreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKDialogBox", vc(wrap_setCDKDialogBox, "wrap_setCDKDialogBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKDialog", vc(wrap_drawCDKDialog, "wrap_drawCDKDialog", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectFileAttribute", vc(wrap_setCDKFselectFileAttribute, "wrap_setCDKFselectFileAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKGraphHorizontalChar", vc(wrap_setCDKGraphHorizontalChar, "wrap_setCDKGraphHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKGraph", vc(wrap_eraseCDKGraph, "wrap_eraseCDKGraph", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKHistogramValue", vc(wrap_getCDKHistogramValue, "wrap_getCDKHistogramValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlistLLChar", vc(wrap_setCDKItemlistLLChar, "wrap_setCDKItemlistLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlistLRChar", vc(wrap_setCDKItemlistLRChar, "wrap_setCDKItemlistLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryCB", vc(wrap_setCDKMentryCB, "wrap_setCDKMentryCB", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMenuPostProcess", vc(wrap_setCDKMenuPostProcess, "wrap_setCDKMenuPostProcess", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_getCDKSelectionModes", vc(wrap_getCDKSelectionModes, "wrap_getCDKSelectionModes", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSwindowBoxAttribute", vc(wrap_setCDKSwindowBoxAttribute, "wrap_setCDKSwindowBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKViewerInfo", vc(wrap_getCDKViewerInfo, "wrap_getCDKViewerInfo", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_writeChtypeAttrib", vc(wrap_writeChtypeAttrib, "wrap_writeChtypeAttrib", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_writeCDKDebugMessage", vc(wrap_writeCDKDebugMessage, "wrap_writeCDKDebugMessage", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_char2DisplayType", vc(wrap_char2DisplayType, "wrap_char2DisplayType", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKEntryValue", vc(wrap_getCDKEntryValue, "wrap_getCDKEntryValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryPostProcess", vc(wrap_setCDKEntryPostProcess, "wrap_setCDKEntryPostProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_deleteCDKScrollItem", vc(wrap_deleteCDKScrollItem, "wrap_deleteCDKScrollItem", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalist", vc(wrap_setCDKAlphalist, "wrap_setCDKAlphalist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKAlphalistBox", vc(wrap_getCDKAlphalistBox, "wrap_getCDKAlphalistBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarBox", vc(wrap_setCDKCalendarBox, "wrap_setCDKCalendarBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKDialogSeparator", vc(wrap_getCDKDialogSeparator, "wrap_getCDKDialogSeparator", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKDialogLLChar", vc(wrap_setCDKDialogLLChar, "wrap_setCDKDialogLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKLabel", vc(wrap_activateCDKLabel, "wrap_activateCDKLabel", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKFselect", vc(wrap_moveCDKFselect, "wrap_moveCDKFselect", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogramURChar", vc(wrap_setCDKHistogramURChar, "wrap_setCDKHistogramURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKHistogram", vc(wrap_eraseCDKHistogram, "wrap_eraseCDKHistogram", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMarqueeULChar", vc(wrap_setCDKMarqueeULChar, "wrap_setCDKMarqueeULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKMatrix", vc(wrap_moveCDKMatrix, "wrap_moveCDKMatrix", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMenuPreProcess", vc(wrap_setCDKMenuPreProcess, "wrap_setCDKMenuPreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKScale", vc(wrap_injectCDKScale, "wrap_injectCDKScale", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_execCDKSwindow", vc(wrap_execCDKSwindow, "wrap_execCDKSwindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKTemplate", vc(wrap_moveCDKTemplate, "wrap_moveCDKTemplate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKViewer", vc(wrap_newCDKViewer, "wrap_newCDKViewer", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_registerCDKScreen", vc(wrap_registerCDKScreen, "wrap_registerCDKScreen", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_unbindCDKObject", vc(wrap_unbindCDKObject, "wrap_unbindCDKObject", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_viewFile", vc(wrap_viewFile, "wrap_viewFile", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_checkForLink", vc(wrap_checkForLink, "wrap_checkForLink", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKEntryMax", vc(wrap_getCDKEntryMax, "wrap_getCDKEntryMax", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKAlphalist", vc(wrap_activateCDKAlphalist, "wrap_activateCDKAlphalist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKAlphalist", vc(wrap_eraseCDKAlphalist, "wrap_eraseCDKAlphalist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKButtonbox", vc(wrap_eraseCDKButtonbox, "wrap_eraseCDKButtonbox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarYearAttribute", vc(wrap_setCDKCalendarYearAttribute, "wrap_setCDKCalendarYearAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKDialogBox", vc(wrap_getCDKDialogBox, "wrap_getCDKDialogBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKFselectFileAttribute", vc(wrap_getCDKFselectFileAttribute, "wrap_getCDKFselectFileAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogramULChar", vc(wrap_setCDKHistogramULChar, "wrap_setCDKHistogramULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKMatrix", vc(wrap_newCDKMatrix, "wrap_newCDKMatrix", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryLRChar", vc(wrap_setCDKMentryLRChar, "wrap_setCDKMentryLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKMenu", vc(wrap_injectCDKMenu, "wrap_injectCDKMenu", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKRadio", vc(wrap_destroyCDKRadio, "wrap_destroyCDKRadio", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioPostProcess", vc(wrap_setCDKRadioPostProcess, "wrap_setCDKRadioPostProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKScale", vc(wrap_drawCDKScale, "wrap_drawCDKScale", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionPostProcess", vc(wrap_setCDKSelectionPostProcess, "wrap_setCDKSelectionPostProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSliderValue", vc(wrap_setCDKSliderValue, "wrap_setCDKSliderValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKSwindow", vc(wrap_destroyCDKSwindow, "wrap_destroyCDKSwindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKViewer", vc(wrap_eraseCDKViewer, "wrap_eraseCDKViewer", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKEntry", vc(wrap_positionCDKEntry, "wrap_positionCDKEntry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalistBackgroundColor", vc(wrap_setCDKAlphalistBackgroundColor, "wrap_setCDKAlphalistBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKButtonboxVerticalChar", vc(wrap_setCDKButtonboxVerticalChar, "wrap_setCDKButtonboxVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKButtonboxPreProcess", vc(wrap_setCDKButtonboxPreProcess, "wrap_setCDKButtonboxPreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKCalendarBox", vc(wrap_getCDKCalendarBox, "wrap_getCDKCalendarBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKLabelHorizontalChar", vc(wrap_setCDKLabelHorizontalChar, "wrap_setCDKLabelHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKLabel", vc(wrap_moveCDKLabel, "wrap_moveCDKLabel", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKFselect", vc(wrap_activateCDKFselect, "wrap_activateCDKFselect", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKGraphCharacter", vc(wrap_setCDKGraphCharacter, "wrap_setCDKGraphCharacter", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKGraphVerticalChar", vc(wrap_setCDKGraphVerticalChar, "wrap_setCDKGraphVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogram", vc(wrap_setCDKHistogram, "wrap_setCDKHistogram", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogramLRChar", vc(wrap_setCDKHistogramLRChar, "wrap_setCDKHistogramLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlistBoxAttribute", vc(wrap_setCDKItemlistBoxAttribute, "wrap_setCDKItemlistBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKMatrix", vc(wrap_activateCDKMatrix, "wrap_activateCDKMatrix", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKMatrixRow", vc(wrap_getCDKMatrixRow, "wrap_getCDKMatrixRow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMatrixULChar", vc(wrap_setCDKMatrixULChar, "wrap_setCDKMatrixULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveToCDKMatrixCell", vc(wrap_moveToCDKMatrixCell, "wrap_moveToCDKMatrixCell", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKMentry", vc(wrap_eraseCDKMentry, "wrap_eraseCDKMentry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScaleURChar", vc(wrap_setCDKScaleURChar, "wrap_setCDKScaleURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKTemplate", vc(wrap_activateCDKTemplate, "wrap_activateCDKTemplate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplateValue", vc(wrap_setCDKTemplateValue, "wrap_setCDKTemplateValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplateVerticalChar", vc(wrap_setCDKTemplateVerticalChar, "wrap_setCDKTemplateVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKViewerHorizontalChar", vc(wrap_setCDKViewerHorizontalChar, "wrap_setCDKViewerHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKScreen", vc(wrap_destroyCDKScreen, "wrap_destroyCDKScreen", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_popupLabel", vc(wrap_popupLabel, "wrap_popupLabel", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_popupDialog", vc(wrap_popupDialog, "wrap_popupDialog", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCursesWindow", vc(wrap_eraseCursesWindow, "wrap_eraseCursesWindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKScroll", vc(wrap_destroyCDKScroll, "wrap_destroyCDKScroll", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKAlphalist", vc(wrap_newCDKAlphalist, "wrap_newCDKAlphalist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalistFillerChar", vc(wrap_setCDKAlphalistFillerChar, "wrap_setCDKAlphalistFillerChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKButtonbox", vc(wrap_setCDKButtonbox, "wrap_setCDKButtonbox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKButtonboxBox", vc(wrap_setCDKButtonboxBox, "wrap_setCDKButtonboxBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKButtonboxHorizontalChar", vc(wrap_setCDKButtonboxHorizontalChar, "wrap_setCDKButtonboxHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKCalendarYearAttribute", vc(wrap_getCDKCalendarYearAttribute, "wrap_getCDKCalendarYearAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKGraphBox", vc(wrap_setCDKGraphBox, "wrap_setCDKGraphBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKGraph", vc(wrap_drawCDKGraph, "wrap_drawCDKGraph", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogramLLChar", vc(wrap_setCDKHistogramLLChar, "wrap_setCDKHistogramLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlistHorizontalChar", vc(wrap_setCDKItemlistHorizontalChar, "wrap_setCDKItemlistHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMatrixHorizontalChar", vc(wrap_setCDKMatrixHorizontalChar, "wrap_setCDKMatrixHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMatrixPreProcess", vc(wrap_setCDKMatrixPreProcess, "wrap_setCDKMatrixPreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMenuCurrentItem", vc(wrap_setCDKMenuCurrentItem, "wrap_setCDKMenuCurrentItem", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKRadio", vc(wrap_drawCDKRadio, "wrap_drawCDKRadio", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScaleULChar", vc(wrap_setCDKScaleULChar, "wrap_setCDKScaleULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKSliderValue", vc(wrap_getCDKSliderValue, "wrap_getCDKSliderValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_cleanCDKObjectBindings", vc(wrap_cleanCDKObjectBindings, "wrap_cleanCDKObjectBindings", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_cleanChtype", vc(wrap_cleanChtype, "wrap_cleanChtype", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_char2Chtype", vc(wrap_char2Chtype, "wrap_char2Chtype", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKButtonbox", vc(wrap_injectCDKButtonbox, "wrap_injectCDKButtonbox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKDialog", vc(wrap_positionCDKDialog, "wrap_positionCDKDialog", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKLabelMessage", vc(wrap_setCDKLabelMessage, "wrap_setCDKLabelMessage", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectDirAttribute", vc(wrap_setCDKFselectDirAttribute, "wrap_setCDKFselectDirAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectLRChar", vc(wrap_setCDKFselectLRChar, "wrap_setCDKFselectLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKGraphCharacter", vc(wrap_getCDKGraphCharacter, "wrap_getCDKGraphCharacter", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKGraph", vc(wrap_destroyCDKGraph, "wrap_destroyCDKGraph", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKItemlist", vc(wrap_positionCDKItemlist, "wrap_positionCDKItemlist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKItemlist", vc(wrap_destroyCDKItemlist, "wrap_destroyCDKItemlist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKMatrix", vc(wrap_injectCDKMatrix, "wrap_injectCDKMatrix", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryValue", vc(wrap_setCDKMentryValue, "wrap_setCDKMentryValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScaleLRChar", vc(wrap_setCDKScaleLRChar, "wrap_setCDKScaleLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKSlider", vc(wrap_eraseCDKSlider, "wrap_eraseCDKSlider", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSliderPostProcess", vc(wrap_setCDKSliderPostProcess, "wrap_setCDKSliderPostProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKSwindow", vc(wrap_drawCDKSwindow, "wrap_drawCDKSwindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKTemplateValue", vc(wrap_getCDKTemplateValue, "wrap_getCDKTemplateValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_deleteCursesWindow", vc(wrap_deleteCursesWindow, "wrap_deleteCursesWindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryURChar", vc(wrap_setCDKEntryURChar, "wrap_setCDKEntryURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKAlphalistFillerChar", vc(wrap_getCDKAlphalistFillerChar, "wrap_getCDKAlphalistFillerChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKButtonboxBox", vc(wrap_getCDKButtonboxBox, "wrap_getCDKButtonboxBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarDate", vc(wrap_setCDKCalendarDate, "wrap_setCDKCalendarDate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarMonthAttribute", vc(wrap_setCDKCalendarMonthAttribute, "wrap_setCDKCalendarMonthAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarVerticalChar", vc(wrap_setCDKCalendarVerticalChar, "wrap_setCDKCalendarVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarBackgroundColor", vc(wrap_setCDKCalendarBackgroundColor, "wrap_setCDKCalendarBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKCalendar", vc(wrap_drawCDKCalendar, "wrap_drawCDKCalendar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKDialogURChar", vc(wrap_setCDKDialogURChar, "wrap_setCDKDialogURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKLabelVerticalChar", vc(wrap_setCDKLabelVerticalChar, "wrap_setCDKLabelVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKGraphBox", vc(wrap_getCDKGraphBox, "wrap_getCDKGraphBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKHistogram", vc(wrap_newCDKHistogram, "wrap_newCDKHistogram", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKMatrix", vc(wrap_eraseCDKMatrix, "wrap_eraseCDKMatrix", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKMenuCurrentItem", vc(wrap_getCDKMenuCurrentItem, "wrap_getCDKMenuCurrentItem", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScaleLLChar", vc(wrap_setCDKScaleLLChar, "wrap_setCDKScaleLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKViewerULChar", vc(wrap_setCDKViewerULChar, "wrap_setCDKViewerULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_unregisterCDKObject", vc(wrap_unregisterCDKObject, "wrap_unregisterCDKObject", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_readFile", vc(wrap_readFile, "wrap_readFile", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntry", vc(wrap_setCDKEntry, "wrap_setCDKEntry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryULChar", vc(wrap_setCDKEntryULChar, "wrap_setCDKEntryULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScrollLRChar", vc(wrap_setCDKScrollLRChar, "wrap_setCDKScrollLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKButtonbox", vc(wrap_newCDKButtonbox, "wrap_newCDKButtonbox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKLabelMessage", vc(wrap_getCDKLabelMessage, "wrap_getCDKLabelMessage", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKLabelBox", vc(wrap_setCDKLabelBox, "wrap_setCDKLabelBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectSocketAttribute", vc(wrap_setCDKFselectSocketAttribute, "wrap_setCDKFselectSocketAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKFselectDirAttribute", vc(wrap_getCDKFselectDirAttribute, "wrap_getCDKFselectDirAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectURChar", vc(wrap_setCDKFselectURChar, "wrap_setCDKFselectURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKFselect", vc(wrap_eraseCDKFselect, "wrap_eraseCDKFselect", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKMentryValue", vc(wrap_getCDKMentryValue, "wrap_getCDKMentryValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_cleanCDKMentry", vc(wrap_cleanCDKMentry, "wrap_cleanCDKMentry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMenuBackgroundColor", vc(wrap_setCDKMenuBackgroundColor, "wrap_setCDKMenuBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioURChar", vc(wrap_setCDKRadioURChar, "wrap_setCDKRadioURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScaleBoxAttribute", vc(wrap_setCDKScaleBoxAttribute, "wrap_setCDKScaleBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKScale", vc(wrap_positionCDKScale, "wrap_positionCDKScale", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionChoice", vc(wrap_setCDKSelectionChoice, "wrap_setCDKSelectionChoice", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSliderLowHigh", vc(wrap_setCDKSliderLowHigh, "wrap_setCDKSliderLowHigh", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSliderBox", vc(wrap_setCDKSliderBox, "wrap_setCDKSliderBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKTemplate", vc(wrap_eraseCDKTemplate, "wrap_eraseCDKTemplate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKViewerTitle", vc(wrap_setCDKViewerTitle, "wrap_setCDKViewerTitle", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_stopCDKDebug", vc(wrap_stopCDKDebug, "wrap_stopCDKDebug", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryLRChar", vc(wrap_setCDKEntryLRChar, "wrap_setCDKEntryLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKScroll", vc(wrap_moveCDKScroll, "wrap_moveCDKScroll", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalistPostProcess", vc(wrap_setCDKAlphalistPostProcess, "wrap_setCDKAlphalistPostProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKButtonboxBackgroundColor", vc(wrap_setCDKButtonboxBackgroundColor, "wrap_setCDKButtonboxBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKButtonbox", vc(wrap_drawCDKButtonbox, "wrap_drawCDKButtonbox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKCalendarDate", vc(wrap_getCDKCalendarDate, "wrap_getCDKCalendarDate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKCalendarMonthAttribute", vc(wrap_getCDKCalendarMonthAttribute, "wrap_getCDKCalendarMonthAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectBox", vc(wrap_setCDKFselectBox, "wrap_setCDKFselectBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKGraphBackgroundColor", vc(wrap_setCDKGraphBackgroundColor, "wrap_setCDKGraphBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKMarquee", vc(wrap_newCDKMarquee, "wrap_newCDKMarquee", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKMarquee", vc(wrap_moveCDKMarquee, "wrap_moveCDKMarquee", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMarqueeVerticalChar", vc(wrap_setCDKMarqueeVerticalChar, "wrap_setCDKMarqueeVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKMatrix", vc(wrap_destroyCDKMatrix, "wrap_destroyCDKMatrix", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryPostProcess", vc(wrap_setCDKMentryPostProcess, "wrap_setCDKMentryPostProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMenuTitleHighlight", vc(wrap_setCDKMenuTitleHighlight, "wrap_setCDKMenuTitleHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioULChar", vc(wrap_setCDKRadioULChar, "wrap_setCDKRadioULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSliderLLChar", vc(wrap_setCDKSliderLLChar, "wrap_setCDKSliderLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKSwindow", vc(wrap_eraseCDKSwindow, "wrap_eraseCDKSwindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_cleanCDKSwindow", vc(wrap_cleanCDKSwindow, "wrap_cleanCDKSwindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplateMin", vc(wrap_setCDKTemplateMin, "wrap_setCDKTemplateMin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_attrbox", vc(wrap_attrbox, "wrap_attrbox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawShadow", vc(wrap_drawShadow, "wrap_drawShadow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKScreen", vc(wrap_drawCDKScreen, "wrap_drawCDKScreen", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_checkCDKObjectBind", vc(wrap_checkCDKObjectBind, "wrap_checkCDKObjectBind", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_selectFile", vc(wrap_selectFile, "wrap_selectFile", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryLLChar", vc(wrap_setCDKEntryLLChar, "wrap_setCDKEntryLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendar", vc(wrap_setCDKCalendar, "wrap_setCDKCalendar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKDialog", vc(wrap_injectCDKDialog, "wrap_injectCDKDialog", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKDialogBackgroundColor", vc(wrap_setCDKDialogBackgroundColor, "wrap_setCDKDialogBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKLabelBox", vc(wrap_getCDKLabelBox, "wrap_getCDKLabelBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKFselectSocketAttribute", vc(wrap_getCDKFselectSocketAttribute, "wrap_getCDKFselectSocketAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectDirContents", vc(wrap_setCDKFselectDirContents, "wrap_setCDKFselectDirContents", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectHorizontalChar", vc(wrap_setCDKFselectHorizontalChar, "wrap_setCDKFselectHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKGraph", vc(wrap_positionCDKGraph, "wrap_positionCDKGraph", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogramDisplayType", vc(wrap_setCDKHistogramDisplayType, "wrap_setCDKHistogramDisplayType", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogramBoxAttribute", vc(wrap_setCDKHistogramBoxAttribute, "wrap_setCDKHistogramBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKItemlist", vc(wrap_eraseCDKItemlist, "wrap_eraseCDKItemlist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlistPostProcess", vc(wrap_setCDKItemlistPostProcess, "wrap_setCDKItemlistPostProcess", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_setCDKMatrixCB", vc(wrap_setCDKMatrixCB, "wrap_setCDKMatrixCB", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioLRChar", vc(wrap_setCDKRadioLRChar, "wrap_setCDKRadioLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKRadio", vc(wrap_positionCDKRadio, "wrap_positionCDKRadio", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKSelectionChoice", vc(wrap_getCDKSelectionChoice, "wrap_getCDKSelectionChoice", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKSliderBox", vc(wrap_getCDKSliderBox, "wrap_getCDKSliderBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_saveCDKSwindowInformation", vc(wrap_saveCDKSwindowInformation, "wrap_saveCDKSwindowInformation", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSwindowPostProcess", vc(wrap_setCDKSwindowPostProcess, "wrap_setCDKSwindowPostProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplateULChar", vc(wrap_setCDKTemplateULChar, "wrap_setCDKTemplateULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplateURChar", vc(wrap_setCDKTemplateURChar, "wrap_setCDKTemplateURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKViewerTitle", vc(wrap_getCDKViewerTitle, "wrap_getCDKViewerTitle", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_alignxy", vc(wrap_alignxy, "wrap_alignxy", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_freeChar", vc(wrap_freeChar, "wrap_freeChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKScroll", vc(wrap_activateCDKScroll, "wrap_activateCDKScroll", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScrollHighlight", vc(wrap_setCDKScrollHighlight, "wrap_setCDKScrollHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKFselectBox", vc(wrap_getCDKFselectBox, "wrap_getCDKFselectBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlistValues", vc(wrap_setCDKItemlistValues, "wrap_setCDKItemlistValues", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlistPreProcess", vc(wrap_setCDKItemlistPreProcess, "wrap_setCDKItemlistPreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKMarquee", vc(wrap_activateCDKMarquee, "wrap_activateCDKMarquee", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMarqueeBackgroundColor", vc(wrap_setCDKMarqueeBackgroundColor, "wrap_setCDKMarqueeBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_cleanCDKMatrix", vc(wrap_cleanCDKMatrix, "wrap_cleanCDKMatrix", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryBox", vc(wrap_setCDKMentryBox, "wrap_setCDKMentryBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKMenu", vc(wrap_activateCDKMenu, "wrap_activateCDKMenu", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKMenuTitleHighlight", vc(wrap_getCDKMenuTitleHighlight, "wrap_getCDKMenuTitleHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMenuSubTitleHighlight", vc(wrap_setCDKMenuSubTitleHighlight, "wrap_setCDKMenuSubTitleHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioLLChar", vc(wrap_setCDKRadioLLChar, "wrap_setCDKRadioLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKSelection", vc(wrap_drawCDKSelection, "wrap_drawCDKSelection", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKSlider", vc(wrap_moveCDKSlider, "wrap_moveCDKSlider", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKSwindow", vc(wrap_positionCDKSwindow, "wrap_positionCDKSwindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKTemplateMin", vc(wrap_getCDKTemplateMin, "wrap_getCDKTemplateMin", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_unregisterCDKScreen", vc(wrap_unregisterCDKScreen, "wrap_unregisterCDKScreen", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKDialog", vc(wrap_setCDKDialog, "wrap_setCDKDialog", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKDialogVerticalChar", vc(wrap_setCDKDialogVerticalChar, "wrap_setCDKDialogVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKDialogBoxAttribute", vc(wrap_setCDKDialogBoxAttribute, "wrap_setCDKDialogBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKLabelBackgroundColor", vc(wrap_setCDKLabelBackgroundColor, "wrap_setCDKLabelBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKFselectDirContents", vc(wrap_getCDKFselectDirContents, "wrap_getCDKFselectDirContents", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectLLChar", vc(wrap_setCDKFselectLLChar, "wrap_setCDKFselectLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKFselect", vc(wrap_drawCDKFselect, "wrap_drawCDKFselect", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKHistogram", vc(wrap_drawCDKHistogram, "wrap_drawCDKHistogram", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlistBox", vc(wrap_setCDKItemlistBox, "wrap_setCDKItemlistBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKItemlistField", vc(wrap_drawCDKItemlistField, "wrap_drawCDKItemlistField", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMatrixLRChar", vc(wrap_setCDKMatrixLRChar, "wrap_setCDKMatrixLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKMentry", vc(wrap_activateCDKMentry, "wrap_activateCDKMentry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryLLChar", vc(wrap_setCDKMentryLLChar, "wrap_setCDKMentryLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKMentryField", vc(wrap_drawCDKMentryField, "wrap_drawCDKMentryField", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScalePreProcess", vc(wrap_setCDKScalePreProcess, "wrap_setCDKScalePreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionURChar", vc(wrap_setCDKSelectionURChar, "wrap_setCDKSelectionURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_cleanCDKTemplate", vc(wrap_cleanCDKTemplate, "wrap_cleanCDKTemplate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKViewer", vc(wrap_destroyCDKViewer, "wrap_destroyCDKViewer", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKScrollHighlight", vc(wrap_getCDKScrollHighlight, "wrap_getCDKScrollHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKCalendar", vc(wrap_newCDKCalendar, "wrap_newCDKCalendar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarHighlight", vc(wrap_setCDKCalendarHighlight, "wrap_setCDKCalendarHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarULChar", vc(wrap_setCDKCalendarULChar, "wrap_setCDKCalendarULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarURChar", vc(wrap_setCDKCalendarURChar, "wrap_setCDKCalendarURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKLabel", vc(wrap_setCDKLabel, "wrap_setCDKLabel", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectFillerChar", vc(wrap_setCDKFselectFillerChar, "wrap_setCDKFselectFillerChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectLinkAttribute", vc(wrap_setCDKFselectLinkAttribute, "wrap_setCDKFselectLinkAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKItemlistValues", vc(wrap_getCDKItemlistValues, "wrap_getCDKItemlistValues", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMarqueeBoxAttribute", vc(wrap_setCDKMarqueeBoxAttribute, "wrap_setCDKMarqueeBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKMentryBox", vc(wrap_getCDKMentryBox, "wrap_getCDKMentryBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKMenuSubTitleHighlight", vc(wrap_getCDKMenuSubTitleHighlight, "wrap_getCDKMenuSubTitleHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioHighlight", vc(wrap_setCDKRadioHighlight, "wrap_setCDKRadioHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScaleHorizontalChar", vc(wrap_setCDKScaleHorizontalChar, "wrap_setCDKScaleHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKScale", vc(wrap_eraseCDKScale, "wrap_eraseCDKScale", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionHighlight", vc(wrap_setCDKSelectionHighlight, "wrap_setCDKSelectionHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionULChar", vc(wrap_setCDKSelectionULChar, "wrap_setCDKSelectionULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKSlider", vc(wrap_activateCDKSlider, "wrap_activateCDKSlider", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSliderBackgroundColor", vc(wrap_setCDKSliderBackgroundColor, "wrap_setCDKSliderBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSwindowLRChar", vc(wrap_setCDKSwindowLRChar, "wrap_setCDKSwindowLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSwindowPreProcess", vc(wrap_setCDKSwindowPreProcess, "wrap_setCDKSwindowPreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_writeCharAttrib", vc(wrap_writeCharAttrib, "wrap_writeCharAttrib", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getListIndex", vc(wrap_getListIndex, "wrap_getListIndex", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScrollPostProcess", vc(wrap_setCDKScrollPostProcess, "wrap_setCDKScrollPostProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKButtonboxButtons", vc(wrap_drawCDKButtonboxButtons, "wrap_drawCDKButtonboxButtons", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKButtonbox", vc(wrap_positionCDKButtonbox, "wrap_positionCDKButtonbox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKButtonbox", vc(wrap_destroyCDKButtonbox, "wrap_destroyCDKButtonbox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKButtonboxPostProcess", vc(wrap_setCDKButtonboxPostProcess, "wrap_setCDKButtonboxPostProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKLabel", vc(wrap_drawCDKLabel, "wrap_drawCDKLabel", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselect", vc(wrap_setCDKFselect, "wrap_setCDKFselect", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectULChar", vc(wrap_setCDKFselectULChar, "wrap_setCDKFselectULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKHistogramLowValue", vc(wrap_getCDKHistogramLowValue, "wrap_getCDKHistogramLowValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlistCurrentItem", vc(wrap_setCDKItemlistCurrentItem, "wrap_setCDKItemlistCurrentItem", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKItemlistBox", vc(wrap_getCDKItemlistBox, "wrap_getCDKItemlistBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKMentry", vc(wrap_moveCDKMentry, "wrap_moveCDKMentry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioChoiceCharacter", vc(wrap_setCDKRadioChoiceCharacter, "wrap_setCDKRadioChoiceCharacter", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioLeftBrace", vc(wrap_setCDKRadioLeftBrace, "wrap_setCDKRadioLeftBrace", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScale", vc(wrap_setCDKScale, "wrap_setCDKScale", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionLRChar", vc(wrap_setCDKSelectionLRChar, "wrap_setCDKSelectionLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKSlider", vc(wrap_injectCDKSlider, "wrap_injectCDKSlider", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSliderURChar", vc(wrap_setCDKSliderURChar, "wrap_setCDKSliderURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_Beep", vc(wrap_Beep, "wrap_Beep", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKEntry", vc(wrap_newCDKEntry, "wrap_newCDKEntry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryBoxAttribute", vc(wrap_setCDKEntryBoxAttribute, "wrap_setCDKEntryBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKAlphalist", vc(wrap_injectCDKAlphalist, "wrap_injectCDKAlphalist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKAlphalist", vc(wrap_drawCDKAlphalist, "wrap_drawCDKAlphalist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKAlphalist", vc(wrap_destroyCDKAlphalist, "wrap_destroyCDKAlphalist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKCalendarHighlight", vc(wrap_getCDKCalendarHighlight, "wrap_getCDKCalendarHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKDialog", vc(wrap_newCDKDialog, "wrap_newCDKDialog", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_drawCDKDialogButton", vc(wrap_drawCDKDialogButton, "wrap_drawCDKDialogButton", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKFselectFillerChar", vc(wrap_getCDKFselectFillerChar, "wrap_getCDKFselectFillerChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKFselectLinkAttribute", vc(wrap_getCDKFselectLinkAttribute, "wrap_getCDKFselectLinkAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogramHorizontalChar", vc(wrap_setCDKHistogramHorizontalChar, "wrap_setCDKHistogramHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKRadioHighlight", vc(wrap_getCDKRadioHighlight, "wrap_getCDKRadioHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionLLChar", vc(wrap_setCDKSelectionLLChar, "wrap_setCDKSelectionLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSlider", vc(wrap_setCDKSlider, "wrap_setCDKSlider", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSliderVerticalChar", vc(wrap_setCDKSliderVerticalChar, "wrap_setCDKSliderVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSliderBoxAttribute", vc(wrap_setCDKSliderBoxAttribute, "wrap_setCDKSliderBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSwindowContents", vc(wrap_setCDKSwindowContents, "wrap_setCDKSwindowContents", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSwindowURChar", vc(wrap_setCDKSwindowURChar, "wrap_setCDKSwindowURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplateCB", vc(wrap_setCDKTemplateCB, "wrap_setCDKTemplateCB", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKViewerLRChar", vc(wrap_setCDKViewerLRChar, "wrap_setCDKViewerLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_writeChtype", vc(wrap_writeChtype, "wrap_writeChtype", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKEntry", vc(wrap_moveCDKEntry, "wrap_moveCDKEntry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryCB", vc(wrap_setCDKEntryCB, "wrap_setCDKEntryCB", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScrollBox", vc(wrap_setCDKScrollBox, "wrap_setCDKScrollBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKCalendar", vc(wrap_positionCDKCalendar, "wrap_positionCDKCalendar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKDialogULChar", vc(wrap_setCDKDialogULChar, "wrap_setCDKDialogULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKLabel", vc(wrap_newCDKLabel, "wrap_newCDKLabel", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKGraphValue", vc(wrap_setCDKGraphValue, "wrap_setCDKGraphValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKItemlistCurrentItem", vc(wrap_getCDKItemlistCurrentItem, "wrap_getCDKItemlistCurrentItem", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKMatrix", vc(wrap_drawCDKMatrix, "wrap_drawCDKMatrix", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryBackgroundColor", vc(wrap_setCDKMentryBackgroundColor, "wrap_setCDKMentryBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKRadioChoiceCharacter", vc(wrap_getCDKRadioChoiceCharacter, "wrap_getCDKRadioChoiceCharacter", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKRadioLeftBrace", vc(wrap_getCDKRadioLeftBrace, "wrap_getCDKRadioLeftBrace", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioBoxAttribute", vc(wrap_setCDKRadioBoxAttribute, "wrap_setCDKRadioBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKRadio", vc(wrap_eraseCDKRadio, "wrap_eraseCDKRadio", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKScaleLowValue", vc(wrap_getCDKScaleLowValue, "wrap_getCDKScaleLowValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScaleVerticalChar", vc(wrap_setCDKScaleVerticalChar, "wrap_setCDKScaleVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKSelection", vc(wrap_positionCDKSelection, "wrap_positionCDKSelection", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionMode", vc(wrap_setCDKSelectionMode, "wrap_setCDKSelectionMode", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_trimCDKSwindow", vc(wrap_trimCDKSwindow, "wrap_trimCDKSwindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKTemplate", vc(wrap_injectCDKTemplate, "wrap_injectCDKTemplate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKTemplate", vc(wrap_drawCDKTemplate, "wrap_drawCDKTemplate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKViewer", vc(wrap_moveCDKViewer, "wrap_moveCDKViewer", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_searchList", vc(wrap_searchList, "wrap_searchList", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScrollLLChar", vc(wrap_setCDKScrollLLChar, "wrap_setCDKScrollLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKDialogHorizontalChar", vc(wrap_setCDKDialogHorizontalChar, "wrap_setCDKDialogHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKDialogPreProcess", vc(wrap_setCDKDialogPreProcess, "wrap_setCDKDialogPreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKLabel", vc(wrap_eraseCDKLabel, "wrap_eraseCDKLabel", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKFselect", vc(wrap_newCDKFselect, "wrap_newCDKFselect", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKGraphCharacters", vc(wrap_setCDKGraphCharacters, "wrap_setCDKGraphCharacters", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKHistogram", vc(wrap_positionCDKHistogram, "wrap_positionCDKHistogram", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKHistogram", vc(wrap_destroyCDKHistogram, "wrap_destroyCDKHistogram", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlistDefaultItem", vc(wrap_setCDKItemlistDefaultItem, "wrap_setCDKItemlistDefaultItem", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlistBackgroundColor", vc(wrap_setCDKItemlistBackgroundColor, "wrap_setCDKItemlistBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryMin", vc(wrap_setCDKMentryMin, "wrap_setCDKMentryMin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryURChar", vc(wrap_setCDKMentryURChar, "wrap_setCDKMentryURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioItems", vc(wrap_setCDKRadioItems, "wrap_setCDKRadioItems", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioRightBrace", vc(wrap_setCDKRadioRightBrace, "wrap_setCDKRadioRightBrace", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKScale", vc(wrap_newCDKScale, "wrap_newCDKScale", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScaleBox", vc(wrap_setCDKScaleBox, "wrap_setCDKScaleBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKSwindowContents", vc(wrap_getCDKSwindowContents, "wrap_getCDKSwindowContents", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSwindowHorizontalChar", vc(wrap_setCDKSwindowHorizontalChar, "wrap_setCDKSwindowHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_addCDKSwindow", vc(wrap_addCDKSwindow, "wrap_addCDKSwindow", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_splitString", vc(wrap_splitString, "wrap_splitString", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKEntry", vc(wrap_activateCDKEntry, "wrap_activateCDKEntry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKScrollBox", vc(wrap_getCDKScrollBox, "wrap_getCDKScrollBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalistURChar", vc(wrap_setCDKAlphalistURChar, "wrap_setCDKAlphalistURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKDialog", vc(wrap_moveCDKDialog, "wrap_moveCDKDialog", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKGraphValue", vc(wrap_getCDKGraphValue, "wrap_getCDKGraphValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogramVerticalChar", vc(wrap_setCDKHistogramVerticalChar, "wrap_setCDKHistogramVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKItemlist", vc(wrap_moveCDKItemlist, "wrap_moveCDKItemlist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMatrixPostProcess", vc(wrap_setCDKMatrixPostProcess, "wrap_setCDKMatrixPostProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentry", vc(wrap_setCDKMentry, "wrap_setCDKMentry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryFillerChar", vc(wrap_setCDKMentryFillerChar, "wrap_setCDKMentryFillerChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryBoxAttribute", vc(wrap_setCDKMentryBoxAttribute, "wrap_setCDKMentryBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKSelection", vc(wrap_eraseCDKSelection, "wrap_eraseCDKSelection", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelection", vc(wrap_setCDKSelection, "wrap_setCDKSelection", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKSelectionMode", vc(wrap_getCDKSelectionMode, "wrap_getCDKSelectionMode", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKSlider", vc(wrap_newCDKSlider, "wrap_newCDKSlider", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplate", vc(wrap_setCDKTemplate, "wrap_setCDKTemplate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKViewer", vc(wrap_activateCDKViewer, "wrap_activateCDKViewer", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKViewerHighlight", vc(wrap_setCDKViewerHighlight, "wrap_setCDKViewerHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_viewInfo", vc(wrap_viewInfo, "wrap_viewInfo", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_cleanChar", vc(wrap_cleanChar, "wrap_cleanChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setWidgetDimension", vc(wrap_setWidgetDimension, "wrap_setWidgetDimension", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryVerticalChar", vc(wrap_setCDKEntryVerticalChar, "wrap_setCDKEntryVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryPreProcess", vc(wrap_setCDKEntryPreProcess, "wrap_setCDKEntryPreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKScroll", vc(wrap_injectCDKScroll, "wrap_injectCDKScroll", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScrollItems", vc(wrap_setCDKScrollItems, "wrap_setCDKScrollItems", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKScroll", vc(wrap_drawCDKScroll, "wrap_drawCDKScroll", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalistHighlight", vc(wrap_setCDKAlphalistHighlight, "wrap_setCDKAlphalistHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalistULChar", vc(wrap_setCDKAlphalistULChar, "wrap_setCDKAlphalistULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_removeCDKCalendarMarker", vc(wrap_removeCDKCalendarMarker, "wrap_removeCDKCalendarMarker", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKCalendar", vc(wrap_eraseCDKCalendar, "wrap_eraseCDKCalendar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKLabel", vc(wrap_positionCDKLabel, "wrap_positionCDKLabel", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectBackgroundColor", vc(wrap_setCDKFselectBackgroundColor, "wrap_setCDKFselectBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKGraphCharacters", vc(wrap_getCDKGraphCharacters, "wrap_getCDKGraphCharacters", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogramBox", vc(wrap_setCDKHistogramBox, "wrap_setCDKHistogramBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKItemlistDefaultItem", vc(wrap_getCDKItemlistDefaultItem, "wrap_getCDKItemlistDefaultItem", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKMarquee", vc(wrap_destroyCDKMarquee, "wrap_destroyCDKMarquee", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKMentryMin", vc(wrap_getCDKMentryMin, "wrap_getCDKMentryMin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMenu", vc(wrap_setCDKMenu, "wrap_setCDKMenu", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKRadioItems", vc(wrap_getCDKRadioItems, "wrap_getCDKRadioItems", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKRadioRightBrace", vc(wrap_getCDKRadioRightBrace, "wrap_getCDKRadioRightBrace", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKScaleBox", vc(wrap_getCDKScaleBox, "wrap_getCDKScaleBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKSelectionHighlight", vc(wrap_getCDKSelectionHighlight, "wrap_getCDKSelectionHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKSliderLowValue", vc(wrap_getCDKSliderLowValue, "wrap_getCDKSliderLowValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSwindowLLChar", vc(wrap_setCDKSwindowLLChar, "wrap_setCDKSwindowLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplateLLChar", vc(wrap_setCDKTemplateLLChar, "wrap_setCDKTemplateLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplateLRChar", vc(wrap_setCDKTemplateLRChar, "wrap_setCDKTemplateLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_startCDKDebug", vc(wrap_startCDKDebug, "wrap_startCDKDebug", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_raiseCDKObject", vc(wrap_raiseCDKObject, "wrap_raiseCDKObject", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_swapIndex", vc(wrap_swapIndex, "wrap_swapIndex", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mode2Char", vc(wrap_mode2Char, "wrap_mode2Char", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryHorizontalChar", vc(wrap_setCDKEntryHorizontalChar, "wrap_setCDKEntryHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalistLRChar", vc(wrap_setCDKAlphalistLRChar, "wrap_setCDKAlphalistLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKAlphalist", vc(wrap_positionCDKAlphalist, "wrap_positionCDKAlphalist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKDialog", vc(wrap_activateCDKDialog, "wrap_activateCDKDialog", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_waitCDKLabel", vc(wrap_waitCDKLabel, "wrap_waitCDKLabel", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKItemlist", vc(wrap_activateCDKItemlist, "wrap_activateCDKItemlist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKMentryFillerChar", vc(wrap_getCDKMentryFillerChar, "wrap_getCDKMentryFillerChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryHiddenChar", vc(wrap_setCDKMentryHiddenChar, "wrap_setCDKMentryHiddenChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioPreProcess", vc(wrap_setCDKRadioPreProcess, "wrap_setCDKRadioPreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionBoxAttribute", vc(wrap_setCDKSelectionBoxAttribute, "wrap_setCDKSelectionBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionPreProcess", vc(wrap_setCDKSelectionPreProcess, "wrap_setCDKSelectionPreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKSliderHighValue", vc(wrap_getCDKSliderHighValue, "wrap_getCDKSliderHighValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSliderHorizontalChar", vc(wrap_setCDKSliderHorizontalChar, "wrap_setCDKSliderHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSliderPreProcess", vc(wrap_setCDKSliderPreProcess, "wrap_setCDKSliderPreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSwindowBox", vc(wrap_setCDKSwindowBox, "wrap_setCDKSwindowBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKViewerHighlight", vc(wrap_getCDKViewerHighlight, "wrap_getCDKViewerHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKViewerInfoLine", vc(wrap_setCDKViewerInfoLine, "wrap_setCDKViewerInfoLine", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKScreen", vc(wrap_eraseCDKScreen, "wrap_eraseCDKScreen", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_justifyString", vc(wrap_justifyString, "wrap_justifyString", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKEntry", vc(wrap_destroyCDKEntry, "wrap_destroyCDKEntry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKScrollItems", vc(wrap_getCDKScrollItems, "wrap_getCDKScrollItems", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScrollBackgroundColor", vc(wrap_setCDKScrollBackgroundColor, "wrap_setCDKScrollBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalistLLChar", vc(wrap_setCDKAlphalistLLChar, "wrap_setCDKAlphalistLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKDialog", vc(wrap_destroyCDKDialog, "wrap_destroyCDKDialog", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectDirectory", vc(wrap_setCDKFselectDirectory, "wrap_setCDKFselectDirectory", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectVerticalChar", vc(wrap_setCDKFselectVerticalChar, "wrap_setCDKFselectVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKFselect", vc(wrap_positionCDKFselect, "wrap_positionCDKFselect", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKHistogramBox", vc(wrap_getCDKHistogramBox, "wrap_getCDKHistogramBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlistVerticalChar", vc(wrap_setCDKItemlistVerticalChar, "wrap_setCDKItemlistVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKMarquee", vc(wrap_eraseCDKMarquee, "wrap_eraseCDKMarquee", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMatrixLLChar", vc(wrap_setCDKMatrixLLChar, "wrap_setCDKMatrixLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKMatrix", vc(wrap_positionCDKMatrix, "wrap_positionCDKMatrix", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKMentry", vc(wrap_newCDKMentry, "wrap_newCDKMentry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadio", vc(wrap_setCDKRadio, "wrap_setCDKRadio", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioHorizontalChar", vc(wrap_setCDKRadioHorizontalChar, "wrap_setCDKRadioHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKScaleHighValue", vc(wrap_getCDKScaleHighValue, "wrap_getCDKScaleHighValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScaleBackgroundColor", vc(wrap_setCDKScaleBackgroundColor, "wrap_setCDKScaleBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKSelection", vc(wrap_newCDKSelection, "wrap_newCDKSelection", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionItems", vc(wrap_setCDKSelectionItems, "wrap_setCDKSelectionItems", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionHorizontalChar", vc(wrap_setCDKSelectionHorizontalChar, "wrap_setCDKSelectionHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKSlider", vc(wrap_drawCDKSlider, "wrap_drawCDKSlider", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSwindowULChar", vc(wrap_setCDKSwindowULChar, "wrap_setCDKSwindowULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKTemplate", vc(wrap_newCDKTemplate, "wrap_newCDKTemplate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKTemplate", vc(wrap_positionCDKTemplate, "wrap_positionCDKTemplate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_stripWhiteSpace", vc(wrap_stripWhiteSpace, "wrap_stripWhiteSpace", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_copyChtype", vc(wrap_copyChtype, "wrap_copyChtype", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryMin", vc(wrap_setCDKEntryMin, "wrap_setCDKEntryMin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScrollURChar", vc(wrap_setCDKScrollURChar, "wrap_setCDKScrollURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarLLChar", vc(wrap_setCDKCalendarLLChar, "wrap_setCDKCalendarLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarLRChar", vc(wrap_setCDKCalendarLRChar, "wrap_setCDKCalendarLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarMarker", vc(wrap_setCDKCalendarMarker, "wrap_setCDKCalendarMarker", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKFselect", vc(wrap_injectCDKFselect, "wrap_injectCDKFselect", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKGraph", vc(wrap_moveCDKGraph, "wrap_moveCDKGraph", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMatrixCell", vc(wrap_setCDKMatrixCell, "wrap_setCDKMatrixCell", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKMentry", vc(wrap_injectCDKMentry, "wrap_injectCDKMentry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKMentryHiddenChar", vc(wrap_getCDKMentryHiddenChar, "wrap_getCDKMentryHiddenChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryVerticalChar", vc(wrap_setCDKMentryVerticalChar, "wrap_setCDKMentryVerticalChar", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_newCDKMenu", vc(wrap_newCDKMenu, "wrap_newCDKMenu", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKMenu", vc(wrap_eraseCDKMenu, "wrap_eraseCDKMenu", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKSwindowBox", vc(wrap_getCDKSwindowBox, "wrap_getCDKSwindowBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplateBoxAttribute", vc(wrap_setCDKTemplateBoxAttribute, "wrap_setCDKTemplateBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mixCDKTemplate", vc(wrap_mixCDKTemplate, "wrap_mixCDKTemplate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKViewerInfoLine", vc(wrap_getCDKViewerInfoLine, "wrap_getCDKViewerInfoLine", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_chlen", vc(wrap_chlen, "wrap_chlen", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKEntry", vc(wrap_eraseCDKEntry, "wrap_eraseCDKEntry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScroll", vc(wrap_setCDKScroll, "wrap_setCDKScroll", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScrollBoxAttribute", vc(wrap_setCDKScrollBoxAttribute, "wrap_setCDKScrollBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKButtonboxButtonCount", vc(wrap_getCDKButtonboxButtonCount, "wrap_getCDKButtonboxButtonCount", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKButtonboxURChar", vc(wrap_setCDKButtonboxURChar, "wrap_setCDKButtonboxURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarDayAttribute", vc(wrap_setCDKCalendarDayAttribute, "wrap_setCDKCalendarDayAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKCalendar", vc(wrap_destroyCDKCalendar, "wrap_destroyCDKCalendar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKFselectDirectory", vc(wrap_getCDKFselectDirectory, "wrap_getCDKFselectDirectory", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKGraphURChar", vc(wrap_setCDKGraphURChar, "wrap_setCDKGraphURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKHistogramHighValue", vc(wrap_getCDKHistogramHighValue, "wrap_getCDKHistogramHighValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogramBackgroundColor", vc(wrap_setCDKHistogramBackgroundColor, "wrap_setCDKHistogramBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlistURChar", vc(wrap_setCDKItemlistURChar, "wrap_setCDKItemlistURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKMarquee", vc(wrap_drawCDKMarquee, "wrap_drawCDKMarquee", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryHorizontalChar", vc(wrap_setCDKMentryHorizontalChar, "wrap_setCDKMentryHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryPreProcess", vc(wrap_setCDKMentryPreProcess, "wrap_setCDKMentryPreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKMenu", vc(wrap_drawCDKMenu, "wrap_drawCDKMenu", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKMenuSubwin", vc(wrap_eraseCDKMenuSubwin, "wrap_eraseCDKMenuSubwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKRadio", vc(wrap_injectCDKRadio, "wrap_injectCDKRadio", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKScale", vc(wrap_destroyCDKScale, "wrap_destroyCDKScale", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKSelectionItems", vc(wrap_getCDKSelectionItems, "wrap_getCDKSelectionItems", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSliderULChar", vc(wrap_setCDKSliderULChar, "wrap_setCDKSliderULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKSwindow", vc(wrap_moveCDKSwindow, "wrap_moveCDKSwindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplateHorizontalChar", vc(wrap_setCDKTemplateHorizontalChar, "wrap_setCDKTemplateHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKViewerBox", vc(wrap_setCDKViewerBox, "wrap_setCDKViewerBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_boxWindow", vc(wrap_boxWindow, "wrap_boxWindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_lowerCDKObject", vc(wrap_lowerCDKObject, "wrap_lowerCDKObject", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_endCDK", vc(wrap_endCDK, "wrap_endCDK", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_initCDKColor", vc(wrap_initCDKColor, "wrap_initCDKColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKEntryMin", vc(wrap_getCDKEntryMin, "wrap_getCDKEntryMin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryBox", vc(wrap_setCDKEntryBox, "wrap_setCDKEntryBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryBackgroundColor", vc(wrap_setCDKEntryBackgroundColor, "wrap_setCDKEntryBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKScroll", vc(wrap_eraseCDKScroll, "wrap_eraseCDKScroll", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalistContents", vc(wrap_setCDKAlphalistContents, "wrap_setCDKAlphalistContents", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKButtonboxULChar", vc(wrap_setCDKButtonboxULChar, "wrap_setCDKButtonboxULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKDialogLRChar", vc(wrap_setCDKDialogLRChar, "wrap_setCDKDialogLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKLabelLRChar", vc(wrap_setCDKLabelLRChar, "wrap_setCDKLabelLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_deleteFileCB", vc(wrap_deleteFileCB, "wrap_deleteFileCB", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKGraph", vc(wrap_activateCDKGraph, "wrap_activateCDKGraph", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKGraph", vc(wrap_setCDKGraph, "wrap_setCDKGraph", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKGraphULChar", vc(wrap_setCDKGraphULChar, "wrap_setCDKGraphULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKHistogramViewType", vc(wrap_getCDKHistogramViewType, "wrap_getCDKHistogramViewType", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogramFillerChar", vc(wrap_setCDKHistogramFillerChar, "wrap_setCDKHistogramFillerChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlist", vc(wrap_setCDKItemlist, "wrap_setCDKItemlist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKMentry", vc(wrap_drawCDKMentry, "wrap_drawCDKMentry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKRadio", vc(wrap_newCDKRadio, "wrap_newCDKRadio", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioVerticalChar", vc(wrap_setCDKRadioVerticalChar, "wrap_setCDKRadioVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKScale", vc(wrap_moveCDKScale, "wrap_moveCDKScale", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionVerticalChar", vc(wrap_setCDKSelectionVerticalChar, "wrap_setCDKSelectionVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplatePostProcess", vc(wrap_setCDKTemplatePostProcess, "wrap_setCDKTemplatePostProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKViewerLLChar", vc(wrap_setCDKViewerLLChar, "wrap_setCDKViewerLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKEntry", vc(wrap_injectCDKEntry, "wrap_injectCDKEntry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKAlphalistHighlight", vc(wrap_getCDKAlphalistHighlight, "wrap_getCDKAlphalistHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKButtonboxLRChar", vc(wrap_setCDKButtonboxLRChar, "wrap_setCDKButtonboxLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKCalendar", vc(wrap_injectCDKCalendar, "wrap_injectCDKCalendar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKCalendarDayAttribute", vc(wrap_getCDKCalendarDayAttribute, "wrap_getCDKCalendarDayAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarBoxAttribute", vc(wrap_setCDKCalendarBoxAttribute, "wrap_setCDKCalendarBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_eraseCDKDialog", vc(wrap_eraseCDKDialog, "wrap_eraseCDKDialog", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKDialogButtons", vc(wrap_drawCDKDialogButtons, "wrap_drawCDKDialogButtons", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKLabelLLChar", vc(wrap_setCDKLabelLLChar, "wrap_setCDKLabelLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectBoxAttribute", vc(wrap_setCDKFselectBoxAttribute, "wrap_setCDKFselectBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKGraphLRChar", vc(wrap_setCDKGraphLRChar, "wrap_setCDKGraphLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKItemlist", vc(wrap_injectCDKItemlist, "wrap_injectCDKItemlist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMarqueeLRChar", vc(wrap_setCDKMarqueeLRChar, "wrap_setCDKMarqueeLRChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioBox", vc(wrap_setCDKRadioBox, "wrap_setCDKRadioBox", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_setCDKSelectionChoices", vc(wrap_setCDKSelectionChoices, "wrap_setCDKSelectionChoices", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionBox", vc(wrap_setCDKSelectionBox, "wrap_setCDKSelectionBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKSwindow", vc(wrap_activateCDKSwindow, "wrap_activateCDKSwindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_jumpToLineCDKSwindow", vc(wrap_jumpToLineCDKSwindow, "wrap_jumpToLineCDKSwindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSwindow", vc(wrap_setCDKSwindow, "wrap_setCDKSwindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplatePreProcess", vc(wrap_setCDKTemplatePreProcess, "wrap_setCDKTemplatePreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKViewerBox", vc(wrap_getCDKViewerBox, "wrap_getCDKViewerBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_initCDKScreen", vc(wrap_initCDKScreen, "wrap_initCDKScreen", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_raiseCDKScreen", vc(wrap_raiseCDKScreen, "wrap_raiseCDKScreen", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_substring", vc(wrap_substring, "wrap_substring", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKEntryBox", vc(wrap_getCDKEntryBox, "wrap_getCDKEntryBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKScroll", vc(wrap_newCDKScroll, "wrap_newCDKScroll", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKAlphalistContents", vc(wrap_getCDKAlphalistContents, "wrap_getCDKAlphalistContents", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalistBoxAttribute", vc(wrap_setCDKAlphalistBoxAttribute, "wrap_setCDKAlphalistBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalistPreProcess", vc(wrap_setCDKAlphalistPreProcess, "wrap_setCDKAlphalistPreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKButtonboxLLChar", vc(wrap_setCDKButtonboxLLChar, "wrap_setCDKButtonboxLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKButtonbox", vc(wrap_moveCDKButtonbox, "wrap_moveCDKButtonbox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKCalendarHorizontalChar", vc(wrap_setCDKCalendarHorizontalChar, "wrap_setCDKCalendarHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKLabelURChar", vc(wrap_setCDKLabelURChar, "wrap_setCDKLabelURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKLabel", vc(wrap_destroyCDKLabel, "wrap_destroyCDKLabel", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKGraphLLChar", vc(wrap_setCDKGraphLLChar, "wrap_setCDKGraphLLChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogramValue", vc(wrap_setCDKHistogramValue, "wrap_setCDKHistogramValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKHistogramFillerChar", vc(wrap_getCDKHistogramFillerChar, "wrap_getCDKHistogramFillerChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogramStatsPos", vc(wrap_setCDKHistogramStatsPos, "wrap_setCDKHistogramStatsPos", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMatrixURChar", vc(wrap_setCDKMatrixURChar, "wrap_setCDKMatrixURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_jumpToCell", vc(wrap_jumpToCell, "wrap_jumpToCell", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMentryULChar", vc(wrap_setCDKMentryULChar, "wrap_setCDKMentryULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKRadio", vc(wrap_moveCDKRadio, "wrap_moveCDKRadio", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKScale", vc(wrap_activateCDKScale, "wrap_activateCDKScale", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScalePostProcess", vc(wrap_setCDKScalePostProcess, "wrap_setCDKScalePostProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKSlider", vc(wrap_positionCDKSlider, "wrap_positionCDKSlider", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSwindowBackgroundColor", vc(wrap_setCDKSwindowBackgroundColor, "wrap_setCDKSwindowBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKTemplateBox", vc(wrap_setCDKTemplateBox, "wrap_setCDKTemplateBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_unmixCDKTemplate", vc(wrap_unmixCDKTemplate, "wrap_unmixCDKTemplate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKViewer", vc(wrap_drawCDKViewer, "wrap_drawCDKViewer", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_quickSort", vc(wrap_quickSort, "wrap_quickSort", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dirName", vc(wrap_dirName, "wrap_dirName", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_cleanCDKEntry", vc(wrap_cleanCDKEntry, "wrap_cleanCDKEntry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScrollVerticalChar", vc(wrap_setCDKScrollVerticalChar, "wrap_setCDKScrollVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalistHorizontalChar", vc(wrap_setCDKAlphalistHorizontalChar, "wrap_setCDKAlphalistHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKDialogHighlight", vc(wrap_setCDKDialogHighlight, "wrap_setCDKDialogHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKLabelULChar", vc(wrap_setCDKLabelULChar, "wrap_setCDKLabelULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKFselect", vc(wrap_destroyCDKFselect, "wrap_destroyCDKFselect", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKGraph", vc(wrap_newCDKGraph, "wrap_newCDKGraph", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKGraphDisplayType", vc(wrap_setCDKGraphDisplayType, "wrap_setCDKGraphDisplayType", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKItemlist", vc(wrap_newCDKItemlist, "wrap_newCDKItemlist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKItemlistULChar", vc(wrap_setCDKItemlistULChar, "wrap_setCDKItemlistULChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMarqueeURChar", vc(wrap_setCDKMarqueeURChar, "wrap_setCDKMarqueeURChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKRadioBox", vc(wrap_getCDKRadioBox, "wrap_getCDKRadioBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKSelection", vc(wrap_destroyCDKSelection, "wrap_destroyCDKSelection", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_getCDKSelectionChoices", vc(wrap_getCDKSelectionChoices, "wrap_getCDKSelectionChoices", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKSelectionBox", vc(wrap_getCDKSelectionBox, "wrap_getCDKSelectionBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_bindCDKObject", vc(wrap_bindCDKObject, "wrap_bindCDKObject", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_freeChtype", vc(wrap_freeChtype, "wrap_freeChtype", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_chtype2Char", vc(wrap_chtype2Char, "wrap_chtype2Char", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScrollHorizontalChar", vc(wrap_setCDKScrollHorizontalChar, "wrap_setCDKScrollHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKScroll", vc(wrap_positionCDKScroll, "wrap_positionCDKScroll", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScrollPreProcess", vc(wrap_setCDKScrollPreProcess, "wrap_setCDKScrollPreProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKButtonbox", vc(wrap_activateCDKButtonbox, "wrap_activateCDKButtonbox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKButtonboxCurrentButton", vc(wrap_setCDKButtonboxCurrentButton, "wrap_setCDKButtonboxCurrentButton", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKButtonboxHighlight", vc(wrap_setCDKButtonboxHighlight, "wrap_setCDKButtonboxHighlight", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_drawCDKButtonboxButton", vc(wrap_drawCDKButtonboxButton, "wrap_drawCDKButtonboxButton", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKCalendar", vc(wrap_moveCDKCalendar, "wrap_moveCDKCalendar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKFselectHighlight", vc(wrap_setCDKFselectHighlight, "wrap_setCDKFselectHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKHistogramStatsPos", vc(wrap_getCDKHistogramStatsPos, "wrap_getCDKHistogramStatsPos", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKItemlist", vc(wrap_drawCDKItemlist, "wrap_drawCDKItemlist", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKMarquee", vc(wrap_positionCDKMarquee, "wrap_positionCDKMarquee", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKMatrixCell", vc(wrap_getCDKMatrixCell, "wrap_getCDKMatrixCell", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKMenu", vc(wrap_destroyCDKMenu, "wrap_destroyCDKMenu", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKRadio", vc(wrap_activateCDKRadio, "wrap_activateCDKRadio", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScaleValue", vc(wrap_setCDKScaleValue, "wrap_setCDKScaleValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKSelection", vc(wrap_moveCDKSelection, "wrap_moveCDKSelection", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_destroyCDKSlider", vc(wrap_destroyCDKSlider, "wrap_destroyCDKSlider", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newCDKSwindow", vc(wrap_newCDKSwindow, "wrap_newCDKSwindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSwindowVerticalChar", vc(wrap_setCDKSwindowVerticalChar, "wrap_setCDKSwindowVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKTemplateBox", vc(wrap_getCDKTemplateBox, "wrap_getCDKTemplateBox", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKViewerBackgroundColor", vc(wrap_setCDKViewerBackgroundColor, "wrap_setCDKViewerBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_writeChar", vc(wrap_writeChar, "wrap_writeChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getString", vc(wrap_getString, "wrap_getString", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryFillerChar", vc(wrap_setCDKEntryFillerChar, "wrap_setCDKEntryFillerChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKEntryHiddenChar", vc(wrap_setCDKEntryHiddenChar, "wrap_setCDKEntryHiddenChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKScrollPosition", vc(wrap_setCDKScrollPosition, "wrap_setCDKScrollPosition", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_addCDKScrollItem", vc(wrap_addCDKScrollItem, "wrap_addCDKScrollItem", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_redrawCDKButtonboxButtonboxs", vc(wrap_redrawCDKButtonboxButtonboxs, "wrap_redrawCDKButtonboxButtonboxs", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKDialogHighlight", vc(wrap_getCDKDialogHighlight, "wrap_getCDKDialogHighlight", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_setCDKGraphValues", vc(wrap_setCDKGraphValues, "wrap_setCDKGraphValues", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKGraphDisplayType", vc(wrap_getCDKGraphDisplayType, "wrap_getCDKGraphDisplayType", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKHistogramStatsAttr", vc(wrap_setCDKHistogramStatsAttr, "wrap_setCDKHistogramStatsAttr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_moveCDKHistogram", vc(wrap_moveCDKHistogram, "wrap_moveCDKHistogram", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMarqueeHorizontalChar", vc(wrap_setCDKMarqueeHorizontalChar, "wrap_setCDKMarqueeHorizontalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKMatrixCol", vc(wrap_getCDKMatrixCol, "wrap_getCDKMatrixCol", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKMatrixBackgroundColor", vc(wrap_setCDKMatrixBackgroundColor, "wrap_setCDKMatrixBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_positionCDKMentry", vc(wrap_positionCDKMentry, "wrap_positionCDKMentry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKRadioBackgroundColor", vc(wrap_setCDKRadioBackgroundColor, "wrap_setCDKRadioBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionTitle", vc(wrap_setCDKSelectionTitle, "wrap_setCDKSelectionTitle", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKSelectionBackgroundColor", vc(wrap_setCDKSelectionBackgroundColor, "wrap_setCDKSelectionBackgroundColor", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dumpCDKSwindow", vc(wrap_dumpCDKSwindow, "wrap_dumpCDKSwindow", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKViewerURChar", vc(wrap_setCDKViewerURChar, "wrap_setCDKViewerURChar", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_lowerCDKScreen", vc(wrap_lowerCDKScreen, "wrap_lowerCDKScreen", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_refreshCDKScreen", vc(wrap_refreshCDKScreen, "wrap_refreshCDKScreen", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_intlen", vc(wrap_intlen, "wrap_intlen", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_drawCDKEntry", vc(wrap_drawCDKEntry, "wrap_drawCDKEntry", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKAlphalistVerticalChar", vc(wrap_setCDKAlphalistVerticalChar, "wrap_setCDKAlphalistVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKButtonboxCurrentButton", vc(wrap_getCDKButtonboxCurrentButton, "wrap_getCDKButtonboxCurrentButton", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKButtonboxHighlight", vc(wrap_getCDKButtonboxHighlight, "wrap_getCDKButtonboxHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKCalendar", vc(wrap_activateCDKCalendar, "wrap_activateCDKCalendar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKDialogPostProcess", vc(wrap_setCDKDialogPostProcess, "wrap_setCDKDialogPostProcess", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKLabelBoxAttribute", vc(wrap_setCDKLabelBoxAttribute, "wrap_setCDKLabelBoxAttribute", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKFselectHighlight", vc(wrap_getCDKFselectHighlight, "wrap_getCDKFselectHighlight", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_deactivateCDKMarquee", vc(wrap_deactivateCDKMarquee, "wrap_deactivateCDKMarquee", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getCDKScaleValue", vc(wrap_getCDKScaleValue, "wrap_getCDKScaleValue", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_activateCDKSelection", vc(wrap_activateCDKSelection, "wrap_activateCDKSelection", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_injectCDKSelection", vc(wrap_injectCDKSelection, "wrap_injectCDKSelection", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_setCDKSelectionModes", vc(wrap_setCDKSelectionModes, "wrap_setCDKSelectionModes", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKViewer", vc(wrap_setCDKViewer, "wrap_setCDKViewer", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKViewerInfo", vc(wrap_setCDKViewerInfo, "wrap_setCDKViewerInfo", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKViewerVerticalChar", vc(wrap_setCDKViewerVerticalChar, "wrap_setCDKViewerVerticalChar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setCDKViewerBoxAttribute", vc(wrap_setCDKViewerBoxAttribute, "wrap_setCDKViewerBoxAttribute", VC_FUNC_BUILTIN_LEAF));

}
