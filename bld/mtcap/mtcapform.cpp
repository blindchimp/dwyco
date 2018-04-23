//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "vgexp.h"
#include "mtcapform.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "VLCommonFilter"
#pragma link "VLDSCapture"
#pragma link "VLGenericFilter"
#pragma link "VLDSVideoLogger"
//#pragma link "VLChangeFormat"
//#pragma link "VLResize"
#pragma link "LPComponent"
#pragma link "SLCommonFilter"
#pragma link "VLBasicGenericFilter"
#pragma link "VLBasicGenericFilter"
//#pragma link "VLChangeRate"
#pragma resource "*.dfm"
TCapForm *CapForm;
//---------------------------------------------------------------------------
__fastcall TCapForm::TCapForm(TComponent* Owner)
    : TForm(Owner)
{
#if 0
	TStringList *a = new TStringList;
	VLDSCapture1->VideoCaptureDevice->GetDeviceList(a);
	mtVideoCaptureDevice->Items = a;
    mtVideoCaptureDevice->ItemIndex = 0;
#endif

}
//---------------------------------------------------------------------------


/*
typedef void DWYCOCALLCONV (*DwycoVVCallback)(void *);
typedef int DWYCOCALLCONV (*DwycoIVCallback)(void *);
typedef void * DWYCOCALLCONV (*DwycoVidGetDataCallback)(void *,
	int *r, int *c,
	void **y, void **cr, void **cb, int *fmt, unsigned long *captime);
DWYCOEXPORT void dwyco_set_external_video_capture_callbacks(
	DwycoVVCallback nw,
	DwycoVVCallback del,
	DwycoIVCallback init,
	DwycoIVCallback has_data,
	DwycoVVCallback need,
	DwycoVVCallback pass,
	DwycoVVCallback stop,
	DwycoVidGetDataCallback get_data
);
*/
#include "dwvecp.h"
#include "dwstr.h"
#include "vidaq.h"

DwVecP<unsigned char> bufs(4, DWVEC_FIXED, !DWVEC_AUTO_EXPAND);
DwVec<unsigned long> y_bufs(4, DWVEC_FIXED, !DWVEC_AUTO_EXPAND);
DwVec<int> bwidth(4, DWVEC_FIXED, !DWVEC_AUTO_EXPAND);
DwVec<int> bheight(4, DWVEC_FIXED, !DWVEC_AUTO_EXPAND);
DwVec<int> bbytes(4, DWVEC_FIXED, !DWVEC_AUTO_EXPAND);
int next_ibuf;
int next_buf;
TApplication *app;
static CRITICAL_SECTION *cs;

void
oopanic(char *a)
{
    ::abort();
}

void
oopanic(const char *a)
{
    ::abort();
}

#define NVIDSIZES 4
static int Sizes[NVIDSIZES][2] = {
{320, 240},
{160, 120},
{640, 480},
{1280, 720}
};

static TStringList *Devnames;
char **
DWYCOEXPORT
vgget_video_devices()
{

	if(!CapForm)
	{
		CapForm = new TCapForm(0);
    }

	TStringList *a = new TStringList;

	CapForm->VLDSCapture1->VideoCaptureDevice->GetDeviceList(a);

	int n = a->Count;

	char **r = new char *[NVIDSIZES * n + 1];
	char tmp[50]; // note: itoa says it will only return 33 chars
    for(int i = 0; i < n; ++i)
	{
		DwString b("Large (~320x240) Camera #");
		b += itoa(i + 1, tmp, 10);
		r[i * NVIDSIZES] = new char [b.length() + 1];
		strcpy(r[i * NVIDSIZES], b.c_str());
		r[i * NVIDSIZES][b.length()] = 0;

		b = "Small (~160x120) Camera #";
		b += itoa(i + 1, tmp, 10);
		r[i * NVIDSIZES + 1] = new char [b.length() + 1];
		strcpy(r[i * NVIDSIZES + 1], b.c_str());
		r[i * NVIDSIZES + 1][b.length()] = 0;

		b = "Huge (~640x480) Camera #";
		b += itoa(i + 1, tmp, 10);
		r[i * NVIDSIZES + 2] = new char [b.length() + 1];
		strcpy(r[i * NVIDSIZES + 2], b.c_str());
		r[i * NVIDSIZES + 2][b.length()] = 0;

		b = "HD 720 (~1280x720) Camera #";
		b += itoa(i + 1, tmp, 10);
		r[i * NVIDSIZES + 3] = new char [b.length() + 1];
		strcpy(r[i * NVIDSIZES + 3], b.c_str());
		r[i * NVIDSIZES + 3][b.length()] = 0;

	}
	r[NVIDSIZES * n] = 0;

	if(Devnames)
    	delete Devnames;
	Devnames = a;
	return r;
}

void
DWYCOEXPORT
vgfree_video_devices(char **d)
{
	char **tmp = d;
    while(*d)
    {
        delete [] *d;
        ++d;
    }
    delete [] tmp;
}

