
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
#include <QImageReader>
#include <QFileDevice>
#include <QMap>
#include <QSet>
#include <QDir>
#include <cstring>
#include "pfx.h"
#include "msgpv.h"
#include "dwycolist2.h"

[[noreturn]] void cdcxpanic(const char *);

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

static
QByteArray
get_no_img()
{
    return add_pfx(User_pfx, "no_img.png");
}

// The sender-supplied filename arrives off the network, so it gets no trust
// at all: it can carry a whole path (some clients send one, and we forward
// it verbatim to our own peers), it can carry characters that have no
// business in a filename, and it can be arbitrarily long. Keep the last path
// component and scrub the rest. This mirrors suggested_name() in MsgView.qml,
// but the ui can't be the only thing standing between the wire and a label --
// preview_saved_msg() also builds real paths out of this string.
static
QByteArray
sanitize_user_filename(const QByteArray& in)
{
    QByteArray n = in;
    // note: check both separator flavors regardless of platform. a peer on the
    // other side may have sent a windows path to a unix build.
    int i = n.lastIndexOf('/');
    if(i >= 0)
        n = n.mid(i + 1);
    i = n.lastIndexOf('\\');
    if(i >= 0)
        n = n.mid(i + 1);

    for(char& c : n)
    {
        unsigned char u = (unsigned char)c;
        // control chars and delete, plus the chars windows reserves
        if(u < 0x20 || u == 0x7f || strchr("<>:\"|?*", c) != nullptr)
            c = '_';
    }

    // leading/trailing dots and spaces are invisible and upset the fs
    int s = 0;
    while(s < n.size() && (n[s] == '.' || n[s] == ' '))
        ++s;
    int e = n.size();
    while(e > s && (n[e - 1] == '.' || n[e - 1] == ' '))
        --e;
    n = n.mid(s, e - s);

    if(n.isEmpty())
        return "attachment";
    if(n.size() > 80)
        n.truncate(80);
    return n;
}

static QSet<QByteArray> Supported_Image_Exts;

void
init_supported_image_exts()
{
    for(const QByteArray& fmt : QImageReader::supportedImageFormats())
        Supported_Image_Exts.insert("." + fmt.toLower());
}

static
int
is_supported_image_ext(const QByteArray& ext)
{
    return Supported_Image_Exts.contains(ext);
}

int
msg_file_is_image(const QByteArray& user_fn)
{
    // note: the extension is whatever comes after the last "." in the name,
    // which is the same thing preview_saved_msg derives.
    int idx = user_fn.lastIndexOf(".");
    if(idx == -1)
        return 0;
    QByteArray ext = user_fn.mid(idx);
    return is_supported_image_ext(ext.toLower());
}

static
int
has_cached_preview(const QByteArray& cached_name, QByteArray& preview_fn)
{
    for(const QByteArray& ext : Supported_Image_Exts)
    {
        if(QFile::exists(cached_name + ext))
        {
            preview_fn = cached_name + ext;
            return 1;
        }
    }
    return 0;
}

int
preview_saved_msg(const QByteArray& mid, QByteArray& preview_fn, int& file, QByteArray& full_size_filename,  QString& local_time)
{
    preview_fn = get_no_img();
    file = 0;

    DWYCO_SAVED_MSG_LIST qsm;
    if(dwyco_get_saved_message3(&qsm, mid.constData()) != DWYCO_GSM_SUCCESS)
    {
        return 0;
    }
    simple_scoped sm(qsm);
    //local_time = gen_time(sm, 0);
    QByteArray aname = sm.get<QByteArray>(DWYCO_QM_BODY_ATTACHMENT);
    if(aname.length() == 0)
    {
        return 0;
    }
    QByteArray cached_name = add_pfx(Tmp_pfx, aname);
    cached_name.replace(".dyc", "");
    cached_name.replace(".fle", "");

    if(has_cached_preview(cached_name, preview_fn))
        return 1;

    QByteArray fn;
    // note: is_file has to be answered by the raw name. sanitize_user_filename()
    // substitutes a placeholder for an empty one, and using that here would
    // make every unnamed attachment look like a file named "attachment".
    QByteArray raw_fn = sm.get<QByteArray>(DWYCO_QM_BODY_FILE_ATTACHMENT);
    int is_file = raw_fn.length() > 0;
    if(is_file)
    {
        QByteArray user_filename = sanitize_user_filename(raw_fn);
        try
        {
            file = 1;
            int idx = user_filename.lastIndexOf(".");
            if(idx == -1)
                throw 0;
            QByteArray rfn;
            user_filename.remove(0, idx);
            user_filename = user_filename.toLower();
            if(!is_supported_image_ext(user_filename))
                throw 0;
            aname.replace(".fle", "");
            rfn = aname;
            rfn += user_filename;
            // copy file out to random user_filename, scaling to preview size
            rfn = add_pfx(Tmp_pfx, rfn);
            full_size_filename = rfn;
            QByteArray rfn_native = QDir::toNativeSeparators(rfn).toLatin1();
            if(!dwyco_copy_out_file_zap2(mid.constData(), rfn_native.constData()))
                throw 0;
            preview_fn = rfn;
        }
        catch(...)
        {
            QByteArray mumble = get_no_img();

            QFile s(mumble);
            s.copy(cached_name + ".png");
            return 0;
        }
        return 1;
    }

    int viewid = dwyco_make_zap_view2(sm, 0);
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
        pvi.save(fn);
        preview_fn = fn;
        ret = 1;
    }
    //dwyco_zap_quick_stats_view(viewid, &video, &audio, &short_video);

    dwyco_delete_zap_view(viewid);
    return ret;
}

