
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "dlli.h"
#include <QByteArray>
#include <QPixmap>
#include <QFile>
#include <QImage>
#include "pfx.h"
#include "msgpv.h"
#include "dwycolistscoped.h"
#include "qfiletweak.h"

void cdcxpanic(const char *);


static QByteArray
dwyco_get_attr(DWYCO_LIST l, int row, const char *col)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        cdcxpanic("bogus list get");
    if(type != DWYCO_TYPE_STRING && type != DWYCO_TYPE_NIL)
        cdcxpanic("bogus type");
    return QByteArray(val, len);
}

struct img_info
{
    img_info(const char *ap, int ar) : p(ap), rows(ar) {}

    const char *p;
    int rows;

};

static
void
image_cleanup(void *info)
{
    img_info *pvi = (img_info *)info;
    dwyco_free_image((char *)pvi->p, pvi->rows);
    delete pvi;
}

int
preview_saved_msg(const QByteArray& uid, const QByteArray& mid, QByteArray& preview_fn, int& file, QByteArray& full_size_filename,  QString& local_time)
{
    preview_fn = add_pfx(Sys_pfx, "no_img.png");
    file = 0;

    DWYCO_SAVED_MSG_LIST qsm;
    if(!dwyco_get_saved_message(&qsm, uid.constData(), uid.length(), mid.constData()))
    {
        return 0;
    }
    simple_scoped sm(qsm);
    //local_time = gen_time(sm, 0);
    QByteArray aname = dwyco_get_attr(sm, 0, DWYCO_QM_BODY_ATTACHMENT);
    if(aname.length() == 0)
    {
        return 0;
    }
    QByteArray cached_name = add_pfx(Tmp_pfx, aname);
    cached_name.replace(".dyc", "");
    cached_name.replace(".fle", "");
    if(QFile::exists(cached_name + ".jpg"))
    {
        preview_fn = cached_name + ".jpg";
        preview_fn = add_pfx("image://untweak/", preview_fn);
        return 1;
    }
    if(QFile::exists(cached_name + ".jpeg"))
    {
        preview_fn = cached_name + ".jpeg";
        preview_fn = add_pfx("image://untweak/", preview_fn);
        return 1;
    }
    if(QFile::exists(cached_name + ".ppm"))
    {
        preview_fn = cached_name + ".ppm";
        preview_fn = add_pfx("image://untweak/", preview_fn);
        return 1;
    }
    if(QFile::exists(cached_name + ".png"))
    {
        preview_fn = cached_name + ".png";
        preview_fn = add_pfx("image://untweak/", preview_fn);
        return 1;
    }
    QByteArray fn;
    QByteArray user_filename = dwyco_get_attr(sm, 0, DWYCO_QM_BODY_FILE_ATTACHMENT);
    int is_file = user_filename.length() > 0;
    if(is_file)
    {
        try
        {
            file = 1;
            // chop everything off except the extension, then add our random name on the
            // front of it. tries to avoid problems with goofy filenames being sent in.
            int idx = user_filename.lastIndexOf(".");
            if(idx == -1)
                throw 0;
            QByteArray rfn;
            user_filename.remove(0, idx);
            user_filename = user_filename.toLower();
            // don't bother with weird images, it can cause qt to choke, esp if it is large
            if(!(user_filename == ".jpg" || user_filename == ".jpeg" || user_filename == ".png" || user_filename == ".ppm"))
                throw 0;
            aname.replace(".fle", "");
            rfn = aname;
            rfn += user_filename;
            // copy file out to random user_filename, scaling to preview size
            rfn = add_pfx(Tmp_pfx, rfn);
            full_size_filename = rfn;
            const char *buf;
            int len_buf;
            if(!dwyco_copy_out_file_zap_buf(uid.constData(), uid.length(), mid.constData(), &buf, &len_buf))
                throw 0;
            QFileTweak f(rfn);
            f.open(QFile::WriteOnly);
            if(f.write(buf, len_buf) != len_buf)
            {
                dwyco_free_array((char *)buf);
                throw 0;
            }
            f.close();
            preview_fn = rfn;
            dwyco_free_array((char *)buf);
            preview_fn = add_pfx("image://untweak", preview_fn);
        }
        catch(...)
        {
            QByteArray mumble = add_pfx(Sys_pfx, "no_img.png");

            QFile s(mumble);
            s.copy(cached_name + ".png");
            return 0;
        }
        return 1;
    }

    int viewid = dwyco_make_zap_view(sm, uid.constData(), uid.length(), 0);
    if(viewid == 0)
    {
        return 0;
    }
    // create random filename
    fn = aname;
    fn.replace(".dyc", "");
    fn += ".jpg";
    fn = add_pfx(Tmp_pfx, fn);
    int ret = 0;
    const char *buf;
    int cols;
    int rows;
    int len;
    if(dwyco_zap_create_preview_buf(viewid, &buf, &len, &cols, &rows))
    {
        // this looks goofy, but we know what comes back is a PPM
        // in memory, which the first word contains a pointer to the
        // image data
        QImage pvi(((const uchar **)buf)[0], cols, rows, cols * 3, QImage::Format_RGB888, image_cleanup, new img_info(buf, rows));
        //pvi.save(fn);
        QFileTweak qft(fn);
        qft.open(QFile::WriteOnly);
        pvi.save(&qft);
        preview_fn = add_pfx("image://untweak/", fn);
        ret = 1;
    }
    //dwyco_zap_quick_stats_view(viewid, &video, &audio, &short_video);

    dwyco_delete_zap_view(viewid);
    return ret;
}