void
DWYCOEXPORT
vgset_video_device(int idx)
{
	if(!CapForm)
	{
		CapForm = new TCapForm(0);
	}
	if(!Devnames)
		return;

	if(idx / NVIDSIZES >= Devnames->Count)
		return;
	AnsiString dname;
	dname = Devnames->Strings[idx / NVIDSIZES];
	CapForm->VLDSCapture1->VideoCaptureDevice->DeviceName = dname;
	CapForm->VLDSCapture1->FrameRate->Current = 0;
	switch(idx % NVIDSIZES)
	{
	case 0:
	default:
		CapForm->VLDSCapture1->FrameRate->Rate = 10;
		break;
	case 1:
		CapForm->VLDSCapture1->FrameRate->Rate = 30;
		break;
	case 2:
	case 3:
		CapForm->VLDSCapture1->FrameRate->Rate = 5;
		break;
	}

	TVLVideoModes *m = CapForm->VLDSCapture1->VideoModes;
	int bestmatch = -1;
	int besthdiff = (1 << 30);
	// note: this size it apparently the physical capture size
	// provided by the device. and the docs have some dire warnings
	// about setting the size to something that isn't supported directly
	// by the hardware. anyway, we try to get the physical size as close
	// as possible, but the final size is determined by the "resize"
	// component. also, we have a "format change" component that changes
	// the format to rgb24 later in the pipe, so the format here doesn't
	// matter.
	for(int i = 0; i < m->Count; ++i)
	{
		TVLVideoModeFormat *f = m->Items[i];
		int hdiff = abs(f->Height - Sizes[idx % NVIDSIZES][1]) + abs(f->Width - Sizes[idx % NVIDSIZES][0]);
		if(hdiff < besthdiff)
		{
			bestmatch = i;
			besthdiff = hdiff;
		}
		if(hdiff == 0)
			break;
	}
	if(bestmatch == -1)
		return;
	CapForm->VLDSCapture1->VideoSize->Current = 0;
	CapForm->VLDSCapture1->VideoSize->Width = m->Items[bestmatch]->Width;
	CapForm->VLDSCapture1->VideoSize->Height = m->Items[bestmatch]->Height;
#if 0
	CapForm->VLResize1->Width = Sizes[idx % NVIDSIZES][0];
	CapForm->VLResize1->Height = Sizes[idx % NVIDSIZES][1];
#endif
}

void
DWYCOEXPORT
vgstop_video_device()
{
	if(CapForm)
	{
		CapForm->VLDSCapture1->Stop();
		CapForm->VLDSCapture1->Close();
		CapForm->VLDSCapture1->Enabled = 0;
		delete CapForm;
		CapForm = 0;
		vgpass(0);
	}
}

void
DWYCOEXPORT
vgshow_source_dialog()
{
	if(!CapForm)
		return;
	CapForm->VLDSCapture1->ShowVideoDialog(cdVideoSource);
}

void
DWYCOEXPORT
vgpreview_on(void *display_window)
{
	if(!CapForm)
		return;
#if 0
    HWND h = (HWND)display_window;
    TForm *f = new TForm(h);
    TPoint p = f->ClientOrigin;
    RECT r;
	::GetWindowRect(h, &r);

	delete f;
#endif
	CapForm->VLDSCapture1->VideoPreview->Enabled = 1;
	CapForm->VLDSCapture1->VideoPreview->Visible = 1;
}

void
DWYCOEXPORT
vgpreview_off()
{
	if(!CapForm)
		return;
	CapForm->VLDSCapture1->VideoPreview->Visible = 0;
	CapForm->VLDSCapture1->VideoPreview->Enabled = 0;
}


void DWYCOEXPORT
vg_set_appdata(void *u1)
{
	app = (TApplication *)u1;
	if(CapForm == 0)
    {
		CapForm = new TCapForm(0);
	}
	if(cs == 0)
	{
		cs = new CRITICAL_SECTION;
		InitializeCriticalSection(cs);
    }
}

void
DWYCOEXPORT
vgnew(void *aqext)
{
	if(!CapForm)
	{
		CapForm = new TCapForm(0);
	}
	if(cs == 0)
	{
		cs = new CRITICAL_SECTION;
		InitializeCriticalSection(cs);
    }
}

void
DWYCOEXPORT
vgdel(void *aqext)
{
	if(CapForm)
	{
		CapForm->VLDSCapture1->Stop();
		CapForm->VLDSCapture1->Close();
		CapForm->VLDSCapture1->Enabled = 0;
		delete CapForm;
		CapForm = 0;
    }
	vgpass(aqext); // delete all outstanding bufs

}

