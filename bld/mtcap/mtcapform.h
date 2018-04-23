//---------------------------------------------------------------------------

#ifndef mtcapformH
#define mtcapformH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "VLCommonFilter.h"
#include "VLDSCapture.h"
#include "VLGenericFilter.h"
#include "VLDSVideoLogger.h"
//#include "VLChangeFormat.h"
//#include "VLResize.h"
#include "LPComponent.h"

#include "SLCommonFilter.h"
#include "VLBasicGenericFilter.h"
//#include "VLChangeRate.h"
#include "VLBasicGenericFilter.h"
//---------------------------------------------------------------------------
class TCapForm : public TForm
{
__published:	// IDE-managed Components
    TVLDSCapture *VLDSCapture1;
    TVLGenericFilter *VLGenericFilter1;
    TComboBox *mtVideoCaptureDevice;
    void __fastcall VLGenericFilter1ProcessData(TObject *Sender,
          TVLCVideoBuffer InBuffer, TVLCVideoBuffer &OutBuffer,
          bool &SendOutputData);
    void __fastcall mtVideoCaptureDeviceChange(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TCapForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TCapForm *CapForm;
//---------------------------------------------------------------------------
#endif