int
preview_unsaved_msg(DWYCO_UNSAVED_MSG_LIST sm, QByteArray& preview_fn, int& file, QByteArray& full_size_filename,  QString& local_time)
{
    preview_fn = add_pfx(Sys_pfx, "no_img.png");
    file = 0;

    //local_time = gen_time(sm, 0);
    QByteArray aname = dwyco_get_attr(sm, 0, DWYCO_QM_BODY_ATTACHMENT);
    if(aname.length() == 0)
    {
        return 0;
    }
    QByteArray cached_name = add_pfx(Tmp_pfx, aname);
    cached_name.replace(".dyc", "");
    cached_name.replace(".fle", "");
    if(QFile::exists(cached_name + ".jpg"))
    {
        preview_fn = cached_name + ".jpg";

        return 1;
    }
    if(QFile::exists(cached_name + ".jpeg"))
    {
        preview_fn = cached_name + ".jpeg";

        return 1;
    }
    if(QFile::exists(cached_name + ".ppm"))
    {
        preview_fn = cached_name + ".ppm";

        return 1;
    }
    if(QFile::exists(cached_name + ".png"))
    {
        preview_fn = cached_name + ".png";

        return 1;
    }
    QByteArray fn;
    QByteArray user_filename = dwyco_get_attr(sm, 0, DWYCO_QM_BODY_FILE_ATTACHMENT);
    int is_file = user_filename.length() > 0;
    if(is_file)
    {
        try
        {
            file = 1;

            // chop everything off except the extension, then add our random name on the
            // front of it. tries to avoid problems with goofy filenames being sent in.
            int idx = user_filename.lastIndexOf(".");
            if(idx == -1)
                throw 0;
            QByteArray rfn;
            user_filename.remove(0, idx);
            if(!(user_filename == ".jpg" || user_filename == ".jpeg" || user_filename == ".png" || user_filename == ".ppm"))
                throw 0;
            aname.replace(".fle", "");
            rfn = aname;
            rfn += user_filename;
            // copy file out to random user_filename, scaling to preview size
            rfn = add_pfx(Tmp_pfx, rfn);
            full_size_filename = rfn;
            if(!dwyco_copy_out_unsaved_file_zap(sm, rfn.constData()))
                throw 0;

            preview_fn = rfn;
        }
        catch(...)
        {
            QByteArray mumble = add_pfx(Sys_pfx, "no_img.png");

            QFile s(mumble);
            s.copy(cached_name + ".png");
            return 0;
        }
        return 1;
    }

    int viewid = dwyco_make_zap_view(sm, 0, 0, 1);
    if(viewid == 0)
    {
        return 0;
    }
    // create random filename
    fn = aname;
    fn.replace(".dyc", "");
    fn += ".jpg";
    fn = add_pfx(Tmp_pfx, fn);
    int ret = 0;
    const char *buf;
    int cols;
    int rows;
    int len;
    if(dwyco_zap_create_preview_buf(viewid, &buf, &len, &cols, &rows))
    {
        // this looks goofy, but we know what comes back is a PPM
        // in memory, which the first word contains a pointer to the
        // image data
        QImage pvi(((const uchar **)buf)[0], cols, rows, cols * 3, QImage::Format_RGB888, image_cleanup, new img_info(buf, rows));
        //pvi.save(fn);
        QFileTweak qft(fn);
        qft.open(QFile::WriteOnly);
        pvi.save(&qft);
        preview_fn = add_pfx("image://untweak", fn);
        ret = 1;
    }

    dwyco_delete_zap_view(viewid);

    return ret;
}
