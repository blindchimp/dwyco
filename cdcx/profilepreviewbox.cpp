
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef CDCX_WEBKIT
#include <QDockWidget>
#include <QSslSocket>
#ifdef DWYCO_QT5
#include <QUrlQuery>
#endif
#include "profilepreviewbox.h"
#include "dlli.h"
#include "mainwin.h"
#include "tfhex.h"
#include "ui_browsebox.h"
#include "ui_simplehtmlbox.h"

#undef LOCAL_TEST
extern DwOString My_uid;
extern int Current_server;
extern DwOString Current_server_id;
extern int AvoidSSL;

ProfilePreviewBox::ProfilePreviewBox(QWidget *parent) :
    WHATBOX(parent)
{
    setWindowTitle("Profiles");
}

void
ProfilePreviewBox::create_new_url()
{
    const char *auth;
    int len;

    if(!dwyco_get_authenticator(&auth, &len))
        return;
    QByteArray au(auth, len);
    QByteArray auth_str = au.toPercentEncoding();
    dwyco_free_array((char *)auth);

    //url.setUrl("http://s6.blindchimp.com/cgi-bin/genhtdir.sh");
#ifdef LOCAL_TEST
    url.setUrl("http://localhost/cgi-bin/mkhtdir.sh");
#else
#ifndef CDCX_NO_SSL
    if(QSslSocket::supportsSsl() && !AvoidSSL)
        url.setUrl("https://profiles.dwyco.org/cgi-bin/mkhtdir.sh");
    else
#endif
        url.setUrl("http://profiles.dwyco.org/cgi-bin/mkhtdir.sh");
#endif
    // this has to be updated for qt5
#ifdef DWYCO_QT5
    QUrlQuery qurl;

    qurl.addQueryItem("uid", to_hex(My_uid).c_str());
    qurl.addQueryItem("filt", QString::number(!ui->filter->isChecked()));
    if(Current_server != -1)
        qurl.addQueryItem("sid", QString::number(Current_server));
    else if(Current_server_id.length() > 0)
        qurl.addQueryItem("sid", Current_server_id.c_str());
    else
        return;
    qurl.addQueryItem("auth", auth_str);

    url.setQuery(qurl);
#else
    url.addQueryItem("uid", to_hex(My_uid).c_str());
    url.addQueryItem("filt", QString::number(!ui->filter->isChecked()));
    if(Current_server != -1)
        url.addQueryItem("sid", QString::number(Current_server));
    else if(Current_server_id.length() > 0)
        url.addQueryItem("sid", Current_server_id.c_str());
    else
        return;

    url.addEncodedQueryItem("auth", auth_str);
#endif
}
#endif
