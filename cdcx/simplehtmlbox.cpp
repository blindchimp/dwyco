
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QNetworkAccessManager>
#include <QDebug>
#include "simplehtmlbox.h"
#include "ui_simplehtmlbox.h"

QNetworkAccessManager *Nam;

SimpleHtmlBox::SimpleHtmlBox(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::SimpleHtmlBox)
{
    ui->setupUi(this);
    if(!Nam)
    {
        Nam = new QNetworkAccessManager;
    }
//    connect(Nam, SIGNAL(finished(QNetworkReply*)),
//             this, SLOT(replyFinished(QNetworkReply*)));
    default_url = 1;
    reply = 0;



}

SimpleHtmlBox::~SimpleHtmlBox()
{
    delete ui;
}

void
SimpleHtmlBox::replyFinished(QNetworkReply *r)
{
    QByteArray resp = r->readAll();

    ui->textBrowser->setHtml(resp);
}

void
SimpleHtmlBox::reply_finished()
{
    QByteArray resp = reply->readAll();

    ui->textBrowser->setHtml(resp);

    QVector<QTextFormat> formats = ui->textBrowser->document()->allFormats();
    for(int i = 0; i <formats.count(); ++i)
    {
        if(formats[i].isCharFormat())
            qDebug() << formats[i].toCharFormat().anchorHref();
        if(formats[i].isImageFormat())
        {
            qDebug() << formats[i].toImageFormat().name();
            QTextImageFormat img = formats[i].toImageFormat();
            img.setName("/2/dwight/local/db/xinfo/0a031fdd90cada943134.jpg");



        }
    }
    reply->deleteLater();
    reply = 0;

}

void SimpleHtmlBox::on_filter_clicked()
{
    if(reply)
        return;
    create_new_url();
    QNetworkRequest r(url);
    reply = Nam->get(r);
    connect(reply, SIGNAL(finished()), this, SLOT(reply_finished()));
}

void
SimpleHtmlBox::set_to_default_url()
{
    url = QUrl("http://www.dwyco.com");
    default_url = 1;
}

void
SimpleHtmlBox::reload_triggered()
{


}