int
preview_msg_body(DWYCO_SAVED_MSG_LIST qsm, QByteArray& preview_fn, int& file, QByteArray& full_size_filename,  QString& local_time)
{
    preview_fn = get_no_img();
    file = 0;

    //local_time = gen_time(sm, 0);
    dwyco_list sm(qsm);
    QByteArray aname = sm.get<QByteArray>(DWYCO_QM_BODY_ATTACHMENT);
    if(aname.length() == 0)
    {
        return 0;
    }
    QByteArray cached_name = add_pfx(Tmp_pfx, aname);
    cached_name.replace(".dyc", "");
    cached_name.replace(".fle", "");
    if(has_cached_preview(cached_name, preview_fn))
        return 1;

    QByteArray fn;
    // note: is_file has to be answered by the raw name. sanitize_user_filename()
    // substitutes a placeholder for an empty one, and using that here would
    // make every unnamed attachment look like a file named "attachment".
    QByteArray raw_fn = sm.get<QByteArray>(DWYCO_QM_BODY_FILE_ATTACHMENT);
    int is_file = raw_fn.length() > 0;
    if(is_file)
    {
        QByteArray user_filename = sanitize_user_filename(raw_fn);
        try
        {
            file = 1;

            int idx = user_filename.lastIndexOf(".");
            if(idx == -1)
                throw 0;
            QByteArray rfn;
            user_filename.remove(0, idx);
            user_filename = user_filename.toLower();
            if(!is_supported_image_ext(user_filename))
                throw 0;
            aname.replace(".fle", "");
            rfn = aname;
            rfn += user_filename;
            // copy file out to random user_filename, scaling to preview size
            rfn = add_pfx(Tmp_pfx, rfn);
            full_size_filename = rfn;
            QByteArray rfn_native = QDir::toNativeSeparators(rfn).toLatin1();
            if(!dwyco_copy_out_qd_file_zap(sm, rfn_native.constData()))
                throw 0;

            preview_fn = rfn;
        }
        catch(...)
        {
            QByteArray mumble = get_no_img();

            QFile s(mumble);
            s.copy(cached_name + ".png");
            return 0;
        }
        return 1;
    }

    int viewid = dwyco_make_zap_view2(sm, 1);
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
        pvi.save(fn);
        preview_fn = fn;
        ret = 1;
    }

    dwyco_delete_zap_view(viewid);

    return ret;
}

int
msg_body_file_info(DWYCO_SAVED_MSG_LIST qsm, QByteArray& user_fn, qint64& size, int& is_image)
{
    user_fn = "";
    size = 0;
    is_image = 0;

    // note: the caller owns the list, so just wrap it, don't take ownership.
    dwyco_list sm(qsm);

    if(sm.is_nil(DWYCO_QM_BODY_ATTACHMENT))
        return 0;
    if(sm.is_nil(DWYCO_QM_BODY_FILE_ATTACHMENT))
        return 0;

    user_fn = sanitize_user_filename(sm.get<QByteArray>(DWYCO_QM_BODY_FILE_ATTACHMENT));
    is_image = msg_file_is_image(user_fn);

    long sz = 0;
    if(dwyco_qd_file_zap_size(qsm, &sz) && sz > 0)
        size = (qint64)sz;

    return 1;
}

int
msg_file_info(const QByteArray& mid, QByteArray& user_fn, qint64& size, int& is_image)
{
    user_fn = "";
    size = 0;
    is_image = 0;

    DWYCO_SAVED_MSG_LIST qsm;
    if(dwyco_get_saved_message3(&qsm, mid.constData()) != DWYCO_GSM_SUCCESS)
        return 0;
    simple_scoped sm(qsm);

    if(sm.is_nil(DWYCO_QM_BODY_ATTACHMENT))
        return 0;
    if(sm.is_nil(DWYCO_QM_BODY_FILE_ATTACHMENT))
        return 0;

    user_fn = sanitize_user_filename(sm.get<QByteArray>(DWYCO_QM_BODY_FILE_ATTACHMENT));
    is_image = msg_file_is_image(user_fn);

    long sz = 0;
    if(dwyco_file_zap_size(mid.constData(), &sz) && sz > 0)
        size = (qint64)sz;

    return 1;
}
