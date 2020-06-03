
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "mainwin.h"
#include "qwiz.h"
#include "ssmap.h"
#include "dlli.h"
#include "config.h"

extern int Block_DLL;

void
dowiz()
{
    QWizard *wiz = new QWizard;
    wiz->setOption(QWizard::NoCancelButton);
    wiz->setOption(QWizard::NoDefaultButton);
    wiz->addPage(new toswarn_page);
    userdata_page *p1;
    wiz->addPage(p1 = new userdata_page);
    wiz->addPage(new netspeed_page);

    wiz->setPixmap(QWizard::LogoPixmap, QPixmap(":/new/prefix1/greenguy.png"));
    wiz->setPixmap(QWizard::WatermarkPixmap, QPixmap(":/new/prefix1/wizdec.png"));
    //wiz->show();
    //wiz->resize(800, 800);
    Block_DLL = 1;

    wiz->exec();

    QVariant qv;
    QByteArray b;

    qv = wiz->field("name");
    QByteArray name = strip_html(qv.toString()).toAscii();
    qv = wiz->field("location");
    QByteArray location = strip_html(qv.toString()).toAscii();
    QByteArray desc = strip_html(p1->ui.desc->toPlainText()).toAscii();
    qv = wiz->field("email");
    QByteArray email = strip_html(qv.toString()).toAscii();

    // if they manage to cancel out of the wizard, make sure
    // some reasonable defaults get set up
    if(name.length() == 0)
        name = fname();

    dwyco_create_bootstrap_profile(
        name.constData(), name.length(),
        desc.constData(), desc.length(),
        location.constData(), location.length(),
        email.constData(), email.length()
    );



    // b/w settings
    int up = wiz->field("upload").toInt();
    int down = wiz->field("download").toInt();
    if(up == 0)
        up = 256;
    if(down == 0)
        down = 256;
    dwyco_set_rate_tweaks(20, 65535, up, down);

    setting_put("first-run", 0);
    settings_save();
    Block_DLL = 0;
    TheConfigForm->load();
    Mainwinform->move(0, Mainwinform->y());
}

void
netspeed_page::on_run_bw_test_button_clicked()
{
    int bw_in;
    int bw_out;

    dwyco_estimate_bandwidth2(&bw_out, &bw_in);
    QString in;
    in.setNum(bw_in / 1000);
    QString out;
    out.setNum(bw_out / 1000);
    ui.upload_speed->setText(out);
    ui.download_speed->setText(in);
    QMessageBox::information(0, "Bandwidth report", QString(
                                 "Download %1 kbps, Upload %2 kbps").arg(in, out));
}
