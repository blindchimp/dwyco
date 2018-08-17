
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
#include "mainwin.h"
#include "profpv.h"
#include "pfx.h"

struct pvcache_entry
{
    DwOString uid;
    QString info;
    DwOString fn;
    int remove_filename;
    pvcache_entry() {
        remove_filename = 0;
    }
};

static QMap<DwOString, pvcache_entry> Preview_cache;
static QSet<DwOString> In_progress;
profpv *ThePreviewCache;
int profpv::Chat_online;

profpv::profpv()
{
    if(ThePreviewCache)
        ::abort();
    ThePreviewCache = this;
    connect(Mainwinform, SIGNAL(content_filter_event(int)), this, SLOT(clear_cache(int)));
    connect(Mainwinform, SIGNAL(invalidate_profile(DwOString)), this, SLOT(remove_entry(DwOString)));
    connect(Mainwinform, SIGNAL(pal_event(DwOString)), this, SLOT(remove_entry(DwOString)));
    connect(Mainwinform, SIGNAL(chat_server_status(int)), this, SLOT(chat_server(int)));
}

static void
DWYCOCALLCONV
display_profile(int succ, const char *reason,
                const char *s1, int len_s1,
                const char *s2, int len_s2,
                const char *s3, int len_s3,
                const char *filename,
                const char *uid, int len_uid,
                int reviewed, int regular,
                void *user_arg)
{

    QByteArray u(uid, len_uid);
    DwOString duid(uid, 0, len_uid);

    cdcx_set_refresh_users(1);
    In_progress.remove(duid);
    int dont_show = !show_profile(duid, reviewed, regular);
    const char *default_guy;
    if(dont_show)
        default_guy = ":/new/prefix1/redguy.png";
    else
        default_guy = ":/new/prefix1/greenguy.png";

    pvcache_entry pve;
    pve.fn = default_guy;
    pve.uid = duid;
    pve.info = dwyco_info_to_display(duid);
    if(!succ)
    {
        Preview_cache.insert(duid, pve);
        return;
    }
    if(succ == -1)
    {
        // no attachment, just text
        if(dont_show)
        {
            pve.fn = ":/new/prefix1/bad_grayguy.png";
        }
        else
        {
            pve.fn = ":/new/prefix1/grayguy.png";
        }

    }
    else if(succ == -2)
    {
        if(!dont_show)
        {
            pve.remove_filename = 1;
            pve.fn = filename;
        }
    }
    else if(dont_show)
    {

    }
    else
    {

        // this is for the popup profile
        DwOString fn = random_fn();
        fn += ".ppm";
        fn = add_pfx(Tmp_pfx, fn);
        if(dwyco_zap_create_preview(succ, fn.c_str(), fn.length()))
        {
            pve.fn = fn;
            pve.remove_filename = 1;
        }
        dwyco_delete_zap_view(succ);
    }
    Preview_cache.insert(duid, pve);
}

void
profpv::clear_cache(int)
{
    QMapIterator<DwOString, pvcache_entry> i(Preview_cache);
    while (i.hasNext())
    {
        i.next();
        if(i.value().remove_filename)
            QFile::remove(i.value().fn);
    }
    Preview_cache.clear();
}

void
profpv::remove_entry(DwOString uid)
{
    const pvcache_entry pve(Preview_cache.value(uid));
    if(pve.remove_filename)
        QFile::remove(pve.fn);

    Preview_cache.remove(uid);
}

void
profpv::chat_server(int online)
{
    Chat_online = online;
}

QPixmap
profpv::get_preview_by_uid(const DwOString& uid)
{
    QPixmap ret;
    const pvcache_entry pve(Preview_cache.value(uid));
    if(pve.uid.length() == 0)
    {
        refresh_profile(uid);
        return ret;
    }

    if(pve.fn.length() > 0)
    {
        QPixmap pm(pve.fn);
        return pm;
    }
    return ret;
}


void
profpv::refresh_profile(const DwOString& uid)
{
    if(!Chat_online || In_progress.contains(uid))
    {
        return;
    }
    In_progress.insert(uid);
    dwyco_get_profile_to_viewer(uid.constData(), uid.length(),
                                display_profile, 0);

}