int
DWYCOEXPORT
vginit(void *aqext, int frame_rate)
{
	CapForm->VLDSCapture1->Enabled = 1;
#if 0
	CapForm->VLDSCapture1->FrameRate->Current = 0;
	CapForm->VLDSCapture1->FrameRate->Rate =  frame_rate;
#endif
	CapForm->VLDSCapture1->Open();
	CapForm->VLDSCapture1->Start();
	return 1;
}

int
DWYCOEXPORT
vghas_data(void *aqext)
{
	EnterCriticalSection(cs);
    if(next_buf == next_ibuf)
	{
		LeaveCriticalSection(cs);
		return 0;
	}
	LeaveCriticalSection(cs);
	return 1;
}

void
DWYCOEXPORT
vgneed(void *aqext)
{
	if(!CapForm)
		return;
	if(!CapForm->VLDSCapture1->Enabled)
	{
    	CapForm->VLDSCapture1->Enabled = 1;
		//CapForm->VLDSCapture1->FrameRate->Current = 0;
		//CapForm->VLDSCapture1->FrameRate->Rate =  frame_rate;
		CapForm->VLDSCapture1->Open();
		CapForm->VLDSCapture1->Start();
    }
}

void
DWYCOEXPORT
vgpass(void *aqext)
{
	EnterCriticalSection(cs);
    while(next_buf != next_ibuf)
	{
		delete [] bufs[next_buf];
		bufs[next_buf] = 0;
		//GRTLOG("chuck %d", (int)y_bufs[next_buf], 0);
		next_buf = (next_buf + 1) % bufs.num_elems();
	}
	LeaveCriticalSection(cs);
}

void
DWYCOEXPORT
vgstop(void *aqext)
{
	if(CapForm)
    	CapForm->VLDSCapture1->Stop();
}

void *
DWYCOEXPORT
vgget_data(
void *aqext,
	int *c_out, int *r_out,
	int *bytes, int *fmt_out, unsigned long *captime_out)
{
	EnterCriticalSection(cs);
	if(next_buf != next_ibuf)
	{
        int nb = next_buf;
        unsigned char *bm = bufs[nb];
        int cols, rows;
        *r_out = rows = bheight[nb];
        *c_out = cols = bwidth[nb];
        *captime_out = y_bufs[nb];
        *fmt_out = (AQ_COLOR|AQ_RGB24);
        *bytes = bbytes[nb];

		next_buf = (next_buf + 1) % bufs.num_elems();
		//GRTLOG("conv %d", (int)y_bufs[nb], 0);
		void *ret = bufs[nb];
		//GRTLOG("conv done", 0, 0);
        // note: callers responsibility to call vgfree_data
        // when it is done with the buffer.
		bufs[nb] = 0;
		bwidth[nb] = bheight[nb] = 0;
		LeaveCriticalSection(cs);
		return ret;
	}
	//oopanic("aqvfw get no data");
	// not reached
	LeaveCriticalSection(cs);
    return 0;
}

void
DWYCOEXPORT
vgfree_data(void *data)
{
    delete [] data;
}

#if 0
void
init_ext_vid()
{
dwyco_set_external_video_capture_callbacks(
    vgnew,
    vgdel,
    vginit,
    vghas_data,
    vgneed,
    vgpass,
    vgstop,
    vgget_data
    );
}
#endif
void __fastcall TCapForm::VLGenericFilter1ProcessData(TObject *Sender,
	  TVLCVideoBuffer InBuffer, TVLCVideoBuffer &OutBuffer,
      bool &SendOutputData)
{
	SendOutputData = 0;
	EnterCriticalSection(cs);
	if(next_buf == (next_ibuf + 1) % bufs.num_elems())
	{
		LeaveCriticalSection(cs);
		return; // drop frame
	}

	TVLCVideoBuffer& InData = InBuffer;

    bufs[next_ibuf] = new unsigned char[InData.GetWidth() * InData.GetHeight() * 3];
    unsigned char *tmp = bufs[next_ibuf];
    bwidth[next_ibuf] = InData.GetWidth();
    bheight[next_ibuf] = InData.GetHeight();


    for(int y = 0; y < InData.GetHeight(); ++y)
        for(int x = 0; x < InData.GetWidth(); ++x)
        {
			tmp[2] = InData.Red[x][y];
            tmp[1] = InData.Green[x][y];
            tmp[0] = InData.Blue[x][y];
            tmp += 3;
        }

	y_bufs[next_ibuf] = timeGetTime(); // turn into msecs
    bbytes[next_ibuf] = InData.GetWidth() * InData.GetHeight() * 3;
	next_ibuf = (next_ibuf + 1) % bufs.num_elems();
	LeaveCriticalSection(cs);
}
//---------------------------------------------------------------------------

void __fastcall TCapForm::mtVideoCaptureDeviceChange(TObject *Sender)
{
	VLDSCapture1->VideoCaptureDevice->DeviceName = mtVideoCaptureDevice->Items->Strings[mtVideoCaptureDevice->ItemIndex];
}
//---------------------------------------------------------------------------

