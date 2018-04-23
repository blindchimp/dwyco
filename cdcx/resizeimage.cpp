
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QFile>
#include <QImage>
#include "qlimitedbuffer.h"
#include "resizeimage.h"
#include "mainwin.h"
#include "pfx.h"

static
void
save_random_filename(const QBuffer& img, QString& out_fn)
{
    DwOString tmp_fn = random_fn();
    tmp_fn += ".jpg";
    tmp_fn = add_pfx(Tmp_pfx, tmp_fn);
    const QByteArray &b = img.data();
    QFile out(tmp_fn);
    out.open(QIODevice::WriteOnly);
    if(out.write(b) != b.length())
        throw -1;
    out.close();
    out_fn = tmp_fn;
    return;
}

// simple heuristic to get a larger image down to less than a max size
int
load_and_resize(const QString &filename, QString& out_fn, int maxsize)
{
    out_fn = "";
    try
    {
        QFile infile(filename);

        if(!infile.exists())
            return 0;
        QImage in_img(filename);
        if(in_img.isNull())
            return 0;

        if(infile.size() <= maxsize)
        {
            out_fn = filename;
            return 1;
        }

        QImage sim;


        {
            QLimitedBuffer qlb(maxsize);

            sim = in_img.scaled(512, 512, Qt::KeepAspectRatio);
            if(sim.save(&qlb, "JPG"))
            {
                save_random_filename(qlb, out_fn);
                return 1;
            }
        }

        {
            QLimitedBuffer qlb(maxsize);

            if(sim.save(&qlb, "JPG", 50))
            {
                save_random_filename(qlb, out_fn);
                return 1;
            }
        }

        {
            QLimitedBuffer qlb(maxsize);
            sim = in_img.scaled(320, 240, Qt::KeepAspectRatio);
            if(sim.save(&qlb, "JPG"))
            {
                save_random_filename(qlb, out_fn);
                return 1;
            }
        }

        {
            QLimitedBuffer qlb(maxsize);

            if(sim.save(&qlb, "JPG", 50))
            {
                save_random_filename(qlb, out_fn);
                return 1;
            }
        }

    }
    catch(...)
    {
        if(out_fn.length() > 0)
            QFile::remove(out_fn);
        return 0;
    }
    return 0;

}

#ifdef TEST_RESIZE
int
main(int, char **argv)
{
    QString out_fn;
    load_and_resize(argv[1], out_fn, 65000);
}
#endif
