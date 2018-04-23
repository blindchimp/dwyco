
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/acqfile.h 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef ACQFILE_H
#define ACQFILE_H

#include <windows.h>
#ifdef LINUX
#include <unistd.h>
#else
#include <io.h>
#endif
#include "dwvec.h"
#include "dwvecp.h"
#include "vidaq.h"
#include "pbmcfg.h"
#include "imgmisc.h"
#include "jccolor.h"

// acquire data from PBM files. init must be called with
// a file that contains a list of filenames that will
// be returned as if they were comming from an acquisition
// device.

gray **readfile(gray *, FILE *f, int *cols, int *rows, gray *maxval);
pixel **readfile(pixel *, FILE *f, int *cols, int *rows, gray *maxval);

template<class T>
class FileAcquire : public VidAcquire
{
public:

    FileAcquire();
    ~FileAcquire();
    int init(const char *file_list, int pattern = 0, int preload_all = 0);
    void need();
    void pass();
    int has_data();
    void *get_data(int& cols, int& rows, void*& = VidAcquire::dummy,
                   void*& = VidAcquire::dummy, void*&  = VidAcquire::dummy, int& out = VidAcquire::dummy2,
                   unsigned long& = VidAcquire::dummy3, int no_convert = 0
                  );

private:
    DwVecP<char> filenames;
    DwVec<T **> imgs;
    int current;
    int filenum;
    int cols;
    int rows;
    gray maxval;
    int preloaded;
    double fake_time;
    void bump(int&);
};

template<class T>
FileAcquire<T>::FileAcquire()
{
    preloaded = 0;
    current = 0;
    filenum = 0;
    rows = 0;
    cols = 0;
    preloaded = 0;
    maxval = 0;
    fake_time = GetTickCount();
}

template<class T>
FileAcquire<T>::~FileAcquire()
{
    int i;
    for(i = 0; i < filenames.num_elems(); ++i)
        free(filenames[i]);
    for(i = 0; i < imgs.num_elems(); ++i)
        if(imgs[i])
            pm_freearray((char **)imgs[i], rows);
}

template<class T>
int
FileAcquire<T>::init(const char *file, int pattern, int preload)
{
    int i;
    if(init_ok())
        return 0;
    FILE *f;
    if(!pattern)
    {
        if((f = fopen(file, "rt")) == 0)
        {
            set_fail_reason("can't open list of files");
            return 0;
        }
        char line[1024];
        while(fgets(line, sizeof(line), f) != 0)
        {
            int l = strlen(line);
            if(line[l - 1] == '\n') line[l - 1] = '\0';
            filenames.append(strdup(line));
        }
        fclose(f);
    }
    else
    {
        int i;
        for(i = 0; ; ++i)
        {
            char s[1024];
            sprintf(s, file, i);
            if(access(s, 0) == 0)
                filenames.append(strdup(s));
            else
                break;
        }

    }
    if(filenames.num_elems() == 0)
    {
        char a[1024];
        sprintf(a, "no files to read? check your file specs");
        set_fail_reason(a);
        return 0;
    }

    preloaded = 0;
    if(preload)
    {
        for(i = 0; i < filenames.num_elems(); ++i)
        {
            if((f = fopen(filenames[i], "rb")) == 0)
            {
                char a[1024];
                sprintf(a, "can't open file %s", filenames[i]);
                set_fail_reason(a);
                for(int j = i - 1; j >= 0; ++j)
                    pm_freearray((char **)imgs[j], rows);
                return 0;
            }
            T dummy;
            T **img = readfile(&dummy, f, &cols, &rows, &maxval);
            flip_in_place(img, cols, rows);
            imgs.append(img);
            fclose(f);
        }
        preloaded = 1;
    }
    else
    {
        for(i = 0; i < filenames.num_elems(); ++i)
        {
            // just check to make sure file is there...
            if(access(filenames[i], 0) < 0)
            {
                char a[1024];
                sprintf(a, "can't open file %s", filenames[i]);
                set_fail_reason(a);
                return 0;
            }
            imgs[i] = 0;
        }
    }

    current = 0;
    inited = 1;
    return 1;
}

template<class T>
int
FileAcquire<T>::has_data()
{
    return imgs[current] != 0 && GetTickCount() > fake_time;
}

template<class T>
void
FileAcquire<T>::need()
{
    if(imgs[current] != 0)
        return;
    FILE *f;
    if((f = fopen(filenames[filenum], "rb")) == 0)
    {
        return;
    }
    T dummy;
    imgs[current] = readfile(&dummy, f, &cols, &rows, &maxval);
    flip_in_place(imgs[current], cols, rows);
    fclose(f);
}

template<class T>
void
FileAcquire<T>::pass()
{
}

template<class T>
void *
FileAcquire<T>::get_data(int& c, int& r, void*& y, void*& cb, void*& cr, int& fmt, unsigned long& captime, int no_convert)
{
    if(imgs[current] == 0)
        need();
    T **g = imgs[current];
    if(g == 0) oopanic("fileaq bad get");
    if(preloaded)
    {
        T **g2 = (T **)pm_allocarray(cols, rows, sizeof(T));
        bcopy(&g[0][0], &g2[0][0], cols * rows * sizeof(T));
        g = g2;
        bump(current);
    }
    else
        imgs[current] = 0;
    captime = (unsigned long)fake_time;
    bump(filenum);
    fake_time += 33.3667; // assume 29.97fps ntsc
    c = cols;
    r = rows;
    fmt = AQ_COLOR|AQ_RGB24;
    if(sizeof(T) == 3 && !no_convert)
    {
        // rgb to ycc conversion
        gray **out1, **out2, **out3;
        out1 = pgm_allocarray(cols, rows);
        out2 = pgm_allocarray(cols, rows);
        out3 = pgm_allocarray(cols, rows);
        JSAMPARRAY a[3];
        a[0] = out1;
        // swap r&b, braindead ms again
        // unfucked 2012
        //a[1] = out2;
        //a[2] = out3;
        // refucked
        a[1] = out3;
        a[2] = out2;
        rgb_ycc_convert((unsigned char **)g, a, 0, cols, rows);
        y = out1;
        cb = out2;
        cr = out3;
    }
    return g;
}

template<class T>
void
FileAcquire<T>::bump(int &i)
{
    i = (i + 1) % imgs.num_elems();
}

#endif
