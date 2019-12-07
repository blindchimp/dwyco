
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QMap>
#include <QSet>
#include <QFile>
#include "dlli.h"
#include "profpv.h"
#include "pfx.h"

struct pvcache_entry
{
    QByteArray uid;
    QString info;
    QByteArray fn;
    int remove_filename;
    QImage img;
    pvcache_entry() {
        remove_filename = 0;
    }
};

static QMap<QByteArray, pvcache_entry> Preview_cache;

profpv *ThePreviewCache;
static QImage *No_img;
static QByteArray No_img_fn;

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

profpv::profpv()
{
    if(ThePreviewCache)
        ::abort();
    ThePreviewCache = this;
    QByteArray pfn = add_pfx(Sys_pfx, "no_img.png");
    No_img_fn = pfn;
    No_img = new QImage(pfn);


    //connect(Mainwinform, SIGNAL(content_filter_event(int)), this, SLOT(clear_cache(int)));
    //connect(Mainwinform, SIGNAL(invalidate_profile(QByteArray)), this, SLOT(remove_entry(QByteArray)));
    //connect(Mainwinform, SIGNAL(pal_event(QByteArray)), this, SLOT(remove_entry(QByteArray)));
    //connect(Mainwinform, SIGNAL(chat_server_status(int,int)), this, SLOT(chat_server(int,int)));
}


void
profpv::clear_cache(int)
{
    QMapIterator<QByteArray, pvcache_entry> i(Preview_cache);
    while (i.hasNext())
    {
        i.next();
        if(i.value().remove_filename)
            QFile::remove(i.value().fn);
    }
    Preview_cache.clear();
}

void
profpv::remove_entry(QByteArray uid)
{
    const pvcache_entry pve(Preview_cache.value(uid));
    if(pve.remove_filename)
        QFile::remove(pve.fn);

    Preview_cache.remove(uid);
}

void
profpv::remove_entry(QString uid)
{
    QByteArray buid = uid.toLatin1();
    buid = QByteArray::fromHex(buid);
    remove_entry(buid);
}

void
profpv::chat_server(int, int online)
{
    //Chat_online = online;
}

QImage
profpv::get_default_image()
{
    return *No_img;
}

QImage
profpv::get_preview_by_uid(const QByteArray& uid)
{
    QImage ret;
    pvcache_entry pve(Preview_cache.value(uid));
    char *ufn = 0;
    int len_ufn;

    if(pve.uid.length() == 0)
    {
        int viewid = dwyco_get_profile_to_viewer_sync(uid.constData(), uid.length(), &ufn, &len_ufn);
        if(viewid == 0)
        {
            // not in cache, or corrupt or something, so update the info
            dwyco_fetch_info(uid.constData(), uid.length());
            return *No_img;
        }
        if(viewid == -1)
        {
            // no image, maybe some kind of default
            pve.uid = uid;
            pve.img = *No_img;
            pve.fn = No_img_fn;
            Preview_cache.insert(uid, pve);
            return pve.img;
        }
        if(viewid == -2)
        {
            // a user speced file (usually a pic, which should have been
            // vetted by the server, we assume)
            QImage img(ufn);
            pve.uid = uid;
            pve.img = img;
            pve.fn = ufn;
            pve.remove_filename = 1;
            Preview_cache.insert(uid, pve);
            dwyco_free_array(ufn);
            return img;
        }
        // it is a dwyco profile video, create a preview
        // create random filename
        QByteArray fn = random_fn();
        fn += ".jpg";
        fn = add_pfx(Tmp_pfx, fn);
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
            pvi.save(fn);
            pve.uid = uid;
            pve.fn = fn;
            pve.img = pvi;
            pve.remove_filename = 1;
            Preview_cache.insert(uid, pve);
        }
        else
        {
            // no preview available (could be all audio, or some other problem)
            pve.uid = uid;
            pve.img = *No_img;
            pve.fn = No_img_fn;
            Preview_cache.insert(uid, pve);
        }
        dwyco_delete_zap_view(viewid);
        return pve.img;
    }

    if(!pve.img.isNull())
        return pve.img;

    if(pve.fn.length() > 0)
    {
        QImage pm(pve.fn);
        pve.img = pm;
        Preview_cache.insert(uid, pve);
        return pm;
    }
    return ret;
}


void
profpv::refresh_profile(const QByteArray& uid)
{

}
