
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/pgmgv.cc 1.5 1997/11/25 20:41:03 dwight Stable095 $
 */
#include "pgmgv.h"
#undef min
#undef max
#include "grayview.h"

static unsigned short Graytab[256];
static void
gen_table()
{
    for(unsigned int i = 0; i < 256; ++i)
    {
        unsigned int j = i >> 3;
        Graytab[i] = (j << 10) | (j << 5) | j;
    }
}

void
pgm_to_grayview(gray **img, int cols, int rows, grayview *gv, int fit, int integral_zoom)
{
    TClientDC tdc(*gv);
    TMemoryDC mc(tdc);
#ifdef USE_DRAWDIB
    // create a dummy format
    static struct foo {
        BITMAPINFOHEADER bmih;
        RGBQUAD pal[256];
    } dd_format;
    static int been_here;
    if(!been_here || dd_format.bmih.biWidth != cols ||
            dd_format.bmih.biHeight != rows)
    {
        been_here = 1;
        dd_format.bmih.biSize = sizeof(dd_format.bmih);
        dd_format.bmih.biWidth = cols;
        dd_format.bmih.biHeight = rows;
        dd_format.bmih.biPlanes = 1;
        dd_format.bmih.biBitCount = 8;
        dd_format.bmih.biCompression = BI_RGB;
        dd_format.bmih.biSizeImage = cols * rows;
        dd_format.bmih.biXPelsPerMeter = 0;
        dd_format.bmih.biYPelsPerMeter = 0;
        dd_format.bmih.biClrUsed = 0;
        dd_format.bmih.biClrImportant = 0;
        for(int i = 0; i < 256; ++i)
        {
            dd_format.pal[i].rgbBlue =
                dd_format.pal[i].rgbGreen =
                    dd_format.pal[i].rgbRed = i;
        }
    }
    TRect gvrect(gv->GetClientRect());
    // keep aspect ratio
    TSize s = gvrect.Size();
    s.cy = (((long)s.cx * (long)rows) / (long)cols);
    if(integral_zoom)
    {
        if(s.cx < cols)
            s.cx = cols;
        if(s.cy < rows)
            s.cy = rows;
        s.cx = cols * (s.cx / cols);
        s.cy = rows * (s.cy / rows);
        s.cy = (((long)s.cx * (long)rows) / (long)cols);
    }
    DrawDibDraw(gv->hdd, tdc, 0, 0, fit ? s.cx : -1, fit ? s.cy : -1, &dd_format.bmih, &img[0][0],
                0, 0, cols, rows, 0);
#else

    int rcaps = tdc.GetDeviceCaps(RASTERCAPS);
    int is_palette = (rcaps & RC_PALETTE) != 0;
    if((rcaps & RC_STRETCHBLT) == 0)
        fit = 0;
    TRect gvrect(gv->GetClientRect());
    // keep aspect ratio
    TSize s = gvrect.Size();
    s.cy = (((long)s.cx * (long)rows) / (long)cols);
    if(integral_zoom)
    {
        if(s.cx < cols)
            s.cx = cols;
        if(s.cy < rows)
            s.cy = rows;
        s.cx = cols * (s.cx / cols);
        s.cy = rows * (s.cy / rows);
        s.cy = (((long)s.cx * (long)rows) / (long)cols);
    }
    TRect newrect(TPoint(0, 0), s);
    if(is_palette)
    {
        tdc.SelectObject(*gv->pal);
        tdc.RealizePalette();
        TBitmap bm(tdc, cols, rows);
        mc.SelectObject(bm);
        bm.SetBitmapBits(cols * rows, &img[0][0]);
        if(!fit)
            tdc.BitBlt(0, 0, cols, rows, mc, 0, 0, SRCCOPY);
        else
        {
            tdc.SetStretchBltMode(COLORONCOLOR);
            tdc.StretchBlt(newrect, mc, TRect(0, 0, cols, rows));
        }
    }
    else
    {
        if(tdc.GetDeviceCaps(BITSPIXEL) <= 16)
        {
            // depending on format, convert gray into
            // proper bitmap
            static int been_here;
            if(!been_here)
            {
                gen_table();
                been_here = 1;
            }
            TBitmap bm(tdc, cols, rows);
            mc.SelectObject(bm);
            unsigned short *s = new unsigned short[rows * cols];
            gray *g = &img[0][0];
            long num = rows * cols;
            long inum = num & ~0x3;
            for(long i = 0; i < inum; i += 4)
            {
                s[i] = Graytab[g[i]];
                s[i + 1] = Graytab[g[i + 1]];
                s[i + 2] = Graytab[g[i + 2]];
                s[i + 3] = Graytab[g[i + 3]];
            }
            for(i = inum; i < num; ++i)
                s[i] = Graytab[g[i]];

            bm.SetBitmapBits(cols * rows * sizeof(*s), s);
            if(!fit)
                tdc.BitBlt(0, 0, cols, rows, mc, 0, 0, SRCCOPY);
            else
                tdc.StretchBlt(newrect, mc, TRect(0, 0, cols, rows));
            delete [] s;
        }
        else // 24 bit device
        {
            struct rgb {
                unsigned char r;
                unsigned char g;
                unsigned char b;
            };
            TBitmap bm(tdc, cols, rows);
            mc.SelectObject(bm);
            rgb *s = new rgb[rows * cols];
            gray *g = &img[0][0];
            long num = rows * cols;
            long inum = num & ~0x3;
            for(long i = 0; i < inum; i += 4)
            {
                gray t = g[i];
                s[i].r = t;
                s[i].g = t;
                s[i].b = t;
                t = g[i + 1];
                s[i + 1].r = t;
                s[i + 1].g = t;
                s[i + 1].b = t;
                t = g[i + 2];
                s[i + 2].r = t;
                s[i + 2].g = t;
                s[i + 2].b = t;
                t = g[i + 3];
                s[i + 3].r = t;
                s[i + 3].g = t;
                s[i + 3].b = t;
            }
            for(i = inum; i < num; ++i)
            {
                gray t = g[i];
                s[i].r = t;
                s[i].g = t;
                s[i].b = t;
            }
            bm.SetBitmapBits(cols * rows * sizeof(*s), s);
            if(!fit)
                tdc.BitBlt(0, 0, cols, rows, mc, 0, 0, SRCCOPY);
            else
                tdc.StretchBlt(newrect, mc, TRect(0, 0, cols, rows));
            delete [] s;
        }
    }
#endif
}
